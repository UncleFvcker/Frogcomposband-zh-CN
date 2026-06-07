/****************************************************************
 * The Weaponmaster
 ****************************************************************/

#include "angband.h"

int       shoot_hack = SHOOT_NONE;
int       shoot_count = 0;
obj_loc_t shoot_item = {0};

#define _MAX_TARGETS 100

/****************************************************************
 * Private Helpers
 ****************************************************************/

static bool _check_direct_shot(int tx, int ty)
{
    bool result = FALSE;
    u16b path[512];
    int ct = project_path(path, 50, py, px, ty, tx, PROJECT_PATH); /* We don't know the length ... just project from source to target, please! */
    int x, y, i;

    for (i = 0; i < ct; i++)
    {
        x = GRID_X(path[i]);
        y = GRID_Y(path[i]);

        /* Reached target! Yay! */
        if (x == tx && y == ty)
        {
            result = TRUE;
            break;
        }

        /* Stopped by walls/doors */
        if (!cave_have_flag_bold(y, x, FF_PROJECT) && !cave[y][x].m_idx) break;

        /* Monster in the way of target */
        if (cave[y][x].m_idx) break;
    }

    return result;
}

static bool _check_speciality_equip(void);
static bool _check_speciality_aux(object_type *o_ptr);
static bool _can_judge(obj_ptr obj);

static int _get_nearest_target_los(void)
{
    int result = 0;
    int dis = AAF_LIMIT + 1;
    int i;
    monster_type *m_ptr = NULL;
    int rng = 0;

    if (p_ptr->shooter_info.slot)
        rng = bow_range(equip_obj(p_ptr->shooter_info.slot));

    for (i = m_max - 1; i >= 1; i--)
    {
        m_ptr = &m_list[i];
        if (!m_ptr->r_idx
        ) continue;
        if (m_ptr->smart & (1U << SM_FRIENDLY)) continue;
        if (m_ptr->smart & (1U << SM_PET)) continue;
        if (m_ptr->cdis > rng) continue;
        if (!m_ptr->ml) continue;
        if (!los(py, px, m_ptr->fy, m_ptr->fx)) continue;

        if (m_ptr->cdis < dis)
        {
            result = i;
            dis = m_ptr->cdis;
        }
    }

    return result;
}

static int _get_greater_many_shot_targets(int *targets, int max)
{
    int result = 0;
    int i;
    monster_type *m_ptr = NULL;
    int rng = 0;

    if (p_ptr->shooter_info.slot)
        rng = bow_range(equip_obj(p_ptr->shooter_info.slot));

    /* shoot *all* line of sight monsters */
    for (i = m_max - 1; i >= 1; i--)
    {
        m_ptr = &m_list[i];
        if (!m_ptr->r_idx) continue;
        if (m_ptr->smart & (1U << SM_FRIENDLY)) continue;
        if (m_ptr->smart & (1U << SM_PET)) continue;
        if (m_ptr->cdis > rng) continue;
        if (!m_ptr->ml) continue;
        if (!los(py, px, m_ptr->fy, m_ptr->fx)) continue;
        if (result >= max) break;

        targets[result] = i;
        result++;
    }

    return result;
}

static int _get_many_shot_targets(int *targets, int max)
{
    int result = 0;
    int i;
    monster_type *m_ptr = NULL;
    int in_sight[_MAX_TARGETS];
    int ct = 0;
    int rng = 0;

    if (p_ptr->shooter_info.slot)
        rng = bow_range(equip_obj(p_ptr->shooter_info.slot));

    /* pass 1: get line of sight monsters */
    for (i = m_max - 1; i >= 1; i--)
    {
        m_ptr = &m_list[i];
        if (!m_ptr->r_idx) continue;
        if (m_ptr->smart & (1U << SM_FRIENDLY)) continue;
        if (m_ptr->smart & (1U << SM_PET)) continue;
        if (m_ptr->cdis > rng) continue;
        if (!m_ptr->ml) continue;
        if (!los(py, px, m_ptr->fy, m_ptr->fx)) continue;
        if (ct >= _MAX_TARGETS) break;

        in_sight[ct] = i;
        ct++;
    }

    /* pass 2: for each monster in los, build a path from the player to the
       monster and make sure there are no other intervening monsters */
    for (i = 0; i < ct; i++)
    {
        m_ptr = &m_list[in_sight[i]];
        if (_check_direct_shot(m_ptr->fx, m_ptr->fy))
        {
            if (result > max) break;
            targets[result] = in_sight[i];
            result++;
        }
    }

    return result;
}

static bool _fire(int power)
{
    bool result = FALSE;
    shoot_hack = power;
    shoot_count = 0;
    command_cmd = 'f'; /* Hack for inscriptions (e.g. '@f1') */
    result = do_cmd_fire();
    shoot_hack = SHOOT_NONE;
    shoot_count = 0;

    return result;
}

static obj_ptr _get_ammo(bool allow_floor)
{
    obj_prompt_t prompt = {0};

    if (allow_floor)
        prompt.prompt = "发射哪种弹药？";
    else
        prompt.prompt = "选择用于此技巧的弹药。";
    prompt.error = "你没有可发射的东西。";
    prompt.filter = obj_can_shoot;
    prompt.where[0] = INV_QUIVER;
    prompt.where[1] = INV_PACK;
    if (allow_floor)
        prompt.where[2] = INV_FLOOR;

    obj_prompt(&prompt);
    return prompt.obj;
}

/* Weaponmasters have toggle based abilities */
static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;
    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();

    return result;
}

static bool _do_blow(int type)
{
    if (!_check_speciality_equip())
    {
        msg_print("失败！你觉得你的武器用着不顺手。");
        return FALSE;
    }
    return do_blow(type);
}

/****************************************************************
 * Private Spells
 ****************************************************************/
static void _fire_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的远程武器用着不顺手。");
            return;
        }
        if (_fire(which))
            var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        var_set_int(res, energy_use);    /* already set correctly by do_cmd_fire() */
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != which)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _judge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "评判");
        break;
    case SPELL_DESC:
        var_set_string(res, "鉴定一件偏好的物品。");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 45)
            var_set_bool(res, identify_fully(_can_judge));
        else
            var_set_bool(res, ident_spell(_can_judge));
        break;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Axemaster
 ****************************************************************/
static void _crusaders_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "十字军打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "用单次打击攻击一名相邻的对手。你将回复生命值。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_CRUSADERS_STRIKE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _power_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "强力攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "你牺牲了命中率，但增加了伤害。");
        break;
    default:
        _toggle_spell(TOGGLE_POWER_ATTACK, cmd, res);
        break;
    }
}

static void _vicious_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "凶恶打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "极其强力地攻击一名对手，但这种发力会让你露出很大破绽。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_do_blow(WEAPONMASTER_VICIOUS_STRIKE))
        {
            set_tim_vicious_strike(p_ptr->tim_vicious_strike + 10, FALSE);
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Bowmaster
 ****************************************************************/
static void _arrow_of_slaying_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "杀戮之箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试击中致命部位以瞬间击杀怪物，如果失败则只造成 1 点伤害。");
        break;
    default:
        _fire_spell(SHOOT_NEEDLE, cmd, res);
    }
}

static void _disintegration_arrow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "解体之箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的对手射出一支箭。即使是中间的墙壁也无法阻挡这一击！");
        break;
    default:
        _fire_spell(SHOOT_DISINTEGRATE, cmd, res);
    }
}

static void _piercing_arrow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "穿透之箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "如果箭矢击中对手，它会穿透并能继续击中同一方向上的下一个对手（需要另一次攻击检定），最多可击中 5 名对手。每次连续的穿透都会受到累加的命中惩罚。");
        break;
    default:
        _toggle_spell(TOGGLE_PIERCING_ARROW, cmd, res);
        break;
    }
}

static void _readied_shot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "蓄势待发");
        break;
    case SPELL_DESC:
        var_set_string(res, "对伤害你的敌人予以回击射出一支箭（在施放时选择）。在触发前一直处于激活状态。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的远程武器用着不顺手。");
            return;
        }
        if (_get_toggle() == TOGGLE_READIED_SHOT)
            _set_toggle(TOGGLE_NONE);
        else
        {
            obj_ptr ammo = _get_ammo(FALSE);
            if (!ammo)
            {
                flush();
                return;
            }
            shoot_item = ammo->loc;
            _set_toggle(TOGGLE_READIED_SHOT);
        }
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != TOGGLE_READIED_SHOT)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _tranquilizing_arrow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镇静之箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试用瞄准良好的一箭让怪物陷入沉睡。");
        break;
    default:
        _fire_spell(SHOOT_TRANQUILIZE, cmd, res);
    }
}

static void _volley_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抛射");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人发射一支箭。这支箭将从任何阻挡在中间的怪物头顶越过。");
        break;
    default:
        _fire_spell(SHOOT_VOLLEY, cmd, res);
    }
}

/****************************************************************
 * Clubmaster
 ****************************************************************/
static void _combat_expertise_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "防御架势");
        break;
    case SPELL_DESC:
        var_set_string(res, "你牺牲了命中率，但增加了护甲等级(AC)。");
        break;
    default:
        _toggle_spell(TOGGLE_COMBAT_EXPERTISE, cmd, res);
        break;
    }
}

static void _cunning_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狡诈打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "攻击次数减半，但打击更有可能让你的对手困惑、震慑或昏迷。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_CUNNING_STRIKE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smite_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "破邪斩");
        break;
    case SPELL_DESC:
        var_set_string(res, "对邪恶怪物进行强力攻击。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_SMITE_EVIL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _smash_ground_dam(void)
{
    int result = 0;
    int hand;
    /* Modified from Samurai "Crack" */
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
        {
            object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
            int          damage, dd, ds;

            if (!o_ptr) continue;

            dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
            ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;

            damage = dd * (ds + 1) * 50;
            damage += o_ptr->to_d * 100;
            damage *= NUM_BLOWS(hand)/100;
            result += damage / 100;
        }
    }
    return result;
}
static void _smash_ground_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "粉碎大地");
        break;
    case SPELL_DESC:
        var_set_string(res, "制造出巨大的、令人震慑的声响。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _smash_ground_dam()));
        break;
    case SPELL_CAST:
    {
        int dam = _smash_ground_dam();
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        msg_print("你将武器用力砸向地面。");
        project(0, 8, py, px, device_power(dam*2), GF_SOUND, PROJECT_KILL | PROJECT_ITEM);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _toss_hit_mon(py_throw_ptr context, int m_idx)
{
    monster_type *m_ptr = &m_list[m_idx];
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    char          m_name[80];
    int           odds = 2;

    monster_desc(m_name, m_ptr, 0);
    if (one_in_(odds))
    {
        if (r_ptr->flags3 & RF3_NO_CONF)
        {
            mon_lore_3(m_ptr, RF3_NO_CONF);
            msg_format("%^s不受影响。", m_name);
        }
        else if (mon_save_p(m_ptr->r_idx, A_STR))
        {
            msg_format("%^s不受影响。", m_name);
        }
        else
        {
            msg_format("%^s看起来<color:U>困惑</color>了。", m_name);
            set_monster_confused(m_idx, MON_CONFUSED(m_ptr) + 10 + randint0(p_ptr->lev) / 5);
        }
    }

    if (p_ptr->lev >= 20 && one_in_(odds))
    {
        if (r_ptr->flags3 & RF3_NO_SLEEP)
        {
            mon_lore_3(m_ptr, RF3_NO_SLEEP);
            msg_format("%^s不受影响。", m_name);
        }
        else if (mon_save_p(m_ptr->r_idx, A_STR))
        {
            msg_format("%^s不受影响。", m_name);
        }
        else
        {
            msg_format("%^s<color:b>昏迷</color>了。", m_name);
            set_monster_csleep(m_idx, MON_CSLEEP(m_ptr) + 500);
        }
    }

    if (p_ptr->lev >= 45 && one_in_(odds))
    {
        if (r_ptr->flags3 & RF3_NO_STUN)
        {
            mon_lore_3(m_ptr, RF3_NO_STUN);
            msg_format("%^s不受影响。", m_name);
        }
        else if (mon_stun_save(r_ptr->level, context->dam))
        {
            msg_format("%^s不受影响。", m_name);
        }
        else
        {
            msg_format("%^s被<color:B>震慑</color>了。", m_name);
            mon_stun(m_ptr, mon_stun_amount(context->dam));
        }
    }
}
static void _init_throw_context(py_throw_ptr context)
{
    context->type = THROW_BOOMERANG;
    context->mult = 100 + 4 * p_ptr->lev;
    context->back_chance = 24 + randint1(5);
    context->after_hit_f = _toss_hit_mon;
}
static void _throw_weapon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "投掷武器");
        break;
    case SPELL_DESC:
        var_set_string(res, "投掷你的武器，它可能会回到你手中。");
        break;
    case SPELL_CAST:
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        else
        {
            py_throw_t context = {0};
            _init_throw_context(&context);
            var_set_bool(res, py_throw(&context));
        }
        break;
    case SPELL_ON_BROWSE:
    {
        bool       screen_hack = screen_is_saved();
        py_throw_t context = {0};
        doc_ptr    doc = doc_alloc(80);

        _init_throw_context(&context);
        context.type |= THROW_DISPLAY;
        py_throw_doc(&context, doc);

        if (screen_hack) screen_load();
        screen_save();
        doc_display(doc, "投掷武器", 0);
        screen_load();
        if (screen_hack) screen_save();

        doc_free(doc);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _trade_blows_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "以伤换伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，你会暴露出一些破绽。然而，每当怪物击中你时，你都会进行反击。");
        break;
    default:
        _toggle_spell(TOGGLE_TRADE_BLOWS, cmd, res);
        break;
    }
}

/****************************************************************
 * Crossbowmaster
 ****************************************************************/
static void _careful_aim_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "仔细瞄准");
        break;
    case SPELL_DESC:
        var_set_string(res, "你射击速度变慢，但准确度大幅提高。");
        break;
    default:
        _toggle_spell(TOGGLE_CAREFUL_AIM, cmd, res);
        break;
    }
}

static void _elemental_bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "元素弩箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一支弩箭，如果击中目标，会在一阵元素之雨中爆炸。");
        break;
    default:
        _fire_spell(SHOOT_ELEMENTAL, cmd, res);
    }
}

static void _exploding_bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆炸弩箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的弩箭在撞击时会爆炸，但你的射击速度会降低。");
        break;
    default:
        _toggle_spell(TOGGLE_EXPLODING_BOLT, cmd, res);
        break;
    }
}

static void _knockback_bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "击退击");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一支弩箭，如果击中目标，会将你的敌人向后推。");
        break;
    default:
        _fire_spell(SHOOT_KNOCKBACK, cmd, res);
    }
}

static void _overdraw_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "满弓");
        break;
    case SPELL_DESC:
        var_set_string(res, "你以额外的力量射击，但你的命中率会降低。");
        break;
    default:
        _toggle_spell(TOGGLE_OVERDRAW, cmd, res);
        break;
    }
}

static void _rapid_reload_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "快速装填");
        break;
    case SPELL_DESC:
        var_set_string(res, "你每回合的射击次数增加，但由于匆忙，你会损失护甲等级(AC)和命中率。");
        break;
    default:
        _toggle_spell(TOGGLE_RAPID_RELOAD, cmd, res);
        break;
    }
}

static void _shattering_bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎裂弩箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一支弩箭，在撞击时会碎裂以造成额外伤害。");
        break;
    default:
        _fire_spell(SHOOT_SHATTER, cmd, res);
    }
}

/****************************************************************
 * Daggermaster
 ****************************************************************/
static void _dagger_toss_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "投掷匕首");
        break;
    case SPELL_DESC:
        var_set_string(res, "将你的武器投掷向目标怪物。");
        break;
    case SPELL_CAST:
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        else
        {
            py_throw_t context = {0};

            context.type = THROW_BOOMERANG;
            context.mult = 100 + 4 * p_ptr->lev;
            context.back_chance = 20;
            if (_get_toggle() == TOGGLE_FLYING_DAGGER_STANCE)
            {
                context.back_chance += 4 + randint1(5);
                context.to_dd += p_ptr->lev/15;
            }
            var_set_bool(res, py_throw(&context));
        }
        break;
    case SPELL_ENERGY:
        if (_get_toggle() == TOGGLE_FLYING_DAGGER_STANCE)
            var_set_int(res, (100 - p_ptr->lev)*2/3);
        else
            var_set_int(res, 100 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _flying_dagger_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "飞刀架势");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，你的“投掷匕首”技巧将变得极为精湛。投掷出的武器更频繁地返回，且伤害大幅增加。然而，这种架势会让你在敌人面前暴露出一些破绽。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (_get_toggle() == TOGGLE_FLYING_DAGGER_STANCE)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(TOGGLE_FLYING_DAGGER_STANCE);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != TOGGLE_FLYING_DAGGER_STANCE)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _elusive_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "飘忽打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "攻击相邻的怪物，并闪现到你当前位置视线内的一个新位置。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_ELUSIVE_STRIKE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _frenzy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂暴架势");
        break;
    case SPELL_DESC:
        var_set_string(res, "在这种架势下，你在近战中以巨大的力量攻击敌人。然而，你的狂暴会让你在敌人面前露出破绽！");
        break;
    default:
        _toggle_spell(TOGGLE_FRENZY_STANCE, cmd, res);
        break;
    }
}

static void _shadow_stance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "暗影架势");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，你走得既快又隐蔽。在攻击敌人时，你们将交换位置。");
        break;
    default:
        _toggle_spell(TOGGLE_SHADOW_STANCE, cmd, res);
        break;
    }
}

/****************************************************************
 * Diggermaster
 ****************************************************************/
static void _tunnel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "挖掘隧道");
        break;
    case SPELL_DESC:
        var_set_string(res, "挖掘一条通往下一层的隧道。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (!cave_valid_bold(py, px))
        {
            msg_print("你需要空间来挖掘！");
        }
        else
        {
            msg_print("你向下挖掘……");
            stair_creation(TRUE);
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _calamity_of_the_living_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生者之灾");
        break;
    case SPELL_DESC:
        var_set_string(res, "引发地震或破坏。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (p_ptr->lev >= 50 || one_in_(3))
        {
            destroy_area(py, px, 12 + randint1(4), 4 * p_ptr->lev);
        }
        else
        {
            msg_print("地面发出轰隆声！");
            earthquake(py, px, 10);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _object_is_corpse_or_skeleton(object_type *o_ptr)
{
    if (o_ptr->tval == TV_CORPSE) return TRUE;
    if (o_ptr->tval == TV_SKELETON) return TRUE;
    return FALSE;
}

static void _bury_dead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "埋葬死者");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过埋葬尸体或骨架，你将获得暂时的强化。");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        char o_name[MAX_NLEN];
        int turns;

        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }

        prompt.prompt = "埋葬哪具尸体？";
        prompt.error = "你没有可埋葬的东西。";
        prompt.filter = _object_is_corpse_or_skeleton;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;

        /* TV_CORPSE, SV_CORPSE = Corpse
           TV_CORPSE, SV_SKELETON = Skeleton
           TV_SKELETON, ??? = Skeleton */
        if (prompt.obj->tval == TV_CORPSE && prompt.obj->sval == SV_CORPSE)
            turns = 40;
        else
            turns = 15;

        object_desc(o_name, prompt.obj, OD_NAME_ONLY | OD_COLOR_CODED | OD_SINGULAR);
        msg_format("你匆忙挖了一个坟墓并把%s扔了进去。", o_name);

        prompt.obj->number--;
        obj_release(prompt.obj, 0);

        set_blessed(p_ptr->blessed + turns, FALSE);
        if (p_ptr->lev >= 15)
            set_hero(p_ptr->hero + turns, FALSE);
        if (p_ptr->lev >= 30)
            set_fast(p_ptr->fast + turns, FALSE);
        if (p_ptr->lev >= 40)
            set_resist_magic(p_ptr->resist_magic + turns, FALSE);

        if (p_ptr->lev >= 15)
            set_oppose_cold(p_ptr->oppose_cold + turns, FALSE);
        if (p_ptr->lev >= 25)
            set_oppose_pois(p_ptr->oppose_pois + turns, FALSE);
        if (p_ptr->lev >= 30)
            set_tim_hold_life(p_ptr->tim_hold_life + turns, FALSE);
        if (p_ptr->lev >= 35)
            set_tim_res_nether(p_ptr->tim_res_nether + turns, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _barricade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "设置路障");
        break;
    case SPELL_DESC:
        var_set_string(res, "在附近制造一些碎石。");
        break;
    case SPELL_CAST:
    {
        int y, x, dir, cdir;
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }

        if (p_ptr->lev >= 45)
        {
            for (dir = 0; dir < 8; dir++)
            {
                y = py + ddy_ddd[dir];
                x = px + ddx_ddd[dir];

                if (!in_bounds(y, x)) continue;
                if (!cave_naked_bold(y, x)) continue;
                cave_set_feat(y, x, feat_rubble);
            }
        }
        else
        {
            if (!get_rep_dir2(&dir)) return;
            if (dir == 5) return;

            for (cdir = 0;cdir < 8; cdir++)
                if (cdd[cdir] == dir) break;

            if (cdir == 8) return;

            y = py + ddy_cdd[cdir];
            x = px + ddx_cdd[cdir];

            if (in_bounds(y, x) && cave_naked_bold(y, x))
                cave_set_feat(y, x, feat_rubble);

            if (p_ptr->lev >= 35)
            {
                y = py + ddy_cdd[(cdir + 7) % 8];
                x = px + ddx_cdd[(cdir + 7) % 8];
                if (in_bounds(y, x) && cave_naked_bold(y, x))
                    cave_set_feat(y, x, feat_rubble);

                y = py + ddy_cdd[(cdir + 1) % 8];
                x = px + ddx_cdd[(cdir + 1) % 8];
                if (in_bounds(y, x) && cave_naked_bold(y, x))
                    cave_set_feat(y, x, feat_rubble);
            }
        }
        p_ptr->update |= (PU_BONUS | PU_FLOW);
        p_ptr->redraw |= PR_MAP;
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _strength_of_the_undertaker_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "掘墓人之力");
        break;
    case SPELL_DESC:
        var_set_string(res, "根据你挖掘工具的品质，你获得额外的力量。");
        break;
    default:
        _toggle_spell(TOGGLE_STRENGTH_OF_THE_UNDERTAKER, cmd, res);
        break;
    }
}

static void _stoicism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "坚忍");
        break;
    case SPELL_DESC:
        var_set_string(res, "根据你挖掘工具的品质，你获得额外的体质和潜行。");
        break;
    default:
        _toggle_spell(TOGGLE_STOICISM, cmd, res);
        break;
    }
}

static void _industrious_mortician_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "勤勉殡葬师");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，根据你挖掘工具的品质，你获得额外的攻击次数和速度。");
        break;
    default:
        _toggle_spell(TOGGLE_INDUSTRIOUS_MORTICIAN, cmd, res);
        break;
    }
}

/****************************************************************
 * Polearmmaster
 ****************************************************************/
static void _many_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "多重打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的攻击广泛散布在周围的敌人之中。");
        break;
    default:
        _toggle_spell(TOGGLE_MANY_STRIKE, cmd, res);
        break;
    }
}

static void _piercing_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "穿透打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "当你攻击敌人时，成功的击中将穿透对手，攻击到额外的怪物。");
        break;
    default:
        _toggle_spell(TOGGLE_PIERCING_STRIKE, cmd, res);
        break;
    }
}

static void _trip_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "绊倒");
        break;
    case SPELL_DESC:
        var_set_string(res, "当你攻击敌人时，成功的击中将尝试绊倒你的对手。");
        break;
    default:
        _toggle_spell(TOGGLE_TRIP, cmd, res);
        break;
    }
}

static void _reach_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "延伸");
        break;
    case SPELL_DESC:
        var_set_string(res, "此法术延长了你的近战攻击范围。");
        break;
    case SPELL_CAST:
        if (_check_speciality_equip())
        {
            int dir = 5;
            bool b = FALSE;

            project_length = 2 + p_ptr->lev/40;
            if (get_fire_dir(&dir))
            {
                project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
                b = TRUE;
            }
            var_set_bool(res, b);
        }
        else
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            var_set_bool(res, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _knock_back_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "击退");
        break;
    case SPELL_DESC:
        var_set_string(res, "成功的攻击会将你的对手向后推一格。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_KNOCK_BACK));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _reaping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "收割");
        break;
    case SPELL_DESC:
        var_set_string(res, "死亡的旋风，以强大的吸血打击攻击所有相邻的敌人。每一次击杀都会进一步增强后续打击的力量。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_REAPING));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Shieldmaster
 ****************************************************************/
static void _desperation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "绝境求生");
        break;
    case SPELL_DESC:
        var_set_string(res, "通过解除你当前武器的魔法强化，你回复生命值。");
        break;
    case SPELL_CAST:
    {
        int slot, ds, hp;
        object_type *o_ptr = NULL;
        char o_name[MAX_NLEN];

        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你需要一面盾牌。");
            return;
        }
        slot = equip_random_slot(object_is_melee_weapon);
        if (!slot)
        {
            msg_print("失败！你需要一把可以解除强化的武器。");
            return;
        }
        o_ptr = equip_obj(slot);
        ds = o_ptr->to_h + o_ptr->to_d;
        object_desc(o_name, o_ptr, OD_NAME_ONLY | OD_OMIT_PREFIX);

        if (ds > 0)
        {
            hp = damroll(7, ds);
            hp_player(hp);

            if (!object_is_artifact(o_ptr) || one_in_(2))
            {
                if (o_ptr->to_h > 0) o_ptr->to_h--;
                if (o_ptr->to_d > 0) o_ptr->to_d--;
                msg_format("你的%s被解除了强化。", o_name);
            }
            else
            {
                msg_format("你的%s抵抗了强化解除。", o_name);
            }
        }
        else
        {
            msg_format("你的%s太弱了，无法再帮助你了。", o_name);
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _sanctuary_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "庇护所");
        break;
    case SPELL_DESC:
        var_set_string(res, "你将变得无敌，直到你对怪物造成伤害为止。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你需要一面盾牌。");
            return;
        }
        set_sanctuary(TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shield_bash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "盾击");
        break;
    case SPELL_DESC:
        var_set_string(res, "你用盾牌而不是武器进行战斗。");
        break;
    default:
        _toggle_spell(TOGGLE_SHIELD_BASH, cmd, res);
        break;
    }
}

static void _bulwark_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "壁垒");
        break;
    case SPELL_DESC:
        var_set_string(res, "你受到的所有近战伤害减少三分之一。");
        break;
    default:
        _toggle_spell(TOGGLE_BULWARK, cmd, res);
        break;
    }
}

static void _shield_revenge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "复仇");
        break;
    case SPELL_DESC:
        var_set_string(res, "每当怪物伤害你时，它们也会受到伤害。");
        break;
    default:
        _toggle_spell(TOGGLE_SHIELD_REVENGE, cmd, res);
        break;
    }
}

/****************************************************************
 * Slingmaster
 ****************************************************************/
static void _bouncing_pebble_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "弹跳石子");
        break;
    case SPELL_DESC:
        var_set_string(res, "向对手发射一颗石子或弹丸。如果你击中目标，石子或弹丸将向随机方向反弹。");
        break;
    default:
        _fire_spell(SHOOT_BOUNCE, cmd, res);
    }
}

static void _greater_many_shot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "高等多重射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "向所有可见的怪物发射石子。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的远程武器用着不顺手。");
            return;
        }
        else
        {
            int i;
            int tgts[_MAX_TARGETS];
            int ct = _get_greater_many_shot_targets(tgts, _MAX_TARGETS);
            obj_ptr bow = equip_obj(p_ptr->shooter_info.slot);
            obj_ptr ammo = _get_ammo(TRUE);

            if (!ammo)
            {
                flush();
                return;
            }

            shoot_hack = SHOOT_ALL;

            for (i = 0; i < ct; i++)
            {
                int tgt = tgts[i];
                int tx = m_list[tgt].fx;
                int ty = m_list[tgt].fy;

                do_cmd_fire_aux2(bow, ammo, px, py, tx, ty);
            }

            obj_release(ammo, 0);
            shoot_hack = SHOOT_NONE;
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _many_shot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "多重射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "向所有可见的怪物发射石子。不过，你需要对每个目标都有直接的射击路线。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的远程武器用着不顺手。");
            return;
        }
        else
        {
            int i;
            int tgts[_MAX_TARGETS];
            int ct = _get_many_shot_targets(tgts, _MAX_TARGETS);
            obj_ptr bow = equip_obj(p_ptr->shooter_info.slot);
            obj_ptr ammo = _get_ammo(TRUE);

            if (!ammo)
            {
                flush();
                return;
            }

            shoot_hack = SHOOT_MANY;

            for (i = 0; i < ct; i++)
            {
                int tgt = tgts[i];
                int tx = m_list[tgt].fx;
                int ty = m_list[tgt].fy;

                do_cmd_fire_aux2(bow, ammo, px, py, tx, ty);
            }

            obj_release(ammo, 0);
            shoot_hack = SHOOT_NONE;
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rapid_shot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "快速射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，你的所有射击都将针对单一对手发射。");
        break;
    default:
        _toggle_spell(TOGGLE_RAPID_SHOT, cmd, res);
        break;
    }
}

static void _shot_on_the_run_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "移动射击");
        break;
    case SPELL_DESC:
        var_set_string(res, "使用此技巧时，你每次移动都会自动向最近的对手射击。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的远程武器用着不顺手。");
            return;
        }
        if (_get_toggle() == TOGGLE_SHOT_ON_THE_RUN)
            _set_toggle(TOGGLE_NONE);
        else
        {
            obj_ptr ammo = _get_ammo(FALSE);
            if (!ammo)
            {
                flush();
                return;
            }
            shoot_item = ammo->loc;
            _set_toggle(TOGGLE_SHOT_ON_THE_RUN);
            /* _move_player() will handle the gritty details */
        }
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (_get_toggle() != TOGGLE_SHOT_ON_THE_RUN)
            var_set_int(res, 0);    /* no charge for dismissing a technique */
        else
            var_set_int(res, 100);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Staffmaster
 ****************************************************************/
bool _design_monkey_clone(void)
{
    int hand, i;
    monster_race *r_ptr = &r_info[MON_MONKEY_CLONE];
    int dd = 10;
    int ds = 10;
    int dam = 0;
    int tdam = 0;
    int blows = 0;
    int acc = 0;

    if (r_ptr->cur_num == 1)
    {
        msg_print("你同时只能拥有一只灵猴分身！");
        return FALSE;
    }

    r_ptr->hdice = 10;
    r_ptr->hside = p_ptr->mhp / 15;
    r_ptr->ac = p_ptr->ac + p_ptr->to_a;
    r_ptr->speed = (byte)p_ptr->pspeed;

    /* Combat */
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].wield_how != WIELD_NONE)
        {
            object_type *o_ptr = equip_obj(p_ptr->weapon_info[hand].slot);
            int          dd, ds;

            if (!o_ptr) continue;

            dd = o_ptr->dd + p_ptr->weapon_info[hand].to_dd;
            ds = o_ptr->ds + p_ptr->weapon_info[hand].to_ds;

            tdam += NUM_BLOWS(hand) *
                (dd * (ds + 1)/2 + p_ptr->weapon_info[hand].to_d + o_ptr->to_d)/100;

            blows += NUM_BLOWS(hand)/100;
            acc = hit_chance(hand, o_ptr->to_h, 150);
        }
    }

    dam = tdam / MIN(4, blows);
    dd = 10;
    ds = dam/5;
    if (ds < 1)
    {
        dd = 1;
        ds = 1;
    }

    r_ptr->level = (60*acc + 5200)/(300 - 3*acc); /* Don't ask ... */

    for (i = 0; i < 4 && i < blows; i++)
    {
        r_ptr->blows[i].method = 0;
    }

    for (i = 0; i < 4; i++)
    {
        r_ptr->blows[i].method = RBM_HIT;
        r_ptr->blows[i].effects[0].effect = RBE_HURT;
        r_ptr->blows[i].effects[0].dd = dd;
        r_ptr->blows[i].effects[0].ds = ds;
    }

    /* Resistances */
    r_ptr->flagsr = 0;
    r_ptr->flags3 = 0;
    r_ptr->flags2 = 0;
    r_ptr->flags7 = 0;

    if (p_ptr->resist[RES_ACID]) r_ptr->flagsr |= RFR_RES_ACID;
    if (p_ptr->resist[RES_ELEC]) r_ptr->flagsr |= RFR_RES_ELEC;
    if (p_ptr->resist[RES_FIRE]) r_ptr->flagsr |= RFR_RES_FIRE;
    if (p_ptr->resist[RES_COLD]) r_ptr->flagsr |= RFR_RES_COLD;
    if (p_ptr->resist[RES_POIS]) r_ptr->flagsr |= RFR_RES_POIS;
    if (p_ptr->resist[RES_LITE]) r_ptr->flagsr |= RFR_RES_LITE;
    if (p_ptr->resist[RES_DARK]) r_ptr->flagsr |= RFR_RES_DARK;
    if (p_ptr->resist[RES_NETHER]) r_ptr->flagsr |= RFR_RES_NETH;
    if (p_ptr->resist[RES_SHARDS]) r_ptr->flagsr |= RFR_RES_SHAR;
    if (p_ptr->resist[RES_SOUND]) r_ptr->flagsr |= RFR_RES_SOUN;
    if (p_ptr->resist[RES_CHAOS]) r_ptr->flagsr |= RFR_RES_CHAO;
    if (p_ptr->resist[RES_NEXUS]) r_ptr->flagsr |= RFR_RES_NEXU;
    if (p_ptr->resist[RES_DISEN]) r_ptr->flagsr |= RFR_RES_DISE;

    if (p_ptr->resist[RES_CONF]) r_ptr->flags3 |= RF3_NO_CONF;
    if (p_ptr->resist[RES_FEAR]) r_ptr->flags3 |= RF3_NO_FEAR;
    if (p_ptr->free_act) r_ptr->flags3 |= RF3_NO_SLEEP;
    if (p_ptr->sh_cold) r_ptr->flags3 |= RF3_AURA_COLD;

    if (p_ptr->reflect) r_ptr->flags2 |= RF2_REFLECTING;
    if (p_ptr->regen >= 200) r_ptr->flags2 |= RF2_REGENERATE;
    if (p_ptr->sh_fire) r_ptr->flags2 |= RF2_AURA_FIRE;
    if (p_ptr->sh_elec) r_ptr->flags2 |= RF2_AURA_ELEC;
    if (p_ptr->pass_wall) r_ptr->flags2 |= RF2_PASS_WALL;
    r_ptr->flags2 |= RF2_OPEN_DOOR;
    r_ptr->flags2 |= RF2_CAN_SPEAK;

    r_ptr->flags7 |= RF7_CAN_SWIM;
    if (p_ptr->levitation) r_ptr->flags7 |= RF7_CAN_FLY;

    r_ptr->r_xtra1 |= MR1_LORE;

    return TRUE;
}

static void _monkey_king_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "猴王绝技");
        break;
    case SPELL_DESC:
        var_set_string(res, "制造一个你的分身，但要付出巨大的代价。");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (_design_monkey_clone() && summon_named_creature(0, py, px, MON_MONKEY_CLONE, PM_FORCE_PET))
            var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->mhp + 2)/3);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _circle_kick(void)
{
    int i;
    int dd = 1;
    int ds = p_ptr->lev;
    int bonus = p_ptr->to_h_m;
    int chance = p_ptr->skills.thn + (bonus * BTH_PLUS_ADJ);
    int slot = equip_find_obj(TV_BOOTS, SV_ANY);

    if (slot)
        dd = equip_obj(slot)->ac;

    for (i = 0; i < 8; i++)
    {
        int           dir = cdd[i];
        int           y = py + ddy[dir];
        int           x = px + ddx[dir];
        cave_type    *c_ptr = &cave[y][x];
        monster_type *m_ptr = &m_list[c_ptr->m_idx];

        if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
        {
            monster_race *r_ptr = &r_info[m_ptr->r_idx];
            char          m_name[MAX_NLEN];

            monster_desc(m_name, m_ptr, 0);

            if (test_hit_norm(chance, mon_ac(m_ptr), m_ptr->ml))
            {
                int dam = damroll(dd, ds) + p_ptr->to_d_m;

                sound(SOUND_HIT);
                msg_format("你踢了%s。", m_name);

                if (!(r_ptr->flags3 & RF3_NO_STUN))
                {
                    if (mon_stun(m_ptr, mon_stun_amount(dam)))
                        msg_format("%s被打懵了。", m_name);
                    else
                        msg_format("%s被打得更懵了。", m_name);
                }
                else
                    msg_format("%s没有受到影响。", m_name);


                dam = mon_damage_mod(m_ptr, dam, FALSE);

                if (dam > 0)
                {
                    bool fear;
                    mon_take_hit(c_ptr->m_idx, dam, DAM_TYPE_MELEE, &fear, NULL);

                    anger_monster(m_ptr);
                }
                retaliation_count = 0; /* AURA_REVENGE */
                touch_zap_player(c_ptr->m_idx);
            }
            else
            {
                sound(SOUND_MISS);
                msg_format("你未击中%s。", m_name);
            }
        }
    }
}

static void _circle_kick_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "旋风踢");
        break;
    case SPELL_DESC:
        var_set_string(res, "踢击所有相邻的对手并使他们震慑。伤害取决于你的靴子！");
        break;
    case SPELL_INFO:
    {
        int ds = p_ptr->lev;
        int dd = 0;
        int slot = equip_find_obj(TV_BOOTS, SV_ANY);

        if (slot)
            dd = equip_obj(slot)->ac;

        var_set_string(res, info_damage(dd, ds, p_ptr->to_d_m));
        break;
    }
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (no_melee_check()) return; 
        _circle_kick();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

 static bool _vault_attack(void)
{
    int tx, ty;
    int tm_idx = 0;
    u16b path_g[32];
    int path_n, i;
    bool moved = FALSE;
    int flg = PROJECT_THRU | PROJECT_KILL;
    int dir;

    project_length = 3;

    if (!get_fire_dir(&dir)) return FALSE;

    tx = px + project_length * ddx[dir];
    ty = py + project_length * ddy[dir];

    if ((dir == 5) && target_okay())
    {
        tx = target_col;
        ty = target_row;
    }

    if (in_bounds(ty, tx)) tm_idx = cave[ty][tx].m_idx;

    path_n = project_path(path_g, project_length, py, px, ty, tx, flg);
    project_length = 0;

    if (!path_n) return FALSE;

    ty = py;
    tx = px;

    /* Scrolling the cave would invalidate our path! */
    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
        wilderness_scroll_lock = TRUE;

    for (i = 0; i < path_n; i++)
    {
        cave_type *c_ptr;
        bool can_enter = FALSE;
        int ny = GRID_Y(path_g[i]);
        int nx = GRID_X(path_g[i]);

        c_ptr = &cave[ny][nx];
        can_enter = !c_ptr->m_idx && player_can_enter(c_ptr->feat, 0);

        if (can_enter)
        {
            ty = ny;
            tx = nx;
            continue;
        }

        if (!c_ptr->m_idx)
        {
            msg_print("失败！");
            break;
        }

        /* Move player before updating the monster */
        if (!player_bold(ty, tx)) move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
        moved = TRUE;

        update_mon(c_ptr->m_idx, TRUE);

        if (tm_idx != c_ptr->m_idx)
        {
            /* Just like "Acrobatic Charge." Attempts to displace monsters on route. */
            set_monster_csleep(c_ptr->m_idx, 0);
            move_player_effect(ny, nx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
            ty = ny;
            tx = nx;
            continue;
        }
        py_attack(ny, nx, 0);
        break;
    }

    if (!moved && !player_bold(ty, tx)) move_player_effect(ty, tx, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);

    if (!dun_level && !p_ptr->wild_mode && !p_ptr->inside_arena && !p_ptr->inside_battle)
    {
        wilderness_scroll_lock = FALSE;
        wilderness_move_player(px, py);
    }
    return TRUE;
}

static void _vault_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "飞跃攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一次移动中冲锋并攻击附近的对手。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(3));
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (!_check_speciality_equip())
        {
            msg_print("失败！你觉得你的武器用着不顺手。");
            return;
        }
        if (_vault_attack())
            var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _flurry_of_blows_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "疾风连击");
        break;
    case SPELL_DESC:
        var_set_string(res, "用大量的打击攻击单一对手，在此过程中会使自己筋疲力尽。");
        break;
    case SPELL_CAST:
        var_set_bool(res, _do_blow(WEAPONMASTER_FLURRY));
        break;
    case SPELL_ENERGY:
        var_set_int(res, 100 + ENERGY_NEED());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


/****************************************************************
 * Swordmaster
 ****************************************************************/
static void _burning_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "燃烧之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃伤害变为火焰伤害，并且绝对不会落空。");
        break;
    default:
        _toggle_spell(TOGGLE_BURNING_BLADE, cmd, res);
        break;
    }
}

static void _ice_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寒冰之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃伤害变为冰霜伤害，并使你的对手减速。");
        break;
    default:
        _toggle_spell(TOGGLE_ICE_BLADE, cmd, res);
        break;
    }
}

static void _thunder_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "雷霆之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃伤害变为闪电伤害，并使你的对手震慑。");
        break;
    default:
        _toggle_spell(TOGGLE_THUNDER_BLADE, cmd, res);
        break;
    }
}

static void _blood_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃渴望着活物。");
        break;
    default:
        _toggle_spell(TOGGLE_BLOOD_BLADE, cmd, res);
        break;
    }
}

static void _holy_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神圣之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃能强力对抗邪恶势力。");
        break;
    default:
        _toggle_spell(TOGGLE_HOLY_BLADE, cmd, res);
        break;
    }
}

static void _order_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "秩序之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的剑刃成为秩序的武器，总是造成最大伤害。");
        break;
    default:
        _toggle_spell(TOGGLE_ORDER_BLADE, cmd, res);
        break;
    }
}

static void _wild_blade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂野之刃");
        break;
    case SPELL_DESC:
        var_set_string(res, "这太疯狂了，无法形容。反正你也不会相信我！");
        break;
    default:
        _toggle_spell(TOGGLE_WILD_BLADE, cmd, res);
        break;
    }
}


/****************************************************************
 * Spell Table and Exports
 ****************************************************************/

#define _MAX_OBJECTS_PER_SPECIALITY 32
#define _MAX_SPECIALITIES           11
#define _MAX_SPELLS_PER_SPECIALITY  10

int weaponmaster_get_toggle(void)
{
    /* exposed for prtstatus() in xtra1.c
       this is easier than rewriting the status code so that classes can maintain it!
    */
    int result = TOGGLE_NONE;
    if (p_ptr->pclass == CLASS_WEAPONMASTER)
        result = _get_toggle();
    return result;
}

void weaponmaster_set_toggle(int toggle)
{
    if (p_ptr->pclass == CLASS_WEAPONMASTER)
        _set_toggle(toggle);
}

#define _WEAPONMASTER_MELEE   1
#define _WEAPONMASTER_SHIELDS 2
#define _WEAPONMASTER_BOWS    3

typedef struct {
    byte tval;
    byte sval;
} _object_kind;

typedef struct {
    cptr name;
    cptr help;
    int kind;
    int stats[MAX_STATS];
    skills_t base_skills;
    skills_t extra_skills;
    _object_kind objects[_MAX_OBJECTS_PER_SPECIALITY];    /* There is always a sentinel at the end */
    spell_info spells[_MAX_SPELLS_PER_SPECIALITY];        /* There is always a sentinel at the end */
    _object_kind birth_obj;
} _speciality;


/*  p_ptr->psubclass indexes into _specialities.
    This index is persisted in savefiles and are chosen
    by the player at startup, so moving things around is
    unwise unless you put code to fix up old savefiles
    in load.c.
*/
static _speciality _specialities[_MAX_SPECIALITIES] = {
    { "斧头",
      "The mighty axe! Your blows will cleave with damage unsurpassed. "
      "Specializing in axes gives great offensive prowess, especially when "
      "your axe is wielded with two hands. However, this speciality offers "
      "little in the way of utility. Kill quickly as your life depends on it!",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      {+3, -2, -1, -2, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  17,  17,  26,   1,  14,   2, 70, 25},
      {   7,   7,   9,   0,   0,   0, 30, 11},
      { { TV_POLEARM, SV_HATCHET },
        { TV_POLEARM, SV_BATTLE_AXE },
        { TV_POLEARM, SV_BEAKED_AXE },
        { TV_POLEARM, SV_BROAD_AXE },
        { TV_POLEARM, SV_LOCHABER_AXE },
        { TV_POLEARM, SV_GREAT_AXE },
        { 0, 0 },
      },
      { { 10,   0,  0, _power_attack_spell },
        { 15,  10,  0, berserk_spell },
        { 25,  20, 50, _judge_spell },
        { 25,  12,  0, _crusaders_strike_spell },
        { 35,  25, 50, massacre_spell },
        { 40,  25,  0, _vicious_strike_spell },
        { -1,   0,  0, NULL },
      },
      { TV_POLEARM, SV_BROAD_AXE },
    },
    { "弓类",
      "You will shoot to kill! The bowmaster gains techniques to enhance shooting, "
      "including more rapid firing, the ability to volley shots over the heads of "
      "intervening monsters, reduced ammo destruction, the ability to kill with a "
      "single arrow and much more. As a shooter, your missile prowess will be quite "
      "formidable, though your melee will be somewhat lacking.",
      _WEAPONMASTER_BOWS,
    /*  S   I   W   D   C   C */
      { 0,  0,  0, +2, -1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  23,  23,  29,   4,  23,  13, 48, 72},
      {   8,   8,  10,   0,   0,   0, 13, 28},
      { { TV_BOW, SV_SHORT_BOW },
        { TV_BOW, SV_LONG_BOW },
        { TV_BOW, SV_NAMAKE_BOW },
        { 0, 0 },
      },
      { {  5,   0,  0, _readied_shot_spell },
        { 10,   5,  0, _volley_spell },
        { 25,  20, 50, _judge_spell },
        { 25,  10,  0, _tranquilizing_arrow_spell },
        { 30,   0,  0, _piercing_arrow_spell },
        { 35,  20,  0, _arrow_of_slaying_spell },
        { 40,  30,  0, _disintegration_arrow_spell },
        { -1,   0,  0, NULL },
      },
      { TV_BOW, SV_SHORT_BOW },
    },
    { "棍棒",
        "You will seek to club your opponents senseless! This speciality gains passive "
        "status effects against monsters, such as confusion, knock out and stunning. Also, "
        "you will gain some limited utility techniques. At high levels, your weapons will "
        "become more likely to score devastating, crushing blows against your hapless enemies.",
        _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      {+2, -2, -1, -2, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  18,  18,  26,   1,  14,   2, 60, 30},
      {   7,   7,   9,   0,   0,   0, 20, 15},
        { { TV_HAFTED, SV_BALL_AND_CHAIN },
          { TV_HAFTED, SV_CLUB },
          { TV_HAFTED, SV_FLAIL },
          { TV_HAFTED, SV_GREAT_HAMMER },
          { TV_HAFTED, SV_LEAD_FILLED_MACE },
          { TV_HAFTED, SV_MACE },
          { TV_HAFTED, SV_MACE_OF_DISRUPTION },
          { TV_HAFTED, SV_MORNING_STAR },
          { TV_HAFTED, SV_TETSUBO },
          { TV_HAFTED, SV_TWO_HANDED_FLAIL },
          { TV_HAFTED, SV_WAR_HAMMER },
          { TV_HAFTED, SV_GROND },
          { TV_HAFTED, SV_BASEBALL_BAT },
          { TV_HAFTED, SV_NAMAKE_HAMMER },
          { 0, 0 },
        },
        { {  5,   0,  0, _combat_expertise_spell },
          { 10,   5,  0, _throw_weapon_spell },
          { 15,  10,  0, _cunning_strike_spell },
          { 25,  20, 50, _judge_spell },
          { 25,  15, 40, _smash_ground_spell },
          { 30,  25,  0, _smite_evil_spell },
          { 35,   0,  0, _trade_blows_spell },
          { -1,   0,  0, NULL },
        },
        { TV_HAFTED, SV_CLUB },
    },
    { "弩类",
      "The crossbowmaster shoots deadly bolts for great damage. Their bolts may explode "
      "powerfully damaging nearby monsters. Also, they may shoot so hard as to knock "
      "their opponents backwards!",
      _WEAPONMASTER_BOWS,
    /*  S   I   W   D   C   C */
      {+1, -1, -2,  0, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  24,  24,  26,   3,  18,  10, 48, 72},
      {   8,   8,   8,   0,   0,   0, 13, 28},
      { { TV_BOW, SV_LIGHT_XBOW },
        { TV_BOW, SV_HEAVY_XBOW },
        { 0, 0 },
      },
      { {  5,   0,  0, _careful_aim_spell },
        { 10,  10,  0, _shattering_bolt_spell },
        { 25,  20, 50, _judge_spell },
        { 25,  15,  0, _knockback_bolt_spell },
        { 30,   0,  0, _exploding_bolt_spell },
        { 35,  30,  0, _elemental_bolt_spell },
        { 38,   0,  0, _rapid_reload_spell },
        { 42,   0,  0, _overdraw_spell },
        { -1,   0,  0, NULL },
      },
      { TV_BOW, SV_LIGHT_XBOW },
    },
    { "匕首",
      "A knife in the back! This speciality favors dual-wielding and rogue-type behavior. "
      "The daggermaster can even assume the posture of The Flying Dagger which greatly "
      "enhances their low level dagger toss capability. Indeed, their prowess with the "
      "dagger toss is legendary and appears almost magical! At high levels, you will also "
      "gain formidable melee prowess with the Frenzy Stance. Finally, daggermasters have "
      "very strong short ranged teleport techniques that synergize well with their toss "
      "abilities.",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      { 0, +1,  0, +3, -1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  30,  24,  31,   5,  30,  20, 60, 66},
      {  12,   8,  10,   0,   0,   0, 18, 20},
      { { TV_SWORD, SV_BASILLARD },
        { TV_SWORD, SV_BROKEN_DAGGER },
        { TV_SWORD, SV_DAGGER },
        { TV_SWORD, SV_FALCON_SWORD },
        { TV_SWORD, SV_MAIN_GAUCHE },
        { TV_SWORD, SV_NINJATO },
        { TV_SWORD, SV_RAPIER },
        { TV_SWORD, SV_SABRE },
        { TV_SWORD, SV_TANTO },
        { TV_SWORD, SV_DRAGON_FANG },
        { 0, 0 },
      },
      {
        {  5,   5,  0, _dagger_toss_spell },
        { 10,   5, 40, strafing_spell },
        { 15,   0,  0, _flying_dagger_spell },
        { 25,  20, 50, _judge_spell },
        { 30,  10,  0, _elusive_strike_spell },
        { 35,   0,  0, _shadow_stance_spell },
        { 45,   0,  0, _frenzy_spell },
        { -1,   0,  0, NULL },
      },
      { TV_SWORD, SV_DAGGER },
    },
    { "长柄武器",
      "You don a grim face before setting out to reap your harvest of death. You will swing "
      "your weapon wide often affecting multiple surrounding opponents.",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      {+2, -1, -1,  0, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  20,  19,  28,   1,  14,   2, 65, 25},
      {   8,   7,  10,   0,   0,   0, 27, 11},
      {
        { TV_POLEARM, SV_AWL_PIKE },
        { TV_POLEARM, SV_BROAD_SPEAR },
        { TV_POLEARM, SV_DEATH_SCYTHE },
        { TV_POLEARM, SV_HALBERD },
        { TV_POLEARM, SV_FAUCHARD },
        { TV_POLEARM, SV_GLAIVE },
        { TV_POLEARM, SV_GUISARME },
        { TV_POLEARM, SV_LUCERNE_HAMMER },
        { TV_POLEARM, SV_NAGINATA },
        { TV_POLEARM, SV_PIKE },
        { TV_POLEARM, SV_SCYTHE },
        { TV_POLEARM, SV_SCYTHE_OF_SLICING },
        { TV_POLEARM, SV_SPEAR },
        { TV_POLEARM, SV_TRIDENT },
        { TV_POLEARM, SV_LANCE },
        { TV_POLEARM, SV_HEAVY_LANCE },
        { TV_POLEARM, SV_TRIFURCATE_SPEAR },
        { 0, 0 },
      },
      {
        {  5,   0,  0, _many_strike_spell },
        { 10,   5,  0, _reach_spell },
        { 15,  15,  0, _knock_back_spell },
        { 25,  20, 50, _judge_spell },
        { 25,   0,  0, _piercing_strike_spell },
        { 35,   0,  0, _trip_spell },
        { 40,  40,  0, _reaping_spell },
        { -1,   0,  0, NULL },
      },
      { TV_POLEARM, SV_SPEAR },
    },
    { "盾牌",
      "Specializing in shields gives excellent powers of defense and retaliation. In addition, "
      "you can even choose to melee with your shield rather than a normal weapon, bashing your "
      "opponents senseless. This form of combat is known as Shield Bashing and is unique to this "
      "speciality.",
      _WEAPONMASTER_SHIELDS,
    /*  S   I   W   D   C   C */
      {+2,  0, +1,  0, +2,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  25,  24,  40,   1,  12,   2, 54, 25},
      {  10,   8,  12,   0,   0,   0, 22, 11},
      {
        { TV_SHIELD, SV_DRAGON_SHIELD },
        { TV_SHIELD, SV_KNIGHT_SHIELD },
        { TV_SHIELD, SV_LARGE_LEATHER_SHIELD },
        { TV_SHIELD, SV_LARGE_METAL_SHIELD },
        { TV_SHIELD, SV_MIRROR_SHIELD },
        { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
        { TV_SHIELD, SV_SMALL_METAL_SHIELD },
        { TV_SHIELD, SV_MITHRIL_SHIELD },
        { 0, 0 },
      },
      {
        { 10,  0,  0, _shield_bash_spell },
        { 15, 10,  0, _desperation_spell },
        { 25,  20, 50, _judge_spell },
        { 30,  0,  0, _bulwark_spell },
        { 35, 50,  0, _sanctuary_spell },
        { 40,  0,  0, _shield_revenge_spell },
        { -1,  0,  0, NULL },
      },
      { TV_SHIELD, SV_SMALL_LEATHER_SHIELD },
    },
    { "弹弓",
      "Watch out, Goliath! As a master of slings you will shoot pebbles with uncanny speed. Your "
      "shots may even ricochet off other monsters to score multiple hits. As with other archery "
      "specializations, your ammo will break less often.",
      _WEAPONMASTER_BOWS,
    /*  S   I   W   D   C   C */
      {-1, +1, +1, +3, -1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  30,  24,  29,   4,  23,  13, 48, 72},
      {   8,   8,  10,   0,   0,   0, 13, 28},
      { { TV_BOW, SV_SLING },
        { 0, 0 },
      },
      {
        {  5,   5,  0, _bouncing_pebble_spell },
        { 15,  15,  0, _many_shot_spell },
        { 25,  20, 50, _judge_spell },
        { 25,   0,  0, _shot_on_the_run_spell },
        { 30,  15,  0, _greater_many_shot_spell },
        { 35,   0,  0, _rapid_shot_spell },
        { -1,   0,  0, NULL },
      },
      { TV_BOW, SV_SLING },
    },
    { "法杖",
      "Monkey King! You will battle opponents with a flurry of blows from your mighty "
      "staff and will be prepared to counter the attacks of your enemies. You can vault into "
      "battle and circle kick enemies for stunning effects, attacking them with your boots! "
      "You may even eventually clone yourself at great cost.",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      { 0, -1,  0, +2, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  24,  24,  31,   2,  14,   4, 60, 25},
      {   8,   8,  10,   0,   0,   0, 24, 11},
      {
        { TV_HAFTED, SV_BO_STAFF },
        { TV_HAFTED, SV_JO_STAFF },
        { TV_HAFTED, SV_QUARTERSTAFF },
        { TV_HAFTED, SV_WIZSTAFF },
        { TV_HAFTED, SV_THREE_PIECE_ROD },
        { 0, 0 },
      },
      {
        { 15, 15, 0, _vault_attack_spell },
        { 25, 20,50, _judge_spell },
        { 25, 25, 0, _circle_kick_spell },
        { 40,  0, 0, _monkey_king_spell },
        { 45, 80, 0, _flurry_of_blows_spell },
        { -1,  0, 0, NULL },
      },
      { TV_HAFTED, SV_QUARTERSTAFF },
    },
    { "剑类",
      "You will become a true swordmaster! Mastery of the blade will augment "
      "your weapon with elemental, vorpal or vampiric powers.",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      {+1, -1, -1, +1, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  20,  18,  29,   1,  14,   2, 67, 25},
      {   8,   7,   9,   0,   0,   0, 27, 11},
      { { TV_SWORD, SV_BASTARD_SWORD } ,
        { TV_SWORD, SV_BROKEN_SWORD } ,
        { TV_SWORD, SV_BLADE_OF_CHAOS } ,
        { TV_SWORD, SV_BROAD_SWORD } ,
        { TV_SWORD, SV_CLAYMORE } ,
        { TV_SWORD, SV_CUTLASS } ,
        { TV_SWORD, SV_DIAMOND_EDGE } ,
        { TV_SWORD, SV_ESPADON } ,
        { TV_SWORD, SV_EXECUTIONERS_SWORD } ,
        { TV_SWORD, SV_FLAMBERGE } ,
        { TV_SWORD, SV_GREAT_SCIMITAR } , /* Falchion */
        { TV_SWORD, SV_KATANA } ,
        { TV_SWORD, SV_LONG_SWORD } ,
        { TV_SWORD, SV_KHOPESH } ,
        { TV_SWORD, SV_NO_DACHI },
        { TV_SWORD, SV_SCIMITAR } ,
        { TV_SWORD, SV_SHORT_SWORD } ,
        { TV_SWORD, SV_SMALL_SWORD } ,
        { TV_SWORD, SV_TULWAR } ,
        { TV_SWORD, SV_TWO_HANDED_SWORD } ,
        { TV_SWORD, SV_WAKIZASHI } ,
        { TV_SWORD, SV_ZWEIHANDER } ,
        { TV_SWORD, SV_RUNESWORD } ,
        { 0, 0 },
      },
      {
        {  5,   0,  0, _burning_blade_spell },
        { 10,   0,  0, _ice_blade_spell },
        { 15,   0,  0, _thunder_blade_spell },
        { 25,  20, 50, _judge_spell },
        { 25,   0,  0, _blood_blade_spell },
        { 30,   0,  0, _holy_blade_spell },
        { 35,   0,  0, _order_blade_spell },
        { 40,   0,  0, _wild_blade_spell },
        { -1,   0,  0, NULL },
      },
      { TV_SWORD, SV_LONG_SWORD } ,
    },
    { "挖掘工具",
      "A master of digging. You prefer rocky enclosures and don't mind "
      "lugging around a corpse or two, which you can bury in a pinch for "
      "a temporary bonus. You can choose one of several postures which allow "
      "you to use the digging bonus of your current weapon as an additional "
      "bonus, such as increased strength, constitution, or even speed! So keep "
      "your eye open for those +8 diggers!",
      _WEAPONMASTER_MELEE,
    /*  S   I   W   D   C   C */
      {+2, -1, -1,  0, +1,  0},
    /* Dsrm Dvce Save Stlh Srch Prcp Thn Thb*/
      {  25,  21,  33,   3,  14,   2, 60, 25},
      {   9,   8,  11,   0,   0,   0, 26, 11},
      {
        { TV_DIGGING, SV_SHOVEL},
        { TV_DIGGING, SV_GNOMISH_SHOVEL},
        { TV_DIGGING, SV_DWARVEN_SHOVEL},
        { TV_DIGGING, SV_PICK},
        { TV_DIGGING, SV_ORCISH_PICK},
        { TV_DIGGING, SV_DWARVEN_PICK},
        { TV_DIGGING, SV_MATTOCK},
        { 0, 0 },
      },
      {
        {  5, 10, 30, _bury_dead_spell },
        { 10,  0,  0, _strength_of_the_undertaker_spell },
        { 20, 20, 40, _tunnel_spell },
        { 25,  20, 50, _judge_spell },
        { 25,  0,  0, _stoicism_spell },
        { 30, 25, 40, _barricade_spell },
        { 35, 30,  0, _calamity_of_the_living_spell },
        { 40,  0,  0, _industrious_mortician_spell },
        { -1,  0,  0, NULL },
      },
      { TV_DIGGING, SV_PICK },
    },
};

static bool _check_speciality_aux(object_type *o_ptr)
{
    int i;
    _speciality *ptr = &_specialities[p_ptr->psubclass];

    for (i = 0; i < _MAX_OBJECTS_PER_SPECIALITY; i++)
    {
        if (ptr->objects[i].tval == 0) break;
        if (ptr->objects[i].tval == o_ptr->tval && ptr->objects[i].sval == o_ptr->sval) return TRUE;
    }

    return FALSE;
}

static bool _can_judge(obj_ptr obj)
{
    switch (p_ptr->psubclass)
    {
    case WEAPONMASTER_CROSSBOWS:
        if (obj->tval == TV_BOLT) return TRUE;
        break;
    case WEAPONMASTER_SLINGS:
        if (obj->tval == TV_SHOT) return TRUE;
        break;
    case WEAPONMASTER_BOWS:
        if (obj->tval == TV_ARROW) return TRUE;
        break;
    }
    return _check_speciality_aux(obj);
}

static bool _check_speciality_equip(void)
{
    bool result = equip_find_first(_check_speciality_aux);

    /* For melee specialities, all melee weapons must be favored. Shieldmasters
       and shooters just need a single matching object to be OK. */
    if (_specialities[p_ptr->psubclass].kind == _WEAPONMASTER_MELEE)
    {
        int slot;
        for (slot = equip_find_first(object_is_melee_weapon);
                slot;
                slot = equip_find_next(object_is_melee_weapon, slot))
        {
            object_type *o_ptr = equip_obj(slot);
            if (!_check_speciality_aux(o_ptr))
                return FALSE;
        }
    }
    return result;
}

bool weaponmaster_is_favorite(object_type *o_ptr)
{
    return _check_speciality_aux(o_ptr);
}

static bool _weaponmaster_object_is_icky(object_type *o_ptr)
{
    if (_specialities[p_ptr->psubclass].kind == _WEAPONMASTER_BOWS)
    {
        if (o_ptr->tval != TV_BOW) return FALSE;
        return (!_check_speciality_aux(o_ptr));
    }
    if (_specialities[p_ptr->psubclass].kind != _WEAPONMASTER_MELEE) return FALSE; /* shieldmasters can use weapons */
    if (!object_is_melee_weapon(o_ptr)) return FALSE;
    return (!_check_speciality_aux(o_ptr));
}

static spell_info *_get_spells(void)
{
    int i;
    int ct = 0;
    int max = MAX_SPELLS;
    static spell_info spells[MAX_SPELLS];

    for (i = 0; ; i++)
    {
        spell_info *base = &_specialities[p_ptr->psubclass].spells[i];
        if (base->level <= 0) break;
        if (ct >= max) break;
        if ((base->level <= p_ptr->lev) || (show_future_spells))
        {
            spell_info* current = &spells[ct++];
            current->fn = base->fn;
            current->level = base->level;
            current->cost = base->cost;
            current->fail = base->fail;
        }
    }
    spells[ct].fn = NULL;
    return spells;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.which_stat = A_STR;
        me.magic_desc = "skill";
        me.options = CASTER_USE_HP;
        init = TRUE;
    }
    return &me;
}

void _on_birth(void)
{
    object_type forge;
    _object_kind kind;
    int i;

    if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
        object_prep(&forge, lookup_kind(TV_SWORD, SV_LONG_SWORD));
        py_birth_obj(&forge);
    }

    /* Give the player a starting weapon from this group */
    kind = _specialities[p_ptr->psubclass].birth_obj;
    object_prep(&forge, lookup_kind(kind.tval, kind.sval));
    py_birth_obj(&forge);

    if (kind.tval == TV_BOW)
    {
        switch (kind.sval)
        {
        case SV_SLING:
            object_prep(&forge, lookup_kind(TV_SHOT, SV_SHOT));
            forge.number = (byte)rand_range(15, 20);
            py_birth_obj(&forge);
            break;
        case SV_SHORT_BOW:
        case SV_LONG_BOW:
            object_prep(&forge, lookup_kind(TV_ARROW, SV_ARROW));
            forge.number = (byte)rand_range(15, 20);
            py_birth_obj(&forge);
            break;
        case SV_LIGHT_XBOW:
        case SV_HEAVY_XBOW:
            object_prep(&forge, lookup_kind(TV_BOLT, SV_BOLT));
            forge.number = (byte)rand_range(15, 20);
            py_birth_obj(&forge);
            break;
        }
        object_prep(&forge, lookup_kind(TV_SWORD, SV_DAGGER));
        py_birth_obj(&forge);
    }

    for (i = 0; i < _MAX_OBJECTS_PER_SPECIALITY; i++)
    {
        kind = _specialities[p_ptr->psubclass].objects[i];
        if (kind.tval == 0) break;

        if (kind.tval != TV_SHIELD)
            p_ptr->weapon_exp[kind.tval-TV_WEAPON_BEGIN][kind.sval] = WEAPON_EXP_BEGINNER;
    }

    if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
        skills_shield_init(SV_SMALL_LEATHER_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_SMALL_METAL_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_LARGE_LEATHER_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_LARGE_METAL_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_MITHRIL_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_MIRROR_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_DRAGON_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
        skills_shield_init(SV_KNIGHT_SHIELD, WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    }
    weaponmaster_adjust_skills();

    py_birth_obj_aux(TV_SOFT_ARMOR, SV_LEATHER_JACK, 1);
}

static void _set_max_skill(int tval, int skill)
{
    int j;
    for (j = 0; j < 64; j++)
        s_info[p_ptr->pclass].w_max[tval - TV_WEAPON_BEGIN][j] = skill;
}

void weaponmaster_adjust_skills(void)
{
    int i, j;
    _object_kind kind;

    /* Fix up skills for Speciality. This needs to be called every time the game is loaded! */
    /* Bang everything in class (melee, bows, shields) down to unskilled max */
    switch (_specialities[p_ptr->psubclass].kind)
    {
    case _WEAPONMASTER_MELEE:
        _set_max_skill(TV_DIGGING, WEAPON_EXP_UNSKILLED);
        _set_max_skill(TV_HAFTED, WEAPON_EXP_UNSKILLED);
        _set_max_skill(TV_POLEARM, WEAPON_EXP_UNSKILLED);
        _set_max_skill(TV_SWORD, WEAPON_EXP_UNSKILLED);
        break;

    case _WEAPONMASTER_BOWS:
        _set_max_skill(TV_BOW, WEAPON_EXP_UNSKILLED);
        break;

    case _WEAPONMASTER_SHIELDS:
        /* This only needs to be done once, in _birth. */
        break;
    }

    /* Now make favored weapons "masterable" */
    for (i = 0; i < _MAX_OBJECTS_PER_SPECIALITY; i++)
    {
        kind = _specialities[p_ptr->psubclass].objects[i];
        if (kind.tval == 0 || kind.tval == TV_SHIELD) break;

        s_info[p_ptr->pclass].w_max[kind.tval-TV_WEAPON_BEGIN][kind.sval] = WEAPON_EXP_MASTER;
    }

    /* Patch up current skills since we keep changing the allowed maximums */
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 64; j++)
        {
            if (p_ptr->weapon_exp[i][j] > s_info[p_ptr->pclass].w_max[i][j])
                p_ptr->weapon_exp[i][j] = s_info[p_ptr->pclass].w_max[i][j];
        }
    }
}

static int _max_pval(void)
{
    int slot;
    int result = 0;

    for (slot = equip_find_first(object_is_melee_weapon);
            slot;
            slot = equip_find_next(object_is_melee_weapon, slot))
    {
        object_type *o_ptr = equip_obj(slot);
        result = MAX(result, o_ptr->pval);
    }
    return result;
}

static void _calc_bonuses(void)
{
    static bool last_spec = FALSE;
    static bool init = FALSE;
    bool spec = _check_speciality_equip();

    p_ptr->speciality_equip = spec;

    /* Handle cases where user swaps in unfavorable gear */
    if (!spec && _get_toggle() != TOGGLE_NONE)
    {
        /* Triggering a recursive call to calc_bonuses would be bad ... */
        /*    _set_toggle(TOGGLE_NONE); */
        /* This assumes all bonus calcs are handled here and in _calc_weapon_bonuses() */
        p_ptr->magic_num1[0] = TOGGLE_NONE;
        p_ptr->redraw |= (PR_STATUS);
    }

    if (p_ptr->psubclass == WEAPONMASTER_SLINGS)
    {
        if (spec)
        {
            p_ptr->return_ammo = TRUE;

            if (p_ptr->lev >= 10)
                p_ptr->painted_target = TRUE;

            if (p_ptr->lev >= 20)
                p_ptr->big_shot = TRUE;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_BOWS)
    {
        if (spec)
        {
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_CROSSBOWS)
    {
        if (spec)
        {
            switch (_get_toggle())
            {
            case TOGGLE_RAPID_RELOAD:
                p_ptr->to_a -= 30;
                p_ptr->dis_to_a -= 30;
                break;
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
    {
        if (spec)
        {
            p_ptr->easy_2weapon = TRUE;
            if (p_ptr->lev >= 25 && p_ptr->weapon_ct > 1)
            {
                p_ptr->to_a += 10 + p_ptr->lev/2;
                p_ptr->dis_to_a += 10 + p_ptr->lev/2;
            }

            p_ptr->skills.stl += p_ptr->lev/12;

            if (p_ptr->lev >= 30)
                p_ptr->sneak_attack = TRUE;

            switch (_get_toggle())
            {
            case TOGGLE_SHADOW_STANCE:
                p_ptr->shooter_info.to_d -= 10;
                p_ptr->shooter_info.dis_to_d -= 10;
                p_ptr->to_d_m -= 10;
                p_ptr->skills.stl += p_ptr->lev/12;
                break;

            case TOGGLE_FRENZY_STANCE:
                p_ptr->skills.stl -= 8;
                p_ptr->to_a -= 25;
                p_ptr->dis_to_a -= 25;
                break;
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_CLUBS)
    {
        if (spec)
        {
            if (p_ptr->lev >= 40)
                p_ptr->enhanced_crit = TRUE;

            switch (_get_toggle())
            {
            case TOGGLE_TRADE_BLOWS:
                p_ptr->to_a -= 30;
                p_ptr->dis_to_a -= 30;
                break;
            case TOGGLE_COMBAT_EXPERTISE:
                p_ptr->to_a += 5 + p_ptr->lev;
                p_ptr->dis_to_a += 5 + p_ptr->lev;
                break;
            }
            p_ptr->skill_tht += 2*p_ptr->lev;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_AXES)
    {
        if (p_ptr->tim_vicious_strike)
        {
            p_ptr->to_a -= 120;
            p_ptr->dis_to_a -= 120;
            /* AC should not go below 0 ... See xtra1.c calc_bonuses() */
        }

        if (spec)
        {
            if (p_ptr->lev >= 5)
                p_ptr->skill_dig += (5 * 20); /* As if you had a +5 digger ... */

            if (p_ptr->lev >= 30)
                p_ptr->cleave = TRUE;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SWORDS)
    {
        if (spec)
        {
            if (p_ptr->lev >= 20)
            {
                /* Hackery to keep the status bar up to date ... sigh */
                if (!IS_HERO())
                {
                    p_ptr->constant_hero = TRUE;
                    p_ptr->redraw |= (PR_STATUS);
                }
                else
                    p_ptr->constant_hero = TRUE;
            }

            if (p_ptr->lev >= 45)
                p_ptr->vorpal = TRUE;
        }
        else if (last_spec && p_ptr->lev >= 20)
        {
            p_ptr->redraw |= (PR_STATUS);
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        if (spec)
        {
            if (p_ptr->lev >= 45)
                p_ptr->whirlwind = TRUE;

            if (p_ptr->lev >= 30)
            {
                if (p_ptr->entrench_ct >= 3)
                {
                    p_ptr->entrenched = TRUE;
                    p_ptr->redraw |= PR_STATUS;

                    p_ptr->to_a += 20 + p_ptr->entrench_ct;
                    p_ptr->dis_to_a += 20 + p_ptr->entrench_ct;
                }
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
        if (spec)
        {
            if (p_ptr->lev >= 20)
                p_ptr->inven_prot = TRUE;

            if (p_ptr->lev >= 45)
            {
                p_ptr->reflect = TRUE;
            }

            /* Block: Shield AC doubled. */
            if (p_ptr->lev >= 5)
            {
                int slot;
                for (slot = equip_find_first(object_is_shield);
                        slot;
                        slot = equip_find_next(object_is_shield, slot))
                {
                    object_type *o_ptr = equip_obj(slot);
                    p_ptr->to_a += k_info[o_ptr->k_idx].ac;
                    p_ptr->dis_to_a += k_info[o_ptr->k_idx].ac;
                    p_ptr->to_a += o_ptr->to_a;
                    if (object_is_known(o_ptr))
                        p_ptr->dis_to_a += o_ptr->to_a;
                }
            }

            /* Stalwart: +20 saving throws */
            if (p_ptr->lev >= 25)
                p_ptr->skills.sav += 20;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        if (spec)
        {
            if (p_ptr->elaborate_defense)
            {
                p_ptr->to_a += 10 + p_ptr->lev*2/3;
                p_ptr->dis_to_a += 10 + p_ptr->lev*2/3;
            }

            if (p_ptr->lev >= 10)
                p_ptr->sh_retaliation = TRUE;

            if (p_ptr->lev >= 20)
                p_ptr->pspeed += 2;

            if (p_ptr->cloak_of_shadows && !p_ptr->elaborate_defense)
            {
                p_ptr->to_a += 10 + p_ptr->lev*2/3;
                p_ptr->dis_to_a += 10 + p_ptr->lev*2/3;
            }

            if (p_ptr->lev >= 35)
                p_ptr->lightning_reflexes = TRUE;
        }

        if (equip_find_first(object_is_shield))
            p_ptr->pspeed -= 5;
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        if (spec)
        {
            int pval = _max_pval();

            p_ptr->skill_dig += (5 + p_ptr->lev/5) * 20;

            if (p_ptr->lev >= 45)
                p_ptr->kill_wall = TRUE;

            if (p_ptr->lev >= 15) /* Earthen Shield */
            {
                int dir, x, y;
                int count = 0;
                int mult = 2 + p_ptr->lev/10;

                for (dir = 0; dir < 8; dir++)
                {
                    y = py + ddy_ddd[dir];
                    x = px + ddx_ddd[dir];

                    if (!in_bounds(y, x)) continue;

                    if ( cave_have_flag_bold(y, x, FF_WALL)
                      || cave[y][x].feat == feat_rubble )
                    {
                        count++;
                    }
                }
                p_ptr->to_a += mult*count;
                p_ptr->dis_to_a += mult*count;
            }
            switch (_get_toggle())
            {
            case TOGGLE_STOICISM:
                p_ptr->skills.stl += pval;
                break;
            case TOGGLE_INDUSTRIOUS_MORTICIAN:
                p_ptr->pspeed += pval;
                break;
            }
        }
    }

    if (!p_ptr->painted_target)
    {
        p_ptr->painted_target_idx = 0;
        p_ptr->painted_target_ct = 0;
    }

    /* Hack -- handle "xtra" mode
       This works around a startup glitch where the screen is only half painted.
       If we do a msg_print at this point, the user gets a face full of yuk!
    */
    if (character_xtra) return;

    /* Message about favored gear */
    if ((!init) || (spec != last_spec))
    {
        int kind = _specialities[p_ptr->psubclass].kind;

        init = TRUE;
        last_spec = spec;

        if (!character_generated) return;

        if (!spec)
        {
            switch (kind)
            {
            case _WEAPONMASTER_BOWS:
                msg_print("你觉得你的远程武器用着不顺手。");
                break;
            case _WEAPONMASTER_SHIELDS:
                msg_print("你觉得你的盾牌用着不顺手。");
                break;
            default:
                msg_print("你觉得你的武器用着不顺手。");
                break;
            }
        }
        else if (init)
        {
            switch (kind)
            {
            case _WEAPONMASTER_BOWS:
                msg_print("你非常喜欢你的远程武器。");
                break;
            case _WEAPONMASTER_SHIELDS:
                msg_print("你非常喜欢你的盾牌。");
                break;
            default:
                msg_print("你非常喜欢你的武器。");
                break;
            }
        }
    }
}

static void _calc_stats(s16b stats[MAX_STATS])
{
    /* Note: _calc_stats() gets called before _calc_bonuses, so p_ptr->speciality_equip
       won't be set yet. I suppose we could take over setting this field, but I don't like
       relying on the non-obvious ordering of callbacks */
    if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
    {
        if (_check_speciality_equip())
        {
            switch (_get_toggle())
            {
            case TOGGLE_FLYING_DAGGER_STANCE:
                stats[A_CON] -= 4;
                break;
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        if (_check_speciality_equip())
        {
            int pval = _max_pval();

            switch (_get_toggle())
            {
            case TOGGLE_STRENGTH_OF_THE_UNDERTAKER:
                stats[A_STR] += pval;
                break;
            case TOGGLE_STOICISM:
                stats[A_CON] += pval;
                break;
            }
        }
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
    {
        if (p_ptr->speciality_equip)
        {
            if (p_ptr->lev >= 10) add_flag(flgs, OF_STEALTH);
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SWORDS)
    {
        if (p_ptr->speciality_equip)
        {
            if (p_ptr->lev >= 45) add_flag(flgs, OF_VORPAL);
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
        if (p_ptr->speciality_equip)
        {
            if (p_ptr->lev >= 45)
            {
                add_flag(flgs, OF_REFLECT);
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        if (p_ptr->speciality_equip)
        {
            if (p_ptr->lev >= 10) add_flag(flgs, OF_AURA_REVENGE);
            if (p_ptr->lev >= 20) add_flag(flgs, OF_SPEED);
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        if (p_ptr->speciality_equip)
        {
            switch (_get_toggle())
            {
            case TOGGLE_STOICISM:
                add_flag(flgs, OF_STEALTH);
                break;
            case TOGGLE_INDUSTRIOUS_MORTICIAN:
                add_flag(flgs, OF_SPEED);
                break;
            }
        }
    }
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    int spec = _check_speciality_aux(o_ptr);

    if (!spec) p_ptr->shooter_info.base_shot = 100;
    if (spec && !p_ptr->shooter_info.heavy_shoot)
    {
        p_ptr->shooter_info.to_d += p_ptr->lev/5;
        p_ptr->shooter_info.dis_to_d += p_ptr->lev/5;
        p_ptr->shooter_info.to_h += p_ptr->lev/5;
        p_ptr->shooter_info.dis_to_h += p_ptr->lev/5;

        if (p_ptr->psubclass == WEAPONMASTER_CROSSBOWS)
        {
            switch (_get_toggle())
            {
            case TOGGLE_RAPID_RELOAD:
                p_ptr->shooter_info.to_h -= 10;
                p_ptr->shooter_info.dis_to_h -= 10;
                p_ptr->shooter_info.base_shot += p_ptr->shooter_info.base_shot * (10 + p_ptr->lev/3) / 100;
                break;
            case TOGGLE_EXPLODING_BOLT:
                p_ptr->shooter_info.base_shot -= p_ptr->lev;
                break;
            case TOGGLE_OVERDRAW:
                p_ptr->shooter_info.to_mult += 100;
                p_ptr->shooter_info.to_h -= 20;
                p_ptr->shooter_info.dis_to_h -= 20;
                break;
            case TOGGLE_CAREFUL_AIM:
                p_ptr->shooter_info.to_h += 20;
                p_ptr->shooter_info.dis_to_h += 20;
                p_ptr->shooter_info.base_shot -= p_ptr->lev * 2;
                break;
            }
        }
        if (p_ptr->psubclass == WEAPONMASTER_BOWS && p_ptr->lev >= 20)
            p_ptr->shooter_info.breakage -= 10;
    }
}

static void _init_blows_calc(object_type *o_ptr, weapon_info_t *info_ptr)
{
    info_ptr->blows_calc.max = 100; /* Default for melee specialties with unfavored weapons */
    info_ptr->blows_calc.wgt = 70;
    info_ptr->blows_calc.mult = 50;
    switch (_specialities[p_ptr->psubclass].kind)
    {
    case _WEAPONMASTER_BOWS:
        info_ptr->blows_calc.max = 300;
        break;

    case _WEAPONMASTER_SHIELDS: /* Shieldmaster can bash or melee with aplomb */
        info_ptr->blows_calc.max = 500;
        break;

    case _WEAPONMASTER_MELEE:
        if (_check_speciality_aux(o_ptr))
        {
            switch (p_ptr->psubclass)
            {
            case WEAPONMASTER_AXES:
                if (info_ptr->wield_how == WIELD_TWO_HANDS)
                    info_ptr->blows_calc.max = 600;
                else
                    info_ptr->blows_calc.max = 500;
                break;
            case WEAPONMASTER_DAGGERS:
                if (_get_toggle() == TOGGLE_FRENZY_STANCE)
                    info_ptr->blows_calc.max = 575;
                else
                    info_ptr->blows_calc.max = 475;
                break;
            case WEAPONMASTER_CLUBS:
                info_ptr->blows_calc.max = 525;
                break;
            case WEAPONMASTER_POLEARMS:
                info_ptr->blows_calc.max = 525;
                break;
            case WEAPONMASTER_STAVES:
                info_ptr->blows_calc.max = 500;
                break;
            case WEAPONMASTER_SWORDS:
                info_ptr->blows_calc.max = 475;
                break;
            case WEAPONMASTER_DIGGERS:
                info_ptr->blows_calc.max = 540;
                break;
            }
        }
        break;
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int spec1 = _check_speciality_aux(o_ptr);

    _init_blows_calc(o_ptr, info_ptr);

    if (_weaponmaster_object_is_icky(o_ptr)) info_ptr->icky_wield = TRUE;

    /* (+L/5, +L/5) for all weaponmasters replacing 'Signature Weapon' */
    if (spec1 && p_ptr->speciality_equip)
    {
        info_ptr->to_d += p_ptr->lev/5;
        info_ptr->dis_to_d += p_ptr->lev/5;
        info_ptr->to_h += p_ptr->lev/5;
        info_ptr->dis_to_h += p_ptr->lev/5;
    }

    if (p_ptr->psubclass == WEAPONMASTER_AXES)
    {
        if (spec1 && p_ptr->speciality_equip && (o_ptr->sval != SV_HATCHET))
        /* Axemasters can use hatchets at a pinch, but prefer large axes */
        {
            info_ptr->to_d += 5;
            info_ptr->dis_to_d += 5;

            if (p_ptr->lev >= 20)
            {
                info_ptr->to_d += 5;
                info_ptr->dis_to_d += 5;
            }

            if (p_ptr->lev >= 45)
            {
                info_ptr->to_d += 10;
                info_ptr->dis_to_d += 10;
            }

            switch (_get_toggle())
            {
            case TOGGLE_POWER_ATTACK:
                info_ptr->dis_to_h -= 2*p_ptr->lev/3;
                info_ptr->to_h -= 2*p_ptr->lev/3;
                if (info_ptr->wield_how == WIELD_TWO_HANDS && !info_ptr->omoi)
                {
                    info_ptr->dis_to_d += p_ptr->lev/2;
                    info_ptr->to_d += p_ptr->lev/2;
                }
                else
                {
                    info_ptr->dis_to_d += p_ptr->lev/4;
                    info_ptr->to_d += p_ptr->lev/4;
                }
                break;
            }
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
    {
        switch (_get_toggle())
        {
        case TOGGLE_SHADOW_STANCE:
            info_ptr->to_d -= 10;
            info_ptr->dis_to_d -= 10;
            break;
        case TOGGLE_FRENZY_STANCE:
            info_ptr->to_h += p_ptr->lev/5;
            info_ptr->dis_to_h += p_ptr->lev/5;
            break;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_CLUBS)
    {
        if (p_ptr->speciality_equip)
        {
            if (p_ptr->lev >= 25)
            {
                info_ptr->to_h += 20;
                info_ptr->dis_to_h += 20;
            }
        }

        switch (_get_toggle())
        {
        case TOGGLE_COMBAT_EXPERTISE:
            info_ptr->to_h -= 5 + p_ptr->lev/2;
            info_ptr->dis_to_h -= 5 + p_ptr->lev/2;
            break;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SWORDS)
    {
        if (spec1 && p_ptr->speciality_equip)
        {
            info_ptr->to_h += 10;
            info_ptr->dis_to_h += 10;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        if (p_ptr->entrenched)
        {
            info_ptr->to_h += 20 + p_ptr->entrench_ct;
            info_ptr->dis_to_h += 20 + p_ptr->entrench_ct;
        }

        switch (_get_toggle())
        {
        case TOGGLE_MANY_STRIKE:
            info_ptr->to_h -= 5;
            info_ptr->dis_to_h -= 5;
            break;
        case TOGGLE_TRIP:
            info_ptr->to_h -= 30;
            info_ptr->dis_to_h -= 30;
            break;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        if (spec1 && p_ptr->speciality_equip && p_ptr->lev >= 30 && p_ptr->chp == p_ptr->mhp)
            info_ptr->xtra_blow += 100;
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        if (spec1 && p_ptr->speciality_equip)
        {
            switch (_get_toggle())
            {
            case TOGGLE_INDUSTRIOUS_MORTICIAN:
                info_ptr->xtra_blow += 100;
                /*info_ptr->xtra_blow += MIN(o_ptr->pval*50, 250);*/
                break;
            }
        }
    }
}

static void _move_monster(int m_idx)
{
    if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        if ((p_ptr->lev >= 20) && (p_ptr->speciality_equip) && (!p_ptr->blind) && (!p_ptr->confused))
        {
            monster_type *m_ptr = &m_list[m_idx];
            if ((m_ptr->cdis == 1) && (is_hostile(m_ptr)))
            {
                char m_name[80];
                monster_desc(m_name, m_ptr, 0);
                msg_format("%^s靠得太近了！", m_name);
                py_attack(m_ptr->fy, m_ptr->fx, WEAPONMASTER_PROXIMITY_ALERT);
            }
        }
    }
}

static void _process_player(void)
{
    if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        /* process_player() fires before move_player() */
        if (px == p_ptr->entrench_x && py == p_ptr->entrench_y)
        {
            if (p_ptr->entrench_ct < 30)
                p_ptr->entrench_ct++;
            p_ptr->update |= PU_BONUS;
            p_ptr->redraw |= PR_STATUS;
        }
        else
        {
            p_ptr->entrench_x = px;
            p_ptr->entrench_y = py;
            p_ptr->entrench_ct = 0;
            p_ptr->update |= PU_BONUS;
            p_ptr->redraw |= PR_STATUS;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        if (p_ptr->elaborate_defense)
        {
            p_ptr->elaborate_defense--;
            if (p_ptr->elaborate_defense <= 0)
            {
                p_ptr->update |= PU_BONUS;
                p_ptr->redraw |= PR_ARMOR;
            }
        }
        if (p_ptr->cloak_of_shadows)
        {
            p_ptr->cloak_of_shadows--;
            if (p_ptr->cloak_of_shadows <= 0)
            {
                p_ptr->update |= PU_BONUS;
                p_ptr->redraw |= PR_ARMOR;
            }
        }
    }
}

static obj_ptr _relocate_ammo(obj_loc_t loc)
{
    obj_ptr ammo = NULL;
    if (loc.where == INV_QUIVER && loc.slot)
        ammo = quiver_obj(loc.slot);
    else if (loc.where == INV_PACK && loc.slot)
        ammo = pack_obj(loc.slot);
    if (ammo && obj_can_shoot(ammo))
        return ammo;
    return NULL;
}

static obj_ptr _find_ammo(void)
{
    slot_t slot = quiver_find_first(obj_can_shoot);
    if (slot)
        return quiver_obj(slot);
    slot = pack_find_first(obj_can_shoot);
    if (slot)
        return pack_obj(slot);
    return NULL;
}

static void _move_player(void)
{
    if (_get_toggle() == TOGGLE_SHOT_ON_THE_RUN)
    {
        int idx = -1;
        int num_shots = 1 + NUM_SHOTS / 400;
        obj_ptr ammo;
        int i;

        /* Paranoia:  Did the player remove their sling? */
        if (!_check_speciality_equip())
        {
            _set_toggle(TOGGLE_NONE);
            return;
        }

        ammo = _relocate_ammo(shoot_item);
        if (!ammo)
            ammo = _find_ammo();
        if (!ammo)
        {
            msg_print("你的弹药用尽了。");
            _set_toggle(TOGGLE_NONE);
            return;
        }

        for (i = 0; i < num_shots; i++)
        {
            /* End the technique when the ammo runs out.  Note that "return ammo"
               might not consume the current shot. Note that we will intentionally spill
               over into the next stack of shots, provided they are legal for this shooter. 
            if (shoot_item != INVEN_UNLIMITED_QUIVER)??? */
            if (!ammo->number)
            {
                obj_release(ammo, 0);
                ammo = _find_ammo();
                if (!ammo)
                {
                    msg_print("你的弹药用尽了。该装填了！");
                    _set_toggle(TOGGLE_NONE);
                    return;
                }
            }

            /* Pick a target to blast */
            idx = _get_nearest_target_los();
            if (idx > 0)
            {
                int     tx, ty;
                obj_ptr bow = equip_obj(p_ptr->shooter_info.slot);

                if (bow)
                {
                    tx = m_list[idx].fx;
                    ty = m_list[idx].fy;
                    shoot_hack = SHOOT_RUN;
                    do_cmd_fire_aux2(bow, ammo, px, py, tx, ty);
                    shoot_hack = SHOOT_NONE;
                }
            }
        }
        obj_release(ammo, OBJ_RELEASE_QUIET);
    }
    if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        int y, x;
        if (one_in_(5) && random_opponent(&y, &x))
        {
            py_attack(y, x, WEAPONMASTER_AUTO_BLOW);
            energy_use = 0;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        if (p_ptr->speciality_equip && p_ptr->lev >= 5)
        {
            if (!p_ptr->cloak_of_shadows)
            {
                p_ptr->update |= PU_BONUS;
                p_ptr->redraw |= PR_ARMOR;
            }
            p_ptr->cloak_of_shadows = 1;
        }
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        if (p_ptr->speciality_equip && p_ptr->lev >= 15)
            p_ptr->update |= PU_BONUS;
    }
}

static void _character_dump(doc_ptr doc)
{
    doc_printf(doc, "<topic:Abilities>================================== <color:keypress>A</color> 能力 ==================================\n\n");

    if (p_ptr->psubclass == WEAPONMASTER_AXES)
    {
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备斧头时，你获得巨额伤害加成。\n");
        else if (p_ptr->lev >= 20)
            doc_printf(doc, "* 当装备斧头时，你获得大量伤害加成。\n");
        else
            doc_printf(doc, "* 当装备斧头时，你获得少量伤害加成。\n");

        doc_printf(doc, "* 当双手装备斧头时，你获得 +1 最大攻击次数。\n");

        if (p_ptr->lev >= 5)
            doc_printf(doc, "* 当装备斧头时，你获得挖掘加成。\n");

        if (p_ptr->lev >= 30)
            doc_printf(doc, "* <indent>当装备斧头时，在击杀一名敌人后，你偶尔会顺势攻击相邻的对手。</indent>\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_CLUBS)
    {
        doc_printf(doc, "* 当装备棍棒时，你的攻击有几率使目标困惑。\n");

        if (p_ptr->lev >= 20)
            doc_printf(doc, "* 当装备棍棒时，你的攻击有几率将目标击晕。\n");

        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备棍棒时，你的攻击有几率使目标震慑。\n");

        if (p_ptr->lev >= 25)
            doc_printf(doc, "* 当装备棍棒时，你获得命中加成。\n");

        if (p_ptr->lev >= 40)
            doc_printf(doc, "* 当装备棍棒时，你的攻击附加粉碎打击。\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_CROSSBOWS)
    {
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* <indent>当使用弩射击时，你对已受伤的怪物造成额外伤害。</indent>\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
    {
        doc_printf(doc, "* 当装备匕首时，你消耗的能量减少（行动更快）。\n");
        doc_printf(doc, "* 当使用匕首时，你的双持效率极高。\n");
        if (p_ptr->lev >= 25)
            doc_printf(doc, "* 当双持匕首时，你获得护甲等级(AC)加成。\n");
        if (p_ptr->lev >= 12)
            doc_printf(doc, "* 当装备匕首时，你获得潜行加成。\n");
        if (p_ptr->lev >= 30)
            doc_printf(doc, "* 当装备匕首时，你获得偷袭和背刺能力。\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_DIGGERS)
    {
        doc_printf(doc, "* 当装备挖掘工具时，你获得挖掘加成。\n");
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备挖掘工具时，你的步伐会破坏墙壁。\n");

        if (p_ptr->lev >= 15)
            doc_printf(doc, "* <indent>当装备挖掘工具时，你根据相邻墙壁的数量获得护甲等级(AC)加成。</indent>\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_POLEARMS)
    {
        doc_printf(doc, "* <indent>当装备长柄武器时，你在移动后偶尔会获得一轮免费攻击。</indent>\n");
        if (p_ptr->lev >= 20)
            doc_printf(doc, "* <indent>当装备长柄武器时，你会自动攻击任何走到你身旁的敌人。</indent>\n");
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备长柄武器时，你偶尔会同时打击所有相邻的敌人。\n");

        if (p_ptr->lev >= 30)
            doc_printf(doc, "* <indent>当装备长柄武器时，如果你连续 3 回合不移动，你将获得命中和护甲等级(AC)加成。</indent>\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SLINGS)
    {
        doc_printf(doc, "* 当使用弹弓时，你的弹药经常会返回给你。\n");
        if (p_ptr->lev >= 20)
            doc_printf(doc, "* 当使用弹弓时，你的弹药获得额外的伤害骰。\n");

        if (p_ptr->lev >= 10)
            doc_printf(doc, "* <indent>当使用弹弓时，一旦你连续命中 3 次，你的射击将绝对不会落空。</indent>\n");
        if (p_ptr->lev >= 40)
            doc_printf(doc, "* 当使用弹弓时，你获得额外的射击次数。\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SHIELDS)
    {
        doc_printf(doc, "* 即使装备了盾牌，你也能获得双手挥舞的加成。\n");
        if (p_ptr->lev >= 20)
            doc_printf(doc, "* <indent>当装备盾牌时，你物品栏中的物品能在一定程度上免受破坏。</indent>\n");
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备盾牌时，你获得反弹能力。\n");

        if (p_ptr->lev >= 5)
            doc_printf(doc, "* 当装备盾牌时，你获得双倍的护甲等级(AC)收益。\n");

        if (p_ptr->lev >= 25)
            doc_printf(doc, "* 当装备盾牌时，你获得豁免检定加成。\n");

    }
    else if (p_ptr->psubclass == WEAPONMASTER_STAVES)
    {
        doc_printf(doc, "* <indent>当装备法杖时，在任何成功的击中后，你将获得护甲等级(AC)加成直到你的下个回合。</indent>\n");
        doc_printf(doc, "* 当装备盾牌时，你会受到速度惩罚。\n");
        if (p_ptr->lev >= 5)
            doc_printf(doc, "* <indent>当装备法杖时，移动后你将获得护甲等级(AC)加成直到你的下个回合。</indent>\n");
        if (p_ptr->lev >= 10)
            doc_printf(doc, "* 当装备法杖时，你在被击中后会进行反击。\n");
        if (p_ptr->lev >= 20)
            doc_printf(doc, "* 当装备法杖时，你获得速度加成。\n");
        if (p_ptr->lev >= 30)
            doc_printf(doc, "* 当你满血且装备法杖时，你获得一次额外攻击。\n");
        if (p_ptr->lev >= 35)
            doc_printf(doc, "* 当装备法杖时，你不受怪物光环的影响。\n");
    }
    else if (p_ptr->psubclass == WEAPONMASTER_SWORDS)
    {
        doc_printf(doc, "* 当装备剑时，你获得命中加成。\n");
        if (p_ptr->lev >= 20)
            doc_printf(doc, "* 当装备剑时，你获得常驻的英雄气概。\n");
        if (p_ptr->lev >= 45)
            doc_printf(doc, "* 当装备剑时，你的攻击附加斩首(Vorpal)效果。\n");
    }

    doc_newline(doc);

    py_dump_spells(doc);
}

class_t *weaponmaster_get_class(int subclass)
{
    static class_t me = {0};
    static bool    init = FALSE;

    /* static info never changes */
    if (!init)
    {
        me.name = "武器大师";
        me.desc = "武器大师精通单一类别的武器，能够根据其选择的专精获得特殊的战斗加成和力量。专注是成功的关键；武器大师在使用非专精武器时真的非常糟糕，但很少有人能比得上他们在手中拿着合适武器时所展现出的威力。";

        me.life = 105;
        me.base_hp = 12;
        me.exp = 135;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.birth = _on_birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_stats = _calc_stats;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        me.move_player = _move_player;
        me.move_monster = _move_monster;
        me.process_player = _process_player;
        me.character_dump = _character_dump;
        me.known_icky_object = _weaponmaster_object_is_icky;
        init = TRUE;
    }
    {
        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  0;
    }
    if (0 <= subclass && subclass < WEAPONMASTER_MAX)
    {
        _speciality *ptr = &_specialities[subclass];
        int          i;

        for (i = 0; i < MAX_STATS; i++)
            me.stats[i] += ptr->stats[i];

        me.base_skills = ptr->base_skills;
        me.extra_skills = ptr->extra_skills;

        me.subname = ptr->name;
        me.subdesc = ptr->help;
        me.life = ((ptr->kind == _WEAPONMASTER_BOWS) ? 102 : 105);
    }
    else
    {
        me.subname = "";
        me.subdesc = "";
    }

    return &me;
}

int weaponmaster_wield_hack(object_type *o_ptr)
{
    int result = 100;

    if (p_ptr->pclass == CLASS_WEAPONMASTER)
    {
        if (p_ptr->psubclass == WEAPONMASTER_DAGGERS)
        {
            if (_check_speciality_aux(o_ptr))
                result = 50 - p_ptr->lev/2;
        }
    }
    return result;
}

cptr weaponmaster_speciality_name(int psubclass)
{
    /* assert(p_ptr->pclass == CLASS_WEAPONMASTER); */
    return _specialities[psubclass].name;
}

cptr weaponmaster_internal_speciality_name(int psubclass)
{
    switch (psubclass)
    {
    case WEAPONMASTER_AXES: return "Axes";
    case WEAPONMASTER_BOWS: return "Bows";
    case WEAPONMASTER_CLUBS: return "Clubs";
    case WEAPONMASTER_CROSSBOWS: return "Crossbows";
    case WEAPONMASTER_DAGGERS: return "Daggers";
    case WEAPONMASTER_POLEARMS: return "Polearms";
    case WEAPONMASTER_SHIELDS: return "Shields";
    case WEAPONMASTER_SLINGS: return "Slings";
    case WEAPONMASTER_STAVES: return "Staves";
    case WEAPONMASTER_SWORDS: return "Swords";
    case WEAPONMASTER_DIGGERS: return "Diggers";
    }
    return "";
}

void weaponmaster_do_readied_shot(monster_type *m_ptr)
{
    if ((!m_ptr) || (m_ptr->hp < 0)) return; /* exploders */
    if (weaponmaster_get_toggle() == TOGGLE_READIED_SHOT)
    {
        int tx = m_ptr->fx;
        int ty = m_ptr->fy;
        obj_ptr bow = equip_obj(p_ptr->shooter_info.slot);

        if (bow)
        {
            obj_ptr ammo = _relocate_ammo(shoot_item);
            /* Probably, the player should ready another? Also, 
             * do_cmd_fire should clear the toggle? There is no
             * game concept of a "loaded arrow", but ideally, the
             * object would be copied out at time of casting and
             * stored someplace else ... then do_cmd_fire would simply
             * use the loaded arrow (rather than prompting) with
             * less energy ... but that seems too complicated to implement.
            if (!ammo)
                ammo = _find_ammo();*/
            if (!ammo)
            {
                msg_print("你准备好的弹药已不存在了！");
                _set_toggle(TOGGLE_NONE);
                return;
            }
            shoot_hack = SHOOT_RETALIATE;
            do_cmd_fire_aux2(bow, ammo, px, py, tx, ty);
            obj_release(ammo, 0);
            shoot_hack = SHOOT_NONE;
        }
        /* Force the player to ready another arrow ... */
        _set_toggle(TOGGLE_NONE);
    }
}
