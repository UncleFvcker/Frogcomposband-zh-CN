#include "angband.h"

/****************************************************************
 * Klackon
 ****************************************************************/
static power_info _klackon_get_powers[] =
{
    { A_DEX, {9, 9, 50, spit_acid_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _klackon_calc_bonuses(void)
{
    res_add(RES_CONF);
    res_add(RES_ACID);
    p_ptr->pspeed += (p_ptr->lev) / 10;
}
static void _klackon_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_CONF);
    add_flag(flgs, OF_RES_ACID);
    if (p_ptr->lev > 9)
        add_flag(flgs, OF_SPEED);
}
race_t *klackon_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "克拉克虫人";
        me.desc = "克拉克虫人是一种奇异的半智能蚂蚁状昆虫生物。他们是伟大的战士，但他们的心智能力受到了严重限制。他们顺从而守秩序，因此对困惑有抗性。他们非常敏捷，并且随着等级的提升会变得更快。他们的酸性很强，天生抵抗强酸，并且在更高等级时能够喷吐强酸。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;

        me.skills.dis = 10;
        me.skills.dev = -2;
        me.skills.sav = 3;
        me.skills.stl = 0;
        me.skills.srh = -1;
        me.skills.fos = 10;
        me.skills.thn = 5;
        me.skills.thb = 3;

        me.life = 105;
        me.base_hp = 23;
        me.exp = 170;
        me.infra = 2;
        me.shop_adjust = 115;

        me.calc_bonuses = _klackon_calc_bonuses;
        me.get_powers = _klackon_get_powers;
        me.get_flags = _klackon_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Kobold
 ****************************************************************/
static power_info _kobold_get_powers[] =
{
    { A_DEX, {12, 8, 50, poison_dart_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _kobold_calc_bonuses(void)
{
    res_add(RES_POIS);
}
static void _kobold_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
}
race_t *kobold_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "狗头人";
        me.desc = "狗头人是一种弱小的地精类种族。他们喜欢带毒的武器，并且能学会投掷毒镖（他们携带着无限数量的毒镖）。他们也天生抵抗毒素。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] = -2;

        me.skills.dis = -2;
        me.skills.dev = -2;
        me.skills.sav = -1;
        me.skills.stl = -1;
        me.skills.srh =  1;
        me.skills.fos =  8;
        me.skills.thn = 10;
        me.skills.thb =  3;

        me.life = 98;
        me.base_hp = 19;
        me.exp = 90;
        me.infra = 3;
        me.shop_adjust = 120;

        me.calc_bonuses = _kobold_calc_bonuses;
        me.get_powers = _kobold_get_powers;
        me.get_flags = _kobold_get_flags;

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Kutar
 ****************************************************************/
static power_info _kutar_get_powers[] =
{
    { A_CHR, {20, 15, 70, kutar_expand_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _kutar_calc_bonuses(void)
{
    res_add(RES_CONF);
}
static void _kutar_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_CONF);
}
race_t *kutar_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "库塔";
        me.desc = "库塔不是狗、猫或熊。尽管他们有着标志性的呆滞表情，但库塔非常可爱，因此拥有很高的魅力。他们的心不在焉损害了他们的搜索和察觉技能，但也使他们对困惑免疫。由于他们那种超凡脱俗的平静与安详，库塔的潜行能力极强。他们能学会横向膨胀身体的特殊能力；这会增加他们的护甲等级，但也会使他们容易受到诅咒。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;

        me.skills.dis = -2;
        me.skills.dev = 3;
        me.skills.sav = 5;
        me.skills.stl = 5;
        me.skills.srh = -2;
        me.skills.fos = 6;
        me.skills.thn = 0;
        me.skills.thb = -3;

        me.life = 102;
        me.base_hp = 21;
        me.exp = 175;
        me.infra = 0;
        me.shop_adjust = 95;

        me.calc_bonuses = _kutar_calc_bonuses;
        me.get_powers = _kutar_get_powers;
        me.get_flags = _kutar_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Mindflayer
 ****************************************************************/
static power_info _mindflayer_get_powers[] =
{
    { A_INT, {5, 3, 50, mind_blast_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _mindflayer_calc_bonuses(void)
{
    p_ptr->sustain_int = TRUE;
    p_ptr->sustain_wis = TRUE;
    if (p_ptr->lev >= 15) p_ptr->see_inv++;
    if (p_ptr->lev >= 30) p_ptr->telepathy = TRUE;
}
static void _mindflayer_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_INT);
    add_flag(flgs, OF_SUST_WIS);
    if (p_ptr->lev >= 15)
        add_flag(flgs, OF_SEE_INVIS);
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_TELEPATHY);
}
race_t *mindflayer_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "夺心魔";
        me.desc = "夺心魔是一个隐秘而神秘的古老种族。他们的文明可能比任何其他种族都要古老；他们的智力和感知能够自然维持，并且极高，这使得夺心魔成为非常高效的施法者，但他们的身体属性就不那么令人钦佩了。随着等级的提升，他们将学会识破隐形生物，并最终发展出心灵感应视力。";

        me.stats[A_STR] = -3;
        me.stats[A_INT] =  4;
        me.stats[A_WIS] =  4;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -1;

        me.skills.dis = 10;
        me.skills.dev = 11;
        me.skills.sav = 9;
        me.skills.stl = 2;
        me.skills.srh = 5;
        me.skills.fos = 12;
        me.skills.thn = -10;
        me.skills.thb = -5;

        me.life = 97;
        me.base_hp = 18;
        me.exp = 150;
        me.infra = 4;
        me.shop_adjust = 115;

        me.calc_bonuses = _mindflayer_calc_bonuses;
        me.get_powers = _mindflayer_get_powers;
        me.get_flags = _mindflayer_get_flags;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Nibelung
 ****************************************************************/
static power_info _nibelung_get_powers[] =
{
    { A_WIS, {10, 5, 50, detect_doors_stairs_traps_spell}},
    { A_CHR, {10, 5, 50, detect_treasure_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _nibelung_calc_bonuses(void)
{
    res_add(RES_DISEN);
    res_add(RES_DARK);
}
static void _nibelung_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_DISEN);
    add_flag(flgs, OF_RES_DARK);
}
race_t *nibelung_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "尼伯龙根人";
        me.desc = "这些在夜间活动的矮人是受人憎恨和迫害的种族，这些穴居者并不怎么受黑暗的困扰。他们天生对魔法物品的偏好使他们能够抵抗那些解除魔法能量的效果。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] = -2;

        me.skills.dis =  3;
        me.skills.dev =  3;
        me.skills.sav =  6;
        me.skills.stl =  1;
        me.skills.srh =  5;
        me.skills.fos = 10;
        me.skills.thn = 10;
        me.skills.thb =  0;

        me.life = 101;
        me.base_hp = 21;
        me.exp = 150;
        me.infra = 5;
        me.shop_adjust = 115;

        me.calc_bonuses = _nibelung_calc_bonuses;
        me.get_powers = _nibelung_get_powers;
        me.get_flags = _nibelung_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Ogre
 ****************************************************************/
static power_info _ogre_get_powers[] =
{
    { A_INT, {25, 35, 70, explosive_rune_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _ogre_calc_bonuses(void)
{
    p_ptr->sustain_int = TRUE;
}
static void _ogre_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_INT);
}
race_t *ogre_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "食人魔";
        me.desc = "食人魔庞大而丑陋，但拥有着低级的狡猾和巨大的力量。他们具备成为战士的所有必要属性，但也出人意料地擅长成为魔法师；在高级时，所有食人魔都能学会“食人魔魔法师”设置陷阱符文的技能。由于头脑简单，食人魔的智力属性能够维持不降。";

        me.stats[A_STR] =  3;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  0;

        me.skills.dis = -3;
        me.skills.dev = -3;
        me.skills.sav = -3;
        me.skills.stl = -2;
        me.skills.srh = -1;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb =  -5;

        me.life = 106;
        me.base_hp = 23;
        me.exp = 140;
        me.infra = 0;
        me.shop_adjust = 125;

        me.calc_bonuses = _ogre_calc_bonuses;
        me.get_powers = _ogre_get_powers;
        me.get_flags = _ogre_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Shadow-Fairy
 ****************************************************************/
static void _shadow_fairy_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    p_ptr->fairy_stealth = TRUE;
    res_add_vuln(RES_LITE);
}
static void _shadow_fairy_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_VULN_LITE);
}
race_t *shadow_fairy_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "暗影妖精";
        me.desc = "暗影妖精是几个妖精种族之一。他们有翅膀，可以飞越他们脚下可能打开的陷阱。作为黑暗生物，暗影妖精在强光下很脆弱。他们身体虚弱，但天生擅长使用魔法。暗影妖精的潜行能力惊人，并且有一个极好的优势，即他们几乎从不激怒怪物；带有激怒属性的装备只会给他们的潜行带来轻微的惩罚，但如果他们具有激怒型性格，那么这个优势就会丧失。";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] = -3;

        me.skills.dis =  7;
        me.skills.dev =  6;
        me.skills.sav =  0;
        me.skills.stl =  6;
        me.skills.srh = 12;
        me.skills.fos = 15;
        me.skills.thn =-10;
        me.skills.thb = -3;

        me.life = 91;
        me.base_hp = 13;
        me.exp = 140;
        me.infra = 4;
        me.shop_adjust = 110;

        me.calc_bonuses = _shadow_fairy_calc_bonuses;
        me.get_flags = _shadow_fairy_get_flags;
        init = TRUE;
    }

    return &me;
}


/****************************************************************
 * Skeleton
 ****************************************************************/
static power_info _skeleton_get_powers[] =
{
    { A_WIS, {30, 30, 70, restore_life_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _skeleton_calc_bonuses(void)
{
    res_add(RES_SHARDS);
    p_ptr->hold_life++;
    p_ptr->see_inv++;
    res_add(RES_POIS);
    if (p_ptr->lev >= 10) res_add(RES_COLD);
}
static void _skeleton_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_RES_COLD);
}
static void _skeleton_birth(void)
{
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}
race_t *skeleton_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "骷髅";
        me.desc = "作为不死生物，骷髅几乎不需要担心毒素或能吸取生命的攻击。骷髅并不真正使用眼睛来感知事物，因此不会被隐形所欺骗。他们的骨头能抵抗尖锐的碎片，并且很快就能抵抗寒冰。虽然药水和蘑菇的魔法效果即使没有进入其（不存在的）胃部也会影响骷髅，但食物本身会从骷髅的下巴掉下去，不提供任何营养；相反，骷髅通过吸收魔法装置的能量来进食。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;

        me.skills.dis = -5;
        me.skills.dev = 0;
        me.skills.sav = 3;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 8;
        me.skills.thn = 10;
        me.skills.thb = 0;

        me.life = 100;
        me.base_hp = 21;
        me.exp = 115;
        me.infra = 2;
        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START | RACE_EATS_DEVICES;
        me.shop_adjust = 125;

        me.birth = _skeleton_birth;
        me.calc_bonuses = _skeleton_calc_bonuses;
        me.get_powers = _skeleton_get_powers;
        me.get_flags = _skeleton_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Snotling (A Joke Race)
 ****************************************************************/
static void _devour_flesh_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吞噬血肉");
        break;
    case SPELL_DESC:
        var_set_string(res, "吞噬血肉（你自己的）以填饱肚子。");
        break;
    case SPELL_FLAGS:
        var_set_int(res, PWR_CONFUSED);
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!get_check("这可能会有点疼。你确定吗？")) return;
        msg_print("你吞噬了你自己的血肉！");
        set_food(PY_FOOD_MAX - 1);
        set_cut(p_ptr->cut + CUT_SEVERE, FALSE);
        take_hit(DAMAGE_USELIFE, p_ptr->mhp / 3, "吞食自己的血肉");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _snotling_get_powers[] =
{
    { A_CHR, {1, 0, 0, _devour_flesh_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _snotling_calc_bonuses(void)
{
}
static void _snotling_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
}
static void _snotling_birth(void)
{
    py_birth_obj_aux(TV_FOOD, SV_FOOD_FAST_RECOVERY, randint1(3));
    py_birth_food();
    py_birth_light();
}
race_t *snotling_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "鼻涕精";
        me.desc = "鼻涕精属于绿皮，是哥布林和兽人的近亲，比前者小，比后者笨，经常被他们用作炮灰、食物甚至是大炮的炮弹。他们处于绿皮社会的最底层，受到所有人的欺负。鼻涕精经常挥舞着蘑菇或木棍参加战斗。";

        me.stats[A_STR] = -2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -5;

        me.skills.dis = -3;
        me.skills.dev = -2;
        me.skills.sav = -2;
        me.skills.stl = 2;
        me.skills.srh = 0;
        me.skills.fos = 7;
        me.skills.thn = -10;
        me.skills.thb = -5;

        me.life = 85;
        me.base_hp = 10;
        me.exp = 45;
        me.infra = 2;
        me.shop_adjust = 125;

        me.birth = _snotling_birth;
        me.calc_bonuses = _snotling_calc_bonuses;
        me.get_powers = _snotling_get_powers;
        me.get_flags = _snotling_get_flags;

        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Spectre
 ****************************************************************/
static power_info _spectre_get_powers[] =
{
    { A_INT, {4, 6, 50, scare_monster_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _spectre_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    res_add(RES_NETHER);
    p_ptr->hold_life++;
    p_ptr->see_inv++;
    res_add(RES_POIS);
    p_ptr->slow_digest = TRUE;
    res_add(RES_COLD);
    p_ptr->pass_wall = TRUE;
}
static void _spectre_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_LEVITATION);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
}
static void _spectre_birth(void)
{
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}
race_t *spectre_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "幽灵";
        me.desc = "作为一种强大的不死生物，幽灵是可怕的幻影，被超自然的绿光所环绕。幽灵只是部分存在于我们的生存位面上：由于半实体的特性，他们可以穿墙，尽管在穿墙的过程中墙壁的密度会伤害他们。作为不死生物，他们牢牢地掌控着自己的生命力，能识破隐形生物，并抵抗毒素、寒冰和虚空。幽灵是非常出色的施法者，但他们的物理形态非常虚弱。他们从凡人的食物中获得的营养极少，但可以吸收魔法装置中的法力作为他们的能量来源。";

        me.stats[A_STR] = -5;
        me.stats[A_INT] =  4;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -3;

        me.skills.dis = 10;
        me.skills.dev = 10;
        me.skills.sav = 12;
        me.skills.stl =  5;
        me.skills.srh =  5;
        me.skills.fos = 14;
        me.skills.thn =-15;
        me.skills.thb = -5;

        me.life = 90;
        me.base_hp = 13;
        me.exp = 250;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START | RACE_EATS_DEVICES;
        me.shop_adjust = 135;

        me.birth = _spectre_birth;
        me.calc_bonuses = _spectre_calc_bonuses;
        me.get_powers = _spectre_get_powers;
        me.get_flags = _spectre_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Sprite
 ****************************************************************/
static power_info _sprite_get_powers[] =
{
    { A_INT, {12, 12, 50, sleeping_dust_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _sprite_calc_bonuses(void)
{
    p_ptr->levitation = TRUE;
    res_add(RES_LITE);
    p_ptr->pspeed += (p_ptr->lev) / 10;
}
static void _sprite_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 10)
        add_flag(flgs, OF_SPEED);
}
race_t *sprite_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "雪碧仙子";
        me.desc = "作为几个妖精种族之一，小妖精体型非常小。他们有微小的翅膀，可以飞越他们脚下可能打开的陷阱。他们极度享受阳光，并且几乎不需要担心基于光照的攻击。虽然身体上属于最弱的种族之列，但小妖精在魔法方面非常有天赋，并且可以成为技术高超的巫师。小妖精拥有喷洒催眠粉的特殊能力，在更高等级时他们会学会飞得更快。";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;

        me.skills.dis = 10;
        me.skills.dev =  6;
        me.skills.sav =  6;
        me.skills.stl =  4;
        me.skills.srh = 10;
        me.skills.fos = 10;
        me.skills.thn =-12;
        me.skills.thb =  0;

        me.life = 92;
        me.base_hp = 14;
        me.exp = 135;
        me.infra = 4;
        me.shop_adjust = 90;

        me.calc_bonuses = _sprite_calc_bonuses;
        me.get_powers = _sprite_get_powers;
        me.get_flags = _sprite_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Tomte
 ****************************************************************/
static power_info _tomte_get_powers[] =
{
    { A_INT, {1, 0, 20, probing_spell}},
    { -1, {-1, -1, -1, NULL} }
};

int tomte_heavy_armor(void)
{
    slot_t slot = equip_find_obj(TV_HELM, SV_ANY);
    if (!slot) slot = equip_find_obj(TV_CROWN, SV_ANY);
    if (slot)
    {
        object_type *lakki = equip_obj(slot);
        if ((lakki) && (lakki->weight > 10))
        {
            return (lakki->weight - 10);
        }
    }
    return 0;
}
static void _tomte_calc_bonuses(void)
{
    int ylipaino = tomte_heavy_armor();
    res_add(RES_COLD);
    p_ptr->pspeed += (p_ptr->lev) / 15;
    if (ylipaino)
    {
        p_ptr->stat_add[A_INT] -= ((ylipaino / 10) + 1);
        p_ptr->skills.dev -= ((ylipaino + 6) / 2);
    }
    else
    {
        p_ptr->auto_pseudo_id = TRUE;
        if (p_ptr->lev >= 40) p_ptr->auto_id = TRUE;
    }
}
static void _tomte_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_COLD);
    if (p_ptr->lev >= 15)
        add_flag(flgs, OF_SPEED);
}
static void _tomte_birth(void)
{
    py_birth_obj_aux(TV_HELM, SV_KNIT_CAP, 1);
    py_birth_food();
    py_birth_light();
}
race_t *tomte_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "尼森小人";
        me.desc = "尼森小人比普通精灵体型更小、更轻盈，他们敏捷、聪明、潜行能力极强并且精通使用魔法装置，但他们的体型在物理战斗中对他们很不利。他们非常偏爱轻便的头饰，如果戴着沉重的头盔挤压他们的大脑，他们就无法清晰地思考。尼森小人有一眼就能评估怪物和物品的能力（他们在40级时获得自动鉴定）；由于习惯了漫长的冬天，他们不太怕冷。";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] =  0;

        me.skills.dis = 10;
        me.skills.dev = 15;
        me.skills.sav =  0;
        me.skills.stl =  6;
        me.skills.srh = 10;
        me.skills.fos = 10;
        me.skills.thn = -1;
        me.skills.thb = -5;

        me.life = 94;
        me.base_hp = 13;
        me.exp = 150;
        me.infra = 4;
        me.shop_adjust = 100;

        me.calc_bonuses = _tomte_calc_bonuses;
        me.get_powers = _tomte_get_powers;
        me.get_flags = _tomte_get_flags;
        me.birth = _tomte_birth;
        me.boss_r_idx = MON_SANTACLAUS;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Tonberry
 ****************************************************************/
static void _tonberry_calc_bonuses(void)
{
    p_ptr->sustain_str = TRUE;
    p_ptr->sustain_con = TRUE;
    res_add(RES_FEAR);

    p_ptr->pspeed -= 1;
    if (p_ptr->lev >= 30)
        p_ptr->pspeed -= 1;
    if (p_ptr->lev >= 40)
        p_ptr->pspeed -= 1;
    if (p_ptr->lev >= 45)
        p_ptr->pspeed -= 1;
    if (p_ptr->lev >= 50)
        p_ptr->pspeed -= 1;

    if (p_ptr->pclass != CLASS_DUELIST)
    {
        int hand;
        for (hand = 0; hand < MAX_HANDS; hand++)
        {
            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            {
                p_ptr->weapon_info[hand].to_d += 2 * p_ptr->lev / p_ptr->weapon_ct;
                p_ptr->weapon_info[hand].dis_to_d += 2 * p_ptr->lev / p_ptr->weapon_ct;
                p_ptr->weapon_info[hand].xtra_blow -= 4 * p_ptr->lev;
            }
        }
    }
    /* Tonberries are also vulnerable to confusion ... cf res_pct_aux in resist.c */
}
static void _tonberry_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SUST_STR);
    add_flag(flgs, OF_SUST_CON);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_DEC_SPEED);
    add_flag(flgs, OF_VULN_CONF);
}
race_t *tonberry_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "冬贝利";
        me.desc = "冬贝利是一种类似蜥蜴的生物，他们拥有巨大的力量，并且偏爱菜刀和宽刃刀。然而，他们的动作和反应都很迟钝；年轻且缺乏经验的冬贝利经常被其他种族捕食。他们拥有类似人类的智力，但由于文化和生理原因很少成为魔法师。冬贝利在近战中的攻击非常强力，尽管攻击次数比正常情况下少。他们也容易被困惑，并且移动速度较慢。";

        me.stats[A_STR] =  4;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -4;
        me.stats[A_CON] =  5;
        me.stats[A_CHR] =  0;

        me.skills.dis = -5;
        me.skills.dev = -3;
        me.skills.sav =  3;
        me.skills.stl =  1;
        me.skills.srh = -2;
        me.skills.fos =  5;
        me.skills.thn = 20;
        me.skills.thb = -7;

        me.life = 108;
        me.base_hp = 26;
        me.exp = 175;
        me.infra = 2;
        me.shop_adjust = 115;

        me.calc_bonuses = _tonberry_calc_bonuses;
        me.get_flags = _tonberry_get_flags;
        me.boss_r_idx = MON_MASTER_TONBERRY;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Vampire
 ****************************************************************/
static power_info _vampire_get_powers[] =
{
    { A_CON, {2, 1, 60, vampirism_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _vampire_calc_bonuses(void)
{
    res_add(RES_DARK);
    res_add(RES_NETHER);
    res_add(RES_COLD);
    res_add(RES_POIS);
    res_add_vuln(RES_LITE);
    p_ptr->hold_life++;
    if (!player_is_ninja) p_ptr->lite = TRUE;
}
static void _vampire_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_VULN_LITE);

    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_DARK);
    add_flag(flgs, OF_RES_NETHER);
    if (!player_is_ninja) add_flag(flgs, OF_LITE);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_COLD);
}
static void _vampire_birth(void)
{
    if (p_ptr->pclass != CLASS_BERSERKER) py_birth_obj_aux(TV_SCROLL, SV_SCROLL_DARKNESS, rand_range(2, 5));
}
race_t *vampire_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "吸血鬼";
        me.desc = "作为比较强大的不死生物之一，吸血鬼那令人敬畏的外表令人印象深刻。然而，这种可怕的生物有一个严重的弱点：明亮的阳光是他们的克星，他们需要从地表逃到地下的深处，直到太阳最终落山。另一方面，黑暗对吸血鬼来说并不可怕。作为不死生物，吸血鬼还能抵抗虚空、寒冰和毒素，并牢牢掌控着自己的生命力；但他们对新鲜血液有着永久的饥渴，这只能通过吸取附近怪物的血液来满足。";

        me.stats[A_STR] =  3;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  2;

        me.skills.dis = 4;
        me.skills.dev = 5;
        me.skills.sav = 6;
        me.skills.stl = 4;
        me.skills.srh = 1;
        me.skills.fos = 8;
        me.skills.thn = 5;
        me.skills.thb = 0;

        me.life = 102;
        me.base_hp = 22;
        me.exp = 200;
        me.infra = 5;
        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START;
        me.shop_adjust = 130;

        me.birth = _vampire_birth;
        me.calc_bonuses = _vampire_calc_bonuses;
        me.get_powers = _vampire_get_powers;
        me.get_flags = _vampire_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Wood-Elf
 ****************************************************************/
static power_info _wood_elf_get_powers[] =
{
    { A_WIS, {20, 15, 50, nature_awareness_spell}},
    { -1, {-1, -1, -1, NULL} }
};
race_t *wood_elf_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "木精灵";
        me.desc = "木精灵是最常见的精灵。他们喜欢幽居在茂密的森林中，在茂密的枝叶中穿行时不受阻碍。他们在追踪和弓箭方面的技巧是无与伦比的，并且随着等级的提升，他们将获得“自然感知”的能力。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -1;
        me.stats[A_CHR] =  1;

        me.skills.dis = 5;
        me.skills.dev = 4;
        me.skills.sav = 4;
        me.skills.stl = 3;
        me.skills.srh = 8;
        me.skills.fos = 12;
        me.skills.thn = -5;
        me.skills.thb = 12;

        me.life = 97;
        me.base_hp = 16;
        me.exp = 125;
        me.infra = 3;
        me.shop_adjust = 95;

        me.get_powers = _wood_elf_get_powers;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Yeek
 ****************************************************************/
static power_info _yeek_get_powers[] =
{
    { A_WIS, {15, 15, 50, scare_monster_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _yeek_calc_bonuses(void)
{
    res_add(RES_ACID);
    if (p_ptr->lev >= 20)
        res_add_immune(RES_ACID);
}
static void _yeek_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_ACID);
    if (p_ptr->lev >= 20)
        add_flag(flgs, OF_IM_ACID);
}
race_t *yeek_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "伊克人";
        me.desc = "伊克人是最可悲的生物之一。幸运的是，他们可怕的尖叫声可以吓跑不太自信的敌人，而且随着经验的增长，他们的皮肤会越来越抗酸。他们在魔法方面也相当不错；但在战斗中，即使是一只平庸的怪物也能把粗心大意的伊克人按在地上摩擦。";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -4;

        me.skills.dis = 2;
        me.skills.dev = 3;
        me.skills.sav = 6;
        me.skills.stl = 3;
        me.skills.srh = 5;
        me.skills.fos = 15;
        me.skills.thn = -5;
        me.skills.thb = -3;

        me.life = 92;
        me.base_hp = 14;
        me.exp = 70;
        me.infra = 2;
        me.shop_adjust = 105;

        me.calc_bonuses = _yeek_calc_bonuses;
        me.get_powers = _yeek_get_powers;
        me.get_flags = _yeek_get_flags;
        init = TRUE;
    }

    return &me;
}

/****************************************************************
 * Zombie
 ****************************************************************/
static power_info _zombie_get_powers[] =
{
    { A_WIS, {30, 30, 70, restore_life_spell}},
    { -1, {-1, -1, -1, NULL} }
};
static void _zombie_calc_bonuses(void)
{
    res_add(RES_NETHER);
    p_ptr->hold_life++;
    p_ptr->see_inv++;
    res_add(RES_POIS);
    p_ptr->slow_digest = TRUE;
    if (p_ptr->lev >= 5) res_add(RES_COLD);
}
static void _zombie_get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SEE_INVIS);
    add_flag(flgs, OF_HOLD_LIFE);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SLOW_DIGEST);
    if (p_ptr->lev >= 5)
        add_flag(flgs, OF_RES_COLD);
}
static void _zombie_birth(void)
{
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
    py_birth_light();
}
race_t *zombie_get_race(void)
{
    static race_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        me.name = "僵尸";
        me.desc = "僵尸是恐怖的不死生物，对生命吸取和虚空力量有抗性。坟墓很冷，但这并不困扰不死生物，毒素也几乎影响不到他们。僵尸从普通食物中获得的营养极少；相反，他们通过吸收魔法装置的能量来维持其不死状态。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -6;
        me.stats[A_WIS] = -6;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  4;
        me.stats[A_CHR] = -3;

        me.skills.dis = -5;
        me.skills.dev = -5;
        me.skills.sav = 5;
        me.skills.stl = -1;
        me.skills.srh = -1;
        me.skills.fos = 5;
        me.skills.thn = 15;
        me.skills.thb = 0;

        me.life = 108;
        me.base_hp = 24;
        me.exp = 180;
        me.infra = 2;
        me.flags = RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START | RACE_EATS_DEVICES;
        me.shop_adjust = 140;

        me.birth = _zombie_birth;
        me.calc_bonuses = _zombie_calc_bonuses;
        me.get_powers = _zombie_get_powers;
        me.get_flags = _zombie_get_flags;
        init = TRUE;
    }

    return &me;
}

