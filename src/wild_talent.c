#include "angband.h"

void burning_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "燃烧打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "对怪物造成更多伤害，除非它对火焰有抗性。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_FIRE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void lightning_eagle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪电飞鹰");
        break;
    case SPELL_DESC:
        var_set_string(res, "对怪物造成更多伤害，除非它对闪电有抗性。");
        break;
    case SPELL_CAST:
        var_set_bool(res, do_blow(HISSATSU_ELEC));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/*
 * We are using p_ptr->magic_num to remember talents.
 * 0 indicates no talent for this group, so we always subtract
 * 1 before indexing into an array.
 */

typedef struct {
int        stat;
cptr       gain_desc;
spell_info spell;
} talent_t;

/*
 * Talents are grouped. Each group of talent will contain abilities
 * of a similar kind, such as offense, defense, detection.
 */
#define _MAX_TALENTS 25
#define _MAX_TALENTS_PER_GROUP 10

static power_info _wonder_powers[2] =
{
    { A_INT,  {10, 10, 30, wonder_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};


static talent_t _talents[_MAX_TALENTS][_MAX_TALENTS_PER_GROUP] = 
{
    /* CL1: Weak offense */
    {
        { A_INT, "像黑暗精灵一样", {1, 1, 30, magic_missile_spell}},
        { A_STR, "像人造人一样", {1, 1, 30, android_ray_gun_spell}},
        { A_CON, "像吸血鬼一样", {1, 1, 70, vampirism_spell}},
        { A_DEX, "像新手游侠一样", {1, 1, 30, shoot_arrow_spell}},
        { A_WIS, "像新手圣骑士一样", {1, 1, 30, cause_wounds_I_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL3: Weak utility */
    {
        { A_CHR, "像伊克人一样", {3, 5, 30, scare_monster_spell}},
        { A_CHR, "像半兽人一样", {3, 5, 50, remove_fear_spell}},
        { A_WIS, "像牧师一样", {3, 5, 60, satisfy_hunger_spell}},
        { A_INT, "像法师一样", {3, 5, 40, light_area_spell}},
        { A_CHR, "像变异人一样", {3, 12, 40, hypnotic_gaze_spell}},
        { A_WIS, "像牧师一样", {3, 5, 40, bless_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL5: Middle Utility */
    {
        { A_INT, "像侏儒一样", {3, 5, 50, phase_door_spell}},
        { A_DEX, "像弓箭手一样", {1, 0, 0, create_ammo_spell}},
        { A_WIS, "像矮人一样", {5, 5, 50, detect_doors_stairs_traps_spell}},
        { A_INT, "像变异人一样", {3, 2, 30, smell_metal_spell}},
        { A_CHR, "像龙一样", {5, 5, 50, detect_objects_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },

    /* CL7: Middle Utility */
    {
        { A_CHR, "像咒术师一样", {5, 5, 40, detect_monsters_spell}},
        { A_STR, "像狂战士一样", {7, 5, 40, detect_menace_spell}},
        { A_INT, "像王牌法师一样", {7, 12, 50, telepathy_spell}},
        { A_WIS, "像牧师一样", {7, 5, 40, detect_evil_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL9: Middle Offense */
    {
        { A_CHR, "像驯兽师一样", {1, 0, 30, dominate_living_I_spell}},
        { A_INT, "像小恶魔一样", {9, 7, 50, imp_fire_spell}},
        { A_STR, "像人造人一样", {9, 7, 30, android_blaster_spell}},
        { A_DEX, "像狗头人一样", {9, 8, 50, poison_dart_spell}},
        { A_WIS, "像月兽一样", {9, 7, 40, cause_wounds_II_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL11: Middle Offense */
    {
        { A_CON, "像变异人一样", {11, 15, 30, radiation_spell}},
        { A_CON, "像变异人一样", {11, 14, 40, shriek_spell}},
        { A_WIS, "像变异人一样", {7, 10, 50, laser_eye_spell}},
        { A_CON, "像变异人一样", {2, 2, 30, cold_touch_spell}},
        { A_STR, "像变异人一样", {1, 0, 40, power_throw_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL13: Good Utility */
    {
        { A_DEX, "像狂战士一样", {10, 10, 50, recall_spell}},
        { A_INT, "像天狗一样", {13, 10, 50, teleport_to_spell}},
        { A_CHR, "像树人一样", {10, 20, 50, summon_tree_spell}},
        { A_INT, "像妖精一样", {7, 12, 50, sleeping_dust_spell}},
        { A_CON, "像变异人一样", {7, 12, 40, eat_rock_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL15: Middle Offense */
    {
        { A_INT, "像夺心魔一样", {15, 12, 50, mind_blast_spell}},
        { A_CON, "像变异人一样", {15,  0,  40, breathe_fire_I_spell}},
        { A_CHR, "像驯兽师一样", {15, 0, 50, dominate_living_II_spell}},
        { A_WIS, "像黑骑士一样", {15, 12, 40, cause_wounds_III_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL17: Middle Offense */
    {
        { A_STR, "像独眼巨人一样", {17, 15, 50, throw_boulder_spell}},
        { A_STR, "像人造人一样", {17, 20, 40, android_bazooka_spell}},
        { A_CHR, "像黑暗精灵邪术师一样", {17, 15, 50, mana_bolt_I_spell}},
        { A_DEX, "像武士一样", {17, 12, 40, burning_strike_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL19: Good Utility */
    {
        { A_INT, "像半泰坦一样", {15, 10, 40, probing_spell}},
        { A_STR, "像半巨人一样", {19, 10, 40, stone_to_mud_spell}},
        { A_CHR, "像咒术师一样", {19, 20, 50, identify_spell}},
        { A_CHR, "像法师一样", {19, 10, 40, teleport_spell}},
        { A_INT, "像变异人一样", {15, 12, 40, swap_pos_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL21: Good Buff*/
    {
        { A_STR, "像半巨魔一样", {10, 12, 50, berserk_spell}},
        { A_CON, "像魔像一样", {20, 15, 50, stone_skin_spell}},
        { A_CHR, "像库塔一样", {20, 15, 40, kutar_expand_spell}},
        { A_CHR, "像神圣骑士一样", {15, 10, 30, heroism_spell}},
        { A_CHR, "像圣战圣骑士一样", {21, 40, 40, protection_from_evil_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL23: Good Recovery */
    {
        { A_CON, "像僵尸一样", {23, 30, 50, restore_life_spell}},
        { A_WIS, "像原力武士一样", {15, 0, 30, clear_mind_spell}},
        { A_CHR, "像牧师一样", {20, 20, 50, remove_curse_I_spell}},
        { A_WIS, "像牧师一样", {20, 15, 50, cure_wounds_III_spell}},
        { A_WIS, "像游侠一样", {8, 7, 50, resist_environment_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL25: Good Offense */
    {
        { A_DEX, "像克拉克恩人一样", {9, 9, 40, spit_acid_spell}},
        { A_CON, "像炎魔一样", {15, 10, 50, demon_breath_spell}},
        { A_WIS, "像青魔导一样", {20, 10,  40, brain_smash_spell}},
        { A_INT, "像混沌战士一样", {5, 4, 30, touch_of_confusion_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL27: Good Utility */
    {
        { A_DEX, "像盗贼一样", {8, 12, 50, panic_hit_spell}},
        { A_INT, "像食人魔一样", {25, 35, 40, explosive_rune_spell}},
        { A_CHR, "像巫师一样", {25, 20, 50, magic_mapping_spell}},
        { A_CHR, "像法师一样", {25, 40, 50, recharging_spell}},
        { A_INT, "像变异人一样", {10, 5, 50, alchemy_spell}},
        { A_WIS, "像心灵感应者一样", {9, 9, 40, telekinesis_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL29: Good Recovery/Ability */
    {
        { A_INT, "像战士法师一样", {25, 0, 50, hp_to_sp_spell}},
        { A_INT, "像战士法师一样", {25, 0, 50, sp_to_hp_spell}},
        { A_INT, "像法师一样", {25, 1, 70, eat_magic_spell}},
        { A_DEX, "像忍者一样", {20, 0, 0, quick_walk_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL31: Good Offense */
    {
        { A_WIS, "像善良圣骑士一样", {30, 30, 40, holy_lance_spell}},
        { A_WIS, "像邪恶圣骑士一样", {30, 30, 40, hell_lance_spell}},
        { A_STR, "像人造人一样", {30, 30, 50, android_beam_cannon_spell}},
        { A_INT, "像修道院巫妖一样", {30, 30, 40, cause_wounds_IV_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL33: Good Utility */
    {
        { A_INT, "像安珀人一样", {30, 50, 50, shadow_shifting_spell}},
        { A_INT, "像游客一样", {25, 20, 30, identify_fully_spell}},
        { A_CHR, "像土元素一样", {30, 10, 40, earthquake_spell}},
        { A_CHR, "像巫师一样", {30, 20, 40, teleport_level_spell}},
        { A_WIS, "像牧师一样", {30, 30, 50, healing_I_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL35: Good Offense/Ability */
    {
        { A_DEX, "像武士一样", {33, 20, 60, lightning_eagle_spell}},
        { A_DEX, "像武僧一样", {30, 30, 60, monk_double_attack_spell}},
        { A_INT, "像暗影杰克一样", {35, 30, 50, darkness_storm_I_spell}},
        { A_WIS, "像耀光天神一样", {35, 30, 50, starburst_I_spell}},
        { A_CON, "像高等炎魔一样", {35, 10, 50, breathe_fire_II_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL37: Great Buff */
    {
        { A_DEX, "像武僧一样", {25, 0, 0, monk_posture_spell}},
        { A_DEX, "像武士一样", {25, 0, 0, samurai_posture_spell}},
        { A_CON, "像变异人一样", {25, 10, 50, resist_elements_spell}},
        { A_INT, "像恶魔法师一样", {35, 40, 80, polymorph_demon_spell}},
        { A_CHR, "像邪术师一样", {35, 40, 80, polymorph_vampire_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL39: Great Utility */
    {
        { A_CHR, "像变异人一样", {12, 23, 70, sterility_spell}},
        { A_CHR, "像变异人一样", {7, 15, 60, dazzle_spell}},
        { A_WIS, "像变异人一样", {25, 25, 70, banish_evil_spell}},
        { A_CON, "像变异人一样", {1, 6, 60, grow_mold_spell}},
        { A_DEX, "像武士一样", {30, 20, 65, rush_attack_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL41: Great Offense */
    {
        { A_STR, "像人造人一样", {40, 40, 70, android_rocket_spell}},
        { A_WIS, "像心灵感应者一样", {40, 40, 70, psycho_spear_spell}},
        { A_CON, "像毁灭者一样", {40, 40, 70, breathe_disintegration_spell}},
        { A_CHR, "像眼之德鲁杰一样", {40, 35, 50, mana_bolt_II_spell}},
        { A_INT, "像混沌大将，哈布一样", {40, 40, 65, mana_storm_I_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL43: Great Utility */
    {
        { A_WIS, "像安珀人一样", {40, 75, 75, pattern_mindwalk_spell}},
        { A_CHR, "像混沌领主一样", {35, 70, 60, destruction_spell}},
        { A_CHR, "像王牌法师一样", {40, 50, 65, dimension_door_spell}},
        { A_WIS, "像生命牧师一样", {35, 70, 90, clairvoyance_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL45: Great Utility */
    {
        { A_INT, "像混沌战士一样", {40, 50, 60, confusing_lights_spell}},
        { A_WIS, "像邪恶牧师一样", {40, 40, 60, evocation_spell}},
        { A_STR, "像狂战士一样", {40, 50, 80, massacre_spell}},
        { A_DEX, "像忍者一样", {40, 50, 80, hide_in_mud_spell}},
        { A_WIS, "像圣战圣骑士一样", {44, 50, 80, eye_for_an_eye_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL47: General Awesomeness */
    {
        { A_DEX, "像忍者一样", {40, 50, 50, super_stealth_spell}},
        { A_WIS, "像牧师一样", {40, 50, 80, healing_II_spell}},
        { A_INT, "像诅咒法师一样", {40, 50, 60, building_up_spell}},
        { A_INT, "像工匠法师一样", {47, 70, 70, polymorph_colossus_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
    /* CL49: Capstone Offense */
    {
        { A_INT, "像蜘蛛之神，阿特拉克·纳克亚一样", {49, 50, 65, darkness_storm_II_spell}},
        { A_WIS, "像信使，拉斐尔一样", {49, 50, 65, starburst_II_spell}},
        { A_STR, "像赛博恶魔领主，奥瑞莫吉一样", {49, 50, 65, rocket_II_spell}},
        { A_CON, "像黑暗领主，魔苟斯一样", {49, 50, 70, mana_storm_II_spell}},
        { A_INT, "像工匠法师一样", {49, 50, 70, force_branding_spell}},
        { -1, NULL, {0, 0, 0, NULL}},
    },
};

static int _group_size(int i)
{
    int result = 0;
    if (i >= 0 && i < _MAX_TALENTS)
    {
        int j;
        for (j = 0; j < _MAX_TALENTS_PER_GROUP ; ++j)
        {
            talent_t *talent_ptr = &_talents[i][j];
            if (talent_ptr->stat == -1)
            {
                result = j;
                break;
            }
        }
    }
    return result;
}

static int _which_stat(int idx)
{
    int which = p_ptr->magic_num1[idx] - 1;    /* Magic Numbers are base 1, Table indices base 0 */
    talent_t *talent = &_talents[idx][which];
    return talent->stat;
}

static int _get_spells_imp(power_info* spells, int max, int start, int stop)
{
    int ct = 0, i;
    for (i = start; i <= stop; ++i)
    {
        int idx = p_ptr->magic_num1[i] - 1;    /* Magic Numbers are base 1, Table indices base 0 */
        if (ct >= max) break;
        if (idx >= 0 && idx < _group_size(i))
        {
            talent_t *talent = &_talents[i][idx];
            power_info *power = &spells[ct++];
            power->spell.level = talent->spell.level;
            power->spell.cost = talent->spell.cost;
            /* We never go through get_spells_aux() or get_powers_aux(), so calculate fail rates right here */ 
            power->spell.fail = calculate_fail_rate(talent->spell.level, talent->spell.fail, p_ptr->stat_ind[talent->stat]);
            power->spell.fn = talent->spell.fn;
            power->stat = talent->stat;
        }
    }
    return ct;
}

/*
 * We now group wild talents. It's hard to pick from a large list of seemingly unsorted
 * choices. Also, there is wide variety in monitor sizes and resolutions, so attempting
 * to prompt for more than 15 or so choices at a time is a bad idea anyway.
 */
typedef struct {
    cptr name;
    cptr help;
    int min_slot;
    int max_slot;
    int color;
} group_choice;

group_choice _groups[] =  {
    { "狂野初始", "你早期的狂野天赋。也许这些并不那么令人敬畏，但它们能让你在游戏前期生存下来。", 0, 7, TERM_GREEN},
    { "狂野冥想", "你中期的力量，更加实用且具有破坏力。", 8, 16, TERM_L_UMBER},
    { "狂野毁灭", "你最强大的狂野天赋。死亡！破坏！毁灭！怪物们在你可怕的力量面前颤抖！", 17, _MAX_TALENTS - 1, TERM_RED},
};

static void _spell_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, _groups[which].name);
        break;
    case MENU_HELP:
        var_set_string(res, _groups[which].help);
        break;
    case MENU_COLOR:
        var_set_int(res, _groups[which].color);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

int wild_talent_get_spells(power_info *spells)
{
    int idx = -1;
    int ct = 0;
    menu_t menu = { "使用哪组天赋？", "浏览哪组天赋？", NULL,
                    _spell_menu_fn, _groups, 3, 0};
    int max = MAX_SPELLS;
    
    /* Mega-hack. Remove Fear is either in group 0 (in which case we get the right
     * group) or not in any group (in which case we avoid a pointless menu) */
    if (spell_problem & PWR_AFRAID) idx = 0;
    else idx = menu_choose(&menu);
    if (idx < 0) return 0;

    /* Hack: Add innate Wonder attack to Wild Beginnings */
    if (idx == 0)
    {
        ct += get_powers_aux(spells, max - ct, _wonder_powers, TRUE);
    }

    ct += _get_spells_imp(spells + ct, max - ct, _groups[idx].min_slot, _groups[idx].max_slot);
    if (ct == 0)
        msg_print("你还没有掌握那些天赋中的任何一个！");
    return ct;
}

void _gain_power(int level)
{
int group_idx = -1;

    if (level % 2 == 1)
        group_idx = level/2;

    if (group_idx >= 0 && group_idx < _MAX_TALENTS && _group_size(group_idx) > 0)
    {
        variant name;
        int idx = randint0(_group_size(group_idx));
        talent_t *talent = &_talents[group_idx][idx];

        if (!talent->spell.fn)
        {
            msg_print("BUG：你的天赋去哪了？！");
            return;
        }

        var_init(&name);
        (talent->spell.fn)(SPELL_NAME, &name);

        msg_format("<color:B>你获得了 <color:R>%s</color> 的力量 %s。</color>", var_get_string(&name), talent->gain_desc);
        p_ptr->magic_num1[group_idx] = idx + 1;

        var_clear(&name);
    }
}

void wild_talent_fix_up(void)
{
    int i;

    if (p_ptr->pclass != CLASS_WILD_TALENT) return;

    for (i = 1; i <= p_ptr->max_plv; ++i)
    {
    int group_idx = -1;

        if (i % 2 == 1)
            group_idx = i/2;

        if (group_idx >= 0 && group_idx < _MAX_TALENTS && _group_size(group_idx) > 0 && p_ptr->magic_num1[group_idx] == 0)
        {
            _gain_power(i);
        }
    }
}



bool _mut_avg_pred(int mut_idx)
{
    if (mut_type(mut_idx) == MUT_TYPE_ACTIVATION) return FALSE;
    if (mut_rating(mut_idx) == MUT_RATING_AVERAGE) return TRUE;
    return FALSE;
}

bool _mut_avg_plus_pred(int mut_idx)
{
    if (mut_type(mut_idx) == MUT_TYPE_ACTIVATION) return FALSE;
    if (mut_rating(mut_idx) >= MUT_RATING_AVERAGE) return TRUE;
    return FALSE;
}

bool _mut_good_pred(int mut_idx)
{
    if (mut_type(mut_idx) == MUT_TYPE_ACTIVATION) return FALSE;
    if (mut_rating(mut_idx) == MUT_RATING_GOOD) return TRUE;
    return FALSE;
}

bool _mut_good_plus_pred(int mut_idx)
{
    if (mut_type(mut_idx) == MUT_TYPE_ACTIVATION) return FALSE;
    if (mut_rating(mut_idx) >= MUT_RATING_GOOD) return TRUE;
    return FALSE;
}

bool _mut_great_pred(int mut_idx)
{
    if (mut_type(mut_idx) == MUT_TYPE_ACTIVATION) return FALSE;
    if (mut_rating(mut_idx) == MUT_RATING_GREAT) return TRUE;
    return FALSE;
}

void _gain_mutation(int level)
{
    switch (level)
    {
    case 10:
        mut_gain_random(_mut_good_pred);
        break;
    case 20:
        mut_gain_random(_mut_good_pred);
        break;
    case 30:
        mut_gain_random(_mut_good_plus_pred);
        break;
    case 40:
        mut_gain_random(_mut_great_pred);
        break;
    case 50:
        mut_gain_random(_mut_great_pred);
        break;
    }
}

static void _gain_level(int new_level)
{
    _gain_power(new_level);
    _gain_mutation(new_level);
}

void wild_talent_scramble(void)
{
    int i;

    if (p_ptr->pclass != CLASS_WILD_TALENT) return;

    /* Forget old talents */
    for (i = 0; i <= _MAX_TALENTS; ++i)
        p_ptr->magic_num1[i] = 0;

    msg_print("你感到狂野的随机力量进入了你的身体！");

    /* Regain new talents */
    for (i = 1; i <= p_ptr->max_plv; ++i)
        _gain_power(i);
}

void wild_talent_new_life(void)
{
    int i;

    if (p_ptr->pclass != CLASS_WILD_TALENT) return;

    /* re-grant powers */
    wild_talent_scramble();

    /* re-grant mutations */
    for (i = 1; i <= p_ptr->max_plv; ++i)
        _gain_mutation(i);
}

static void _calc_bonuses(void)
{
    samurai_posture_calc_bonuses();
    monk_posture_calc_bonuses();
}
static void _calc_stats(s16b stats[MAX_STATS])
{
    samurai_posture_calc_stats(stats);
    monk_posture_calc_stats(stats);
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    samurai_posture_get_flags(flgs);
    monk_posture_get_flags(flgs);
}

static void _character_dump(doc_ptr doc)
{
    int i;
    power_info spells[MAX_SPELLS];
    int ct = _get_spells_imp(spells, MAX_SPELLS, 0, _MAX_TALENTS - 1);

    for (i = 0; i < ct; i++)
    {
        spell_info* current = &spells[i].spell;
        current->cost += get_spell_cost_extra(current->fn);
        current->fail = MAX(current->fail, get_spell_fail_min(current->fn));
    }

    if (ct > 0)
    {
        int i;
        variant name, info;

        var_init(&name);
        var_init(&info);

        doc_printf(doc, "<topic:WildTalent>================================= <color:keypress>W</color> 狂野天赋 ================================\n\n");
        doc_printf(doc, "<color:G>%-23.23s 等级 属性 消耗 失败率 信息</color>\n", "");
        for (i = 0; i < ct; ++i)
        {
            spell_info *spell = &spells[i].spell;

            (spell->fn)(SPELL_NAME, &name);
            (spell->fn)(SPELL_INFO, &info);

            doc_printf(doc, "%-23.23s %2d %4.4s %4d %3d%% %s\n",
                            var_get_string(&name),
                            spell->level,
                            stat_abbrev_true[_which_stat(i)],
                            spell->cost,
                            spell->fail,
                            var_get_string(&info));
        }

        var_clear(&name);
        var_clear(&info);

        doc_newline(doc);
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "狂野法术";
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SMALL_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
}

class_t *wild_talent_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  25,  31,   2,  24,  16,  56,  50 };
    skills_t xs = {  8,  11,  10,   0,   0,   0,  18,  18 };

        me.name = "狂野天赋者";
        me.desc = "狂野天赋者在升级时会获得随机的天赋和能力。他们是优秀的战士，并且在使用魔法装置方面表现不错，但他们真正的强项在于其庞大的潜在随机能力库。只是你永远不知道这些能力会是什么！\n \n狂野天赋者没有统一的施法属性。相反，他们获得的每一项能力都需要其单独的属性来计算失败率；例如，“投掷巨石”需要力量，而“魔法飞弹”需要智力。每个法术都需要消耗法力来施放，但可用的法力值总数不受任何特定属性的影响，而是完全由经验值决定。";
        
        me.stats[A_STR] = -1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] =  1;
        
        me.base_skills = bs;
        me.extra_skills = xs;
        
        me.life = 100;
        me.base_hp = 4;
        me.exp = 110;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_stats = _calc_stats;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.gain_level = _gain_level;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
