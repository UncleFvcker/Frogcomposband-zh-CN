#include "angband.h"

#include <assert.h>

static const char * _desc = 
    "One of the mightier undead creatures, the Vampire is an awe-inspiring sight. Yet this "
    "dread creature has a serious weakness: the bright rays of sun are its bane, and it "
    "will need to flee the surface to the deep recesses of earth until the sun finally "
    "sets. Being undead, Vampires resist nether and have a firm hold on their life force; "
    "vampires also resists cold and poison, and darkness only makes them feel at home. "
    "Vampires are, however, susceptible to their "
    "perpetual hunger for fresh blood, which can only be satiated by sucking blood "
    "from a nearby monster.\n \n"
    "Vampires gain access to various dark powers as they evolve. Of course, they gain a vampiric bite at a "
    "very early stage, as they must use this power to feed on the living. Killing humans with this "
    "power is also a means of perpetuating the vampire species, and many are the servants of a "
    "true prince of darkness! Vampires are rumored to have limited shapeshifting abilities "
    "and a powerful, hypnotic gaze.";

bool vampiric_drain_hack = FALSE;

bool get_adjacent_target(int *x, int *y, int *mon_idx)
{
    int dir = 0;
    *x = 0;
    *y = 0;
    *mon_idx = 0;

    if (old_target_okay())
    {
        *y = target_row;
        *x = target_col;
        *mon_idx = cave[*y][*x].m_idx;
        if (*mon_idx)
        {
            if (m_list[*mon_idx].cdis > 1)
                *mon_idx = 0;
            else
                dir = 5;
        }
    }

    if (!(*mon_idx))
    {
        if (!get_rep_dir2(&dir)) return FALSE;
        if (dir == 5) return FALSE;
        *y = py + ddy[dir];
        *x = px + ddx[dir];
        *mon_idx = cave[*y][*x].m_idx;

        if (!(*mon_idx))
        {
            msg_print("那里没有怪物。");
            return FALSE;
        }
    }

    return TRUE;
}

/******************************************************************************
 *                  25                35              45
 * Vampire: Vampire -> Master Vampire -> Vampire Lord -> Elder Vampire
 ******************************************************************************/
static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_VAMPIRE;
    equip_on_change_race();
    
    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_DAGGER));
    forge.name2 = EGO_WEAPON_DEATH;
    py_birth_obj(&forge);

    /* Encourage shapeshifting! */
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 4;
    py_birth_obj(&forge);
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_VAMPIRE && new_level >= 25)
    {
        p_ptr->current_r_idx = MON_MASTER_VAMPIRE;
        msg_print("你进化成了吸血鬼大师(Master Vampire)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MASTER_VAMPIRE && new_level >= 35)
    {
        p_ptr->current_r_idx = MON_VAMPIRE_LORD;
        msg_print("你进化成了吸血鬼领主(Vampire Lord)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_VAMPIRE_LORD && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_ELDER_VAMPIRE;
        msg_print("你进化成了远古吸血鬼(Elder Vampire)。");
        p_ptr->redraw |= PR_MAP;
    }
}

/******************************************************************************
 * Powers
 ******************************************************************************/
static int _bite_amt(void)
{
    return 5 + py_prorata_level_aux(300, 1, 2, 3);
}
static void _bite_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血之咬");
        break;
    case SPELL_DESC:
        var_set_string(res, "作为一名吸血鬼，你必须吸食新鲜的血液以维持你的不死生命！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _bite_amt()));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if ((d_info[dungeon_type].flags1 & DF1_NO_MELEE) || (no_melee_challenge))
        {
            msg_print("某种力量阻止了你进行攻击。");
            return;
        }
        else
        {
            int x = 0, y = 0, amt, m_idx = 0;
            if (!get_adjacent_target(&x, &y, &m_idx))
            {
                if (p_ptr->blind > 0) var_set_bool(res, TRUE);
                return;
            }

            var_set_bool(res, TRUE);

            msg_print("你咧开嘴露出了獠牙……");
            amt = _bite_amt();

            vampiric_drain_hack = TRUE;
            if (project(0, 0, y, x, amt, GF_OLD_DRAIN, PROJECT_STOP | PROJECT_KILL | PROJECT_THRU))
            {
                vampire_feed(amt);
            }
            else
                msg_print("呸。味道真恶心。");
            vampiric_drain_hack = FALSE;
        }
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MIN(_bite_amt() / 10, 29));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
static int _gaze_power(void)
{
    int power = p_ptr->lev;
    if (p_ptr->lev > 40)
        power += p_ptr->lev - 40;
    power += adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1;
    return power;
}
void _gaze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血鬼凝视");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试支配一个有智能的敌人，导致其震慑、困惑、恐惧，甚至是被奴役。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(_gaze_power()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_DOMINATION, dir, _gaze_power(), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _grasp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血鬼之握");
        break;
    case SPELL_DESC:
        var_set_string(res, "将目标生物拉向你。");
        break;
    case SPELL_CAST:
    {
        int           m_idx;
        bool          fear = FALSE;
        monster_type *m_ptr;
        char m_name[MAX_NLEN];

        var_set_bool(res, FALSE);

        if (!target_set(TARGET_KILL)) break;
        if (!cave[target_row][target_col].m_idx) break;
        if (!player_has_los_bold(target_row, target_col)) break;
        if (!projectable(py, px, target_row, target_col)) break;
        

        var_set_bool(res, TRUE);

        m_idx = cave[target_row][target_col].m_idx;
        m_ptr = &m_list[m_idx];
        monster_desc(m_name, m_ptr, 0);
        if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
        msg_format("你抓住了 %s。", m_name);
        teleport_monster_to(m_idx, py, px, 100, TELEPORT_PASSIVE);
        mon_take_hit(m_idx, damroll(10, 10), DAM_TYPE_MELEE, &fear, extract_note_dies(real_r_ptr(m_ptr)));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void equip_shuffle(cptr tag)
{
    slot_t slot, equip_slot;
    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = pack_obj(slot);
        cptr    inscription;

        if (!obj) continue;
        if (!obj->inscription) continue;
        
        inscription = quark_str(obj->inscription);
        if (!strstr(inscription, tag)) continue;
        
        equip_slot = equip_first_empty_slot(obj);
        if (equip_slot)
        {
            equip_wield(obj, equip_slot);
            obj_release(obj, OBJ_RELEASE_QUIET);
        }
    }
}

static void _set_mimic_form(int which)
{
    p_ptr->mimic_form = which;
    equip_on_change_race();

    if (p_ptr->action == ACTION_QUICK_WALK || p_ptr->action == ACTION_STALK) /* Wolf form ... */
        set_action(ACTION_NONE);

    p_ptr->redraw |= PR_BASIC | PR_STATUS | PR_MAP | PR_EQUIPPY;
    p_ptr->update |= PU_BONUS | PU_HP;
    handle_stuff();
}

static void _polymorph_undo_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变回吸血鬼");
        break;
    case SPELL_DESC:
        var_set_string(res, "你停止维持当前的形态并变回你自然的吸血鬼本体。");
        break;
    case SPELL_CAST:
        _set_mimic_form(MIMIC_NONE);
        equip_shuffle("@vampire");
        msg_print("你变回了自然的形态。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _polymorph_bat_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "化身蝙蝠");
        break;
    case SPELL_DESC:
        var_set_string(res, "你化身为一只巨型蝙蝠。这赋予了你惊人的速度、潜行能力和感官敏锐度，但也让你变得极度脆弱。此外，蝙蝠的装备选择非常受限！");
        break;
    case SPELL_CAST:
        _set_mimic_form(MIMIC_BAT);
        equip_shuffle("@bat");
        msg_print("你变成了一只吸血蝙蝠！");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _polymorph_mist_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "化身薄雾");
        break;
    case SPELL_DESC:
        var_set_string(res, "你失去了实体形态，化作一团有知觉的邪恶薄雾！");
        break;
    case SPELL_CAST:
        _set_mimic_form(MIMIC_MIST);
        equip_shuffle("@mist");
        msg_print("你变成了吸血鬼薄雾！");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _polymorph_wolf_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "化身巨狼");
        break;
    case SPELL_DESC:
        var_set_string(res, "你化身为一只饥渴地寻找猎物的巨狼。");
        break;
    case SPELL_CAST:
        _set_mimic_form(MIMIC_WOLF);
        equip_shuffle("@wolf");
        msg_print("你变成了一只恐狼！");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _repose_of_the_dead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "死者的安息");
        break;
    case SPELL_DESC:
        var_set_string(res, "像死人一样沉睡几个回合，在此期间除了死亡外没有什么能唤醒你。当（如果？）你醒来时，你将会完全恢复精力！");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!get_check("你将进入深深的沉睡中。你确定吗？")) return;
        repose_of_the_dead = TRUE;
        set_paralyzed(4 + randint1(4), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _get_spells[] =
{
    {  2,  1, 30, _bite_spell },
    {  5,  3, 30, detect_life_spell },
    {  7,  4, 30, _polymorph_bat_spell },
    { 11,  7, 35, _polymorph_wolf_spell },
    { 15, 12, 40, _gaze_spell },
    { 20, 15, 40, create_darkness_spell },
    { 25,  7, 40, nether_bolt_spell },       /* Master Vampire */
    { 25, 10, 50, mind_blast_spell },
    { 25, 20, 50, _polymorph_mist_spell },
    { 35, 25, 50, nether_ball_spell },       /* Vampire Lord */
    { 35, 30, 60, _grasp_spell },
    { 40, 50, 70, _repose_of_the_dead_spell },
    { 45, 50, 80, darkness_storm_II_spell }, /* Elder Vampire */
    { -1, -1, -1, NULL}
};

static caster_info * _caster_info(void) 
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "暗黑力量";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

/******************************************************************************
 * Bonuses (... and Penalties!)
 ******************************************************************************/
static int _light_penalty = 0;

static void _calc_bonuses(void) 
{
    p_ptr->align -= 200;

    res_add(RES_DARK);
    res_add(RES_NETHER);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add_vuln(RES_LITE);
    p_ptr->hold_life++;
    p_ptr->see_nocto = TRUE;

    if (p_ptr->lev >= 35)
    {
        res_add(RES_DARK);
        p_ptr->levitation = TRUE;
        p_ptr->pspeed += 1;
        p_ptr->regen += 100;
    }

    if (p_ptr->lev >= 45)
    {
        res_add_immune(RES_DARK);
        p_ptr->pspeed += 2;
    }

    if (_light_penalty)
    {
        p_ptr->to_a -= 5*_light_penalty;
        p_ptr->dis_to_a -= 5*_light_penalty;

        p_ptr->life -= 3*_light_penalty;
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (_light_penalty)
    {
        info_ptr->dis_to_h -= 3*_light_penalty;
        info_ptr->to_h -= 3*_light_penalty;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_VULN_LITE);

    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_NIGHT_VISION);
    if (p_ptr->lev >= 35)
    {
        add_flag(flgs, OF_LEVITATION);
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_REGEN);
    }
    if (p_ptr->lev >= 45)
        add_flag(flgs, OF_IM_DARK);
}

static void _move_player(void)
{
    vampire_check_light_status();
}

/******************************************************************************
 * Public
 ******************************************************************************/
race_t *mon_vampire_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;
    static cptr titles[4] =  {"吸血鬼", "吸血鬼大师", "吸血鬼领主", "远古吸血鬼"};    
    int         rank = 0;

    if (p_ptr->lev >= 25) rank++;
    if (p_ptr->lev >= 35) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  37,  36,   0,  32,  25,  60,  35};
    skills_t xs = {  7,  12,  10,   0,   0,   0,  21,  11};

        me.name = "吸血鬼";
        me.desc = _desc;

        me.skills = bs;
        me.extra_skills = xs;

        me.base_hp = 20;
        me.exp = 250;
        me.infra = 5;
        me.shop_adjust = 130;

        me.birth = _birth;
        me.gain_level = _gain_level;
        me.move_player = _move_player;

        me.get_spells = _get_spells;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.get_flags = _get_flags;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_IS_MONSTER | RACE_NIGHT_START;
        me.pseudo_class_idx = CLASS_ROGUE;

        me.boss_r_idx = MON_VLAD;

        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  2 + rank;
    me.stats[A_INT] =  1;
    me.stats[A_WIS] = -1 - rank;
    me.stats[A_DEX] =  0 + rank;
    me.stats[A_CON] = -2;
    me.stats[A_CHR] =  1 + 3*rank/2;
    me.life = 90 + 3*rank;

    me.skills.stl = 7 + 4*rank/3; /* 7, 8, 9, 11 */

    me.equip_template = mon_get_equip_template();

    if (birth_hack || spoiler_hack)
    {
        me.subname = NULL;
        me.subdesc = NULL;
    }
    return &me;
}

void vampire_feed(int amt)
{
    int food;
    int div = 4;

    if (prace_is_(MIMIC_BAT))
        div = 16;

    if (p_ptr->food < PY_FOOD_FULL)
        hp_player(amt);
    else
        msg_print("你并不饿。");

    /* Experimental: Scale the feeding asymptotically. Historically, vampiric feeding
        was too slow in the early game (low damage) hence tedious. But by the end game,
        a mere two bites would fill the vampire, rendering the talent rather useless. */
    if (p_ptr->food < PY_FOOD_VAMP_MAX)
        food = p_ptr->food + (PY_FOOD_VAMP_MAX - p_ptr->food) / div;
    /* Exceeding PY_FOOD_VAMP_MAX is unlikely, but possible (eg. eating rations of food?!) */
    else if (p_ptr->food < PY_FOOD_MAX)
        food = p_ptr->food + (PY_FOOD_MAX - p_ptr->food) / div;
    else
        food = p_ptr->food + amt;

    assert(food >= p_ptr->food);
    set_food(food);
}

void vampire_check_light_status(void)
{
    static int _last_light_penalty = -1;

    if ((cave[py][px].info & (CAVE_GLOW | CAVE_MNDK)) == CAVE_GLOW)
    {
        _light_penalty = 1;
        if (!dun_level && is_daytime())
            _light_penalty++;
        if (res_pct(RES_LITE) < 0)
            _light_penalty++;
    }
    else
        _light_penalty = 0;

    if (_light_penalty != _last_light_penalty)
    {
        _last_light_penalty = _light_penalty;
        if (_light_penalty)
        {
            int n = _light_penalty * _light_penalty * _light_penalty * MAX(1, dun_level/5);
            if (!fear_save_p(n))
            {
                msg_print("你害怕光芒！");
                fear_add_p(FEAR_SCARED);
            }
        }
        p_ptr->update |= PU_BONUS;
        p_ptr->redraw |= PR_STATUS;
    }
}

void vampire_take_light_damage(int amt)
{
    if (!fear_save_p(amt))
    {
        msg_print("你害怕光芒！");
        fear_add_p(FEAR_SCARED);
    }

    if (randint1(p_ptr->chp) < amt && !res_save_default(RES_LITE))
    {
        int k = 0;
        cptr act = NULL;

        switch (randint1(12))
        {
        case 1: case 2: case 3: case 4: case 5:
            msg_print("你感到自己的不死之力在衰退。");
            lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
            break;

        case 6: case 7: case 8: case 9:
            switch (randint1(6))
            {
                case 1: k = A_STR; act = "strong"; break;
                case 2: k = A_INT; act = "bright"; break;
                case 3: k = A_WIS; act = "wise"; break;
                case 4: k = A_DEX; act = "agile"; break;
                case 5: k = A_CON; act = "hale"; break;
                case 6: k = A_CHR; act = "confident"; break;
            }
            msg_format("你不如以前那么%s了。", act);
            p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
            if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
            break;

        case 10:
            msg_print("你不如以前那么强大了。");
            for (k = 0; k < 6; k++)
            {
                p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 7) / 8;
                if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
            }
            break;

        case 11: case 12:
            if (disenchant_player())
                msg_print("你感到变得虚弱了！");
            break;
        }

        p_ptr->update |= PU_BONUS;
    }
}

void vampire_take_dark_damage(int amt)
{
    if (randint1(p_ptr->chp) < amt)
    {
        /* TODO */
    }
}

/****************************************************************
 * Vampire Shapeshifting (Bat, Mist, Wolf, ...)
 *
 * The vampire can maintain their form for as long as they like
 * since equipment juggling is rather tedious. However, they can
 * only feed as a bat or as a vampire, so they will quickly grow
 * hungry for fresh blood!
 ****************************************************************/
static spell_info _mimic_get_spells[] =
{
    { 1,  0,  0, _polymorph_undo_spell }, 
    {-1, -1, -1, NULL }
};

/****************************************************************
 * Bat
 ****************************************************************/
static void _bat_calc_innate_attacks(void) 
{
    innate_attack_t    a = {0};

    a.dd = 1 + p_ptr->lev/12;
    a.ds = 4 + p_ptr->lev/15;
    a.weight = 50;
    a.to_h = p_ptr->lev/5;

    a.effect[0] = GF_OLD_DRAIN;
    calc_innate_blows(&a, 400);

    a.msg = "You bite.";
    a.name = "撕咬";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}
static void _bat_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->see_inv++;
    p_ptr->regen += 100;
    res_add(RES_DARK);
    res_add(RES_COLD);
    res_add(RES_POIS);
    p_ptr->see_nocto = TRUE;
    p_ptr->pspeed += 5 + p_ptr->lev * 3 / 10;
    p_ptr->hold_life++;
}
static void _bat_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_REGEN);
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_NIGHT_VISION);
}
race_t *bat_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  45,  38,  10,  24,  16,  48,  30 };
    skills_t xs = { 12,  18,  11,   1,   0,   0,  13,  10 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "吸血蝙蝠";
        me.desc = "";

        me.stats[A_STR] = -3;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  4;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] = -3;
        
        me.life = 75;
        me.base_hp = 10;
        me.exp = 75;
        me.infra = 10;
        me.shop_adjust = 120;

        me.get_spells = _mimic_get_spells;
        me.calc_innate_attacks = _bat_calc_innate_attacks;
        me.calc_bonuses = _bat_calc_bonuses;
        me.get_flags = _bat_get_flags;
        me.caster_info = _caster_info;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_IS_MONSTER | RACE_NIGHT_START;

        me.equip_template = &b_info[r_info[MON_VAMPIRE_BAT].body.body_idx];
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Mist
 ****************************************************************/
static void _mist_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->pass_wall = TRUE;
    p_ptr->no_passwall_dam = TRUE;
    p_ptr->see_inv++;
    p_ptr->see_nocto = TRUE;
    p_ptr->hold_life++;

    res_add(RES_ACID);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add(RES_NETHER);

    p_ptr->magic_resistance = 50;
}
static void _mist_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_HOLD_LIFE);

    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_ACID);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_NIGHT_VISION);

    add_flag(flgs, OF_MAGIC_RESISTANCE);
}
race_t *mist_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  20,  40,  10,  10,   7,  0,  0};
    skills_t xs = {  6,   7,  10,   1,   0,   0,  0,  0};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "吸血鬼薄雾";
        me.desc = "你是一团邪恶且有感知的薄雾。因此你是无实体的，无法直接攻击敌人。相反，你对物理伤害有抗性，并且可以穿墙而过。也许，在呈现这个形态时你该赶紧逃跑。";

        me.stats[A_STR] = -3;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] = -3;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] = -3;    

        me.life = 80;
        me.base_hp = 15;
        me.exp = 75;
        me.infra = 10;
        me.shop_adjust = 130;

        me.get_spells = _mimic_get_spells;
        me.calc_bonuses = _mist_calc_bonuses;
        me.get_flags = _mist_get_flags;
        me.caster_info = _caster_info;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_IS_MONSTER | RACE_NIGHT_START;

        me.equip_template = &b_info[r_info[MON_VAMPIRIC_MIST].body.body_idx];
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Wolf
 ****************************************************************/
static power_info _wolf_get_powers[] =
{
    { A_DEX, {  1,  1, 30, hound_sniff_spell } },
    { A_DEX, { 10,  0,  0, hound_stalk_spell}},
    { A_DEX, { 15,  0,  0, hound_run_spell}},
    { A_DEX, { 20, 10, 30, hound_leap_spell}},
    {    -1, { -1, -1, -1, NULL}}
};

static void _wolf_calc_bonuses(void)
{
    p_ptr->see_nocto = TRUE;
    p_ptr->pspeed += 2 + p_ptr->lev / 10;
}
static void _wolf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
    add_flag(flgs, OF_NIGHT_VISION);
}
race_t *wolf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  20,  31,   4,  20,  15,  56,  30};
    skills_t xs = {  8,   8,  10,   1,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "恐狼";
        me.desc = "";

        me.life = 100;
        me.base_hp = 22;
        me.exp = 120;
        me.infra = 5;
        me.shop_adjust = 115;

        me.get_spells = _mimic_get_spells;
        me.get_powers = _wolf_get_powers;
        me.calc_innate_attacks = hound_calc_innate_attacks;
        me.calc_bonuses = _wolf_calc_bonuses;
        me.get_flags = _wolf_get_flags;
        me.caster_info = _caster_info;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_IS_MONSTER | RACE_NIGHT_START;

        me.equip_template = &b_info[73];
        init = TRUE;
    }
    me.stats[A_STR] =  1 + p_ptr->lev/12;
    me.stats[A_INT] = -3;
    me.stats[A_WIS] = -5;
    me.stats[A_DEX] =  2 + p_ptr->lev/15;
    me.stats[A_CON] =  1 + p_ptr->lev/15;
    me.stats[A_CHR] =  0 + p_ptr->lev/25;

    return &me;
}

