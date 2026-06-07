#include "angband.h"

void _precognition_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "预知");
        break;
    case SPELL_DESC:
    {
        if (p_ptr->lev < 5)
            var_set_string(res, "探测你附近可见的怪物。");
        else if (p_ptr->lev < 15)
            var_set_string(res, "探测你附近可见的怪物、陷阱和门。");
        else if (p_ptr->lev < 20)
            var_set_string(res, "探测你附近的怪物、陷阱和门。");
        else if (p_ptr->lev < 25)
            var_set_string(res, "探测你附近的怪物、陷阱和门，并绘制附近区域的地图。");
        else if (p_ptr->lev < 30)
            var_set_string(res, "探测你附近的怪物、陷阱和门，并绘制附近区域的地图。提供临时的心灵感应(ESP)。");
        else if (p_ptr->lev < 40)
            var_set_string(res, "探测你附近的怪物、陷阱、门、楼梯和物品，并绘制附近区域的地图。提供临时的心灵感应(ESP)。");
        else if (p_ptr->lev < 45)
            var_set_string(res, "探测你附近的怪物、陷阱、门、楼梯和物品，并绘制附近区域的地图。");
        else
            var_set_string(res, "探测你附近的怪物、陷阱、门、楼梯和物品，并绘制整个楼层的地图。");
        break;
    }
    case SPELL_SPOIL_DESC:
        var_set_string(res, "探测怪物(1级)，陷阱和门(5级)，隐形怪物(15级)和物品(30级)。提供魔法测绘(20级)和心灵感应(25级)。启明整个楼层(45级)。");
        break;
    case SPELL_CAST:
    {
        int b = 0;
        if (p_ptr->lev > 44)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
        }
        else if (p_ptr->lev > 19)
            map_area(DETECT_RAD_MAP);

        if (p_ptr->lev < 30)
        {
            b = detect_monsters_normal(DETECT_RAD_DEFAULT);
            if (p_ptr->lev > 14) b |= detect_monsters_invis(DETECT_RAD_DEFAULT);
            if (p_ptr->lev > 4)  {
                b |= detect_traps(DETECT_RAD_DEFAULT, TRUE);
                b |= detect_doors(DETECT_RAD_DEFAULT);
            }
        }
        else
        {
            b = detect_all(DETECT_RAD_DEFAULT);
        }

        if ((p_ptr->lev > 24) && (p_ptr->lev < 40))
            set_tim_esp(p_ptr->lev + randint1(p_ptr->lev), FALSE);

        if (!b) msg_print("你感到很安全。");

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;

        if (p_ptr->lev >= 45)
            n += 9;
        else if (p_ptr->lev >= 30)
            n += 4;
        else if (p_ptr->lev >= 25)
            n += 3;
        else if (p_ptr->lev >= 20)
            n += 1;
        else if (p_ptr->lev >= 15)
            n += 0;
        else if (p_ptr->lev >= 5)
            n += 0;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _neural_blast_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神经爆破");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射造成灵能伤害的射线或球体。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "发射造成 (3 + (L-1)/4)d(3 + L/15) 灵能伤害的射线或球体(半径0)。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(3 + ((p_ptr->lev - 1) / 4)), 3 + p_ptr->lev / 15, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dice = 3 + ((p_ptr->lev - 1) / 4);
        int sides = (3 + p_ptr->lev / 15);
        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;

        if (randint1(100) < p_ptr->lev * 2)
            fire_beam(GF_PSI, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        else
            fire_ball(GF_PSI, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell), 0);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _minor_displacement_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= 45)
            var_set_string(res, "任意门");
        else
            var_set_string(res, "微级位移");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "微级位移");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "尝试传送到指定位置。");
        else
            var_set_string(res, "传送一小段距离。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "传送玩家(范围10)。在45级时，改为任意门(范围L/2 + 10)。");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev >= 45)
            var_set_bool(res, dimension_door(p_ptr->lev / 2 + 10));
        else
        {
            teleport_player(10, 0L);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 45)
            n += 40;
        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _major_displacement_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "宏级位移");
        break;
    case SPELL_DESC:
        var_set_string(res, "传送很长一段距离。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "传送玩家(范围L*5)。");
        break;
    case SPELL_CAST:
    {
        teleport_player(p_ptr->lev * 5, 0L);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _domination_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "支配");
        break;
    case SPELL_DESC:
        var_set_string(res, "震慑、混乱、恐吓或魅惑一只怪物。从30级起，尝试魅惑视线内的所有怪物。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "震慑、混乱、恐吓或魅惑一只怪物。或者在30级时尝试魅惑视线内的所有怪物。");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;

            fire_ball(GF_DOMINATION, dir, spell_power(p_ptr->lev), 0);
        }
        else
        {
            charm_monsters(spell_power((p_ptr->lev * 3 / 2) + 15));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _pulverise_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "念力粉碎");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个使用念力伤害怪物的球体。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "发射一个念力球体(半径0 或 (L-20)/8 + 1) (伤害 (8 + (L-5)/4)d8)。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(8 + ((p_ptr->lev - 5) / 4)), 8, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dice = 8 + ((p_ptr->lev - 5) / 4);
        int sides = 8;
        int rad = p_ptr->lev > 20 ? spell_power((p_ptr->lev - 20) / 8 + 1) : 0;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(
            GF_TELEKINESIS,
            dir,
            spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
            rad
        );

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _character_armor_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "躯体装甲");
        break;
    case SPELL_DESC:
        var_set_string(res, "使你的皮肤像石头一样坚硬，并提供对元素的临时抵抗。更高等级会提供更多的抗性。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "提供石肤术，抗酸(15级)、抗火(20级)、抗寒(25级)、抗电(30级)和抗毒(35级)。");
        break;
    case SPELL_CAST:
    {
        int dur = spell_power(p_ptr->lev + randint1(p_ptr->lev));
        set_shield(dur, FALSE);
        if (p_ptr->lev > 14) set_oppose_acid(dur, FALSE);
        if (p_ptr->lev > 19) set_oppose_fire(dur, FALSE);
        if (p_ptr->lev > 24) set_oppose_cold(dur, FALSE);
        if (p_ptr->lev > 29) set_oppose_elec(dur, FALSE);
        if (p_ptr->lev > 34) set_oppose_pois(dur, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psychometry_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "精神测量");
        break;
    case SPELL_DESC:
        var_set_string(res, "反馈关于一件物品的感觉。20级及以上时直接鉴定该物品。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "伪鉴定一件物品。在20级时，改为直接鉴定物品。");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev < 20)
            var_set_bool(res, psychometry());
        else
            var_set_bool(res, ident_spell(NULL));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _mind_wave_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "精神波");
        break;
    case SPELL_DESC:
        var_set_string(res, "低等级时，以你为中心产生一个灵能球。25级及以上时，对视线内的所有怪物造成灵能伤害。");
        break;

    case SPELL_SPOIL_DESC:
        var_set_string(res, "产生一个灵能球(半径 2 + L/10) (伤害 L*3)。在25级时，改为对视线内所有怪物造成伤害(伤害 1d(L*((L-5)/10 + 1)))。");
        break;

    case SPELL_INFO:
        if (p_ptr->lev < 25)
            var_set_string(res, format("伤害 %d", spell_power(p_ptr->lev * 3 / 2 + p_ptr->to_d_spell)));
        else
            var_set_string(res, info_damage(1, p_ptr->lev * ((p_ptr->lev - 5) / 10 + 1), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        msg_print("扭曲心智的力量从你的大脑中散发出来！");

        if (p_ptr->lev < 25)
        {
            project(0, 2 + p_ptr->lev / 10, py, px,
                        spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell), GF_PSI, PROJECT_KILL);
        }
        else
        {
            int ds = p_ptr->lev * ((p_ptr->lev - 5) / 10 + 1);
            mindblast_monsters(spell_power(randint1(ds) + p_ptr->to_d_spell));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _adrenaline_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "肾上腺素引导");
        break;
    case SPELL_DESC:
        var_set_string(res, "解除恐惧和震慑。提供英雄气概和速度。除非你已经拥有英雄气概和临时速度提升，否则稍微恢复生命值。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "解除震慑。恢复 10 + 1d(L*3/2) 点生命值。提供英雄气概和加速。");
        break;
    case SPELL_CAST:
    {
        int dur = spell_power(15 + randint1(p_ptr->lev*3/2));
        bool heal = !IS_FAST() || !IS_HERO(); /* Prevent spamming this as a weak healing spell */

        set_stun(0, TRUE);

        set_hero(dur, FALSE);
        set_fast(dur, FALSE);

        if (heal) /* Heal after granting Heroism to fill the +10 mhp */
            hp_player(p_ptr->lev);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _telekinesis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "念力");
        break;
    case SPELL_DESC:
        var_set_string(res, "将远处的物品拉到你身边。");        
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "取回附近的物品(重量 <= L*15)。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        fetch(dir, p_ptr->lev * 15, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psychic_drain_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "精神吸取");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个伤害怪物并吸收其心智力量的球体。成功的吸收也会消耗你自己的一些能量，额外消耗 0.01 到 1.50 个回合。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "吸取目标怪物(伤害 (L/2)d6)以恢复 5d(伤害)/4 点法力值。但这道法术也会额外消耗 1d150 能量。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(p_ptr->lev/2), 6, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(damroll(p_ptr->lev / 2, 6) + p_ptr->to_d_spell);
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        /* Only charge extra energy if the drain succeeded */
        if (fire_ball(GF_PSI_DRAIN, dir, dam, 0))
            p_ptr->energy_need += randint1(150);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void psycho_spear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "精神之矛");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一束纯粹的能量射线，能够穿透无敌屏障。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(p_ptr->lev * 3), spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_PSY_SPEAR, dir, spell_power(randint1(p_ptr->lev*3)+p_ptr->lev*3 + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _psycho_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "精神风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个由纯粹精神能量组成的巨大球体。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 5 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_PSI_STORM, dir, spell_power(p_ptr->lev * 5 + damroll(10, 10) + p_ptr->to_d_spell), 4);

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
static spell_info _get_spells[] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _neural_blast_spell},
    { 2,   1,  20, _precognition_spell},
    { 3,   2,  25, _minor_displacement_spell},
    { 7,   6,  35, _major_displacement_spell},
    { 9,   7,  50, _domination_spell},
    { 11,  7,  30, _pulverise_spell},
    { 13, 12,  50, _psychometry_spell},
    { 15, 12,  60, _character_armor_spell},
    { 18, 10,  45, _mind_wave_spell},
    { 23, 15,  50, _adrenaline_spell},
    { 26, 28,  60, _telekinesis_spell},
    { 28, 10,  40, _psychic_drain_spell},
    { 35, 35,  75, psycho_spear_spell},
    { 45, 50,  80, _psycho_storm_spell},
    { -1, -1,  -1, NULL}
};

static power_info _get_powers[] =
{
    { A_WIS, {15, 0, 30, clear_mind_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 10) res_add(RES_FEAR);
    if (p_ptr->lev >= 15) p_ptr->clear_mind = TRUE;
    if (p_ptr->lev >= 20) p_ptr->sustain_wis = TRUE;
    if (p_ptr->lev >= 25) p_ptr->auto_id_sp = 12;
    if (p_ptr->lev >= 30) res_add(RES_CONF);
    if (p_ptr->lev >= 40) p_ptr->telepathy = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_FEAR);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_SUST_WIS);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_CONF);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_TELEPATHY);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);

        if (b < 5)
        {
            msg_print("哦，不！你的大脑一片空白！");
            lose_all_info();
        }
        else if (b < 15)
        {
            msg_print("奇怪的幻象似乎在你眼前舞动……");
            set_image(p_ptr->image + 5 + randint1(10), FALSE);
        }
        else if (b < 45)
        {
            msg_print("你的大脑混乱了！");
            set_confused(p_ptr->confused + randint1(8), FALSE);
        }
        else if (b < 90)
        {
            set_stun(p_ptr->stun + randint1(8), FALSE);
        }
        else
        {
            msg_print("你的心智释放出不可控制的风暴力量！");

            project(PROJECT_WHO_UNCTRL_POWER, 2 + p_ptr->lev / 10, py, px, p_ptr->lev * 2,
                GF_MANA, PROJECT_JUMP | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM);
            p_ptr->csp = MAX(0, p_ptr->csp - p_ptr->lev * MAX(1, p_ptr->lev / 10));
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "mindcraft";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SMALL_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(2, 5));
}

class_t *mindcrafter_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  33,  38,   3,  22,  16,  48,  40 };
    skills_t xs = { 10,  11,  10,   0,   0,   0,  12,  17 };

        me.name = "心灵术士";
        me.desc = "心灵术士不依靠书本魔法，而是依靠心灵的力量。这些心灵力量是该职业所特有的，从超感官知觉到精神攻击，再到对他人的精神支配。大多数心灵力量在很早的时候就能获得，并随着经验的增加而变得更强。感知决定了心灵术士使用心灵力量的能力。\n \n心灵术士的战斗技能相当不错，但远非杰出；物理攻击和心灵攻击的结合通常比单一的攻击更适合他们。一个经验丰富的心灵术士很难被混乱，并且能毫不费力地探测他人的心灵。心灵术士拥有一个职业能力“清晰心智(Clear Mind)”，允许他们快速恢复法力。"; 
        me.stats[A_STR] = -1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 99;
        me.base_hp = 4;
        me.exp = 125;
        me.pets = 35;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = py_dump_spells;
        init = TRUE;
    }

    return &me;
}
