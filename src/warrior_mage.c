#include "angband.h"

static power_info _get_powers[] =
{
    { A_INT, { 25, 0, 50, hp_to_sp_spell}},
    { A_INT, { 25, 0, 50, sp_to_hp_spell}},
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
        me.encumbrance.enc_wgt = 1200;
        me.options = CASTER_ALLOW_DEC_MANA | CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_SHORT_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_spellbooks();
}

class_t *warrior_mage_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  36,  34,   2,  18,  16,  56,  50};
    skills_t xs = {  7,  10,  10,   0,   0,   0,  18,  15};

        me.name = "战法师";
        me.desc = "战法师正如其名：战士和法师的结合体。为了支持他们对法师来说不错的战斗能力，战法师在游戏开始时拥有奥秘魔法，并能自由选择另一个法术领域。虽然他们获得新法术的速度不如纯粹的法师快，但他们最终能学会两个领域中所有或几乎所有的法术，这使他们成为欣赏奥秘魔法的人的极具竞争力的选择。他们的职业能力允许他们根据需要在生命值(HP)和法力值(Mana)之间相互转换。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  2;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 105;
        me.base_hp = 8;
        me.exp = 140;
        me.pets = 35;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;
        
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
