#include "angband.h"

static int _force_boost(void) { return p_ptr->magic_num1[0]; }

static void _small_force_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "小型原力弹");
        break;
    case SPELL_DESC:
    {        
        var_set_string(res, "发射一颗非常小的能量弹。");
        break;
    }
    case SPELL_INFO:
    {
        int dice = 3 + ((p_ptr->lev - 1) / 5) + _force_boost()/ 12;
        int sides = 4;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 3 + ((p_ptr->lev - 1) / 5) + _force_boost()/ 12;
            int sides = 4;
            fire_ball(
                GF_MISSILE,
                dir,
                spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                0
            );
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _flying_technique_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "舞空术");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得漂浮能力。");
        break;
    case SPELL_CAST:
    {
        set_tim_levitation(spell_power(randint1(30) + 30 + _force_boost() / 5), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _kamehameha_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "龟派气功");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道短程能量光束。");
        break;
    case SPELL_INFO:
    {
        int dice = 5 + ((p_ptr->lev - 1) / 5) + _force_boost() / 10;
        int sides = 5;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        project_length = p_ptr->lev / 8 + 3;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 5 + ((p_ptr->lev - 1) / 5) + _force_boost() / 10;
            int sides = 5;
            fire_beam(
                GF_MISSILE,
                dir,
                spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
            );
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _magic_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法抗性");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得魔法抗性。");
        break;
    case SPELL_CAST:
    {
        int dur = randint1(20) + 20 + _force_boost() / 5;
        set_resist_magic(spell_power(dur), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _improve_force_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "强化原力");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时强化精神能量。被强化的精神能量会变得越来越强大或持续时间更长。强化次数过多会导致精神能量不可控制地爆炸。");
        break;
    case SPELL_CAST:
    {
        msg_print("你强化了原力。");
        p_ptr->magic_num1[0] += (70 + p_ptr->lev);
        p_ptr->update |= (PU_BONUS);
        if (randint1(p_ptr->magic_num1[0]) > (p_ptr->lev * 4 + 120))
        {
            msg_print("原力爆炸了！");
            fire_ball(GF_MANA, 0, p_ptr->magic_num1[0] / 2, 10);
            take_hit(DAMAGE_LOSELIFE, p_ptr->magic_num1[0] / 2, "原力爆炸");
            p_ptr->magic_num1[0] = 0;
            p_ptr->update |= (PU_BONUS);
            var_set_bool(res, FALSE); /* no energy consumed?? */
        }
        else var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        int j;
        for (j = 0; j < p_ptr->magic_num1[0] / 50; j++)
            n += (j+1) * 3 / 2;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _aura_of_force_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "原力光环");
        break;
    case SPELL_DESC:
        var_set_string(res, "产生一个光环，在一段时间内对所有攻击你的怪物造成伤害。");
        break;
    case SPELL_CAST:
    {
        set_tim_sh_touki(spell_power(randint1(p_ptr->lev / 2) + 15 + _force_boost() / 7), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shock_power_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "冲击力场");
        break;
    case SPELL_DESC:
        var_set_string(res, "对相邻的怪物造成伤害，并将其吹飞。");
        break;
    case SPELL_INFO:
    {
        int dice = 8 + ((p_ptr->lev - 5) / 4) + _force_boost() / 12;
        int sides = 8;
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int y, x, dam, dir;
        project_length = 1;
        /*if (!get_fire_dir(&dir)) XXX blow away won't work if a target is chosen ... dir == 5 */
        if (!get_rep_dir2(&dir))
        { 
            var_set_bool(res, FALSE);
            return;
        }

        y = py + ddy[dir];
        x = px + ddx[dir];
        dam = spell_power(damroll(8 + ((p_ptr->lev - 5) / 4) + _force_boost() / 12, 8) + p_ptr->to_d_spell);
        fire_beam(GF_MISSILE, dir, dam);
        if (cave[y][x].m_idx)
        {
            int i;
            int ty = y, tx = x;
            int oy = y, ox = x;
            int m_idx = cave[y][x].m_idx;
            monster_type *m_ptr = &m_list[m_idx];
            monster_race *r_ptr = &r_info[m_ptr->r_idx];
            char m_name[80];

            monster_desc(m_name, m_ptr, 0);

            if (randint1(r_ptr->level * 3 / 2) > randint0(dam / 2) + dam/2)
            {
                msg_format("%^s没有被吹飞。", m_name);
            }
            else
            {
                for (i = 0; i < 5; i++)
                {
                    y += ddy[dir];
                    x += ddx[dir];
                    if (cave_empty_bold(y, x))
                    {
                        ty = y;
                        tx = x;
                    }
                    else break;
                }
                if ((ty != oy) || (tx != ox))
                {
                    msg_format("你将 %s 吹飞了！", m_name);

                    cave[oy][ox].m_idx = 0;
                    cave[ty][tx].m_idx = m_idx;
                    m_ptr->fy = ty;
                    m_ptr->fx = tx;

                    update_mon(m_idx, TRUE);
                    lite_spot(oy, ox);
                    lite_spot(ty, tx);

                    if (r_ptr->flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
                        p_ptr->update |= (PU_MON_LITE);
                }
            }
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _large_force_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大型原力弹");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一颗巨大的能量弹。");
        break;
    case SPELL_INFO:
    {
        int dice = spell_power(10);
        int sides = 6;
        int base = spell_power(p_ptr->lev * 3 / 2 + _force_boost() * 3 / 5 + p_ptr->to_d_spell);
        var_set_string(res, info_damage(dice, sides, base));
        break;
    }
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dice = 10;
            int sides = 6;
            int base = p_ptr->lev * 3 / 2 + _force_boost() * 3 / 5;
            int radius = spell_power((p_ptr->lev < 30) ? 2 : 3);
            int dam = spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell);
            fire_ball(GF_MISSILE, dir, dam, radius);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_ghost_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤幽灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤幽灵。");
        break;
    case SPELL_CAST:
    {
        int i;
        bool success = FALSE;

        for (i = 0; i < 1 + _force_boost()/100; i++)
            if (summon_specific(-1, py, px, p_ptr->lev, SUMMON_PHANTOM, PM_FORCE_PET))
                success = TRUE;
        if (success)
            msg_print("“有什么吩咐，大师？”");
        else
            msg_print("什么也没发生。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _exploding_flame_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆裂火焰");
        break;
    case SPELL_DESC:
        var_set_string(res, "以你为中心产生一个巨大的火球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(100 + p_ptr->lev + _force_boost() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        fire_ball(
            GF_FIRE,
            0,
            spell_power((100 +  p_ptr->lev + _force_boost() + p_ptr->to_d_spell) * 2),
            10
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _super_kamehameha_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "超级龟派气功");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道远程、强大的能量光束。");
        break;
    case SPELL_INFO:
    {
        int dice = spell_power(10 + p_ptr->lev/2 + _force_boost()*3/10);
        int sides = 15;
        var_set_string(res, info_damage(dice, sides, spell_power(p_ptr->to_d_spell)));
        break;
    }
    case SPELL_CAST:
    {
        int dir;
        int dice = 10 + p_ptr->lev/2 + _force_boost()*3/10;
        int sides = 15;
        
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_beam(
            GF_MANA,
            dir,
            spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _light_speed_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光速移动");
        break;
    case SPELL_DESC:
        var_set_string(res, "提供极快的移动速度。");
        break;
    case SPELL_CAST:
    {
        set_lightspeed(spell_power(randint1(16) + 16 + _force_boost() / 20), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/

#define MAX_FORCETRAINER_SPELLS    15

static spell_info _spells[MAX_FORCETRAINER_SPELLS] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _small_force_ball_spell},
    { 3,   3,  30, light_area_spell},
    { 5,   6,  35, _flying_technique_spell},
    { 8,   5,  40, _kamehameha_spell},
    { 10,  7,  45, _magic_resistance_spell},
    { 13,  5,  60, _improve_force_spell},
    { 17, 17,  50, _aura_of_force_spell},
    { 20, 20,  50, _shock_power_spell},
    { 23, 18,  55, _large_force_ball_spell},
    { 25, 30,  70, dispel_magic_spell},
    { 28, 26,  50, _summon_ghost_spell},
    { 32, 35,  65, _exploding_flame_spell},
    { 38, 42,  75, _super_kamehameha_spell},
    { 44, 50,  80, _light_speed_spell},
    { -1, -1,  -1, NULL},
};

static spell_info *_get_spells(void)
{
    int i, hand;
//    int ct = 0;
    int stat_idx = p_ptr->stat_ind[A_WIS];
    int penalty1 = 0;
    int penalty2 = 0;
    static spell_info spells[MAX_FORCETRAINER_SPELLS];

    /* These penalties should only apply to Force spells ... at the moment, choice
       of a conventional spellbook and realm is handled elsewhere, but should some
       day be moved here. */
    if (heavy_armor()) 
    {
        penalty1 += 20;
        penalty2 += 5;
    }
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].icky_wield) 
        {
            penalty1 += 20;
            penalty2 += 5;
        }
        else if ( p_ptr->weapon_info[hand].wield_how != WIELD_NONE
              && !p_ptr->weapon_info[hand].bare_hands )
        {
            penalty1 += 10;
        }
    }
    for (i = 0; i < MAX_FORCETRAINER_SPELLS; i++)
    {
        spell_info *base = &_spells[i];
        spells[i].fn = base->fn;
        spells[i].level = base->level;
        spells[i].cost = base->cost;
        if (!base->fn) 
        {
            spells[i].fail = 0;
            continue;
        }
        /* The first penalty can be overcome... */ 
        spells[i].fail = calculate_fail_rate(base->level, base->fail + penalty1, stat_idx);            
        /* ...but the second penalty is just added */
        spells[i].fail += penalty2;
        if (spells[i].fail > 95) spells[i].fail = 95;
    }
    return spells;
}

static power_info _get_powers[] =
{
    { A_WIS, {15, 0, 30, clear_mind_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    p_ptr->monk_lvl = (p_ptr->lev * 94 + 50) / 100;
    if (p_ptr->lev >= 15) 
        p_ptr->clear_mind = TRUE;

    if (!(heavy_armor()))
    {
        p_ptr->pspeed += (p_ptr->lev) / 10;
        p_ptr->sh_retaliation = TRUE;
        if  (p_ptr->lev >= 25)
            p_ptr->free_act++;

    }
    monk_ac_bonus();
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (!heavy_armor())
    {
        add_flag(flgs, OF_AURA_REVENGE);
        if (p_ptr->lev >= 10)
            add_flag(flgs, OF_SPEED);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_FREE_ACT);
    }
}

static void _on_fail(const spell_info *spell)
{
    /* reset force counter for all spells *except* Improve Force */
    if (spell->fn != _improve_force_spell && p_ptr->magic_num1[0])
    {
        msg_print("你强化的原力消退了……");
        p_ptr->magic_num1[0] = 0;
        p_ptr->update |= (PU_BONUS);
    }
}

static void _on_cast(const spell_info *spell)
{
    /* reset force counter for all spells *except* Improve Force */
    if (spell->fn != _improve_force_spell && p_ptr->magic_num1[0])
    {
        p_ptr->magic_num1[0] = 0;
        p_ptr->update |= (PU_BONUS);
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "原力";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 350;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        me.on_cast = _on_cast;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CLARITY, rand_range(5, 10));
    py_birth_spellbooks();
}

static void _character_dump(doc_ptr doc)
{
    spellbook_character_dump(doc);

    doc_insert(doc, "<color:r>领域：</color> <color:B>原力</color>\n");
    py_dump_spells_aux(doc);
}

class_t *force_trainer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  34,  38,   4,  32,  24,  50,  40 };
    skills_t xs = { 10,  11,  11,   0,   0,   0,  14,  15 };

        me.name = "原力行者";
        me.desc = "原力行者是精神原力的大师。他们倾向于不使用武器也不穿戴护甲进行战斗。他们的肉搏能力不如武僧，但他们能够同时使用魔法和精神原力。挥舞武器或穿戴重型护甲会干扰原力的使用。感知是原力行者的首要属性。\n \n原力行者既使用法术书魔法，也使用被称为“原力”的特殊精神力量。他们可以从生命、自然、工匠、死亡和圣战领域中选择一个。要使用原力，你可以像选择法术书 'F' 一样选择它；这意味着你需要按 'm' 然后按 'F' 来选择原力。原力最重要的法术是“强化原力”；原力行者每次激活它时，他们的原力就会变得更强，并且他们在徒手近战中的攻击力会暂时增加。当原力行者激活其他原力法术（通常是攻击法术）时，被强化的原力可以一举释放出来。他们有一个职业能力 —— “明镜止水” —— 可以让他们迅速恢复法力。";
        
        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 135;
        me.pets = 40;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = _character_dump;
        me.known_icky_object = skills_obj_is_icky_weapon;
        init = TRUE;
    }

    return &me;
}
