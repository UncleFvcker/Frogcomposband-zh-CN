#include "angband.h"

void _double_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "双重施法");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!can_do_cmd_cast()) return;
        handle_stuff();
        do_cmd_cast();
        handle_stuff();
        if (!p_ptr->paralyzed && can_do_cmd_cast())
            do_cmd_cast();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _get_powers[] =
{
    { A_NONE, { 48, 20, 0, _double_magic_spell}},
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
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        me.min_fail = 5;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    int i;
    for (i = 0; i < 64; i++)
        p_ptr->spell_exp[i] = SPELL_EXP_EXPERT;

    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_HARD_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_ARCANE_BOOK, 0, 1);
}

class_t *red_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  34,  34,   1,  16,  10,  56,  25};
    skills_t xs = {  7,  11,  11,   0,   0,   0,  18,  11};

        me.name = "红魔道士";
        me.desc = "红魔道士是比大多数魔法师更优秀的战士，他们也是多才多艺的施法者，可以使用许多魔法领域，而无需只专精于一两个。这种广泛涉猎的缺点是，红魔道士的魔法相当肤浅；他们有很高的失败率、最低等级要求和法力消耗，并且除了奥秘领域外，其他每个领域都受限于前两本法术书。\n\n红魔道士拥有一项职业能力——“双重施法”，允许他们同时施展两个法术。与普通魔法师一样，他们的施法属性是智力(Intelligence)。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] = -1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 106;
        me.base_hp = 8;
        me.exp = 140;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_SLOW | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _get_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
