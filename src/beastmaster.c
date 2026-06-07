#include "angband.h"

static power_info _beastmaster_powers[] =
{
    { A_CHR, { 1, 0, 70, dominate_living_I_spell}},
    { A_CHR, {30, 0, 70, dominate_living_II_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.min_fail = 5;
        me.min_level = 3;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_POLEARM, SV_SPEAR, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *beastmaster_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  25,  32,   2,  18,  16,  52,  63};
    skills_t xs = {  7,  10,  10,   0,   0,   0,  14,  25};

        me.name = "驯兽师";
        me.desc = "驯兽师将世上凶猛的生物视为机遇而非威胁，他们的骑乘技巧几乎无人能及。结合他们的职业能力和使用的王牌（Trump）魔法，他们有许多方法可以召唤或魅惑活物，让其作为自己的手脚效劳。驯兽师是除了箭术专家之外最优秀的射手之一，而且他们的近战技巧也足以应付各种情况。魅力（Charisma）决定了驯兽师的施法能力。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 103;
        me.base_hp = 6;
        me.exp = 120;
        me.pets = 10;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_WEAK;
        
        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers = _beastmaster_powers;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
