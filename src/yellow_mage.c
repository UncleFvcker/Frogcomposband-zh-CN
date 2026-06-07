#include "angband.h"

static power_info _get_powers[] =
{
    { A_INT, { 25, 1, 90, eat_magic_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 100;
        me.encumbrance.enc_wgt = 600;
        me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _calc_bonuses(void)
{
    p_ptr->spells_per_round += py_prorata_level(150);
}


static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_spellbooks();
}

class_t *yellow_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  40,  38,   3,  16,  20,  34,  20};
    skills_t xs = {  7,  15,  11,   0,   0,   0,   6,   7};

        me.name = "黄衣法师";
        me.desc = "黄衣法师是一类专注于快速施法的法师。黄衣法师的施法速度会随着经验的增长而提高；低级法术施放起来特别快，最终甚至可以在一回合内施放多达五次！而且对于一名熟练的黄衣法师来说，即使是 50 级的法术也不需要半个回合。\n\n在大多数其他方面，黄衣法师与普通法师相似；他们使用智力作为主要的施法属性，并且可以选择学习两个法术领域的法术。然而，由于他们专注于快速施法，一些最深奥、最强大的魔法将永远无法被他们掌握。";

        me.stats[A_STR] = -4;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  0;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 95;
        me.base_hp = 0;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG |
                   CLASS_REGEN_MANA;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        me.character_dump = spellbook_character_dump;
        me.get_powers = _get_powers;
        init = TRUE;
    }

    return &me;
}
