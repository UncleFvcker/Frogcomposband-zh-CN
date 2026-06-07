#include "angband.h"

/****************************************************************
 * Amberite
 ****************************************************************/
static power_info _amberite_get_powers[] =
{
    { A_INT, {30, 50, 70, shadow_shifting_spell}},
    { A_WIS, {40, 75, 75, pattern_mindwalk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _amberite_calc_bonuses(void)
{
    p_ptr->sustain_con = TRUE;
    p_ptr->regen += 100;
}
static void _amberite_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_CON);
    add_flag(flgs, OF_REGEN);
}
race_t *amberite_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "安珀人";
        me.desc = "安珀人被认为是一个不朽的种族，除了寿命悠长之外，他们还被赋予了许多优势。他们非常坚韧，体质无法被降低，他们从伤痛中恢复的能力超过了大多数其他种族。由于几乎见识过一切，很少有事物对他们来说是新鲜的，因此他们升级的速度相对较慢。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  4;
        me.skills.dev =  3;
        me.skills.sav =  3;
        me.skills.stl =  2;
        me.skills.srh =  3;
        me.skills.fos = 13;
        me.skills.thn = 15;
        me.skills.thb =  7;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 190;
        me.infra = 0;
        me.shop_adjust = 100;


        me.calc_bonuses = _amberite_calc_bonuses;
        me.get_powers = _amberite_get_powers;
        me.get_flags = _amberite_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Android
 ****************************************************************/
static int _obj_value(object_type *o_ptr)
{
    object_type  copy = *o_ptr;
    copy.discount = 0;
    copy.curse_flags = 0;
    return obj_value_real(&copy);
}
int android_obj_exp(object_type *o_ptr)
{
    int value, exp, level;

    if (!o_ptr) return 0;
    if (!object_is_wearable(o_ptr)) return 0;
    if (object_is_jewelry(o_ptr)) return 0;
    if (o_ptr->tval == TV_LITE) return 0;

    value = _obj_value(o_ptr);
    if (value <= 0) return 0;
    if (object_is_(o_ptr, TV_SOFT_ARMOR, SV_ABUNAI_MIZUGI) && p_ptr->personality != PERS_SEXY
        && p_ptr->pclass != CLASS_POLITICIAN) value /= 32;
    if (value > 5000000) value = 5000000;

    level = MAX(k_info[o_ptr->k_idx].level - 8, 1);

    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];
        int            a_lvl = MAX(a_ptr->level - 8, 5);
        int            r_div = a_ptr->gen_flags & OFG_INSTA_ART ? 10 : 3;

        level = (level + a_lvl) / 2;
        level += MIN(20, a_ptr->rarity/r_div);
    }
    else if (o_ptr->art_name || o_ptr->name2)
    {
        int fake_level = 10 + value / 1500;

        if (fake_level > 90)
            fake_level = 90;

        fake_level = MAX(fake_level - 8, 5);
        level = MAX(level, (level + fake_level) / 2 + 3);
    }

    if (o_ptr->tval == TV_DRAG_ARMOR || o_ptr->tval == TV_CARD) level /= 2;

    if ( object_is_artifact(o_ptr)
      || object_is_ego(o_ptr)
      || o_ptr->tval == TV_DRAG_ARMOR
      || object_is_dragon_armor(o_ptr)
      || object_is_(o_ptr, TV_SWORD, SV_DIAMOND_EDGE) )
    {
        if (level > 65) level = 35 + (level - 65) / 5;
        else if (level > 35) level = 25 + (level - 35) / 3;
        else if (level > 15) level = 15 + (level - 15) / 2;
        exp = MIN(100000L, value) * level * level / 2;
        if (value > 100000L)
            exp += (value - 100000L) * level * level / 8;
    }
    else
    {
        exp = MIN(100000L, value) * level;
        if (value > 100000L)
            exp += (value - 100000L) * level / 4;
    }
    if (object_is_melee_weapon(o_ptr) || o_ptr->tval == TV_BOW)
        return exp / 48;
    else if (object_is_body_armour(o_ptr))
        return 3 * exp / 32;
    else
        return exp / 16;
}

void android_calc_exp(void)
{
    int slot;
    s32b total_exp = 0;

    if (p_ptr->is_dead) return;

    if (p_ptr->prace != RACE_ANDROID) return;

    for (slot = 1; slot <= equip_max(); slot++)
    {
        object_type *o_ptr = equip_obj(slot);
        total_exp += android_obj_exp(o_ptr);
    }
    p_ptr->exp = p_ptr->max_exp = total_exp;
    if (p_ptr->pclass == CLASS_POLITICIAN) politician_android_experience();
    check_experience();
}


static power_info *_android_get_powers(void)
{
    static power_info android_powers[2] =
    {
        { A_STR, { -1, -1, -1, NULL }},
        { -1, {-1, -1, -1, NULL}}
    };
    spell_info *spell = &android_powers[0].spell;

    if (p_ptr->lev < 10)
    {
        spell->level = 1;
        spell->cost = 7;
        spell->fail = 30;
        spell->fn = android_ray_gun_spell;
    }
    else if (p_ptr->lev < 25)
    {
        spell->level = 10;
        spell->cost = 13;
        spell->fail = 30;
        spell->fn = android_blaster_spell;
    }
    else if (p_ptr->lev < 35)
    {
        spell->level = 25;
        spell->cost = 26;
        spell->fail = 40;
        spell->fn = android_bazooka_spell;
    }
    else if (p_ptr->lev < 45)
    {
        spell->level = 35;
        spell->cost = 40;
        spell->fail = 50;
        spell->fn = android_beam_cannon_spell;
    }
    else
    {
        spell->level = 45;
        spell->cost = 60;
        spell->fail = 70;
        spell->fn = android_rocket_spell;
    }
    return android_powers;
}
static void _android_calc_bonuses(void)
{
    int ac = 10 + (p_ptr->lev * 2 / 5);

    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;

    p_ptr->slow_digest = TRUE;
    p_ptr->free_act++;
    res_add(RES_POIS);
    /*res_add_vuln(RES_ELEC); cf resists.c res_pct_aux() for an alternative*/
    p_ptr->hold_life++;
}
static void _android_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_HOLD_LIFE);
    /*add_flag(flgs, TR_VULN_ELEC);*/
}
static void _android_birth(void)
{
    object_type forge = {0};
    object_prep(&forge, lookup_kind(TV_FLASK, SV_ANY));
    apply_magic(&forge, 1, AM_NO_FIXED_ART); /* Hack (pval->xtra4) */
    forge.number = rand_range(7, 12);
    py_birth_obj(&forge);

    py_birth_light();

    p_ptr->au /= 5;
}
race_t *android_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "人造人";
        me.desc = "人造人是用机械躯体创造出的人工产物。他们的智力几乎与有机种族相当，尽管他们的感知并不比魔像好多少。当然，他们的机械躯体提供了巨大的物理优势，远远超过人类的力量。在所有种族中，唯独人造人不会从杀怪中获取经验，而是随着他们将新装备安装到自己的身躯上而获得力量的提升。戒指、护身符和光源不影响其成长。人造人对毒素有抗性，免疫吸取生命的攻击；此外，由于他们坚硬的金属身躯，他们能获得护甲(AC)加成。人造人全身布满电子电路，因此必须小心电击。他们从凡人的食物中获得的营养极少，但可以使用油瓶作为能量来源。";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  0;
        me.skills.dev = -3;
        me.skills.sav =  0;
        me.skills.stl = -2;
        me.skills.srh =  3;
        me.skills.fos = 14;
        me.skills.thn = 20;
        me.skills.thb =  6;

        me.life = 108;
        me.base_hp = 26;
        me.exp = 200;
        me.infra = 0;
        me.shop_adjust = 120;


        me.birth = _android_birth;
        me.calc_bonuses = _android_calc_bonuses;
        me.get_powers_fn = _android_get_powers;
        me.get_flags = _android_get_flags;
        me.flags = RACE_IS_NONLIVING | RACE_NO_POLY;

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Archon
 ****************************************************************/
static void _archon_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->see_inv++;
    p_ptr->align += 200;
}
static void _archon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_SEE_INVIS);
}
race_t *archon_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "神圣使者";
        me.desc = "神圣使者是天使中的更高阶层。他们精通所有的技能，强壮、睿智，并且受到所有人的喜爱。他们能够看穿隐形事物，他们的翅膀使他们能够安全地飞越陷阱和其他危险的地方。然而，因为属于更高的位面，这个世界的经历无法在他们身上留下深刻的印记，因此他们升级很慢。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  4;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  3;

        me.skills.dis =  0;
        me.skills.dev =  8;
        me.skills.sav =  8;
        me.skills.stl =  2;
        me.skills.srh =  2;
        me.skills.fos = 11;
        me.skills.thn = 10;
        me.skills.thb =  7;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 200;
        me.infra = 3;
        me.shop_adjust = 90;

        me.calc_bonuses = _archon_calc_bonuses;
        me.get_flags = _archon_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Balrog
 ****************************************************************/
static power_info _balrog_get_powers[] =
{
    { A_CON, {15, 10, 70, demon_breath_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _balrog_calc_bonuses(void)
{
    res_add(RES_FIRE);
    res_add(RES_NETHER);
    p_ptr->hold_life++;
    if (p_ptr->lev >= 10) p_ptr->see_inv++;
    if (p_ptr->lev >= 45) res_add(RES_FIRE);
    p_ptr->align -= 200;
}
static void _balrog_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
}
static void _balrog_birth(void)
{
    int i, ct = rand_range(3, 4);
    get_mon_num_prep(monster_hook_human, NULL);
    for (i = 0; i < ct; i++)
    {
        object_type forge = {0};
        object_prep(&forge, lookup_kind(TV_CORPSE, SV_CORPSE));
        forge.pval = get_mon_num(2);
        py_birth_obj(&forge);
    }
    py_birth_light();
}
race_t *balrog_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "炎魔";
        me.desc = "炎魔是恶魔中的更高阶层。他们强壮、聪明、坚韧，但缺乏感知，不适合成为牧师。炎魔对火焰和虚空有抗性，并且能够牢牢掌控自己的生命力；他们也能很快学会识破隐形。除了潜行之外，他们擅长几乎所有的技能。他们从凡人的食物中获得的营养极少，需要献祭人类的尸体才能恢复活力。";

        me.stats[A_STR] =  4;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =-10;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  2;

        me.skills.dis = -3;
        me.skills.dev =  8;
        me.skills.sav = 15;
        me.skills.stl = -2;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn = 20;
        me.skills.thb =  0;

        me.life = 106;
        me.base_hp = 24;
        me.exp = 180;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_DEMON;
        me.shop_adjust = 140;

        me.birth = _balrog_birth;
        me.calc_bonuses = _balrog_calc_bonuses;
        me.get_powers = _balrog_get_powers;
        me.get_flags = _balrog_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Barbarian
 ****************************************************************/
static void _barbarian_gain_level(int new_level)
{
	if ((new_level >= 30) && (p_ptr->prace != RACE_DOPPELGANGER)) 
      /* bostock says doppel barbies are strong enough without a demigod power */
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static power_info _barbarian_get_powers[] =
{
    { A_STR, {8, 10, 30, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _barbarian_calc_bonuses(void)
{
    res_add(RES_FEAR);
}
static void _barbarian_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FEAR);
}
race_t *barbarian_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "野蛮人";
        me.desc = "野蛮人是来自北方的坚韧人类；他们在战斗中无比勇猛，他们的愤怒令全世界畏惧。战斗就是他们的生活：他们不知恐惧为何物，而且他们比半巨魔更早地学会了随意进入战斗狂暴状态。然而，野蛮人对魔法充满怀疑，这使得魔法装置对他们来说相当难用。野蛮人在30级时获得一项半神天赋。";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;

        me.skills.dis = -2;
        me.skills.dev = -7;
        me.skills.sav = 2;
        me.skills.stl = -1;
        me.skills.srh = 1;
        me.skills.fos = 7;
        me.skills.thn = 12;
        me.skills.thb =  6;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 135;
        me.infra = 0;
        me.shop_adjust = 120;
        me.flags = RACE_DEMI_TALENT;

        me.calc_bonuses = _barbarian_calc_bonuses;
        me.get_powers = _barbarian_get_powers;
        me.get_flags = _barbarian_get_flags;
		me.gain_level = _barbarian_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Beastman
 ****************************************************************/
static void _beastman_gain_level(int new_level)
{
    if (one_in_(5))
    {
        msg_print("你感觉不太一样了……");
        mut_gain_random(NULL);
    }
}
static void _beastman_calc_bonuses(void)
{
    res_add(RES_CONF);
    res_add(RES_SOUND);
}
static void _beastman_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_CONF);
}
static void _beastman_birth(void)
{
    mut_gain_random(mut_good_pred);
    py_birth_food();
    py_birth_light();
}
race_t *beastman_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "混沌兽人";
        me.desc = "这个种族是混沌产生的亵渎可憎之物。它并非一个独立的种族，而是一种被混沌扭曲的类人生物，通常是人类；或者是一种人类与野兽梦魇般的混血产物。所有的混沌兽人都极为习惯混沌，以至于他们不受困惑和声音的困扰，尽管纯粹的混沌之源(Logrus)仍然能伤害他们。混沌兽人沉迷于混沌之中，因为混沌会越来越深地扭曲他们。混沌兽人极易发生变异：当他们被创建时，会获得一个随机变异，并且每次升级时都有很小的几率获得更多的变异。与普通种族不同，混沌兽人可以拥有许多变异，而不会对他们的再生能力产生负面影响。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = -5;
        me.skills.dev = -1;
        me.skills.sav = -1;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 5;
        me.skills.thn = 12;
        me.skills.thb = 3;

        me.life = 102;
        me.base_hp = 22;
        me.exp = 150;
        me.infra = 0;
        me.shop_adjust = 130;

        me.birth = _beastman_birth;
        me.calc_bonuses = _beastman_calc_bonuses;
        me.gain_level = _beastman_gain_level;
        me.get_flags = _beastman_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Boit
 ****************************************************************/

static void _boit_vomit_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "呕吐");
        break;
    case SPELL_DESC:
        var_set_string(res, "吐出你胃里的东西，使你变得极度饥饿并解除中毒状态。在空腹状态下呕吐会有点疼。");
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_AFRAID | PWR_CONFUSED);
        break;
    case SPELL_CAST:
        msg_print("你吐了！");
        if (p_ptr->food < PY_FOOD_FAINT + 24)
        {
            take_hit(DAMAGE_NOESCAPE, 10, "vomiting on an empty stomach");
            energy_use += 15;
        }
        set_food(MAX(1, MIN(p_ptr->food - 100, PY_FOOD_FAINT + 12)));
        fire_ball(GF_POIS, 0, p_ptr->poisoned * 2 / 7, 1);
        set_poisoned(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _boit_get_powers[] =
{
    { A_STR, {1, 0, 0, _boit_vomit_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _boit_calc_bonuses(void)
{
    p_ptr->pspeed += 2;
    p_ptr->skill_tht += 25;
}
static void _boit_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPEED);
}
race_t *boit_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "波伊特";
        me.desc = "波伊特是一种体型较小、双足行走、身上覆盖着黄色、淡紫色、棕色或绿色毛皮的生物，与伊克人有远亲关系。波伊特移动迅速，并且可以随心所欲地吐出胃里的东西，但它们并不特别适合近战、远程战斗、使用魔法装置或施展法术。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] = -2;

        me.skills.dis = 2;
        me.skills.dev = -5;
        me.skills.sav = -1;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = -8;
        me.skills.thb = -8;

        me.life = 95;
        me.base_hp = 15;
        me.exp = 80;
        me.infra = 1;
        me.shop_adjust = 105;

        me.calc_bonuses = _boit_calc_bonuses;
        me.get_powers = _boit_get_powers;
        me.get_flags = _boit_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Centaur
 ****************************************************************/
static void _centaur_birth(void)
{
    equip_on_change_race();
    skills_innate_init("马蹄", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    py_birth_food();
    py_birth_light();
}

void jump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "跳跃");
        break;
    case SPELL_DESC:
        var_set_string(res, "向前跳跃一段短距离，越过任何中间的怪物或障碍物。");
        break;
    case SPELL_CAST:
    {
        int x, y;
        int len = 2 + p_ptr->lev/35;

        var_set_bool(res, FALSE);

        if (!tgt_pt(&x, &y, len)) return;

        if (distance(y, x, py, px) > len)
        {
            msg_print("你跳不了那么远。");
            return;
        }
        if (!los(py, px, y, x))
        {
            msg_print("你看不见那个位置。");
            return;
        }
        if (!cave_player_teleportable_bold(y, x, 0L))
        {
            msg_print("你无法跳跃到那里！");
            return;
        }
        teleport_player_to(y, x, 0L);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _centaur_get_powers[] =
{
    { A_DEX, {15, 10, 50, jump_spell}},
    { -1, {-1, -1, -1, NULL} }
};

static void _centaur_calc_bonuses(void)
{
    int slot = equip_find_first(object_is_body_armour);
    p_ptr->pspeed += p_ptr->lev / 10;

    if (slot)
    {
        object_type *o_ptr = equip_obj(slot);
        p_ptr->to_a -= o_ptr->ac / 3;
        p_ptr->dis_to_a -= o_ptr->ac / 3;

        if (o_ptr->to_a > 0)
        {
            p_ptr->to_a -= o_ptr->to_a / 3;
            p_ptr->dis_to_a -= o_ptr->to_a / 3;
        }
    }
}

static void _centaur_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SPEED);
}

static void _centaur_calc_innate_attacks(void)
{
    int l = p_ptr->lev;
    int to_d = py_prorata_level(15);
    int to_h = l/2;
    innate_attack_t    a = {0};

    a.dd = 1 + l / 16;
    a.ds = 4 + l / 21;
    a.to_d += to_d;
    a.to_h += to_h;

    a.weight = 150;
    calc_innate_blows(&a, 200);
    a.msg = "You kick.";
    a.name = "马蹄";

    p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
}

race_t *centaur_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "半人马";
        me.desc = "半人马拥有人类的头部、手臂和躯干，并结合了马的身体和腿。因此，随着经验的增长，他们能够移动得更快，并且有能力跳跃很长的距离。半人马精通格斗和箭术，除了常规的近战武器外，还能用蹄子攻击怪物；但由于没有正常人的脚，他们无法穿靴子。作为世界森林中的原住民，半人马能够快速穿越树叶等植被。";

        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;

        me.skills.dis =  0;
        me.skills.dev = -3;
        me.skills.sav =  2;
        me.skills.stl =  1;
        me.skills.srh =  3;
        me.skills.fos =  5;
        me.skills.thn = 10;
        me.skills.thb =  8;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 190;
        me.infra = 0;
        me.shop_adjust = 95;

        me.birth = _centaur_birth;
        me.calc_innate_attacks = _centaur_calc_innate_attacks;
        me.get_powers = _centaur_get_powers;
        me.calc_bonuses = _centaur_calc_bonuses;
        me.get_flags = _centaur_get_flags;

        me.equip_template = &b_info[46];

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Cyclops
 ****************************************************************/
static power_info _cyclops_get_powers[] =
{
    { A_STR, {20, 0, 50, throw_boulder_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _cyclops_calc_bonuses(void)
{
    res_add(RES_SOUND);
}
static void _cyclops_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SOUND);
}
race_t *cyclops_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "独眼巨人";
        me.desc = "虽然只有一只眼睛，但独眼巨人看到的比许多双眼健全的人还要多。他们固执己见，巨大的噪音也很少能干扰到他们。他们不太适合使用魔法的职业，但正如某位尤利西斯先生可以作证的那样，他们投掷石块的精准度可能是致命的！";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] = -1;

        me.skills.dis = -4;
        me.skills.dev = -3;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -2;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb = 10;

        me.life = 108;
        me.base_hp = 24;
        me.exp = 155;
        me.infra = 1;
        me.shop_adjust = 135;

        me.calc_bonuses = _cyclops_calc_bonuses;
        me.get_powers = _cyclops_get_powers;
        me.get_flags = _cyclops_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Dark-Elf
 ****************************************************************/
static power_info _dark_elf_get_powers[] =
{
    { A_INT, {1, 2, 30, magic_missile_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _dark_elf_calc_bonuses(void)
{
    res_add(RES_DARK);
    p_ptr->spell_cap += 3;
    if (p_ptr->lev >= 20) p_ptr->see_inv++;
}
static void _dark_elf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_SPELL_CAP);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_SEE_INVIS);
}
race_t *dark_elf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "黑暗精灵";
        me.desc = "作为一个不受黑暗攻击阻碍的穴居种族，黑暗精灵与尼伯龙根人的共同点比与其他精灵的共同点还要多。他们在魔法和施法方面有着极高的天赋，无论其职业为何，都能在低等级时学会“魔法飞弹”法术。凭借敏锐的视力，他们很快就能获得识破隐形事物的能力。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] =  3;

        me.skills.dis = 5;
        me.skills.dev = 7;
        me.skills.sav = 12;
        me.skills.stl = 3;
        me.skills.srh = 8;
        me.skills.fos = 12;
        me.skills.thn = -5;
        me.skills.thb =  6;

        me.life = 97;
        me.base_hp = 18;
        me.exp = 155;
        me.infra = 5;
        me.shop_adjust = 120;

        me.calc_bonuses = _dark_elf_calc_bonuses;
        me.get_powers = _dark_elf_get_powers;
        me.get_flags = _dark_elf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Draconian
 ****************************************************************/
static int _draconian_breath_amount(void)
{
    int l = p_ptr->lev;
    int amt = 0;

    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
    case DRACONIAN_WHITE:
    case DRACONIAN_BLUE:
    case DRACONIAN_BLACK:
    case DRACONIAN_GREEN:
        amt = MIN(500, p_ptr->chp * (25 + l*l*l/2500) / 100);
        break;

    case DRACONIAN_SHADOW:
        amt = MIN(400, p_ptr->chp * (20 + l*l*l*35/125000) / 100);
        break;

    case DRACONIAN_CRYSTAL:
    case DRACONIAN_GOLD:
        amt = MIN(350, p_ptr->chp * (20 + l*l*l*30/125000) / 100);
        break;

    case DRACONIAN_BRONZE:
        amt = MIN(300, p_ptr->chp * (17 + l*l*l*25/125000) / 100);
        break;
    }

    if (!mut_present(MUT_DRACONIAN_BREATH))
        amt /= 2;

    return MAX(amt, 1);
}

static int _draconian_breath_cost(void)
{
    int l = p_ptr->lev;
    int cost = l/2 + l*l*15/2500;
    if (!mut_present(MUT_DRACONIAN_BREATH))
        cost = cost * 2 / 3;
    return MAX(cost, 1);
}

static cptr _draconian_breath_desc(void)
{
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED: return "fire";
    case DRACONIAN_WHITE: return "cold";
    case DRACONIAN_BLUE: return "lightning";
    case DRACONIAN_BLACK: return "acid";
    case DRACONIAN_GREEN: return "poison";
    case DRACONIAN_CRYSTAL: return "shards";
    case DRACONIAN_BRONZE: return "confusion";
    case DRACONIAN_GOLD: return "sound";
    case DRACONIAN_SHADOW: return "nether";
    }
    return 0;
}

static int _draconian_breath_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED: return GF_FIRE;
    case DRACONIAN_WHITE: return GF_COLD;
    case DRACONIAN_BLUE: return GF_ELEC;
    case DRACONIAN_BLACK: return GF_ACID;
    case DRACONIAN_GREEN: return GF_POIS;
    case DRACONIAN_BRONZE: return GF_CONFUSION;
    case DRACONIAN_GOLD: return GF_SOUND;
    case DRACONIAN_SHADOW: return GF_NETHER;
    case DRACONIAN_CRYSTAL: return GF_SHARDS;
    }
    return 0;
}

static void _draconian_do_breathe(int effect, int dir, int dam)
{
    /* Dragon breath changes shape with maturity */
    if (p_ptr->lev < 20)
        fire_bolt(effect, dir, dam);
    else if (p_ptr->lev < 30)
        fire_beam(effect, dir, dam);
    else
        fire_ball(effect, dir, dam, -1 - (p_ptr->lev / 20));
}

static void _draconian_breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐%s。", _draconian_breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _draconian_breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _draconian_breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _draconian_breath_effect();
            int dam = _draconian_breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            _draconian_do_breathe(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _draconian_get_powers[] =
{
    { A_CON, {1, 0, 70, _draconian_breathe_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _draconian_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
        res_add(RES_FIRE);
        break;
    case DRACONIAN_WHITE:
        res_add(RES_COLD);
        break;
    case DRACONIAN_BLUE:
        res_add(RES_ELEC);
        break;
    case DRACONIAN_BLACK:
        res_add(RES_ACID);
        break;
    case DRACONIAN_GREEN:
        res_add(RES_POIS);
        break;
    case DRACONIAN_BRONZE:
        res_add(RES_CONF);
        break;
    case DRACONIAN_CRYSTAL:
        res_add(RES_SHARDS);
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        if (p_ptr->lev >= 40)
            p_ptr->reflect = TRUE;
        break;
    case DRACONIAN_GOLD:
        res_add(RES_SOUND);
        break;
    case DRACONIAN_SHADOW:
        res_add(RES_NETHER);
        break;
    }
    if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        int l = p_ptr->lev;
        int to_a = py_prorata_level(75);
        int ac = 15 + (l/10)*5;

        p_ptr->ac += ac;
        p_ptr->dis_ac += ac;

        p_ptr->to_a += to_a;
        p_ptr->dis_to_a += to_a;
    }
}
static void _draconian_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
        add_flag(flgs, OF_RES_FIRE);
        break;
    case DRACONIAN_WHITE:
        add_flag(flgs, OF_RES_COLD);
        break;
    case DRACONIAN_BLUE:
        add_flag(flgs, OF_RES_ELEC);
        break;
    case DRACONIAN_BLACK:
        add_flag(flgs, OF_RES_ACID);
        break;
    case DRACONIAN_GREEN:
        add_flag(flgs, OF_RES_POIS);
        break;
    case DRACONIAN_BRONZE:
        add_flag(flgs, OF_RES_CONF);
        break;
    case DRACONIAN_CRYSTAL:
        add_flag(flgs, OF_RES_SHARDS);
        if (p_ptr->lev >= 40)
            add_flag(flgs, OF_REFLECT);
        break;
    case DRACONIAN_GOLD:
        add_flag(flgs, OF_RES_SOUND);
        break;
    case DRACONIAN_SHADOW:
        add_flag(flgs, OF_RES_NETHER);
        break;
    }
}
/* cf design/dragons.ods */
static int _draconian_attack_level(void)
{
    int l = p_ptr->lev * 2;
    switch (p_ptr->psubrace)
    {
    case DRACONIAN_RED:
    case DRACONIAN_WHITE:
        l = MAX(1, l * 105 / 100);
        break;

    case DRACONIAN_BLACK:
    case DRACONIAN_GREEN:
        break;

    case DRACONIAN_BLUE:
        l = MAX(1, l * 95 / 100);
        break;

    case DRACONIAN_CRYSTAL:
    case DRACONIAN_BRONZE:
    case DRACONIAN_GOLD:
        l = MAX(1, l * 90 / 100);
        break;

    case DRACONIAN_SHADOW:
        l = MAX(1, l * 85 / 100);
        break;
    }

    switch (p_ptr->pclass)
    {
    case CLASS_BERSERKER:
        l = MAX(1, l * 170 / 100);
        break;
    case CLASS_WARRIOR:
    case CLASS_MONK:
    case CLASS_BLOOD_KNIGHT:
        l = MAX(1, l * 120 / 100);
        break;
    case CLASS_PALADIN:
    case CLASS_CHAOS_WARRIOR:
        l = MAX(1, l * 110 / 100);
        break;
    case CLASS_RED_MAGE:
    case CLASS_WEAPONSMITH:
    case CLASS_ROGUE:
    case CLASS_ALCHEMIST:
        l = MAX(1, l * 105 / 100);
        break;
    case CLASS_PRIEST:
    case CLASS_MINDCRAFTER:
    case CLASS_MAGIC_EATER:
    case CLASS_ARCHAEOLOGIST:
    case CLASS_LAWYER:
    case CLASS_WILD_TALENT:
    case CLASS_PSION:
    case CLASS_SCOUT:
    case CLASS_DEVICEMASTER:
    case CLASS_FORCETRAINER:
    case CLASS_SKILLMASTER:
    case CLASS_DISCIPLE:
        /*l = MAX(1, l * 100 / 100);*/
        break;
    case CLASS_BARD:
    case CLASS_BLUE_MAGE:
    case CLASS_TIME_LORD:
    case CLASS_WARLOCK:
    case CLASS_POLITICIAN:
    case CLASS_RAGE_MAGE:
        l = MAX(1, l * 90 / 100);
        break;
    case CLASS_NINJA:
    case CLASS_NINJA_LAWYER:
    case CLASS_MAGE:
    case CLASS_HIGH_MAGE:
    case CLASS_TOURIST:
    case CLASS_MIRROR_MASTER:
    case CLASS_BLOOD_MAGE:
    case CLASS_YELLOW_MAGE:
    case CLASS_GRAY_MAGE:
        l = MAX(1, l * 80 / 100);
        break;
    case CLASS_SORCERER:
        l = MAX(1, l * 50 / 100);
        break;
    }

    return MAX(1, l);
}
static void _draconian_calc_innate_attacks(void)
{
    int l = _draconian_attack_level();
    int l2 = p_ptr->lev; /* Note: Using attack_level() for both dd and ds gives too much variation */
    int to_d = 0;
    int to_h = l2*3/5;

    if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
    {
        /* These monk postures get "1 try" when selecting a monk attack.
           Translate this into a lower attack rating. */
        if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
            l = (l + 2) / 3;
        /* Conversely, the White Tiger gets more "tries" for martial arts,
           so needs a better power rating */
        else if (p_ptr->special_defense & KAMAE_BYAKKO)
            l = l * 5 / 4;
    }

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 15;
        a.ds = 3 + l2 / 16; /* d6 max for everybody */
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 100 + l;
        calc_innate_blows(&a, 400);
        a.msg = "You claw.";
        a.name = "爪击";

        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
        {
            a.effect[1] = GF_STUN;
            a.effect_chance[1] = 15 + l/4;

            if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
                a.blows = MAX(100, a.blows - 200);
        }

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l2 / 10; /* 6d max for everybody */
        a.ds = 4 + l / 6;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 200 + 2 * l;

        if (l >= 175) /* White Berserker Only */
            calc_innate_blows(&a, 400);
        else if (l >= 160) /* Berserker Only */
            calc_innate_blows(&a, 300);
        else if (l >= 135) /* Berserker Only */
            calc_innate_blows(&a, 250);
        else if (l >= 85)  /* CL50 for a Shadow Priest */
            calc_innate_blows(&a, 200);
        else if (l >= 70) /* CL45 for a White Mage (Max Rating = 84) */
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite.";
        a.name = "撕咬";

        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
        {
            if (p_ptr->special_defense & (KAMAE_GENBU | KAMAE_SUZAKU))
                a.blows = MAX(100, a.blows - 100);
        }

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}
static void _draconian_gain_power(void)
{
    if (p_ptr->draconian_power < 0)
    {
        int idx = mut_gain_choice(mut_draconian_pred);
        mut_lock(idx);
        p_ptr->draconian_power = idx;
        if (idx == MUT_DRACONIAN_METAMORPHOSIS)
        {
            msg_print("你变成了一条巨龙！");
            equip_on_change_race();
        }
    }
    else if (!mut_present(p_ptr->draconian_power))
    {
        mut_gain(p_ptr->draconian_power);
        mut_lock(p_ptr->draconian_power);
        if (p_ptr->draconian_power == MUT_DRACONIAN_METAMORPHOSIS)
            equip_on_change_race();
    }
}
static void _draconian_gain_level(int new_level)
{
    if (new_level >= 35)
        _draconian_gain_power();
}
race_t *draconian_get_race(int psubrace)
{
    static race_t me = {0};
    static bool init = FALSE;
    static int subrace_init = -1;

    if (!init)
    {
        me.name = "龙裔";
        me.desc = "龙裔是一个具有类似龙类特征的类人种族。龙裔有几个亚种，拥有不同的抗性、喷吐能力和属性；例如，红龙裔对火焰有抗性，并且能随意喷吐火焰，而白龙裔则喷吐和抵抗寒冰。所有的龙裔都能漂浮。此外，当他们足够成熟时，他们可以选择一种特殊的龙裔力量。";

        me.base_hp = 22;

        me.flags = RACE_DEMI_TALENT;
        me.calc_bonuses = _draconian_calc_bonuses;
        me.get_powers = _draconian_get_powers;
        me.get_flags = _draconian_get_flags;
        me.gain_level = _draconian_gain_level;

        init = TRUE;
    }

    if (subrace_init != psubrace)
    {
        /* Reset to baseline */
        me.subname = NULL;
        me.subdesc = NULL;
        me.stats[A_STR] =  1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;

        me.skills.dis = -2;
        me.skills.dev = 1;
        me.skills.sav = 2;
        me.skills.stl = 1;
        me.skills.srh = 1;
        me.skills.fos = 10;
        me.skills.thn = 5;
        me.skills.thb = 1;

        me.infra = 2;

        me.exp = 190;
        me.life = 101;
        me.shop_adjust = 105;


        /* Override with New Type */
        switch (psubrace)
        {
        case DRACONIAN_RED:
            me.subname = "红色";
            me.subdesc = "Red Draconians have an affinity for fire, which they both breathe at will and resist. "
                         "Together with their White kin, they are the strongest in combat of the draconians; "
                         "but they are not so good with magic, and their stealth is quite poor. Should they choose "
                         "the power of Dragon Skin, they will gain a fiery aura as well. Should they choose the "
                         "power of Dragon Strike, their blows will burn their enemies.";
            me.stats[A_STR] += 2;
            me.stats[A_INT] -= 1;
            me.stats[A_WIS] -= 1;
            me.skills.dev -= 3;
            me.skills.stl -= 2;
            me.skills.thn += 10;
            me.life += 3;
            me.shop_adjust = 115;
            break;
        case DRACONIAN_WHITE:
            me.subname = "白色";
            me.subdesc = "White Draconians have an affinity for frost, which they both breathe at will and resist. "
                         "Together with their Red kin, they are the strongest in combat of the draconians; "
                         "but they are not so good with magic, and their stealth is quite poor. Should they choose "
                         "the power of Dragon Skin, they will gain an aura of cold as well. Should they choose the "
                         "power of Dragon Strike, their blows will freeze their enemies.";
            me.stats[A_STR] += 2;
            me.stats[A_INT] -= 1;
            me.stats[A_WIS] -= 1;
            me.skills.dev -= 3;
            me.skills.stl -= 2;
            me.skills.thn += 9;
            me.life += 3;
            me.shop_adjust = 115;
            break;
        case DRACONIAN_BLUE:
            me.subname = "蓝色";
            me.subdesc = "Blue Draconians have an affinity for lightning, which they both breathe at will "
                         "and resist. They are strong in combat, but not so good with magic or stealth. "
                         "Should they choose the power of Dragon Skin, they will gain a shocking aura as well. "
                         "Should they choose the power of Dragon Strike, their blows will electrocute "
                         "their enemies.";
            me.stats[A_STR] += 1;
            me.skills.dev -= 2;
            me.skills.stl -= 1;
            me.skills.thn += 7;
            me.life += 2;
            me.shop_adjust = 110;
            break;
        case DRACONIAN_BLACK:
            me.subname = "黑色";
            me.subdesc = "Black Draconians have an affinity for acid, which they both breathe at will "
                         "and resist. They are strong in combat, but not so good with magic or stealth. "
                         "With the power of Dragon Strike, their blows will corrode their enemies.";
            me.stats[A_STR] += 1;
            me.skills.dev -= 2;
            me.skills.stl -= 1;
            me.skills.thn += 8;
            me.life += 2;
            me.shop_adjust = 110;
            break;
        case DRACONIAN_GREEN:
            me.subname = "绿色";
            me.subdesc = "Green Draconians have an affinity for poison, which they both breathe at will "
                         "and resist. They are average in all respects among the draconians. With the "
                         "power of Dragon Strike, their blows will poison their enemies.";
            me.exp += 15;
            break;
        case DRACONIAN_BRONZE:
            me.subname = "青铜";
            me.subdesc = "Bronze Draconians are the most intelligent of their kind, and the best with "
                         "magic as well. They are seldom confused, though the same may not be said of "
                         "their enemies. With the power of Dragon Strike, even the melee attacks of "
                         "the Bronze Draconian will baffle their enemies.";
            me.stats[A_INT] += 1;
            me.skills.sav += 1;
            me.skills.thn -= 2;
            me.skills.dev += 5;
            me.exp += 25;
            me.shop_adjust = 100;
            break;
        case DRACONIAN_CRYSTAL:
            me.subname = "水晶";
            me.subdesc = "Hard of skin, the Crystal Draconian is difficult to hit in melee; but their agility "
                         "suffers, and they are not the brightest of their kind. They resist shards, which they "
                         "may also breathe on command. With the power of Dragon Skin, they gain an aura of "
                         "shards as well. With the power of Dragon Strike, even their melee attacks will shred "
                         "their enemies.";
            me.stats[A_INT] -= 2;
            me.stats[A_DEX] -= 1;
            me.stats[A_CON] += 1;
            me.skills.dev -= 3;
            me.skills.stl -= 1;
            me.skills.thn += 7;
            me.life += 2;
            me.exp += 60;
            break;
        case DRACONIAN_GOLD:
            me.subname = "金色";
            me.subdesc = "The wisest of their kind, Gold Draconians are resilient in the face of magical "
                         "attacks. They are resistant to sound, which they may also breathe at will, stunning "
                         "their opponents. With the power of Dragon Strike, even their melee attacks will "
                         "stun enemies.";
            me.stats[A_WIS] += 1;
            me.skills.dev += 3;
            me.skills.sav += 3;
            me.life += 1;
            me.exp += 30;
            me.shop_adjust = 95;
            break;
        case DRACONIAN_SHADOW:
            me.subname = "阴影";
            me.subdesc = "Lithe, stealthy and nimble, the Shadow Draconian is seldom seen in this world. "
                         "They are resistant to the forces of nether, which they may also breathe. They are the "
                         "weakest of the draconians, and the poorest in melee; but they are better than average "
                         "with magic. With the power of Dragon Strike, they may steal life from their enemies "
                         "in melee.";
            me.stats[A_STR] -= 1;
            me.stats[A_DEX] += 2;
            me.skills.dev += 2;
            me.skills.stl += 3;
            me.skills.thn -= 5;
            me.life -= 1;
            me.exp += 35;
            me.infra += 2;
            break;
        }
        subrace_init = psubrace;
    }
    me.equip_template = NULL;
    me.calc_innate_attacks = NULL;
    if (mut_present(MUT_DRACONIAN_METAMORPHOSIS))
    {
        me.equip_template = &b_info[20];
        me.calc_innate_attacks = _draconian_calc_innate_attacks;
    }
    return &me;
}

/****************************************************************
 * Dunadan
 ****************************************************************/
static void _dunadan_gain_level(int new_level)
{
	if ((new_level >= 30) && (p_ptr->prace != RACE_DOPPELGANGER))
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static void _dunadan_calc_bonuses(void)
{
    p_ptr->sustain_con = TRUE;
}
static void _dunadan_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_CON);
}
race_t *dunadan_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "登丹人";
        me.desc = "登丹人是来自西方的坚韧人类种族。这个古老的种族在各个领域都超越了普通人类的能力；然而，作为见多识广的人，很少有事物对他们来说是新鲜的，因此他们极难提升等级。登丹人在30级时获得一项半神天赋。他们的体质特别好，无法被降低。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis =  4;
        me.skills.dev =  3;
        me.skills.sav =  3;
        me.skills.stl =  2;
        me.skills.srh =  3;
        me.skills.fos = 13;
        me.skills.thn = 15;
        me.skills.thb =  7;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 160;
        me.infra = 0;
        me.shop_adjust = 100;

        me.flags = RACE_DEMI_TALENT;
        me.calc_bonuses = _dunadan_calc_bonuses;
        me.get_flags = _dunadan_get_flags;

	me.gain_level = _dunadan_gain_level;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Dwarf
 ****************************************************************/
static power_info _dwarf_get_powers[] =
{
    { A_WIS, {5, 5, 50, detect_doors_stairs_traps_spell}},
    { A_CHR, {10, 5, 50, detect_treasure_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _dwarf_calc_bonuses(void)
{
    res_add(RES_BLIND);
}
static void _dwarf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_BLIND);
}
race_t *dwarf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "矮人";
        me.desc = "矮人是传说中固执的矿工和战士。他们往往强壮而坚韧，但缺乏敏捷。由于他们非常固执并且也有几分智慧，矮人对施加在他们身上的邪恶诅咒相对较有抵抗力。他们非常擅长战斗、搜索和察觉，并能抵抗致盲，但他们的潜行能力之差是出了名的。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = 2;
        me.skills.dev = 5;
        me.skills.sav = 6;
        me.skills.stl = -1;
        me.skills.srh = 7;
        me.skills.fos = 10;
        me.skills.thn = 15;
        me.skills.thb = 0;

        me.life = 103;
        me.base_hp = 22;
        me.exp = 135;
        me.infra = 5;
        me.shop_adjust = 115;

        me.calc_bonuses = _dwarf_calc_bonuses;
        me.get_powers = _dwarf_get_powers;
        me.get_flags = _dwarf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Einheri
 ****************************************************************/
static power_info _einheri_get_powers[] =
{
    { A_STR, {1, 10, 50, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};

static void _einheri_gain_level(int new_level)
{
	if (new_level >= 30)
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}

static void _einheri_calc_bonuses(void)
{
    p_ptr->hold_life++;
    p_ptr->regen += 100;
}
static void _einheri_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_REGEN);
}
race_t *einheri_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "英灵战士";
        me.desc = "作为返回凡间进行最后一场战斗的已故英雄，英灵战士在肉搏战中技艺高超，并且一开始就拥有进入战斗狂暴的能力；不过，他们在使用装置方面也还过得去，而且不一定是弱小的魔法使用者。由于已经经历了一生一次的冒险，英灵战士在游戏初期非常强大，但学习新事物的速度很慢。与其他不死种族一样，他们免疫吸取生命的攻击。英灵战士有一个严重的弱点：魔法治疗对他们只有部分效果，治疗量只有其他种族的一半。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;

        me.skills.dis = 5;
        me.skills.dev = 3;
        me.skills.sav = 8;
        me.skills.stl = -1;
        me.skills.srh = 7;
        me.skills.fos = 10;
        me.skills.thn = 22;
        me.skills.thb = 8;

        me.life = 113;
        me.base_hp = 44;
        me.exp = 160;
        me.infra = 3;
        me.shop_adjust = 100;

        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_DEMI_TALENT;
        me.calc_bonuses = _einheri_calc_bonuses;
        me.get_powers = _einheri_get_powers;
        me.get_flags = _einheri_get_flags;
        me.gain_level = _einheri_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Ent
 ****************************************************************/
static power_info _ent_get_powers[] =
{
    { A_WIS, {10, 20, 70, summon_tree_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _ent_calc_bonuses(void)
{
    /*res_add_vuln(RES_FIRE); cf resists.c res_pct_aux() for an alternative*/
    res_add(RES_POIS);
    if (!equip_find_first(object_is_melee_weapon))
        p_ptr->skill_dig += p_ptr->lev * 10;
}
static void _ent_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
    /*add_flag(flgs, TR_VULN_FIRE);*/
}
static void _ent_birth(void)
{
    py_birth_obj_aux(TV_POTION, SV_POTION_WATER, rand_range(15, 23));
    py_birth_light();
}
race_t *ent_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "树人";
        me.desc = "树人是一个从世界伊始就存在的强大种族，是居住在阿尔达（Arda）的所有动植物中最古老的。作为这片土地的精灵，他们守护着中土世界的森林。由于非常像树木，他们强壮但非常笨拙，并且极易受到火焰的伤害。树人通过喝水来获取营养；凡人的食物对他们几乎没有用处。树人拥有召唤森林中的树木来到他们身边的特殊能力。";

        me.skills.dis = -5;
        me.skills.dev =  1;
        me.skills.sav =  5;
        me.skills.stl = -1;
        me.skills.srh =  0;
        me.skills.fos =  9;
        me.skills.thn = 15;
        me.skills.thb = -3;

        me.life = 105;
        me.base_hp = 25;
        me.exp = 135;
        me.infra = 0;
        me.shop_adjust = 95;

        me.birth = _ent_birth;
        me.calc_bonuses = _ent_calc_bonuses;
        me.get_powers = _ent_get_powers;
        me.get_flags = _ent_get_flags;
        init = TRUE;
    }

    /* Since Ent racial stat bonuses are level dependent, we recalculate.
       Note, this prevents hackery in files.c for displaying racial stat bonuses correctly.
    */
    {
        int amount = 0;
        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;

        if (p_ptr->lev >= 26) amount++;
        if (p_ptr->lev >= 41) amount++;
        if (p_ptr->lev >= 46) amount++;
        me.stats[A_STR] += amount;
        me.stats[A_DEX] -= amount;
        me.stats[A_CON] += amount;
    }
    return &me;
}

/****************************************************************
 * Gnome
 ****************************************************************/
static power_info _gnome_get_powers[] =
{
    { A_INT, {5, 2, 50, phase_door_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _gnome_calc_bonuses(void)
{
    p_ptr->free_act++;
}
static void _gnome_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_FREE_ACT);
}
race_t *gnome_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "侏儒";
        me.desc = "侏儒比矮人小，但比霍比特人大；和霍比特人一样，他们住在地下地洞般的房子里。侏儒是优秀的魔法师，并且拥有非常好的豁免判定。他们天生擅长大多数事情，但不是很强壮，也不擅长肉搏战。侏儒拥有相当不错的红外视力，使他们能够在远处探测到温血生物。他们天生免疫麻痹。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -1;

        me.skills.dis = 10;
        me.skills.dev = 6;
        me.skills.sav = 7;
        me.skills.stl = 3;
        me.skills.srh = 6;
        me.skills.fos = 13;
        me.skills.thn = -8;
        me.skills.thb =  8;

        me.life = 95;
        me.base_hp = 16;
        me.exp = 115;
        me.infra = 4;
        me.shop_adjust = 115;

        me.calc_bonuses = _gnome_calc_bonuses;
        me.get_powers = _gnome_get_powers;
        me.get_flags = _gnome_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Golem
 ****************************************************************/
static power_info _golem_get_powers[] =
{
    { A_CON, {20, 20, 50, stone_skin_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _golem_calc_bonuses(void)
{
    int ac = 10 + (p_ptr->lev * 2 / 5);
    p_ptr->to_a += ac;
    p_ptr->dis_to_a += ac;
    p_ptr->no_stun = TRUE;

    p_ptr->slow_digest = TRUE;
    p_ptr->free_act++;
    p_ptr->see_inv++;
    res_add(RES_POIS);
    if (p_ptr->lev >= 35) p_ptr->hold_life++;

    p_ptr->pspeed -= p_ptr->lev/16;
}
static void _golem_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    if (p_ptr->lev >= 35)
        add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 16)
        add_flag(flgs, OF_DEC_SPEED);
}
static void _golem_birth(void)
{
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}
race_t *golem_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "魔像";
        me.desc = "魔像是一种人造生物，由黏土等无生命的原材料制成并被唤醒了生命。他们几乎没有心智，因此无法从事依赖魔法的职业，但作为战士他们非常坚韧。他们对毒素和麻痹有抗性，并且能识破隐形。在更高等级时，他们还会对试图吸取他们生命力的攻击产生抗性。魔像从普通食物中获得的营养极少，但可以吸收法杖和魔杖中的法力作为他们的能量来源。魔像坚硬的身躯也能为他们提供天然的护甲加成。随着年龄的增长，魔像的动作会变得迟缓。";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -5;
        me.stats[A_WIS] = -5;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] =  0;

        me.skills.dis = -5;
        me.skills.dev = -5;
        me.skills.sav = 6;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 8;
        me.skills.thn = 20;
        me.skills.thb = 0;

        me.life = 105;
        me.base_hp = 23;
        me.exp = 185;
        me.infra = 4;
        me.flags = RACE_IS_NONLIVING | RACE_EATS_DEVICES;
        me.shop_adjust = 120;

        me.birth = _golem_birth;
        me.get_powers = _golem_get_powers;
        me.calc_bonuses = _golem_calc_bonuses;
        me.get_flags = _golem_get_flags;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Half-Giant
 ****************************************************************/
static power_info _half_giant_get_powers[] =
{
    { A_STR, {20, 10, 70, stone_to_mud_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _half_giant_calc_bonuses(void)
{
    p_ptr->sustain_str = TRUE;
    res_add(RES_SHARDS);
}
static void _half_giant_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_SUST_STR);
}
race_t *half_giant_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "半巨人";
		me.desc = "半巨人有限的智力使他们很难成为纯粹的施法者，但凭借他们巨大的力量，他们能成为优秀的战士。他们厚实的皮肤使他们对碎片有抗性，并且像半巨魔一样，他们的力量属性可以维持不降。";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis = -6;
        me.skills.dev = -5;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 25;
        me.skills.thb =  0;

        me.life = 108;
        me.base_hp = 26;
        me.exp = 150;
        me.infra = 3;
        me.shop_adjust = 125;

        me.calc_bonuses = _half_giant_calc_bonuses;
        me.get_powers = _half_giant_get_powers;
        me.get_flags = _half_giant_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
* Half-Orc
****************************************************************/

static void _half_orc_gain_level(int new_level)
{
	if (new_level >= 30)
	{
		if (p_ptr->demigod_power[0] < 0)
		{
			int idx = mut_gain_choice(mut_demigod_pred/*mut_human_pred*/);
			mut_lock(idx);
			p_ptr->demigod_power[0] = idx;
		}
		else if (!mut_present(p_ptr->demigod_power[0]))
		{
			mut_gain(p_ptr->demigod_power[0]);
			mut_lock(p_ptr->demigod_power[0]);
		}
	}
}
static void _half_orc_calc_bonuses(void)
{
	res_add(RES_DARK);
}
static void _half_orc_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
	add_flag(flgs, OF_RES_DARK);
}
race_t *half_orc_get_race(void)
{
	static race_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{
		me.name = "半兽人";
		me.desc = "半兽人能成为出色的战士，但在魔法方面非常糟糕。他们在潜行方面和矮人一样差，在搜索、解除陷阱和察觉方面更是惨不忍睹。由于习惯了地下生活，半兽人对黑暗攻击有抗性。半兽人长得很丑，因此在城里买东西往往要花更多的钱。他们人类血统的那一部分允许他们在30级时选择一项天赋。";

		me.stats[A_STR] = 2;
		me.stats[A_INT] = -1;
		me.stats[A_WIS] = 0;
		me.stats[A_DEX] = 0;
		me.stats[A_CON] = 1;
		me.stats[A_CHR] = -1;

		me.skills.dis = -3;
		me.skills.dev = -3;
		me.skills.sav = -1;
		me.skills.stl = -2;
		me.skills.srh = -1;
		me.skills.fos = 5;
		me.skills.thn = 20;
		me.skills.thb = -5;

		me.life = 103;
		me.base_hp = 20;
		me.exp = 110;
		me.infra = 3;
		me.shop_adjust = 120;

                me.flags = RACE_DEMI_TALENT;
		me.calc_bonuses = _half_orc_calc_bonuses;
		me.get_flags = _half_orc_get_flags;

		me.gain_level = _half_orc_gain_level;
		init = TRUE;
	}

	return &me;
}

/****************************************************************
 * Half-Titan
 ****************************************************************/
static power_info _half_titan_get_powers[] =
{
    { A_INT, {15, 10, 60, probing_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _half_titan_calc_bonuses(void)
{
    res_add(RES_CHAOS);
}
static void _half_titan_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_CHAOS);
}
race_t *half_titan_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "半泰坦";
        me.desc = "作为强大的泰坦神族的半凡人后代，这些无比强大的生物让几乎所有其他种族都自愧不如。他们可能缺乏其他种族拥有的一些迷人的特殊能力，但他们超强的属性足以弥补这一点。他们懂得如何评估敌人的实力，而他们对法律和秩序的热爱使他们对混沌的效果具有抗性。";

        me.stats[A_STR] =  5;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  3;

        me.skills.dis = -5;
        me.skills.dev =  3;
        me.skills.sav =  1;
        me.skills.stl = -2;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn = 25;
        me.skills.thb =  0;

        me.life = 110;
        me.base_hp = 28;
        me.exp = 200;
        me.infra = 0;
        me.shop_adjust = 90;

        me.calc_bonuses = _half_titan_calc_bonuses;
        me.get_powers = _half_titan_get_powers;
        me.get_flags = _half_titan_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Half-Troll
 ****************************************************************/
static power_info _half_troll_get_powers[] =
{
    { A_STR, {10, 12, 50, berserk_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _half_troll_calc_bonuses(void)
{
    p_ptr->sustain_str = TRUE;
    p_ptr->regen += 100;
}
static void _half_troll_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_STR);
    add_flag(flgs, OF_REGEN);
}
race_t *half_troll_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "半巨魔";
        me.desc = "半巨魔非常强壮，并且比大多数其他种族拥有更多的生命值。他们非常愚蠢，不擅长搜索、解除陷阱、察觉和潜行；但尽管他们敏捷度很差，他们仍然是出色的战士。他们是如此之丑，以至于连半兽人看到他们都会做鬼脸。玩这个种族碰巧也很有趣……半巨魔的力量属性总是能维持不降，而且他们的生命再生速度极快。";

        me.stats[A_STR] =  4;
        me.stats[A_INT] = -4;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -3;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] = -2;

        me.skills.dis = -5;
        me.skills.dev = -6;
        me.skills.sav = -5;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb = -6;

        me.life = 107;
        me.base_hp = 25;
        me.exp = 150;
        me.infra = 3;
        me.shop_adjust = 135;

        me.calc_bonuses = _half_troll_calc_bonuses;
        me.get_powers = _half_troll_get_powers;
        me.get_flags = _half_troll_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * High-Elf
 ****************************************************************/
static void _high_elf_calc_bonuses(void)
{
    res_add(RES_LITE);
    p_ptr->see_inv++;
}
static void _high_elf_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_SEE_INVIS);
}
race_t *high_elf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "高等精灵";
        me.desc = "高等精灵是一个从时间伊始就存在的不朽种族。他们精通所有的技能，强壮而聪明，尽管他们的感知有时值得怀疑。高等精灵从出生起就能看穿隐形事物，并且像普通精灵一样抵抗光照效果。然而，很少有事物是他们没有见过的，因此他们很难提升新的等级。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;

        me.skills.dis =  4;
        me.skills.dev =  9;
        me.skills.sav = 12;
        me.skills.stl =  4;
        me.skills.srh =  3;
        me.skills.fos = 14;
        me.skills.thn = 10;
        me.skills.thb = 15;

        me.life = 99;
        me.base_hp = 19;
        me.exp = 190;
        me.infra = 4;
        me.shop_adjust = 90;

        me.calc_bonuses = _high_elf_calc_bonuses;
        me.get_flags = _high_elf_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Hobbit
 ****************************************************************/
static power_info _hobbit_get_powers[] =
{
    { A_INT, {15, 10, 50, create_food_spell}},
    { -1, {-1, -1, -1, NULL} }
};
race_t *hobbit_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "霍比特人";
        me.desc = "霍比特人（或半身人）拥有卓越的潜行能力，并且非常擅长弓箭和投掷；事实上，他们通常很适合冒险生活。然而，他们的小个子有时也是一种障碍；他们不太擅长近战，而且不能像体型较大的种族那样承受严重的打击。霍比特人以热爱美食而闻名，并且拥有一种近乎奇迹般的能力，可以在地牢黑暗的深处找到食物。";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = 15;
        me.skills.dev = 8;
        me.skills.sav = 10;
        me.skills.stl = 5;
        me.skills.srh = 12;
        me.skills.fos = 15;
        me.skills.thn = -10;
        me.skills.thb = 10;

        me.life = 92;
        me.base_hp = 14;
        me.exp = 120;
        me.infra = 4;
        me.shop_adjust = 100;

        me.get_powers = _hobbit_get_powers;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Human
 ****************************************************************/
static int _human_gain_weakness(void)
{
    caster_info *caster_ptr = get_caster_info();
    int _mut, _stat = A_STR;
    if ((caster_ptr) && (caster_ptr->which_stat > A_STR)) _stat = caster_ptr->which_stat;
    _mut = (MUT_HUMAN_STR + _stat - A_STR);
    mut_gain(_mut);
    return _mut;
}

static void _human_gain_power(int which)
{
	 if (p_ptr->demigod_power[which] < 0)
	 {
		 int idx = (which == 1) ? _human_gain_weakness() : mut_gain_choice(mut_demigod_pred);
		 mut_lock(idx);
		 p_ptr->demigod_power[which] = idx;
	 }
	 else if (!mut_present(p_ptr->demigod_power[which]))
	 {
		 mut_gain(p_ptr->demigod_power[which]);
		 mut_lock(p_ptr->demigod_power[which]);
	 }
}
static void _human_gain_level(int new_level)
{
	 if (new_level >= 20)
		 _human_gain_power(0);
	 if ((new_level >= 35) && (p_ptr->prace != RACE_DOPPELGANGER)) 
		 _human_gain_power(1);
}

race_t *human_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "人类";
        me.desc = "人类在各方面都很平庸，但由于寿命较短，他们往往能迅速提升等级。选择人类的角色没有任何种族属性调整或天生能力。人类在20级时会获得一项特殊天赋，这有助于弥补他们明显的平庸；但在35级时，他们会获得一个特殊的人类弱点。有关更多信息，请参阅 <link:Demigods.txt#Weaknesses>。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  0;

        me.skills.dis = 0;
        me.skills.dev = 0;
        me.skills.sav = 0;
        me.skills.stl = 0;
        me.skills.srh = 0;
        me.skills.fos = 10;
        me.skills.thn = 0;
        me.skills.thb = 0;

        me.life = 100;
        me.base_hp = 20;
        me.exp = 100;
        me.infra = 0;
        me.shop_adjust = 100;

        me.flags = RACE_DEMI_TALENT;
        me.gain_level = _human_gain_level;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Imp
 ****************************************************************/
static power_info _imp_get_powers[] =
{
    { A_INT, {9, 8, 50, imp_fire_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _imp_calc_bonuses(void)
{
    res_add(RES_FIRE);
    if (p_ptr->lev >= 10) p_ptr->see_inv++;
}
static void _imp_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_FIRE);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SEE_INVIS);
}
race_t *imp_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "小恶魔";
        me.desc = "来自下界的恶魔生物，天生对火焰攻击有抗性，并且能够学习火焰箭和火球术攻击。他们不被其他种族所喜爱，但在大多数职业中都能表现得相当好。随着等级的提升，他们将获得察觉隐形生物的能力。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] = -1;

        me.skills.dis = -3;
        me.skills.dev = 1;
        me.skills.sav = -1;
        me.skills.stl = 1;
        me.skills.srh = -1;
        me.skills.fos = 10;
        me.skills.thn = 5;
        me.skills.thb = -3;

        me.life = 99;
        me.base_hp = 19;
        me.exp = 90;
        me.infra = 3;
        me.flags = RACE_IS_DEMON;
        me.shop_adjust = 120;

        me.calc_bonuses = _imp_calc_bonuses;
        me.get_powers = _imp_get_powers;
        me.get_flags = _imp_get_flags;
        init = TRUE;
    }

    return &me;
}

