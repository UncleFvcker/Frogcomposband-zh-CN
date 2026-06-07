#include "angband.h"

static void _birth(void)
{
    p_ptr->current_r_idx = MON_GAZER;
    equip_on_change_race();
    skills_innate_init("凝视", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    py_birth_food();
    py_birth_light();
}

static int _rank(void)
{
    int rank = 0;
    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 35) rank++;
    if (p_ptr->lev >= 45) rank++;
    return rank;
}

/**********************************************************************
 * Innate Attacks
 **********************************************************************/
static void _calc_innate_attacks(void)
{
    if (!p_ptr->blind)
    {
        innate_attack_t a = {0};
        int l = p_ptr->lev;
        int r = _rank();

        /* Beholder melee is unusual as the attacks are not physical. So, Str and Dex
           do not affect blows, accuracy or damage (cf calc_bonuses in xtra1.c). Rings
           of Combat still work, though, as does Weaponmastery (if you can find it!) */

        a.weight = 250; /* unused */
        a.flags = INNATE_NO_CRIT; /* You are gazing at enemies, not bashing them with your eyeballs! */

        a.dd = 2 + r;    /* Max: 6d7 (+50, +25) ... We miss out (+32, +20) from Str and Dex bonuses! */
        a.ds = 3 + r;
        a.to_h = p_ptr->lev;
        a.to_d = p_ptr->lev/2;

        a.effect[0] = GF_MISSILE;

        a.effect[1] = GF_DRAIN_MANA;
        a.effect_chance[1] = 50+l;

        a.effect[2] = GF_OLD_CONF;
        a.effect_chance[2] = 40+l;

        if (p_ptr->lev >= 45)
        {
            a.effect[3] = GF_STASIS;
            a.effect_chance[3] = 15 + l/2;
        }
        else
        {
            a.effect[3] = GF_OLD_SLEEP;
            a.effect_chance[3] = 30+l;
        }

        a.effect[4] = GF_TURN_ALL;
        a.effect_chance[4] = 15 + l/2;

        if (p_ptr->lev >= 35)
        {
            a.effect[5] = GF_STUN;
            a.effect_chance[5] = 15 + l/2;

            a.effect[6] = GF_AMNESIA;
            a.effect_chance[6] = 15 + l/2;
        }

        {
            int pow = p_ptr->lev + adj_dex_blow[p_ptr->stat_ind[A_INT]];
            a.blows = 100 + MIN(300, 300 * pow / 60);
        }
        a.msg = "You gaze.";
        a.name = "凝视";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _gaze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "凝视");
        break;
    case SPELL_DESC:
        var_set_string(res, "凝视附近的怪物，产生各种效果。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(6 + _rank()));
        break;
    case SPELL_COST_EXTRA:
    {
        int costs[5] = {1, 5, 15, 25, 35};
        var_set_int(res, costs[_rank()]);
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        project_length = 6 + _rank();
        if (get_fire_dir(&dir))
        {
            project_hook(GF_ATTACK, dir, BEHOLDER_GAZE, PROJECT_STOP | PROJECT_KILL);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _vision_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "视野");
        break;
    case SPELL_DESC:
        var_set_string(res, "映射附近的地图。");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_MAP);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/***********************************************************************************
 *                 10           25          35                 45
 * Beholder: Gazer -> Spectator -> Beholder -> Undead Beholder -> Ultimate Beholder
 ***********************************************************************************/
static spell_info _beholder_spells[] = {
    {  1,  1, 30, detect_monsters_spell},
    {  1,  0,  0, _gaze_spell},
    { 15,  7, 30, _vision_spell},
    { 25,  7, 30, drain_mana_spell},
    { -1, -1, -1, NULL}
};
static spell_info _undead_beholder_spells[] = {
    {  1,  1, 30, detect_monsters_spell},
    {  1,  0,  0, _gaze_spell},
    { 15,  7, 30, _vision_spell},
    { 25,  7, 30, drain_mana_spell},
    { 35, 10, 30, animate_dead_spell},
    { 35, 15, 50, mana_bolt_I_spell},
    { 35, 20, 50, brain_smash_spell},
    { 35, 30, 50, summon_undead_spell},
    { -1, -1, -1, NULL}
};
static spell_info _ultimate_beholder_spells[] = {
    {  1,  1, 30, detect_monsters_spell},
    {  1,  0,  0, _gaze_spell},
    { 15,  7, 30, _vision_spell},
    { 25,  7, 30, drain_mana_spell},
    { 35, 20, 50, brain_smash_spell},
    { 45, 30, 50, mana_bolt_II_spell},
    { 45, 40, 60, summon_kin_spell},
    { 45, 40, 60, dispel_magic_spell},
    { 45, 50, 80, darkness_storm_II_spell},
    { -1, -1, -1, NULL}
};
static spell_info *_get_spells(void) {
    if (p_ptr->blind)
    {
        msg_print("你看不见！");
        return NULL;
    }

    if (p_ptr->lev >= 45)
        return _ultimate_beholder_spells;
    else if (p_ptr->lev >= 35)
        return _undead_beholder_spells;
    else
        return _beholder_spells;
}
static power_info _get_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void) {
    int l = p_ptr->lev;
    int ac = l/2;

    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;

    p_ptr->levitation = TRUE;
    if (p_ptr->lev >= 45)
    {
        p_ptr->to_a += 40;
        p_ptr->dis_to_a += 40;

        p_ptr->telepathy = TRUE;
        res_add(RES_TELEPORT);
        res_add(RES_POIS);
        res_add(RES_CONF);
        p_ptr->free_act++;
        p_ptr->pspeed += 6;
    }
    else if (p_ptr->lev >= 35)
    {
        p_ptr->to_a += 30;
        p_ptr->dis_to_a += 30;

        p_ptr->telepathy = TRUE;
        res_add(RES_TELEPORT);
        res_add(RES_ACID);
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ELEC);
        res_add(RES_POIS);
        res_add(RES_NETHER);
        res_add(RES_CONF);
        p_ptr->free_act++;
        p_ptr->pspeed += 4;
        p_ptr->hold_life++;
    }
    else if (p_ptr->lev >= 25)
    {
        p_ptr->to_a += 20;
        p_ptr->dis_to_a += 20;

        res_add(RES_TELEPORT);
        res_add(RES_POIS);
        res_add(RES_CONF);
        p_ptr->free_act++;
        p_ptr->pspeed += 2;
    }
    else if (p_ptr->lev >= 10)
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;

        res_add(RES_FEAR);
        res_add(RES_CONF);
        p_ptr->free_act++;
    }
    else
    {
        res_add(RES_POIS);
    }
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 45)
    {
        add_flag(flgs, OF_TELEPATHY);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SPEED);
    }
    else if (p_ptr->lev >= 35)
    {
        add_flag(flgs, OF_TELEPATHY);
        add_flag(flgs, OF_RES_ACID);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_NETHER);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_HOLD_LIFE);
    }
    else if (p_ptr->lev >= 25)
    {
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SPEED);
    }
    else if (p_ptr->lev >= 10)
    {
        add_flag(flgs, OF_RES_FEAR);
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_FREE_ACT);
    }
    else
    {
        add_flag(flgs, OF_RES_POIS);
    }
}
static void _gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_GAZER && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_SPECTATOR;
        msg_print("你进化成了旁观者(Spectator)。");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    if (p_ptr->current_r_idx == MON_SPECTATOR && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_BEHOLDER;
        msg_print("你进化成了眼魔(Beholder)。");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    if (p_ptr->current_r_idx == MON_BEHOLDER && new_level >= 35)
    {
        p_ptr->current_r_idx = MON_UNDEAD_BEHOLDER;
        msg_print("你进化成了不死眼魔(Undead Beholder)。");
        equip_on_change_race();
        lp_player(1000); /* undead - no life drain */
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
    if (p_ptr->current_r_idx == MON_UNDEAD_BEHOLDER && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_ULTIMATE_BEHOLDER;
        p_ptr->psex = SEX_FEMALE;
        msg_print("你进化成了终极眼魔(Ultimate Beholder)。");
        equip_on_change_race();
        p_ptr->redraw |= PR_MAP | PR_BASIC;
    }
}
static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "力量";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 100;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_beholder_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[5] =  {"凝视者", "旁观者", "眼魔", "不死眼魔", "终极眼魔"};
    int           rank = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  50,  47,   7,  20,  20,  40,  20};
    skills_t xs = { 10,  20,  15,   1,  20,  20,  16,   7};

        me.name = "眼魔";
        me.desc = "眼魔是漂浮的肉球，中央长有一只大眼睛，周围环绕着许多长在眼柄上的小眼睛。它们用凝视进行攻击，通常会使敌人混乱甚至瘫痪。它们无法使用普通的武器或护甲，但可以在每个眼柄上装备一枚戒指，并且眼柄的数量会随着眼魔的进化而增加。\n \n随着眼魔的进化，它们会获得更强大的力量，包括一些强力的攻击性法术；但它们的主要攻击手段始终是强大的凝视。智力是它们的施法属性，而眼魔确实非常聪明。它们的搜索和察觉能力是传说级别的，并且非常擅长使用魔法装置。然而，它们并不算强壮，在被敌人包围时必须小心。\n \n眼魔的攻击方式很独特。因为凝视不是普通的物理攻击，它们无法从力量和敏捷中获得近战加成。此外，攻击次数是由等级决定的，而不是通常的方式：在这方面，它们类似于武僧。最后，眼魔不需要靠近敌人就能进行“近战”攻击；它们可以凝视远处的怪物，尽管其凝视的范围受到一定限制，而且长时间的远距离凝视会使它们疲惫不堪。";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 8;
        me.exp = 200;
        me.base_hp = 20;
        me.life = 100;
        me.shop_adjust = 140;

        me.birth = _birth;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.get_powers = _get_powers;
        me.get_spells_fn = _get_spells;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;

        me.boss_r_idx = MON_OMARAX;
        init = TRUE;
    }

    if (!birth_hack && !spoiler_hack)
        me.subname = titles[rank];
    me.stats[A_STR] = -3;
    me.stats[A_INT] =  4 + rank;
    me.stats[A_WIS] =  0;
    me.stats[A_DEX] =  1 + rank/2;
    me.stats[A_CON] =  0;
    me.stats[A_CHR] =  0 + rank/2;

    me.pseudo_class_idx = CLASS_MAGE;
    me.flags = RACE_IS_MONSTER;
    if (p_ptr->current_r_idx == MON_UNDEAD_BEHOLDER)
    {
        me.flags |= (RACE_IS_NONLIVING | RACE_IS_UNDEAD);
    }

    me.equip_template = mon_get_equip_template();

    return &me;
}
