#include "angband.h"

#include <assert.h>

bool devicemaster_desperation = FALSE;

static int _speciality_tval(int psubclass)
{
    switch (psubclass)
    {
    case DEVICEMASTER_RODS: return TV_ROD;
    case DEVICEMASTER_STAVES: return TV_STAFF;
    case DEVICEMASTER_WANDS: return TV_WAND;
    case DEVICEMASTER_POTIONS: return TV_POTION;
    case DEVICEMASTER_SCROLLS: return TV_SCROLL;
    }
    return TV_WAND;
}

void _desperation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "绝境");
        break;
    case SPELL_DESC:
        var_set_string(res, "消耗所选装置的多次使用次数以获得额外的力量。然而，所选的装置可能会发生爆炸。");
        break;
    case SPELL_CAST:
        devicemaster_desperation = TRUE;
        switch (p_ptr->psubclass)
        {
        case DEVICEMASTER_RODS: do_cmd_zap_rod(); break;
        case DEVICEMASTER_WANDS: do_cmd_aim_wand(); break;
        case DEVICEMASTER_STAVES: do_cmd_use_staff(); break;
        case DEVICEMASTER_POTIONS: do_cmd_quaff_potion(); break;
        case DEVICEMASTER_SCROLLS: do_cmd_read_scroll(); break;
        }
        devicemaster_desperation = FALSE;
        var_set_bool(res, energy_use != 0);
        break;
    case SPELL_ENERGY:
        var_set_int(res, energy_use);     /* already set correctly by do_cmd_*() */
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

bool _detect_devices(int range)
{
    bool result = FALSE;
    int i, y, x;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        if (!o_ptr->k_idx) continue;
        if (o_ptr->held_m_idx) continue;

        y = o_ptr->loc.y;
        x = o_ptr->loc.x;

        if (distance(py, px, y, x) > range) continue;

        switch (o_ptr->tval)
        {
        case TV_ROD:
        case TV_WAND:
        case TV_STAFF:
        case TV_SCROLL:
        case TV_POTION:
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;
            lite_spot(y, x);
            result = TRUE;
        }
    }

    if (result)
        msg_print("你感觉到了魔法装置的存在！");

    return result;
}

void _detect_devices_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测装置");
        break;
    case SPELL_DESC:
        var_set_string(res, "探测附近的魔法装置。");
        break;
    case SPELL_CAST:
        _detect_devices(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _is_device(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_ROD:
    case TV_WAND:
    case TV_STAFF:
    case TV_SCROLL:
    case TV_POTION:
        return TRUE;
    }
    return FALSE;
}

void _identify_device_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鉴定装置");
        break;
    case SPELL_DESC:
        var_set_string(res, "鉴定一件魔法装置。");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 25)
            var_set_bool(res, identify_fully(_is_device));
        else
            var_set_bool(res, ident_spell(_is_device));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _recharging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "充能");
        break;
    case SPELL_DESC:
        var_set_string(res, "它尝试使用另一个装置的能量来为目标装置充能。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("强度 %d", 50 + 2*p_ptr->lev));
        break;
    case SPELL_CAST:
        /* Devicemasters have no mana */
        var_set_bool(res, recharge_from_player(50 + 2*p_ptr->lev));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static object_type *_transfer_src_obj = NULL;
static bool _transfer_obj_p(object_type *o_ptr)
{
    if ( o_ptr->tval == _speciality_tval(p_ptr->psubclass)
      && o_ptr != _transfer_src_obj )
    {
        if (p_ptr->psubclass == DEVICEMASTER_POTIONS || p_ptr->psubclass == DEVICEMASTER_SCROLLS)
        {
            /* One may not use worthless high level items as source objects (e.g. Curse Armor could make Genocide!!) */
            if (!_transfer_src_obj && k_info[o_ptr->k_idx].cost <= 0)
                return FALSE;
            /* Potions and scrolls must transfer to weaker destination objects */
            if (_transfer_src_obj && k_info[_transfer_src_obj->k_idx].level < k_info[o_ptr->k_idx].level)
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

static obj_ptr _get_src_obj(void)
{
    obj_prompt_t prompt = {0};

    _transfer_src_obj = NULL;

    prompt.prompt = "从哪件物品转移？";
    prompt.error = "你没有可用的来源物品。";
    prompt.filter = _transfer_obj_p;
    prompt.where[0] = INV_PACK;

    obj_prompt(&prompt);
    return prompt.obj;
}

static obj_ptr _get_dest_obj(obj_ptr src_obj)
{
    obj_prompt_t prompt = {0};

    _transfer_src_obj = src_obj;

    prompt.prompt = "转移到哪件物品？";
    prompt.error = "你没有可用的目标物品。";
    prompt.filter = _transfer_obj_p;
    prompt.where[0] = INV_PACK;

    obj_prompt(&prompt);
    return prompt.obj;
}

static bool _transfer_effect(void)
{
    obj_ptr src_obj, dest_obj;

    /* Choose the objects */
    src_obj = _get_src_obj();
    if (!src_obj) return FALSE;
    if (object_is_artifact(src_obj))
    {
        msg_print("失败！你不能从神器中转移。");
        return FALSE;
    }

    dest_obj = _get_dest_obj(src_obj);
    if (!dest_obj) return FALSE;
    if (dest_obj == src_obj)
    {
        msg_print("失败！请为来源和目标选择不同的物品。");
        return FALSE;
    }
    if (object_is_artifact(dest_obj))
    {
        msg_print("失败！你不能转移到神器中。");
        return FALSE;
    }
    if (device_level(dest_obj) < src_obj->activation.difficulty)
    {
        msg_print("失败！目标装置不够强大，无法接收来源的效果。");
        return FALSE;
    }

    /* Move the effect */
    dest_obj->activation.type = src_obj->activation.type;
    dest_obj->activation.difficulty = src_obj->activation.difficulty;
    dest_obj->activation.cost = src_obj->activation.cost;
    dest_obj->activation.extra = src_obj->activation.extra;

    /* Destroy the source */
    assert(src_obj->number == 1); /* Wands/Rods/Staves no longer stack */
    obj_zero(src_obj);
    obj_release(src_obj, 0);

    return TRUE;
}

static bool _transfer_essence(void)
{
    int tval = _speciality_tval(p_ptr->psubclass);
    object_type *src_obj = NULL, *dest_obj = NULL;
    object_kind *src_kind = NULL, *dest_kind = NULL;
    int src_charges = 0, dest_charges = 0, max_charges = 0, power = 0;

    /* Choose the objects */
    src_obj = _get_src_obj();
    if (!src_obj) return FALSE;
    src_kind = &k_info[src_obj->k_idx];

    dest_obj = _get_dest_obj(src_obj);
    if (!dest_obj) return FALSE;
    dest_kind = &k_info[dest_obj->k_idx];

    if (dest_obj == src_obj)
    {
        msg_print("失败！请为来源和目标选择不同的物品。");
        return FALSE;
    }
    if (tval == TV_SCROLL || tval == TV_POTION)
    {
        if (dest_kind->level > src_kind->level) /* Double Check ... should already be excluded! */
        {
            msg_print("失败！这股精华太弱了，无法转移到该物品上。");
            return FALSE;
        }
    }

    src_charges = src_obj->number;
    max_charges = 99 - dest_obj->number;

    if (max_charges <= 0)
    {
        msg_print("失败！目标物品已经充满电了。");
        return FALSE;
    }

    power = src_charges * src_kind->level;
    power = power / 2;
    dest_charges = power / MAX(dest_kind->level, 10);

    if (dest_charges > max_charges)
        dest_charges = max_charges;

    if (dest_charges <= 0)
    {
        msg_print("失败！来源物品不够强大，连一次使用次数都无法转移。");
        return FALSE;
    }

    /* Perform the transfer */
    dest_obj->number += dest_charges;
    obj_dec_number(src_obj, src_charges, TRUE);

    obj_release(dest_obj, 0);
    obj_release(src_obj, 0);
    return TRUE;
}
void _transfer_charges_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->psubclass != DEVICEMASTER_POTIONS && p_ptr->psubclass != DEVICEMASTER_SCROLLS)
            var_set_string(res, "转移效果");
        else
            var_set_string(res, "转移精华");
        break;
    case SPELL_DESC:
        if (p_ptr->psubclass == DEVICEMASTER_POTIONS)
            var_set_string(res, "将精华从一瓶药水转移到另一瓶药水。");
        else if (p_ptr->psubclass == DEVICEMASTER_SCROLLS)
            var_set_string(res, "将精华从一张卷轴转移到另一张卷轴。");
        else if (p_ptr->psubclass == DEVICEMASTER_WANDS)
            var_set_string(res, "将效果从一根魔杖转移到另一根魔杖，此举会摧毁来源的魔杖。");
        else if (p_ptr->psubclass == DEVICEMASTER_RODS)
            var_set_string(res, "将效果从一根魔棒转移到另一根魔棒，此举会摧毁来源的魔棒。");
        else if (p_ptr->psubclass == DEVICEMASTER_STAVES)
            var_set_string(res, "将效果从一根法杖转移到另一根法杖，此举会摧毁来源的法杖。");
        break;
    case SPELL_CAST:
        if (p_ptr->psubclass != DEVICEMASTER_POTIONS && p_ptr->psubclass != DEVICEMASTER_SCROLLS)
            var_set_bool(res, _transfer_effect());
        else
            var_set_bool(res, _transfer_essence());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


static spell_info _get_spells[] = 
{
    /*lvl cst fail spell */
    {  1,  1, 30, _detect_devices_spell},
    {  5,  5, 30, _identify_device_spell},
    { 10, 15, 60, _recharging_spell},
    { 15, 25, 60, _transfer_charges_spell},
    { 25,  0,  0, _desperation_spell},
    { -1, -1, -1, NULL}
};

cptr devicemaster_speciality_name(int psubclass)
{
    switch (psubclass)
    {
    case DEVICEMASTER_RODS: return "魔棒";
    case DEVICEMASTER_STAVES: return "法杖";
    case DEVICEMASTER_WANDS: return "魔杖";
    case DEVICEMASTER_POTIONS: return "药水";
    case DEVICEMASTER_SCROLLS: return "卷轴";
    }
    return "";
}

cptr devicemaster_internal_speciality_name(int psubclass)
{
    switch (psubclass)
    {
    case DEVICEMASTER_RODS: return "Rods";
    case DEVICEMASTER_STAVES: return "Staves";
    case DEVICEMASTER_WANDS: return "Wands";
    case DEVICEMASTER_POTIONS: return "Potions";
    case DEVICEMASTER_SCROLLS: return "Scrolls";
    }
    return "";
}

cptr devicemaster_speciality_desc(int psubclass)
{
    switch (psubclass)
    {
    case DEVICEMASTER_RODS: return "你专精于使用魔棒。";
    case DEVICEMASTER_STAVES: return "你专精于使用法杖。";
    case DEVICEMASTER_WANDS: return "你专精于使用魔杖。";
    case DEVICEMASTER_POTIONS: return "你专精于使用药水。";
    case DEVICEMASTER_SCROLLS: return "你专精于使用卷轴。";
    }
    return "";
}

bool devicemaster_is_speciality(object_type *o_ptr)
{
    if (p_ptr->pclass == CLASS_DEVICEMASTER)
    {
        if (_speciality_tval(p_ptr->psubclass) == o_ptr->tval)
            return TRUE;
    }
    return FALSE;
}


static void _birth(void) 
{ 
    object_type    forge;

    switch (p_ptr->psubclass)
    {
    case DEVICEMASTER_RODS:
        object_prep(&forge, lookup_kind(TV_ROD, SV_ANY));
        if (device_init_fixed(&forge, EFFECT_DETECT_MONSTERS))
            py_birth_obj(&forge);
        break;
    case DEVICEMASTER_STAVES:
        object_prep(&forge, lookup_kind(TV_STAFF, SV_ANY));
        if (device_init_fixed(&forge, EFFECT_SLEEP_MONSTERS))
            py_birth_obj(&forge);
        break;
    case DEVICEMASTER_WANDS:
        object_prep(&forge, lookup_kind(TV_WAND, SV_ANY));
        if (device_init_fixed(&forge, EFFECT_SLEEP_MONSTER))
            py_birth_obj(&forge);
        break;
    case DEVICEMASTER_POTIONS:
        object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_SPEED));
        forge.number = 6;
        py_birth_obj(&forge);
        break;
    case DEVICEMASTER_SCROLLS:
        object_prep(&forge, lookup_kind(TV_SCROLL, SV_SCROLL_TELEPORT));
        forge.number = 6;
        py_birth_obj(&forge);
        break;
    }
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_WAND, EFFECT_BOLT_MISSILE, 1);
}

static void _character_dump(doc_ptr doc)
{
    cptr desc = devicemaster_speciality_name(p_ptr->psubclass);

    doc_printf(doc, "<topic:Abilities>==================================== <color:keypress>A</color> 能力 ====================================\n\n");

    {
        int pow = p_ptr->lev / 10;
        if (pow)
            doc_printf(doc, "* 你在使用%s时获得 +%d%% 的强度。\n", desc, device_power_aux(100, pow) - 100);
    }
    doc_printf(doc, "* 你使用%s的速度更快。\n", desc);
    if (p_ptr->psubclass != DEVICEMASTER_POTIONS && p_ptr->psubclass != DEVICEMASTER_SCROLLS)
        doc_printf(doc, "* 你在使用%s时有几率不消耗使用次数。\n", desc);
    else
        doc_printf(doc, "* 你在使用%s时有几率不消耗物品。\n", desc);
    if (p_ptr->psubclass != DEVICEMASTER_POTIONS && p_ptr->psubclass != DEVICEMASTER_SCROLLS)
        doc_printf(doc, "* 即使在恐惧状态下，你也可以使用%s。\n", desc);
    doc_printf(doc, "* 你对吸取充能效果有抗性 (强度=%d)。\n\n", p_ptr->lev);

    py_dump_spells(doc); 
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.which_stat = A_INT;
        me.magic_desc = "才能";
        me.encumbrance.max_wgt = 1000;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 1200;
        init = TRUE;
    }
    return &me;
}

class_t *devicemaster_get_class(int psubclass)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  40,  36,   2,  20,  16,  48,  35 };
    skills_t xs = {  7,  15,  10,   0,   0,   0,  13,  11 };

        me.name = "装置大师";
        me.desc = 
            "Devicemasters are excellent with magical devices, but poor in most other skills. "
            "They may shoot or use melee in a pinch, but this will never be their forte; instead, "
            "they conquer through their arsenal of magical devices.\n \n"
            "Each Devicemaster chooses to specialize in a particular class of devices, and will "
            "gain extra bonuses for speciality items. These bonuses include increased "
            "damage, increased speed of activation, extra resistance to charge draining, "
            "and even the ability to occasionally power these devices without consuming charges; "
            "each of these abilities becomes stronger with experience. In addition, speciality devices "
            "are easier to use, although all Devicemasters are also good with devices of other types.\n \n"
            "Devicemasters have several useful device-related powers; they may detect magical devices from "
            "a distance, and gain the powerful talent of Recharging very early on. "
            "At higher levels, they gain the ability to move effects from "
            "one device to another, or to move magical essences from one item to another if they specialize "
            "in Potions or Scrolls. In times of desperate need, the Devicemaster may drain a speciality item "
            "of multiple charges at once; this greatly increases the power of the "
            "effect, but may destroy the device in the process.";
    
        me.stats[A_STR] = -1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 101;
        me.base_hp = 6;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.get_spells = _get_spells;
        me.character_dump = _character_dump;
        me.caster_info = _caster_info;
        init = TRUE;
    }

    me.subname = devicemaster_speciality_name(psubclass);
    me.subdesc = devicemaster_speciality_desc(psubclass);
    return &me;
}
