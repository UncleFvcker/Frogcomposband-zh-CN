#include "angband.h"

static power_info _get_good_powers[] =
{
    { A_WIS, { 35, 70, 90, bless_weapon_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info _get_evil_powers[] =
{
    { A_WIS, { 42, 40, 80, evocation_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static power_info *_get_powers(void)
{
    if (is_good_realm(p_ptr->realm1))
    {
        return _get_good_powers;
    }
    else
    {
        return _get_evil_powers;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "祈祷";
        me.which_stat = A_WIS;
        me.encumbrance.max_wgt = 430;
        me.encumbrance.weapon_pct = 67;
        me.encumbrance.enc_wgt = 800;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

static bool _priest_weapon_is_icky(object_type *o_ptr)
{
    if (!object_is_weapon(o_ptr)) return FALSE;
    if (!obj_is_identified(o_ptr)) return FALSE; /* Might be icky... but we don't know yet */
    if ((o_ptr->tval != TV_SWORD) && (o_ptr->tval != TV_POLEARM)) return FALSE;
    if (is_evil_realm(p_ptr->realm1)) return FALSE;
    else
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        return !have_flag(flgs, OF_BLESSED);
    }
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if (o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM)
    {
        u32b flgs[OF_ARRAY_SIZE];
        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_BLESSED))
        {
        }
        else if (is_evil_realm(p_ptr->realm1))
        {
        }
        else
        {
            info_ptr->to_h -= 2;
            info_ptr->dis_to_h -= 2;

            info_ptr->to_d -= 2;
            info_ptr->dis_to_d -= 2;

            info_ptr->icky_wield = TRUE;
        }
    }
}

bool priest_is_good(void)
{
    if (p_ptr->pclass == CLASS_PRIEST && is_good_realm(p_ptr->realm1))
        return TRUE;
    return FALSE;
}

bool priest_is_evil(void)
{
    if (p_ptr->pclass == CLASS_PRIEST && is_evil_realm(p_ptr->realm1))
        return TRUE;
    return FALSE;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_HAFTED, SV_MACE, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_HEALING, 1);
    py_birth_spellbooks();
}

class_t *priest_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  28,  40,   2,  16,   8,  48,  35};
    skills_t xs = {  7,  11,  12,   0,   0,   0,  13,  11};

        me.name = "牧师";
        me.desc = "牧师是致力于侍奉更高力量的角色。他们为了侍奉神明而探索地下城。他们对魔法装置相当熟悉，因为他们认为这些装置是神明干预自然秩序的焦点。\n\n牧师分为两种：善良和邪恶。如果牧师选择生命或圣战作为他们的第一领域，他们将走上善良的道路；因此，他们不能选择邪恶领域作为他们的第二领域。善良的牧师厌恶流血，所以不习惯使用带刃的武器，尽管他们最终会学会祝福这些武器，从而在不干扰祈祷的情况下使用它们。然而，如果牧师选择死亡或恶魔作为他们的第一领域，他们将侍奉邪神；邪恶的牧师实际上喜欢流血，并且不受这种武器限制。当然，邪恶的牧师厌恶善良的事物，无法选择生命或圣战作为他们的第二领域。\n\n善良的牧师对生命祈祷有着强烈的亲和力，并且能学得非常好，甚至比高等法师学得还要好。相反，邪恶的牧师偏爱死亡祈祷，并在选择这个邪恶领域时获得强大的加成。除此之外，牧师学习魔法的效率不如法师；但他们更强的体力和战斗技能弥补了这一点。与法师不同，牧师不能选择学习特定的法术；相反，他们是由神明的心血来潮而赐予新的祈祷，这大概是符合某种更宏大的神圣计划。牧师的主要属性是感知。";

        me.stats[A_STR] = -1;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] =  3;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 120;
        me.pets = 35;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers_fn = _get_powers;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.character_dump = spellbook_character_dump;
        me.known_icky_object = _priest_weapon_is_icky;
        init = TRUE;
    }

    return &me;
}
