#include "angband.h"

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static bool _summon_aux(int num, bool pet, int y, int x, int lev, int type, u32b mode)
{
    int plev = p_ptr->lev;
    int who;
    int i;
    bool success = FALSE;

    /*if (!lev) lev = spell_power(plev) + randint1(spell_power(plev * 2 / 3));*/
    if (!lev) lev = MAX(plev, dun_level);

    if (pet)
    {
        mode |= PM_FORCE_PET;
        if (mode & PM_ALLOW_UNIQUE)
        {
            if (randint1(50 + plev) >= plev / 10)
                mode &= ~PM_ALLOW_UNIQUE;
        }
        who = -1;
    }
    else
    {
        mode |= PM_NO_PET;
        who = 0;
    }

    for (i = 0; i < num; i++)
    {
        if (summon_specific(who, y, x, lev, type, mode))
            success = TRUE;
    }

    if (!success)
        msg_print("没有人回应你的求救。");

    return success;
}

static void _summon(int what, int num, bool fail)
{
    int x = px;
    int y = py;

    if (fail) /* Failing spells should not be insta-death ... */
    {
        num = MAX(1, num/4);
        /* I'm debating this ... with -10 speed, Q's just die if they fail early on! */
        return;
    }
    else
        num = spell_power(num);

    if (!fail && p_ptr->lev >= 20 && !one_in_(3) && get_direct_target() && los(py, px, target_row, target_col))
    {
        y = target_row;
        x = target_col;
    }
    if (_summon_aux(num, !fail, y, x, 0, what, PM_ALLOW_UNIQUE | PM_ALLOW_GROUP))
    {
        if (fail)
        {
            if (num == 1)
                msg_print("召唤来的怪物被激怒了！");
            else
                msg_print("召唤来的怪物们被激怒了！");
        }
    }
}

/**********************************************************************
 * Spells
 **********************************************************************/
void _heal_monster_spell(int cmd, variant *res)
{
    int heal = spell_power(p_ptr->lev * 10 + 200);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试治疗选定的怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, heal));
        break;
    case SPELL_CAST:
    {
        int dir;
        bool result;
        bool old_target_pet = target_pet;

        var_set_bool(res, FALSE);

        target_pet = TRUE;
        result = get_fire_dir(&dir);
        target_pet = old_target_pet;

        if (!result) return;

        heal_monster(dir, heal);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ancient_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤远古龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一条远古龙来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤蚂蚁");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤蚂蚁来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ANT, randint1(2), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_balrog_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤炎魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一只炎魔来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_BALROG, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_clubber_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤大头棒恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一只大头棒恶魔来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_CLUBBER_DEMON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dark_elf_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤黑暗精灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤黑暗精灵来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DARK_ELF, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一只恶魔来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_demon_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤恶魔召唤师");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一名恶魔召唤师来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一条龙来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_dragon_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤龙语召唤师");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一名龙语召唤师来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DRAGON_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_elemental_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤元素");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一个元素生物来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ELEMENTAL, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_giant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤巨人");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一名巨人来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_GIANT, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_golem_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤魔像");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一具魔像来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_GOLEM, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤高等龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤多条高等龙来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_DRAGON, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤高等恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤多只高等恶魔来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_DEMON, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_high_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤高等亡灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤高等亡灵来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HI_UNDEAD, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_hounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤猎犬");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤猎犬群来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_HOUND, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_lich_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤巫妖");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一名巫妖来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_LICH, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_mature_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤成年龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一条成年龙来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_MATURE_DRAGON, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_orc_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤兽人");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤兽人群来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ORC, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_spider_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤蜘蛛");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤蜘蛛群来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_SPIDER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_ultimate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤终极怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤最强大的怪物来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_ULTIMATE, randint1(2), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤亡灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一个亡灵来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_UNDEAD, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_undead_summoner_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤亡灵召唤师");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一名亡灵召唤师来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_UNDEAD_SUMMONER, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_wight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤尸妖");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤一只尸妖来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_WIGHT, 1, cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _summon_yeek_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤伊克人");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试召唤伊克人来协助。");
        break;
    case SPELL_FAIL:
    case SPELL_CAST:
    {
        _summon(SUMMON_YEEK, randint1(3), cmd == SPELL_FAIL);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _baby_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    {  1,  2, 35, _summon_yeek_spell},
    {  2,  6, 35, _summon_spider_spell},
    {  3,  7, 35, _summon_ant_spell},
    {  4,  9, 35, _summon_orc_spell},
    {  8, 10, 35, _summon_dark_elf_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 15, 15, 45, _summon_giant_spell},
    { 18, 20, 45, _summon_golem_spell},
    { 20, 20, 45, _summon_elemental_spell},
    { 23, 25, 45, _summon_hounds_spell},
    { 25, 35, 55, _heal_monster_spell},
    { -1, -1, -1, NULL}
};

static spell_info _rotting_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_lich_spell},
    { 35, 23, 55, _summon_wight_spell},
    { 38, 30, 55, _summon_undead_summoner_spell},
    { 42, 45, 65, _summon_high_undead_spell},
    { -1, -1, -1, NULL}
};
static spell_info _draconic_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_mature_dragon_spell},
    { 38, 30, 55, _summon_ancient_dragon_spell},
    { 38, 30, 55, _summon_dragon_summoner_spell},
    { 42, 45, 65, _summon_high_dragon_spell},
    { -1, -1, -1, NULL}
};
static spell_info _demonic_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 45, _summon_clubber_demon_spell},
    { 38, 30, 55, _summon_balrog_spell},
    { 38, 30, 55, _summon_demon_summoner_spell},
    { 42, 45, 65, _summon_high_demon_spell},
    { -1, -1, -1, NULL}
};

static spell_info _master_spells[] = 
{
    {  1,  0, 25, phase_door_spell},
    {  1,  5, 35, teleport_spell},
    {  1, 10, 50, teleport_other_spell},
    { 13, 15, 45, _summon_demon_spell},
    { 13, 15, 45, _summon_undead_spell},
    { 13, 15, 45, _summon_dragon_spell},
    { 25, 35, 55, _heal_monster_spell},
    { 33, 20, 55, _summon_lich_spell},
    { 33, 20, 45, _summon_clubber_demon_spell},
    { 35, 23, 55, _summon_wight_spell},
    { 38, 30, 55, _summon_balrog_spell},
    { 38, 30, 55, _summon_demon_summoner_spell},
    { 38, 30, 55, _summon_dragon_summoner_spell},
    { 38, 30, 55, _summon_undead_summoner_spell},
    { 42, 45, 65, _summon_high_demon_spell},
    { 42, 45, 65, _summon_high_dragon_spell},
    { 42, 45, 65, _summon_high_undead_spell},
    { 50, 50, 75, _summon_ultimate_spell},
    { -1, -1, -1, NULL}
};

static spell_info *_get_spells(void) 
{
    switch (p_ptr->current_r_idx)
    {
    case MON_ROTTING_QUYLTHULG:
    case MON_GREATER_ROTTING_QUYLTHULG:
        return _rotting_spells;
    case MON_DRACONIC_QUYLTHULG:
    case MON_GREATER_DRACONIC_QUYLTHULG:
        return _draconic_spells;
    case MON_DEMONIC_QUYLTHULG:
    case MON_GREATER_DEMONIC_QUYLTHULG:
        return _demonic_spells;
    case MON_MASTER_QUYLTHULG:
        return _master_spells;
    }

    return _baby_spells;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "颤动力量";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        init = TRUE;
    }
    return &me;
}


/**********************************************************************
 * Bonuses
 **********************************************************************/
static void _calc_bonuses(void) 
{
    p_ptr->pspeed -= 10;
    p_ptr->pspeed += p_ptr->lev / 10;

    res_add(RES_FEAR);
    res_add(RES_BLIND);
    res_add(RES_CONF);
    p_ptr->regen += 100;
    p_ptr->slow_digest = TRUE;
    p_ptr->telepathy = TRUE;
    p_ptr->free_act++;
    p_ptr->see_inv++;
    p_ptr->sustain_chr = TRUE;

    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG)
        res_add(RES_NEXUS);

    if ( p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
    {
        res_add(RES_NETHER);
        p_ptr->hold_life++;
    }

    if ( p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG )
    {
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ACID);
        res_add(RES_ELEC);
        res_add(RES_POIS);
    }

    if ( p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG )
    {
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add(RES_FIRE);
        res_add(RES_CHAOS);
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_DEC_SPEED);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_RES_BLIND);
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_REGEN);
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_TELEPATHY);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_SUST_CHR);

    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG)
        add_flag(flgs, OF_RES_NEXUS);

    if ( p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_HOLD_LIFE);
    }

    if ( p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
    }

    if ( p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG
      || p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG )
    {
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_CHAOS);
    }
}

/**********************************************************************
 * Birth and Evolution
 **********************************************************************/
static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_QUYLTHULG;
    equip_on_change_race();
    
    object_prep(&forge, lookup_kind(TV_CAPTURE, 0));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_WHISTLE, 1));
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_QUYLTHULG && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_NEXUS_QUYLTHULG;
        msg_print("你进化成了时空库尔苏格(Nexus Quylthulg)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_NEXUS_QUYLTHULG && new_level >= 30)
    {
        int which = randint1(3);
        if (spoiler_hack) which = 1;
        switch (which)
        {
        case 1:
            p_ptr->current_r_idx = MON_ROTTING_QUYLTHULG;
            msg_print("你进化成了腐烂库尔苏格(Rotting Quylthulg)。");
            break;
        case 2:
            p_ptr->current_r_idx = MON_DRACONIC_QUYLTHULG;
            msg_print("你进化成了龙裔库尔苏格(Draconic Quylthulg)。");
            break;
        case 3:
            p_ptr->current_r_idx = MON_DEMONIC_QUYLTHULG;
            msg_print("你进化成了恶魔库尔苏格(Demonic Quylthulg)。");
            break;
        }
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ROTTING_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_ROTTING_QUYLTHULG;
        msg_print("你进化成了高等腐烂库尔苏格(Greater Rotting Quylthulg)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DRACONIC_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_DRACONIC_QUYLTHULG;
        msg_print("你进化成了高等龙裔库尔苏格(Greater Draconic Quylthulg)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEMONIC_QUYLTHULG && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREATER_DEMONIC_QUYLTHULG;
        msg_print("你进化成了高等恶魔库尔苏格(Greater Demonic Quylthulg)。");
        p_ptr->redraw |= PR_MAP;
    }
    if ( ( p_ptr->current_r_idx == MON_GREATER_DEMONIC_QUYLTHULG 
        || p_ptr->current_r_idx == MON_GREATER_DRACONIC_QUYLTHULG
        || p_ptr->current_r_idx == MON_GREATER_ROTTING_QUYLTHULG )
      && new_level >= 50 )
    {
        p_ptr->current_r_idx = MON_MASTER_QUYLTHULG;
        msg_print("你进化成了库尔苏格大师(Master Quylthulg)。");
        p_ptr->redraw |= PR_MAP;
    }
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_quylthulg_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;
    if (p_ptr->lev >= 50) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  45,  40,   7,  20,  30,   0,   0 };
    skills_t xs = {  7,  15,  12,   0,   0,   0,   0,   0 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "库尔苏格";
        me.desc = "库尔苏格是一堆令人作呕的、不断颤动和脉动的肉块，几乎无法移动。在身体上，它们非常可怜。然而，它们控制他人服从命令的能力是传说级别的，并且在情况不妙时它们非常擅长逃跑。库尔苏格没有任何物理攻击能力。";

        me.infra = 5;
        me.exp = 150;
        me.base_hp = 0;
        me.shop_adjust = 120;

        me.get_spells_fn = _get_spells;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;
        me.pseudo_class_idx = CLASS_SORCERER;

        me.flags = RACE_IS_MONSTER;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    me.stats[A_STR] = -5 + rank;
    me.stats[A_INT] = -5 + rank;
    me.stats[A_WIS] = -5 + rank;
    me.stats[A_DEX] = -5 + rank;
    me.stats[A_CON] = -5 + rank;
    me.stats[A_CHR] =  6 + rank;
    me.life = 95;
    me.boss_r_idx = MON_EMPEROR_QUYLTHULG;

    me.equip_template = mon_get_equip_template();
    return &me;
}
