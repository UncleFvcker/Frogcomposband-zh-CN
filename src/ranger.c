#include "angband.h"

static void _calc_bonuses(void)
{
    /* rangers are decent shooters all around, but especially good with bows */
    slot_t slot = equip_find_obj(TV_BOW, SV_ANY); /* fyi, shooter_info not set yet ... */
    if (slot) p_ptr->skills.thb += 20 + p_ptr->lev;
}

static void _calc_shooter_bonuses(object_type *o_ptr, shooter_info_t *info_ptr)
{
    if (p_ptr->shooter_info.tval_ammo != TV_ARROW )
        p_ptr->shooter_info.base_shot = 100;
}

static power_info _get_powers[] =
{
    { A_WIS, { 15, 20, 90, probing_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        me.min_level = 3;
        me.min_fail = 5;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(20, 40));
    py_birth_spellbooks();
}

class_t *ranger_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  37,  36,   3,  24,  16,  56,  50};
    skills_t xs = {  8,  11,  10,   0,   0,   0,  18,  16};

        me.name = "游侠";
        me.desc = "游侠是来自平原或树林经验丰富的流浪者，他们与自然世界息息相关。游侠几乎没有弱点；像魔法师一样，他们精通魔法装置，但在使用弓箭和近战武器方面也相当不错。他们在荒野中的时光磨练了他们的潜行、搜索和察觉能力，他们与自然精灵的联盟甚至赋予了他们良好的豁免判定。\n \n所有的游侠都接受过自然魔法的训练，所有的自然法术他们都可以使用；他们学习这些法术的速度甚至几乎和魔法师一样快。他们还可以选择一个次系领域（咒术、混沌、死亡、王牌、奥秘和恶魔）；但在这些领域他们学习得很慢，并且可能会发现自己无法学习某些最高级别的法术。另一个缺点是，像牧师类职业一样，游侠缺乏选择自己法术的能力；他们会学习反复无常的自然之神选择教给他们的任何东西。\n\n游侠拥有一项职业能力——“探查怪物”，这允许他们评估遇到的怪物的优点和弱点。游侠的魔法力量取决于感知(Wisdom)。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] =  2;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 8;
        me.exp = 140;
        me.pets = 35;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_SLOW | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        me.calc_bonuses = _calc_bonuses;
        me.calc_shooter_bonuses = _calc_shooter_bonuses;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
