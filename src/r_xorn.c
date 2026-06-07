#include "angband.h"

static void _birth(void) 
{ 
    object_type    forge;

    p_ptr->current_r_idx = MON_UMBER_HULK;
    skills_innate_init("凝视", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_SWORD, SV_LONG_SWORD));
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_d = 3;
    py_birth_obj(&forge);

    object_prep(&forge, lookup_kind(TV_BOOTS, SV_PAIR_OF_METAL_SHOD_BOOTS));
    py_birth_obj(&forge);

    equip_on_change_race();

    py_birth_food();
    py_birth_light();
}

static void _calc_innate_attacks(void)
{
    if (p_ptr->lev < 20 && !p_ptr->blind) /* Umber Hulk only ... */
    {
        innate_attack_t    a = {0};

        a.flags |= INNATE_NO_DAM;
        a.effect[0] = GF_OLD_CONF;
        a.blows = 100;
        a.to_h = p_ptr->lev/5;
        a.msg = "You gaze.";
        a.name = "凝视";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _calc_bonuses(void) {
    int to_a = py_prorata_level(75);
    int ac = (p_ptr->lev < 20) ? 5 : 10;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    p_ptr->skill_dig += 500;

    p_ptr->free_act++;
    res_add(RES_POIS);
    res_add(RES_CONF);
    p_ptr->sustain_str = TRUE;
    if (p_ptr->lev < 20)
        p_ptr->kill_wall = TRUE;

    if (p_ptr->lev >= 20)
    {
        res_add(RES_COLD);
        res_add(RES_ELEC);
        res_add(RES_FIRE);
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 35)
    {
        p_ptr->pspeed += 2 + (p_ptr->lev - 35)/5;
    }
}
static void _get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_FREE_ACT);
    add_flag(flgs, OF_SUST_STR);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_CONF);

    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, OF_RES_COLD);
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_RES_FIRE);
    }
    if (p_ptr->lev >= 35)
    {
        add_flag(flgs, OF_SPEED);
    }
}
static void _gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_UMBER_HULK && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_XORN;
        equip_on_change_race();
        msg_print("你进化成了索恩(Xorn)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_XORN && new_level >= 35)
    {
        p_ptr->current_r_idx = MON_XAREN;
        msg_print("你进化成了扎伦(Xaren)。");
        p_ptr->redraw |= PR_MAP;
    }
}
race_t *mon_xorn_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"土巨怪", "索恩", "扎伦"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 35) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  20,  31,   2,  14,   5,  51,  30};
    skills_t xs = { 12,   8,  10,   0,   0,   0,  21,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "索恩";
        me.desc = "索恩是体型巨大的土元素生物。它们生命之初是土巨怪，这是一种奇异的生物，拥有能切开岩石的巨大下颚和能使敌人困惑的耀眼双目。在进化的这个阶段，它们的身体隐约呈现人形，这允许它们戴头盔、护身符、斗篷甚至一双靴子；然而，一旦土巨怪进一步进化，它就不能再穿戴这些物品了。取而代之的是，成年的索恩可以利用它四条粗壮的手臂来装备武器、盾牌、戒指和手套；成熟的索恩也可以毫不费力地穿过岩石。\n \n索恩没有主动的魔法能力，而是依靠它们隐藏在岩石中的能力以及多出的手臂在近战中提供的优势。像大多数战士角色一样，索恩在击杀怪物方面非常有效，但在使用高级魔法装置时会遇到困难。";

        me.infra = 5;
        me.exp = 150;
        me.base_hp = 25;
        me.shop_adjust = 120;

        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.birth = _birth;

        me.flags = RACE_IS_MONSTER;
        me.pseudo_class_idx = CLASS_WARRIOR;

        init = TRUE;
    }

    me.subname = titles[rank];
    me.stats[A_STR] =  2 + rank;
    me.stats[A_INT] = -4;
    me.stats[A_WIS] = -2;
    me.stats[A_DEX] = -3 + rank;
    me.stats[A_CON] =  rank;
    me.stats[A_CHR] = -1;
    me.life = (rank > 0) ? (96 + 4 * rank) : 92;

    me.equip_template = mon_get_equip_template();

    if (birth_hack || spoiler_hack)
    {
        me.subname = NULL;
        me.subdesc = NULL;
    }
    return &me;
}
