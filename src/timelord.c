#include "angband.h"

#include <assert.h>

/****************************************************************
 * Private Helpers
 ****************************************************************/

/* Finding what monster to evolve into is trivial, since the monster_race type
   keeps a pointer in that direction. However, we would like to reverse evolution
   turning harder monsters into easier ones. This fn will scan the monster race
   table looking for a monster that evolves into this one. Of course, we assume
   there is at most one such race to be found (Not True!)
   Returns 0 if no such race can be found.
*/
static int _find_evolution_idx(int r_idx)
{
    monster_race *r_ptr;

    if (r_idx <= 0) return 0;
    r_ptr = &r_info[r_idx];
    return r_ptr->next_r_idx;
}

static int _find_devolution_idx(int r_idx)
{
    int i;

    if (r_idx <= 0) return 0;

    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (r_ptr->next_r_idx == r_idx)
            return i;
    }

    return 0;
}

/*  Evolve or Devolve a Monster. I spiked this from monster_gain_exp() in melee2.c without
    any great understanding on my part.
    UPDATE: Use this for polymorph monster as well (spells3.c).
*/
void mon_change_race(mon_ptr mon, int new_r_idx, cptr verb)
{
    char m_name[80], new_name[80];
    int old_hp, old_maxhp, old_r_idx;
    byte old_sub_align;
    monster_race *race;

    assert(mon);
    if (new_r_idx <= 0) return;
    if (!quest_allow_poly(mon)) return;
    if (p_ptr->inside_arena) return;

    old_hp = mon->hp;
    old_maxhp = mon->max_maxhp;
    old_r_idx = mon->r_idx;
    old_sub_align = mon->sub_align;

    inc_cur_num(mon, -1);

    monster_desc(m_name, mon, 0);
    mon->r_idx = new_r_idx;
    mon->drop_ct = get_monster_drop_ct(mon);

    inc_cur_num(mon, 1);

    mon->ap_r_idx = mon->r_idx;
    race = mon_race(mon);

    if (race->flags1 & RF1_FORCE_MAXHP)
    {
        mon->max_maxhp = maxroll(race->hdice, race->hside);
    }
    else
    {
        mon->max_maxhp = damroll(race->hdice, race->hside);
    }
    if (ironman_nightmare)
    {
        u32b hp = mon->max_maxhp * 2L;

        mon->max_maxhp = (s16b)MIN(30000, hp);
    }
    mon->maxhp = mon->max_maxhp;
    mon->hp = old_hp * mon->maxhp / old_maxhp;

    mon->mspeed = get_mspeed(race);

    if (!is_pet(mon) && !(race->flags3 & (RF3_EVIL | RF3_GOOD)))
        mon->sub_align = old_sub_align;
    else
    {
        mon->sub_align = SUB_ALIGN_NEUTRAL;
        if (race->flags3 & RF3_EVIL) mon->sub_align |= SUB_ALIGN_EVIL;
        if (race->flags3 & RF3_GOOD) mon->sub_align |= SUB_ALIGN_GOOD;
    }

    mon->exp = 0;

    if (is_pet(mon) || mon->ml)
    {
        if (!ignore_unview || player_can_see_bold(mon->fy, mon->fx))
        {
            if (p_ptr->image)
            {
                monster_race *hallu_race;
                cptr hallu_name;
                do
                {
                    hallu_race = &r_info[randint1(max_r_idx - 1)];
                }
                while (!hallu_race->name || (hallu_race->flags1 & RF1_UNIQUE));
                hallu_name = r_name + hallu_race->name;
                cmsg_format(TERM_L_BLUE, "%^s%s成了%.0s%s。", m_name, verb, is_a_vowel(hallu_name[0]) ? "一只" : "a", r_name + hallu_race->name);
            }
            else if (mon->nickname)
            {
                cptr my_desc = r_name + race->name; /* hack - no monster_desc() flags fully suppress the nickname */
                cmsg_format(TERM_L_BLUE, "%^s%s成了%.0s%s。", m_name, verb, is_a_vowel(my_desc[0]) ? "一只" : "a", my_desc);
            }
            else
            {
                monster_desc(new_name, mon, MD_INDEF_VISIBLE);
                cmsg_format(TERM_L_BLUE, "%^s%s成了%s。", m_name, verb, new_name);
                if (race->r_sights < MAX_SHORT) race->r_sights++;
            }
        }
        if (!p_ptr->image) r_info[old_r_idx].r_xtra1 |= MR1_SINKA;

        /* Now you feel very close to this pet. */
        mon_set_parent(mon, 0);
    }

    update_mon(mon->id, FALSE);
    lite_spot(mon->fy, mon->fx);

    p_ptr->window |= (PW_MONSTER_LIST);
}

static bool _monster_save(monster_race* r_ptr, int power)
{
    if (r_ptr->flagsr & RFR_RES_ALL)
        return TRUE;
    else if (r_ptr->flags1 & RF1_UNIQUE)
        return r_ptr->level > randint1(2*power/3);
    else
        return r_ptr->level > randint1(power);
}

bool devolve_monster(int m_idx, bool msg)
{
    monster_type* m_ptr = &m_list[m_idx];
    monster_race *r_ptr;
    int r_idx = real_r_idx(m_ptr);
    char m_name[MAX_NLEN];

    if (r_idx <= 0) return FALSE;

    r_ptr = &r_info[r_idx];    /* We'll use the current race for a saving throw */
    r_idx = _find_devolution_idx(r_idx);
    monster_desc(m_name, m_ptr, 0);

    if (r_idx <= 0)
    {
        if (msg)
            msg_format("%^s太原始了，无法进一步退化。", m_name);
        return FALSE;
    }

    if (_monster_save(r_ptr, 2*p_ptr->lev))
    {
        if (msg)
            msg_format("%^s抵抗了。", m_name);
        return FALSE;
    }

    set_monster_csleep(m_idx, 0);
    mon_change_race(m_ptr, r_idx, "devolved");
    return TRUE;
}

bool evolve_monster(int m_idx, bool msg)
{
    monster_type* m_ptr = &m_list[m_idx];
    monster_race *r_ptr;
    int r_idx = real_r_idx(m_ptr);
    char m_name[MAX_NLEN];

    if (r_idx <= 0) return FALSE;
    monster_desc(m_name, m_ptr, 0);
    r_idx = _find_evolution_idx(r_idx);

    if (r_idx <= 0)
    {
        if (msg)
            msg_format("%^s已经达到了进化的完美境界。", m_name);
        return FALSE;
    }
    r_ptr = &r_info[r_idx];    /* We'll use the target race for a saving throw */
    set_monster_csleep(m_idx, 0);
    if (_monster_save(r_ptr, 2*p_ptr->lev))
    {
        if (msg)
            msg_format("%^s抵抗了。", m_name);
        return FALSE;
    }
    mon_change_race(m_ptr, r_idx, "evolved");
    return TRUE;
}

bool check_foresight(void)
{
    if (psion_check_foresight())
        return TRUE;

    if (p_ptr->tim_foresight && randint1(100) <= 25)
    {
        msg_print("你早料到了！");
        return TRUE;
    }

    return FALSE;
}

/****************************************************************
 * Private Spells
 ****************************************************************/
static void _bolt_spell(int cmd, variant *res)
{
    int dd = 3 + p_ptr->lev/4;
    int ds = 4;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人发射时间箭。基于时间的攻击可能会对怪物产生各种影响，包括减速、停滞等。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_bolt_or_beam(
            beam_chance() - 10,
            GF_TIME,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _regeneration_spell(int cmd, variant *res)
{
    int b = spell_power(80);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "再生");
        break;
    case SPELL_DESC:
        var_set_string(res, "加快你从物理伤害中恢复的速度。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(b, b));
        break;
    case SPELL_CAST:
        set_tim_regen(b + randint1(b), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _foretell_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "预言");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _quicken_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "加速");
        break;
    case SPELL_DESC:
        var_set_string(res, "在短时间内提供少量的速度加成。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(7, 7));
        break;
    case SPELL_CAST:
        set_tim_spurt(spell_power(7 + randint1(7)), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _withering_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "枯萎");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁相邻的墙壁、树木或门。");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        var_set_bool(res, TRUE);

        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (!in_bounds(y, x)) return;
        if (cave_have_flag_bold(y, x, FF_DOOR))
        {
            cave_alter_feat(y, x, FF_TUNNEL);
            if (!cave_have_flag_bold(y, x, FF_DOOR)) /* Hack: Permanent Door in Arena! */
            {
                msg_print("门枯萎消失了。");
                p_ptr->update |= (PU_FLOW);
            }
        }
        else if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            msg_print("墙壁化为尘土。");

            p_ptr->update |= (PU_FLOW);
        }
        else if (cave_have_flag_bold(y, x, FF_TREE))
        {
            cave_set_feat(y, x, one_in_(3) ? feat_brake : feat_grass);
            msg_print("树木枯萎而死。");
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _blast_spell(int cmd, variant *res)
{
    int dam = spell_power(3*p_ptr->lev/2 + 15 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间爆破");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人发射时间爆破。基于时间的攻击可能会对怪物产生各种影响，包括减速、停滞等。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_TIME, dir, dam, 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _back_to_origins_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "退化术");
        break;
    case SPELL_DESC:
        var_set_string(res, "消除怪物的后代。");
        break;
    case SPELL_CAST:
    {
        int i, ct;

        ct = 0;
        for (i = 1; i < max_m_idx; i++)
        {
        monster_type *m_ptr = &m_list[i];
        monster_race *r_ptr;

            if (!m_ptr->r_idx) continue;
            r_ptr = real_r_ptr(m_ptr);
            if ( (r_ptr->flags2 & RF2_MULTIPLY)
                && r_ptr->cur_num > 1  /* shouldn't this be 2 ... well, breeding in *band has never been biologically accurate */
                && !_monster_save(r_ptr, 3*p_ptr->lev) )
            {
                delete_monster_idx(i);
                ct++;
            }
        }

        if (ct > 0)
            msg_print("你感觉当地的怪物种群退化到了早期的状态。");
        else
            msg_print("你感觉当地的怪物种群很稳定。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _haste_spell(int cmd, variant *res)
{
    int base = spell_power(p_ptr->lev);
    int sides = spell_power(20 + p_ptr->lev);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "急速");
        break;
    case SPELL_DESC:
        var_set_string(res, "你获得了暂时的速度加成。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, sides));
        break;
    case SPELL_CAST:
        set_fast(base + randint1(sides), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wave_spell(int cmd, variant *res)
{
    int ds = 3*p_ptr->lev/2;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间波");
        break;
    case SPELL_DESC:
        var_set_string(res, "产生一道时间波，影响视线内的所有怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        project_hack(GF_TIME, spell_power(randint1(ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shield_spell(int cmd, variant *res)
{
    int b = spell_power(15);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间盾");
        break;
    case SPELL_DESC:
        var_set_string(res, "在短时间内赋予时间光环。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(b, b));
        break;
    case SPELL_CAST:
        set_tim_sh_time(b + randint1(b), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rewind_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时光倒流");
        break;
    case SPELL_DESC:
        var_set_string(res, "时间逃脱：你逃到安全的地方，但会忘记一些最近的经历。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!get_check("你将不可逆转地改变时间线。你确定吗？")) return;
        var_set_bool(res, TRUE);

        if (p_ptr->inside_arena || only_downward() || !dun_level)
        {
            msg_print("什么也没有发生。");
            return;
        }

        recall_player(1, FALSE);
        p_ptr->leaving_method = LEAVING_REWIND_TIME; /* Set after recall_player() to override LEAVING_RECALL */
        process_world_aux_movement(); /* Hack! Recall Now, Now, Now!!! */

        if (p_ptr->prace == RACE_ANDROID)
        {
            dec_stat(A_CON, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_INT, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_DEX, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_WIS, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_STR, 10, TRUE);
            if (one_in_(2)) return;
            dec_stat(A_CHR, 10, TRUE);
        }
        else
        {
            int amount = 0;

            if (p_ptr->lev < 3) return;
            amount = exp_requirement(p_ptr->lev-1);
            amount -= exp_requirement(p_ptr->lev-2);
            if (amount > 100000) amount = 100000;
            if (amount > p_ptr->max_exp) amount = p_ptr->max_exp;
            if (amount > p_ptr->exp) p_ptr->exp = 0;
            else p_ptr->exp -= amount;
            p_ptr->max_exp -= amount;
            check_experience();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int  _breath_dam(void) {
    int l = (p_ptr->lev - 30);
    return spell_power(11*p_ptr->lev/3 + l*l/4 + p_ptr->to_d_spell); /* 283 max damage ... */
}
static void _breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人喷吐时间。基于时间的攻击可能会对怪物产生各种影响，包括减速、停滞等。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_dam()));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_TIME, dir, _breath_dam(), -3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _remember_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "追忆");
        break;
    case SPELL_DESC:
        var_set_string(res, "恢复生命值和属性。");
        break;
    case SPELL_CAST:
        do_res_stat(A_STR);
        do_res_stat(A_INT);
        do_res_stat(A_WIS);
        do_res_stat(A_DEX);
        do_res_stat(A_CON);
        do_res_stat(A_CHR);
        restore_level();
        lp_player(1000);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _stasis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时间停滞");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图让视线内的所有怪物时间停滞。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(spell_power(7 * p_ptr->lev / 3)));
        break;
    case SPELL_CAST:
        stasis_monsters(spell_power(7 * p_ptr->lev / 3));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _travel_spell(int cmd, variant *res)
{
    int r = spell_power(p_ptr->lev / 2 + 10);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "时空旅行");
        break;
    case SPELL_DESC:
        var_set_string(res, "瞬间旅行到给定位置。小心别意外迷路了！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(r));
        break;
    case SPELL_CAST:
        var_set_bool(res, dimension_door(r));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _double_move_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "双重移动");
        break;
    case SPELL_DESC:
        var_set_string(res, "施放此法术后，你可以额外进行两次免费移动。好好利用它们！");
        break;
    case SPELL_CAST:
        if (p_ptr->free_turns)
        {
            msg_print("你在浪费你的免费回合！");
        }
        else
        {
            /* 3 is a bit of a hack to prevent chain casting this spell.
               See process_player in dungeon.c for details */
            p_ptr->free_turns = 3;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _foresee_spell(int cmd, variant *res)
{
    int b = spell_power(7);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "预见未来");
        break;
    case SPELL_DESC:
        var_set_string(res, "在极短的时间内，你将能够预见未来。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("持续 %d", b));
        break;
    case SPELL_CAST:
        set_tim_foresight(b, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _get_spells[] =
{
    /*lvl cst fail spell */
    {  1,  2, 30, _bolt_spell},
    {  3,  3, 40, _regeneration_spell},
    {  6,  4, 40, _foretell_spell},
    {  8,  8, 50, _quicken_spell},
    { 10,  9, 50, _withering_spell},
    { 13, 10, 50, _blast_spell},
    { 17, 12, 50, _back_to_origins_spell},
    { 23, 15, 60, _haste_spell},
    { 27, 20, 60, _wave_spell},
    { 30, 10, 60, _shield_spell},
    { 33, 50, 70, _rewind_time_spell},
    { 35, 35, 70, _breath_spell},
    { 37, 50, 70, _remember_spell},
    { 39, 30, 70, _stasis_spell},
    { 41, 20, 80, _travel_spell},
    { 45, 80, 80, _double_move_spell},
    { 49,100, 80, _foresee_spell},
    { -1, -1, -1, NULL}
};

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 30) res_add(RES_TIME);
    p_ptr->pspeed += (p_ptr->lev) / 7;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_TIME);
    if (p_ptr->lev >= 7) add_flag(flgs, OF_SPEED);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);
        if (b <= 90)
        {
        }
        else if (b <= 95)
        {
            set_fast(0, TRUE);
            set_slow(randint1(5) + 5, FALSE);
            msg_print("你感觉陷入了时间倒转！");
        }
        else if (b <= 99)
        {
            lose_exp(p_ptr->exp / 4);
            msg_print("你感觉生活的经历逐渐褪去！");
        }
        else
        {
            dec_stat(A_DEX, 10, FALSE);
            dec_stat(A_WIS, 10, FALSE);
            dec_stat(A_CON, 10, FALSE);
            dec_stat(A_STR, 10, FALSE);
            dec_stat(A_CHR, 10, FALSE);
            dec_stat(A_INT, 10, FALSE);
            msg_print("你感觉自己像刚出生的小猫一样虚弱！");
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "timecraft";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 80;
        me.encumbrance.enc_wgt = 600;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(4, 7));
}

class_t *time_lord_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  35,  35,   2,  16,   8,  48,  20 };
    skills_t xs = {  7,  11,  10,   0,   0,   0,  13,  13 };

        me.name = "时间领主";
        me.desc = "时间领主是时间魔法的大师，他们能改变时间流以获取优势。他们会随着经验增长获得新能力，并且能独特地将时间法术用于攻击怪物。这不仅能伤害敌人，还能造成从减速到失忆、从进化到退化、从虚弱到停滞等各种效果。除了时间攻击外，时间领主还能获得极快的速度，并且天生随经验变得更敏捷。传说中，时间大师甚至能预见未来，从而避开致命的攻击！\n \n时间领主相当精通魔法装置，但近战平庸，且完全不擅长箭术。在高等级时，他们会对时间效果产生抗性。时间领主的主要属性是感知。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  0;

        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 96;
        me.base_hp = 0;
        me.exp = 125;
        me.pets = 20;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.character_dump = py_dump_spells;
        init = TRUE;
    }

    return &me;
}
