#include "angband.h"

void magic_mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法感应");
        break;
    case SPELL_DESC:
        var_set_string(res, "绘制你附近的地下城地图。");
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
bool cast_magic_mapping(void) { return cast_spell(magic_mapping_spell); }

void magic_missile_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法飞弹");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一枚无法抵抗的微弱魔法飞弹。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(3 + ((p_ptr->lev - 1) / 5)), 4, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dice = 3 + (p_ptr->lev - 1) / 5;
        int sides = 4;
        int dir = 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance() - 10,
            GF_MISSILE,
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
bool cast_magic_missile(void) { return cast_spell(magic_missile_spell); }

void mana_branding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力烙印");
        break;
    case SPELL_DESC:
        var_set_string(res, "使当前武器获得某种元素烙印。你必须装备着武器。");
        break;
    case SPELL_CAST:
        var_set_bool(res, choose_ele_attack());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_bolt_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发纯粹的法力箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 7 / 2), spell_power(p_ptr->lev + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你施放了法力箭。");
        fire_bolt(
            GF_MANA,
            dir,
            spell_power(randint1(p_ptr->lev * 7 / 2) + p_ptr->lev + p_ptr->to_d_spell)
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_bolt_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发强大的纯粹法力箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 7), spell_power(p_ptr->lev*2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你施放了法力箭。");
        fire_bolt(
            GF_MANA,
            dir,
            spell_power(randint1(p_ptr->lev * 7) + p_ptr->lev*2 + p_ptr->to_d_spell)
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_storm_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的纯粹法力球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 5 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你施放了法力风暴。");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 5 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mana_storm_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的纯粹法力球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 8 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你施放了法力风暴。");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 8 + 50 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void massacre_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大屠杀");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一种狂野、无法控制的愤怒中攻击所有相邻的怪物。");
        break;
    case SPELL_CAST:
    {
        int              dir, x, y;
        cave_type       *c_ptr;
        monster_type    *m_ptr;

        for (dir = 0; dir < 8; dir++)
        {
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            m_ptr = &m_list[c_ptr->m_idx];

            if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                py_attack(y, x, 0);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void mind_blast_spell(int cmd, variant *res)
{
    int dice = 3 + (p_ptr->lev - 1)/5;
    int sides = 3;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "心灵震爆");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图用幽能(psionic energy)轰击你的对手。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了心灵震爆的力量。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了心灵震爆的力量。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以用心灵震爆攻击你的敌人。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            msg_print("你集中精神……");
            fire_bolt(
                GF_PSI,
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
bool cast_mind_blast(void) { return cast_spell(mind_blast_spell); }

void nature_awareness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "自然感知");
        break;
    case SPELL_DESC:
        var_set_string(res, "绘制附近区域的地图。侦测所有的怪物、陷阱、门和楼梯。");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_MAP);
        detect_traps(DETECT_RAD_DEFAULT, TRUE);
        detect_doors(DETECT_RAD_DEFAULT);
        detect_stairs(DETECT_RAD_DEFAULT);
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nether_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 100 + p_ptr->to_d_spell);
    int rad = spell_power(p_ptr->lev / 20 + 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幽冥球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的幽冥法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NETHER, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void nether_bolt_spell(int cmd, variant *res)
{
    int dd = 8 + (p_ptr->lev - 5) / 4;
    int ds = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幽冥箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发幽冥箭或幽冥射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_NETHER,
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

void orb_of_entropy_spell(int cmd, variant *res)
{
    int base;

    if (p_ptr->pclass == CLASS_MAGE || p_ptr->pclass == CLASS_BLOOD_MAGE || p_ptr->pclass == CLASS_HIGH_MAGE || p_ptr->pclass == CLASS_SORCERER || p_ptr->pclass == CLASS_YELLOW_MAGE || p_ptr->pclass == CLASS_GRAY_MAGE)
        base = p_ptr->lev + p_ptr->lev / 2;
    else
        base = p_ptr->lev + p_ptr->lev / 4;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "熵之法球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个能伤害活物怪物的法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, spell_power(6), spell_power(base + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        int rad = (p_ptr->lev < 30) ? 2 : 3;

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_OLD_DRAIN, dir, spell_power(damroll(3, 6) + base + p_ptr->to_d_spell), rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void panic_hit_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "惊恐打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "攻击一个相邻的怪物并试图逃跑。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然理解了盗贼们的感受。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感到心惊肉跳了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以在打中目标后拼命逃跑。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) break;
        y = py + ddy[dir];
        x = px + ddx[dir];
        if (cave[y][x].m_idx)
        {
            py_attack(y, x, 0);
            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("你传送失败了。");
            else
                teleport_player(30, 0L);

            var_set_bool(res, TRUE);
        }
        else
        {
            msg_print("在这个方向上你没有看到任何怪物");
            msg_print(NULL);
            /* No Charge for this Action ... */
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_panic_hit(void) { return cast_spell(panic_hit_spell); }

void paralyze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "麻痹术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图冻结（麻痹）一只怪物。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stasis_monster(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void pattern_mindwalk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "漫步全知阵");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你的心智中漫步全知阵。恢复生命和属性。");
        break;
    case SPELL_CAST:
        msg_print("你在脑海中描绘出全知阵，并在其中漫步……");

        set_poisoned(0, TRUE);
        set_image(0, TRUE);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_blind(0, TRUE);
        fear_clear_p();
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

void perception_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "察觉术");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

void phase_door_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "相位之门");
        break;
    case SPELL_DESC:
        var_set_string(res, "短距离传送。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了次级传送的力量。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了次级传送的力量。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以进行短距离传送。");
        break;
    case SPELL_CAST:
        teleport_player(10, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, 30);
            break;
        } /* Fall through */
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_phase_door(void) { return cast_spell(phase_door_spell); }

void plasma_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 80 + p_ptr->to_d_spell);
    int rad = spell_power(2 + p_ptr->lev / 40);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "等离子球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个等离子球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_PLASMA, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void plasma_bolt_spell(int cmd, variant *res)
{
    int dd = 11 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "等离子箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发等离子箭或等离子射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_PLASMA,
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

void poison_dart_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毒镖");
        break;
    case SPELL_DESC:
        var_set_string(res, "向单个敌人发射一枚毒镖。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你投出了一枚毒镖。");
        fire_bolt(GF_POIS, dir, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polish_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "擦亮盾牌");
        break;
    case SPELL_DESC:
        var_set_string(res, "使你的盾牌能够反射飞弹和箭矢类法术。");
        break;
    case SPELL_CAST:
        var_set_bool(res, polish_shield());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_polish_shield(void) {    return cast_spell(polish_shield_spell); }

void polymorph_colossus_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形为巨像");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内拟态成巨像。失去原有种族的能力，并获得巨像的强大能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(15);
        set_mimic(base + randint1(base), MIMIC_COLOSSUS, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形为恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内拟态成恶魔。失去原有种族的能力，并获得恶魔的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_demonlord_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形为恶魔领主");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内拟态成恶魔领主。失去原有种族的能力，并获得恶魔领主的强大能力。连坚硬的墙壁也无法阻挡你的脚步。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(15), spell_power(15)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(15);
        set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_self_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形术");
        break;
    case SPELL_DESC:
        var_set_string(res, "使你自己变异。这可能很危险！");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的身体似乎变得不稳定了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的身体似乎恢复了稳定。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以随意变形自己。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (get_check("你将会使自己变形。确定要继续吗？"))
        {
            do_poly_self();
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_polymorph_self(void) { return cast_spell(polymorph_self_spell); }

void polymorph_vampire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形为吸血鬼");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内拟态成强大的吸血鬼。失去原有种族的能力，并获得吸血鬼的能力。");
        break;
    case SPELL_INFO:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        var_set_string(res, info_duration(base, base));
        break;
    }
    case SPELL_CAST:
    {
        int base = spell_power(10 + p_ptr->lev / 2);
        set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void power_throw_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "投掷物品");
        break;
    case SPELL_DESC:
        var_set_string(res, "用巨大的力量投掷一件物品。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你觉得用来投掷的手臂变得强壮多了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你觉得用来投掷的手臂变得虚弱多了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以用巨大的力量投掷物品。");
        break;
    case SPELL_CAST: /* This is a hack for wild-talent use only */
    {
        bool old_mt = p_ptr->mighty_throw;
        if (p_ptr->pclass != CLASS_WILD_TALENT) break;
        p_ptr->mighty_throw = TRUE;
        if (!p_ptr->wild_mode)
        {
            py_throw_t context = {0};
            py_throw(&context);
        }
        p_ptr->mighty_throw = old_mt;
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_CALC_BONUS:
        p_ptr->mighty_throw = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_power_throw(void) { return cast_spell(power_throw_spell); }

void probing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "查明附近怪物的能力、优势和弱点。");
        break;
    case SPELL_CAST:
        probing();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_probing(void) { return cast_spell(probing_spell); }

void protection_from_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "防护邪恶");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图阻止邪恶怪物攻击你。当弱小的邪恶怪物近战攻击你时，它可能会被善良的力量击退。");
        break;
    case SPELL_CAST:
        set_protevil(randint1(3 * p_ptr->lev) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_protection_from_evil(void) { return cast_spell(protection_from_evil_spell); }

void punishment_spell(int cmd, variant *res)
{
    int dd = 3 + (p_ptr->lev - 1)/5;
    int ds = 4;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "惩戒");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发闪电箭或闪电射线。");
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
            GF_ELEC,
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

void radiation_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "辐射球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个辐射球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(10), 6, spell_power(p_ptr->lev*2)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NUKE, dir, spell_power(damroll(10, 6) + p_ptr->lev*2), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void radiation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "散发辐射");
        break;
    case SPELL_DESC:
        var_set_string(res, "以你为中心生成一个巨大的辐射球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始散发出强辐射。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止散发强辐射。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以随心所欲地散发强辐射。");
        break;
    case SPELL_CAST:
        msg_print("辐射从你的体内涌出！");
        fire_ball(GF_NUKE, 0, spell_power(p_ptr->lev * 2), 3 + (p_ptr->lev / 20));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_radiation(void) { return cast_spell(radiation_spell); }

void ray_of_sunlight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "阳光射线");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道光束，对畏光怪物造成伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(6, 8, 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        lite_line(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void recall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "归还术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在城镇和地下城之间往返旅行。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你短暂地感到了一丝思乡，但很快就过去了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你短暂地感到了一丝思乡。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以往返于城镇和深渊之间。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (word_of_recall(TRUE))
            var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_recall(void) { return cast_spell(recall_spell); }

void recharging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "充能");
        break;
    case SPELL_DESC:
        if (p_ptr->prace == RACE_MON_LEPRECHAUN)
            var_set_string(res, "消耗你的金币来尝试为一个魔法装置充能。");
        else if (!p_ptr->msp)
            var_set_string(res, "消耗另一个魔法装置的能量来尝试为一个魔法装置充能。");
        else
            var_set_string(res, "消耗你的法力来尝试为一个魔法装置充能。");
        break;
    case SPELL_CAST:
        if (p_ptr->prace == RACE_MON_LEPRECHAUN)
            var_set_bool(res, recharge_from_player(2 * p_ptr->lev));
        else if (!p_ptr->msp)
            var_set_bool(res, recharge_from_device(3 * p_ptr->lev));
        else
            var_set_bool(res, recharge_from_player(3 * p_ptr->lev));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_recharging(void) { return cast_spell(recharging_spell); }

void remove_curse_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "解除诅咒");
        break;
    case SPELL_DESC:
        var_set_string(res, "解除一件物品的诅咒，以便你可以将其脱下。");
        break;
    case SPELL_CAST:
        if (remove_curse())
            msg_print("你感觉仿佛有人在庇护着你。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_curse_I(void) { return cast_spell(remove_curse_I_spell); }

void remove_curse_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "*完全解咒*");
        break;
    case SPELL_DESC:
        var_set_string(res, "解除一件物品的诅咒，以便你可以将其脱下。甚至可以解除重度诅咒物品。");
        break;
    case SPELL_CAST:
        if (remove_all_curse())
            msg_print("你感觉仿佛有人在庇护着你。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_curse_II(void) { return cast_spell(remove_curse_II_spell); }

void remove_fear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "消除恐惧");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        fear_clear_p();
        var_set_bool(res, TRUE);
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_AFRAID);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_remove_fear(void) { return cast_spell(remove_fear_spell); }

void resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抗性");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内赋予对火、冷、电、酸和毒的抗性。");
        break;
    case SPELL_CAST:
    {
        int base = spell_power(20);

        set_oppose_base(randint1(base) + base, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_elements_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗元素");
        break;
    case SPELL_DESC:
        var_set_string(res, "保护自己免受元素的摧残。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你觉得你能保护自己了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你觉得你可能变得脆弱了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以使自己强化，以抵御元素的摧残。");
        break;
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 20)
            n += 5;
        if (p_ptr->lev >= 30)
            n += 5;
        if (p_ptr->lev >= 40)
            n += 5;
        if (p_ptr->lev >= 50)
            n += 5;
        var_set_int(res, n);
        break;
    }
    case SPELL_CAST:
    {
        int num = p_ptr->lev / 10;
        int dur = randint1(20) + 20;

        if (randint0(5) < num)
        {
            set_oppose_acid(dur, FALSE);
            num--;
        }
        if (randint0(4) < num)
        {
            set_oppose_elec(dur, FALSE);
            num--;
        }
        if (randint0(3) < num)
        {
            set_oppose_fire(dur, FALSE);
            num--;
        }
        if (randint0(2) < num)
        {
            set_oppose_cold(dur, FALSE);
            num--;
        }
        if (num)
        {
            set_oppose_pois(dur, FALSE);
            num--;
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_resist_elements(void) { return cast_spell(resist_elements_spell); }

void resist_environment_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗环境");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内赋予对火、冷和电的抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_cold(randint1(base) + base, FALSE);
        set_oppose_fire(randint1(base) + base, FALSE);
        set_oppose_elec(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_fire_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗火焰");
        break;
    case SPELL_DESC:
        var_set_string(res, "在短暂时间内赋予额外的火焰抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_fire(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_heat_cold_spell(int cmd, variant *res)
{
    int base = spell_power(20);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗冷热");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予对火和冷的抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, base));
        break;
    case SPELL_CAST:
        set_oppose_cold(randint1(base) + base, FALSE);
        set_oppose_fire(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resist_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗毒素");
        break;
    case SPELL_DESC:
        var_set_string(res, "提供暂时的毒素抗性。");
        break;
    case SPELL_CAST:
        set_oppose_pois(randint1(20) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void restoration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "全面恢复");
        break;
    case SPELL_DESC:
        var_set_string(res, "恢复所有的属性和经验值。");
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

void restore_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恢复生命");
        break;
    case SPELL_DESC:
        var_set_string(res, "恢复所有失去的经验值。");
        break;
    case SPELL_CAST:
        restore_level();
        lp_player(150);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_restore_life(void) { return cast_spell(restore_life_spell); }

void rocket_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法火箭 I");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一枚魔法火箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(120 + p_ptr->lev * 2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(120 + p_ptr->lev * 2 + p_ptr->to_d_spell);
        int rad = 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了一枚火箭！");
        fire_rocket(GF_ROCKET, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void rocket_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法火箭 II");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一枚火力无与伦比的魔法火箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(500 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(500 + p_ptr->to_d_spell);
        int rad = 2;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你发射了一枚火箭！");
        fire_rocket(GF_ROCKET, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void rush_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "冲锋攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "冲向附近的怪物并用你的武器进行攻击。");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

