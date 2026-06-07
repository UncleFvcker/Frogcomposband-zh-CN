#include "angband.h"

static power_info _get_powers[] =
{
    { A_NONE, { 1, 0,  0, hex_stop_spelling_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static caster_info * _caster_info(void)
{
	static caster_info me = { 0 };
	static bool init = FALSE;
	if (!init)
	{
		me.magic_desc = "法术";
		me.which_stat = A_INT;
		me.encumbrance.max_wgt = 430;
		me.encumbrance.weapon_pct = 33;
		me.encumbrance.enc_wgt = 1200;
		me.options = CASTER_GLOVE_ENCUMBRANCE;
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

class_t *hex_mage_get_class(void)
{
	static class_t me = { 0 };
	static bool init = FALSE;

	if (!init)
	{           /* dis, dev, sav, stl, srh, fos, thn, thb */
		skills_t bs = { 30,  35,  36,   2,  18,  16,  50,  50 };
		skills_t xs = { 7,  10,  10,   0,   0,   0,  15,  15 };

		me.name = "咒术法师";
		me.desc = "正如其名，咒术法师是介于战士与法师之间的职业。虽然他们的兄弟——游侠，专精于自然魔法与生存技能，但真正的战斗法师则试图同时兼顾两者之长。作为战士，他们远比普通的法师职业优秀。智力决定了战斗法师的施法能力。\n \n战斗法师开局即掌握奥秘魔法，且能自由选择另一个魔法领域。虽然他们获取新法术的速度不如普通法师快，但最终他们能学会这两个领域的所有法术，这使他们成为喜欢奥秘魔法的玩家极具竞争力的选择。他们有两个职业能力——“生命转法力”和“法力转生命”——允许他们用法力治疗生命值，或用生命值换取法力。";

		me.stats[A_STR] = 2;
		me.stats[A_INT] = 2;
		me.stats[A_WIS] = 0;
		me.stats[A_DEX] = 1;
		me.stats[A_CON] = 0;
		me.stats[A_CHR] = 1;
		me.base_skills = bs;
		me.extra_skills = xs;
		me.life = 106;
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
