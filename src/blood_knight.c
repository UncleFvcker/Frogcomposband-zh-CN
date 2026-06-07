#include "angband.h"

void _blood_flow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "血液流淌");
        break;
    case SPELL_DESC:
        var_set_string(res, "割伤你自己。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "赋予玩家轻微流血(10)状态，或者增加当前流血状态的20%，取两者中较大者。");
        break;
    case SPELL_CAST:
    {
        int cut = p_ptr->cut;
        cut += cut/5;
        if (cut < CUT_LIGHT)
            cut = CUT_LIGHT;

        set_cut(cut, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_sight_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血视觉");
        break;
    case SPELL_DESC:
        var_set_string(res, "探测附近的活物。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "探测附近的活物。在30级时，提供暂时的活物心灵感应(ESP)，持续 30+d30 回合。");
        break;
    case SPELL_CAST:
    {
        if (p_ptr->lev < 30)
            detect_monsters_living(DETECT_RAD_DEFAULT, "You sense potential blood!");
        else
            set_tim_blood_sight(randint1(30) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_spray_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血喷涌");
        break;
    case SPELL_DESC:
        var_set_string(res, "割伤你自己，将鲜血溅在附近的敌人身上。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "以玩家为中心生成半径为3的鲜血球，造成 2*(3d5+L+L/4) 点伤害。在30级时，半径增加至4。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, 5, p_ptr->lev + p_ptr->lev/4));
        break;
    case SPELL_CAST:
    {
        int dice = 3;
        int sides = 5;
        int rad = (p_ptr->lev < 30) ? 3 : 4;
        int base = p_ptr->lev + p_ptr->lev/4;

        project(0, rad, py, px, 2*(damroll(dice, sides) + base), GF_BLOOD, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_bath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血沐浴");
        break;
    case SPELL_DESC:
        var_set_string(res, "恢复体质并减少中毒。");
        break;
    case SPELL_CAST:
    {
        bool chg = FALSE;
        if (do_res_stat(A_CON)) chg = TRUE;
        if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE)) chg = TRUE;
        if (!chg) msg_print("你现在还不需要沐浴。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_shield_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血护盾");
        break;
    case SPELL_DESC:
        var_set_string(res, "根据你的受伤程度提供护甲等级(AC)加成。如果你伤得很重，会赋予反射效果。");
        break;

    case SPELL_SPOIL_DESC:
        var_set_string(res, "玩家获得 100*(MHP-CHP)/MHP 的护甲等级(AC)加成。如果受伤超过60%，玩家还会获得反射。");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_shield(randint1(20) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_seeking_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寻血");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予你的武器“戮杀活物”属性。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "在 30+d30 回合内，玩家的武器将获得戮杀活物(x2)属性。");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_seek(randint1(30) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血狂怒");
        break;
    case SPELL_DESC:
        var_set_string(res, "进入鲜血狂乱。提供速度，以及命中和伤害的巨额加成。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "在 L/2+d(L/2) 回合内，玩家将获得加速并进入狂暴状态。");
        break;
    case SPELL_CAST:
    {
        int dur = randint1(p_ptr->lev/2) + p_ptr->lev/2;
        set_fast(dur, FALSE);
        set_shero(dur, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_feast_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血盛宴");
        break;
    case SPELL_DESC:
        var_set_string(res, "你开始享用对手的鲜血，造成额外伤害，但会消耗你自己的生命值。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "在 25+d25 回合内，每次近战攻击造成 +35 伤害，但玩家每次攻击会受到 15 点伤害。");
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (p_ptr->tim_blood_feast)
        {
            if (!get_check("取消鲜血盛宴吗？")) return;
            set_tim_blood_feast(0, TRUE);
        }
        else
        {
            set_tim_blood_feast(randint1(25) + 25, FALSE);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_revenge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血复仇");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予血腥复仇光环。怪物会根据你的流血状态受到伤害。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "在 5+d5 回合内，任何对玩家造成 X 点近战伤害的敌人将受到 X*C/100 的复仇伤害，其中 C 是玩家当前的流血状态数值。然而，这种反击伤害被限制在每次攻击 C/10 到 50 点之间。");
        break;
    case SPELL_CAST:
    {
        set_tim_blood_revenge(randint1(5) + 5, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _is_blood_potion(obj_ptr obj)
    { return obj->tval == TV_POTION && obj->sval == SV_POTION_BLOOD; }
static int _count_blood_potions(void)
    { return pack_count(_is_blood_potion); }

void _blood_pool_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血之池");
        break;
    case SPELL_DESC:
        var_set_string(res, "用你自己的鲜血制造一瓶令人毛骨悚然的治疗药水。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "制造一瓶鲜血药水。玩家最多只能拥有30瓶此类药水，并且不能丢弃、投掷或出售它们。喝下鲜血药水可恢复100点生命值，并治愈盲目、困惑、中毒和眩晕。");
        break;
    case SPELL_CAST:
    {
        object_type forge;
        int ct = _count_blood_potions();

        if (ct >= 30)
        {
            msg_print("你现在有太多鲜血药水了。为什么不喝一点呢？");
            var_set_bool(res, FALSE);
            return;
        }

        msg_print("你感到头晕目眩。");
        object_prep(&forge, lookup_kind(TV_POTION, SV_POTION_BLOOD));
        if (!object_is_known(&forge)) obj_identify(&forge);
        object_origins(&forge, ORIGIN_BLOOD);
        pack_carry(&forge);
        msg_print("你储存了你的血液以备后用。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _blood_explosion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鲜血爆炸");
        break;
    case SPELL_DESC:
        var_set_string(res, "对视线内的所有活物造成伤害，但这会极大地消耗你自己的生命值。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "玩家视线内的所有活物受到 500 点伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, 500));
        break;
    case SPELL_CAST:
    {
        msg_print("你割得太深了……你的鲜血爆炸了！");
        dispel_living(500);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void _cauterize_wounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "烧灼伤口");
        break;
    case SPELL_DESC:
        var_set_string(res, "治愈流血");
        break;
    case SPELL_CAST:
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _powers[] =
{
    { A_CON, {30, 20, 50, _cauterize_wounds_spell} }, 
    { -1, { -1, -1, -1, NULL} }
};

static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    {  1,   1, 20, _blood_flow_spell },
    {  5,  5,  30, _blood_sight_spell},
    { 10, 10,  30, _blood_spray_spell},
    { 15, 20,  30, _blood_bath_spell},
    { 20, 30,  30, _blood_shield_spell},
    { 25, 50,  40, _blood_seeking_spell},
    { 30, 60,  40, _blood_rage_spell},
    { 40, 60,  50, _blood_feast_spell},
    { 42, 60,   0, _blood_revenge_spell},
    { 45,200,   0, _blood_pool_spell},
    { 50,500,  60, _blood_explosion_spell},
    { -1, -1,  -1, NULL}
}; 

static void _calc_bonuses(void)
{
    p_ptr->regen += 100 + 2*p_ptr->lev;
    if (p_ptr->lev >= 30) res_add(RES_FEAR);

    if (p_ptr->cut > 0)
    {
        int to_h = 0;
        int to_d = 0;
        int to_stealth = 0;
        if (p_ptr->cut >= CUT_MORTAL_WOUND)
        {
            to_h = 25;
            to_d = 25;
            to_stealth = -10;
        }
        else if (p_ptr->cut >= CUT_DEEP_GASH)
        {
            to_h = 15;
            to_d = 15;
            to_stealth = -3;
        }
        else if (p_ptr->cut >= CUT_SEVERE)
        {
            to_h = 8;
            to_d = 8;
            to_stealth = -2;
        }
        else if (p_ptr->cut >= CUT_NASTY)
        {
            to_h = 6;
            to_d = 6;
            to_stealth = -2;
        }
        else if (p_ptr->cut >= CUT_BAD)
        {
            to_h = 4;
            to_d = 4;
            to_stealth = -1;
        }
        else if (p_ptr->cut >= CUT_LIGHT)
        {
            to_h = 2;
            to_d = 2;
            to_stealth = -1;
        }
        else
        {
            to_h = 1;
            to_d = 1;
            to_stealth = -1;
        }
        p_ptr->weapon_info[0].to_h += to_h;
        p_ptr->weapon_info[1].to_h += to_h;
        p_ptr->to_h_m  += to_h;
        p_ptr->weapon_info[0].dis_to_h += to_h;
        p_ptr->weapon_info[1].dis_to_h += to_h;

        p_ptr->weapon_info[0].to_d += to_d;
        p_ptr->weapon_info[1].to_d += to_d;
        p_ptr->to_d_m  += to_d;
        p_ptr->weapon_info[0].dis_to_d += to_d;
        p_ptr->weapon_info[1].dis_to_d += to_d;

        p_ptr->skills.stl += to_stealth;
    }

    if (p_ptr->tim_blood_shield)
    {
        int amt = 100 * (p_ptr->mhp - p_ptr->chp) / p_ptr->mhp; 
        p_ptr->to_a += amt;
        p_ptr->dis_to_a += amt;    
        if (amt > 60)
            p_ptr->reflect = TRUE;
    }

    if (p_ptr->tim_blood_feast)
    {
        p_ptr->weapon_info[0].to_d += 35;
        p_ptr->weapon_info[1].to_d += 35;
        p_ptr->to_d_m  += 35; /* Tentacles, beak, etc. */
        p_ptr->weapon_info[0].dis_to_d += 35;
        p_ptr->weapon_info[1].dis_to_d += 35;
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_REGEN);
    if (p_ptr->lev >= 30) add_flag(flgs, OF_RES_FEAR);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    int frac = p_ptr->chp * 100 / p_ptr->mhp;
    static point_t taulukko[7] = { {0, 0}, {1, 50}, {20, 128}, {40, 242}, {60, 440}, {80, 666}, {100, 1000} };
    if (p_ptr->chp < p_ptr->mhp) info_ptr->xtra_blow += interpolate(MIN(100 - frac, (p_ptr->lev * 5 / 3) + MAX(5, p_ptr->lev / 2 - 8)), taulukko, 7);
}

static void _on_cast(const spell_info *spell)
{
    set_cut(p_ptr->cut + spell->level, FALSE);
    p_ptr->update |= PU_BONUS;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "bloodcraft";
        me.options = CASTER_USE_HP;
        me.which_stat = A_CON;
        me.on_cast = _on_cast;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CURE_CRITICAL, rand_range(2, 5));
}

class_t *blood_knight_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  32,   2,  16,   6,  70,  20};
    skills_t xs = { 12,   7,  10,   0,   0,   0,  23,  15};

        me.name = "血骑士";
        me.desc = "血骑士是深入研究黑暗艺术的战士，可以使用他们自己的生命值来施展一些特殊的攻击效果。除了生命值消耗外，使用此类能力还会导致流血，流血量与能力消耗成正比。这些特殊能力依赖于体质(Constitution)。\n \n血骑士非常与众不同，流血和低生命值会让他们变得更强大；在满血时，他们的近战能力平平无奇，但他们受到的伤害和流血越严重，他们的攻击就越快、越致命。在濒死之际，他们是你能想象到的最强战士，关于他们传奇事迹的故事比比皆是；但巨大的力量伴随着巨大的死亡风险，你不记得曾经亲眼见过这些传说中的英雄！\n \n像血法师一样，血骑士从治疗魔法中获得的收益会减少。由于血骑士依赖自己的血液获取力量，这个职业仅限于特定的种族；没有任何非生命种族可以踏上这条猩红之路。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -2;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  3;
        me.stats[A_CHR] =  2;
        
        me.base_skills = bs;
        me.extra_skills = xs;
        
        me.life = 120;
        me.base_hp = 20;
        me.exp = 150;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _spells;
        me.get_powers = _powers;
        me.character_dump = py_dump_spells;
        init = TRUE;
    }

    return &me;
}

