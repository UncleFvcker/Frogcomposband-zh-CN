/* File: do-spell.c */

/* Purpose: Do everything for each spell */

#include "angband.h"

/* Hack: Increase spell power! */
static int _current_realm_hack = 0;

int spell_power_aux(int pow, int bonus)
{
    return MAX(0, pow + pow*bonus/13);
}

int spell_power(int pow)
{
    int tmp = p_ptr->spell_power;
    if (p_ptr->tim_blood_rite)
        tmp += 7;
/*  if (_current_realm_hack && _current_realm_hack == p_ptr->easy_realm1)
        tmp += 2; */
    return spell_power_aux(pow, tmp);
}

int device_power_aux(int pow, int bonus)
{
    return MAX(0, pow + pow*bonus/20);
}

int device_power(int pow)
{
    return device_power_aux(pow, p_ptr->device_power);
}

int spell_cap_aux(int cap, int bonus)
{
    return MAX(0, cap + cap*bonus/20);
}

int spell_cap(int cap)
{
    return spell_cap_aux(cap, p_ptr->spell_cap);
}

/*
 * Generate dice info string such as "foo 2d10"
 */
static cptr info_string_dice(cptr str, int dice, int sides, int base)
{
    /* Fix value */
    if (!dice)
        return format("%s%d", str, base);

    /* Dice only */
    else if (!base)
        return format("%s%dd%d", str, dice, sides);

    /* Dice plus base value */
    else
        return format("%s%dd%d%+d", str, dice, sides, base);
}


/*
 * Generate damage-dice info string such as "dam 2d10"
 */
cptr info_damage(int dice, int sides, int base)
{
    return info_string_dice("dam ", dice, sides, base);
}


/*
 * Generate duration info string such as "dur 20+1d20"
 */
cptr info_duration(int base, int sides)
{
    return format("持续 %d+1d%d", base, sides);
}


/*
 * Generate range info string such as "range 5"
 */
cptr info_range(int range)
{
    return format("射程 %d", range);
}


/*
 * Generate heal info string such as "heal 2d8"
 */
cptr info_heal(int dice, int sides, int base)
{
    if ( p_ptr->pclass == CLASS_BLOOD_MAGE
      || p_ptr->pclass == CLASS_BLOOD_KNIGHT )
    {
        sides /= 2;
        base /= 2;
    }

    if ( p_ptr->prace == RACE_EINHERI
      || p_ptr->mimic_form == RACE_EINHERI )
    {
        if (dice % 2) sides /= 2;
        else dice /= 2;
        base /= 2;
    }

    return info_string_dice("heal ", dice, sides, base);
}


/*
 * Generate delay info string such as "delay 15+1d15"
 */
cptr info_delay(int base, int sides)
{
    return format("延迟 %d+1d%d", base, sides);
}


/*
 * Generate multiple-damage info string such as "dam 25 each"
 */
static cptr info_multi_damage(int dam)
{
    return format("各 %d 点伤害", dam);
}


/*
 * Generate multiple-damage-dice info string such as "dam 5d2 each"
 */
static cptr info_multi_damage_dice(int dice, int sides)
{
    return format("各 %dd%d 点伤害", dice, sides);
}


/*
 * Generate power info string such as "power 100"
 */
cptr info_power(int power)
{
    return format("强度 %d", power);
}

/*
 * Generate level info string such as "lvl 50" or "lvl 50+1d50"
 */
cptr info_level(int base, int sides)
{
    if (!sides) return format("等级 %d", base);
    else if (!base) return format("等级 1d%d", sides);
    else return format("等级 %d+1d%d", base, sides);
}


/*
 * Generate power info string such as "power 1d100"
 */
static cptr info_power_dice(int dice, int sides)
{
    return format("强度 %dd%d", dice, sides);
}


/*
 * Generate radius info string such as "rad 100"
 */
cptr info_radius(int rad)
{
    return format("半径 %d", rad);
}


/*
 * Generate weight info string such as "max wgt 15"
 */
cptr info_weight(int weight)
{
    return format("最大重量 %d", weight/10);
}

/*
 * Generate distance info string such as "dist 100"
 */

cptr info_dist(int dist)
{
    return format("距离 %d", dist);
}


/*
 * Prepare standard probability to become beam for fire_bolt_or_beam()
 */
int beam_chance(void)
{
    if (p_ptr->pclass == CLASS_MAGE || p_ptr->pclass == CLASS_BLOOD_MAGE || p_ptr->pclass == CLASS_NECROMANCER || p_ptr->pclass == CLASS_YELLOW_MAGE || p_ptr->pclass == CLASS_GRAY_MAGE)
        return p_ptr->lev;
    if (p_ptr->pclass == CLASS_HIGH_MAGE || p_ptr->pclass == CLASS_SORCERER)
        return p_ptr->lev + 10;

    return p_ptr->lev / 2;
}


/*
 * Handle summoning and failure of trump spells
 */
bool trump_summoning(int num, bool pet, int y, int x, int lev, int type, u32b mode)
{
    int plev = p_ptr->lev;

    int who = SUMMON_WHO_PLAYER;
    int i;
    bool success = FALSE;

    /* Default level */
    if (!lev) lev = spell_power(plev) + randint1(spell_power(plev * 2 / 3));

    if (pet)
    {
        /* Become pet */
        mode |= PM_FORCE_PET;

        /* Only sometimes allow unique monster */
        if (mode & PM_ALLOW_UNIQUE)
        {
            /* Forbid often */
            if (randint1(50 + plev) >= plev / 10)
                mode &= ~PM_ALLOW_UNIQUE;
        }
    }
    else
    {
        /* Prevent taming, allow unique monster */
        mode |= PM_NO_PET;
    }

    for (i = 0; i < num; i++)
    {
        if (summon_specific(who, y, x, lev, type, mode))
            success = TRUE;
    }

    if (!success)
    {
        if (p_ptr->pclass == CLASS_NECROMANCER)
            msg_print("没有人回应你邪恶的召唤。");
        else
            msg_print("没有人回应你的王牌召唤。");
    }

    return success;
}


/*
 * This spell should become more useful (more controlled) as the
 * player gains experience levels. Thus, add 1/5 of the player's
 * level to the die roll. This eliminates the worst effects later on,
 * while keeping the results quite random. It also allows some potent
 * effects only at high level.
 */
void cast_wonder(int dir)
{
    int plev = p_ptr->lev;
    int die = randint1(100) + plev / 5;
    int vir = virtue_current(VIRTUE_CHANCE);

    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    if (p_ptr->pclass == CLASS_WILD_TALENT)
        die += randint1(25 + p_ptr->lev/2);

    if (die < 26)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die > 100)
    {
        msg_print("你感觉到一股力量涌现！");
    }

    if (die < 8) clone_monster(dir);
    else if (die < 14) speed_monster(dir);
    else if (die < 26) heal_monster(dir, damroll(4, 6));
    else if (die < 31) poly_monster(dir);
    else if (die < 36)
        fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
                  damroll(3 + ((plev - 1) / 5) + p_ptr->to_d_spell, 4));
    else if (die < 41) confuse_monster(dir, plev);
    else if (die < 46) fire_ball(GF_POIS, dir, 20 + (plev / 2) + p_ptr->to_d_spell, 3);
    else if (die < 51) (void)lite_line(dir);
    else if (die < 56)
        fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
                  damroll(3 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 61)
        fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
                  damroll(5 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 66)
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
                  damroll(6 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 71)
        fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
                  damroll(8 + ((plev - 5) / 4) + p_ptr->to_d_spell, 8));
    else if (die < 76) drain_life(dir, 75 + p_ptr->to_d_spell);
    else if (die < 81) fire_ball(GF_ELEC, dir, 30 + plev / 2 + p_ptr->to_d_spell, 2);
    else if (die < 86) fire_ball(GF_ACID, dir, 40 + plev + p_ptr->to_d_spell, 2);
    else if (die < 91) fire_ball(GF_ICE, dir, 70 + plev + p_ptr->to_d_spell, 3);
    else if (die < 96) fire_ball(GF_FIRE, dir, 80 + plev + p_ptr->to_d_spell, 3);
    else if (die < 101) drain_life(dir, 100 + plev + p_ptr->to_d_spell);
    else if (die < 104)
    {
        earthquake(py, px, 12);
    }
    else if (die < 106)
    {
        (void)destroy_area(py, px, 13 + randint0(5), 2 * p_ptr->lev);
    }
    else if (die < 108)
    {
        symbol_genocide(plev+50, TRUE);
    }
    else if (die < 110) dispel_monsters(120);
    else /* RARE */
    {
        dispel_monsters(150 + p_ptr->to_d_spell);
        slow_monsters(p_ptr->lev);
        sleep_monsters(p_ptr->lev);
        hp_player(300);
    }
    if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_CHANCE);
}


static void cast_invoke_spirits(int dir)
{
    int plev = p_ptr->lev;
    int die = spell_power(randint1(100) + plev / 5);
    int vir = virtue_current(VIRTUE_CHANCE);

    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    msg_print("你呼唤死者的力量……");

    if (die < 26)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die > 100)
    {
        msg_print("你感觉到一股诡异的力量涌现！");
    }


    if (die < 8)
    {
        msg_print("糟糕！腐朽的身躯从你周围的土地中爬出！");

        (void)summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_UNDEAD, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
        virtue_add(VIRTUE_UNLIFE, 1);
    }
    else if (die < 14)
    {
        msg_print("一种不可名状的邪恶掠过你的脑海……");

        fear_add_p(FEAR_TERRIFIED);
    }
    else if (die < 26)
    {
        msg_print("你的脑海被一群喋喋不休的幽灵般的声音所侵入……");

        set_confused(p_ptr->confused + randint1(4) + 4, FALSE);
    }
    else if (die < 31)
    {
        poly_monster(dir);
    }
    else if (die < 36)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_MISSILE, dir,
                  damroll(3 + ((plev - 1) / 5) + p_ptr->to_d_spell, 4));
    }
    else if (die < 41)
    {
        confuse_monster (dir, plev);
    }
    else if (die < 46)
    {
        fire_ball(GF_POIS, dir, 20 + (plev / 2) + p_ptr->to_d_spell, 3);
    }
    else if (die < 51)
    {
        (void)lite_line(dir);
    }
    else if (die < 56)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_ELEC, dir,
                  damroll(3+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 61)
    {
        fire_bolt_or_beam(beam_chance() - 10, GF_COLD, dir,
                  damroll(5+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 66)
    {
        fire_bolt_or_beam(beam_chance(), GF_ACID, dir,
                  damroll(6+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 71)
    {
        fire_bolt_or_beam(beam_chance(), GF_FIRE, dir,
                  damroll(8+((plev-5)/4),8) + p_ptr->to_d_spell);
    }
    else if (die < 76)
    {
        drain_life(dir, 75 + p_ptr->to_d_spell);
    }
    else if (die < 81)
    {
        fire_ball(GF_ELEC, dir, 30 + plev / 2 + p_ptr->to_d_spell, 2);
    }
    else if (die < 86)
    {
        fire_ball(GF_ACID, dir, 40 + plev + p_ptr->to_d_spell, 2);
    }
    else if (die < 91)
    {
        fire_ball(GF_ICE, dir, 70 + plev + p_ptr->to_d_spell, 3);
    }
    else if (die < 96)
    {
        fire_ball(GF_FIRE, dir, 80 + plev + p_ptr->to_d_spell, 3);
    }
    else if (die < 101)
    {
        drain_life(dir, 100 + plev + p_ptr->to_d_spell);
    }
    else if (die < 104)
    {
        earthquake(py, px, 12);
    }
    else if (die < 106)
    {
        (void)destroy_area(py, px, 13 + randint0(5), 2 * p_ptr->lev);
    }
    else if (die < 108)
    {
        symbol_genocide(plev+50, TRUE);
    }
    else if (die < 110)
    {
        dispel_monsters(120 + p_ptr->to_d_spell);
    }
    else
    { /* RARE */
        dispel_monsters(150 + p_ptr->to_d_spell);
        slow_monsters(p_ptr->lev);
        sleep_monsters(p_ptr->lev);
        hp_player(300);
    }

    if (die < 31)
    {
        msg_print("阴森的声音轻笑道：“凡人，你很快就会加入我们了。”");
    }
}

void do_sneeze(void)
{
    int kiep_ct = randint1(3), y = 0, x = 0, yrk = 1000, i;
    while (yrk--)
    {
        scatter(&y, &x, py, px, 4, 0);
        if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
        if (!player_bold(y, x)) break;
    }

    for (i = 0; i < kiep_ct; i++)
    {
        disturb(0, 0);
        msg_print("阿——嚏！");
        msg_print(NULL);

        project(0, 0, y, x, p_ptr->lev / 2, GF_COLD,
            PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL);
    }
}

static void wild_magic(int spell)
{
    int counter = 0;
    int type = SUMMON_BIZARRE1 + randint0(6);

    if (type < SUMMON_BIZARRE1) type = SUMMON_BIZARRE1;
    else if (type > SUMMON_BIZARRE6) type = SUMMON_BIZARRE6;

    switch (randint1(spell) + randint1(8) + 1)
    {
    case 1:
    case 2:
    case 3:
        teleport_player(10, TELEPORT_PASSIVE);
        break;
    case 4:
    case 5:
    case 6:
        teleport_player(100, TELEPORT_PASSIVE);
        break;
    case 7:
    case 8:
        teleport_player(200, TELEPORT_PASSIVE);
        break;
    case 9:
    case 10:
    case 11:
        unlite_area(10, 3);
        break;
    case 12:
    case 13:
    case 14:
        lite_area(damroll(2, 3), 2);
        break;
    case 15:
        destroy_doors_touch();
        break;
    case 16: case 17:
        wall_breaker();
    case 18:
        sleep_monsters_touch();
        break;
    case 19:
    case 20:
        trap_creation(py, px);
        break;
    case 21:
    case 22:
        door_creation();
        break;
    case 23:
    case 24:
    case 25:
        aggravate_monsters(0);
        break;
    case 26:
        earthquake(py, px, 5);
        break;
    case 27:
    case 28:
        mut_gain_random(NULL);
        break;
    case 29:
    case 30:
        apply_disenchant(1);
        break;
    case 31:
        lose_all_info();
        break;
    case 32:
        fire_ball(GF_CHAOS, 0, spell + 5, 1 + (spell / 10));
        break;
    case 33:
        wall_stone();
        break;
    case 34:
    case 35:
        while (counter++ < 8)
        {
            (void)summon_specific(0, py, px, (dun_level * 3) / 2, type, (PM_ALLOW_GROUP | PM_NO_PET));
        }
        break;
    case 36:
    case 37:
        activate_hi_summon(py, px, FALSE);
        break;
    case 38:
        (void)summon_cyber(-1, py, px);
        break;
    default:
        {
            int count = 0;
            (void)activate_ty_curse(FALSE, &count);
            break;
        }
    }
}


void cast_shuffle(void)
{
    int plev = p_ptr->lev;
    int dir;
    int die;
    int vir = virtue_current(VIRTUE_CHANCE);
    int i;

    /* Card sharks and high mages get a level bonus */
    if ((p_ptr->pclass == CLASS_ROGUE) ||
        (p_ptr->pclass == CLASS_HIGH_MAGE) ||
        (p_ptr->pclass == CLASS_SORCERER) ||
        (p_ptr->pclass == CLASS_DISCIPLE))
        die = (randint1(110)) + plev / 5;
    else
        die = randint1(120);


    if (vir > 0)
    {
        while (randint1(400) < vir) die++;
    }
    else if (vir < 0)
    {
        while (randint1(400) < -vir) die--;
    }

    msg_print("你洗了洗牌并抽出一张牌……");

    if (die < 30)
        virtue_add(VIRTUE_CHANCE, 1);

    if (die < 7)
    {
        msg_print("糟糕！是“死神”！");

        for (i = 0; i < randint1(3); i++)
            activate_hi_summon(py, px, FALSE);
    }
    else if (die < 14)
    {
        msg_print("糟糕！是“恶魔”！");

        summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DEMON, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
    }
    else if (die < 18)
    {
        int count = 0;
        msg_print("糟糕！是“倒吊人”。");

        activate_ty_curse(FALSE, &count);
    }
    else if (die < 22)
    {
        msg_print("是“纷争之剑”。");

        aggravate_monsters(0);
    }
    else if (die < 26)
    {
        msg_print("是“愚者”。");

        do_dec_stat(A_INT);
        do_dec_stat(A_WIS);
    }
    else if (die < 30)
    {
        msg_print("这是一张奇怪怪物的图画。");

        trump_summoning(1, FALSE, py, px, (dun_level * 3 / 2), (SUMMON_BIZARRE1 + randint0(6)), PM_ALLOW_GROUP | PM_ALLOW_UNIQUE);
    }
    else if (die < 33)
    {
        msg_print("是“月亮”。");

        unlite_area(10, 3);
    }
    else if (die < 38)
    {
        msg_print("是“命运之轮”。");

        wild_magic(randint0(32));
    }
    else if (die < 40)
    {
        msg_print("这是一张传送王牌。");

        teleport_player(10, TELEPORT_PASSIVE);
    }
    else if (die < 42)
    {
        msg_print("是“正义”。");

        set_blessed(p_ptr->lev, FALSE);
    }
    else if (die < 47)
    {
        msg_print("这是一张传送王牌。");

        teleport_player(100, TELEPORT_PASSIVE);
    }
    else if (die < 52)
    {
        msg_print("这是一张传送王牌。");

        teleport_player(200, TELEPORT_PASSIVE);
    }
    else if (die < 60)
    {
        msg_print("是“高塔”。");

        wall_breaker();
    }
    else if (die < 72)
    {
        msg_print("是“节制”。");

        sleep_monsters_touch();
    }
    else if (die < 80)
    {
        msg_print("是“高塔”。");

        earthquake(py, px, 5);
    }
    else if (die < 82)
    {
        msg_print("这是一张友善怪物的图画。");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE1, 0L);
    }
    else if (die < 84)
    {
        msg_print("这是一张友善怪物的图画。");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE2, 0L);
    }
    else if (die < 86)
    {
        msg_print("这是一张友善怪物的图画。");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE4, 0L);
    }
    else if (die < 88)
    {
        msg_print("这是一张友善怪物的图画。");

        trump_summoning(1, TRUE, py, px, (dun_level * 3 / 2), SUMMON_BIZARRE5, 0L);
    }
    else if (die < 96)
    {
        msg_print("是“恋人”。");

        if (get_fire_dir(&dir))
            charm_monster(dir, MIN(p_ptr->lev, 20));
    }
    else if (die < 101)
    {
        msg_print("是“隐者”。");

        wall_stone();
    }
    else if (die < 111)
    {
        msg_print("是“审判”。");

        do_cmd_rerate(FALSE);
        mut_lose_all();
    }
    else if (die < 120)
    {
        msg_print("是“太阳”。");

        virtue_add(VIRTUE_KNOWLEDGE, 1);
        virtue_add(VIRTUE_ENLIGHTENMENT, 1);
        wiz_lite(p_ptr->tim_superstealth > 0);
    }
    else
    {
        msg_print("是“世界”。");

        if (p_ptr->exp < PY_MAX_EXP)
        {
            s32b ee = (p_ptr->exp / 25) + 1;
            if (ee > 5000) ee = 5000;
            msg_print("你感觉自己更有经验了。");

            gain_exp(ee);
        }
    }
}


/*
 * Drop 10+1d10 meteor ball at random places near the player
 */
static void cast_meteor(int dam, int rad)
{
    int i;
    int b = 10 + randint1(10);

    for (i = 0; i < b; i++)
    {
        int y = 0, x = 0;
        int count;

        for (count = 0; count <= 20; count++)
        {
            int dy, dx, d;

            x = px - 8 + randint0(17);
            y = py - 8 + randint0(17);

            dx = (px > x) ? (px - x) : (x - px);
            dy = (py > y) ? (py - y) : (y - py);

            /* Approximate distance */
            d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

            if (d >= 9) continue;

            if (!in_bounds(y, x) || !projectable(py, px, y, x)
                || !cave_have_flag_bold(y, x, FF_PROJECT)) continue;

            /* Valid position */
            break;
        }

        if (count > 20) continue;

        project(0, rad, y, x, dam, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM);
    }
}


/*
 * Drop 10+1d10 disintegration ball at random places near the target
 */
bool cast_wrath_of_the_god(int dam, int rad)
{
    int x, y, tx, ty;
    int nx, ny;
    int dir, i;
    int b = 10 + randint1(10);

    if (!get_fire_dir(&dir)) return FALSE;

    /* Use the given direction */
    tx = px + 99 * ddx[dir];
    ty = py + 99 * ddy[dir];

    /* Hack -- Use an actual "target" */
    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }

    x = px;
    y = py;

    while (1)
    {
        /* Hack -- Stop at the target */
        if ((y == ty) && (x == tx)) break;

        ny = y;
        nx = x;
        mmove2(&ny, &nx, py, px, ty, tx);

        /* Stop at maximum range */
        if (MAX_RANGE <= distance(py, px, ny, nx)) break;

        /* Stopped by walls/doors */
        if (!cave_have_flag_bold(ny, nx, FF_PROJECT)) break;

        /* Stopped by monsters */
        if ((dir != 5) && cave[ny][nx].m_idx != 0) break;

        /* Save the new location */
        x = nx;
        y = ny;
    }
    tx = x;
    ty = y;

    for (i = 0; i < b; i++)
    {
        int count = 20, d = 0;

        while (count--)
        {
            int dx, dy;

            x = tx - 5 + randint0(11);
            y = ty - 5 + randint0(11);

            dx = (tx > x) ? (tx - x) : (x - tx);
            dy = (ty > y) ? (ty - y) : (y - ty);

            /* Approximate distance */
            d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));
            /* Within the radius */
            if (d < 5) break;
        }

        if (count < 0) continue;

        /* Cannot penetrate perm walls */
        if (!in_bounds(y,x) ||
            cave_stop_disintegration(y,x) ||
            !in_disintegration_range(ty, tx, y, x))
            continue;

        project(0, rad, y, x, dam, GF_DISINTEGRATE, PROJECT_JUMP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
    }

    return TRUE;
}


/*
 * An "item_tester_hook" for offer
 */
static bool item_tester_offer(object_type *o_ptr)
{
    /* Flasks of oil are okay */
    if (o_ptr->tval != TV_CORPSE) return (FALSE);

    if (o_ptr->sval != SV_CORPSE) return (FALSE);

    if (my_strchr("pht", r_info[o_ptr->pval].d_char)) return (TRUE);

    /* Assume not okay */
    return (FALSE);
}


/*
 * Daemon spell Summon Greater Demon
 */
bool cast_summon_greater_demon(void)
{
    obj_prompt_t prompt = {0};
    int plev = p_ptr->lev;
    int summon_lev;

    prompt.prompt = "献祭哪具尸体？";
    prompt.error = "你没有什么可献祭的。";
    prompt.filter = item_tester_offer;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    summon_lev = plev * 2 / 3 + r_info[prompt.obj->pval].level;

    if (summon_specific(SUMMON_WHO_PLAYER, py, px, summon_lev, SUMMON_HI_DEMON, (PM_ALLOW_GROUP | PM_FORCE_PET)))
    {
        msg_print("这片区域弥漫着硫磺和烈火的恶臭。");
        msg_print("'您有何吩咐……主人？'");

        prompt.obj->number--;
        obj_release(prompt.obj, 0);
    }
    else
    {
        msg_print("没有高阶恶魔降临。");
    }

    return TRUE;
}



static cptr do_life_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool spoil = (mode == SPELL_SPOIL_DESC) ? TRUE : FALSE;

    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "治疗轻伤";
        if (desc) return "轻微治愈割伤并恢复少量生命值。";

        {
            int dice = 2;
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_cut(p_ptr->cut - 10, TRUE);
            }
        }
        break;

    case 1:
        if (name) return "祝福";
        if (desc) return "在几个回合内提供命中和防御(AC)加值。";

        {
            int base = spell_power(12);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_blessed(randint1(base) + base, FALSE);
            }
        }
        break;

    case 2:
        if (name) return "再生";
        if (desc) return "在一段时间内赋予生命再生能力。";

        {
            int base = spell_power(80);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_regen(base + randint1(base), FALSE);
            }
        }
        break;

    case 3:
        if (name) return "召唤光芒";
        if (desc) return "永久照亮附近区域和房间内部。";

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = spell_power(plev / 10 + 1);

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(spell_power(damroll(dice, sides)), rad);
            }
        }
        break;

    case 4:
        if (name) return "探测门与陷阱";
        if (desc) return "探测你附近的陷阱、门和楼梯。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 5:
        if (name) return "治疗中伤";
        if (desc) return "较多地治愈割伤并恢复生命值。";

        {
            int dice = 6;
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_cut((p_ptr->cut / 2) - 20, TRUE);
            }
        }
        break;
    case 6:
        if (name) return "解毒";
        if (desc) return "稍微减轻中毒程度。完全治愈低级中毒。";
        if (cast)
            set_poisoned(p_ptr->poisoned - MAX(125, p_ptr->poisoned / 3), TRUE);
        break;
    case 7:
        if (name) return "充饥";
        if (desc) return "满足饥饿。";
        if (cast)
            set_food(PY_FOOD_MAX - 1);
        break;
    case 8:
        if (name) return "移除诅咒";
        if (desc) return "解除已装备物品上的普通诅咒。";

        {
            if (cast)
            {
                if (remove_curse())
                {
                    msg_print("你感觉好像有人在守护着你。");
                }
            }
        }
        break;

    case 9:
        if (name) return "禁食";
        if (desc) return "开始宗教绝食。随着时间推移，你的神明或许会为你恢复！";
        if (spoil) return "玩家开始绝食。一旦进入饥饿状态，有小概率随机恢复一项属性，或恢复生命经验。";

        if (cast)
        {
            if (p_ptr->fasting)
            {
                msg_print("你已经在禁食了。也许你也该祈祷一下？");
                return NULL;
            }
            msg_print("你开始了禁食。");
            set_food(p_ptr->food/2);
            p_ptr->redraw |= PR_STATUS;
            p_ptr->fasting = TRUE;
        }
        break;

    case 10:
        if (name) return "治疗重伤";
        if (desc) return "极大地治愈割伤、震慑并恢复生命值。";

        {
            int dice = 12;
            int sides = 12;

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(spell_power(damroll(dice, sides)));
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 11:
        if (name) return "抵抗冷热";
        if (desc) return "赋予临时的火焰与寒冷抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_fire(dur, FALSE);
            }
        }
        break;

    case 12:
        if (name) return "感知周围";
        if (desc) return "绘制附近区域的地图。";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 13:
        if (name) return "亡灵退散";
        if (desc) return "尝试恐吓视线内的不死怪物。";

        {
            if (cast)
            {
                turn_undead();
            }
        }
        break;

    case 14:
        if (name) return "治疗";
        if (desc) return "极为强大的治疗魔法，并完全治愈割伤与震慑。";

        {
            int heal = spell_power(300);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 15:
        if (name) return "守护结界";
        if (desc) return "在你脚下的地板上设置一个结界符文。当你在结界上时怪物无法攻击你，但它们会尝试破坏结界。";

        {
            if (cast)
            {
                warding_glyph();
            }
        }
        break;

    case 16:
        if (name) return "驱除诅咒";
        if (desc) return "解除已装备物品上的普通和重度诅咒。";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("你感觉好像有人在守护着你。");
                }
            }
        }
        break;

    case 17:
        if (name) return "察觉";
        if (desc) return "鉴定一件物品。";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
                /*identify_pack();*/
            }
        }
        break;

    case 18:
        if (name) return "驱散不死生物";
        if (desc) return "对视线内的所有不死怪物造成伤害。";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
                dispel_undead(dam);
        }
        break;

    case 19:
        if (name) return "维持属性";
        if (desc) return "赋予临时的属性维持效果，持续时间取决于你的等级。";
        if (spoil) return "玩家获得最多 等级/7 项属性维持，持续 等级 回合。";

        {
            int dur = spell_power(plev);

            if (info) return info_duration(dur, 0);

            if (cast)
            {
                int num = plev / 7;

                if (randint0(7) < num)
                {
                    set_tim_hold_life(dur, FALSE);
                    num--;
                }
                if (randint0(6) < num)
                {
                    set_tim_sustain_con(dur, FALSE);
                    num--;
                }
                if (randint0(5) < num)
                {
                    set_tim_sustain_str(dur, FALSE);
                    num--;
                }
                if (randint0(4) < num)
                {
                    set_tim_sustain_int(dur, FALSE);
                    num--;
                }
                if (randint0(3) < num)
                {
                    set_tim_sustain_dex(dur, FALSE);
                    num--;
                }
                if (randint0(2) < num)
                {
                    set_tim_sustain_wis(dur, FALSE);
                    num--;
                }
                if (num)
                {
                    set_tim_sustain_chr(dur, FALSE);
                    num--;
                }

            }
        }
        break;

    case 20:
        if (name) return "治愈变异";
        if (desc) return "移除一项随机的变异。";
        if (spoil) return "移除一项随机的变异。有 1 / (100/等级) 的几率仅移除一项有害变异。";

        if (cast)
        {
            if (one_in_(100/plev))
                mut_lose_random(mut_bad_pred);
            else
                mut_lose_random(NULL);
        }
        break;

    case 21:
        if (name) return "召回之语";
        if (desc) return "将玩家从地下城召回城镇，或从城镇送回地下城的最深层。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 22:
        if (name) return "超越";
        if (desc) return "在短时间内，你受到的任何伤害都将由你的法力值吸收。";

        {
            int dur = spell_power(plev/10);

            if (info) return format("持续 %d", dur);

            if (cast)
                set_tim_transcendence(dur, FALSE);
        }
        break;

    case 23:
        if (name) return "真实结界";
        if (desc) return "在所有相邻的方格及你脚下生成结界符文。";

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cast)
            {
                warding_glyph();
                glyph_creation();
            }
        }
        break;

    case 24:
        if (name) return "绝育";
        if (desc) return "阻止当前楼层内的任何繁殖类怪物进行繁殖。";

        {
            if (cast)
            {
                num_repro += MAX_REPRO;
            }
        }
        break;

    case 25:
        if (name) return "探测";
        if (desc) return "探测你附近所有的怪物、陷阱、门、楼梯、宝藏和物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 26:
        if (name) return "湮灭不死生物";
        if (desc) return "消灭附近所有的不死怪物，这会让你筋疲力尽。强大或唯一的怪物可能能够抵抗。";

        {
            int power = spell_power(plev + 50);

            if (info) return info_power(power);

            if (cast)
            {
                mass_genocide_undead(power, TRUE);
            }
        }
        break;

    case 27:
        if (name) return "透视";
        if (desc) return "为整个地下城楼层绘制地图并将其照亮。知晓所有物品的位置。并在一段时间内赋予心灵感应。";

        {
            if (cast)
            {
                wiz_lite(p_ptr->tim_superstealth > 0);
            }
        }
        break;

    case 28:
        if (name) return "完全恢复";
        if (desc) return "恢复所有属性、生命力与经验值。";

        {
            if (cast)
            {
                do_res_stat(A_STR);
                do_res_stat(A_INT);
                do_res_stat(A_WIS);
                do_res_stat(A_DEX);
                do_res_stat(A_CON);
                do_res_stat(A_CHR);
                restore_level();
                lp_player(1000);
            }
        }
        break;

    case 29:
        if (name) return "真实治疗";
        if (desc) return "最伟大的治疗魔法。完全恢复生命值，治愈所有割伤与震慑。";
        if (spoil) return "消除割伤和震慑，并治愈玩家 2000 点生命值。";

        {
            int heal = spell_power(2000);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }
        break;

    case 30:
        if (name) return "神圣视界";
        if (desc) return "全面鉴定一件物品。";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "终极抵抗";
        if (desc) return "赋予终极抗性，以及防御(AC)和速度加值。";
        if (spoil) return "玩家获得所有抗性、光环、维持属性、行动自如(FA)、识破隐形(SI)、缓慢消化、生命再生、漂浮和反射，以及双倍基础抗性、加速和 +100 防御等级(AC)，持续 X+dX 回合（其中 X = 等级/2）。";

        {
            int base = spell_power(plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int v = randint1(base) + base;
                set_fast(v, FALSE);
                set_oppose_base(v, FALSE);
                set_ultimate_res(v, FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_sorcery_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "探测怪物";
        if (desc) return "探测你附近所有的怪物，除非它们隐形。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 1:
        if (name) return "相位门";
        if (desc) return "短距离传送。";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 2:
        if (name) return "探测门与陷阱";
        if (desc) return "探测你附近的陷阱、门和楼梯。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 3:
        if (name) return "区域照明";
        if (desc) return "永久照亮附近区域和房间内部。";

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(spell_power(damroll(dice, sides)), rad);
            }
        }
        break;

    case 4:
        if (name) return "困惑单一怪物";
        if (desc) return "尝试使一个怪物混乱。";

        {
            int power = spell_power((plev * 3) / 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                confuse_monster(dir, power);
            }
        }
        break;

    case 5:
        if (name) return "传送";
        if (desc) return "长距离传送。";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 6:
        if (name) return "催眠单一怪物";
        if (desc) return "尝试催眠一个怪物。";

        {
            int power = spell_power(plev * 3 /2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                sleep_monster(dir, power);
            }
        }
        break;

    case 7:
        if (name) return "充能";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "它尝试消耗你的血液为一件装置充能。";
            else
                return "它尝试消耗你的法力为一件装置充能。";
        }

        {
            int power = spell_power(plev * 3);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 8:
        if (name) return "魔法探知";
        if (desc) return "绘制附近区域的地图。";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 9:
        if (name) return plev < 30 ? "鉴定" : "批量鉴定";
        if (desc) return plev < 30 ? "鉴定一件物品。" : "鉴定背包中的所有物品";

        {
            if (cast)
            {
                if (plev < 30) 
                {
                    if (!ident_spell(NULL))
                        return NULL;
                }
                else
                    mass_identify(FALSE);
            }
        }
        break;

    case 10:
        if (name) return "减速单一怪物";
        if (desc) return "尝试使一个怪物减速。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                slow_monster(dir, power);
            }
        }
        break;

    case 11:
        if (plev < 35)
        {
            if (name) return "群体催眠";
            if (desc) return "尝试催眠视线内的所有怪物。";
        }
        else
        {
            if (name) return "群体静滞";
            if (desc) return "尝试定身(suspend)视线内的所有怪物。";
        }

        {
            int power = spell_power(plev * ((plev < 35) ? 10 : 7) / 3);

            if (info) return info_power(power);

            if (cast)
            {
                if (plev < 35)
                    sleep_monsters(power);
                else
                    stasis_monsters(power);
            }
        }
        break;

    case 12:
        if (name) return "传送离开";
        if (desc) return "除非被抵抗，否则将射线路径上的所有怪物传送走。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 13:
        if (name) return "自我加速";
        if (desc) return "使你加速一段时间。";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_fast(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 14:
        if (name) return "真实探测";
        if (desc) return "探测你附近所有的怪物、陷阱、门、楼梯、宝藏和物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 15:
        if (name) return "真实鉴定";
        if (desc) return "*鉴定*一件物品（全面鉴定）。";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 16:
        if (name) return "物品栏保护";
        if (desc) return "在短时间内，你背包中的物品有几率抵抗被破坏。";

        {
            int base = spell_power(30);

            if (info) return info_duration(30, base);

            if (cast)
                set_tim_inven_prot(base + randint1(base), FALSE);
        }
        break;

    case 17:
        if (name) return "制造阶梯";
        if (desc) return "生成一个向下或向上的楼梯。";

        if (cast)
            stair_creation(FALSE);
        break;

    case 18:
        if (name) return "感知心灵";
        if (desc) return "在一段时间内赋予心灵感应。";

        {
            int base = 25;
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "传送到城镇";
        if (desc) return "立刻传送到你选择的一个城镇。只能在户外使用。";

        {
            if (cast)
            {
                if (!tele_town()) return NULL;
            }
        }
        break;

    case 20:
        if (name) return "自我知识";
        if (desc) return "为你提供关于当前抗性、武器力量以及属性最大上限的有用信息。";

        {
            if (cast)
            {
                self_knowledge();
            }
        }
        break;

    case 21:
        if (name) return "传送楼层";
        if (desc) return "立刻传送到向上或向下的楼梯处。";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 22:
        if (name) return "召回之语";
        if (desc) return "将玩家从地下城召回城镇，或从城镇送回地下城的最深层。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 23:
        if (name) return "任意门";
        if (desc) return "传送到指定位置。";

        {
            int range = spell_power(plev / 2 + 10);

            if (info) return info_range(range);

            if (cast)
            {
                msg_print("你打开了一道次元门。选择一个目的地。");

                if (!dimension_door(range)) return NULL;
            }
        }
        break;

    case 24:
        if (name) return "探知";
        if (desc) return "探明(Probes)所有怪物的阵营、生命值、速度及其真实性格。";

        {
            if (cast)
            {
                probing();
            }
        }
        break;

    case 25:
        if (name) return "制造门";
        if (desc) return "在所有周围的方格上生成门。";

        if (cast)
        {
            project(0, 1, py, px, 0, GF_MAKE_DOOR, PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE);
            p_ptr->update |= (PU_FLOW);
            p_ptr->redraw |= (PR_MAP);
        }
        break;

    case 26:
        if (name) return "隔空取物";
        if (desc) return "将远处的物品拉到你身边。";

        {
            int weight = spell_power(plev * 15);

            if (info) return info_weight(weight);

            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                fetch(dir, weight, FALSE);
            }
        }
        break;

    case 27:
        if (name) return "透视";
        if (desc) return "为整个地下城楼层绘制地图并将其照亮。知晓所有物品的位置。并在一段时间内赋予心灵感应。";

        {
            int base = 25;
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);

                wiz_lite(p_ptr->tim_superstealth > 0);

                if (!p_ptr->telepathy)
                {
                    set_tim_esp(randint1(sides) + base, FALSE);
                }
            }
        }
        break;

    case 28:
        if (name) return "装置精通";
        if (desc) return "在极短的时间内，你的魔法装置威力变得更加强大。";

        {
            int base = spell_power(plev/10);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_device_power(base + randint1(base), FALSE);
        }
        break;

    case 29:
        if (name) return "炼金术";
        if (desc) return (no_selling) ? "将一件物品转化为相当于其价值 1/9 的金币。" : "将一件物品转化为相当于其价值 1/3 的金币。";

        {
            if (cast)
            {
                if (!alchemy()) return NULL;
            }
        }
        break;

    case 30:
        if (name) return "放逐";
        if (desc) return "除非被抵抗，否则将视线内所有的怪物传送走。";

        {
            int power = spell_power(plev * 4);

            if (info) return info_power(power);

            if (cast)
            {
                banish_monsters(power);
            }
        }
        break;

    case 31:
        if (name) return "无敌结界";
        if (desc) return "生成一道屏障，使你完全免疫几乎所有的伤害。当屏障破裂或持续时间结束时，会消耗你几个回合的时间（僵直）。";

        {
            int base = 4;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_invuln(spell_power(randint1(base) + base), FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_nature_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "探测生物";
        if (desc) return "探测你附近所有的怪物，除非它们隐形。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 1:
        if (name) return "闪电";
        if (desc) return "发射一道短距离的闪电射线。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;
            int range = spell_power(plev / 6 + 2);

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                project_length = range;

                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_ELEC, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 2:
        if (name) return "探测门与陷阱";
        if (desc) return "探测你附近的陷阱、门和楼梯。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 3:
        if (name) return "制造食物";
        if (desc) return "制造一份口粮。";

        {
            if (cast)
            {
                object_type forge, *q_ptr = &forge;

                msg_print("一份口粮被制造出来了。");

                /* Create the food ration */
                object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
                object_origins(q_ptr, ORIGIN_ACQUIRE);

                /* Drop the object from heaven */
                drop_near(q_ptr, -1, py, px);
            }
        }
        break;

    case 4:
        if (name) return "昼光";
        if (desc) return "永久照亮附近区域和房间内部。";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = spell_power((plev / 10) + 1);

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);

                if ( (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || p_ptr->mimic_form == MIMIC_VAMPIRE)
                  && !res_save_default(RES_LITE) )
                {
                    msg_print("昼光灼伤了你的肉体！");
                    take_hit(DAMAGE_NOESCAPE, damroll(2, 2), "日光");
                }
            }
        }
        break;

    case 5:
        if (name) return "御风而行";
        if (desc) return "赋予临时的漂浮能力。";

        {
            int dur = spell_power(30);

            if (info) return info_duration(dur, dur);

            if (cast)
                set_tim_levitation(randint1(dur) + dur, FALSE);
        }
        break;

    case 6:
        if (name) return "抵抗环境";
        if (desc) return "在一段时间内赋予火焰、寒冷和闪电抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_fire(dur, FALSE);
                set_oppose_elec(dur, FALSE);
            }
        }
        break;

    case 7:
        if (name) return "治愈伤口与中毒";
        if (desc) return "治愈所有割伤。轻微恢复生命值。稍微减轻中毒程度；完全治愈低级中毒。";

        {
            int dice = 2;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(damroll(dice, sides));
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(125, p_ptr->poisoned / 3), TRUE);
            }
        }
        break;

    case 8:
        if (name) return "化石为泥";
        if (desc) return "将一个岩石方格化为泥土。";

        {
            int dice = 1;
            int sides = 30;
            int base = 20;

            if (info) return info_damage(dice, sides, base);

            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                wall_to_mud(dir);
            }
        }
        break;

    case 9:
        if (name) return "冰霜之矢";
        if (desc) return "发射一道寒冷魔法弹或射线。";

        {
            int dice = 3 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_COLD,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "自然感知";
        if (desc) return "绘制附近区域的地图。探测所有的怪物、陷阱、门和楼梯。";

        {
            int rad1 = DETECT_RAD_MAP;
            int rad2 = DETECT_RAD_DEFAULT;

            if (info) return info_radius(MAX(rad1, rad2));

            if (cast)
            {
                map_area(rad1);
                detect_traps(rad2, TRUE);
                detect_doors(rad2);
                detect_stairs(rad2);
                detect_monsters_normal(rad2);
            }
        }
        break;

    case 11:
        if (name) return "火之矢";
        if (desc) return "发射一道火焰魔法弹或射线。";

        {
            int dice = 5 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "阳光射线";
        if (desc) return "发射一道闪光射线，对惧光的怪物造成伤害。";

        {
            int dice = 6;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("一道阳光射线出现了。");

                project_hook(
                    GF_LITE_WEAK,
                    dir,
                    spell_power(damroll(6, 8) + p_ptr->to_d_spell),
                    PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL
                );
            }
        }
        break;

    case 13:
        if (name) return "纠缠";
        if (desc) return "尝试使视线内的所有怪物减速。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                slow_monsters(power);
            }
        }
        break;

    case 14:
        if (name) return "自然之门";
        if (desc) return "召唤一只或多只动物。在较高等级时，可能会召唤猎犬、爬行动物甚至树人！";

        if (cast)
        {
            bool success = FALSE;
            if (plev < 30)
                success = trump_summoning(1, TRUE, py, px, 0, SUMMON_ANIMAL_RANGER, PM_ALLOW_GROUP);
            else if (plev < 47)
            {
                switch (randint1(3))
                {
                case 1:
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_HOUND, PM_ALLOW_GROUP);
                    break;
                case 2:
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_HYDRA, PM_ALLOW_GROUP);
                    break;
                case 3:
                    success = trump_summoning((1 + (plev - 15)/ 10), TRUE, py, px, 0, SUMMON_ANIMAL_RANGER, PM_ALLOW_GROUP);
                    break;
                }
            }
            else
            {
                if (one_in_(5))
                    success = trump_summoning(1, TRUE, py, px, 0, SUMMON_ENT, PM_ALLOW_GROUP);
            }
            if (!success)
                msg_print("没有援军到来。");
        }
        break;

    case 15:
        if (name) return "草药治疗";
        if (desc) return "极大地恢复生命值。并治愈割伤、震慑，甚至可能解毒。";

        {
            int heal = spell_power(500);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE);
            }
        }
        break;

    case 16:
        if (name) return "建造阶梯";
        if (desc) return "生成一个向下或向上的楼梯。";

        {
            if (cast)
            {
                stair_creation(FALSE);
            }
        }
        break;

    case 17:
        if (name) return "石肤术";
        if (desc) return "在一段时间内提供防御(AC)加值。";

        {
            int base = spell_power(20);
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_shield(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "真实抵抗";
        if (desc) return "在一段时间内赋予火焰、寒冷、闪电、酸液和毒素抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_base(randint1(base) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "制造森林";
        if (desc) return "在所有相邻的方格内生成树木。";

        {
            if (cast)
            {
                tree_creation();
            }
        }
        break;

    case 20:
        if (name) return "与石交谈";
        if (desc) return "*鉴定*一件物品（全面鉴定）。";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 21:
        if (name) return "石墙术";
        if (desc) return "在所有相邻的方格内生成花岗岩墙。";

        {
            if (cast)
            {
                wall_stone();
            }
        }
        break;

    case 22:
        if (name) return "防腐保护";
        if (desc) return "使一件装备防酸（免疫酸液破坏）。";

        {
            if (cast)
            {
                if (!rustproof()) return NULL;
            }
        }
        break;

    case 23:
        if (name) return "召唤阳光";
        if (desc) return "以你为中心生成一颗强光球。为整个地下城楼层绘制地图并将其照亮。知晓所有物品的位置。";

        {
            int dam = spell_power(150 + p_ptr->to_d_spell);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_LITE, 0, dam, rad);
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);
                wiz_lite(FALSE);

                if ( (prace_is_(RACE_VAMPIRE) || prace_is_(RACE_MON_VAMPIRE) || p_ptr->mimic_form == MIMIC_VAMPIRE)
                  && !res_save_default(RES_LITE) )
                {
                    msg_print("阳光灼伤了你的肉体！");
                    take_hit(DAMAGE_NOESCAPE, 50, "阳光");
                }
            }
        }
        break;

    case 24:
        if (name) return "地震";
        if (desc) return "震动地下城结构，导致地板和墙壁随机交换。";

        {
            int rad = spell_power(10);

            if (info) return info_radius(rad);

            if (cast)
            {
                earthquake(py, px, rad);
            }
        }
        break;

    case 25:
        if (name) return "火焰风暴";
        if (desc) return "发射一颗巨大的火球。";

        {
            int dam = spell_power(60 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 26:
        if (name) return "暴风雪";
        if (desc) return "发射一颗巨大的寒冰球。";

        {
            int dam = spell_power(70 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;

    case 27:
        if (name) return "闪电风暴";
        if (desc) return "发射一颗巨大的电球。";

        {
            int dam = spell_power(90 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
                break;
            }
        }
        break;

    case 28:
        if (name) return "漩涡";
        if (desc) return "发射一颗巨大的水球。";

        {
            int dam = spell_power(100 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_WATER, dir, dam, rad);
            }
        }
        break;

    case 29:
        if (name) return "寒冰之矢";
        if (desc) return "发射一道坚冰弹。";

        {
            int dice = 5 + 15*plev/50;
            int sides = 15;

            if (info) return info_damage(spell_power(dice), sides, p_ptr->to_d_spell);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ICE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 30:
        if (name) return "重力风暴";
        if (desc) return "发射一颗巨大的重力球。";

        {
            int dam = spell_power(70 + plev * 2 + p_ptr->to_d_spell);
            int rad = plev / 12 + 1;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_GRAVITY, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "自然之怒";
        if (desc) return "你释放出自然界的全部狂怒，其确切后果无法预测。";

        if (cast)
        {
            int i;
            switch (randint1(6))
            {
            case 1: /* The original effect: Line of Sight damage, earthquake, disintegration ball */
                msg_print("自然的狂怒被释放了！");
                dispel_monsters(spell_power(4 * plev + p_ptr->to_d_spell));
                earthquake(py, px, spell_power(20 + plev / 2));
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((100 + plev + p_ptr->to_d_spell) * 2),
                    GF_DISINTEGRATE,
                    PROJECT_KILL | PROJECT_ITEM
                );
                break;

            case 2: /* Deadly bolt of lightning */
                msg_print("你的双手闪烁着电火花！");
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ELEC,
                    dir,
                    spell_power(plev * 8 + p_ptr->to_d_spell)
                );
                break;

            case 3: /* Immense thunderclap */
                msg_print("响起了一声巨大的雷鸣！");
                project_hack(GF_SOUND, spell_power(plev * 5 + p_ptr->to_d_spell));
                break;

            case 4: /* Gravitational Wave */
                msg_print("你周围的空间扭曲了！");
                project_hack(GF_GRAVITY, spell_power(plev * 4 + p_ptr->to_d_spell));
                break;

            case 5: /* Elemental Storm */
                msg_print("你释放了元素！");
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_FIRE,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_COLD,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                project(
                    0,
                    spell_power(1 + plev / 12),
                    py,
                    px,
                    spell_power((120 + plev + p_ptr->to_d_spell) * 2),
                    GF_ELEC,
                    PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL
                );
                break;

            case 6: /* Rock Storm */
                msg_print("你发射出了一阵巨石风暴！");
                if (!get_fire_dir(&dir)) return NULL;
                for (i = 0; i < 3; i++)
                    fire_ball(GF_SHARDS, dir, spell_power(70 + plev + p_ptr->to_d_spell), 1);
                break;
            }
        }
        break;
    }

    return "";
}


static cptr do_chaos_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";
    static const char s_random[] = "随机";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "魔法飞弹";
        if (desc) return "发射一道微弱的魔法弹。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_MISSILE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "陷阱/门破坏";
        if (desc) return "摧毁相邻方格内的所有陷阱。";

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cast)
            {
                destroy_doors_touch();
            }
        }
        break;

    case 2:
        if (name) return "闪光";
        if (desc) return "永久照亮附近区域和房间内部。";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = (plev / 10) + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 3:
        if (name) return "困惑之触";
        if (desc) return "尝试使你下一个击中的怪物混乱。";

        {
            if (cast)
            {
                if (!(p_ptr->special_attack & ATTACK_CONFUSE))
                {
                    msg_print("你的双手开始发光。");

                    p_ptr->special_attack |= ATTACK_CONFUSE;
                    p_ptr->redraw |= (PR_STATUS);
                }
            }
        }
        break;

    case 4:
        if (name) return "法力爆发";
        if (desc) return "发射一颗魔法球。";

        {
            int dice = 3;
            int sides = 5;
            int rad = spell_power((plev < 30) ? 2 : 3);
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_MISSILE, /* GF_MANA? */
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 5:
        if (name) return "火之矢";
        if (desc) return "发射一道火焰魔法弹或射线。";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 6:
        if (name) return "力场之拳";
        if (desc) return "发射一颗微小的分解球。";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_DISINTEGRATE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                    0
                );
            }
        }
        break;

    case 7:
        if (name) return "传送自身";
        if (desc) return "长距离传送。";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                teleport_player(range, 0L);
            }
        }
        break;

    case 8:
        if (name) return "奇迹";
        if (desc) return "发射某种带有随机效果的投射物。";

        {
            if (info) return s_random;

            if (cast)
            {

                if (!get_fire_dir(&dir)) return NULL;

                cast_wonder(dir);
            }
        }
        break;

    case 9:
        if (name) return "混沌之矢";
        if (desc) return "发射一道混沌魔法弹或射线。";

        {
            int dice = 10 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_CHAOS,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "音爆";
        if (desc) return "以你为中心生成一颗音波球。";

        {   /* Note: Damage is high relative to say fireball, but this is
               an adjacent spell while fireball is a distance spell. So it should
               do something like 33% *more* damage than a fireball. However, stunning
               is a bonus to be reckoned with ... (Design: Ranged = 75% Melee) */
            int dam = spell_power(50 + plev + p_ptr->to_d_spell);
            int rad = spell_power(plev / 10 + 2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                msg_print("轰！震动了整个房间！");
                project(0, rad, py, px, dam*2, GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 11:
        if (name) return "毁灭之矢";
        if (desc) return "发射一道纯净法力射线。";

        {
            int dice = 11 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_MANA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "火球";
        if (desc) return "发射一颗火球。";

        {
            int dam = spell_power(plev + 55 + p_ptr->to_d_spell);
            int rad = spell_power(2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 13:
        if (name) return "传送他人";
        if (desc) return "除非被抵抗，否则将射线路径上的所有怪物传送走。";

        {
            int power = spell_power(plev*2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 14:
        if (name) return "毁灭之语";
        if (desc) return "摧毁附近区域内的一切。";

        {
            int base = 12;
            int sides = 4;
            int power = spell_power(4 * plev);

            if (info) return info_power(power);

            if (cast)
            {
                destroy_area(py, px, base + randint1(sides), power);
            }
        }
        break;

    case 15:
        if (name) return "唤起洛格鲁斯";
        if (desc) return "发射一颗巨大的混沌球。";

        {
            int dam = spell_power(plev * 2 + 99 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 5);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_CHAOS, dir, dam, rad);
            }
        }
        break;

    case 16:
        if (name) return "变形他人";
        if (desc) return "尝试使一个怪物变形。";

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                poly_monster(dir);
            }
        }
        break;

    case 17:
        if (name) return "连锁闪电";
        if (desc) return "向所有方向发射闪电射线。";

        {
            int dice = 5 + plev / 10;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                for (dir = 0; dir <= 9; dir++)
                {
                    fire_beam(
                        GF_ELEC,
                        dir,
                        spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                    );
                }
            }
        }
        break;

    case 18:
        if (name) return "奥术绑定";
        if (desc) return "它尝试使用你的法力为一件装置充能。";

        {
            int power = spell_power(90);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 19:
        if (name) return "解离";
        if (desc) return "发射一颗巨大的分解球。";

        {
            int dam = spell_power(plev + 70 + p_ptr->to_d_spell);
            int rad = 3 + plev / 40;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_DISINTEGRATE, dir, dam, rad);
            }
        }
        break;

    case 20:
        if (name) return "改变现实";
        if (desc) return "重新生成当前的地下城楼层（改变楼层）。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                alter_reality();
            }
        }
        break;

    case 21:
        if (name) return "魔法火箭";
        if (desc) return "发射一枚魔法火箭。";

        {
            int dam = spell_power(50 + plev * 4 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("你发射了一枚火箭！");
                fire_rocket(GF_ROCKET, dir, dam, rad);
            }
        }
        break;

    case 22:
        if (name) return "混沌烙印";
        if (desc) return "使当前武器变成混沌(Chaotic)武器。";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_CHAOS);
            }
        }
        break;

    case 23:
        if (name) return "召唤恶魔";
        if (desc) return "召唤一只恶魔。";

        {
            if (cast)
            {
                u32b mode = 0L;
                bool pet = !one_in_(3);

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, SUMMON_DEMON, mode))
                {
                    msg_print("这片区域弥漫着硫磺和烈火的恶臭。");

                    if (pet)
                    {
                        msg_print("'您有何吩咐……主人？'");
                    }
                    else
                    {
                        msg_print("'我绝不服从！可怜虫！我要吞噬你凡人的灵魂！'");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "重力射线";
        if (desc) return "发射一道重力射线。";

        {
            int dice = 9 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_GRAVITY,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 25:
        if (name) return "陨石雨";
        if (desc) return "使陨石球坠落到附近的随机位置。";

        {
            int dam = spell_power(plev * 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                cast_meteor(dam, rad);
            }
        }
        break;

    case 26:
        if (name) return "烈焰打击";
        if (desc) return "以你为中心生成一颗巨大的火球。";

        {
            int dam = spell_power(300 + 3 * plev + p_ptr->to_d_spell*2);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_FIRE, 0, dam, rad);
            }
        }
        break;

    case 27:
        if (name) return "召唤混沌";
        if (desc) return "生成随机类型的魔法球或射线。";

        {
            if (cast)
            {
                call_chaos(100);
            }
        }
        break;

    case 28:
        if (name) return "变形自身";
        if (desc) return "将你自己变形为一个新的形态。";

        if (cast)
        {
            int which;
            switch (randint1(50))
            {
            case 1: which = one_in_(10) ? MIMIC_DEMON_LORD : MIMIC_DEMON; break;
            case 2: which = MIMIC_VAMPIRE; break;
            default:
                for (;;)
                {
                    which = randint0(MAX_RACES);
                    if ( which != RACE_HUMAN
                      && which != RACE_DEMIGOD
                      && which != RACE_DRACONIAN
                      && which != RACE_ANDROID
                      && which != RACE_WEREWOLF
                      && which != RACE_BEORNING
                      && which != RACE_DOPPELGANGER
                      && p_ptr->prace != which
                      && !(get_race_aux(which, 0)->flags & RACE_IS_MONSTER)
                      && !(get_race_aux(which, 0)->flags & RACE_NO_POLY) )
                    {
                        break;
                    }
                }
            }
            set_mimic(50 + randint1(50), which, FALSE);
            if (p_ptr->mimic_form == MIMIC_NONE) msg_print("毫无效果。");
        }
        break;

    case 29:
        if (name) return "法力风暴";
        if (desc) return "发射一颗威力极其强大的纯净法力巨球。";

        {
            int dam = spell_power(300 + plev * 4 + p_ptr->to_d_spell);
            int rad = spell_power(4);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;

    case 30:
        if (name) return "喷吐洛格鲁斯";
        if (desc) return "发射一颗威力极其强大的混沌球。";

        {
            int dam = spell_power(3*p_ptr->chp/4 + p_ptr->to_d_spell);
            int rad = spell_power(2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_CHAOS, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "呼唤虚空";
        if (desc) return "除非你不与任何墙壁相邻，否则向所有方向发射火箭、法力球和核废料球。否则将*摧毁*巨大的区域。";

        {
            if (info) return format("%s3 * 175", s_dam);

            if (cast)
            {
                call_the_();
            }
        }
        break;
    }

    return "";
}


static cptr do_death_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";
    static const char s_random[] = "随机";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "探测非生命体";
        if (desc) return "探测你附近所有的非活体怪物。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_nonliving(rad);
            }
        }
        break;

    case 1:
        if (name) return "恶咒";
        if (desc) return "发射一颗微小的邪恶能量球，对善良阵营的怪物造成极大的伤害。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;
            int rad = 0;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                int dam;
                if (!get_fire_dir(&dir)) return NULL;
                dam = spell_power(damroll(dice, sides) + p_ptr->to_d_spell);
                fire_ball(GF_HELL_FIRE, dir, dam, rad);

                if (one_in_(5))
                {
                    /* Special effect first */
                    int effect = randint1(1000);

                    if (effect == 666)
                        fire_ball_hide(GF_DEATH_RAY, dir, spell_power(plev * 200), 0);
                    else if (effect < 500)
                        fire_ball_hide(GF_TURN_ALL, dir, spell_power(plev), 0);
                    else if (effect < 800)
                        fire_ball_hide(GF_OLD_CONF, dir, dam, 0);
                    else
                        fire_ball_hide(GF_STUN, dir, dam, 0);
                }
            }
        }
        break;

    case 2:
        if (name) return "探测邪恶";
        if (desc) return "探测你附近所有的邪恶怪物。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_evil(rad);
            }
        }
        break;

    case 3:
        if (name) return "臭气云";
        if (desc) return "发射一颗毒素球。";

        {
            int dam = spell_power(10 + plev / 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_POIS, dir, dam, rad);
            }
        }
        break;

    case 4:
        if (name) return "黑死眠";
        if (desc) return "尝试催眠一个怪物。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                sleep_monster(dir, power);
            }
        }
        break;

    case 5:
        if (name) return "死灵抗性";
        if (desc) return "赋予毒素与寒冷抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_cold(dur, FALSE);
                set_oppose_pois(dur, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "惊骇";
        if (desc) return "尝试恐吓并震慑一个怪物。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
                stun_monster(dir, 5 + plev/5);
            }
        }
        break;

    case 7:
        if (name) return "奴役不死生物";
        if (desc) return "尝试魅惑一个不死怪物。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                control_one_undead(dir, power);
            }
        }
        break;

    case 8:
        if (name) return "熵之法球";
        if (desc) return "发射一颗对活体怪物造成伤害的魔法球。";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_OLD_DRAIN,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 9:
        if (name) return "地狱之矢";
        if (desc) return "发射一道地狱能量弹或射线。";

        {
            int dice = 8 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_NETHER,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 10:
        if (name) return "死云术";
        if (desc) return "以你为中心生成一颗毒素球。";

        {
            int dam = spell_power((30 + plev) * 2 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 10 + 2);

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                project(0, rad, py, px, dam, GF_POIS, PROJECT_GRID | PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 11:
        if (name) return "单体灭绝";
        if (desc) return "尝试使一个怪物消失（抹除）。";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball_hide(GF_GENOCIDE, dir, power, 0);
            }
        }
        break;

    case 12:
        if (name) return "毒素烙印";
        if (desc) return "使当前武器附上毒素(poison branded)属性。";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_BRAND_POIS, OF_RES_POIS);
            }
        }
        break;

    case 13:
        if (name) return "吸血汲取";
        if (desc) return "从一个怪物身上吸取部分生命值并转移给你。你也能从中获得营养补给。";

        {
            int dice = 1;
            int sides = spell_power(plev * 2);
            int base = spell_power(plev * 2 + p_ptr->to_d_spell);

            if (info) return info_damage(dice, sides, base);

            if (cast)
            {
                int dam = base + damroll(dice, sides);

                if (!get_fire_dir(&dir)) return NULL;

                if (drain_life(dir, dam))
                {
                    if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                    {
                        msg_print("你不受影响。");
                        break;
                    }

                    virtue_add(VIRTUE_SACRIFICE, -1);
                    virtue_add(VIRTUE_VITALITY, -1);

                    hp_player(dam);

                    /*
                     * Gain nutritional sustenance:
                     * 150/hp drained
                     *
                     * A Food ration gives 5000
                     * food points (by contrast)
                     * Don't ever get more than
                     * "Full" this way But if we
                     * ARE Gorged, it won't cure
                     * us
                     */
                    dam = p_ptr->food + MIN(5000, 100 * dam);

                    /* Not gorged already */
                    if (p_ptr->food < PY_FOOD_MAX)
                        set_food(dam >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dam);
                }
            }
        }
        break;

    case 14:
        if (name) return "操纵死尸";
        if (desc) return "复活附近的尸体和骸骨，并让它们成为你的宠物。";

        {
            if (cast)
            {
                animate_dead(0, py, px);
            }
        }
        break;

    case 15:
        if (name) return "灭绝";
        if (desc) return "灭绝一整类怪物，此举会使你筋疲力尽。强大或唯一的怪物可能会抵抗。";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                symbol_genocide(power, TRUE);
            }
        }
        break;

    case 16:
        if (name) return "狂暴";
        if (desc) return "在一段时间内提供命中和生命值(HP)加值，并免疫恐惧。但会降低防御(AC)。";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_shero(randint1(base) + base, FALSE);
                hp_player(30);
            }
        }
        break;

    case 17:
        if (name) return "唤起幽魂";
        if (desc) return "引发随机效果。";

        {
            if (info) return s_random;

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                cast_invoke_spirits(dir);
            }
        }
        break;

    case 18:
        if (name) return "暗之矢";
        if (desc) return "发射一道暗黑魔法弹或射线。";

        {
            int dice = 4 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_DARK,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 19:
        if (name) return "战斗狂热";
        if (desc) return "在一段时间内提供命中、生命值(HP)加值和恐惧抗性，并使你加速，提高防御(AC)。";

        {
            int b_base = spell_power(25);
            int sp_base = spell_power(plev / 2);
            int sp_sides = 20 + plev / 2;

            if (info) return info_duration(b_base, b_base);

            if (cast)
            {
                set_hero(randint1(b_base) + b_base, FALSE);
                set_blessed(randint1(b_base) + b_base, FALSE);
                set_fast(randint1(sp_sides) + sp_base, FALSE);
            }
        }
        break;

    case 20:
        if (name) return "吸血烙印";
        if (desc) return "使当前武器变成吸血(Vampiric)武器。";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_DEATH);
            }
        }
        break;

    case 21:
        if (name) return "真实吸血";
        if (desc) return "发射三道魔法弹。每一道都能从怪物身上吸取部分生命值并转移给你。";

        {
            int dam = spell_power(100 + p_ptr->to_d_spell/3);

            if (info) return format("%s3*%d", s_dam, dam);

            if (cast)
            {
                int i;

                if (!get_fire_dir(&dir)) return NULL;

                virtue_add(VIRTUE_SACRIFICE, -1);
                virtue_add(VIRTUE_VITALITY, -1);

                for (i = 0; i < 3; i++)
                {
                    if (drain_life(dir, dam) && p_ptr->pclass != CLASS_BLOOD_MAGE)
                        vamp_player(dam);
                }
            }
        }
        break;

    case 22:
        if (name) return "地狱冲击波";
        if (desc) return "对视线内的所有活体怪物造成伤害。";

        {
            int sides = plev * 3;

            if (info) return info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_living(spell_power(randint1(sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 23:
        if (name) return "黑暗风暴";
        if (desc) return "发射一颗巨大的暗黑球。";

        {
            int dam = 100 + py_prorata_level_aux(200, 1, 1, 2);
            int rad = spell_power(4);

            dam = spell_power(dam + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_DARK, dir, dam, rad);
            }
        }
        break;

    case 24:
        if (name) return "死亡射线";
        if (desc) return "发射一道死亡射线。";

        {
            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                project_hook(GF_DEATH_RAY, dir, plev * 200, /*  v--- This is mean as it auto kills the player! */
                                PROJECT_STOP | PROJECT_KILL /*| PROJECT_REFLECTABLE*/);
            }
        }
        break;

    case 25:
        if (name) return "复活死者";
        if (desc) return "召唤一只不死怪物。";

        {
            if (cast)
            {
                int type;
                bool pet = one_in_(3);
                u32b mode = 0L;

                type = (plev > 47 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD);

                if (!pet || (pet && (plev > 24) && one_in_(3)))
                    mode |= PM_ALLOW_GROUP;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, type, mode))
                {
                    msg_print("寒风开始在你周围吹拂，夹杂着腐烂的恶臭……");


                    if (pet)
                    {
                        msg_print("古老、早已死去的躯体从地下爬起，听候你的差遣！");
                    }
                    else
                    {
                        msg_print("'死者苏醒了……来惩罚你惊扰了他们！'");
                    }

                    virtue_add(VIRTUE_UNLIFE, 1);
                }
            }
        }
        break;

    case 26:
        if (name) return "秘术";
        if (desc) return "鉴定一件物品。在较高等级时则会*鉴定*（全面鉴定）物品。";

        {
            if (cast)
            {
                if (randint1(50) > spell_power(plev))
                {
                    if (!ident_spell(NULL)) return NULL;
                }
                else
                {
                    if (!identify_fully(NULL)) return NULL;
                }
            }
        }
        break;

    case 27:
        if (name) return "变形吸血鬼";
        if (desc) return "暂时模仿吸血鬼。失去原本种族的能力，并获得吸血鬼的能力。";

        {
            int base = spell_power(10 + plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_VAMPIRE, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("毫无效果。");
            }
        }
        break;

    case 28:
        if (name) return "恢复生命";
        if (desc) return "恢复流失的生命力与经验值。";

        {
            if (cast)
            {
                restore_level();
                lp_player(1000);
            }
        }
        break;

    case 29:
        if (name) return "群体灭绝";
        if (desc) return "消灭附近所有的怪物，这会让你筋疲力尽。强大或唯一的怪物可能能够抵抗。";

        {
            int power = spell_power(plev*3);

            if (info) return info_power(power);

            if (cast)
            {
                mass_genocide(power, TRUE);
            }
        }
        break;

    case 30:
        if (name) return "地狱风暴";
        if (desc) return "生成一颗巨大的地狱球。";

        {
            int dam = spell_power(plev * 12 + p_ptr->to_d_spell + 5);
            int rad = spell_power(plev / 5);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_NETHER, dir, dam, rad);
            }
        }
        break;

    case 31:
        if (name) return "幽灵形态";
        if (desc) return "赋予穿墙能力，并将受到的绝大部分伤害减半。";

        {
            int base = spell_power(plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_wraith_form(randint1(base) + base, FALSE);
            }
        }
        break;
    }

    return "";
}

static cptr do_trump_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

    static const char s_random[] = "随机";

    int dir;
    int plev = p_ptr->lev;
    int x = px;
    int y = py;

    if (!fail && old_target_okay() && los(py, px, target_row, target_col) && !one_in_(3))
    {
        y = target_row;
        x = target_col;
    }

    switch (spell)
    {
    case 0:
        if (name) return "相位门";
        if (desc) return "短距离传送。";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 1:
        if (name) return "王牌蜘蛛";
        if (desc) return "召唤蜘蛛。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张蜘蛛王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_SPIDER, PM_ALLOW_GROUP))
                {
                    if (fail)
                    {
                        msg_print("被召唤的蜘蛛发怒了！");
                    }
                }
            }
        }
        break;

    case 2:
        if (name) return "洗牌";
        if (desc) return "引发随机效果。";

        {
            if (info) return s_random;

            if (cast)
            {
                if (TRUE || get_check("你确定要洗牌吗？"))
                    cast_shuffle();
                else
                    return NULL;
            }
        }
        break;

    case 3:
        if (name) return "重置召回";
        if (desc) return "重置召回法术的目标“最深”层数。";

        {
            if (cast)
            {
                if (!reset_recall()) return NULL;
            }
        }
        break;

    case 4:
        if (name) return "传送";
        if (desc) return "长距离传送。";

        {
            int range = plev * 4;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 5:
        if (name) return "王牌探知";
        if (desc) return "在一段时间内赋予心灵感应。";

        {
            int base = spell_power(25);
            int sides = spell_power(30);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "传送离开";
        if (desc) return "除非被抵抗，否则将射线路径上的所有怪物传送走。";

        {
            int power = spell_power(plev*2);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 7:
        if (name) return "王牌动物";
        if (desc) return "召唤一只动物。";

        {
            if (cast || fail)
            {
                int type = (!fail ? SUMMON_ANIMAL_RANGER : SUMMON_ANIMAL);

                msg_print("你集中精神在一张动物王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的动物发怒了！");
                    }
                }
            }
        }
        break;

    case 8:
        if (name) return "王牌触及";
        if (desc) return "将远处的物品拉到你身边。";

        {
            int weight = spell_power(plev * 15);

            if (info) return info_weight(weight);

            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                fetch(dir, weight, FALSE);
            }
        }
        break;

    case 9:
        if (name) return "王牌神风特攻";
        if (desc) return "召唤会自爆的怪物。";

        {
            if (cast || fail)
            {
                int x, y;
                int type;

                if (cast)
                {
                    if (!target_set(TARGET_KILL)) return NULL;
                    x = target_col;
                    y = target_row;
                }
                else
                {
                    /* Summons near player when failed */
                    x = px;
                    y = py;
                }

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_KAMIKAZE_LIVING;
                else
                    type = SUMMON_KAMIKAZE;

                msg_print("你同时集中精神在多张王牌上……");

                if (trump_summoning(2 + randint0(plev / 7), !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的生物发怒了！");
                    }
                }
            }
        }
        break;

    case 10:
        if (name) return "幻影仆从";
        if (desc) return "召唤一只幽灵。";

        {
            /* Phantasmal Servant is not summoned as enemy when failed */
            if (cast)
            {
                int summon_lev = plev * 2 / 3 + randint1(plev / 2);

                if (trump_summoning(1, !fail, y, x, (summon_lev * 3 / 2), SUMMON_PHANTOM, 0L))
                {
                    msg_print("'您的愿望，主人？'");
                }
            }
        }
        break;

    case 11:
        if (name) return "加速怪物";
        if (desc) return "使一个怪物加速。";

        {
            if (cast)
            {
                bool result;

                /* Temporary enable target_pet option */
                bool old_target_pet = target_pet;
                target_pet = TRUE;

                result = get_fire_dir(&dir);

                /* Restore target_pet option */
                target_pet = old_target_pet;

                if (!result) return NULL;

                speed_monster(dir);
            }
        }
        break;

    case 12:
        if (name) return "传送楼层";
        if (desc) return "立刻传送到向上或向下的楼梯处。";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 13:
        if (name) return "任意门";
        if (desc) return "传送到指定位置。";

        {
            int range = plev / 2 + 10;

            if (info) return info_range(range);

            if (cast)
            {
                msg_print("你打开了一道次元门。选择一个目的地。");

                if (!dimension_door(range)) return NULL;
            }
        }
        break;

    case 14:
        if (name) return "召回之语";
        if (desc) return "将玩家从地下城召回城镇，或从城镇送回地下城的最深层。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 15:
        if (name) return "驱逐";
        if (desc) return "除非被抵抗，否则将视线内所有的怪物传送走。";

        {
            int power = spell_power(plev * 4);

            if (info) return info_power(power);

            if (cast)
            {
                banish_monsters(power);
            }
        }
        break;

    case 16:
        if (name) return "交换位置";
        if (desc) return "与一个怪物交换位置。";

        {
            if (cast)
            {
                bool result;

                /* HACK -- No range limit */
                project_length = -1;

                result = get_fire_dir(&dir);

                /* Restore range to default */
                project_length = 0;

                if (!result) return NULL;

                teleport_swap(dir);
            }
        }
        break;

    case 17:
        if (name) return "王牌不死生物";
        if (desc) return "召唤一只不死怪物。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张不死生物王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_UNDEAD, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的不死生物发怒了！");
                    }
                }
            }
        }
        break;

    case 18:
        if (name) return "王牌爬行动物";
        if (desc) return "召唤一只九头蛇。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张爬行动物王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HYDRA, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的爬行动物发怒了！");
                    }
                }
            }
        }
        break;

    case 19:
        if (name) return "王牌怪物";
        if (desc) return "召唤一些怪物。";

        {
            if (cast || fail)
            {
                int type;

                msg_print("你同时集中精神在多张王牌上……");

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_LIVING;
                else
                    type = 0;

                if (trump_summoning((1 + (plev - 15)/ 10), !fail, y, x, 0, type, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的生物发怒了！");
                    }
                }

            }
        }
        break;

    case 20:
        if (name) return "王牌猎犬";
        if (desc) return "召唤一群猎犬。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张猎犬王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HOUND, PM_ALLOW_GROUP))
                {
                    if (fail)
                    {
                        msg_print("被召唤的猎犬发怒了！");
                    }
                }
            }
        }
        break;

    case 21:
        if (name) return "王牌烙印";
        if (desc) return "使当前武器变成王牌(Trump)武器。";

        {
            if (cast)
            {
                brand_weapon(EGO_WEAPON_TRUMP);
            }
        }
        break;

    case 22:
        if (name) return "活体王牌";
        if (desc) return "赋予使你随机传送或能够随意传送的变异。";

        {
            if (cast)
            {
                int mutation;

                if (one_in_(7))
                    /* Teleport control */
                    mutation = MUT_TELEPORT;
                else
                    /* Random teleportation (uncontrolled) */
                    mutation = MUT_TELEPORT_RND;

                /* Gain the mutation */
                if (mut_gain(mutation))
                {
                    msg_print("你变成了一个活体王牌。");
                }
            }
        }
        break;

    case 23:
        if (name) return "王牌机械恶魔";
        if (desc) return "召唤一只赛博恶魔(cyber demon)。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张机械恶魔王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_CYBER, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的机械恶魔发怒了！");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "王牌占卜";
        if (desc) return "探测你附近所有的怪物、陷阱、门、楼梯、宝藏和物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 25:
        if (name) return "王牌学识";
        if (desc) return "*鉴定*一件物品（全面鉴定）。";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 26:
        if (name) return "治疗怪物";
        if (desc) return "治愈一个怪物。";

        {
            int heal = spell_power(plev * 10 + 200);

            if (info) return info_heal(0, 0, heal);

            if (cast)
            {
                bool result;

                /* Temporary enable target_pet option */
                bool old_target_pet = target_pet;
                target_pet = TRUE;

                result = get_fire_dir(&dir);

                /* Restore target_pet option */
                target_pet = old_target_pet;

                if (!result) return NULL;

                heal_monster(dir, heal);
            }
        }
        break;

    case 27:
        if (name) return "王牌龙";
        if (desc) return "召唤一只龙。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张龙的王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_DRAGON, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的龙发怒了！");
                    }
                }
            }
        }
        break;

    case 28:
        if (name) return "王牌陨石";
        if (desc) return "使陨石球坠落到附近的随机位置。";

        {
            int dam = spell_power(plev * 2 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                cast_meteor(dam, rad);
            }
        }
        break;

    case 29:
        if (name) return "王牌恶魔";
        if (desc) return "召唤一只恶魔。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张恶魔王牌上……");

                if (trump_summoning(1, !fail, y, x, 0, SUMMON_DEMON, 0L))
                {
                    if (fail)
                    {
                        msg_print("被召唤的恶魔发怒了！");
                    }
                }
            }
        }
        break;

    case 30:
        if (name) return "王牌高阶不死生物";
        if (desc) return "召唤一只高等不死生物。";

        {
            if (cast || fail)
            {
                msg_print("你集中精神在一张高阶不死生物王牌上……");
                /* May allow unique depend on level and dice roll */
                if (trump_summoning(1, !fail, y, x, 0, SUMMON_HI_UNDEAD, PM_ALLOW_UNIQUE))
                {
                    if (fail)
                    {
                        msg_print("被召唤的高阶不死生物发怒了！");
                    }
                }
            }
        }
        break;

    case 31:
        if (name) return "王牌远古龙";
        if (desc) return "召唤一只远古龙。";

        {
            if (cast)
            {
                int type;

                if (p_ptr->pclass == CLASS_BEASTMASTER)
                    type = SUMMON_HI_DRAGON_LIVING;
                else
                    type = SUMMON_HI_DRAGON;

                msg_print("你集中精神在一张远古龙王牌上……");

                /* May allow unique depend on level and dice roll */
                if (trump_summoning(1, !fail, y, x, 0, type, PM_ALLOW_UNIQUE))
                {
                    if (fail)
                    {
                        msg_print("被召唤的远古龙发怒了！");
                    }
                }
            }
        }
        break;
    }

    return "";
}


static cptr do_arcane_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "发射法术";
        if (desc) return "发射一道闪电魔法弹或射线。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 3;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "巫师之锁";
        if (desc) return "锁上一扇门。";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                wizard_lock(dir);
            }
        }
        break;

    case 2:
        if (name) return "探测隐形";
        if (desc) return "探测你附近所有的隐形怪物。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_invis(rad);
            }
        }
        break;

    case 3:
        if (name) return "探测怪物";
        if (desc) return "探测你附近所有的怪物，除非它们隐形。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_normal(rad);
            }
        }
        break;

    case 4:
        if (name) return "闪烁";
        if (desc) return "短距离传送。";

        {
            int range = 10;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 5:
        if (name) return "区域照明";
        if (desc) return "永久照亮附近区域和房间内部。";

        {
            int dice = 2;
            int sides = spell_power(plev / 2);
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 6:
        if (name) return "陷阱与门破坏";
        if (desc) return "发射一道能摧毁陷阱和门的射线。";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                destroy_door(dir);
            }
        }
        break;

    case 7:
        if (name) return "治疗轻伤";
        if (desc) return "轻微治愈割伤并恢复少量生命值。";

        {
            int dice = 2;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(damroll(dice, sides));
                set_cut(p_ptr->cut - 10, TRUE);
            }
        }
        break;

    case 8:
        if (name) return "探测门与陷阱";
        if (desc) return "探测你附近的陷阱、门和楼梯。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);
            }
        }
        break;

    case 9:
        if (name) return "燃素";
        if (desc) return "为提灯或火把增加照明的回合数。";

        {
            if (cast)
            {
                phlogiston();
            }
        }
        break;

    case 10:
        if (name) return "探测财宝";
        if (desc) return "探测你附近的所有宝藏。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_treasure(rad);
                detect_objects_gold(rad);
            }
        }
        break;

    case 11:
        if (name) return "探测魔法物品";
        if (desc) return "探测你附近所有的魔法物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_objects_magic(rad);
            }
        }
        break;

    case 12:
        if (name) return "探测物品";
        if (desc) return "探测你附近的所有物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_objects_normal(rad);
            }
        }
        break;

    case 13:
        if (name) return "解毒";
        if (desc) return "稍微减轻中毒程度。完全治愈低级中毒。";

        {
            if (cast)
            {
                set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE);
            }
        }
        break;

    case 14:
        if (name) return "抵抗寒冷";
        if (desc) return "赋予寒冷抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_cold(randint1(base) + base, FALSE);
            }
        }
        break;

    case 15:
        if (name) return "抵抗火焰";
        if (desc) return "赋予火焰抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 16:
        if (name) return "抵抗闪电";
        if (desc) return "赋予闪电抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_elec(randint1(base) + base, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "抵抗酸液";
        if (desc) return "赋予酸液抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_acid(randint1(base) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "治疗中伤";
        if (desc) return "较多地治愈割伤并恢复生命值。";

        {
            int dice = 4;
            int sides = spell_power(8);

            if (info) return info_heal(dice, sides, 0);

            if (cast)
            {
                hp_player(damroll(dice, sides));
                set_cut((p_ptr->cut / 2) - 50, TRUE);
            }
        }
        break;

    case 19:
        if (name) return "传送";
        if (desc) return "长距离传送。";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 20:
        if (name) return "鉴定";
        if (desc) return "鉴定一件物品。";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
            }
        }
        break;

    case 21:
        if (name) return "化石为泥";
        if (desc) return "将一个岩石方格化为泥土。";

        {
            int dice = 1;
            int sides = 30;
            int base = 20;

            if (info) return info_damage(dice, sides, base);

            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                wall_to_mud(dir);
            }
        }
        break;

    case 22:
        if (name) return "光之射线";
        if (desc) return "发射一道闪光射线，对惧光的怪物造成伤害。";

        {
            int dice = 6;
            int sides = 8;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                msg_print("一道光之射线出现了。");

                lite_line(dir);
            }
        }
        break;

    case 23:
        if (name) return "充饥";
        if (desc) return "满足饥饿。";

        {
            if (cast)
            {
                set_food(PY_FOOD_MAX - 1);
            }
        }
        break;

    case 24:
        if (name) return "识破隐形";
        if (desc) return "在一段时间内赋予识破隐形。";

        {
            int base = spell_power(24);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 25:
        if (name) return "抵抗毒素";
        if (desc) return "赋予毒素抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_pois(randint1(base) + base, FALSE);
            }
        }
        break;

    case 26:
        if (name) return "传送楼层";
        if (desc) return "立刻传送到向上或向下的楼梯处。";

        {
            if (cast)
            {
                if (!py_teleport_level("Are you sure? (Teleport Level) ")) return NULL;
            }
        }
        break;

    case 27:
        if (name) return "传送离开";
        if (desc) return "除非被抵抗，否则将射线路径上的所有怪物传送走。";

        {
            int power = spell_power(plev);

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(GF_AWAY_ALL, dir, power);
            }
        }
        break;

    case 28:
        if (name) return "充能";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "它尝试消耗你的血液为一件装置充能。";
            else
                return "它尝试消耗你的法力为一件装置充能。";
        }

        {
            int power = spell_power(plev * 3 / 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!recharge_from_player(power)) return NULL;
            }
        }
        break;

    case 29:
        if (name) return "探测";
        if (desc) return "探测你附近所有的怪物、陷阱、门、楼梯、宝藏和物品。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_all(rad);
            }
        }
        break;

    case 30:
        if (name) return "召回之语";
        if (desc) return "将玩家从地下城召回城镇，或从城镇送回地下城的最深层。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            if (cast)
            {
                if (!word_of_recall(TRUE)) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "透视";
        if (desc) return "为整个地下城楼层绘制地图并将其照亮。知晓所有物品的位置。并在一段时间内赋予心灵感应。";

        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                virtue_add(VIRTUE_KNOWLEDGE, 1);
                virtue_add(VIRTUE_ENLIGHTENMENT, 1);

                wiz_lite(p_ptr->tim_superstealth > 0);

                if (!p_ptr->telepathy)
                {
                    set_tim_esp(randint1(sides) + base, FALSE);
                }
            }
        }
        break;
    }

    return "";
}

static bool _can_enchant(obj_ptr obj) {
    if (object_is_(obj, TV_SWORD, SV_POISON_NEEDLE)) return FALSE;
    return object_is_weapon_armour_ammo(obj);
}
bool craft_enchant(int max, int inc)
{
    obj_prompt_t prompt = {0};
    char         o_name[MAX_NLEN];
    bool         improved = FALSE;
    u32b         flgs[OF_ARRAY_SIZE];

    prompt.prompt = "附魔哪件物品？";
    prompt.error = "你没有什么可附魔的。";
    prompt.filter = _can_enchant;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return FALSE;

    object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    /* Some objects cannot be enchanted */
    obj_flags(prompt.obj, flgs);
    if (have_flag(flgs, OF_NO_ENCHANT))
        return FALSE;

    /* Enchanting is now automatic ... It was always possible to max
     * out enchanting quickly with skilled macro usage, but other players
     * are inviting carpal tunnel issues to no purpose. */
    if (object_is_weapon_ammo(prompt.obj))
    {
        if (prompt.obj->to_h < max)
        {
            prompt.obj->to_h = MIN(max, prompt.obj->to_h + inc);
            if (prompt.obj->to_h >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
        if (prompt.obj->to_d < max)
        {
            prompt.obj->to_d = MIN(max, prompt.obj->to_d + inc);
            if (prompt.obj->to_d >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
    }
    else
    {
        if (prompt.obj->to_a < max)
        {
            prompt.obj->to_a = MIN(max, prompt.obj->to_a + inc);
            if (prompt.obj->to_a >= 0)
                break_curse(prompt.obj);
            improved = TRUE;
        }
    }


    msg_format("%s%s闪烁着明亮的光芒！",
            (prompt.obj->loc.where != INV_FLOOR) ? "你的" : "这件", o_name,
            (prompt.obj->number > 1) ? "" : "s");

    if (!improved)
    {
        msg_print("附魔失败。");
        if (one_in_(3) && virtue_current(VIRTUE_ENCHANTMENT) < 100)
            virtue_add(VIRTUE_ENCHANTMENT, -1);
    }
    else
    {
        virtue_add(VIRTUE_ENCHANTMENT, 1);
        /* Enchantment should not allow gold farming ... */
        if (object_is_nameless(prompt.obj))
            prompt.obj->discount = 99;
        obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
    }
    return TRUE;
}

static cptr do_craft_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "次级附魔";
        if (desc) return "尝试增加武器的命中和伤害加值，或者增加护甲的防御(AC)加值。";

        if (cast)
        {
            if (!craft_enchant(2 + plev/5, 1))
                return NULL;
        }
        break;

    case 1:
        if (name) return "再生";
        if (desc) return "在一段时间内赋予生命再生能力。";

        {
            int base = spell_power(80);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_regen(base + randint1(base), FALSE);
            }
        }
        break;

    case 2:
        if (name) return "充饥";
        if (desc) return "满足饥饿。";

        {
            if (cast)
            {
                set_food(PY_FOOD_MAX - 1);
            }
        }
        break;

    case 3:
        if (name) return "抵抗寒冷";
        if (desc) return "赋予寒冷抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_cold(randint1(base) + base, FALSE);
            }
        }
        break;

    case 4:
        if (name) return "抵抗火焰";
        if (desc) return "赋予火焰抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 5:
        if (name) return "英雄气概";
        if (desc) return "消除恐惧，并在一段时间内提供命中加值和 10 点额外生命值。";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
            }
        }
        break;

    case 6:
        if (name) return "抵抗闪电";
        if (desc) return "赋予闪电抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_elec(randint1(base) + base, FALSE);
            }
        }
        break;

    case 7:
        if (name) return "抵抗酸液";
        if (desc) return "赋予酸液抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_acid(randint1(base) + base, FALSE);
            }
        }
        break;

    case 8:
        if (name) return "识破隐形";
        if (desc) return "在一段时间内赋予识破隐形。";

        {
            int base = spell_power(24);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 9:
        if (name) return "移除诅咒";
        if (desc) return "解除已装备物品上的普通诅咒。";

        if (cast)
        {
            if (remove_curse())
                msg_print("你感觉好像有人在守护着你。");
        }
        break;

    case 10:
        if (name) return "抵抗毒素";
        if (desc) return "赋予毒素抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_pois(randint1(base) + base, FALSE);
            }
        }
        break;

    case 11:
        if (name) return "狂暴";
        if (desc) return "在一段时间内提供命中和生命值(HP)加值，并免疫恐惧。但会降低防御(AC)。";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_shero(randint1(base) + base, FALSE);
                hp_player(30);
            }
        }
        break;

    case 12:
        if (name) return "自我知识";
        if (desc) return "为你提供关于当前抗性、武器力量以及属性最大上限的有用信息。";

        {
            if (cast)
            {
                self_knowledge();
            }
        }
        break;

    case 13:
        if (name) return "鉴定";
        if (desc) return "鉴定一件物品。";

        {
            if (cast)
            {
                if (!ident_spell(NULL)) return NULL;
            }
        }
        break;

    case 14:
        if (name) return "治愈";
        if (desc) return "它能消除你的异常状态，包括恐惧、中毒、震慑、割伤和幻觉。严重的中毒可能无法被完全治愈。";
        {
            if (cast)
            {
                fear_clear_p();
                set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
                set_image(0, TRUE);
            }
        }
        break;

    case 15:
        if (name) return "元素烙印";
        if (desc) return "使你的攻击暂时附带你选择的元素烙印。";

        {
            int base = plev / 2;

            if (info) return info_duration(base, base);

            if (cast)
            {
                if (!choose_ele_attack()) return NULL;
            }
        }
        break;

    case 16:
        if (name) return "心灵感应";
        if (desc) return "在一段时间内赋予心灵感应。";

        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "石肤术";
        if (desc) return "在一段时间内提供防御(AC)加值。";

        {
            int base = 30;
            int sides = 20;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_shield(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 18:
        if (name) return "抵抗";
        if (desc) return "在一段时间内赋予火焰、寒冷、闪电、酸液和毒素抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_base(randint1(base) + base, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "自我加速";
        if (desc) return "使你加速一段时间。";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_fast(randint1(sides) + base, FALSE);
            }
        }
        break;

    case 20:
        if (name) return "旋风斩";
        if (desc) return "攻击所有相邻的怪物。";

        {
            if (cast)
            {
                int              y = 0, x = 0;
                cave_type       *c_ptr;
                monster_type    *m_ptr;
                int              dir;

                for (dir = 0; dir < 8; dir++)
                {
                    y = py + ddy_ddd[dir];
                    x = px + ddx_ddd[dir];
                    c_ptr = &cave[y][x];
                    m_ptr = &m_list[c_ptr->m_idx];
                    if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                        py_attack(y, x, 0);
                }
            }
        }
        break;

    case 21:
        if (name) return "打磨盾牌";
        if (desc) return "使你的盾牌能反射投射物和魔法弹法术。";
        if (cast)
        {
            if (!polish_shield()) return NULL;
        }
        break;

    case 22:
        if (name) return "武器精通";
        if (desc) return "在短时间内，你的近战武器变得更加致命。";
        {
            int base = spell_power(3 + plev / 10);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_weaponmastery(randint1(base) + base, FALSE);
        }
        break;

    case 23:
        if (name) return "魔法护甲";
        if (desc) return "在一段时间内赋予魔法抗性、防御(AC)加值、免疫混乱和致盲、反射、行动自如和漂浮能力。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_magicdef(randint1(base) + base, FALSE);
            }
        }
        break;

    case 24:
        if (name) return "移除所有诅咒";
        if (desc) return "解除已装备物品上的普通和重度诅咒。";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("你感觉好像有人在守护着你。");
                }
            }
        }
        break;

    case 25:
        if (name) return "纯化";
        if (desc) return "移除物品的所有魔法属性，包括其作为自我物品或神器的状态。经过魔法强化的自我物品无法被纯化。";

        {
            if (cast)
            {
                if (!mundane_spell(FALSE)) return NULL;
            }
        }
        break;

    case 26:
        if (name) return "真实知识";
        if (desc) return "*鉴定*一件物品（全面鉴定）。";

        {
            if (cast)
            {
                if (!identify_fully(NULL)) return NULL;
            }
        }
        break;

    case 27:
        if (name) return "附魔";
        if (desc) return "尝试增加武器的命中和伤害加值，或者增加护甲的防御(AC)加值。";

        if (cast)
        {
            if (!craft_enchant(15, 3))
                return NULL;
        }
        break;

    case 28:
        if (name) return "工艺";
        if (desc) return "使一件选定的武器、护甲或弹药变为Ego（有词缀的魔法）物品。";

        if (cast)
        {
            if (!cast_crafting())
                return NULL;
        }
        break;

    case 29:
        if (name) return "活体王牌";
        if (desc) return "赋予使你随机传送或能够随意传送的变异。";

        if (cast)
        {
            int mutation;

            if (one_in_(7) || dun_level == 0)
                mutation = MUT_TELEPORT;
            else
                mutation = MUT_TELEPORT_RND;

            if (mut_gain(mutation))
                msg_print("你变成了一个活体王牌。");
        }
        break;

    case 30:
        if (name) return "免疫";
        if (desc) return "在一段时间内赋予火焰、寒冷、闪电或酸液免疫。";

        {
            int base = spell_power(13);

            if (info) return info_duration(base, base);

            if (cast)
            {
                if (!choose_ele_immune(base + randint1(base))) return NULL;
            }
        }
        break;

    case 31:
        if (name) return "法力烙印";
        if (desc) return "暂时为你的武器附上原力(force)属性。";

        {
        int base = spell_power(plev / 4);

            if (info) return info_duration(base, base);
            if (cast)
            {
                set_tim_force(base + randint1(base), FALSE);
            }
        }
        break;
    }

    return "";
}


static cptr do_daemon_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    static const char s_dam[] = "dam ";

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "魔法飞弹";
        if (desc) return "发射一道微弱的魔法弹。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_MISSILE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "探测非生命体";
        if (desc) return "探测你附近所有的非活体怪物。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_nonliving(rad);
            }
        }
        break;

    case 2:
        if (name) return "邪恶祝福";
        if (desc) return "在几个回合内提供命中和防御(AC)加值。";

        {
            int base = spell_power(12);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_blessed(randint1(base) + base, FALSE);
            }
        }
        break;

    case 3:
        if (name) return "抵抗火焰";
        if (desc) return "在一段时间内赋予火焰抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_oppose_fire(randint1(base) + base, FALSE);
            }
        }
        break;

    case 4:
        if (name) return "惊骇";
        if (desc) return "尝试恐吓并震慑一个怪物。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
                stun_monster(dir, 5 + plev/5);
            }
        }
        break;

    case 5:
        if (name) return "地狱之矢";
        if (desc) return "发射一道地狱能量弹或射线。";

        {
            int dice = 6 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_NETHER,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 6:
        if (name) return "召唤劣魔";
        if (desc) return "召唤一只亡魂(manes)。";

        {
            if (cast)
            {
                if (!summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(plev * 3 / 2), SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
                {
                    msg_print("没有劣魔降临。");
                }
            }
        }
        break;

    case 7:
        if (name) return "地狱之焰";
        if (desc) return "发射一颗邪恶能量球。对善良阵营的怪物造成极大伤害。";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_MAGE ||
                p_ptr->pclass == CLASS_BLOOD_MAGE ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER ||
                p_ptr->pclass == CLASS_YELLOW_MAGE ||
                p_ptr->pclass == CLASS_GRAY_MAGE)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;


            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_HELL_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 8:
        if (name) return "支配恶魔";
        if (desc) return "尝试魅惑一只恶魔。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                control_one_demon(dir, power);
            }
        }
        break;

    case 9:
        if (name) return "视界";
        if (desc) return "绘制附近区域的地图。";

        {
            int rad = DETECT_RAD_MAP;

            if (info) return info_radius(rad);

            if (cast)
            {
                map_area(rad);
            }
        }
        break;

    case 10:
        if (name) return "抵抗地狱";
        if (desc) return "在一段时间内赋予地狱能量抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_res_nether(randint1(base) + base, FALSE);
            }
        }
        break;

    case 11:
        if (name) return "等离子之矢";
        if (desc) return "发射一道等离子魔法弹或射线。";

        {
            int dice = 11 + (plev - 5) / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance(),
                    GF_PLASMA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 12:
        if (name) return "火球";
        if (desc) return "发射一颗火球。";

        {
            int dam = spell_power(plev + 55 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;

    case 13:
        if (name) return "火焰烙印";
        if (desc) return "使当前武器附上火焰属性。";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_BRAND_FIRE, OF_RES_FIRE);
            }
        }
        break;

    case 14:
        if (name) return "地狱球";
        if (desc) return "发射一颗巨大的地狱球。";

        {
            int dam = spell_power(plev * 3 / 2 + 100 + p_ptr->to_d_spell);
            int rad = spell_power(plev / 20 + 2);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_NETHER, dir, dam, rad);
            }
        }
        break;

    case 15:
        if (name) return "召唤恶魔";
        if (desc) return "召唤一只恶魔。";

        {
            if (cast)
            {
                bool pet = !one_in_(3);
                u32b mode = 0L;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(plev*2/3+randint1(plev/2)), SUMMON_DEMON, mode))
                {
                    msg_print("这片区域弥漫着硫磺和烈火的恶臭。");


                    if (pet)
                    {
                        msg_print("'您有何吩咐……主人？'");
                    }
                    else
                    {
                        msg_print("'我绝不服从！可怜虫！我要吞噬你凡人的灵魂！'");
                    }
                }
                else
                {
                    msg_print("没有恶魔降临。");
                }
                break;
            }
        }
        break;

    case 16:
        if (name) return "恶魔之眼";
        if (desc) return "在一段时间内赋予心灵感应。";

        {
            int base = spell_power(30);
            int sides = 25;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_tim_esp(randint1(base) + sides, FALSE);
            }
        }
        break;

    case 17:
        if (name) return "恶魔斗篷";
        if (desc) return "赋予火焰、酸液和毒素抗性，并附带一层火焰光环。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;

                set_oppose_fire(dur, FALSE);
                set_oppose_acid(dur, FALSE);
                set_oppose_pois(dur, FALSE);
                set_tim_sh_fire(dur, FALSE);
                break;
            }
        }
        break;

    case 18:
        if (name) return "熔岩之流";
        if (desc) return "以你为中心生成一颗火球，能将地板转化为岩浆。";

        {
            int dam = spell_power((55 + plev + p_ptr->to_d_spell) * 2);
            int rad = 3;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                fire_ball(GF_FIRE, 0, dam, rad);
                fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), rad);
            }
        }
        break;

    case 19:
        if (name) return "等离子球";
        if (desc) return "发射一颗等离子球。";

        {
            int dam = spell_power(plev * 3 / 2 + 80 + p_ptr->to_d_spell);
            int rad = spell_power(2 + plev / 40);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;

    case 20:
        if (name) return "变形恶魔";
        if (desc) return "暂时模仿恶魔。失去原本种族的能力，并获得恶魔的能力。";

        {
            int base = spell_power(10 + plev / 2);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_DEMON, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("毫无效果。");
            }
        }
        break;

    case 21:
        if (name) return "地狱冲击波";
        if (desc) return "对视线内所有的怪物造成伤害。对善良阵营的怪物造成巨大伤害。";

        {
            if (info) return info_damage(1, spell_power(plev*2), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_monsters(spell_power(randint1(plev * 2) + p_ptr->to_d_spell));
                dispel_good(spell_power(randint1(plev * 2) + p_ptr->to_d_spell));
            }
        }
        break;

    case 22:
        if (name) return "魅魔之吻";
        if (desc) return "发射一颗时空(nexus)球。";

        {
            int dam = spell_power(75 + plev * 3 + p_ptr->to_d_spell);
            int rad = 4;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_NEXUS, dir, dam, rad);
            }
        }
        break;

    case 23:
        if (name) return "末日之手";
        if (desc) return "尝试使目标怪物受到致命伤，吸取它们剩余生命值的很大一部分。";

        {
            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                else msg_print("你唤起了末日之手！");

                fire_ball_hide(GF_HAND_DOOM, dir, spell_power(plev * 3), 0);
            }
        }
        break;

    case 24:
        if (name) return "提振士气";
        if (desc) return "在一段时间内提供命中加值和 10 点额外生命值。";

        {
            int base = spell_power(25);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
            }
        }
        break;

    case 25:
        if (name) return "不朽之躯";
        if (desc) return "在一段时间内赋予时间抗性。";

        {
            int base = spell_power(20);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_res_time(randint1(base)+base, FALSE);
            }
        }
        break;

    case 26:
        if (name) return "疯狂法阵";
        if (desc) return "以你为中心生成混沌球、混乱球和魅惑球。";

        {
            int dam = spell_power(50 + plev + p_ptr->to_d_spell);
            int power = spell_power(20 + plev);
            int rad = spell_power(3 + plev / 20);

            if (info) return format("%s%d+%d", s_dam, dam/2, dam/2);

            if (cast)
            {
                fire_ball(GF_CHAOS, 0, dam, rad);
                fire_ball(GF_CONFUSION, 0, dam, rad);
                fire_ball(GF_CHARM, 0, power, rad);
            }
        }
        break;

    case 27:
        if (name) return "引爆宠物";
        if (desc) return "使你所有的宠物自爆。";

        {
            if (cast)
            {
                discharge_minion();
            }
        }
        break;

    case 28:
        if (name) return "召唤高阶恶魔";
        if (desc) return "召唤高等恶魔。需要献祭一具人类尸体（'p'，'h' 或 't'）。";

        {
            if (cast)
            {
                if (!cast_summon_greater_demon()) return NULL;
            }
        }
        break;

    case 29:
        if (name) return "地狱之火";
        if (desc) return "发射一颗强大的邪恶能量球。对善良阵营的怪物造成极大伤害。";

        {
            int dam = spell_power(666 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_HELL_FIRE, dir, dam, rad);
                take_hit(DAMAGE_USELIFE, 20 + randint1(30), "施展地狱火的反噬");
            }
        }
        break;

    case 30:
        if (name) return "送入地狱";
        if (desc) return "尝试将一个怪物直接送入地狱（秒杀）。";

        {
            int power = 666;

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball_hide(GF_GENOCIDE, dir, power, 0);
            }
        }
        break;

    case 31:
        if (name) return "变形恶魔领主";
        if (desc) return "暂时模仿恶魔领主。失去原本种族的能力，并获得恶魔领主的强大能力。即使是坚硬的岩墙也无法阻挡你的脚步。";

        {
            int base = spell_power(15);

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_mimic(base + randint1(base), MIMIC_DEMON_LORD, FALSE);
                if (p_ptr->mimic_form == MIMIC_NONE) msg_print("毫无效果。");
            }
        }
        break;
    }

    return "";
}


static cptr do_crusade_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "惩戒";
        if (desc) return "发射一道闪电魔法弹或射线。";

        {
            int dice = 3 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt_or_beam(
                    beam_chance() - 10,
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 1:
        if (name) return "探测邪恶";
        if (desc) return "探测你附近所有的邪恶怪物。";

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cast)
            {
                detect_monsters_evil(rad);
            }
        }
        break;

    case 2:
        if (name) return "移除恐惧";
        if (desc) return "消除恐惧。";

        if (cast)
            fear_clear_p();
        break;

    case 3:
        if (name) return "恐吓怪物";
        if (desc) return "尝试恐吓一个怪物。";

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fear_monster(dir, power);
            }
        }
        break;

    case 4:
        if (name) return "避难所";
        if (desc) return "尝试催眠相邻方格内的怪物。";

        {
            int power = plev;

            if (info) return info_power(power);

            if (cast)
            {
                sleep_monsters_touch();
            }
        }
        break;

    case 5:
        if (name) return "传送门";
        if (desc) return "中距离传送。";

        {
            int range = 25 + plev / 2;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(range, 0L);
            }
        }
        break;

    case 6:
        if (name) return "星尘";
        if (desc) return "向目标附近发射多道闪光弹。";

        {
            int dice = spell_power(3 + (plev - 1) / 9);
            int sides = 2;

            if (info) return info_multi_damage_dice(dice, sides);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_blast(GF_LITE, dir, dice, sides, 10, 3);
            }
        }
        break;

    case 7:
        if (name) return "净化";
        if (desc) return "治愈割伤和震慑，并减轻中毒程度。";

        {
            if (cast)
            {
                set_cut(0, TRUE);
                set_poisoned(p_ptr->poisoned - MAX(50, p_ptr->poisoned / 3), TRUE);
                set_stun(0, TRUE);
            }
        }
        break;

    case 8:
        if (name) return "驱散邪恶";
        if (desc) return "尝试将一个邪恶怪物传送走。";

        {
            int power = MAX_SIGHT * 5;

            if (info) return info_dist(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_AWAY_EVIL, dir, power, 0);
            }
        }
        break;

    case 9:
        if (name) return "神圣法球";
        if (desc) return "发射一颗神圣能量球。对邪恶怪物造成极大伤害，但不会影响善良怪物。";

        {
            int dice = 3;
            int sides = 6;
            int rad = (plev < 30) ? 2 : 3;
            int base;

            if (p_ptr->pclass == CLASS_PRIEST ||
                p_ptr->pclass == CLASS_HIGH_MAGE ||
                p_ptr->pclass == CLASS_SORCERER)
                base = plev + plev / 2;
            else
                base = plev + plev / 4;

            if (info) return info_damage(dice, spell_power(sides), spell_power(base + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(
                    GF_HOLY_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + base + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 10:
        if (name) return "驱魔";
        if (desc) return "对视线内的所有不死生物和恶魔造成伤害，并恐吓视线内的所有邪恶怪物。";

        {
            int sides = plev;
            int power = plev;

            if (info) return info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                dispel_undead(spell_power(randint1(sides) + p_ptr->to_d_spell));
                dispel_demons(spell_power(randint1(sides) + p_ptr->to_d_spell));
                turn_evil(power);
            }
        }
        break;

    case 11:
        if (name) return "移除诅咒";
        if (desc) return "解除已装备物品上的普通诅咒。";

        {
            if (cast)
            {
                if (remove_curse())
                {
                    msg_print("你感觉好像有人在守护着你。");
                }
            }
        }
        break;

    case 12:
        if (name) return "感知隐形";
        if (desc) return "在一段时间内赋予识破隐形。";

        {
            int base = 24;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_invis(randint1(base) + base, FALSE);
            }
        }
        break;

    case 13:
        if (name) return "防护邪恶";
        if (desc) return "赋予一种能保护你免受邪恶怪物物理攻击的光环。";

        {
            int base = 25;
            int sides = 3 * plev;

            if (info) return info_duration(base, sides);

            if (cast)
            {
                set_protevil(randint1(sides) + sides, FALSE);
            }
        }
        break;

    case 14:
        if (name) return "制裁之雷";
        if (desc) return "发射一道强大的闪电魔法弹。";

        {
            int dam = spell_power(plev * 5 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(GF_ELEC, dir, dam);
            }
        }
        break;

    case 15:
        if (name) return "圣言";
        if (desc) return "对视线内的所有邪恶怪物造成伤害，恢复部分生命值，并完全治愈中毒、震慑和割伤状态。";

        {
            int dam_sides = plev * 6;
            int heal = spell_power(100);

            if (info) return format("伤害:d%d/治疗:%d", spell_power(dam_sides), heal);

            if (cast)
            {
                dispel_evil(spell_power(randint1(dam_sides) + p_ptr->to_d_spell));
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(heal);
                set_stun(0, TRUE);
                set_cut(0, TRUE);
				set_poisoned(0, TRUE);
            }
        }
        break;

    case 16:
        if (name) return "开启通路";
        if (desc) return "发射一道能摧毁陷阱和门的射线。";

        {
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;

                destroy_door(dir);
            }
        }
        break;

    case 17:
        if (name) return "拘捕";
        if (desc) return "尝试麻痹一个邪恶怪物。";

        {
            int power = spell_power(plev * 2);

            if (info) return info_power(power);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                stasis_evil(dir);
            }
        }
        break;

    case 18:
        if (name) return "天使斗篷";
        if (desc) return "获得对酸液、寒冷和闪电的抵抗力。并为你提供神圣力量的光环，在一段时间内对攻击你的邪恶怪物造成伤害。";

        {
            int base = 20;

            if (info) return info_duration(base, base);

            if (cast)
            {
                int dur = randint1(base) + base;
                set_oppose_acid(dur, FALSE);
                set_oppose_cold(dur, FALSE);
                set_oppose_elec(dur, FALSE);
                set_tim_sh_holy(dur, FALSE);
            }
        }
        break;

    case 19:
        if (name) return "驱散不死生物与恶魔";
        if (desc) return "对视线内的所有不死生物和恶魔造成伤害。";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                dispel_undead(dam);
                dispel_demons(dam);
            }
        }
        break;

    case 20:
        if (name) return "驱散邪恶";
        if (desc) return "对视线内的所有邪恶怪物造成伤害。";

        {
            int dam = spell_power(plev * 3 + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                dispel_evil(dam);
            }
        }
        break;

    case 21:
        if (name) return "神圣之刃";
        if (desc) return "使当前武器对邪恶怪物特别致命。";

        {
            if (cast)
            {
                brand_weapon_slaying(OF_SLAY_EVIL, OF_INVALID);
            }
        }
        break;

    case 22:
        if (name) return "星爆";
        if (desc) return "发射一颗巨大的强光球。";

        {
            int dam = 100 + py_prorata_level_aux(200, 1, 1, 2);
            int rad = spell_power(4);

            dam = spell_power(dam + p_ptr->to_d_spell);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(GF_LITE, dir, dam, rad);
            }
        }
        break;

    case 23:
        if (name) return "召唤天使";
        if (desc) return "召唤一名天使。";

        {
            if (cast)
            {
                bool pet = !one_in_(3);
                u32b mode = 0L;

                if (pet) mode |= PM_FORCE_PET;
                else mode |= PM_NO_PET;
                if (!(pet && (plev < 50))) mode |= PM_ALLOW_GROUP;

                if (summon_specific(SUMMON_WHO_PLAYER, py, px, (plev * 3) / 2, SUMMON_ANGEL, mode))
                {
                    if (pet)
                    {
                        msg_print("'您有何吩咐……主人？'");
                    }
                    else
                    {
                        msg_print("凡人！为你的不敬忏悔吧。");
                    }
                }
            }
        }
        break;

    case 24:
        if (name) return "英雄气概";
        if (desc) return "消除恐惧，并在一段时间内提供命中加值和 10 点额外生命值。";

        {
            int base = 25;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_hero(randint1(base) + base, FALSE);
                hp_player(10);
            }
        }
        break;

    case 25:
        if (name) return "驱除诅咒";
        if (desc) return "解除已装备物品上的普通和重度诅咒。";

        {
            if (cast)
            {
                if (remove_all_curse())
                {
                    msg_print("你感觉好像有人在守护着你。");
                }
            }
        }
        break;

    case 26:
        if (name) return "驱逐邪恶";
        if (desc) return "除非被抵抗，否则将视线内的所有邪恶怪物传送走。";

        {
            int power = spell_power(100);

            if (info) return info_power(power);

            if (cast)
            {
                if (banish_evil(power))
                {
                    msg_print("神圣的力量驱逐了邪恶！");

                }
            }
        }
        break;

    case 27:
        if (name) return "末日审判";
        if (desc) return "摧毁附近区域内的一切。";

        {
            int base = 12;
            int sides = 4;

            if (cast)
            {
                destroy_area(py, px, base + randint1(sides), spell_power(4 * plev));
            }
        }
        break;

    case 28:
        if (name) return "以眼还眼";
        if (desc) return "在一段时间内赋予特殊光环。当你受到怪物攻击时，怪物会承受与你所受伤害等量的反弹伤害。";

        {
            int base = 10;

            if (info) return info_duration(base, base);

            if (cast)
            {
                set_tim_eyeeye(randint1(base) + base, FALSE);
            }
        }
        break;

    case 29:
        if (name) return "神之愤怒";
        if (desc) return "在目标附近落下大量分解球。";

        {
            int dam = spell_power(plev * 3 + 25 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_multi_damage(dam);

            if (cast)
            {
                if (!cast_wrath_of_the_god(dam, rad)) return NULL;
            }
        }
        break;

    case 30:
        if (name) return "神圣干预";
        if (desc) return "以神圣力量对所有相邻怪物造成伤害。对视线内的所有怪物造成伤害，并尝试使其减速、震慑、困惑、恐惧和冻结。同时恢复生命值。";

        {
            int b_dam = spell_power(plev * 11);
            int d_dam = spell_power(plev * 4 + p_ptr->to_d_spell);
            int heal = spell_power(100);
            int power = spell_power(plev * 4);

            if (info) return format("治疗:%d/伤害:%d+%d", heal, d_dam, b_dam/2);

            if (cast)
            {
                project(0, 1, py, px, b_dam, GF_HOLY_FIRE, PROJECT_KILL);
                dispel_monsters(d_dam);
                slow_monsters(power);
                stun_monsters(5 + plev/5);
                confuse_monsters(power);
                turn_monsters(power);
                stasis_monsters(power/3);
                if (p_ptr->pclass != CLASS_BLOOD_MAGE)
                    hp_player(heal);
            }
        }
        break;

    case 31:
        if (name) return "圣战";
        if (desc) return "尝试魅惑视线内的所有善良怪物，恐吓所有未被魅惑的怪物，召唤大量骑士，并赋予英雄气概、祝福、加速以及邪恶防护。";
        if (cast) cast_crusade();
        break;
    }

    return "";
}


static cptr do_music_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;
    bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
    bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

    int dir;
    int plev = p_ptr->lev;

    switch (spell)
    {
    case 0:
        if (name) return "定身之歌";
        if (desc) return "尝试使视线内的所有怪物减速。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始哼唱一段缓慢、平稳的旋律……");
            bard_start_singing(spell, MUSIC_SLOW);
        }

        {
            int power = plev;

            if (info) return info_power(power);

            if (cont)
            {
                slow_monsters(power);
            }
        }
        break;

    case 1:
        if (name) return "祝福之歌";
        if (desc) return "在几个回合内提供命中和防御(AC)加值。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("埃努大乐章的神圣力量进入了你的体内……");
            bard_start_singing(spell, MUSIC_BLESS);
        }

        if (stop)
        {
            if (!p_ptr->blessed)
            {
                msg_print("祈祷的效果已经消失。");
            }
        }

        break;

    case 2:
        if (name) return "破坏音符";
        if (desc) return "发射一道音波弹。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        {
            int dice = 4 + (plev - 1) / 5;
            int sides = 4;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_bolt(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 3:
        if (name) return "震慑旋律";
        if (desc) return "尝试震慑视线内所有的怪物。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段音符来迷惑和震晕敌人……");
            bard_start_singing(spell, MUSIC_STUN);
        }

        {
            int dice = spell_power(plev / 10);
            int sides = 2;

            if (info) return info_power_dice(dice, sides);

            if (cont)
            {
                stun_monsters(damroll(dice, sides));
            }
        }

        break;

    case 4:
        if (name) return "生命之流";
        if (desc) return "轻微恢复生命值。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("当你唱起治愈之歌时，生命力流经你的全身……");
            bard_start_singing(spell, MUSIC_L_LIFE);
        }

        {
            int dice = 2;
            int sides = spell_power(6);

            if (info) return info_heal(dice, sides, 0);

            if (cont)
            {
                hp_player(damroll(dice, sides));
            }
        }

        break;

    case 5:
        if (name) return "太阳之歌";
        if (desc) return "永久照亮附近区域和房间内部。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        {
            int dice = 2;
            int sides = plev / 2;
            int rad = plev / 10 + 1;

            if (info) return info_damage(dice, sides, 0);

            if (cast)
            {
                msg_print("你激昂的歌声为黑暗的地方带来了光明……");

                lite_area(damroll(dice, sides), rad);
            }
        }
        break;

    case 6:
        if (name) return "恐惧之歌";
        if (desc) return "尝试恐吓视线内所有的怪物。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始编织出一段令人恐惧的旋律……");
            bard_start_singing(spell, MUSIC_FEAR);
        }

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cont)
            {
                project_hack(GF_TURN_ALL, power);
            }
        }

        break;

    case 7:
        if (name) return "英雄战歌";
        if (desc) return "消除恐惧，并在一段时间内提供命中加值和 10 点额外生命值。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始唱起一首激昂的战歌……");

            (void)hp_player(10);
            fear_clear_p();

            /* Recalculate hitpoints */
            p_ptr->update |= (PU_HP);

            bard_start_singing(spell, MUSIC_HERO);
        }

        if (stop)
        {
            if (!p_ptr->hero)
            {
                msg_print("英雄气概消失了。");
                /* Recalculate hitpoints */
                p_ptr->update |= (PU_HP);
            }
        }

        break;

    case 8:
        if (name) return "顺风耳";
        if (desc) return "探测你附近的陷阱、门和楼梯。在15级时能探测所有怪物，20级时探测宝藏和物品，25级时绘制附近区域地图，40级时照亮并知晓整个楼层。只要这首歌继续演唱，这些效果就会逐回合发生。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你宁静的音乐使你的听觉变得敏锐……");

            /* Hack -- Initialize the turn count */
            p_ptr->magic_num1[2] = 0;

            bard_start_singing(spell, MUSIC_DETECT);
        }

        {
            int rad = DETECT_RAD_DEFAULT;

            if (info) return info_radius(rad);

            if (cont)
            {
                int count = p_ptr->magic_num1[2];

                if (count >= 19) wiz_lite(FALSE);
                if (count >= 11)
                {
                    map_area(rad);
                    if (plev > 39 && count < 19)
                        p_ptr->magic_num1[2] = count + 1;
                }
                if (count >= 6)
                {
                    /* There are too many hidden treasure. So... */
                    /* detect_treasure(rad); */
                    detect_objects_gold(rad);
                    detect_objects_normal(rad);

                    if (plev > 24 && count < 11)
                        p_ptr->magic_num1[2] = count + 1;
                }
                if (count >= 3)
                {
                    detect_monsters_invis(rad);
                    detect_monsters_normal(rad);

                    if (plev > 19 && count < 6)
                        p_ptr->magic_num1[2] = count + 1;
                }
                detect_traps(rad, TRUE);
                detect_doors(rad);
                detect_stairs(rad);

                if (plev > 14 && count < 3)
                    p_ptr->magic_num1[2] = count + 1;
            }
        }

        break;

    case 9:
        if (name) return "灵魂尖啸";
        if (desc) return "对视线内所有的怪物造成灵能伤害。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始唱起一首灵魂痛苦之歌……");
            bard_start_singing(spell, MUSIC_PSI);
        }

        {
            int dice = 1;
            int sides = plev * 3 / 2;

            if (info) return info_damage(dice, spell_power(sides), spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                project_hack(
                    GF_PSI,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }

        break;

    case 10:
        if (name) return "学识之歌";
        if (desc) return "鉴定所有相邻方格内的物品。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你回想起了这个世界丰富的学识……");
            bard_start_singing(spell, MUSIC_ID);
        }

        {
            int rad = 1;

            if (info) return info_radius(rad);

            if (cont || cast)
            {
                project(0, rad, py, px, 0, GF_IDENTIFY, PROJECT_ITEM);
            }
        }

        break;

    case 11:
        if (name) return "隐匿曲调";
        if (desc) return "赋予提升的潜行能力。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你的歌声将你带离凡人视线之外……");
            bard_start_singing(spell, MUSIC_STEALTH);
        }

        if (stop)
        {
            if (!p_ptr->tim_stealth)
            {
                msg_print("你不再隐匿了。");
            }
        }

        break;

    case 12:
        if (name) return "幻象旋律";
        if (desc) return "尝试使视线内所有的怪物陷入混乱。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段音符来欺骗和困惑敌人……");
            bard_start_singing(spell, MUSIC_CONF);
        }

        {
            int power = plev * 2;

            if (info) return info_power(power);

            if (cont)
            {
                confuse_monsters(power);
            }
        }

        break;

    case 13:
        if (name) return "厄运召唤";
        if (desc) return "用轰鸣的声波对视线内所有的怪物造成伤害。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("努曼诺尔沦亡的狂怒爆发了……");
            bard_start_singing(spell, MUSIC_SOUND);
        }

        {
            int dice = 10 + plev / 5;
            int sides = 7;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                project_hack(
                    GF_SOUND,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }

        break;

    case 14:
        if (name) return "菲瑞尔之歌";
        if (desc) return "复活附近的尸体和骸骨，并让它们成为你的宠物。";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("生命与复苏的主题交织在你的歌声中……");

                animate_dead(0, py, px);
            }
        }
        break;

    case 15:
        if (name) return "团契圣歌";
        if (desc) return "尝试魅惑视线内所有的怪物。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段缓慢、舒缓的恳求旋律……");
            bard_start_singing(spell, MUSIC_CHARM);
        }

        {
            int dice = spell_power(10 + plev / 15);
            int sides = 6;

            if (info) return info_power_dice(dice, sides);

            if (cont)
            {
                charm_monsters(damroll(dice, sides));
            }
        }

        break;

    case 16:
        if (name) return "解离之音";
        if (desc) return "使你能够挖掘(burrow)进入墙壁。你脚下的物品会蒸发消失。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段狂暴的音符来破坏墙壁。");
            bard_start_singing(spell, MUSIC_WALL);
        }

        {
            if (cont || cast)
            {
                project(0, 0, py, px,
                    0, GF_DISINTEGRATE, PROJECT_KILL | PROJECT_ITEM | PROJECT_HIDE);
            }
        }
        break;

    case 17:
        if (name) return "芬罗德的抵抗";
        if (desc) return "赋予火焰、寒冷、闪电、酸液和毒素抗性。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你唱起一首对抗强权的坚韧之歌……");
            bard_start_singing(spell, MUSIC_RESIST);
        }

        if (stop)
        {
            if (!p_ptr->oppose_acid)
            {
                msg_print("你对酸液的抵抗力减弱了。");
            }

            if (!p_ptr->oppose_elec)
            {
                msg_print("你对闪电的抵抗力减弱了。");
            }

            if (!p_ptr->oppose_fire)
            {
                msg_print("你对火焰的抵抗力减弱了。");
            }

            if (!p_ptr->oppose_cold)
            {
                msg_print("你对寒冷的抵抗力减弱了。");
            }

            if (!p_ptr->oppose_pois)
            {
                msg_print("你对毒素的抵抗力减弱了。");
            }
        }

        break;

    case 18:
        if (name) return "霍比特旋律";
        if (desc) return "使你加速。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始唱起欢快的流行歌曲……");
            bard_start_singing(spell, MUSIC_SPEED);
        }

        if (stop)
        {
            if (!p_ptr->fast)
            {
                msg_print("你感觉自己变慢了。");
            }
        }

        break;

    case 19:
        if (name) return "世界扭曲";
        if (desc) return "除非被抵抗，否则将附近所有的怪物传送走。";

        {
            int rad = spell_power(plev / 15 + 1);
            int power = spell_power(plev * 3 + 1);

            if (info) return info_radius(rad);

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("当你唱起令人目眩的旋律时，现实开始疯狂旋转……");

                project(0, rad, py, px, power, GF_AWAY_ALL, PROJECT_KILL);
            }
        }
        break;

    case 20:
        if (name) return "驱散圣歌";
        if (desc) return "对视线内所有的怪物造成伤害。对邪恶阵营的怪物造成巨大伤害。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你发出震耳欲聋的尖叫声……");
            bard_start_singing(spell, MUSIC_DISPEL);
        }

        {
            int m_sides = plev * 3;
            int e_sides = plev * 3;

            if (info) return info_damage(1, spell_power(m_sides), spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                dispel_monsters(spell_power(randint1(m_sides) + p_ptr->to_d_spell));
                dispel_evil(spell_power(randint1(e_sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 21:
        if (name) return "萨鲁曼之声";
        if (desc) return "尝试使视线内所有的怪物减速并催眠它们。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你开始哼唱一首温柔而迷人的歌……");
            bard_start_singing(spell, MUSIC_SARUMAN);
        }

        {
            int power = spell_power(plev);

            if (info) return info_power(power);

            if (cont)
            {
                slow_monsters(power);
                sleep_monsters(power);
            }
        }

        break;

    case 22:
        if (name) return "风暴之歌";
        if (desc) return "发射一道音波射线。";

        {
            int dice = 15 + (plev - 1) / 2;
            int sides = 10;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_beam(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;

    case 23:
        if (name) return "安巴坎塔";
        if (desc) return "重新生成当前的地下城楼层（改变楼层）。";

        {
            int base = 15;
            int sides = 20;

            if (info) return info_delay(base, sides);

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("你歌唱着中土世界的太古重塑……");

                alter_reality();
            }
        }
        break;

    case 24:
        if (name) return "破坏旋律";
        if (desc) return "震动地下城结构，导致地板和墙壁随机交换。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段音符来扭曲和粉碎……");
            bard_start_singing(spell, MUSIC_QUAKE);
        }

        {
            int rad = 10;

            if (info) return info_radius(rad);

            if (cont)
            {
                earthquake(py, px, 10);
            }
        }

        break;


    case 25:
        if (name) return "停滞尖啸";
        if (desc) return "尝试冻结视线内所有的怪物。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你编织出一段极其缓慢、几乎要停滞的旋律……");
            bard_start_singing(spell, MUSIC_STASIS);
        }

        {
            int power = spell_power(plev * 3 + 10);

            if (info) return info_power(power);

            if (cont)
            {
                stasis_monsters(power);
            }
        }

        break;

    case 26:
        if (name) return "耐力";
        if (desc) return "在你脚下的地板上设置一个结界符文。当你在结界上时怪物无法攻击你，但它们会尝试破坏结界。";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("乐章的神圣力量正在创造神圣领域……");

                warding_glyph();
            }
        }
        break;

    case 27:
        if (name) return "英雄诗篇";
        if (desc) return "使你加速。赋予英雄气概。对视线内所有的怪物造成伤害。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("你吟唱起一声强大、英雄般的武装号召……");
            (void)hp_player(10);

            /* Recalculate hitpoints */
            p_ptr->update |= (PU_HP);

            bard_start_singing(spell, MUSIC_SHERO);
        }

        if (stop)
        {
            if (!p_ptr->hero)
            {
                msg_print("英雄气概消失了。");
                /* Recalculate hitpoints */
                p_ptr->update |= (PU_HP);
            }

            if (!p_ptr->fast)
            {
                msg_print("你感觉自己变慢了。");
            }
        }

        {
            int dice = 1;
            int sides = plev * 3;

            if (info) return info_damage(dice, sides, spell_power(p_ptr->to_d_spell));

            if (cont)
            {
                dispel_monsters(spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
            }
        }
        break;

    case 28:
        if (name) return "雅凡娜的慰藉";
        if (desc) return "强大的治疗之歌。同时完全治愈割伤与震慑。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
            msg_print("当你唱起这首歌时，生命力流经你的全身……");
            bard_start_singing(spell, MUSIC_H_LIFE);
        }

        {
            int dice = spell_power(15);
            int sides = 10;

            if (info) return info_heal(dice, sides, 0);

            if (cont)
            {
                hp_player(damroll(dice, sides));
                set_stun(0, TRUE);
                set_cut(0, TRUE);
            }
        }

        break;

    case 29:
        if (name) return "女神重生";
        if (desc) return "恢复所有属性与经验值。";

        {
            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                msg_print("当你歌唱时，你在黑暗中播撒光明与美丽。你感到神清气爽。");
                (void)do_res_stat(A_STR);
                (void)do_res_stat(A_INT);
                (void)do_res_stat(A_WIS);
                (void)do_res_stat(A_DEX);
                (void)do_res_stat(A_CON);
                (void)do_res_stat(A_CHR);
                (void)restore_level();
                lp_player(1000);
            }
        }
        break;

    case 30:
        if (name) return "索伦的巫术";
        if (desc) return "发射一颗威力极其强大的微小音波球。";

        {
            int dice = 50 + plev;
            int sides = 10;
            int rad = 0;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            /* Stop singing before start another */
            if (cast || fail) bard_stop_singing();

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;

                fire_ball(
                    GF_SOUND,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell),
                    rad
                );
            }
        }
        break;

    case 31:
        if (name) return "芬国昐的挑战";
        if (desc) return "使你暂时对绝大多数攻击无敌。当无敌状态结束时会消耗一个额外的回合。";

        /* Stop singing before start another */
        if (cast || fail) bard_stop_singing();

        if (cast)
        {
                msg_print("你回想起了芬国昐向黑魔王挑战时的英勇……");

                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

                bard_start_singing(spell, MUSIC_INVULN);
        }

        if (stop)
        {
            if (!p_ptr->invuln)
            {
                msg_print("无敌状态消失了。");
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

                /* Take an extra turn */
                p_ptr->energy_need += PY_ENERGY_NEED();
            }
        }

        break;
    }

    return "";
}



/* Hex */
static bool item_tester_hook_weapon_except_bow(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

static cptr do_hex_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
    bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

    bool add = TRUE;

    int plev = p_ptr->lev;
    int power;

    switch (spell)
    {
    /*** 1st book (0-7) ***/
    case 0:
        if (name) return "邪恶祝福";
        if (desc) return "尝试增加武器的命中加值和防御(AC)加值。";
        if (cast)
        {
            if (!p_ptr->blessed)
            {
                msg_print("你感觉充满了正义感！");
            }
        }
        if (stop)
        {
            if (!p_ptr->blessed)
            {
                msg_print("祈祷的效果已经消失。");
            }
        }
        break;

    case 1:
        if (name) return "治疗轻伤";
        if (desc) return "轻微治愈割伤并恢复少量生命值。";
        if (info) return info_heal(1, 10, 0);
        if (cast)
        {
            msg_print("你感觉越来越好了。");
        }
        if (cast || cont)
        {
            hp_player(damroll(1, 10));
            set_cut(p_ptr->cut - 10, TRUE);
        }
        break;

    case 2:
        if (name) return "恶魔光环";
        if (desc) return "赋予火焰光环和生命再生能力。";
        if (cast)
        {
            msg_print("你被火焰光环包围了！");
        }
        if (stop)
        {
            msg_print("火焰光环消失了。");
        }
        break;

    case 3:
        if (name) return "恶臭迷雾";
        if (desc) return "对视线内所有的怪物造成少量的毒素伤害。";
        power = plev / 2 + 5 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_POIS, randint1(power));
        }
        break;

    case 4:
        if (name) return "额外力量";
        if (desc) return "尝试提升你的力量。";
        if (cast)
        {
            msg_print("你感觉自己变强了。");
        }
        break;

    case 5:
        if (name) return "诅咒武器";
        if (desc) return "诅咒你的武器。";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            char o_name[MAX_NLEN];
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "你要诅咒哪把武器？";
            prompt.error = "你没有装备任何武器。";
            prompt.filter = item_tester_hook_weapon_except_bow;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            object_desc(o_name, prompt.obj, OD_NAME_ONLY);
            obj_flags(prompt.obj, f);

            if (!get_check(format("你真的要诅咒%s吗？", o_name))) return FALSE;

            if (!one_in_(3) &&
                (object_is_artifact(prompt.obj) || have_flag(f, OF_BLESSED)))
            {
                msg_format("%s抵抗了效果。", o_name);
                if (one_in_(3))
                {
                    if (prompt.obj->to_d > 0)
                    {
                        prompt.obj->to_d -= randint1(3) % 2;
                        if (prompt.obj->to_d < 0) prompt.obj->to_d = 0;
                    }
                    if (prompt.obj->to_h > 0)
                    {
                        prompt.obj->to_h -= randint1(3) % 2;
                        if (prompt.obj->to_h < 0) prompt.obj->to_h = 0;
                    }
                    if (prompt.obj->to_a > 0)
                    {
                        prompt.obj->to_a -= randint1(3) % 2;
                        if (prompt.obj->to_a < 0) prompt.obj->to_a = 0;
                    }
                    msg_format("你的%s被解除了魔法！", o_name);
                }
            }
            else
            {
                int power = 0;
                msg_format("一阵可怕的黑色光环冲击了你的%s！", o_name);
                prompt.obj->curse_flags |= (OFC_CURSED);

                if (object_is_artifact(prompt.obj) || object_is_ego(prompt.obj))
                {

                    if (one_in_(3)) prompt.obj->curse_flags |= (OFC_HEAVY_CURSE);
                    if (one_in_(666))
                    {
                        prompt.obj->curse_flags |= (OFC_TY_CURSE);
                        if (one_in_(666)) prompt.obj->curse_flags |= (OFC_PERMA_CURSE);

                        add_flag(prompt.obj->flags, OF_AGGRAVATE);
                        add_flag(prompt.obj->flags, OF_VORPAL);
                        add_flag(prompt.obj->flags, OF_BRAND_VAMP);
                        msg_print("血，鲜血，鲜血！");
                        power = 2;
                    }
                }

                /* Clouded says getting actual bad curses on Hex objects is annoying,
                 * so we only rarely do it */
                if ((power > 0) || (one_in_(26))) prompt.obj->curse_flags |= get_curse(power, prompt.obj);
                else if (one_in_(2)) prompt.obj->curse_flags |= (OFC_ALLERGY);
            }

            p_ptr->update |= (PU_BONUS);
            add = FALSE;
        }
        break;

    case 6:
        if (name) return "邪恶探测";
        if (desc) return "探测邪恶怪物。";
        if (info) return info_range(MAX_SIGHT);
        if (cast)
        {
            msg_print("你注意到了邪恶生物的存在。");
        }
        break;

    case 7:
        if (name) return "忍耐";
        if (desc) return "在承受伤害后的几回合内爆发出强烈的地狱火。";
        power = MIN(200, p_ptr->magic_num1[2] * 2 + p_ptr->to_d_spell);
        if (info) return info_damage(0, 0, power);
        if (cast)
        {
            int a = 3 - (p_ptr->pspeed - 100) / 10;
            int r = 3 + randint1(3) + MAX(0, MIN(3, a));

            if (p_ptr->magic_num2[2] > 0)
            {
                msg_print("你已经在忍耐了。");
                return NULL;
            }

            p_ptr->magic_num2[1] = 1;
            p_ptr->magic_num2[2] = r;
            p_ptr->magic_num1[2] = 0;
            msg_print("你决定忍受所有的伤害。");
            add = FALSE;
        }
        if (cont)
        {
            int rad = 2 + (power / 50);

            p_ptr->magic_num2[2]--;

            if ((p_ptr->magic_num2[2] <= 0) || (power >= 200))
            {
                msg_print("忍耐的时间结束了！");
                if (power)
                {
                    project(0, rad, py, px, power, GF_HELL_FIRE,
                        (PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
                }
                if (p_ptr->wizard || easy_damage)
                {
                    msg_format("你返还了%d点伤害。", power);
                }

                /* Reset */
                p_ptr->magic_num2[1] = 0;
                p_ptr->magic_num2[2] = 0;
                p_ptr->magic_num1[2] = 0;
            }
        }
        break;

    /*** 2nd book (8-15) ***/
    case 8:
        if (name) return "寒冰装甲";
        if (desc) return "赋予寒冰光环和防御(AC)加值。";
        if (cast)
        {
            msg_print("你被寒冰装甲包围了！");
        }
        if (stop)
        {
            msg_print("寒冰装甲消失了。");
        }
        break;

    case 9:
        if (name) return "治疗重伤";
        if (desc) return "较多地治愈割伤并恢复生命值。";
        if (info) return info_heal(2, 10, 0);
        if (cast)
        {
            msg_print("你感觉越来越好了。");
        }
        if (cast || cont)
        {
            hp_player(damroll(2, 10));
            set_cut((p_ptr->cut / 2) - 10, TRUE);
        }
        break;

    case 10:
        if (name) return "吸入药水";
        if (desc) return "喝下药水而不会中断施法。";
        if (cast)
        {
            p_ptr->magic_num1[0] |= (1L << HEX_INHAIL);
            do_cmd_quaff_potion();
            p_ptr->magic_num1[0] &= ~(1L << HEX_INHAIL);
            add = FALSE;
        }
        break;

    case 11:
        if (name) return "吸血迷雾";
        if (desc) return "对视线内所有的怪物造成少量的吸血(drain life)伤害。";
        power = (plev / 2) + 5 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_OLD_DRAIN, randint1(power));
        }
        break;

    case 12:
        if (name) return "武器化为符文剑";
        if (desc) return "赋予你的武器斩首(vorpal)能力。根据你武器上的诅咒来增加武器的伤害。";
        if (cast)
        {
            if (p_ptr->weapon_ct > 1)
                msg_print("你的武器闪烁着明亮的黑光。");
            else
                msg_print("你的武器闪烁着明亮的黑光。");
        }
        if (stop)
            msg_format("武器%s的光芒消失了。", (p_ptr->weapon_ct <= 1) ? "" : "s");
        break;

    case 13:
        if (name) return "困惑之触";
        if (desc) return "当你攻击时使一个怪物混乱。";
        if (cast)
        {
            msg_print("你的双手闪烁着明亮的红光。");
        }
        if (stop)
        {
            msg_print("你手上的光芒消失了。");
        }
        break;

    case 14:
        if (name) return "强化体格";
        if (desc) return "尝试提升你的力量、敏捷和体质。";
        if (cast)
        {
            msg_print("你感觉自己的身体变得更强壮了。");
        }
        break;

    case 15:
        if (name) return "反传送结界";
        if (desc) return "阻止视线内所有怪物的传送。";
        power = plev * 3 / 2;
        if (info) return info_power(power);
        if (cast)
        {
            msg_print("你感觉除了你之外任何人都无法传送。");
        }
        break;

    /*** 3rd book (16-23) ***/
    case 16:
        if (name) return "冲击斗篷";
        if (desc) return "赋予闪电光环和速度加值。";
        if (cast)
        {
            msg_print("你被闪电光环包围了！");
        }
        if (stop)
        {
            msg_print("闪电光环消失了。");
        }
        break;

    case 17:
        if (name) return "治疗致命伤";
        if (desc) return "极大地治愈割伤并恢复生命值。";
        if (info) return info_heal(4, 10, 0);
        if (cast)
        {
            msg_print("你感觉越来越好了。");
        }
        if (cast || cont)
        {
            hp_player(damroll(4, 10));
            set_stun(0, TRUE);
            set_cut(0, TRUE);
        }
        break;

    case 18:
        if (name) return "充能";
        if (desc)
        {
            if (p_ptr->pclass == CLASS_BLOOD_MAGE)
                return "它尝试消耗你的血液为一件装置充能。";
            else
                return "它尝试消耗你的法力为一件装置充能。";
        }

        power = plev * 2;
        if (info) return info_power(power);
        if (cast)
        {
            if (!recharge_from_player(power)) return NULL;
            add = FALSE;
        }
        break;

    case 19:
        if (name) return "操纵死尸";
        if (desc) return "从死亡中复活尸体和骸骨。";
        if (cast)
        {
            msg_print("你开始呼唤死者！");
        }
        if (cast || cont)
        {
            animate_dead(0, py, px);
        }
        break;

    case 20:
        if (name) return "诅咒防具";
        if (desc) return "诅咒一件你所穿戴的护甲。";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            char o_name[MAX_NLEN];
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "你要诅咒哪件防具？";
            prompt.error = "你没有装备任何防具。";
            prompt.filter = object_is_armour;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            object_desc(o_name, prompt.obj, OD_NAME_ONLY);
            obj_flags(prompt.obj, f);

            if (!get_check(format("你真的要诅咒%s吗？", o_name))) return FALSE;

            if (!one_in_(3) &&
                (object_is_artifact(prompt.obj) || have_flag(f, OF_BLESSED)))
            {
                msg_format("%s抵抗了效果。", o_name);
                if (one_in_(3))
                {
                    if (prompt.obj->to_d > 0)
                    {
                        prompt.obj->to_d -= randint1(3) % 2;
                        if (prompt.obj->to_d < 0) prompt.obj->to_d = 0;
                    }
                    if (prompt.obj->to_h > 0)
                    {
                        prompt.obj->to_h -= randint1(3) % 2;
                        if (prompt.obj->to_h < 0) prompt.obj->to_h = 0;
                    }
                    if (prompt.obj->to_a > 0)
                    {
                        prompt.obj->to_a -= randint1(3) % 2;
                        if (prompt.obj->to_a < 0) prompt.obj->to_a = 0;
                    }
                    msg_format("你的%s被解除了魔法！", o_name);
                }
            }
            else
            {
                int power = 0;
                msg_format("一阵可怕的黑色光环冲击了你的%s！", o_name);
                prompt.obj->curse_flags |= (OFC_CURSED);

                if (object_is_artifact(prompt.obj) || object_is_ego(prompt.obj))
                {

                    if (one_in_(3)) prompt.obj->curse_flags |= (OFC_HEAVY_CURSE);
                    if (one_in_(666))
                    {
                        prompt.obj->curse_flags |= (OFC_TY_CURSE);
                        if (one_in_(666)) prompt.obj->curse_flags |= (OFC_PERMA_CURSE);

                        add_flag(prompt.obj->flags, OF_AGGRAVATE);
                        add_flag(prompt.obj->flags, OF_RES_POIS);
                        add_flag(prompt.obj->flags, OF_RES_DARK);
                        add_flag(prompt.obj->flags, OF_RES_NETHER);
                        msg_print("血，鲜血，鲜血！");
                        power = 2;
                    }
                }

                prompt.obj->curse_flags |= get_curse(power, prompt.obj);
            }

            p_ptr->update |= (PU_BONUS);
            add = FALSE;
        }
        break;

    case 21:
        if (name) return "阴影斗篷";
        if (desc) return "赋予暗影光环。";
        if (cast)
        {
            int slot = equip_find_first(object_is_cloak);
            object_type *o_ptr = NULL;

            if (!slot)
            {
                msg_print("你没有穿戴斗篷。");
                return NULL;
            }
            o_ptr = equip_obj(slot);
            if (!object_is_cursed(o_ptr))
            {
                msg_print("你的斗篷没有被诅咒。");
                return NULL;
            }
            else
            {
                msg_print("你被阴影光环包围了！");
            }
        }
        if (cont)
        {
            int slot = equip_find_first(object_is_cloak);
            if (!slot || !object_is_cursed(equip_obj(slot)))
            {
                do_spell(REALM_HEX, spell, SPELL_STOP);
                p_ptr->magic_num1[0] &= ~(1L << spell);
                p_ptr->magic_num2[0]--;
                if (!p_ptr->magic_num2[0]) set_action(ACTION_NONE);
            }
        }
        if (stop)
        {
            msg_print("阴影光环消失了。");
        }
        break;

    case 22:
        if (name) return "痛苦化法";
        if (desc) return "对视线内所有的怪物造成精神伤害，并吸取一些法力。";
        power = plev * 3 / 2 + p_ptr->to_d_spell;
        if (info) return info_damage(1, power, 0);
        if (cast || cont)
        {
            project_hack(GF_PSI_DRAIN, randint1(power));
        }
        break;

    case 23:
        if (name) return "以眼还眼";
        if (desc) return "将你受到的等量伤害反弹给伤害你的怪物。";
        if (cast)
        {
            msg_print("你产生了强烈的报复欲望。");
        }
        break;

    /*** 4th book (24-31) ***/
    case 24:
        if (name) return "反繁殖结界";
        if (desc) return "阻止整个楼层内所有怪物的繁殖。";
        if (cast)
        {
            msg_print("你感觉任何怪物都无法再繁殖了。");
        }
        break;

    case 25:
        if (name) return "恢复生命";
        if (desc) return "恢复生命能量和状态。";
        if (cast)
        {
            msg_print("你感觉你的生命能量开始回归。");
        }
        if (cast || cont)
        {
            bool flag = FALSE;
            int d = (p_ptr->max_exp - p_ptr->exp);
            int r = (p_ptr->exp / 20);
            int l = (1000 - p_ptr->clp);
            int i;

            if (d > 0)
            {
                if (d < r)
                    p_ptr->exp = p_ptr->max_exp;
                else
                    p_ptr->exp += r;

                /* Check the experience */
                check_experience();

                flag = TRUE;
            }
            if (l > 0)
            {
                lp_player(MIN(l, 15));
                flag = TRUE;
            }
            for (i = A_STR; i < 6; i ++)
            {
                if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
                {
                    if (p_ptr->stat_cur[i] < 18)
                        p_ptr->stat_cur[i]++;
                    else
                        p_ptr->stat_cur[i] += 10;

                    if (p_ptr->stat_cur[i] > p_ptr->stat_max[i])
                        p_ptr->stat_cur[i] = p_ptr->stat_max[i];

                    /* Recalculate bonuses */
                    p_ptr->update |= (PU_BONUS);

                    flag = TRUE;
                }
            }

            if (!flag)
            {
                msg_format("完成施放 '%^s'。", do_spell(REALM_HEX, HEX_RESTORE, SPELL_NAME));
                p_ptr->magic_num1[0] &= ~(1L << HEX_RESTORE);
                if (cont) p_ptr->magic_num2[0]--;
                if (!p_ptr->magic_num2[0]) set_action(ACTION_NONE);

                /* Redraw status */
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
                p_ptr->redraw |= (PR_EXTRA);

                return "";
            }
        }
        break;

    case 26:
        if (name) return "吸取诅咒力量";
        if (desc) return "吸收你武器上的诅咒，并恢复少量的法力值(SP)。";
        if (cast)
        {
            obj_prompt_t prompt = {0};
            u32b f[OF_ARRAY_SIZE];

            prompt.prompt = "你要从哪件被诅咒的装备中吸取法力？";
            prompt.error = "你没有被诅咒的装备。";
            prompt.filter = object_is_cursed;
            prompt.where[0] = INV_EQUIP;

            obj_prompt(&prompt);
            if (!prompt.obj) return FALSE;

            obj_flags(prompt.obj, f);

            p_ptr->csp += (plev / 5) + randint1(plev / 5);
            if (have_flag(f, OF_TY_CURSE) || (prompt.obj->curse_flags & OFC_TY_CURSE)) p_ptr->csp += randint1(5);
            if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;

            if (prompt.obj->curse_flags & OFC_PERMA_CURSE)
            {
                /* Nothing */
            }
            else if (prompt.obj->curse_flags & OFC_HEAVY_CURSE)
            {
                if (one_in_(7))
                {
                    msg_print("沉重的诅咒消失了。");
                    prompt.obj->curse_flags = 0L;
                    prompt.obj->known_curse_flags = 0L;
                }
            }
            else if ((prompt.obj->curse_flags & (OFC_CURSED)) && one_in_(3))
            {
                msg_print("诅咒消失了。");
                prompt.obj->curse_flags = 0L;
                prompt.obj->known_curse_flags = 0L;
            }

            add = FALSE;
        }
        break;

    case 27:
        if (name) return "武器化为吸血剑";
        if (desc) return "赋予你武器吸血能力。";
        if (cast)
        {
            if (p_ptr->weapon_ct > 1)
                msg_print("你的武器现在渴望更多鲜血。");
            else
                msg_print("你的武器现在渴望更多鲜血。");
        }
        if (stop)
            msg_format("武器%s的嗜血消失了。", (p_ptr->weapon_ct <= 1) ? "" : "s");
        break;

    case 28:
        if (name) return "震慑之语";
        if (desc) return "震慑视线内所有的怪物。";
        if (cast || cont)
        {
            stun_monsters(5 + plev/5);
        }
        break;

    case 29:
        if (name) return "潜入暗影";
        if (desc) return "将你传送到一个怪物附近（突脸传送）。";
        if (cast)
        {
            int i, y, x, dir;
            bool flag;

            for (i = 0; i < 3; i++)
            {
                if (!tgt_pt(&x, &y, plev+2)) return FALSE;

                flag = FALSE;

                for (dir = 0; dir < 8; dir++)
                {
                    int dy = y + ddy_ddd[dir];
                    int dx = x + ddx_ddd[dir];
                    if (dir == 5) continue;
                    if(cave[dy][dx].m_idx) flag = TRUE;
                }

                if (!cave_empty_bold(y, x) || (cave[y][x].info & CAVE_ICKY) ||
                    (distance(y, x, py, px) > plev + 2))
                {
                    msg_print("无法传送到那里。");
                    continue;
                }
                break;
            }

            if (flag && randint0(plev * plev / 2))
            {
                teleport_player_to(y, x, 0L);
            }
            else
            {
                msg_print("哎呀！");
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(30, 0L);
            }

            add = FALSE;
        }
        break;

    case 30:
        if (name) return "反魔法结界";
        if (desc) return "阻碍你视线内怪物的施法。";
        power = plev * 3 / 2;
        if (info) return info_power(power);
        if (cast)
        {
            msg_print("你感觉除了你之外任何人都无法施法。");
        }
        break;

    case 31:
        if (name) return "复仇宣言";
        if (desc) return "Fires  a ball of hell fire to try revenging after few turns.";
        power = p_ptr->magic_num1[2] + p_ptr->to_d_spell;
        if (info) return info_damage(0, 0, power);
        if (cast)
        {
            int r;
            int a = 3 - (p_ptr->pspeed - 100) / 10;
            r = 3 + randint1(2) + MAX(0, MIN(3, a));

            if (p_ptr->magic_num2[2] > 0)
            {
                msg_print("你已经宣告了你的复仇。");
                return NULL;
            }

            p_ptr->magic_num2[1] = 2;
            p_ptr->magic_num2[2] = r;
            msg_format("你宣告了复仇。剩余 %d 回合。", r - 1);
            add = FALSE;
        }
        if (cont)
        {
            p_ptr->magic_num2[2]--;

            if (p_ptr->magic_num2[2] <= 0)
            {
                int dir;

                if (power)
                {
                    command_dir = 0;

                    do
                    {
                        msg_print("复仇的时刻到了！");
                    }
                    while (!get_fire_dir(&dir));

                    fire_ball(GF_HELL_FIRE, dir, power, 1);

                    if (p_ptr->wizard || easy_damage)
                    {
                        msg_format("你返还了 %d 点伤害。", power);
                    }
                }
                else
                {
                    msg_print("你现在没有心情复仇。");
                }
                p_ptr->magic_num1[2] = 0;
            }
        }
        break;
    }

    /* start casting */
    if ((cast) && (add))
    {
        /* add spell */
        p_ptr->magic_num1[0] |= 1L << (spell);
        p_ptr->magic_num2[0]++;

        if (p_ptr->action != ACTION_SPELL) set_action(ACTION_SPELL);
    }

    /* Redraw status */
    if (!info)
    {
        p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
        p_ptr->redraw |= (PR_EXTRA | PR_HP | PR_MANA);
    }

    return "";
}

static cptr do_armageddon_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    int plev = p_ptr->lev;
    int dir;

    switch (spell)
    {
    /* Book of Elements */
    case 0:
        if (name) return "闪电之矢";
        if (desc) return "发射一道闪电魔法弹或射线。";

        {
            int dice = 3 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_ELEC,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 1:
        if (name) return "冰霜之矢";
        if (desc) return "发射一道寒冷魔法弹或射线。";

        {
            int dice = 4 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_COLD,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 2:
        if (name) return "火之矢";
        if (desc) return "发射一道火焰魔法弹或射线。";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_FIRE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 3:
        if (name) return "酸液之矢";
        if (desc) return "发射一道酸液魔法弹或射线。";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_ACID,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 4:
        if (name) return "闪电球";
        if (desc) return "发射一颗电球。";

        {
            int dam = spell_power(3*plev/2 + 20 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
            }
        }
        break;
    case 5:
        if (name) return "冰霜球";
        if (desc) return "发射一颗寒冰球。";

        {
            int dam = spell_power(3*plev/2 + 25 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;
    case 6:
        if (name) return "火球";
        if (desc) return "发射一颗火球。";

        {
            int dam = spell_power(3*plev/2 + 30 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;
    case 7:
        if (name) return "酸液球";
        if (desc) return "发射一颗酸液球。";

        {
            int dam = spell_power(3*plev/2 + 35 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ACID, dir, dam, rad);
            }
        }
        break;

    /* Earth, Wind and Fire */
    case 8:
        if (name) return "碎片之矢";
        if (desc) return "发射一道破片魔法弹或射线。";

        {
            int dice = 7 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_SHARDS,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 9:
        if (name) return "重力之矢";
        if (desc) return "发射一道重力魔法弹或射线。";

        {
            int dice = 5 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_GRAVITY,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 10:
        if (name) return "等离子之矢";
        if (desc) return "发射一道等离子魔法弹或射线。";

        {
            int dice = 11 + plev / 4;
            int sides = 8;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt_or_beam(
                    beam_chance(),
                    GF_PLASMA,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 11:
        if (name) return "陨石";
        if (desc) return "召唤一颗陨石。";

        {
            int dam = spell_power(plev + 60 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_METEOR, dir, dam, rad);
            }
        }
        break;
    case 12:
        if (name) return "雷鸣爆";
        if (desc) return "以你为中心生成一颗音波球。";

        {
            int dam = spell_power((40 + plev + p_ptr->to_d_spell)*2);
            int rad = plev / 10 + 2;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
            {
                msg_print("轰！");
                project(0, rad, py, px, dam, GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            }
        }
        break;

    case 13:
        if (name) return "狂风爆";
        if (desc) return "发射一阵强烈的微下击暴流(microburst)。";

        {
            int dam = spell_power(plev + 40 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_TELEKINESIS, dir, dam, rad);
            }
        }
        break;
    case 14:
        if (name) return "地狱风暴";
        if (desc) return "以你为中心生成一颗巨大的火球。";

        {
            int dam = spell_power((6 * plev + p_ptr->to_d_spell)*2);
            int rad = 8;

            if (info) return info_damage(0, 0, dam/2);

            if (cast)
                fire_ball(GF_FIRE, 0, dam, rad);
        }
        break;
    case 15:
        if (name) return "火箭";
        if (desc) return "发射一枚火箭弹。";

        {
            int dam = spell_power(60 + plev * 4 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                msg_print("你发射了一枚火箭！");
                fire_rocket(GF_ROCKET, dir, dam, rad);
            }
        }
        break;

    /* Path of Destruction */
    case 16:
        if (name) return "寒冰之矢";
        if (desc) return "发射一道坚冰弹。";

        {
            int dice = 5 + plev/4;
            int sides = 15;

            if (info) return info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_ICE,
                    dir,
                    spell_power(damroll(dice, sides) + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 17:
        if (name) return "水球";
        if (desc) return "发射一颗水球。";

        {
            int dam = spell_power(2*plev + 30 + p_ptr->to_d_spell);
            int rad = 2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_WATER, dir, dam, rad);
            }
        }
        break;
    case 18:
        if (name) return "喷吐闪电";
        if (desc) return "向选定的目标喷吐闪电锥。";

        {
            int dam = spell_power(9*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ELEC, dir, dam, rad);
            }
        }
        break;
    case 19:
        if (name) return "喷吐冰霜";
        if (desc) return "向选定的目标喷吐寒冰锥。";

        {
            int dam = spell_power(9*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_COLD, dir, dam, rad);
            }
        }
        break;
    case 20:
        if (name) return "喷吐火焰";
        if (desc) return "向选定的目标喷吐火焰锥。";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_FIRE, dir, dam, rad);
            }
        }
        break;
    case 21:
        if (name) return "喷吐酸液";
        if (desc) return "向选定的目标喷吐酸液锥。";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_ACID, dir, dam, rad);
            }
        }
        break;
    case 22:
        if (name) return "喷吐等离子";
        if (desc) return "向选定的目标喷吐等离子锥。";

        {
            int dam = spell_power(11*plev/2 + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;
    case 23:
        if (name) return "喷吐重力";
        if (desc) return "向选定的目标喷吐重力锥。";

        {
            int dam = spell_power(4*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_GRAVITY, dir, dam, rad);
            }
        }
        break;

    /* Day of Ragnarok */
    case 24:
        if (name) return "法力之矢";
        if (desc) return "发射一道法力弹。";

        {
            int dice = 1;
            int sides = 5*plev;

            if (info) return info_damage(dice, spell_power(sides), spell_power(50 + p_ptr->to_d_spell));

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_bolt(
                    GF_MANA,
                    dir,
                    spell_power(damroll(dice, sides) + 50 + p_ptr->to_d_spell)
                );
            }
        }
        break;
    case 25:
        if (name) return "等离子球";
        if (desc) return "发射一颗等离子球。";

        {
            int dam = spell_power(2*plev + 90 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_PLASMA, dir, dam, rad);
            }
        }
        break;
    case 26:
        if (name) return "法力球";
        if (desc) return "发射一颗纯净法力球。";

        {
            int dam = spell_power(4*plev + 100 + p_ptr->to_d_spell);
            int rad = 3;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;
    case 27:
        if (name) return "喷吐声波";
        if (desc) return "向选定的目标喷吐音波锥。";

        {
            int dam = spell_power(6*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_SOUND, dir, dam, rad);
            }
        }
        break;
    case 28:
        if (name) return "喷吐迟缓";
        if (desc) return "向选定的目标喷吐迟缓(inertia)锥。";

        {
            int dam = spell_power(5*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_INERT, dir, dam, rad);
            }
        }
        break;
    case 29:
        if (name) return "喷吐解离";
        if (desc) return "向选定的目标喷吐分解锥。";

        {
            int dam = spell_power(7*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir_aux(&dir, TARGET_DISI)) return NULL;
                fire_ball(GF_DISINTEGRATE, dir, dam, rad);
            }
        }
        break;
    case 30:
        if (name) return "喷吐法力";
        if (desc) return "向选定的目标喷吐法力锥。";

        {
            int dam = spell_power(9*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_MANA, dir, dam, rad);
            }
        }
        break;
    case 31:
        if (name) return "喷吐碎片";
        if (desc) return "向选定的目标喷吐破片锥。";

        {
            int dam = spell_power(10*plev + p_ptr->to_d_spell);
            int rad = plev > 40 ? -3 : -2;

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_SHARDS, dir, dam, rad);
            }
        }
        break;
    }
    return "";
}
/*
 * Do everything for each spell
 */
cptr do_spell(int realm, int spell, int mode)
{
    cptr result = NULL;

    _current_realm_hack = realm;

    switch (realm)
    {
    case REALM_LIFE:     result = do_life_spell(spell, mode); break;
    case REALM_SORCERY:  result = do_sorcery_spell(spell, mode); break;
    case REALM_NATURE:   result = do_nature_spell(spell, mode); break;
    case REALM_CHAOS:    result = do_chaos_spell(spell, mode); break;
    case REALM_DEATH:    result = do_death_spell(spell, mode); break;
    case REALM_TRUMP:    result = do_trump_spell(spell, mode); break;
    case REALM_ARCANE:   result = do_arcane_spell(spell, mode); break;
    case REALM_CRAFT:    result = do_craft_spell(spell, mode); break;
    case REALM_DAEMON:   result = do_daemon_spell(spell, mode); break;
    case REALM_CRUSADE:  result = do_crusade_spell(spell, mode); break;
    case REALM_LAW:      result = do_law_spell(spell, mode); break;
    case REALM_MUSIC:    result = do_music_spell(spell, mode); break;
    case REALM_HISSATSU: result = do_hissatsu_spell(spell, mode); break;
    case REALM_HEX:      result = do_hex_spell(spell, mode); break;
    case REALM_NECROMANCY: result = do_necromancy_spell(spell, mode); break;
    case REALM_ARMAGEDDON: result = do_armageddon_spell(spell, mode); break;
    case REALM_BURGLARY: result = do_burglary_spell(spell, mode); break;
    }

    _current_realm_hack = 0;
    return result;
}

int get_realm_idx(cptr name)
{
    int i;
    for (i = 0; i < MAX_REALM; i++)
    {
        if (strcmp(name, realm_internal_name(i)) == 0)
            return i;
        if (strcmp(name, realm_names[i]) == 0)
            return i;
    }
    return -1;
}
