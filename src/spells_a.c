#include "angband.h"

void acid_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "酸球术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在选定目标处生成一个酸液球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 35 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ACID, dir, spell_power(3*p_ptr->lev/2 + 35 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void acid_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 8;

    if (elemental_is_(ELEMENTAL_WATER)) dd = 2 + p_ptr->lev / 5 + water_flow_rate() / 10;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "酸箭术");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发酸液箭或酸液射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(damroll(dd, ds) + p_ptr->to_d_spell);
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir, dam);
        var_set_bool(res, TRUE);
        water_mana_action(2, dam * 3 / 5);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        if (elemental_is_(ELEMENTAL_WATER)) var_set_int(res, dd * 3 / 2 - 6);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void alchemy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "炼金术");
        break;
    case SPELL_DESC:
        var_set_string(res, "将有价值的物品变成金币。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了点石成金的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了点石成金的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以把普通物品变成金子。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (alchemy())
            var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_alchemy(void) { return cast_spell(alchemy_spell); }

void alter_reality_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "改变现实");
        break;
    case SPELL_DESC:
        var_set_string(res, "重置当前的地下城层。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_delay(15, 20));
        break;
    case SPELL_CAST:
        alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void amnesia_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "失忆术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图让目标怪物忘记某些事情。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int lvl = p_ptr->lev;
        if (p_ptr->lev > 40)
            lvl += (p_ptr->lev - 40) * 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        project_hook(GF_AMNESIA, dir, lvl, PROJECT_STOP | PROJECT_KILL | PROJECT_REFLECTABLE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_ray_gun_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "射线枪");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人发射不可抵抗的伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(5 + (p_ptr->lev+1) / 2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        
        msg_print("你发射了你的射线枪。");
        fire_bolt(GF_MISSILE, dir, spell_power(5 + (p_ptr->lev+1) / 2));
        
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_blaster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆能枪");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(5 + p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了你的爆能枪。");
        fire_bolt(GF_MISSILE, dir, spell_power(5 + p_ptr->lev));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_bazooka_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火箭筒");
        break;
    case SPELL_DESC:
        var_set_string(res, "向附近的怪物发射你的火箭筒。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(25 + p_ptr->lev * 2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了你的火箭筒。");
        fire_ball(GF_MISSILE, dir, spell_power(25 + p_ptr->lev * 2), 2);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_beam_cannon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光束炮");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(25 + p_ptr->lev * 3)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了一发光束炮。");
        fire_beam(GF_MISSILE, dir, spell_power(25 + p_ptr->lev * 3));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void android_rocket_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火箭发射器");
        break;
    case SPELL_DESC:
        var_set_string(res, "向你的对手发射一枚威力强大的火箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 7)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了一枚火箭。");
        fire_rocket(GF_ROCKET, dir, spell_power(p_ptr->lev * 7), 2);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void animate_dead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "操纵死尸");
        break;
    case SPELL_DESC:
        var_set_string(res, "复活附近的尸体和骷髅。并使它们成为你的宠物。");
        break;
    case SPELL_CAST:
        animate_dead(0, py, px);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void awesome_blow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "惊骇一击");
        break;
    case SPELL_DESC:
        var_set_string(res, "用单次近战攻击打击怪物。如果命中，将造成正常的近战伤害，并将怪物向后击退。");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        var_set_bool(res, FALSE);

        if (!equip_find_first(object_is_melee_weapon))
        {
            msg_print("你需要装备一把武器！");
            return;
        }
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (cave[y][x].m_idx)
        {
            py_attack(y, x, MELEE_AWESOME_BLOW);
        }
        else
        {
            msg_print("没有怪物。");
            return;
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void banish_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "放逐邪恶");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图移除单个邪恶对手。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感到一种神圣的愤怒充满了你。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感到神圣的愤怒了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以把邪恶生物直接送进地狱。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;
        cave_type *c_ptr;
        monster_type *m_ptr;
        monster_race *r_ptr;

        if (!get_rep_dir2(&dir)) 
        {
            var_set_bool(res, FALSE);
            break;
        }

        var_set_bool(res, TRUE);

        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];

        if (!c_ptr->m_idx)
        {
            msg_print("你在那里感觉不到邪恶！");
            break;
        }

        m_ptr = &m_list[c_ptr->m_idx];
        r_ptr = &r_info[m_ptr->r_idx];

        if ((r_ptr->flags3 & RF3_EVIL) &&
            !(m_ptr->mflag2 & MFLAG2_QUESTOR) &&
            !(r_ptr->flags1 & RF1_UNIQUE) &&
            !p_ptr->inside_arena && !quests_get_current() &&
            (r_ptr->level < randint1(p_ptr->lev+50)) &&
            !(m_ptr->mflag2 & MFLAG2_NOGENO))
        {
            /* Delete the monster, rather than killing it. */
            delete_monster_idx(c_ptr->m_idx);
            msg_print("邪恶生物在一阵硫磺味的烟雾中消失了！");
        }
        else
        {
            msg_print("你的祈求没有效果！");
            if (one_in_(13)) m_ptr->mflag2 |= MFLAG2_NOGENO;
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_banish_evil(void) { return cast_spell(banish_evil_spell); }

void battle_frenzy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "战斗狂怒");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时提供额外的命中和生命值加成，以及对恐惧的抗性。使你加速。但会降低防御等级(AC)。");
        break;
    case SPELL_CAST:
    {
        int b_base = spell_power(25);
        int sp_base = spell_power(p_ptr->lev / 2);
        int sp_sides = 20 + p_ptr->lev / 2;

        set_shero(randint1(b_base) + b_base, FALSE);
        set_fast(randint1(sp_sides) + sp_base, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void berserk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "进入狂暴状态，获得极大的战斗加成，但失去清醒思考的能力。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感到一阵受控的狂怒。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感到受控的狂怒了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以使自己进入狂暴状态。");
        break;
    case SPELL_CAST:
    {
        if ((elemental_is_(ELEMENTAL_WATER)) && (p_ptr->shero))
        {
            msg_print("你已经处于狂暴状态中了！");
            break;
        }
        msg_print("嗷嗷嗷！你觉得想要揍点什么。");
        set_shero(10 + randint1(p_ptr->lev), FALSE);
        var_set_bool(res, TRUE);
        water_mana_action(FALSE, 50);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_berserk(void) { return cast_spell(berserk_spell); }

void bless_spell(int cmd, variant *res)
{
    int base = spell_power(12);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "祝福术");
        break;
    case SPELL_DESC:
        var_set_string(res, "几回合内提供命中和防御等级(AC)加成。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_blessed(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void bless_weapon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "祝福武器");
        break;
    case SPELL_DESC:
        var_set_string(res, "祝福你当前的武器。");
        break;
    case SPELL_CAST:
        var_set_bool(res, bless_weapon());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void brain_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎脑术");
        break;
    case SPELL_DESC:
        var_set_string(res, "凝视单个敌人，造成伤害、困惑和震慑效果。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(12, spell_power(12), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(
            GF_BRAIN_SMASH,
            dir,
            spell_power(damroll(12, 12) + p_ptr->to_d_spell),
            0
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void breathe_disintegration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐分解");
        break;
    case SPELL_DESC:
        var_set_string(res, "一次分解喷吐。连地下城的墙壁也无法抵挡它的力量！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(MIN(p_ptr->chp / 6, 150))));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir_aux(&dir, TARGET_DISI)) return;

        stop_mouth();
        msg_print("你喷吐出分解波。");
        fire_ball(GF_DISINTEGRATE, dir, 
            spell_power(MIN(p_ptr->chp / 6, 150)), 
            (p_ptr->lev > 40 ? -3 : -2));

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void breathe_fire_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐火焰");
        break;
    case SPELL_DESC:
        var_set_string(res, "向你的对手喷吐火焰。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了喷吐火焰的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了喷吐火焰的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以喷吐火焰（伤害为 等级 * 2）。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(2 * p_ptr->lev)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+1)/2);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("你喷吐出火焰……");
            fire_ball(GF_FIRE, dir, spell_power(2 * p_ptr->lev), -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_breathe_fire_I(void) { return cast_spell(breathe_fire_I_spell); }

void breathe_fire_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐火焰");
        break;
    case SPELL_DESC:
        var_set_string(res, "向你的对手喷吐火焰。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->chp*2/5)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("你喷吐出火焰……");
            fire_ball(GF_FIRE, dir, spell_power(p_ptr->chp*2/5), -1 - (p_ptr->lev / 20));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void building_up_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "蓄力");
        break;
    case SPELL_DESC:
        var_set_string(res, "提升你的物理威力。");
        break;
    case SPELL_CAST:
        set_tim_building_up(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
