#include "angband.h"

typedef struct
{
    int id;
    byte flags;
    char nimi[20];
    ang_spell fn;
} dpl_mut;

#define _DPL_ONE_TIME 0x01
#define _DPL_LOW_LV 0x02
#define _DPL_QUICK_PAYOFF 0x04
#define _DPL_GLOBAL_MUT 0x08
#define _DPL_NO_BONUS 0x10

#define _YEQREZH_PICKS 25
#define _INVALID_SPELL -1

static s16b _yq_pick[_YEQREZH_PICKS];

static void _beam_of_chaos_spell(int cmd, variant *res)
{
    int dice = 10 + (p_ptr->lev / 10);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混沌打击");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道纯粹的混沌射线。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_CHAOS, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _beam_of_light_spell(int cmd, variant *res)
{
    int dam = p_ptr->lev * 2 - 10;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光之射线");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(dam + p_ptr->to_d_spell - 10)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道明亮的光之射线。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_LITE, dir, spell_power(dam + p_ptr->to_d_spell - 10));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _beam_of_disintegration_spell(int cmd, variant *res)
{
    int dice = 10 + (p_ptr->lev / 8);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "分解射线");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道分解射线。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_DISINTEGRATE, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_sound_spell(int cmd, variant *res)
{
    int dam = spell_power(11*p_ptr->lev/2 + p_ptr->to_d_spell);
    int rad = p_ptr->lev > 40 ? -3 : -2;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "声音吐息");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_DESC:
        var_set_string(res, "向你选择的目标喷吐锥形声音。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SOUND, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cone_of_plasma_spell(int cmd, variant *res)
{
    int dam = spell_power(9*p_ptr->lev/2 + p_ptr->to_d_spell);
    int rad = p_ptr->lev > 40 ? -3 : -2;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "等离子锥");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_DESC:
        var_set_string(res, "向你选择的目标发射等离子锥。");
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

static void _confuse_monster_spell(int cmd, variant *res)
{
    int pow = spell_power(p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混乱怪物");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(pow));
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试使一个怪物陷入混乱。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        confuse_monster(dir, pow);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cure_critical_wounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗重伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "治疗割伤、震慑，并恢复一些生命值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(12, 12, 0));
        break;
    case SPELL_CAST:
        var_set_bool(res, TRUE);
        hp_player(spell_power(damroll(12, 12)));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _door_creation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "造门术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在周围所有格子制造门。");
        break;
    case SPELL_CAST:
        door_creation();
        p_ptr->update |= (PU_FLOW);
        p_ptr->redraw |= (PR_MAP);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _genocide_one_spell(int cmd, variant *res)
{
    int power = spell_power(p_ptr->lev * 10 / 3);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "湮灭");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(power));
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试使一个怪物消失。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_GENOCIDE, dir, power, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色治疗");
        break;
    case SPELL_DESC:
        var_set_string(res, "恢复 300 点生命值，治愈割伤、震慑和中毒。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(300)));
        break;
    case SPELL_CAST:
        var_set_bool(res, TRUE);
        hp_player(spell_power(300));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_poisoned(0, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 8);
    int sides = 10;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力箭");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell + p_ptr->lev)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一支法力箭。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_MANA, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell + p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shuffle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "洗牌");
        break;
    case SPELL_DESC:
        var_set_string(res, "抽取一张牌，引发随机效果。");
        break;
    case SPELL_CAST:
        cast_shuffle();
        if (!(p_ptr->knowledge & KNOW_HPRATE)) self_knowledge();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_hook_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色钩爪");
        break;
    case SPELL_DESC:
        var_set_string(res, "将一个视野内的怪物传送到你身边。");
        break;
    case SPELL_CAST:
    {
        monster_type *m_ptr;
        char m_name[80];

        if (!target_set(TARGET_KILL)) break;
        if (!cave[target_row][target_col].m_idx) break;
        if (!player_has_los_bold(target_row, target_col)) break;
        if (!projectable(py, px, target_row, target_col)) break;
        if (cave[target_row][target_col].m_idx == p_ptr->riding) break;

        var_set_bool(res, TRUE);

        m_ptr = &m_list[cave[target_row][target_col].m_idx];
        monster_desc(m_name, m_ptr, 0);
        if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
        msg_format("你命令%s回来。", m_name);
        teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rocket_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火箭");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 7 + p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一枚火箭。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你发射了一枚火箭！");
        fire_rocket(GF_ROCKET, dir, spell_power(p_ptr->lev * 7 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _stalking_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "暗影潜行");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时赋予强化的潜行能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(30, 30));
        break;
    case SPELL_CAST:
        set_tim_dark_stalker(30 + randint1(30), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_trees_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤树木");
        break;
    case SPELL_DESC:
        var_set_string(res, "用翠绿环绕你。");
        break;
    case SPELL_CAST:
        tree_creation();
        p_ptr->update |= (PU_FLOW);
        p_ptr->redraw |= (PR_MAP);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _ultimate_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "终极抗性");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予终极抗性，以及护甲和速度加成。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        {
            int v = randint1(25) + 25;
            set_fast(v, FALSE);
            set_oppose_base(v, FALSE);
            set_ultimate_res(v, FALSE);
            var_set_bool(res, TRUE);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _unbarring_ways_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "破除障碍");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道射线，摧毁陷阱和门。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        destroy_door(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _word_of_banishment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "放逐之语");
        break;
    case SPELL_INFO:
        var_set_string(res, "距离 125");
        break;
    case SPELL_DESC:
        var_set_string(res, "将视野内的所有怪物传送走，除非其抵抗。");
        break;
    case SPELL_CAST:
        banish_monsters(125);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wall_of_stone_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "石墙术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在周围所有空地上制造墙壁。");
        break;
    case SPELL_CAST:
        wall_stone();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wrecking_yelp_spell(int cmd, variant *res)
{
    int dice = 4 + (p_ptr->lev - 1) / 5;
    int sides = 4;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "破坏之嚎");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一束音波。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_SOUND, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _yeqrezh_spells[] =
{
    /* lvl cst fail spell */
    {  2,  1, 30, magic_missile_spell},
    {  2,  2, 30, detect_traps_spell},
    {  4,  3, 30, _wrecking_yelp_spell},
    {  4,  4, 30, phase_door_spell},
    {  6,  4, 30, light_area_spell},
    {  6,  5, 35, detect_monsters_spell},
    {  8,  5, 30, lightning_bolt_spell},
    {  8,  4, 30, _unbarring_ways_spell},
    {  8,  8, 40, heroism_spell},
    { 12,  8, 40, magic_mapping_spell},
    { 12,  6, 40, frost_bolt_spell},
    { 14,  6, 40, stone_to_mud_spell},
    { 14,  8, 40, fire_bolt_spell},
    { 16,  8, 40, identify_spell},
    { 16,  8, 40, teleport_spell},
    { 18, 12, 40, _confuse_monster_spell},
    { 18, 15, 45, _cure_critical_wounds_spell},
    { 22, 15, 50, nether_bolt_spell},
    { 22, 20, 50, telepathy_spell},
    { 24, 12, 50, detection_spell},
    { 24, 15, 50, _purple_hook_spell},
    { 26, 20, 50, haste_self_spell},
    { 26, 20, 55, teleport_other_spell},
    { 28, 20, 55, _beam_of_chaos_spell},
    { 28, 20, 55, _beam_of_light_spell},
    { 28, 16, 55, _stalking_spell},
    { 28, 20, 55, _mana_bolt_spell},
    { 32, 22, 55, plasma_bolt_spell},
    { 32, 25, 55, recall_spell},
    { 32, 28, 55, _door_creation_spell},
    { 34, 35, 55, _beam_of_disintegration_spell},
    { 34, 30, 55, stone_skin_spell},
    { 36, 30, 60, _summon_trees_spell},
    { 36, 35, 60, _word_of_banishment_spell},
    { 36, 40, 60, clairvoyance_spell},
    { 38, 20, 55, recharging_spell},
    { 38, 50, 66, destruction_spell},
    { 38, 40, 66, _cone_of_plasma_spell},
    { 42, 60, 72, dimension_door_spell},
    { 42, 50, 66, _genocide_one_spell},
    { 42, 60, 72, hide_in_mud_spell},
    { 44, 60, 72, _wall_of_stone_spell},
    { 44, 55, 72, _rocket_spell},
    { 44, 50, 66, _breathe_sound_spell},
    { 46, 60, 78, _healing_spell},
    { 46, 82, 78, mana_storm_II_spell},
    { 48, 125, 84, invulnerability_spell},
    { 48, 100, 90, genocide_spell},
    { 48, 150, 84, _ultimate_resistance_spell},
    { -1, -1, -1, NULL},
};

enum
{
    _YQ_STRDEX = 0,
    _YQ_HEAL,
    _YQ_SPEED,
    _YQ_MELEE,
    _YQ_STEALTH,
    _YQ_STEEL,
    _YQ_MD,
    _YQ_XS,
    _YQ_FELL,
    _YQ_CRAVEN,
    _YQ_EASY,
    _YQ_YELLOW,
    _YQ_REGEN,
    _YQ_WHEEL,
    _YQ_NOTES,
    _YQ_LIFE,
    _YQ_MEMO,
    _YQ_WEIRD,
    _YQ_CONTROL,
    _YQ_OBJECT,
    _YQ_SHUFFLE,

    MAX_YEQREZH_MUT
};

static int _talent_count(int which)
{
    int i, laskuri = 0;
    if ((which < 0) || (which >= MAX_YEQREZH_MUT) || (spoiler_hack) || (p_ptr->max_plv < 10)) return 0;
    for (i = 1; i <= p_ptr->max_plv / 10; i++)
    {
        if (_yq_pick[(i * 5) - 1] == which) laskuri++;
    }
    return laskuri;
}

static void _device_mastery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "装置精通");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日给你上了一堂装置使用课！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "赋予相当于 +1 装置精通(Md) 的装置技能。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.dev += 8;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _easy_casting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "轻松施法");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日传授了你<color:v>轻松施法</color>的技艺！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "降低 4 个百分点的法术失败率。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _essence_of_life_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生命精华");
        break;
    case SPELL_GAIN_MUT:
        {
            object_type forge;
            msg_print("耶克列日送了你一份礼物！");
            object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_LIFE));
            object_origins(&forge, ORIGIN_PATRON);
            forge.number = 3;
            obj_identify_fully(&forge);
            pack_carry(&forge);
            break;
        }
    case SPELL_HELP_DESC:
        var_set_string(res, "你将从耶克列日那里收到三瓶生命药水。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _notes_of_death_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "死亡笔记");
        break;
    case SPELL_GAIN_MUT:
        {
            object_type forge;
            msg_print("耶克列日送了你一份礼物！");
            object_prep(&forge, lookup_kind(TV_SCROLL, SV_SCROLL_MASS_GENOCIDE));
            object_origins(&forge, ORIGIN_PATRON);
            forge.number = 5;
            obj_identify_fully(&forge);
            pack_carry(&forge);
            break;
        }
    case SPELL_HELP_DESC:
        var_set_string(res, "你将从耶克列日那里收到五张群体大屠杀卷轴。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _nervous_casting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紧张施法");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的想象力开始暴走！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "使你的法术强度提高 +15%，但会让你变得容易恐惧。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->spell_power += 2;
        res_add_vuln(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _object_lessons_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "实物教学");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日给你上了一堂实物教学课！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "允许你感知远处物品的品质，并自动鉴定你踩到的所有物品。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->auto_id = TRUE;
        p_ptr->munchkin_pseudo_id = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _power_sorcery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "强力咒术");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你将灵魂倾注到魔法中！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "使你的法术强度提高 +15%，但会降低你 3 点生命乘数。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->spell_power += 2;
        p_ptr->life -= 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_melee_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色致命");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你以焕发的力量打击敌人！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +8 的近战伤害加成（不适用于天生攻击）。");
        break;
    case SPELL_CALC_BONUS:
    {
        int hand;
        for (hand = 0; hand < MAX_HANDS; hand++)
        {
            if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
            {
                p_ptr->weapon_info[hand].to_d += 8;
                p_ptr->weapon_info[hand].dis_to_d += 8;
            }
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_muscles_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色肌肉");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉自己健步如飞！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +2 力量（STR）和 +2 敏捷（DEX）。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->stat_add[A_STR] += 2;
        p_ptr->stat_add[A_DEX] += 2;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_prot_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色防护");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日用防护光环环绕着你！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +15 护甲（AC）。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 15;
        p_ptr->dis_to_a += 15;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_speed_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色速度");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的移动速度变快了！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +1 速度。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed += 1;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_stealth_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色潜行");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日传授了你无声移动的技艺！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +4 潜行。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.stl += 4;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_vitality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫色活力");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日赋予了你惊人的恢复能力！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, format("所有治疗效果恢复的生命值将比平时多 %d%%。", (_talent_count(_YQ_HEAL) + 1) * 10));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rapid_shooting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "急速射击");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日传授了你<color:v>急速射击</color>的技艺！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供相当于 +1 额外射击(Xs) 的射击速度加成。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->shooter_info.xtra_shot += 15;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _regen_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "快速恢复");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始快速再生。");
        mut_lose(MUT_FLESH_ROT);
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +100% 再生率。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen += 100;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _trump_shuffling_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "王牌洗牌");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日传授了你<color:o>王牌洗牌</color>的技艺。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "使你能够使用“洗牌”法术，并在抽到“审判”牌时自动获得自我感知效果。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wheel_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "彩票");
        break;
    case SPELL_GAIN_MUT:
        {
            object_type forge;
            msg_print("耶克列日送了你一份礼物！");
            object_prep(&forge, lookup_kind(TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT));
            object_origins(&forge, ORIGIN_PATRON);
            forge.number = 3;
            obj_identify_fully(&forge);
            pack_carry(&forge);
            break;
        }
    case SPELL_HELP_DESC:
        var_set_string(res, "你将从耶克列日那里收到三张*获取*卷轴。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _yellow_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "敏捷施法");
        break;
    case SPELL_GAIN_MUT:
        msg_print("耶克列日传授了你敏捷施法的技艺！");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, format("施放法术只需消耗通常 %d%% 的时间。", 93 - (7 * _talent_count(_YQ_YELLOW))));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _yeqrezh_memories_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "耶克列日的记忆");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的脑海中充满了耶克列日的记忆！");
        gain_exp(exp_requirement(p_ptr->max_plv + 4) - p_ptr->exp);
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "立即提升 5 个角色等级。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

int hp_player_yeqrezh(int num)
{
    int j = _talent_count(_YQ_HEAL);
    if (!j) return num;
    return (num + (num * j / 10));
}

static dpl_mut _yeqrezh_muts[MAX_YEQREZH_MUT] =
{
    { _YQ_STRDEX, 0, "紫色肌肉", _purple_muscles_mut },
    { _YQ_HEAL, _DPL_NO_BONUS, "紫色活力", _purple_vitality_mut },
    { _YQ_SPEED, 0, "紫色速度", _purple_speed_mut },
    { _YQ_MELEE, 0, "紫色致命", _purple_melee_mut },
    { _YQ_STEALTH, 0, "紫色潜行", _purple_stealth_mut },
    { _YQ_STEEL, 0, "紫色防护", _purple_prot_mut },
    { _YQ_MD, 0, "装置精通", _device_mastery_mut },
    { _YQ_XS, 0, "急速射击", _rapid_shooting_mut },
    { _YQ_FELL, 0, "强力咒术", _power_sorcery_mut },
    { _YQ_CRAVEN, 0, "紧张施法", _nervous_casting_mut },
    { _YQ_EASY, 0, "轻松施法", _easy_casting_mut },
    { _YQ_YELLOW, _DPL_NO_BONUS, "敏捷施法", _yellow_mut },
    { _YQ_REGEN, 0, "快速恢复", _regen_mut },
    { _YQ_WHEEL, _DPL_QUICK_PAYOFF, "彩票", _wheel_mut },
    { _YQ_NOTES, _DPL_QUICK_PAYOFF, "死亡笔记", _notes_of_death_mut },
    { _YQ_LIFE, _DPL_QUICK_PAYOFF, "生命精华", _essence_of_life_mut },
    { _YQ_MEMO, (_DPL_LOW_LV | _DPL_NO_BONUS), "耶克记忆", _yeqrezh_memories_mut },
    { MUT_WEIRD_MIND, (_DPL_ONE_TIME | _DPL_GLOBAL_MUT), "怪异思维", weird_mind_mut },
    { MUT_STRONG_MIND, (_DPL_ONE_TIME | _DPL_GLOBAL_MUT), "坚韧思维", strong_mind_mut },
    { _YQ_OBJECT, _DPL_ONE_TIME, "实物教学", _object_lessons_mut },
    { _YQ_SHUFFLE, (_DPL_ONE_TIME | _DPL_NO_BONUS), "王牌洗牌", _trump_shuffling_mut },
};

static bool _yeqrezh_gain_spell(int slot);

static int _my_calculate_fail_rate(int level, int base_fail, int stat_idx)
{
    int fail = calculate_fail_rate(level, base_fail, stat_idx);
    return MAX(0, fail - (4 * _talent_count(_YQ_EASY)));
}

static spell_info *_yeqrezh_get_spells_learned(void)
{
    int ct = 0, i;
    static spell_info spells[MAX_SPELLS];
    for (i = 0; i < (p_ptr->max_plv / 2); i++)
    {
        spell_info *src, *dest;
        if ((i % 5) == 4)
        {
            if (_yq_pick[i] != _YQ_SHUFFLE) continue;
            dest = &spells[ct++];
            dest->level = (i + 1) * 2;
            dest->cost = 5;
            dest->fail = _my_calculate_fail_rate(dest->level, 45, p_ptr->stat_ind[A_INT]);
            dest->fn = _shuffle_spell;
            continue;
        }

        if (_yq_pick[i] < 0)
        {
            if (!_yeqrezh_gain_spell(i)) continue;
        }

        src = &_yeqrezh_spells[_yq_pick[i]];

        if ((!src) || (src->level <= 0)) continue;

        dest = &spells[ct++];
        dest->level = (i + 1) * 2;
        dest->cost = src->cost;
        dest->fail = _my_calculate_fail_rate(dest->level, src->fail, p_ptr->stat_ind[A_INT]);
        dest->fn = src->fn;
    }
    spells[ct].fn = NULL;
    return spells;
}

static int _yeqrezh_get_spells_unlearned(power_info* spells, bool check_lv)
{
    int ct = 0, i, j;
    for (i = 0;; i++)
    {
        spell_info *src;
        power_info *dest;
        bool opittu = FALSE;

        src = &_yeqrezh_spells[i];

        if (src->level < 0) break;
        if ((check_lv) && (src->level > p_ptr->max_plv)) break;
        for (j = 0; j < _YEQREZH_PICKS; j++)
        {
            if ((j % 5) == 4) continue; /* muts, not spells */
            if (_yq_pick[j] == i)
            {
                opittu = TRUE;
                continue;
            }
        }
        if (opittu) continue;

        dest = &spells[ct++];
        dest->spell.level = src->level;
        dest->spell.cost = src->cost;
        dest->spell.fail = _my_calculate_fail_rate((check_lv ? p_ptr->max_plv : src->level), src->fail, p_ptr->stat_ind[A_INT]);
        dest->spell.fn = src->fn;
        dest->stat = A_NONE;
    }
    return ct;
}

static bool _yeqrezh_gain_spell(int slot)
{
    power_info spells[MAX_SPELLS];
    bool valitse = ((slot % 8) == 7) ? TRUE : FALSE;
    int i, ct, uusi = _INVALID_SPELL, tosipaikka = _INVALID_SPELL;
    int taso = (slot + 1) * 2;

    /* Massive paranoia follows */
    if ((slot % 5) == 4) return FALSE;
    if ((slot < 0) || (slot >= _YEQREZH_PICKS)) return FALSE;
    if (!character_dungeon) return FALSE;
    if (_yq_pick[slot] != _INVALID_SPELL) return FALSE;
    if (p_ptr->max_plv < taso) return FALSE;
    ct = _yeqrezh_get_spells_unlearned(spells, valitse);
    if (!ct) return FALSE;

    if (!valitse)
    {
        while (uusi < 0)
        {
            int koitto = 0;
            for (i = 0; i < ct; i++)
            {
                spell_info *spell = &spells[i].spell;
                int mahis = 1;
                if ((!spell) || (!spell->level) || (spell->level < 0))
                {
                    if (uusi == _INVALID_SPELL) return FALSE;
                    break; /* paranoia */
                }
                if (spell->cost > taso * 4) continue;
                if (spell->level != taso)
                {
                    mahis = ABS(taso - spell->level);
                    if (mahis > 5) mahis *= 2;
                    if (mahis > 20) mahis *= 2;
                    if (mahis > 80) mahis = (mahis * 3 / 2);
                }
                if (!one_in_(mahis)) continue;
                koitto++;
                if (one_in_(koitto)) uusi = i;
            }
        }
    }
    else
    {
        while ((uusi < 0) && (ct > 0))
        {
            uusi = choose_spell(spells, ct, "学习", "法术", 1000, FALSE);
        }
    }
    if ((uusi < 0) || (uusi >= ct)) return FALSE;

    for (i = 0; _yeqrezh_spells[i].level > 0; i++)
    {
        if (spells[uusi].spell.fn == _yeqrezh_spells[i].fn)
        {
            tosipaikka = i;
            break;
        }
    }
    if (tosipaikka == _INVALID_SPELL) return FALSE;
    msg_format("耶克列日传授了你<color:v>%s</color>法术。", get_spell_name(_yeqrezh_spells[tosipaikka].fn));
    _yq_pick[slot] = tosipaikka;
    return TRUE;
}

static void _talent_help_desc(int i, char* buf)
{
    if (_yeqrezh_muts[i].flags & _DPL_GLOBAL_MUT)
    {
        mut_help_desc(_yeqrezh_muts[i].id, buf);
        return;
    }
    else
    {
        variant v;
        var_init(&v);

        if (i >= 0 && i < MAX_YEQREZH_MUT)
        {
            (_yeqrezh_muts[i].fn)(SPELL_HELP_DESC, &v);
            if (strlen(var_get_string(&v)) == 0)
                (_yeqrezh_muts[i].fn)(SPELL_DESC, &v);
        }
        else
            var_set_string(&v, "");

        sprintf(buf, "%s", var_get_string(&v));
        var_clear(&v);
    }
}


static void _yeqrezh_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    static char _yeqrezh_menu_options[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int idx = ((int*)cookie)[which];
    switch (cmd)
    {
    case MENU_KEY:
    {
        var_set_int(res, _yeqrezh_menu_options[which]);
        break;
    }
    case MENU_TEXT:
    {
        var_set_string(res, format("%s", _yeqrezh_muts[idx].nimi));
        break;
    }
    case MENU_COLOR:
    {
        if (_yeqrezh_muts[idx].flags & _DPL_QUICK_PAYOFF)
            var_set_int(res, TERM_L_BLUE);
        else if (_yeqrezh_muts[idx].flags & _DPL_GLOBAL_MUT)
            var_set_int(res, TERM_WHITE);
        else if (_yeqrezh_muts[idx].flags & _DPL_ONE_TIME)
            var_set_int(res, TERM_ORANGE);
        else var_set_int(res, TERM_VIOLET);
        break;
    }
    case MENU_HELP:
    {
        char buf[255];
        _talent_help_desc(idx, buf);
        var_set_string(res, buf);
        break;
    }
    case MENU_COLUMN:
    {
        var_set_int(res, (which >= Term->hgt - 6) ? 45 : 13);
        break;
    }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static int _yeqrezh_gain_talent(int slot)
{
    int choices[MAX_YEQREZH_MUT];
    int i, ct = 0;
    static bool _lock = FALSE;
    menu_t menu = { "获得哪一份礼物？", "查看哪一份礼物？", NULL,
                    _yeqrezh_menu_fn, choices, 0, Term->hgt - 6};

    if (_lock) return -1;
    if ((slot % 5) != 4) return -1;
    if ((slot >= _YEQREZH_PICKS) || (slot < 0)) return -1;
    if (!character_dungeon) return -1;
    if (_yq_pick[slot] != _INVALID_SPELL) return -1;
    if (p_ptr->max_plv < slot * 2 + 2) return -1;

    for (i = 0; i < MAX_YEQREZH_MUT; i++)
    {
        if ((prace_is_(RACE_ANDROID)) && (i == _YQ_MEMO)) continue; /* Androids can't get Yeqrezh's Memories */
        if ((_yeqrezh_muts[i].flags & _DPL_ONE_TIME) && (_talent_count(i))) continue;
        if ((_yeqrezh_muts[i].flags & _DPL_LOW_LV) && (slot > 20)) continue;
        if ((_yeqrezh_muts[i].flags & _DPL_GLOBAL_MUT) && (mut_present(_yeqrezh_muts[i].id))) continue;
        choices[ct++] = i;
    }

    if (ct == 0) return -1;

    menu.count = ct;

    _lock = TRUE;

    for (;;)
    {
        i = menu_choose(&menu);
        if (i >= 0)
        {
            char buf[1024];
            int idx = choices[i];
            sprintf(buf, "你将获得 %s。你确定吗？", _yeqrezh_muts[idx].nimi);
            if (get_check(buf))
            {
                /* We need to do this first of all because mut_gain() can trigger a
                 * call to _calc_bonuses() which can call _yeqrezh_gain_talent()...
                 * but with  _yq_pick[slot] already set, the extra instances of
                 * _yeqrezh_gain_talent() will return -1 instantly */
                _yq_pick[slot] = idx;

                if (_yeqrezh_muts[idx].flags & _DPL_GLOBAL_MUT)
                {
                    (void)mut_gain(_yeqrezh_muts[idx].id);
                    mut_lock(_yeqrezh_muts[idx].id);
                }
                else
                {
                    variant v;
                    var_init(&v);
                    (_yeqrezh_muts[idx].fn)(SPELL_GAIN_MUT, &v);
                    var_clear(&v);
                }

                _lock = FALSE;

                return idx;
            }
        }
        msg_print("请做出选择！");
    }

    _lock = FALSE;

    return -1;
}

static void _yeqrezh_gain_item(int new_level)
{
    int taso = new_level * 9 / 5;
    static arena_type _yeqrezh_items[] = {
     { 3, TV_STAFF, EFFECT_IDENTIFY},
     { 5, TV_POTION, SV_POTION_SPEED},
     { 7, TV_WAND, EFFECT_BEAM_LITE_WEAK},
     { 9, TV_WAND, EFFECT_STONE_TO_MUD},
     { 11, TV_WAND, EFFECT_BOLT_FIRE},
     { 13, TV_ROD, EFFECT_LITE_AREA},
     { 15, TV_ROD, EFFECT_CLARITY},
     { 17, TV_STAFF, EFFECT_CURING},
     { 19, TV_ROD, EFFECT_RECALL},
     { 21, TV_WAND, EFFECT_TELEPORT_AWAY},
     { 25, TV_STAFF, EFFECT_CONFUSE_MONSTERS},
     { 25, TV_WAND, EFFECT_BOLT_ICE},
     { 27, TV_ROD, EFFECT_BOLT_MANA},
     { 29, TV_ROD, EFFECT_BALL_ACID},
     { 31, TV_SCROLL, SV_SCROLL_ACQUIREMENT},
     { 33, TV_ROD, EFFECT_GREAT_CLARITY},
     { 35, TV_STAFF, EFFECT_SPEED},
     { 35, TV_ROD, EFFECT_BEAM_DISINTEGRATE},
     { 37, TV_ROD, EFFECT_ENLIGHTENMENT},
     { 39, TV_STAFF, EFFECT_TELEPATHY},
     { 39, TV_POTION, SV_POTION_POLYMORPH},
     { 41, TV_STAFF, EFFECT_CONFUSING_LITE},
     { 43, TV_STAFF, EFFECT_IDENTIFY_FULL},
     { 43, TV_SCROLL, SV_SCROLL_CRAFTING},
     { 45, TV_STAFF, EFFECT_STARBURST},
     { 45, TV_STAFF, EFFECT_BANISH_ALL},
     { 47, TV_ROD, EFFECT_HEAL_CURING_HERO},
     { 47, TV_POTION, SV_POTION_STAR_HEALING},
     { 49, TV_STAFF, EFFECT_RESTORE_MANA},
     { 49, TV_SCROLL, SV_SCROLL_ARTIFACT},
     { -1, -1, -1},
    };
    msg_print("耶克列日的声音隆隆响起：<color:v>干得漂亮！希望这份礼物能帮到你。</color>");
    if (one_in_(2))
    {
        int vtaso = object_level;
        object_level = taso;
        do
        {
            acquirement(py, px, 1, randint1(75) < taso, FALSE, ORIGIN_PATRON);
        } while (randint1(200) < taso);
        object_level = vtaso;
        return;
    }
    else if (new_level >= 3)
    {
        int uusi = -1, tval, sval;
        object_type *q_ptr, forge;
        while (uusi < 0)
        {
            int i, koitto = 0;
            for (i = 0;; i++)
            {
                arena_type *item = &_yeqrezh_items[i];
                int mahis = 1;
                if (item->r_idx > new_level) break; /* Hack - use r_idx to indicate item level */
                if (item->r_idx == -1) break;
                if (item->r_idx < new_level)
                {
                    mahis = new_level - item->r_idx;
                }
                mahis -= (mahis / 3);
                if (!one_in_(mahis)) continue;
                koitto++;
                if (one_in_(koitto)) uusi = i;
            }
        }
        tval = _yeqrezh_items[uusi].tval;
        sval = _yeqrezh_items[uusi].sval;

        q_ptr = &forge;
        switch (tval)
        {
            case TV_WAND: case TV_ROD: case TV_STAFF:
            {
                device_effect_info_ptr e_ptr = device_get_effect_info(tval, sval);
                if (!e_ptr) return; /* paranoia */
                object_prep(q_ptr, lookup_kind(tval, SV_ANY));
                if (taso > e_ptr->level) q_ptr->level = e_ptr->level - taso;
                device_init_fixed(q_ptr, sval);
                break;
            }
            default:
                object_prep(q_ptr, lookup_kind(tval, sval));
                apply_magic(q_ptr, object_level, AM_NO_FIXED_ART);
                obj_make_pile(q_ptr);
        }
        object_origins(q_ptr, ORIGIN_PATRON);
        obj_identify_fully(q_ptr);
        (void)drop_near(q_ptr, -1, py, px);
    }
}

static void _calc_bonuses(void)
{
    if (p_ptr->max_plv < 10) return;
    else
    {
        int i;
        variant v;
        var_init(&v);

        for (i = 0; i < (p_ptr->max_plv / 10); i++)
        {
            int kohde = _yq_pick[i * 5 + 4];
            if (kohde == _INVALID_SPELL)
            {
                kohde = _yeqrezh_gain_talent(i * 5 + 4);
            }
            if ((kohde < 0) || (kohde >= MAX_YEQREZH_MUT)) continue;
            if (_yeqrezh_muts[kohde].flags & (_DPL_NO_BONUS | _DPL_QUICK_PAYOFF | _DPL_GLOBAL_MUT)) continue;
            (_yeqrezh_muts[kohde].fn)(SPELL_CALC_BONUS, &v);
        }
        var_clear(&v);
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (_talent_count(_YQ_CRAVEN))
    {
        add_flag(flgs, OF_VULN_FEAR);
        add_flag(flgs, OF_SPELL_POWER);
    }
    if (_talent_count(_YQ_FELL))
    {
        add_flag(flgs, OF_DEC_LIFE);
        add_flag(flgs, OF_SPELL_POWER);
    }
    if (_talent_count(_YQ_STEALTH)) add_flag(flgs, OF_STEALTH);
    if (_talent_count(_YQ_SPEED)) add_flag(flgs, OF_SPEED);
    if (_talent_count(_YQ_REGEN)) add_flag(flgs, OF_REGEN);
}

static void _gain_level(int new_level)
{
    if (new_level < 2) return;
    if ((new_level % 2) == 0)
    {
        if (new_level % 10) _yeqrezh_gain_spell(new_level / 2 - 1);
        else _yeqrezh_gain_talent(new_level / 2 - 1);
    }
    else _yeqrezh_gain_item(new_level);
}

static void _on_cast(const spell_info *spell)
{
    int nopeus = _talent_count(_YQ_YELLOW);
    if (nopeus)
    {
        energy_use *= 100 - (7 * nopeus);
        energy_use /= 100;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 0;
        me.min_level = 2;
        me.on_cast = _on_cast;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

void yeqrezh_help(FILE *fp)
{
    int i;
    variant v;
    var_init(&v);
    fputs("<topic:Powers><style:heading>表 4 - 耶克瑞什天赋</style>\n\n", fp);
    fputs("Disciples of Yeqrezh can choose five special gifts, one every ten levels. "
               "These gifts can take the form of new abilities, physical powers, or rare items; "
               "some can be chosen repeatedly, while others can only be picked once. "
               "Once you have selected a gift, you can never go back on your decision, so you "
               "might want to study this list to decide which gifts to choose for your character.\n\n", fp);

    for (i = 0; i < MAX_YEQREZH_MUT; i++)
    {
         (_yeqrezh_muts[i].fn)(SPELL_HELP_DESC, &v);
         if (strlen(var_get_string(&v)) == 0)
         (_yeqrezh_muts[i].fn)(SPELL_DESC, &v);
         fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n", _yeqrezh_muts[i].nimi, var_get_string(&v));
    }
    var_clear(&v);
}

static void _yeqrezh_ini_picks(void)
{
    int i;
    for (i = 0; i < _YEQREZH_PICKS; i++) _yq_pick[i] = _INVALID_SPELL;
}

static void _birth(void)
{
    disciple_birth();
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_HARD_STUDDED_LEATHER, 1);
    _yeqrezh_ini_picks();
}

static void _yeqrezh_load(savefile_ptr file)
{
    int i;
    for (i = 0; i < _YEQREZH_PICKS; i++)
    {
        _yq_pick[i] = savefile_read_s16b(file);
    }
}

static void _yeqrezh_save(savefile_ptr file)
{
    int i;
    for (i = 0; i < _YEQREZH_PICKS; i++)
    {
        savefile_write_s16b(file, _yq_pick[i]);
    }
}

static void _yeqrezh_character_dump(doc_ptr doc)
{
    int        i, loydetty = 0;
    py_dump_spells(doc);

    for (i = 0; i < p_ptr->max_plv / 10; i++)
    {
        if ((_yq_pick[i * 5 + 4] < 0) || (_yq_pick[i * 5 + 4] >= MAX_YEQREZH_MUT)) continue;
        loydetty++;
        if (loydetty == 1) doc_printf(doc, "\n");
        doc_printf(doc, "第 %d 级礼物：%s\n", (i + 1) * 10, _yeqrezh_muts[_yq_pick[i * 5 + 4]].nimi);
    }
    if (loydetty) doc_printf(doc, "\n");
}

class_t *yeqrezh_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 23,  25,  34,   3,  16,  20,  52,  52};
    skills_t xs = {  9,  10,  10,   0,   0,   0,  18,  18};

        me.name = "耶克列日";
        me.subdesc = "Yeqrezh is the friendliest of the Purples, supportive of his Disciples "
                    "and willing to let them find their own way. Disciples of Yeqrezh will "
                    "learn 20 spells over the course of the game - one every two character "
                    "levels, apart from levels 10, 20, 30, 40 and 50; seventeen of these "
                    "spells are offered by Yeqrezh, while the other three (at levels 16, "
                    "32 and 48) are chosen by you. Every ten levels, Yeqrezh offers direct "
                    "help to his Disciples; depending on your choice, he can give you rare "
                    "items or a permanent mental or physical ability, or share some of his "
                    "life experience with you. Some of these abilities can be improved "
                    "repeatedly, allowing you to accumulate exceptional mastery.\n\n"
                    "Disciples of Yeqrezh gain many practical tools as well as solid offensive "
                    "options, and a useful item as a gift from their patron every odd-numbered "
                    "character level. They are the most mage-like of all Disciples, and lack of "
                    "hit points is their only true weakness. Nevertheless, they are extremely "
                    "versatile, and can also win through melee, archery or even sneaking about "
                    "silently collecting loot.\n\n"
                    "Yeqrezh is the recommended patron for new (or highly cautious) players, as he "
                    "is the only Purple not to actively incentivize a demanding, push-the-envelope "
                    "approach. Yeqrezh's disciples, like Karrot's, rely on Intelligence to "
                    "power their spells.";

        me.stats[A_STR] =  0;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] = -3;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 90;
        me.base_hp = 12;
        me.exp = 115;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _yeqrezh_get_spells_learned;
        me.gain_level = _gain_level;
        me.load_player = _yeqrezh_load;
        me.save_player = _yeqrezh_save;
        me.character_dump = _yeqrezh_character_dump;
        init = TRUE;
    }

    return &me;
}

