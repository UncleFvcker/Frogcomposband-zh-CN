#include "angband.h"

static cptr _desc = 
    "Golems are creatures animated by powerful magics: living stone conjured "
    "to serve their creators. But you have broken loose and are running amok!\n \n"
    "Golems are very slow when it comes to moving about; their non-movement actions "
    "also tend to be slow, but not to the same extent. They excel in melee despite being limited to a "
    "single (but mighty) blow; they can equip a weapon, but fight much better with their powerful fists. "
    "Golems have a very high armor class and possess good resistances; they are also resistant to magic, and become "
    "more resistant as they evolve, reducing the damage taken from all magical attacks.\n \n"
    "There are several varieties of golem, all sharing a common evolutionary "
    "heritage. First there is the mighty Colossus, the biggest (and slowest) of their "
    "kind. Next is the Sky Golem, a creature of powerful magic, resistant to the "
    "ravages of time. Finally, there is the Spellwarp Automaton, a creature of "
    "utter destruction, almost completely immune to magic; unfortunately, this last form "
    "takes nearly forever to build.";

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static int _rank(void)
{
    int r = 0;
    if (p_ptr->lev >= 10) r++;
    if (p_ptr->lev >= 20) r++;
    if (p_ptr->lev >= 30) r++;
    if (p_ptr->lev >= 40) r++;
    if (p_ptr->lev >= 45) r++;
    return r;
}

/**********************************************************************
 * Common Evolution: 
 *       10       20      30         40     45
 *  Clay -> Stone -> Iron -> Mithril -> Eog -> Colossus | Sky | Spellwarp
 **********************************************************************/
static void _birth(void) 
{ 
    object_type forge;

    p_ptr->current_r_idx = MON_CLAY_GOLEM;
    skills_innate_init("重拳", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_BOOTS, SV_PAIR_OF_METAL_SHOD_BOOTS));
    py_birth_obj(&forge);

    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}

static int _attack_level(void)
{
    int l = p_ptr->lev;
    switch (p_ptr->psubrace)
    {
    case GOLEM_COLOSSUS:
        l = MAX(1, l * 100 / 100);
        break;
    case GOLEM_SKY:
        l = MAX(1, l * 95 / 100);
        break;
    case GOLEM_SPELLWARP:
        l = MAX(1, l * 97 / 100);
        break;
    }
    return l;
}

static void _calc_innate_attacks(void)
{
    if (p_ptr->weapon_ct == 0 && equip_find_empty_hand())
    {
        innate_attack_t a = {0};
        int l = _attack_level();

        a.dd = 5 + l/5 + l*l/250;
        a.ds = 5 + l/5 + l*l/250;
        a.weight = 100 + 8*l;
        a.to_h = l;
        a.to_d = l + l*l/50 + l*l*l/2500;

        if (!equip_find_first(object_is_shield))
        {
            a.to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128) + ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
            a.to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128) * 3/4;
        }

        if (l >= 30)
            a.effect[1] = GF_STUN;

        a.blows = 100;
        a.msg = "你挥拳打去。";
        a.name = "重拳";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_CLAY_GOLEM && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_STONE_GOLEM;
        msg_print("你进化成了石魔像(Stone Golem)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GOLEM && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_IRON_GOLEM;
        msg_print("你进化成了铁魔像(Iron Golem)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_IRON_GOLEM && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_MITHRIL_GOLEM;
        msg_print("你进化成了秘银魔像(Mithril Golem)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MITHRIL_GOLEM && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_EOG_GOLEM;
        msg_print("你进化成了伊奥格魔像(Eog Golem)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_EOG_GOLEM && new_level >= 45)
    {
        switch (p_ptr->psubrace)
        {
        case GOLEM_COLOSSUS:
            p_ptr->current_r_idx = MON_COLOSSUS;
            msg_print("你进化成了巨像(Colossus)。");
            break;
        case GOLEM_SKY:
            p_ptr->current_r_idx = MON_SKY_GOLEM;
            msg_print("你进化成了天空魔像(Sky Golem)。");
            break;
        case GOLEM_SPELLWARP:
            p_ptr->current_r_idx = MON_SPELLWARP_AUTOMATON;
            msg_print("你进化成了咒曲自动机(Spellwarp Automaton)。");
            break;
        }
        p_ptr->redraw |= PR_MAP;
    }
}

static void _calc_bonuses(void) 
{
    if (p_ptr->innate_attack_info.xtra_blow > 0)
        p_ptr->innate_attack_info.xtra_blow = 0;

    /* Clay Golem */
    p_ptr->to_a += 5;
    p_ptr->dis_to_a += 5;
    p_ptr->free_act++;
    p_ptr->hold_life++;
    p_ptr->pspeed -= 1;
    res_add(RES_POIS);

    /* Stone Golem */
    if (p_ptr->lev >= 10)
    {
        p_ptr->to_a += 5;
        p_ptr->dis_to_a += 5;
        res_add(RES_FEAR);
    }

    /* Iron Golem */
    if (p_ptr->lev >= 20)
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->see_inv++;
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ELEC);
        p_ptr->magic_resistance += 5;
    }

    /* Mithril Golem */
    if (p_ptr->lev >= 30)
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->pspeed -= 1;
        p_ptr->reflect = TRUE;
        res_add(RES_CONF);
        res_add(RES_SHARDS);
        p_ptr->magic_resistance += 5;
    }

    /* Eog Golem */
    if (p_ptr->lev >= 40)
    {
        p_ptr->to_a += 10;            /* +40 */
        p_ptr->dis_to_a += 10;
        p_ptr->pspeed -= 1;
        p_ptr->magic_resistance += 5; /* 15% */
    }

    if (p_ptr->current_r_idx == MON_COLOSSUS)
    {
        res_add(RES_SOUND);
        res_add(RES_DISEN);
        p_ptr->pspeed -= 4;
        p_ptr->to_a += 25;
        p_ptr->dis_to_a += 25;
        p_ptr->magic_resistance += 5;
    }

    if (p_ptr->current_r_idx == MON_SKY_GOLEM)
    {
        res_add(RES_COLD);
        res_add(RES_TIME);
        res_add(RES_LITE);
        p_ptr->magic_resistance += 5;
    }

    if (p_ptr->current_r_idx == MON_SPELLWARP_AUTOMATON)
    {
        res_add(RES_TELEPORT);

        p_ptr->pspeed -= 1;
        p_ptr->no_stun = TRUE;
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->magic_resistance += 20;

        /* RES_ALL ... Magic Resistance no longer applies
           to innate breath attacks which are not really magical */
        res_add(RES_ACID);
        res_add(RES_LITE);
        res_add(RES_DARK);
        res_add(RES_NETHER);
        res_add(RES_NEXUS);
        res_add(RES_SOUND);
        res_add(RES_CHAOS);
        res_add(RES_DISEN);
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    /* Clay Golem */
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_DEC_SPEED);

    /* Stone Golem */
    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, OF_RES_FEAR);
    }

    /* Iron Golem */
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, OF_SEE_INVIS);
        add_flag(flgs, OF_RES_FIRE);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_MAGIC_RESISTANCE);
    }

    /* Mithril Golem */
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_CONF);
        add_flag(flgs, OF_RES_SHARDS);
        add_flag(flgs, OF_REFLECT);
    }

    /* Eog Golem */
    if (p_ptr->lev >= 40)
    {
    }

    if (p_ptr->current_r_idx == MON_COLOSSUS)
    {
        add_flag(flgs, OF_RES_SOUND);
        add_flag(flgs, OF_RES_DISEN);
    }

    if (p_ptr->current_r_idx == MON_SKY_GOLEM)
    {
        add_flag(flgs, OF_RES_TIME);
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_LITE);
    }

    if (p_ptr->current_r_idx == MON_SPELLWARP_AUTOMATON)
    {
    }
}

/**********************************************************************
 * Powers
 **********************************************************************/
static void _big_punch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "重拳击退");
        break;
    case SPELL_DESC:
        var_set_string(res, "用一次毁灭性的打击攻击相邻的对手。");
        break;
    case SPELL_CAST:
        if (p_ptr->innate_attack_ct)
            var_set_bool(res, do_blow(GOLEM_BIG_PUNCH));
        else
        {
            msg_print("不能在挥舞武器时使用！");
            var_set_bool(res, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_cold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寒冰吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐寒冰");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你喷吐出寒冰。");
        fire_ball(GF_COLD, dir, p_ptr->chp / 2, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_disintegration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "解体吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "一种能引发物体解体的吐息。就连地下城的墙壁也无法抵挡它的威力！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 4));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir_aux(&dir, TARGET_DISI)) return;

        msg_print("你喷吐出解体光波。");
        fire_ball(GF_DISINTEGRATE, dir, p_ptr->chp / 4, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光明吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定目标喷吐光芒");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 3));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你喷吐出光芒。");
        fire_ball(GF_LITE, dir, p_ptr->chp / 3, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐时间能量");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 5));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你喷吐出时间能量。");
        fire_ball(GF_TIME, dir, p_ptr->chp / 5, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _shoot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标发射一枚飞弹。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(15, 15, 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你发射了一枚飞弹。");
        fire_bolt(GF_MISSILE, dir, damroll(15, 15));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _stone_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎石击");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁相邻的选定墙壁。");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        
        if (!in_bounds(y, x)) return;

        if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            p_ptr->update |= PU_FLOW;
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _powers[] = 
{
    { A_STR, {  5,  5, 30, _stone_smash_spell} },
    { A_CON, { 10, 10, 30, stone_skin_spell} },
    { A_STR, { 40,100,  0, _big_punch_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _colossus_powers[] = 
{
    { A_STR, { 45, 15, 50, _shoot_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _sky_powers[] = 
{
    { A_STR, { 45, 10, 50, ice_bolt_spell} },
    { A_CON, { 45, 30, 50, _breathe_light_spell} },
    { A_CON, { 45, 30, 50, _breathe_cold_spell} },
    { A_CON, { 50, 35, 60, _breathe_time_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _spellwarp_powers[] = 
{
    { A_STR, { 45, 25, 50, _breathe_disintegration_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info *_get_powers(void) 
{
    static power_info spells[MAX_SPELLS];
    int max = MAX_SPELLS;
    int ct = get_powers_aux(spells, max, _powers, FALSE);
    switch (p_ptr->current_r_idx)
    {
    case MON_COLOSSUS:
        ct += get_powers_aux(spells + ct, max - ct, _colossus_powers, FALSE);
        break;
    case MON_SKY_GOLEM:
        ct += get_powers_aux(spells + ct, max - ct, _sky_powers, FALSE);
        break;
    case MON_SPELLWARP_AUTOMATON:
        ct += get_powers_aux(spells + ct, max - ct, _spellwarp_powers, FALSE);
        break;
    }
    spells[ct].spell.fn = NULL;

    return spells;
}

static name_desc_t _info[GOLEM_MAX] = {
    { "巨像", "The Colossus is the biggest of all golems, truly immense. "
                  "Unfortunately, they are also the slowest of all golems on "
                  "account of their great size." },
    { "天空魔像", "The Sky Golem is the product of powerful enchantments, resistant "
                   "to the ravages of time. They may even breathe time!" },
    { "咒曲自动机", "The Spellwarp Automaton is nearly indestructible, being "
                             "almost completely immune to magic. However, their great "
                             "power takes a seeming eternity to mature." },
};

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_golem_get_race(int psubrace)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  40,   0,  10,   7,  70,  30};
    skills_t xs = { 10,   7,  15,   0,   0,   0,  30,  10};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.shop_adjust = 130;

        me.name = "魔像";
        me.desc = _desc;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.birth = _birth;
        me.gain_level = _gain_level;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers_fn = _get_powers;
        me.get_flags = _get_flags;
        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING | RACE_EATS_DEVICES;
        me.base_hp = 50;
        me.boss_r_idx = MON_DESTROYER;
        me.pseudo_class_idx = CLASS_MAULER;

        init = TRUE;

    }

    me.subname = _mon_name(p_ptr->current_r_idx);

    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -2 - (rank+1)/2;
    me.stats[A_WIS] = -2 - (rank+1)/2;
    me.stats[A_DEX] = -1 - (rank+1)/2;
    me.stats[A_CON] =  1 + rank;
    me.stats[A_CHR] =  0 + (rank+1)/2;

    switch (psubrace)
    {
    case GOLEM_SKY:
        me.life = 100 + rank;
        me.exp = 250;
        break;

    case GOLEM_SPELLWARP:
        me.life = 100 + (rank * 3 / 2);
        me.exp = 350;
        break;

    case GOLEM_COLOSSUS:
    default:
        if (p_ptr->current_r_idx == MON_COLOSSUS)
        {
            me.stats[A_STR] += 3;
            me.stats[A_DEX] -= 2;
            me.stats[A_CON] += 3;
        }
        me.life = 102 + (rank * ((rank < 4) ? 2 : 3));
        me.exp = 200;
        break;
    }

    if (birth_hack || spoiler_hack)
    {
        me.subname = _info[psubrace].name;
        me.subdesc = _info[psubrace].desc;
    }

    return &me;
}

