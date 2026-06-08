#include "angband.h"
#include <assert.h>

/**********************************************************************
 * Essences
 **********************************************************************/
#define _MAX_ESSENCE 255
#define _MIN_SPECIAL 240  /* cf TR_FLAG_MAX */
enum {
    _ESSENCE_AC = _MIN_SPECIAL,
    _ESSENCE_TO_HIT,
    _ESSENCE_TO_DAM,
};

#define _INVALID_SLOT -1
#define _RAG_SLOT 0
#define _GLOVE_SLOT 1
#define _BOOT_SLOT 2

static int _essences[3][_MAX_ESSENCE] = {{0}, {0}, {0}};

static void _load(savefile_ptr file)
{
    int ct, i, k;

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        _essences[0][i] = 0;
        _essences[1][i] = 0;
        _essences[2][i] = 0;
    }

    for (k = 0; k < 3; k++)
    {
        ct = savefile_read_s16b(file);
        for (i = 0; i < ct; i++)
        {
            int j = savefile_read_s16b(file);
            int n = savefile_read_s16b(file);

            if (0 <= j && j < _MAX_ESSENCE)
                _essences[k][j] += n;
        }
    }
//    if ((p_ptr->personality == PERS_SEXY) && (equip_obj(equip_find_first(object_is_body_armour)).sval == SV_ABUNAI_MIZUGI)) _sup_triggered = TRUE;
}

static void _save(savefile_ptr file)
{
    int k;
    for (k = 0; k < 3; k++)
    {
        int ct = 0, i;

        for (i = 0; i < _MAX_ESSENCE; i++)
        {
            if (_essences[k][i])
                ct++;
        }

        savefile_write_s16b(file, ct);

        for (i = 0; i < _MAX_ESSENCE; i++)
        {
            if (_essences[k][i])
            {
                savefile_write_s16b(file, i);
                savefile_write_s16b(file, _essences[k][i]);
            }
        }
    }
}

static bool _skip_flag(int which)
{
    switch (which)
    {
    case OF_RIDING:
    case OF_THROWING:
    case OF_HIDE_TYPE:
    case OF_SHOW_MODS:
    case OF_IGNORE_ACID:
    case OF_IGNORE_ELEC:
    case OF_IGNORE_FIRE:
    case OF_IGNORE_COLD:
    case OF_ACTIVATE:
    case OF_FULL_NAME:
    case OF_FIXED_FLAVOR:
    case OF_PLURAL:
        return TRUE;
    }
    return FALSE;
}

static bool _add_essence(int slot, int which, int amount)
{
    int n = _essences[slot][which];

    if (amount > 0)
        n += amount;

    if (n > 25000)
        n = 25000;

    if (n != _essences[slot][which])
    {
        _essences[slot][which] = n;
        return TRUE;
    }

    return FALSE;
}

static int _rag_object_slot(object_type *o_ptr)
{
    if (!o_ptr) return _INVALID_SLOT;
    if (o_ptr->tval == TV_GLOVES) return _GLOVE_SLOT;
    if (o_ptr->tval == TV_BOOTS) return _BOOT_SLOT;
    if ((o_ptr->tval == TV_SOFT_ARMOR) ||
        (o_ptr->tval == TV_HARD_ARMOR) ||
        (o_ptr->tval == TV_DRAG_ARMOR)) return _RAG_SLOT;
    return _INVALID_SLOT;
}

static bool _object_is_rag_usable(object_type *o_ptr)
{
    return (_rag_object_slot(o_ptr) != _INVALID_SLOT);
}

static int _slot_resist_cap(int slot)
{
    return (slot == _RAG_SLOT) ? 5 : 2;
}

static void _update_object(int slot);

static bool _absorb(object_type *o_ptr)
{
    bool result = FALSE;
    int i;
    int mult = o_ptr->number, div = 1;
    int target_slot = _rag_object_slot(o_ptr);
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    u32b flags[OF_ARRAY_SIZE];
    char o_name[MAX_NLEN];
    bool tunnettu = (((obj_is_identified(o_ptr)) || (o_ptr->feeling == FEEL_AVERAGE)) && (obj_is_identified_fully(o_ptr))); /* bizarrely, an object can be "fully identified" without being "identified", so we need to check both */
    obj_essence_flags(o_ptr, flags);

    /* Check whether the item we are absorbing completes a quest */
    quests_on_get_obj(o_ptr);

    assert(target_slot != _INVALID_SLOT);

    if (!tunnettu)
    {
        obj_identify_fully(o_ptr);
        object_desc(o_name, o_ptr, OD_COLOR_CODED);
    }
    else object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_COLOR_CODED);
    msg_format("你尝试从 %s 中吸取力量。", o_name);

    /* No absorbing the same artifact repeatedly... */
    if (o_ptr->name1) 
    {
        if (!p_ptr->noscore) assert(a_info[o_ptr->name1].generated);
        a_info[o_ptr->name1].found = TRUE;
    }
    else if ((o_ptr->art_name) && (!(o_ptr->marked & OM_ART_COUNTED))) /* Bookkeeping */
    {
        stats_rand_art_counts.found += o_ptr->number;
    }

    if (o_ptr->curse_flags & OFC_AGGRAVATE)
        div++;
    if (o_ptr->curse_flags & (OFC_TY_CURSE | OFC_HEAVY_CURSE))
        div++;

    for (i = 0; i < OF_COUNT; i++)
    {
        if (_skip_flag(i)) continue;
        if (have_flag(flags, i))
        {
            if (is_pval_flag(i))
            {
                if (_add_essence(target_slot, i, o_ptr->pval*mult/div))
                    result = TRUE;
            }
            else
            {
                _essences[target_slot][i] += mult;
                result = TRUE;
            }
        }
    }

    if (_add_essence(target_slot, _ESSENCE_AC, (o_ptr->to_a - MAX(0, k_ptr->to_a))*mult/div))
        result = TRUE;
    if (_add_essence(target_slot, _ESSENCE_TO_HIT, (o_ptr->to_h - MAX(0, k_ptr->to_h))*mult/div))
        result = TRUE;
    if (_add_essence(target_slot, _ESSENCE_TO_DAM, (o_ptr->to_d - MAX(0, k_ptr->to_d))*mult/div))
        result = TRUE;

    if (result)
    {
        p_ptr->update |= PU_BONUS;
        msg_print("你变得更强了！");
        _update_object(target_slot);
    }
    return result;
}

static bool _absorb_object(object_type *o_ptr)
{
    if (_object_is_rag_usable(o_ptr))
    {
        _absorb(o_ptr);
        return TRUE;
    }
    return FALSE;
}


static int _calc_amount(int amount, int power, int rep)
{
    int result = 0;
    int ct = 0;

    while (amount > 0)
    {
        if (amount >= power)
        {
            result++;
            ct++;
        }
        amount -= power;

        if (ct % rep == 0)
        {
            power *= 2;
            if (power > 1024)
                power = 1024;
        }
    }

    return result;
}

static int _calc_needed(int slot, int flag, int power, int rep)
{
    int result = 0;
    int ct = 0;
    /* It would be cleaner to loop until _calc_amount increased, but the
       power required increases exponentially! */
    int amount = _essences[slot][flag];
    for (;;)
    {
        if (amount >= power)
            ct++;
        result += power;
        amount -= power;
        if (amount < 0)
            break;
        if (ct % rep == 0)
        {
            power *= 2;
            if (power > 1024)
                power = 1024;
        }
    }

    if (result > 25000) return 0;

    return result;
}

/**********************************************************************
 * Attacks and Bonuses
 **********************************************************************/
typedef struct {
    int flag;
    int power;
    cptr name;
} _flag_info_t;

static _flag_info_t _slay_flag_info[] = {
    { OF_SLAY_EVIL, 2, "屠杀邪恶" },
    { OF_SLAY_GOOD, 2, "屠杀善良" },
    { OF_SLAY_UNDEAD, 2, "屠杀亡灵" },
    { OF_SLAY_DEMON, 2, "屠杀恶魔" },
    { OF_SLAY_DRAGON, 2, "屠杀龙类" },
    { OF_SLAY_HUMAN, 2, "屠杀人类" },
    { OF_SLAY_ANIMAL, 2, "屠杀动物" },
    { OF_SLAY_ORC, 2, "屠杀兽人" },
    { OF_SLAY_TROLL, 2, "屠杀巨魔" },
    { OF_SLAY_GIANT, 2, "屠杀巨人" },
    { OF_SLAY_LIVING, 2, "屠杀活物" },

    { OF_BRAND_ACID, 2, "强酸附魔" },
    { OF_BRAND_ELEC, 2, "闪电附魔" },
    { OF_BRAND_FIRE, 2, "火焰附魔" },
    { OF_BRAND_COLD, 2, "寒冰附魔" },
    { OF_BRAND_VAMP, 2, "吸血附魔" },

    { -1, 0, NULL}
};

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    info_ptr->to_dd = 3;
    info_ptr->to_ds = 3 + MIN(8, p_ptr->lev / 5);
    info_ptr->to_d += MIN(6, p_ptr->lev / 5);
}

typedef struct {
    int req;
    int reps;
    int max;
} _essence_req_t;

void _aseta_vaatimus(_essence_req_t *req_ptr, int req, int reps, int max)
{
    req_ptr->req = req;
    req_ptr->reps = reps;
    req_ptr->max = max;
}

_essence_req_t _vaatimus(int lippu)
{
    _essence_req_t pyydetaan = {0};
    switch (lippu)
    {
        case OF_LITE:
        case OF_RES_TIME:
            _aseta_vaatimus(&pyydetaan, 1, 1, 0);
            break;

        case OF_MAGIC_MASTERY:
        case OF_DEVICE_POWER:
        case OF_SEARCH:
        case OF_INFRA:
        case OF_TUNNEL:
        case OF_BLOWS:
            _aseta_vaatimus(&pyydetaan, 2, 1, 0);
            break;

        case OF_RES_DISEN:
        case OF_RES_POIS:
        case OF_RES_LITE:
        case OF_RES_DARK:
        case OF_RES_NETHER:
        case OF_RES_NEXUS:
        case OF_RES_CONF:
        case OF_RES_SOUND:
        case OF_RES_SHARDS:
        case OF_RES_CHAOS:
        case OF_RES_BLIND:
            _aseta_vaatimus(&pyydetaan, 2, 2, 0);
            break;

        case OF_SPEED:
            _aseta_vaatimus(&pyydetaan, 2, 3, 18);
            break;

        case OF_STEALTH:
        case OF_MAGIC_RESISTANCE:
        case OF_HOLD_LIFE:
        case OF_RES_FEAR:
            _aseta_vaatimus(&pyydetaan, 3, 1, 0);
            break;

        case OF_RES_ACID:
        case OF_RES_FIRE:
        case OF_RES_COLD:
        case OF_RES_ELEC:
            _aseta_vaatimus(&pyydetaan, 2, 1, 0);
            break;

        case OF_REGEN:
        case OF_AURA_COLD:
        case OF_AURA_FIRE:
        case OF_AURA_ELEC:
        case OF_AURA_SHARDS:
        case OF_AURA_REVENGE:
            _aseta_vaatimus(&pyydetaan, 4, 1, 0);
            break;

        case OF_TELEPATHY:
        case OF_ESP_EVIL:
        case OF_ESP_GOOD:
        case OF_ESP_ANIMAL:
        case OF_ESP_TROLL:
        case OF_ESP_ORC:
        case OF_ESP_HUMAN:
        case OF_ESP_DEMON:
        case OF_ESP_DRAGON:
        case OF_ESP_GIANT:
        case OF_ESP_LIVING:
        case OF_ESP_NONLIVING:
        case OF_ESP_UNIQUE:
        case OF_ESP_UNDEAD:
        case OF_SLAY_EVIL:
        case OF_SLAY_GOOD:
        case OF_SLAY_UNDEAD:
        case OF_SLAY_DEMON:
        case OF_SLAY_DRAGON:
        case OF_SLAY_HUMAN:
        case OF_SLAY_ANIMAL:
        case OF_SLAY_ORC:
        case OF_SLAY_TROLL:
        case OF_SLAY_GIANT:
        case OF_SLAY_LIVING:
        case OF_BRAND_ACID:
        case OF_BRAND_ELEC:
        case OF_BRAND_FIRE:
        case OF_BRAND_COLD:
        case OF_BRAND_VAMP:
        case OF_FREE_ACT:
            _aseta_vaatimus(&pyydetaan, 2, 1, 1);
            break;

        case OF_REFLECT:
        case OF_NO_MAGIC:
        case OF_IM_ACID:
        case OF_IM_FIRE:
        case OF_IM_COLD:
        case OF_IM_ELEC:
        case OF_LEVITATION:
            _aseta_vaatimus(&pyydetaan, 3, 1, 1);
            break;

        case OF_LORE2:
        case OF_SLOW_DIGEST:
            _aseta_vaatimus(&pyydetaan, 4, 1, 1);
            break;

        case OF_SUST_STR:
        case OF_SUST_INT:
        case OF_SUST_WIS:
        case OF_SUST_DEX:
        case OF_SUST_CON:
        case OF_SUST_CHR:
            _aseta_vaatimus(&pyydetaan, 5, 1, 1);
            break;

        case OF_SEE_INVIS:
            _aseta_vaatimus(&pyydetaan, 3, 1, 2);
            break;

        case OF_STR:
        case OF_INT:
        case OF_WIS:
        case OF_DEX:
        case OF_CON:
        case OF_CHR:
            _aseta_vaatimus(&pyydetaan, 3, 1, 6);
            break;

        case OF_LIFE:
            _aseta_vaatimus(&pyydetaan, 6, 1, 6);
            break;

        case _ESSENCE_AC:
            _aseta_vaatimus(&pyydetaan, 2, 5, 74);
            break;

        case _ESSENCE_TO_HIT:
        case _ESSENCE_TO_DAM:
            _aseta_vaatimus(&pyydetaan, 1, 4, 36);
            break;

        default: break;
    }
    return pyydetaan;
}

int rag_effect_pval(object_type *o_ptr, int slot, int i, bool is_resist)
{
    _essence_req_t pyydetty;
    if (slot == _INVALID_SLOT) slot = _rag_object_slot(o_ptr);
    if (slot == _INVALID_SLOT) return 0;
    if (i < 1) return 0;
    if ((i >= OF_COUNT) && (i != _ESSENCE_AC) && (i != _ESSENCE_TO_HIT) &&
        (i != _ESSENCE_TO_DAM)) return 0;
    if (!_essences[slot][i]) return 0;
    pyydetty = _vaatimus(i);
    if (!pyydetty.req) return 0; /* Not a rag flag */
//    if ((i == _ESSENCE_TO_HIT) || (i == _ESSENCE_TO_DAM)) return pienempi(pyydetty.max, _calc_amount(_essences[slot][i], pyydetty.req, pyydetty.reps) / 2);
    if (is_resist) return pienempi(_slot_resist_cap(slot), _calc_amount(_essences[slot][i], pyydetty.req, pyydetty.reps));
    if (pyydetty.max == 1) return ((_essences[slot][i] >= pyydetty.req) ? 1 : 0);
    if (pyydetty.max > 1) return pienempi(pyydetty.max, _calc_amount(_essences[slot][i], pyydetty.req, pyydetty.reps));
    return _calc_amount(_essences[slot][i], pyydetty.req, pyydetty.reps);
}

static int _manage_my_pval(object_type *o_ptr, int slot, bool get_flags, int flag, int kerroin)
{
    int my_pval = rag_effect_pval(o_ptr, slot, flag, FALSE);
    if (my_pval > 0)
    {
        if (get_flags) add_flag(o_ptr->flags, flag);
        else return kerroin * my_pval;
    }
    return 0;
}

void armor_calc_obj_bonuses(object_type *o_ptr, bool get_flags)
{
    int slot = _rag_object_slot(o_ptr);
    int i;
    if (slot == _INVALID_SLOT) return;

    for (i = OF_STR; i <= OF_CHR; i++)
    {
        p_ptr->stat_add[A_STR + i - OF_STR] += _manage_my_pval(o_ptr, slot, get_flags, i, 1);
    }

    p_ptr->skills.dev += _manage_my_pval(o_ptr, slot, get_flags, OF_MAGIC_MASTERY, 8);
    p_ptr->skills.stl += _manage_my_pval(o_ptr, slot, get_flags, OF_STEALTH, 1);
    p_ptr->skills.srh += _manage_my_pval(o_ptr, slot, get_flags, OF_SEARCH, 5);
    if (!get_flags) p_ptr->skills.fos += _manage_my_pval(o_ptr, slot, get_flags, OF_SEARCH, 5);
    p_ptr->device_power += _manage_my_pval(o_ptr, slot, get_flags, OF_DEVICE_POWER, 1);
    p_ptr->see_infra += _manage_my_pval(o_ptr, slot, get_flags, OF_INFRA, 1);
    p_ptr->skill_dig += _manage_my_pval(o_ptr, slot, get_flags, OF_TUNNEL, 20);
    p_ptr->pspeed += _manage_my_pval(o_ptr, slot, get_flags, OF_SPEED, 1);
    p_ptr->weapon_info[0].xtra_blow += _manage_my_pval(o_ptr, slot, get_flags, OF_BLOWS, 10);
    p_ptr->life += _manage_my_pval(o_ptr, slot, get_flags, OF_LIFE, 3);
    p_ptr->magic_resistance += _manage_my_pval(o_ptr, slot, get_flags, OF_MAGIC_RESISTANCE, 5);
    p_ptr->regen += _manage_my_pval(o_ptr, slot, get_flags, OF_REGEN, 50);
    p_ptr->see_inv += _manage_my_pval(o_ptr, slot, get_flags, OF_SEE_INVIS, 1);
    p_ptr->free_act += _manage_my_pval(o_ptr, slot, get_flags, OF_FREE_ACT, 1);
    p_ptr->hold_life += _manage_my_pval(o_ptr, slot, get_flags, OF_HOLD_LIFE, 1);
    p_ptr->sh_fire += _manage_my_pval(o_ptr, slot, get_flags, OF_AURA_FIRE, 1);
    p_ptr->sh_elec += _manage_my_pval(o_ptr, slot, get_flags, OF_AURA_ELEC, 1);
    p_ptr->sh_cold += _manage_my_pval(o_ptr, slot, get_flags, OF_AURA_COLD, 1);
    p_ptr->sh_shards += _manage_my_pval(o_ptr, slot, get_flags, OF_AURA_SHARDS, 1);
    if (get_flags)
    {
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SLOW_DIGEST, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_TELEPATHY, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_ANIMAL, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_UNDEAD, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_DEMON, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_ORC, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_TROLL, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_GIANT, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_DRAGON, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_HUMAN, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_EVIL, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_GOOD, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_NONLIVING, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_LIVING, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_ESP_UNIQUE, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_REFLECT, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_AURA_REVENGE, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_NO_MAGIC, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_STR, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_INT, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_WIS, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_DEX, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_CON, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_SUST_CHR, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_LEVITATION, 0);
        (void)_manage_my_pval(o_ptr, slot, TRUE, OF_LORE2, 0);
    }
    else
    {
        if (have_flag(o_ptr->flags, OF_SLOW_DIGEST)) p_ptr->slow_digest = TRUE;
        if (have_flag(o_ptr->flags, OF_TELEPATHY))   p_ptr->telepathy = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_ANIMAL))  p_ptr->esp_animal = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_UNDEAD))  p_ptr->esp_undead = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_DEMON))   p_ptr->esp_demon = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_ORC))     p_ptr->esp_orc = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_TROLL))   p_ptr->esp_troll = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_GIANT))   p_ptr->esp_giant = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_DRAGON))  p_ptr->esp_dragon = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_HUMAN))   p_ptr->esp_human = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_EVIL))    p_ptr->esp_evil = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_GOOD))    p_ptr->esp_good = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_NONLIVING)) p_ptr->esp_nonliving = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_LIVING)) p_ptr->esp_living = TRUE;
        if (have_flag(o_ptr->flags, OF_ESP_UNIQUE))  p_ptr->esp_unique = TRUE;
        if (have_flag(o_ptr->flags, OF_REFLECT))  p_ptr->reflect = TRUE;
        if (have_flag(o_ptr->flags, OF_AURA_REVENGE))  p_ptr->sh_retaliation = TRUE;
        if (have_flag(o_ptr->flags, OF_NO_MAGIC)) p_ptr->anti_magic = TRUE;

        if (have_flag(o_ptr->flags, OF_SUST_STR)) p_ptr->sustain_str = TRUE;
        if (have_flag(o_ptr->flags, OF_SUST_INT)) p_ptr->sustain_int = TRUE;
        if (have_flag(o_ptr->flags, OF_SUST_WIS)) p_ptr->sustain_wis = TRUE;
        if (have_flag(o_ptr->flags, OF_SUST_DEX)) p_ptr->sustain_dex = TRUE;
        if (have_flag(o_ptr->flags, OF_SUST_CON)) p_ptr->sustain_con = TRUE;
        if (have_flag(o_ptr->flags, OF_SUST_CHR)) p_ptr->sustain_chr = TRUE;        
        if (have_flag(o_ptr->flags, OF_LEVITATION)) p_ptr->levitation = TRUE;
        if (have_flag(o_ptr->flags, OF_LORE2)) p_ptr->auto_id = TRUE;
    }

    if ((!get_flags) && (slot == _GLOVE_SLOT))
    {
        for (i = 0; ; i++)
        {
            int j = _slay_flag_info[i].flag;
            if (j < 0) break;
            if (have_flag(o_ptr->flags, j))
            {
                add_flag(p_ptr->weapon_info[0].flags, j);
                add_flag(p_ptr->weapon_info[0].known_flags, j);
            }
        }
    }

    if (get_flags) res_calc_rag_flags(o_ptr);
    else res_calc_rag_bonuses(o_ptr);

    if (get_flags) return;

    /* Modify the base armor class */
    p_ptr->ac += o_ptr->ac;
    p_ptr->dis_ac += o_ptr->ac;

    /* Apply the bonuses to armor class */
    p_ptr->to_a += o_ptr->to_a;
    p_ptr->dis_to_a += o_ptr->to_a;

    if (slot != _GLOVE_SLOT)
    {
        p_ptr->to_h_m += o_ptr->to_h;
        p_ptr->to_d_m += o_ptr->to_d;

        p_ptr->weapon_info[0].to_h += o_ptr->to_h;
        p_ptr->weapon_info[0].to_d += o_ptr->to_d;
        p_ptr->weapon_info[0].dis_to_h += o_ptr->to_h;
        p_ptr->weapon_info[0].dis_to_d += o_ptr->to_d;
    }
}

static void _update_object(int slot)
{
    int i, e_slot = 0;
    object_type *o_ptr;
    switch (slot)
    {
        case _RAG_SLOT: e_slot = equip_find_first(object_is_body_armour); break;
        case _GLOVE_SLOT: e_slot = equip_find_first(object_is_gloves); break;
        case _BOOT_SLOT: e_slot = equip_find_first(object_is_boots); break;
        default: break;
    }
    if (!e_slot) return;
    o_ptr = equip_obj(e_slot);

    for (i = 0; i < OF_ARRAY_SIZE; i++)
        o_ptr->flags[i] = 0;

    o_ptr->pval = 0; /* paranoia */

    add_flag(o_ptr->flags, OF_NO_REMOVE);
    add_flag(o_ptr->flags, OF_NO_ENCHANT);
    add_flag(o_ptr->flags, OF_IGNORE_ACID);
    if (object_is_(o_ptr, TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI))
        add_flag(o_ptr->flags, OF_AGGRAVATE);

    /* Calculate new essences */
    o_ptr->to_a = rag_effect_pval(o_ptr, slot, _ESSENCE_AC, FALSE);
    if (object_is_(o_ptr, TV_SOFT_ARMOR, SV_FILTHY_RAG)) o_ptr->to_a--; /* -1 base to-AC */
    o_ptr->to_h = rag_effect_pval(o_ptr, slot, _ESSENCE_TO_HIT, FALSE);
    o_ptr->to_d = rag_effect_pval(o_ptr, slot, _ESSENCE_TO_DAM, FALSE);

    if (slot == _GLOVE_SLOT)
    {
        for (i = 0; ; i++)
        {
            int j = _slay_flag_info[i].flag;
            if (j < 0) break;
            if (_essences[slot][j] >= _slay_flag_info[i].power)
            {
                add_flag(o_ptr->flags, j);
                add_flag(o_ptr->known_flags, j);
            }
        }
    }

    /* This game doesn't support multi-pval, but these items are effectively
     * multi-pval anyway. Here we just want the flags though */
    armor_calc_obj_bonuses(o_ptr, TRUE);
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        o_ptr->known_flags[i] = o_ptr->flags[i];
}

static void _calc_bonuses(void) 
{
    p_ptr->no_cut = TRUE;
    p_ptr->to_a += p_ptr->lev + 5;
    p_ptr->dis_to_a += p_ptr->lev + 5;
    if (p_ptr->lev > 30) p_ptr->pspeed += (p_ptr->lev - 28) / 3;
    res_add_amt(RES_BLIND, 2);
    res_add_amt(RES_POIS, 2);

    /* Otherwise !resistance is weaker on rags due to the different resist calcs */
    if (IS_OPPOSE_ACID()) res_add(RES_ACID);
    if (IS_OPPOSE_ELEC()) res_add(RES_ELEC);
    if (IS_OPPOSE_FIRE()) res_add(RES_FIRE);
    if (IS_OPPOSE_COLD()) res_add(RES_COLD);
    if (IS_OPPOSE_POIS()) res_add(RES_POIS);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_LITE);
    add_flag(flgs, OF_RES_BLIND);
    add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev > 30) add_flag(flgs, OF_SPEED);
}

/**********************************************************************
 * Powers
 **********************************************************************/
static void _absorb_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸收力量");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁一副手套、一双靴子或一件身体护甲，吸收其力量的精华。");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};

        var_set_bool(res, FALSE);
        prompt.prompt = "吸收哪件物品的力量？";
        prompt.error = "你没有什么可以吸收的。";
        prompt.filter = _object_is_rag_usable;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;

        _absorb(prompt.obj);

        prompt.obj->number = 0;
        obj_release(prompt.obj, 0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测护甲");
        break;
    case SPELL_DESC:
        var_set_string(res, "定位附近的手套、靴子和身体护甲。");
        break;
    case SPELL_CAST:
    {
        int rng = DETECT_RAD_DEFAULT;
        int i, y, x;
        bool detect = FALSE;

        if (d_info[dungeon_type].flags1 & DF1_DARKNESS) rng /= 3;

        for (i = 1; i < o_max; i++)
        {
            object_type *o_ptr = &o_list[i];

            if (!o_ptr->k_idx) continue;
            if (o_ptr->held_m_idx) continue;
            y = o_ptr->loc.y;
            x = o_ptr->loc.x;
            if (distance(py, px, y, x) > rng) continue;
            if (!_object_is_rag_usable(o_ptr)) continue;
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;
            lite_spot(y, x);
            detect = TRUE;
        }
        if (detect_monsters_string(DETECT_RAD_DEFAULT, "["))
            detect = TRUE;

        if (detect)
            msg_print("你感应到了你的同类。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _judge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鉴定护甲");
        break;
    case SPELL_DESC:
        var_set_string(res, "鉴定手套、靴子或身体护甲。");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 35)
            var_set_bool(res, identify_fully(_object_is_rag_usable));
        else
            var_set_bool(res, ident_spell(_object_is_rag_usable));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _get_powers[] =
{
    { A_STR, {  1,  0,  0, _absorb_spell } },
    { A_STR, {  5,  1, 30, _detect_spell } },
    { A_STR, { 10, 10, 50, _judge_spell } },
    {    -1, { -1, -1, -1, NULL}}
};

/**********************************************************************
 * Birth and Evolution
 **********************************************************************/
static void _birth(void) 
{ 
    object_type forge;
    int i;

    for (i = 0; i < _MAX_ESSENCE; i++)
    {
        _essences[0][i] = 0;
        _essences[1][i] = 0;
        _essences[2][i] = 0;
    }

    p_ptr->current_r_idx = MON_FILTHY_RAG;
    equip_on_change_race();

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_FILTHY_RAG));
    add_flag(forge.flags, OF_NO_REMOVE);
    add_flag(forge.flags, OF_NO_ENCHANT);
    add_flag(forge.flags, OF_IGNORE_ACID);
    py_birth_obj(&forge);
    object_prep(&forge, lookup_kind(TV_GLOVES, SV_SET_OF_LEATHER_GLOVES));
    add_flag(forge.flags, OF_NO_REMOVE);
    add_flag(forge.flags, OF_NO_ENCHANT);
    add_flag(forge.flags, OF_IGNORE_ACID);
    py_birth_obj(&forge);
    object_prep(&forge, lookup_kind(TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS));
    add_flag(forge.flags, OF_NO_REMOVE);
    add_flag(forge.flags, OF_NO_ENCHANT);
    add_flag(forge.flags, OF_IGNORE_ACID);
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
}

static void _gain_level(int new_level) 
{
    if (p_ptr->personality != PERS_SEXY) return;
    if (p_ptr->current_r_idx == MON_SEXY_SWIMSUIT) return;
    if (new_level < 40) return;
    else
    {
        object_type *o_ptr = equip_obj(equip_find_first(object_is_body_armour));
        msg_print("你进化成了性感泳装(Sexy Swimsuit)。");
        p_ptr->current_r_idx = MON_SEXY_SWIMSUIT;
        o_ptr->k_idx = lookup_kind(TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI);
        o_ptr->sval = SV_ABUNAI_MIZUGI;
        o_ptr->weight = 2;
        o_ptr->ac = 0;
        o_ptr->to_a += 1;
        add_flag(o_ptr->flags, OF_AGGRAVATE);
    }
    p_ptr->redraw |= (PR_MAP | PR_BASIC | PR_EQUIPPY);
    p_ptr->update |= (PU_BONUS);
    p_ptr->window |= (PW_INVEN | PW_EQUIP);
}

/**********************************************************************
 * Character Dump
 **********************************************************************/
static void _dump_flag(doc_ptr doc, int which, cptr name, bool is_resist)
{
    if (_essences[0][which] || _essences[1][which] || _essences[2][which])
    {
        int i, threshold[3], bonus = 0;
        bool is_ability;
        char buf[4][15];
        _essence_req_t pyydetyt = _vaatimus(which);
        if (!pyydetyt.req) return;
        is_ability = (pyydetyt.max == 1);
        for (i = 0; i < 3; i++)
        {
            int lisays = 0;
            int huippu = 999;
            threshold[i] = _calc_needed(i, which, pyydetyt.req, pyydetyt.reps);
            lisays = rag_effect_pval(NULL, i, which, is_resist);
            if (pyydetyt.max) huippu = pyydetyt.max;
            if (is_resist) huippu = _slot_resist_cap(i);
            lisays = MIN(lisays, huippu);
            if (!is_ability) bonus += lisays;
            if ((_essences[i][which] >= threshold[i]) || (lisays >= huippu))
            {
                if (is_ability) sprintf(buf[i], "     Y");
                else sprintf(buf[i], "最大");
            }
            else sprintf(buf[i], "%5d/%-5d", _essences[i][which], threshold[i]);
        }
        doc_printf(doc, "   %-21.21s %-11.11s %-11.11s %-11.11s",
            name, buf[0], buf[1], buf[2]);
        if (bonus > 0)
        {
            if ((is_resist) || (which == OF_HOLD_LIFE) || (which == OF_FREE_ACT) || (which == OF_SEE_INVIS))
                doc_printf(doc, "%dx\n", bonus);
            else doc_printf(doc, "   %+d\n", bonus);
        }
        else doc_printf(doc, "\n");
    }
}

static void _character_dump(doc_ptr doc)
{
    int i;
    doc_printf(doc, "<topic:Essences>=================================== 精 华 (<color:keypress>E</color>) ==================================\n\n");
    doc_printf(doc, "<color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "属性");
    for (i = 0; i < MAX_STATS; i++) /* Assume in order */
        _dump_flag(doc, OF_STR + i, stat_name_true[A_STR + i], FALSE);

    doc_printf(doc, "\n <color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "技能");
    _dump_flag(doc, _ESSENCE_TO_HIT, "命中", FALSE);
    _dump_flag(doc, _ESSENCE_TO_DAM, "伤害", FALSE);
    _dump_flag(doc, _ESSENCE_AC, "护甲", FALSE);
    _dump_flag(doc, OF_STEALTH, "潜行", FALSE);
    _dump_flag(doc, OF_SPEED, "速度", FALSE);
    _dump_flag(doc, OF_BLOWS, "攻击次数", FALSE);
    _dump_flag(doc, OF_LIFE, "生命", FALSE);
    _dump_flag(doc, OF_SEARCH, "搜索", FALSE);
    _dump_flag(doc, OF_INFRA, "红外视力", FALSE);
    _dump_flag(doc, OF_TUNNEL, "挖掘", FALSE);
    _dump_flag(doc, OF_MAGIC_MASTERY, "魔法精通", FALSE);
    _dump_flag(doc, OF_DEVICE_POWER, "装置威力", FALSE);
    _dump_flag(doc, OF_LITE, "光照", FALSE);
 
    doc_printf(doc, "\n <color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "斩杀");
    for (i = 0; ; i++)
    {
        int j = _slay_flag_info[i].flag;
        if (j < 0) break;
        _dump_flag(doc, j, _slay_flag_info[i].name, FALSE);
    }

    doc_printf(doc, "\n <color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "抗性");
    for (i = 0; i < RES_MAX; i++)
        _dump_flag(doc, res_get_object_flag(i), format("%^s", res_name(i)), TRUE);

    _dump_flag(doc, OF_IM_ACID, "强酸免疫", FALSE);
    _dump_flag(doc, OF_IM_ELEC, "闪电免疫", FALSE);
    _dump_flag(doc, OF_IM_FIRE, "火焰免疫", FALSE);
    _dump_flag(doc, OF_IM_COLD, "寒冰免疫", FALSE);

    doc_printf(doc, "\n <color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "能力");
    _dump_flag(doc, OF_FREE_ACT, "自由行动", FALSE);
    _dump_flag(doc, OF_SEE_INVIS, "识破隐形", FALSE);
    _dump_flag(doc, OF_SLOW_DIGEST, "缓慢消化", FALSE);
    _dump_flag(doc, OF_REGEN, "再生", FALSE);
    _dump_flag(doc, OF_LEVITATION, "漂浮", FALSE);
    _dump_flag(doc, OF_NO_MAGIC, "反魔法", FALSE);
    _dump_flag(doc, OF_LORE2, "自动鉴定", FALSE);
    _dump_flag(doc, OF_REFLECT, "反射", FALSE);
    _dump_flag(doc, OF_HOLD_LIFE, "维持生命", FALSE);
    _dump_flag(doc, OF_MAGIC_RESISTANCE, "抵抗魔法", FALSE);
    _dump_flag(doc, OF_AURA_FIRE, "火焰光环", FALSE);
    _dump_flag(doc, OF_AURA_ELEC, "闪电光环", FALSE);
    _dump_flag(doc, OF_AURA_COLD, "寒冰光环", FALSE);
    _dump_flag(doc, OF_AURA_SHARDS, "碎片光环", FALSE);
    _dump_flag(doc, OF_AURA_REVENGE, "复仇", FALSE);
    for (i = 0; i < 6; i++) /* Assume in order */
        _dump_flag(doc, OF_SUST_STR + i, format("维持%s", stat_name_true[A_STR + i]), FALSE);

    doc_printf(doc, "\n <color:G>%-22.22s 破布 手套 靴子 加成</color>\n", "侦测/心电感应");
    _dump_flag(doc, OF_TELEPATHY, "心灵感应", FALSE);
    _dump_flag(doc, OF_ESP_ANIMAL, "侦测动物", FALSE);
    _dump_flag(doc, OF_ESP_UNDEAD, "侦测亡灵", FALSE);
    _dump_flag(doc, OF_ESP_DEMON, "侦测恶魔", FALSE);
    _dump_flag(doc, OF_ESP_ORC, "侦测兽人", FALSE);
    _dump_flag(doc, OF_ESP_TROLL, "侦测巨魔", FALSE);
    _dump_flag(doc, OF_ESP_GIANT, "侦测巨人", FALSE);
    _dump_flag(doc, OF_ESP_DRAGON, "侦测龙类", FALSE);
    _dump_flag(doc, OF_ESP_HUMAN, "侦测人类", FALSE);
    _dump_flag(doc, OF_ESP_EVIL, "侦测邪恶", FALSE);
    _dump_flag(doc, OF_ESP_GOOD, "侦测善良", FALSE);
    _dump_flag(doc, OF_ESP_NONLIVING, "侦测非活物", FALSE);
    _dump_flag(doc, OF_ESP_LIVING, "侦测活物", FALSE);
    _dump_flag(doc, OF_ESP_UNIQUE, "侦测独特怪物", FALSE);

    doc_newline(doc);
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_armor_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  21,  40,   4,  14,   5,  42,  20};
    skills_t xs = { 12,   8,  12,   0,   0,   0,  18,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "破布";
        me.desc = "一堆又脏又旧、饱经风霜的衣服，在魔法的作用下（或者仅仅是因为积聚了足够的微生物）获得了生命和意识，并且充满了报复那些干净、光亮、时髦的板甲的强烈欲望。某种东西，很可能是一层厚厚的污垢，将一双旧靴子和两只普通的皮手套永久地粘附在最初的破布上，使它能够四处走动并痛打敌人。如果你觉得这东西现在看起来很丑，那就等它找到其他的防护护甲，并有时间将其拆解吸收掉其中的魔法属性之后再说吧……";

        me.infra = 2;
        me.exp = 110;
        me.base_hp = 20;
        me.shop_adjust = 120;

        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.character_dump = _character_dump;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.get_powers = _get_powers;
        me.birth = _birth;

        me.load_player = _load;
        me.save_player = _save;
        me.destroy_object = _absorb_object;

        me.flags = RACE_IS_MONSTER | RACE_EATS_DEVICES;
        me.pseudo_class_idx = CLASS_RUNE_KNIGHT;

        init = TRUE;
    }

    me.subname = (p_ptr->current_r_idx == MON_SEXY_SWIMSUIT) ? "性感泳装" : NULL;
    me.stats[A_STR] = -1;
    me.stats[A_INT] = -1;
    me.stats[A_WIS] = -1;
    me.stats[A_DEX] = -1;
    me.stats[A_CON] = -2;
    me.stats[A_CHR] = -4;
    me.life = 95 + p_ptr->lev / 10;

    me.equip_template = mon_get_equip_template();

    if (birth_hack || spoiler_hack)
    {
        me.subname = NULL;
        me.subdesc = NULL;
    }
    return &me;
}

int armor_calc_torch(void)
{
    int sade = 1;
    sade += rag_effect_pval(NULL, _RAG_SLOT, OF_LITE, FALSE);
    sade += rag_effect_pval(NULL, _GLOVE_SLOT, OF_LITE, FALSE);
    sade += rag_effect_pval(NULL, _BOOT_SLOT, OF_LITE, FALSE);
    return MIN(4, 1 + sade);
}
