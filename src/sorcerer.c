#include "angband.h"

static power_info _get_powers[] =
{
    { A_CHR, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    p_ptr->to_a -= 50;
    p_ptr->dis_to_a -= 50;
}

static bool _sorcerer_weapon_is_icky(object_type *o_ptr)
{
    if (!object_is_weapon(o_ptr)) return FALSE;
    if (object_is_(o_ptr, TV_HAFTED, SV_WIZSTAFF) || object_is_(o_ptr, TV_HAFTED, SV_NAMAKE_HAMMER)) return FALSE;
    return TRUE;
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if ( object_is_(o_ptr, TV_HAFTED, SV_WIZSTAFF)
      || object_is_(o_ptr, TV_HAFTED, SV_NAMAKE_HAMMER) )
    {
        info_ptr->to_h -= 30;
        info_ptr->to_d -= 10;
        info_ptr->dis_to_h -= 30;
        info_ptr->dis_to_d -= 10;
    }
    else
    {
        info_ptr->to_h -= 200;
        info_ptr->to_d -= 200;
        info_ptr->dis_to_h -= 200;
        info_ptr->dis_to_d -= 200;
        info_ptr->icky_wield = TRUE;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 40;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 900;
        me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    int i;
    for (i = 0; i < 64; i++)
        p_ptr->spell_exp[i] = SPELL_EXP_MASTER;

    py_birth_obj_aux(TV_HAFTED, SV_WIZSTAFF, 1);
    py_birth_obj_aux(TV_WAND, EFFECT_BOLT_MISSILE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CLARITY, rand_range(10, 20));

    for (i = TV_LIFE_BOOK; i < TV_LIFE_BOOK + MAX_MAGIC; i++)
    {
        if (i == TV_NECROMANCY_BOOK) continue;
        py_birth_obj_aux(i, 0, 1);
    }
}

class_t *sorcerer_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  48,  75,   2,  12,  22,   0,   0};
    skills_t xs = {  7,  18,  13,   0,   0,   0,   0,   0};

        me.name = "巫师";
        me.desc = "巫师是全能的最佳魔法师，能够精通几乎任何魔法领域的所有法术，而无需研究或提升熟练度。不利的一面是，他们是所有职业中生命值最少的；他们也是地下城中最糟糕的战士，除了巫师法杖外不能使用任何武器。\n \n巫师有一个职业能力——“吞噬魔法”——可以从魔杖、法杖或魔棒中吸收法力。与其他法师不同，他们依赖魅力作为他们的施法属性。";

        me.stats[A_STR] = -5;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -2;
        me.stats[A_DEX] =  2;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  6;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 65;
        me.base_hp = 0;
        me.exp = 160;
        me.pets = 25;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG |
                   CLASS_REGEN_MANA;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        me.known_icky_object = _sorcerer_weapon_is_icky;
        init = TRUE;
    }

    return &me;
}
