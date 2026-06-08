/****************************************************************
 * The Rune Knight
 ****************************************************************/

#include "angband.h"

static byte notional_stat_ind[PY_MAX_LEVEL] =
{ 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
  16, 16, 16, 16, 16, 17, 17, 17, 18, 18,
  19, 19, 19, 20, 20, 20, 21, 22, 23, 24,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 33,
  34, 34, 35, 35, 36, 36, 37, 37, 37, 37};

int _notional_mana(void) /* Don't reward low INT by making full mana gain easier */
{
    return spell_cap(calc_mana_aux(notional_stat_ind[p_ptr->lev - 1], A_INT, p_ptr->lev));
}

/****************************************************************
 * Public Helpers
 ****************************************************************/
int rune_knight_absorption(int m_idx, int type, int dam)
{
    int drain = 0;
    s32b hat;
    s32b pow;
    int msp;
    int wdam;
    s32b csp;
    static s32b last_py_turn = 0;
    static s16b py_turn_dam = 0;
    bool is_guardian = FALSE;

    if (p_ptr->pclass != CLASS_RUNE_KNIGHT) return dam;
    if (!p_ptr->magic_resistance) return dam;
    if (type == GF_ARROW || type == GF_ROCK) return dam;
    if (!p_ptr->msp) return dam;

    msp = _notional_mana();
//    msg_format("Notional mana: %d", msp);
    if (!msp) return dam; /* This should never happen... */
    csp = (s32b)p_ptr->csp * msp / p_ptr->msp;
//    msg_format("Notional SP: %d", csp);

    if (m_idx > 0)
    {
        monster_type *m_ptr = &m_list[m_idx];
        if ((m_ptr->r_idx) && (m_ptr->smart & (1U << SM_GUARDIAN)))
        {
            is_guardian = TRUE;
        }
    }

    /* Don't shrug off being triple-blasted by arch-viles or gravity hounds
     * even if each individual attack does low damage */
    if (player_turn != last_py_turn)
    {
        last_py_turn = player_turn;
        py_turn_dam = is_guardian ? 0 : dam;
    }
    else
    {
        if ((!is_guardian) && (dam > p_ptr->mhp / 40)) py_turn_dam += dam;
//        msg_format("Py-turn-dam: %d", py_turn_dam);
    }

    wdam = MAX(dam, py_turn_dam);

    drain = dam * (p_ptr->magic_resistance + 5) / 100;
    hat = ((wdam * 3 / 2) + 50 + MAX(80, MIN(msp, wdam * 6))) / 2;
    if ((hat < msp / 2) && (dam > p_ptr->mhp / 30))
    {
        s32b testhat = msp / 2;
        if (dam < (p_ptr->mhp / 10))
        {
            testhat -= testhat * ((p_ptr->mhp / 2) - (dam * 5)) / p_ptr->mhp;
        }
        hat = MAX(testhat, hat);
    }
    if ((is_guardian) && (hat > (msp * 3 / 4))) /* Limit scumming entrance guardians */
    {
        hat = msp * 3 / 4;
    }
//    msg_format("Hat: %d", hat);
    hat -= csp;
    if (hat > 0)
    {
        int divisor = MIN(20, MAX(5, p_ptr->lev / 2));
        pow = drain + 4; /* Slow SP gain is very tedious */
        if (csp + divisor <= msp / 2)
        {
            pow += (msp / 2 - csp) / divisor;
        }
        if (pow > hat) pow = hat;
        /* Adjust by ratio of real mana and notional mana */
        if (msp != p_ptr->msp) pow = ((pow * p_ptr->msp) + msp / 2) / msp;
        sp_player(pow);
    }

    return dam - drain;
}

void rune_calc_bonuses(object_type *o_ptr)
{
    if (o_ptr->rune == RUNE_ABSORPTION)
        p_ptr->magic_resistance += 15;
    if (o_ptr->rune == RUNE_UNDERSTANDING && object_is_helmet(o_ptr))
        p_ptr->auto_id = TRUE;
    if (o_ptr->rune == RUNE_SHADOW)
    {
        if (object_is_body_armour(o_ptr) || o_ptr->tval == TV_CLOAK)
            p_ptr->skills.stl += 5 * p_ptr->lev / 50;
    }
    if (o_ptr->rune == RUNE_HASTE)
    {
        if (o_ptr->tval == TV_BOOTS)
            p_ptr->pspeed += 3 * p_ptr->lev / 50;
    }
}

void rune_calc_stats(object_type *o_ptr, s16b stats[MAX_STATS])
{
    if (o_ptr->rune == RUNE_UNDERSTANDING)
    {
        if (o_ptr->tval == TV_LITE)
            stats[A_INT] += 1;
        else
            stats[A_INT] += 2;
    }
    if (o_ptr->rune == RUNE_HASTE)
    {
        if (o_ptr->tval == TV_GLOVES)
            stats[A_DEX] += 2;
    }
    if (o_ptr->rune == RUNE_LIFE)
    {
        if (object_is_body_armour(o_ptr))
            stats[A_CON] += 1;
    }
    if (o_ptr->rune == RUNE_MIND)
    {
        if (object_is_helmet(o_ptr))
            stats[A_INT] += 2;
    }
    if (o_ptr->rune == RUNE_MIGHT)
    {
        stats[A_STR] += 2;
        stats[A_CON] += 2;
        if (object_is_body_armour(o_ptr))
            stats[A_DEX] += 2;
    }
}

cptr rune_desc(int which)
{
    switch (which)
    {
    case RUNE_ABSORPTION:
        return "{吸收}";
    case RUNE_PROTECTION:
        return "{保护}";
    case RUNE_REGENERATION:
        return "{再生}";
    case RUNE_FIRE:
        return "{火焰}";
    case RUNE_AIR:
        return "{气流}";
    case RUNE_WATER:
        return "{水流}";
    case RUNE_LIGHT:
        return "{光明}";
    case RUNE_SHADOW:
        return "{暗影}";
    case RUNE_EARTH:
        return "{大地}";
    case RUNE_UNDERSTANDING:
        return "{理解}";
    case RUNE_ELEMENTAL_PROTECTION:
        return "{防腐}";
    case RUNE_HASTE:
        return "{加速}";
    case RUNE_SEEING:
        return "{识破}";
    case RUNE_SACRIFICE:
        return "{献祭}";
    case RUNE_LIFE:
        return "{生命}";
    case RUNE_STABILITY:
        return "{稳定}";
    case RUNE_REFLECTION:
        return "{反射}";
    case RUNE_DEATH:
        return "{死亡}";
    case RUNE_MIND:
        return "{心灵}";
    case RUNE_MIGHT:
        return "{威力}";
    case RUNE_DESTRUCTION:
        return "{毁灭}";
    case RUNE_GOOD_FORTUNE:
        return "{幸运}";
    case RUNE_IMMORTALITY:
        return "{不朽}";
    }
    return "{未知}";
}

void _add_flag(obj_ptr obj, int which)
{
    add_flag(obj->flags, which);
    add_flag(obj->known_flags, which);
}

bool rune_add(object_type *o_ptr, int which, bool prompt)    /* Birthing needs access to this ... */
{
    char o_name[MAX_NLEN];

    if (!which) return FALSE;
    object_desc(o_name, o_ptr, 0);

    if (o_ptr->rune)
    {
        msg_format("%^s 已经附有一个符文了。", o_name);
        return FALSE;
    }

    if (object_is_(o_ptr, TV_SWORD, SV_RUNESWORD))
    {
        msg_print("失败！符文之剑已经有符文了，尽管那是你无法理解的符文。");
        return FALSE;
    }

    if (o_ptr->number > 1)
    {
        msg_print("失败！你一次只能在一个单独的物品上添加一个符文。");
        return FALSE;
    }

    if (prompt)
    {
        if (!get_check(
                format("真的要将%^s加到 %s 上吗？",
                    rune_desc(which), o_name))) return FALSE;
    }

    o_ptr->rune = which;
    if (object_is_nameless(o_ptr))
        o_ptr->discount = 99;

    /* Note: Any effect that requires a pval will need to be handled
       silently in calc_bonuses(). This is because we keep the pval
       of the original object (e.g. Crown of Might (+3) <<Might>>
       Gives +5 Str/Con and +3 Dex, where the Rune adds +2 Str/Con) */
    switch (which)
    {
    case RUNE_PROTECTION:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        o_ptr->to_a += 2 + randint1(8);
        break;

    case RUNE_REGENERATION:
        _add_flag(o_ptr, OF_REGEN);
        break;

    case RUNE_FIRE:
        if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_GLOVES)
            _add_flag(o_ptr, OF_BRAND_FIRE);
        if (object_is_shield(o_ptr))
            _add_flag(o_ptr, OF_RES_FIRE);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_FIRE);
            _add_flag(o_ptr, OF_AURA_FIRE);
        }
        if (o_ptr->tval == TV_LITE || o_ptr->tval == TV_CLOAK)
            _add_flag(o_ptr, OF_AURA_FIRE);
        break;

    case RUNE_AIR:
        if (!object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_LEVITATION);
        break;

    case RUNE_WATER:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_GLOVES)
            _add_flag(o_ptr, OF_BRAND_ACID);
        else
            _add_flag(o_ptr, OF_RES_ACID);
        break;

    case RUNE_LIGHT:
        _add_flag(o_ptr, OF_RES_LITE);
        break;

    case RUNE_SHADOW:
        if (o_ptr->tval != TV_CLOAK)
            _add_flag(o_ptr, OF_RES_DARK);
        break;

    case RUNE_EARTH:
        if (object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_VORPAL);
        else if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_SHARDS);
            _add_flag(o_ptr, OF_AURA_SHARDS);
        }
        else if (object_is_shield(o_ptr))
            _add_flag(o_ptr, OF_RES_SHARDS);
        else if (o_ptr->tval == TV_CLOAK)
            _add_flag(o_ptr, OF_AURA_SHARDS);
        break;

    case RUNE_SEEING:
        _add_flag(o_ptr, OF_RES_BLIND);
        if (object_is_helmet(o_ptr))
            _add_flag(o_ptr, OF_SEE_INVIS);
        break;

    case RUNE_LIFE:
        _add_flag(o_ptr, OF_HOLD_LIFE);
        break;

    case RUNE_STABILITY:
        _add_flag(o_ptr, OF_RES_NEXUS);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_RES_CHAOS);
            _add_flag(o_ptr, OF_RES_DISEN);
        }
        break;
    
    case RUNE_REFLECTION:
        _add_flag(o_ptr, OF_REFLECT);
        break;

    case RUNE_DEATH:
        if (object_is_melee_weapon(o_ptr))
            _add_flag(o_ptr, OF_BRAND_VAMP);
        else
        {
            _add_flag(o_ptr, OF_RES_NETHER);
            if (object_is_body_armour(o_ptr))
                _add_flag(o_ptr, OF_RES_POIS);
        }
        break;

    case RUNE_MIND:
        _add_flag(o_ptr, OF_TELEPATHY);
        if (object_is_helmet(o_ptr))
            _add_flag(o_ptr, OF_SUST_INT);
        break;

    case RUNE_MIGHT:
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_SUST_STR);
            _add_flag(o_ptr, OF_SUST_DEX);
            _add_flag(o_ptr, OF_SUST_CON);
        }
        break;

    case RUNE_DESTRUCTION:
        if (object_is_melee_weapon(o_ptr))
            o_ptr->dd += 2;
        else
        {
            o_ptr->to_h += 3 + randint1(8);
            o_ptr->to_d += 3 + randint1(8);
        }
        break;

    case RUNE_IMMORTALITY:
        _add_flag(o_ptr, OF_RES_TIME);
        if (object_is_body_armour(o_ptr))
        {
            _add_flag(o_ptr, OF_SUST_STR);
            _add_flag(o_ptr, OF_SUST_INT);
            _add_flag(o_ptr, OF_SUST_WIS);
            _add_flag(o_ptr, OF_SUST_DEX);
            _add_flag(o_ptr, OF_SUST_CON);
            _add_flag(o_ptr, OF_SUST_CHR);
            _add_flag(o_ptr, OF_HOLD_LIFE);
        }
        break;

    case RUNE_ELEMENTAL_PROTECTION:
    case RUNE_GOOD_FORTUNE:
        _add_flag(o_ptr, OF_IGNORE_ACID);
        _add_flag(o_ptr, OF_IGNORE_FIRE);
        _add_flag(o_ptr, OF_IGNORE_COLD);
        _add_flag(o_ptr, OF_IGNORE_ELEC);
        break;
    }

    if (prompt)
        msg_format("%^s 闪烁着微光。", o_name);
    p_ptr->update |= PU_BONUS;

    return TRUE;
}

/****************************************************************
 * Runes of Creation
 ****************************************************************/
static object_type *_rune_object_prompt(obj_p filter)
{
    obj_prompt_t prompt = {0};

    prompt.prompt = "附魔哪件物品？";
    prompt.error = "你没有什么可以附魔的。";
    prompt.filter = filter;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_FLOOR;

    obj_prompt(&prompt);
    return prompt.obj;
}

static void _rune_default_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(25, p_ptr->msp));
        break;
    /*case SPELL_COLOR:
        var_set_int(res, TERM_L_BLUE);
        break; */
    default:
        default_spell(cmd, res);
    }
}

static bool _obj_absorption_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_absorption_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸收");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_absorption");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的近战武器、身体防具或盾牌上放置一个吸收符文。这个符文会赋予你一种特殊的魔法防御，吸收来自所有怪物法术的伤害，并在此过程中恢复你的法力。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_absorption_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_ABSORPTION, TRUE));

        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, 0); /* was MAX(1, p_ptr->msp * 3 / 10)*/
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_protection_pred(object_type *o_ptr)
{
    if (object_is_armour(o_ptr))
        return TRUE;
    return FALSE;
}
static void _obj_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "保护");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的护甲上放置一个保护符文，使其免疫强酸，同时略微增强其防护能力。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_protection_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_PROTECTION, TRUE));
        
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(5, p_ptr->msp * 5 / 10));
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_regeneration_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_regeneration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "再生");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_regeneration");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个再生符文，赋予其再生的力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_regeneration_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_REGENERATION, TRUE));
        
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(10, p_ptr->msp * 5 / 10));
        break;
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_fire_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) 
      || o_ptr->tval == TV_CLOAK
      || (o_ptr->tval == TV_GLOVES && p_ptr->lev >= 45)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个火焰符文，赋予基于火焰的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_fire_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_FIRE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_air_pred(object_type *o_ptr)
{
    if ( (object_is_melee_weapon(o_ptr) && p_ptr->lev >= 40)
      || o_ptr->tval == TV_CLOAK
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "空气");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个空气符文，赋予基于风的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_air_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_AIR, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_water_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr) 
      || o_ptr->tval == TV_CLOAK
      || (o_ptr->tval == TV_GLOVES && p_ptr->lev >= 45) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个水之符文，赋予基于酸的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_water_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_WATER, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_light_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光照");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_light");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个光之符文，赋予基于光照的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_light_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_LIGHT, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_shadow_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_shadow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "暗影");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_shadow");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个暗影符文，赋予基于黑暗的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_shadow_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SHADOW, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_earth_pred(object_type *o_ptr)
{
    if ( (object_is_melee_weapon(o_ptr) && p_ptr->lev >= 35)
      || object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大地");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个大地符文，赋予基于碎片的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_earth_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_EARTH, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_understanding_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_understanding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "理解");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_understanding");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个理解符文，赋予基于知识的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_understanding_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_UNDERSTANDING, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_elemental_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "保存");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_preservation");
        break;
    case SPELL_DESC:
        var_set_string(res, "创造一个独立的符文。只要你的物品栏里有这个符文，你物品栏里的物品就不太可能被元素攻击所摧毁。");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_RUNE, SV_RUNE));
        rune_add(&forge, RUNE_ELEMENTAL_PROTECTION, FALSE);
        drop_near(&forge, -1, py, px);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_haste_pred(object_type *o_ptr)
{
    if ( o_ptr->tval == TV_GLOVES
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_haste_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "加速");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个加速符文，赋予基于速度的特殊力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_haste_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_HASTE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_seeing_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_seeing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "洞察");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_seeing");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个洞察符文，赋予视觉相关的力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_seeing_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SEEING, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

/* XXX see comments in the spell table below ...
static void _obj_sacrifice_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Sacrifice");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_sacrifice");
        break;
    case SPELL_DESC:
        var_set_string(res, "Places a Rune of Sacrifice on an artifact. You can now destroy (with 'k' command) the artifact, and if you do so, you restore HP and SP.");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(object_is_artifact);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_SACRIFICE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}
*/

static bool _obj_life_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || o_ptr->tval == TV_LITE )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生命");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个生命符文，保护你的生命精华。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_life_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_LIFE, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_stability_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr)
      || o_ptr->tval == TV_CLOAK
      || o_ptr->tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_stability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "稳定");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_stability");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个稳定符文，保护你免受周围世界变迁的影响。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_stability_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_STABILITY, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_reflection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "反射");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_reflection");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你的盾牌上放置一个反射符文。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(object_is_shield);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_REFLECTION, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_death_pred(object_type *o_ptr)
{
    if ( object_is_melee_weapon(o_ptr)
      || object_is_shield(o_ptr)
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr) )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_death_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "死亡");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_death");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个死亡符文，赋予虚空世界的力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_death_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_DEATH, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_mind_pred(object_type *o_ptr)
{
    if ( object_is_helmet(o_ptr) 
      || o_ptr->tval == TV_LITE ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_mind_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "心灵");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_mind");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个心灵符文，赋予思想和感知的能力。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_mind_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_MIND, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_might_pred(object_type *o_ptr)
{
    if ( object_is_body_armour(o_ptr) 
      || object_is_helmet(o_ptr) ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_might_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "力量");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_might");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个力量符文，赋予力量与坚韧的能力。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_might_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_MIGHT, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_destruction_pred(object_type *o_ptr)
{
    if ( object_is_melee_weapon(o_ptr) 
      || o_ptr->tval == TV_GLOVES ) 
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毁灭");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_destruction");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个毁灭符文，赋予强大的战斗能力。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_destruction_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_DESTRUCTION, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static void _obj_good_fortune_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幸运");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_luck");
        break;
    case SPELL_DESC:
        var_set_string(res, "创造一个独立的符文。只要你的物品栏里有这个符文，在你的冒险中你就会经历更好的运气。");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_RUNE, SV_RUNE));
        rune_add(&forge, RUNE_GOOD_FORTUNE, FALSE);
        drop_near(&forge, -1, py, px);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

static bool _obj_immortality_pred(object_type *o_ptr)
{
    if ( object_is_shield(o_ptr) 
      || object_is_body_armour(o_ptr)
      || object_is_helmet(o_ptr) 
      || o_ptr->tval == TV_CLOAK )
    {
        return TRUE;
    }
    return FALSE;
}
static void _obj_immortality_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "不朽");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_obj_immortality");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你选择的物品上放置一个不朽符文，赋予掌控时间本身的力量。");
        break;
    case SPELL_CAST:
    {
        object_type *o_ptr = _rune_object_prompt(_obj_immortality_pred);
        var_set_bool(res, FALSE);

        if (o_ptr)
            var_set_bool(res, rune_add(o_ptr, RUNE_IMMORTALITY, TRUE));

        break;
    }
    default:
        _rune_default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Enhancement
 *
 * Note: Durations are long since mana is often scarce. Feel free
 * to tweak upwards as playtesting dictates.
 ****************************************************************/
#define _DURATION 100

void _self_darkness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑暗");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_darkness");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的黑暗符文，赋予增强的潜行能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_tim_dark_stalker(_DURATION + randint1(_DURATION), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_seeing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "洞察");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_seeing");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的洞察符文，赋予心灵感应能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_tim_esp(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_understanding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "理解");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_understanding");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的理解符文，赋予对自身的认知。");
        break;
    default:
        self_knowledge_spell(cmd, res);
    }
}

void _self_haste_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "加速");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_haste");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的加速符文，赋予增强的移动速度。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_fast(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "保护");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的保护符文，赋予对基础元素的临时抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:         
        set_oppose_base(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大地");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你身上放置一个临时的大地符文，硬化你的皮肤以抵御敌人的攻击。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_shield(randint1(_DURATION) + _DURATION, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生命");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你身上放置一个临时的生命符文，你可以恢复已经失去的东西。");
        break;
    default:
        restore_life_spell(cmd, res);
    }
}

void _self_daemon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恶魔");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_daemon");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你身上放置一个临时的恶魔符文，你将自己转化为更强大的形态。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION, _DURATION));
        break;
    case SPELL_CAST:
        set_mimic(_DURATION + randint1(_DURATION), MIMIC_DEMON, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

void _self_might_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "力量");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_self_might");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你身上放置一个临时的力量符文，你将获得巨人的力量。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(_DURATION/2, _DURATION/2));
        break;
    case SPELL_CAST:
        set_tim_building_up(_DURATION/2 + randint1(_DURATION/2), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Alteration
 ****************************************************************/
void _feat_spell(int feat, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST: {
        int        dir, x, y;
        cave_type *c_ptr;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        if (!in_bounds(y, x)) return;
        c_ptr = &cave[y][x];
        if ((c_ptr->info & CAVE_OBJECT) || c_ptr->o_idx)
            msg_print("<color:r>该物品抵抗了你的符文。</color>");
        else if (c_ptr->m_idx)
            msg_print("<color:r>有一个怪物挡住了你的路。</color>");
        else
            cave_set_feat(y, x, feat);

        var_set_bool(res, TRUE);
        break; }
    default:
        default_spell(cmd, res);
    }
}

void _feat_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光照");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_light");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你当前位置放置一个光之符文，你可以照亮周围的环境。");
        break;
    default:
        light_area_spell(cmd, res);
    }
}

void _feat_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文会冲刷掉其路径上的一切。");
        break;
    default:
        _feat_spell(p_ptr->lev < 35 ? feat_shallow_water : feat_deep_water, cmd, res);
    }
}

void _feat_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大地");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文会阻挡敌人的通行。");
        break;
    default:
        _feat_spell(p_ptr->lev < 45 ? feat_rubble : feat_granite, cmd, res);
    }
}

void _feat_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文会燃烧周围的地形。");
        break;
    default:
        _feat_spell(p_ptr->lev < 40 ? feat_shallow_lava : feat_deep_lava, cmd, res);
    }
}

void _feat_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "空气");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文在曾经没有空气的地方召唤出空气。");
        break;
    default:
        _feat_spell(feat_dark_pit, cmd, res);
    }
}

void _feat_stability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "稳定");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_stability");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文会创造出最普通的坚实立足点。");
        break;
    default:
        _feat_spell(feat_floor, cmd, res);
    }
}

void _feat_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生命");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_life");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个符文无论放在哪里，都会导致植物快速生长。");
        break;
    default:
        _feat_spell(feat_tree, cmd, res);
    }
}

void _feat_protection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "保护");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_protection");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你当前位置放置一个保护符文，你可以阻挡除了最强大的敌人之外的所有人的通行。");
        break;
    default:
        glyph_of_warding_spell(cmd, res);
    }
}

void _feat_destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毁灭");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_feat_destruction");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过在你当前位置放置一个极不稳定的毁灭符文，你可以摧毁附近的环境。");
        break;
    case SPELL_CAST:
        destroy_area(py, px, 12 + randint1(4), spell_power(8 * p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Runes of Battle
 ****************************************************************/
void _blow_confusion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "困惑");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_confusion");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，使你的敌人感到困惑。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_CONF));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_fire");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，烧伤你的敌人。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_FIRE));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_water_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_water");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，腐蚀你的敌人。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_ACID));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_earth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大地");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_earth");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，割伤你的敌人。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_VORPAL));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_death_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "死亡");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_death");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，吸取敌人的生命。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_VAMP));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_elec_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪电");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_lightning");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文会强化你的近战攻击，电击你的敌人。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_ELEC));
        break;
    default:
        default_spell(cmd, res);
    }
}

void _blow_air_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "空气");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_air");
        break;
    case SPELL_DESC:
        var_set_string(res, "这个临时符文允许你在毁灭旋风中攻击所有相邻的敌人。");
        break;
    default:
        massacre_spell(cmd, res);
    }
}

void _blow_mana_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力");
        break;
    case SPELL_STAT_NAME:
        var_set_string(res, "_blow_mana");
        break;
    case SPELL_DESC:
        var_set_string(res, "你最强大的战斗符文使用你的法力来强力撕裂你的敌人。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(PY_ATTACK_MANA));
        break;
    default:
        default_spell(cmd, res);
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
 #define _MAX_SPELLS_PER_GROUP  25
 #define _MAX_SPELL_GROUPS       4

typedef struct {
    cptr name;
    cptr help;
    int color;
    spell_info spells[_MAX_SPELLS_PER_GROUP];    /* There is always a sentinel at the end */
} _spell_group, *_spell_group_ptr;

static _spell_group _spell_groups[_MAX_SPELL_GROUPS] = {
    { "创造符文",
      "Augment your equipment by attaching runes of various powers. Also, you may create "
      "certain stand-alone runes that grant powers by virtue of being present in your "
      "inventory. Be sure to always keep Absorption handy, for it is your only means "
      "of regaining spell points!",
      TERM_L_BLUE,
      { {  1,   0, 0, _obj_absorption_spell },
        {  5,   0, 0, _obj_protection_spell },
        {  7,   0, 0, _obj_regeneration_spell },
        {  9,   0, 0, _obj_fire_spell },
        { 11,   0, 0, _obj_air_spell },
        { 13,   0, 0, _obj_water_spell },
        { 15,   0, 0, _obj_light_spell },
        { 17,   0, 0, _obj_shadow_spell },
        { 19,   0, 0, _obj_earth_spell },
        { 21,   0, 0, _obj_understanding_spell },
        { 23,   0, 0, _obj_elemental_protection_spell },
        { 25,   0, 0, _obj_haste_spell },
        { 27,   0, 0, _obj_seeing_spell },
        /* XXX I really don't like the playstyle that {Sacrifice} encourages:
         * viz., a tedious process of storing up mana, denying interesting spell
         * casting opportunities, in order to stock pile for the end game. Look,
         * there will be hundreds of junk artifacts, yet it takes about 30 minutes
         * to recover full mana for each rune (Might be faster later on ... and, no,
         * I don't scum for sp). How long should the game be? Personally, I found
         * myself using {Sacrifice} to speed up equipment reshuffles, which is
         * probably not the intended purpose!
        { 29,   0, 0, _obj_sacrifice_spell }, */
        { 29,   0, 0, _obj_life_spell },
        { 31,   0, 0, _obj_stability_spell },
        { 33,   0, 0, _obj_reflection_spell },
        { 35,   0, 0, _obj_death_spell },
        { 37,   0, 0, _obj_mind_spell },
        { 39,   0, 0, _obj_might_spell },
        { 41,   0, 0, _obj_destruction_spell },
        { 43,   0, 0, _obj_good_fortune_spell },
        { 45,   0, 0, _obj_immortality_spell },
        { -1,   0, 0, NULL },
      }
    },
    { "强化符文",
      "将符文刻印在自己身上，以获得暂时或一次性的效果。",
      TERM_L_GREEN, {
        { 10,   5, 20, _self_darkness_spell },
        { 20,   9, 30, _self_seeing_spell },
        { 25,  50, 35, _self_understanding_spell },
        { 30,  25, 35, _self_haste_spell },
        { 32,  15, 35, _self_protection_spell },
        { 37,  20, 50, _self_earth_spell },
        { 39,  15, 50, _self_life_spell },
        { 41,  60, 70, _self_daemon_spell },
        { 50, 100, 80, _self_might_spell },
        { -1,   0,  0, NULL },
      }
    },
    { "转化符文",
      "这些变化符文允许你永久性地改变周围环境。",
      TERM_UMBER, { 
        {  5,  1, 20, _feat_light_spell },
        {  7,  5, 30, _feat_water_spell },
        { 15, 15, 50, _feat_earth_spell },
        { 20, 10, 50, _feat_fire_spell },
        { 25, 15, 60, _feat_air_spell },
        { 30, 15, 65, _feat_stability_spell },
        { 35, 20, 70, _feat_life_spell },
        { 40, 70, 70, _feat_protection_spell },
        { 45, 35, 80, _feat_destruction_spell },
        { -1,  0,  0, NULL },
      }
    },
    { "战斗符文",
      "These runes allow you to effect adjacent enemies. By creating a temporary rune "
      "attached to your melee weapon, you may make a single attack with enhanced power.",
      TERM_RED, {
        { 10,  5,  0, _blow_confusion_spell },
        { 15,  7,  0, _blow_fire_spell },
        { 25,  9,  0, _blow_water_spell },
        { 30,  5,  0, _blow_earth_spell },
        { 35, 12,  0, _blow_death_spell },
        { 40, 20,  0, _blow_elec_spell },
        { 45, 30, 80, _blow_air_spell },
        { 50, 20,  0, _blow_mana_spell },
        { -1,  0,  0, NULL },
      }
    },
};

static int _get_spells_imp(spell_info* spells, int max, _spell_group *spell_group)
{
    int i;
    int ct = 0;
    
    for (i = 0; ; i++)
    {
        spell_info *base = &spell_group->spells[i];
        if (base->level < 0) break;
        if (ct >= max) break;
        if ((base->level <= p_ptr->lev) || (show_future_spells))
        {
            spell_info* current = &spells[ct];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;
            current->fail = base->fail;
            ct++;
        }
    }
    return ct;
}

static void _character_dump(doc_ptr doc)
{
    int i;
    doc_printf(doc, "<topic:Spells>==================================== 法 术 (<color:keypress>S</color>) ===================================\n\n");
    for (i = 0; i < _MAX_SPELL_GROUPS; i++)
    {
        _spell_group_ptr group = &_spell_groups[i];
        spell_info       spells[_MAX_SPELLS_PER_GROUP];
        power_info       powers[_MAX_SPELLS_PER_GROUP];
        int              ct = _get_spells_imp(spells, _MAX_SPELLS_PER_GROUP, group); 

        if (!ct) continue;
        spells[ct].fn = NULL; /* Careful... */
        ct = get_spells_aux(powers, _MAX_SPELLS_PER_GROUP, spells, TRUE);
        doc_printf(doc, "<color:%c>%s</color>\n", attr_to_attr_char(group->color), group->name);
        py_display_spells_aux(doc, powers, ct);
    }
}

static void _spell_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, _spell_groups[which].name);
        break;
    case MENU_HELP:
        var_set_string(res, _spell_groups[which].help);
        break;
    case MENU_COLOR:
        var_set_int(res, _spell_groups[which].color);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static spell_info *_get_spells(void)
{
    int idx = -1;
    int ct = 0;
    int max = MAX_SPELLS;
    menu_t menu = { "使用哪一类法术？", "浏览哪一类法术？", NULL,
                    _spell_menu_fn, _spell_groups, _MAX_SPELL_GROUPS, 0};
    static spell_info spells[MAX_SPELLS];

    idx = menu_choose(&menu);
    if (idx < 0) return 0;
    ct = _get_spells_imp(spells, max, &_spell_groups[idx]);
    if (ct == 0)
    {
        msg_print("你还不懂其中的任何法术！");
        return NULL;
    }
    spells[ct].fn = NULL;
    return spells;
}

static void _calc_bonuses(void)
{
    p_ptr->spell_cap += 7;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 3000;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 1200;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    object_type forge = {0};
    object_prep(&forge, lookup_kind(TV_SWORD, SV_BROAD_SWORD));
    rune_add(&forge, RUNE_ABSORPTION, FALSE);
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

static bool _destroy_object(obj_ptr obj)
{
    if (obj->rune == RUNE_SACRIFICE)
    {   /* XXX Score the object and scale the effect. The bigger the 'sacrifice'
         * the greater the benefit, no? Perhaps allow non-artifacts as well? */
        bool is_equipped = obj->loc.where == INV_EQUIP;
        int add_hp = is_equipped ? p_ptr->mhp : p_ptr->mhp/3;
        int add_sp = is_equipped ? p_ptr->msp : p_ptr->msp/3;

        msg_print("你感觉一股奇妙的力量涌入体内。");

        p_ptr->chp = MIN(p_ptr->mhp, p_ptr->chp + add_hp);
        p_ptr->chp_frac = 0;
        p_ptr->csp = MIN(p_ptr->msp, p_ptr->csp + add_sp);
        p_ptr->csp_frac = 0;

        p_ptr->redraw |= (PR_MANA);
        p_ptr->window |= (PW_SPELL);
        p_ptr->redraw |= (PR_HP);

        if (is_equipped)
        {
            blast_object(obj);
            obj->curse_flags = OFC_HEAVY_CURSE;
        }
        return TRUE;
    }
    return FALSE;
}

class_t *rune_knight_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  23,  36,   2,  18,  16,  48,  35};
    skills_t xs = {  7,   9,   9,   0,   0,   0,  14,  11};

        me.name = "符文骑士";
        me.desc = 
            "The Rune-Knight is a mythical warrior, "
            "dedicated to the discovery of ancient runes that hold immense power. They "
            "may fix mystical runes of various types to their equipment in order to "
            "gain permanent bonuses; even artifacts can be improved this way! "
            "Alternatively, they may conjure a temporary rune which allows "
            "them to attack with enhanced effects, or place one directly on "
            "their person for a one-time or temporary bonus. "
            "Finally, they may alter their surroundings with various runes of "
            "change.\n \n"
            "All runes (except <color:B>{Absorption}</color>) require mana for "
            "creation; however, unlike ordinary spellcasters, Rune-Knights do not "
            "regenerate mana on their own. Rather, they must siphon mana from "
            "magical or elemental attacks directed against them, and doing so "
            "requires the special rune of <color:B>{Absorption}</color>. This "
            "rune should be worn at all times, or at least kept handy.\n \n"
            "Despite having mana, the Rune-Knight does not play like an ordinary "
            "spellcaster; they are more like a weaponsmith but can, on occasion, "
            "cast useful spells. If you have mana available, consider using these "
            "spells; otherwise, play as a warrior and wait to absorb mana. This "
            "can take time, depending on the foes you face, but the Rune-Knight's "
            "honor should prevent them from seeking out weak, defenseless spellcasters; "
            "and in any case, weak attacks can only take your mana so high.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 101;
        me.base_hp = 7;
        me.exp = 150;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.destroy_object = _destroy_object;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
