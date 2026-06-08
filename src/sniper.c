#include "angband.h"

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_BOW, SV_LIGHT_XBOW, 1);
    py_birth_obj_aux(TV_BOLT, SV_BOLT, rand_range(20, 30));
}

/************************************************************************
 * Bonuses
 ***********************************************************************/
static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if ((info_ptr->tval_ammo == TV_BOLT) || (info_ptr->tval_ammo == TV_ANY_AMMO))
    {
        info_ptr->to_h += 10 + p_ptr->lev/5;
        info_ptr->dis_to_h += 10 + p_ptr->lev/5;
    }
    if (info_ptr->base_shot > 100)
        info_ptr->base_shot = 100 + (info_ptr->base_shot - 100) / 2;
}

/************************************************************************
 * Concentration
 ***********************************************************************/
static int _max_concentration(void)
{
    return 2 + (p_ptr->lev + 5)/10;
}

void reset_concentration(bool msg)
{
    if (msg)
        msg_print("你停止了专注。");

    p_ptr->concent = 0;
    reset_concent = FALSE;
    p_ptr->update |= PU_BONUS;
    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_MONSTERS;
}

int boost_concentration_damage(int tdam)
{
    return tdam * (10 + p_ptr->concent) / 10;
}

static void _concentrate(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "专注");
        break;
    case SPELL_DESC:
        var_set_string(res,
            "Concentrate your mind for more powerful shooting. As you increase "
            "your focus, you will gain access to more deadly archery techniques. "
            "In addition, you will land more critical shots as your aim improves.");
        break;
    case SPELL_INFO:
        var_set_string(res, format("(%d / %d)", p_ptr->concent, _max_concentration()));
        break;
    case SPELL_CAST: {
        int max = _max_concentration();
        if (p_ptr->concent < max)
        {
            p_ptr->concent++;
            msg_format("你进行了深度专注 (<color:%c>%dx</color>)。",
                p_ptr->concent == max ? 'r' : 'B',
                p_ptr->concent);
            p_ptr->update |= PU_BONUS;
            p_ptr->redraw |= PR_STATUS;
            p_ptr->update |= PU_MONSTERS;
        }
        else
            msg_format("你保持了最大程度的专注 (<color:r>%dx</color>)。", p_ptr->concent);
        reset_concent = FALSE;
        var_set_bool(res, TRUE);
        break; }
    default:
        default_spell(cmd, res);
    }
}

/************************************************************************
 * Snipe Techniques: Callbacks for do_cmd_fire based on shoot_hack
 ***********************************************************************/
int sniper_multiplier(int which, obj_ptr ammo, monster_type *m_ptr)
{
    int           mult = 10;
    monster_race *r_ptr = NULL;
    u32b          flgs[OF_ARRAY_SIZE] = {0};

    if (m_ptr)
        r_ptr = &r_info[m_ptr->r_idx];
    if (ammo)
        missile_flags(ammo, flgs);

    switch (which)
    {
    case SP_LITE:
        if (!r_ptr || (r_ptr->flags3 & RF3_HURT_LITE))
        {
            mult = 20 + p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_HURT_LITE);
        }
        break;
    case SP_FIRE:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_FIRE_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_FIRE_MASK);
        }
        else
        {
            mult = 15 + 3*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_FIRE))
                mult += 5;
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_FIRE))
            {
                mult *= 2;
                mon_lore_3(m_ptr, RF3_HURT_FIRE);
            }
        }
        break;
    case SP_COLD:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_COLD_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_COLD_MASK);
        }
        else
        {
            mult = 15 + 3*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_COLD))
                mult += 5;
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_COLD))
            {
                mult *= 2;
                mon_lore_3(m_ptr, RF3_HURT_COLD);
            }
        }
        break;
    case SP_ELEC:
        if (r_ptr && (r_ptr->flagsr & RFR_EFF_IM_ELEC_MASK))
        {
            mon_lore_r(m_ptr, RFR_EFF_IM_ELEC_MASK);
        }
        else
        {
            mult = 18 + 4*p_ptr->concent;
            if (have_flag(flgs, OF_BRAND_ELEC))
                mult += 7;
        }
        break;
    case SP_KILL_WALL:
        if (!r_ptr || (r_ptr->flags3 & RF3_HURT_ROCK))
        {
            mult = 15 + 2*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_HURT_ROCK);
        }
        else if (!r_ptr || (r_ptr->flags3 & RF3_NONLIVING))
        {
            mult = 15 + 2*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_NONLIVING);
        }
        break;
    case SP_EVILNESS:
        if (!r_ptr || (r_ptr->flags3 & RF3_GOOD))
        {
            mult = 15 + 4*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_GOOD);
			if (have_flag(flgs, OF_KILL_GOOD))
				mult += 10;
            if (have_flag(flgs, OF_SLAY_GOOD))
                mult += 5;
        }
        break;
    case SP_HOLYNESS:
        if (!r_ptr || (r_ptr->flags3 & RF3_EVIL))
        {
            mult = 15 + 4*p_ptr->concent;
            if (m_ptr) mon_lore_3(m_ptr, RF3_EVIL);
            if (r_ptr && (r_ptr->flags3 & RF3_HURT_LITE))
            {
                mult += 3*p_ptr->concent;
                mon_lore_3(m_ptr, RF3_HURT_LITE);
            }
            if (have_flag(flgs, OF_KILL_EVIL))
                mult += 10;
            if (have_flag(flgs, OF_SLAY_EVIL))
                mult += 5;
        }
        break;
    case SP_FINAL:
        mult = 50;
        break;
    }

    return mult;
}
/************************************************************************
 * Spells
 ***********************************************************************/
static bool _do_shot(int which)
{
    bool result = FALSE;
    if (!equip_find_obj(TV_BOW, SV_ANY))
    {
        msg_print("你需要装备一把弓！");
        return FALSE;
    }
    shoot_hack = which;
    command_cmd = 'f'; /* hack for @fa inscriptions */
    result = do_cmd_fire();
    shoot_hack = 0;
    return result;
}
static char *_mult_info(int mult)
{
    return format("%d.%dx", mult/10, mult%10);
}
static void _default(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_INFO:
        var_set_string(res, _mult_info(sniper_multiplier(which, NULL, NULL)));
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_shot(which));
        break;
    case SPELL_ON_BROWSE: {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        display_shooter_mode = which;
        do_cmd_knowledge_shooter();
        display_shooter_mode = 0;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break; }
    case SPELL_ENERGY:
        var_set_int(res, energy_use); /* roundabout ... but do_cmd_fire already set this */
        break;
    default:
        default_spell(cmd, res);
    }
}
static void _shining_arrow(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪耀之箭");
        break;
    case SPELL_DESC:
        var_set_string(res,
            "Shoot a glowing arrow that lights up the dungeon. This "
            "shot also does increased damage from light against "
            "enemies that are hurt by bright light.");
        break;
    default:
        _default(SP_LITE, cmd, res);
    }
}
static void _shoot_and_away(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "射击撤退");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一个动作内向目标射击然后闪现离开。");
        break;
    default:
        _default(SP_AWAY, cmd, res);
    }
}
static void _disarming_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "除陷射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一只能粉碎陷阱的箭矢。");
        break;
    default:
        _default(SP_KILL_TRAP, cmd, res);
    }
}
static void _burning_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "燃烧射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "造成额外的火焰伤害。");
        break;
    default:
        _default(SP_FIRE, cmd, res);
    }
}
static void _shatter(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎岩射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一只能粉碎岩石的箭矢。");
        break;
    default:
        _default(SP_KILL_WALL, cmd, res);
    }
}
static void _freezing_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "冰冻射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "造成额外的寒冷伤害。");
        break;
    default:
        _default(SP_COLD, cmd, res);
    }
}
static void _knockback(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "击退射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "将敌方目标向后击退的强力射击。");
        break;
    default:
        _default(SP_RUSH, cmd, res);
    }
}
static void _piercing_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "穿透射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "箭矢能穿透某些怪物。");
        break;
    default:
        _default(SP_PIERCE, cmd, res);
    }
}
static void _evil_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "邪恶射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "对善良怪物造成更多伤害。");
        break;
    default:
        _default(SP_EVILNESS, cmd, res);
    }
}
static void _holy_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神圣射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "对邪恶怪物造成更多伤害。");
        break;
    default:
        _default(SP_HOLYNESS, cmd, res);
    }
}
static void _exploding_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆炸射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "箭矢在击中怪物时会爆炸。");
        break;
    default:
        _default(SP_EXPLODE, cmd, res);
    }
}
static void _double_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "双重射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "连续射出两箭。");
        break;
    default:
        _default(SP_DOUBLE, cmd, res);
    }
}
static void _thunder_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "雷霆射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "造成大量的额外闪电伤害。");
        break;
    default:
        _default(SP_ELEC, cmd, res);
    }
}
static void _needle_shot(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "针刺射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "造成瞬间死亡或仅有 1 点伤害。");
        break;
    default:
        _default(SP_NEEDLE, cmd, res);
    }
}
static void _saint_stars_arrow(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "圣星之箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "对所有怪物造成巨大伤害，但会给你带来一些副作用。");
        break;
    default:
        _default(SP_FINAL, cmd, res);
    }
}
static spell_info _get_spells[] =
{
   /*lvl  cst fail  spell */
    {  1,   0,   0, _concentrate },
    {  2,   1,   0, _shining_arrow },
    {  3,   1,   0, _shoot_and_away },
    {  5,   1,   0, _disarming_shot },
    {  8,   2,   0, _burning_shot },
    { 10,   2,   0, _shatter },
    { 13,   2,   0, _freezing_shot },
    { 18,   2,   0, _knockback },
    { 22,   3,   0, _piercing_shot },
    { 25,   4,   0, _evil_shot },
    { 26,   4,   0, _holy_shot },
    { 30,   3,   0, _exploding_shot },
    { 32,   4,   0, _double_shot },
    { 36,   3,   0, _thunder_shot },
    { 40,   3,   0, _needle_shot },
    { 48,   7,   0, _saint_stars_arrow },
    { -1,  -1,  -1, NULL}
};
static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "狙击";
        me.options = CASTER_USE_CONCENTRATION;
        me.which_stat = A_DEX;
        init = TRUE;
    }
    return &me;
}

/************************************************************************
 * Powers
 ***********************************************************************/
static power_info _get_powers[] =
{
    { A_INT, {15, 20, 80, probing_spell}},
    { -1, {-1, -1, -1, NULL} }
};

/************************************************************************
 * Class
 ***********************************************************************/
class_t *sniper_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  24,  28,   5,  32,  18,  35,  72};
    skills_t xs = { 12,  10,  10,   0,   0,   0,  12,  28};

        me.name = "狙击手";
        me.desc = "狙击手是枪法方面的专家，但他们不像弓箭手那样迅速地一箭接一箭连射。他们不仅通过专注来提高射击的精度和威力，而且还能使用可怕的箭术技巧。\n \n他们需要的是强大的弓或弩，优质的弹药，以及在任何情况下都能不退缩地坚持下去的坚韧。\n \n狙击手非常了解他们的敌人，并且可以从暗处射击他们。他们没有时间去学魔法。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 110;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG;
        
        me.birth = _birth;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        me.get_powers = _get_powers;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.character_dump = py_dump_spells;
        init = TRUE;
    }

    return &me;
}
