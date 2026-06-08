#include "angband.h"

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 40)
        res_add(RES_FEAR);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_FEAR);
}

static power_info _get_good_powers[] =
{
    { A_WIS, { 30, 30, 70, holy_lance_spell}},
    { -1, {-1, -1, -1, NULL}}
};
static power_info _get_evil_powers[] =
{
    { A_WIS, { 30, 30, 70, hell_lance_spell}},
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
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 5;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_RING_MAIL, 1);
    py_birth_spellbooks();
}

class_t *paladin_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  24,  34,   1,  12,   2,  68,  40};
    skills_t xs = {  7,  10,  11,   0,   0,   0,  21,  18};

        me.name = "圣骑士";
        me.desc = "圣骑士是战士和牧师的结合体。圣骑士擅长近战，但在远程战斗中表现平平；他们的潜行、察觉和使用魔法装置的技能同样平庸，不过他们与神明的结盟赋予了他们不错的豁免能力。感知决定了圣骑士向其神明祈祷的成功率。\n\n圣骑士可以从生命、圣战、恶魔和死亡中选择一个领域。和牧师一样，他们不能选择要学习哪些祈祷，而是由他们的神明赐予新的祈祷。他们可以学习所有法术，但不如牧师快。他们极其厌恶异教，甚至可以通过摧毁高级的异教法术书来获得经验：“异教”对于死亡/恶魔圣骑士来说指的是生命或圣战法术书，而对于生命/圣战圣骑士来说指的是除生命或圣战之外的所有法术书。圣骑士会获得一项职业能力：“神圣长枪”或“地狱长枪”，具体取决于他们领域的阵营。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] = -3;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 110;
        me.base_hp = 12;
        me.exp = 135;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.get_powers_fn = _get_powers;
        me.character_dump = spellbook_character_dump;
        me.get_flags = _get_flags;
        init = TRUE;
    }

    return &me;
}
