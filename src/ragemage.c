#include "angband.h"

static bool _unclear_mind = TRUE;

void rage_mage_rage_fueled(int dam)
{
    int x = dam;
    int y = p_ptr->chp;
    int sp = x*(p_ptr->mhp*3/2 - y)/p_ptr->mhp;

    if (sp < 1)
        sp = 1;

    p_ptr->csp += sp;
    if (p_ptr->csp > p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    p_ptr->redraw |= PR_MANA;

    /*_unclear_mind = FALSE;*/
}

void rage_mage_blood_lust(int dam)
{
    int sp;
    if (p_ptr->shero)
        sp = dam/8;
    else
        sp = dam/12;

    if (sp < 1)
        sp = 1;

    p_ptr->csp += sp;
    if (p_ptr->csp > p_ptr->msp)
    {
        p_ptr->csp = p_ptr->msp;
        p_ptr->csp_frac = 0;
    }
    p_ptr->redraw |= PR_MANA;

    _unclear_mind = FALSE;
}

static void _anti_magic_ray_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "反魔法射线");
        break;
    case SPELL_DESC:
        var_set_string(res, "封锁选定敌人的法术。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ANTIMAGIC, dir, 1, 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _armor_of_fury_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂怒之甲");
        break;
    case SPELL_DESC:
        var_set_string(res, "每当怪物用魔法攻击你时，它们可能会被减速并被震慑。");
        break;
    case SPELL_CAST:
        set_tim_armor_of_fury(25 + randint1(25), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _barbarian_lore_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "野蛮人传说");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

static void _barbaric_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "野蛮抗性");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予免受元素伤害的临时保护。");
        break;
    case SPELL_CAST:
    {
        int base = 10;
        int dur;

        if (p_ptr->shero)
            base = 20;

        dur = randint1(base) + base;
        set_oppose_acid(dur, FALSE);
        set_oppose_elec(dur, FALSE);
        set_oppose_fire(dur, FALSE);
        set_oppose_cold(dur, FALSE);
        set_oppose_pois(dur, FALSE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _crude_mapping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "粗略绘图");
        break;
    case SPELL_DESC:
        var_set_string(res, "绘制你附近的地牢地图。");
        break;
    case SPELL_CAST:
        map_area(DETECT_RAD_DEFAULT); /* Was 14, but that was just plain annoying! */
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _detect_objects_ego(int range)
{
    int i, y, x;

    bool detect = FALSE;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS) range /= 3;

    /* Scan all objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        if (!o_ptr->k_idx) continue;
        if (o_ptr->held_m_idx) continue;

        y = o_ptr->loc.y;
        x = o_ptr->loc.x;

        if (distance(py, px, y, x) > range) continue;

        if (object_is_artifact(o_ptr) ||
            object_is_ego(o_ptr) )
        {
            o_ptr->marked |= OM_FOUND;
            p_ptr->window |= PW_OBJECT_LIST;
            lite_spot(y, x);
            detect = TRUE;
        }
    }

    if (detect)
        msg_print("你感应到了魔法物品的存在！");

    return detect;
}

static void _detect_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测魔法");
        break;
    case SPELL_DESC:
        var_set_string(res, "探测附近的魔法使用者和物品。");
        break;
    case SPELL_CAST:
        detect_monsters_magical(DETECT_RAD_DEFAULT);
        _detect_objects_ego(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_magical_foes_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测魔法敌人");
        break;
    case SPELL_DESC:
        var_set_string(res, "探测附近的魔法使用者。");
        break;
    case SPELL_CAST:
        detect_monsters_magical(DETECT_RAD_DEFAULT);
        if (p_ptr->shero)
            set_tim_esp_magical(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _evasive_leap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "规避跳跃");
        break;
    case SPELL_ENERGY:
        if (p_ptr->shero)
        {
            var_set_int(res, 30);
            break;
        }
    default:
        strafing_spell(cmd, res);
        break;
    }
}

static void _focus_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "专注狂怒");
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害你自己并恢复法力值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 10 + p_ptr->lev/2));
        break;
    case SPELL_FAIL:
    {
        int hp = 10 + p_ptr->lev/2;
        take_hit(DAMAGE_NOESCAPE, hp, "狂怒");
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_CAST:
    {
        int hp = 10 + p_ptr->lev/2;

        var_set_bool(res, FALSE);

        if (p_ptr->chp < hp)
        {
            if (!get_check("真的吗？这会杀死你！")) return;
        }

        take_hit(DAMAGE_NOESCAPE, hp, "狂怒");
        sp_player(hp);

        _unclear_mind = FALSE; /* Hack to avoid automatic mana drain for this action */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _force_brand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "原力附魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时为你的武器附魔上原力。");
        break;
    case SPELL_CAST:
    {
        int base = 4;
        if (p_ptr->shero)
            base = 10;
        set_tim_force(base + randint1(base), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _greater_focus_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "专注*狂怒*");
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害你自己并恢复法力值。");
        break;
    case SPELL_INFO:
        if (p_ptr->shero)
            var_set_string(res, info_damage(0, 0, 2 * p_ptr->lev));
        else
            var_set_string(res, info_damage(0, 0, 10 + p_ptr->lev));
        break;
    case SPELL_FAIL:
    {
        int hp = 10 + p_ptr->lev;
        if (p_ptr->shero)
            hp = 2 * p_ptr->lev;
        take_hit(DAMAGE_NOESCAPE, hp, "狂怒");
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_CAST:
    {
        int hp = 10 + p_ptr->lev;

        var_set_bool(res, FALSE);

        if (p_ptr->shero)
            hp = 2 * p_ptr->lev;

        if (p_ptr->chp < hp)
        {
            if (!get_check("真的吗？这会杀死你！")) return;
        }

        take_hit(DAMAGE_NOESCAPE, hp, "狂怒");
        sp_player(hp * 2);

        _unclear_mind = FALSE; /* Hack to avoid automatic mana drain for this action */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _greater_shout_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "高等怒吼");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人投射一个锥形的声波。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(p_ptr->lev - 10, 8, 0));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SOUND, dir, damroll(p_ptr->lev - 10, 8), -3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_clash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力碰撞");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定目标发射一个能量球。只有施法者才会受到伤害。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_MANA_CLASH, dir, 18 * p_ptr->lev, 2); /* dam = dam * spell_freq / 100 in spells1.c */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _rage_strike_dam(void)
{
    int sp = p_ptr->csp;
    int z = sp*sp/100;
    return 1200*z/(1000+z);
}

static void _rage_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂怒打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人发射一个纯粹由狂怒构成的能量球，用你所有的一切进行打击！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _rage_strike_dam()));
        break;
    case SPELL_FAIL:
        sp_player(-p_ptr->csp);
        var_set_bool(res, TRUE);
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);

        if (p_ptr->chp < 100)
        {
            if (!get_check("真的吗？这会杀死你！")) return;
        }

        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_MISSILE, dir, _rage_strike_dam(), 0);
        take_hit(DAMAGE_NOESCAPE, 100, "狂怒");
        if (!p_ptr->shero)
            set_stun(99, FALSE); /* 100 is Knocked Out */

        sp_player(-p_ptr->csp); /* Don't use SPELL_COST_EXTRA since we pay mana up front these days! */
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rage_sustenance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂怒维持");
        break;
    default:
        satisfy_hunger_spell(cmd, res);
        break;
    }
}

static void _resist_curses_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗诅咒");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予临时的魔法抗性。");
        break;
    case SPELL_CAST:
        set_tim_resist_curses(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _resist_disenchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抵抗解除附魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予临时的抵抗解除附魔能力。");
        break;
    case SPELL_CAST:
        set_tim_res_disenchantment(10 + randint1(10), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _object_dam_type(object_type *o_ptr)
{
    switch (o_ptr->activation.type)
    {
    case EFFECT_BEAM_ACID:
    case EFFECT_BALL_ACID:
    case EFFECT_BOLT_ACID:
        return GF_ACID;

    case EFFECT_BEAM_ELEC:
    case EFFECT_BALL_ELEC:
    case EFFECT_BOLT_ELEC:
        return GF_ELEC;

    case EFFECT_BEAM_FIRE:
    case EFFECT_BREATHE_FIRE:
    case EFFECT_BOLT_PLASMA:
    case EFFECT_BALL_FIRE:
    case EFFECT_BOLT_FIRE:
        return GF_FIRE;

    case EFFECT_BEAM_COLD:
    case EFFECT_BREATHE_COLD:
    case EFFECT_BOLT_ICE:
    case EFFECT_BALL_COLD:
    case EFFECT_BOLT_COLD:
        return GF_COLD;

    case EFFECT_BALL_POIS:
        return GF_POIS;

    case EFFECT_BREATHE_ONE_MULTIHUED:
    {
        switch (randint1(5))
        {
        case 1: return GF_ACID;
        case 2: return GF_ELEC;
        case 3: return GF_FIRE;
        case 4: return GF_COLD;
        case 5: return GF_POIS;
        }
    }

    case EFFECT_CONFUSE_MONSTERS:
    case EFFECT_CONFUSING_LITE:
        return GF_CONFUSION;

    case EFFECT_STARBURST:
    case EFFECT_STARLITE:
    case EFFECT_BALL_LITE:
    case EFFECT_BEAM_LITE:
    case EFFECT_LITE_AREA:
    case EFFECT_BEAM_LITE_WEAK:
        return GF_LITE;

    case EFFECT_DARKNESS:
    case EFFECT_DARKNESS_STORM:
        return GF_DARK;

    case EFFECT_BALL_NETHER:
        return GF_NETHER;

    case EFFECT_BALL_NEXUS:
        return GF_NEXUS;

    case EFFECT_BALL_SOUND:
    case EFFECT_BEAM_SOUND:
        return GF_SOUND;

    case EFFECT_BALL_SHARDS:
        return GF_SHARDS;

    case EFFECT_BALL_CHAOS:
    case EFFECT_BEAM_CHAOS:
        return GF_CHAOS;

    case EFFECT_BALL_DISEN:
        return GF_DISENCHANT;

    case EFFECT_BEAM_GRAVITY:
        return GF_GRAVITY;

    case EFFECT_BEAM_DISINTEGRATE:
    case EFFECT_BALL_DISINTEGRATE:
        return GF_DISINTEGRATE;

    case EFFECT_ROCKET:
        return GF_ROCKET;

    case EFFECT_SPEED:
    case EFFECT_SLOWNESS:
    case EFFECT_HASTE_MONSTERS:
    case EFFECT_SLOW_MONSTERS:
        return GF_INERT;

    case EFFECT_HOLINESS:
        return GF_HOLY_FIRE;
    }

    return GF_MANA;
}

static void _shatter_device_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "粉碎装置");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁你物品栏中的一个魔法装置以获得各种效果。");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};

        var_set_bool(res, FALSE);

        prompt.prompt = "粉碎哪个装置？";
        prompt.error = "你没有什么可以粉碎的。";
        prompt.filter = object_is_device;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj) return;

        var_set_bool(res, TRUE);

        if (prompt.obj->activation.type == EFFECT_NONE)
        {
            msg_print("什么也没有发生。");
        }
        else if (prompt.obj->activation.type == EFFECT_DESTRUCTION)
        {
            if (destroy_area(py, px, 15 + p_ptr->lev + randint0(11), 4 * p_ptr->lev))
                msg_print("地牢坍塌了……");
            else
                msg_print("地牢在颤抖。");
        }
        else if ( prompt.obj->activation.type == EFFECT_HEAL_CURING
               || prompt.obj->activation.type == EFFECT_HEAL_CURING_HERO
               || prompt.obj->activation.type == EFFECT_RESTORING )
        {
            msg_print("你感觉生命流过你的身体！");
            restore_level();
            lp_player(1000);
            (void)set_poisoned(0, TRUE);
            (void)set_blind(0, TRUE);
            (void)set_confused(0, TRUE);
            (void)set_image(0, TRUE);
            (void)set_stun(0, TRUE);
            (void)set_cut(0, TRUE);
            (void)do_res_stat(A_STR);
            (void)do_res_stat(A_CON);
            (void)do_res_stat(A_DEX);
            (void)do_res_stat(A_WIS);
            (void)do_res_stat(A_INT);
            (void)do_res_stat(A_CHR);
            update_stuff(); /* hp may change if Con was drained ... */
            hp_player(5000);
        }
        else if ( prompt.obj->activation.type == EFFECT_TELEPORT_AWAY
               || prompt.obj->activation.type == EFFECT_BANISH_EVIL
               || prompt.obj->activation.type == EFFECT_BANISH_ALL )
        {
            banish_monsters(p_ptr->lev * 4);
        }
        else
        {
            project(0, 5, py, px,
                prompt.obj->activation.difficulty * 16,
                _object_dam_type(prompt.obj),
                PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
        }
        obj_zero(prompt.obj);
        obj_release(prompt.obj, 0);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shout_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "怒吼");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的敌人投射一个锥形的声波。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3 + (p_ptr->lev-1)/5, 4, 0));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SOUND, dir, damroll(3 + (p_ptr->lev-1)/5, 4), -2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "猛击");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁相邻的门、陷阱或墙壁。");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (!in_bounds(y, x)) return;

        if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            p_ptr->update |= PU_FLOW;
        }
        else
        {
            int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
            project(0, 0, y, x, 0, GF_KILL_DOOR, flg);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spell_reaction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法术反击");
        break;
    case SPELL_DESC:
        var_set_string(res, "每当你成为魔法攻击的目标时，赋予临时的速度加成。");
        break;
    case SPELL_CAST:
        set_tim_spell_reaction(30 + randint1(30), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _spell_turning_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法术反转");
        break;
    case SPELL_DESC:
        var_set_string(res, "每当你成为魔法的目标时，都有几率将法术反弹给施法者。");
        break;
    case SPELL_CAST:
        set_tim_spell_turning(20 + randint1(20), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _dispel_curse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散诅咒");
        break;
    case SPELL_DESC:
        var_set_string(res, "驱散你装备上的任何弱诅咒，并有几率驱散强诅咒。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "驱散你装备上所有的弱诅咒，并有三分之一的几率驱散重度诅咒。");
        break;
    case SPELL_CAST:
        if (one_in_(3) ? remove_all_curse() : remove_curse()) msg_print("你驱散了邪恶魔法！");
        else msg_print("什么也没有发生。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _summon_horde_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤部落");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤黎明战士来协助。");
        break;
    case SPELL_CAST:
    {
        int num = 3 + randint1(3);
        int mode = PM_FORCE_PET;
        int i;

        if (p_ptr->shero)
            mode |= PM_HASTE;

        for (i = 0; i < num; i++)
        {
            summon_named_creature(-1, py, px, MON_DAWN, mode);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _veterans_blessing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "老兵祝福");
        break;
    default:
        heroism_spell(cmd, res);
        break;
    }
}

static void _whirlwind_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "旋风攻击");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

/* The Rage Mage uses spellbooks to learn spells
   like other magic classes. However, learning a
   spell destroys the book, and casting a spell
   does not require the book (cf The Samurai).
   Rage is a class specific realm.
*/
#define _SPELLS_PER_BOOK 8

typedef struct {
    cptr name;
    spell_info spells[_SPELLS_PER_BOOK];
} book_t;

static book_t _books[4] = {
    { "情绪管理",
        {{ 1,  2, 30, _shout_spell},
         { 2,  2, 25, _detect_magical_foes_spell},
         { 3,  3, 30, _smash_spell},
         { 5,  5, 25, _evasive_leap_spell},
         { 5,  5, 35, light_area_spell},
         { 7,  0, 50, _focus_rage_spell},
         { 8, 10, 50, _rage_sustenance_spell},
         {12,  6, 35, _veterans_blessing_spell}}
    },
    { "北方惊魂",
        {{15,  8, 45, _crude_mapping_spell},
         {18, 18, 50, _resist_disenchantment_spell},
         {20, 30, 55, awesome_blow_spell},
         {22, 15, 60, _spell_reaction_spell},
         {23, 21, 60, _greater_shout_spell},
         {25, 18, 60, _whirlwind_attack_spell},
         {27, 20, 55, _resist_curses_spell},
         {28, 23, 70, _detect_magic_spell}}
    },
    { "喧哗与骚动",
        {{10, 12, 35, berserk_spell},
         {25, 16, 60, sterility_spell},
         {26, 20, 80, _barbaric_resistance_spell},
         {28, 22, 55, _summon_horde_spell},
         {32, 28, 75, _armor_of_fury_spell},
         {35, 55, 70, _force_brand_spell},
         {38, 30, 50, dispel_magic_spell},
         {40, 60, 85, _mana_clash_spell}}
    },
    { "极度愤怒",
        {{30, 25, 75, _barbarian_lore_spell},
         {32, 15, 65, earthquake_spell},
         {35,  0, 90, _greater_focus_rage_spell},
         {38, 60, 95, _spell_turning_spell},
         {40, 55, 80, _shatter_device_spell},
         {42, 50, 75, _dispel_curse_spell},
         {43, 70, 80, _anti_magic_ray_spell},
         {47,  0, 80, _rage_strike_spell}}
    },
};

static int _spell_index(int book, int spell)
{
    return book * _SPELLS_PER_BOOK + spell;
}

static bool _is_spell_known(int book, int spell)
{
    int idx = _spell_index(book, spell);
    if (p_ptr->spell_learned1 & (1L << idx)) return TRUE;
    return FALSE;
}

static void _learn_spell(int book, int spell)
{
    int idx = _spell_index(book, spell);
    int i;

    p_ptr->spell_learned1 |= (1L << idx);

    /* Find the next open entry in "p_ptr->spell_order[]" */
    for (i = 0; i < 64; i++)
    {
        /* Stop at the first empty space */
        if (p_ptr->spell_order[i] == 99) break;
    }

    /* Add the spell to the known list */
    p_ptr->spell_order[i++] = spell;
    p_ptr->learned_spells++;
    p_ptr->update |= PU_SPELLS;
    p_ptr->redraw |= PR_EFFECTS;

    msg_format("你学会了“%s”的技艺。", get_spell_name(_books[book].spells[spell].fn));
}

static bool _gain_spell(int book)
{
    power_info spells[_SPELLS_PER_BOOK];
    int        indices[_SPELLS_PER_BOOK];
    int        which;
    int        ct = 0, lct = 0, i;

    /* Build a list of learnable spells. Spells can only be
       learned once (no spell skills) */
    for (i = 0; i < _SPELLS_PER_BOOK; i++)
    {
        spell_info *src = &_books[book].spells[i];

        if (!_is_spell_known(book, i))
        {
            power_info *dest = &spells[ct];

            dest->spell.level = src->level;
            dest->spell.cost = src->cost;
            dest->spell.fail = calculate_fail_rate(
                src->level,
                src->fail,
                p_ptr->stat_ind[A_STR]
            );
            dest->spell.fn = src->fn;
            dest->stat = A_STR;
            indices[ct] = i;

            ct++;
            if (src->level <= p_ptr->lev) lct++;
        }
    }

    if (lct == 0)
    {
        if (ct) msg_print("你现在无法从那本书中学习任何技艺。");
        else msg_print("你已经学会了那本书中描述的所有技艺。");
        return FALSE;
    }    

    while (1)
    {
        which = choose_spell(spells, ct, "学习", "rage", 1000, FALSE);
        if ((which >= 0) && (which < ct))
        {
            if (spells[which].spell.level > p_ptr->lev)
            {
            }
            else
            {
                _learn_spell(book, indices[which]);
                return TRUE;
            }
        }
        else return FALSE;
    }

}

static bool _is_rage_book(obj_ptr obj) { return obj->tval == TV_RAGE_BOOK; }

void rage_mage_gain_spell(void)
{
    obj_prompt_t prompt = {0};

    if (p_ptr->blind || no_lite())
    {
        msg_print("你看不见！");
        return;
    }
    if (p_ptr->confused)
    {
        msg_print("你太困惑了！");
        return;
    }
    if (!p_ptr->new_spells)
    {
        msg_print("你不能学习任何新技艺！");
        return;
    }

    prompt.prompt = "学习哪本书？";
    prompt.error = "你没有可以阅读的书。";
    prompt.filter = _is_rage_book;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    if (_gain_spell(prompt.obj->sval))
    {
        char o_name[MAX_NLEN];

        object_desc(o_name, prompt.obj, OD_SINGULAR);

        msg_format("<color:%c>%^s</color> 被摧毁了。", tval_to_attr_char(prompt.obj->tval), o_name);
        prompt.obj->number--;
        obj_release(prompt.obj, 0);

        energy_use = 100;
    }
}

void rage_mage_browse_spell(void)
{
    /* TODO: Perhaps browse should display contents of rage
       spellbooks in inventory rather than already known spells? */
    do_cmd_spell_browse();
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "rage";
        me.encumbrance.max_wgt = 1000;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        init = TRUE;
    }
    return &me;
}

static void _player_action(int energy_use)
{
    /* Unclear Mind */
    if (_unclear_mind)    /* Hack for Focus Rage spell to bypass sp loss for one action */
    {
        int loss;
        loss = p_ptr->csp/8 + p_ptr->lev/10 + 1;
        loss = loss * energy_use / 100; /* Prorata normal action energy */

        p_ptr->csp -= loss;
        if (p_ptr->csp < 0)
        {
            p_ptr->csp = 0;
            p_ptr->csp_frac = 0;
        }
        p_ptr->redraw |= PR_MANA;
    }
    else
        _unclear_mind = TRUE; /* Resume normal sp loss */
}

static void _calc_bonuses(void)
{
    int squish = 5 + py_prorata_level(55);
    p_ptr->spell_cap += 3;

    /* Squishy */
    p_ptr->to_a -= squish;
    p_ptr->dis_to_a -= squish;

    if (p_ptr->tim_resist_curses)
    {
        p_ptr->skills.sav += 20;
        if (p_ptr->shero)
            p_ptr->skills.sav += 20;
    }
}

static int _get_spells_imp(spell_info* spells, int book, bool total_skip)
{
    int ct = 0, skip = 0, max = MAX_SPELLS, i;
    for (i = 0; i < _SPELLS_PER_BOOK; i++)
    {
        spell_info *src;
        spell_info *dest;

        if (ct >= max) break;
        src = &_books[book].spells[i];

        if ((!total_skip) || (_is_spell_known(book, i)))
        {
            dest = &spells[ct++];
            dest->level = src->level;
            dest->cost = src->cost;
            dest->fail = calculate_fail_rate(src->level, src->fail, p_ptr->stat_ind[A_STR]);
            dest->fn = src->fn;
            if (!_is_spell_known(book, i)) 
            {
                dest->level = 99;
                skip++;
            }
        }
    }
    return (ct - skip);
}

static void _book_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, _books[which].name);
        break;
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static spell_info *_get_spells(void)
{
    int idx = -1;
    int ct = 0;
    menu_t menu = { "使用哪一类？", NULL, NULL,
                    _book_menu_fn, _books, 4, 0 };
    static spell_info spells[10];

    idx = menu_choose(&menu);
    if (idx < 0) return NULL;

    ct = _get_spells_imp(spells, idx, FALSE);
    if (ct == 0)
    {
        msg_print("你还不懂其中的任何技艺！");
        return NULL;
    }
    spells[8].fn = NULL;
    return spells;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_SPELL_CAP);
}

static void _character_dump(doc_ptr doc)
{
    power_info powers[MAX_SPELLS];
    spell_info spells[MAX_SPELLS];
    int        ct = 0, i;

    for (i = 0; i < 4; i++)
        ct += _get_spells_imp(spells + ct, i, TRUE);

    if (!ct) return;
    spells[ct].fn = NULL;

    ct = get_spells_aux(powers, MAX_SPELLS, spells, TRUE);

    py_display_spells(doc, powers, ct);
}


static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *rage_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  20,  40,  -1,  12,   2,  50,  30 };
    skills_t xs = {  7,   8,  15,   0,   0,   0,  15,  15 };

        me.name = "狂怒法师";
        me.desc = "狂怒法师是一个秘密教派的一部分，最初由野蛮人为了对抗他们的天敌——魔法师而创立。随着时间的推移，其他种族也开始研究这些技艺。\n\n狂怒技艺是从书中学习的，但在许多方面与普通法术不同。首先，狂怒法师必须执行一种特殊的“愤怒仪式”来学习力量，而这个仪式会摧毁书籍；因此，狂怒法师可能需要很长时间才能学会他们所有的高级力量。书籍仅用于学习；力量一旦学会，就可以在没有书的情况下使用。\n\n狂怒法师的另一个独特之处在于他们的法力池。与普通施法者不同，狂怒法师的法力不会自行恢复；事实上，它实际上会在每个回合迅速减少，这意味着狂怒法师最好在还能使用力量时迅速使用它们。每当狂怒法师成为魔法法术的目标时，他就会获得法力；的确，魔法会让狂怒法师非常愤怒！狂怒法师还可以通过伤害周围的人来补充法力，这在拥挤的情况下会非常有效。";
        me.stats[A_STR] =  3;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] = -2;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 6;
        me.exp = 150;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_spells_fn = _get_spells;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.player_action = _player_action;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}
