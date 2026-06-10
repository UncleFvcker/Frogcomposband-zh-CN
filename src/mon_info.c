/* Display Monster Lore to the User
   Adapted from the ancient roff_aux() but redesigned for a more
   readable display; a 'Monster Sheet' if you will. */

#include "angband.h"

#include <stdlib.h>
#include <assert.h>

extern void mon_display(monster_race *r_ptr);
extern void mon_display_rect(monster_race *r_ptr, rect_t display);
extern void mon_display_doc(monster_race *r_ptr, doc_ptr doc);
extern void mon_display_possessor(monster_race *r_ptr, doc_ptr doc);
static bool _possessor_hack = FALSE;

static void _display_basic(monster_race *r_ptr, doc_ptr doc);
static void _display_resists(monster_race *r_ptr, doc_ptr doc);
static void _display_spells(monster_race *r_ptr, doc_ptr doc);
static void _display_attacks(monster_race *r_ptr, doc_ptr doc);
static void _display_other(monster_race *r_ptr, doc_ptr doc);
static void _display_kills(monster_race *r_ptr, doc_ptr doc);
static void _display_desc(monster_race *r_ptr, doc_ptr doc);

static void _print_list(vec_ptr v, doc_ptr doc, char sep, char term);
static string_ptr _get_res_name(int res);

/**************************************************************************
 * Helpers
 **************************************************************************/
static void _print_list(vec_ptr v, doc_ptr doc, char sep, char term)
{
    int ct = vec_length(v);
    int i;
    for (i = 0; i < ct; i++)
    {
        string_ptr s = vec_get(v, i);
        if (i < ct - 1 && sep)
            doc_printf(doc, "%s%c ", string_buffer(s), sep);
        else if (i == ct - 1 && term)
            doc_printf(doc, "%s%c", string_buffer(s), term);
        else
            doc_insert(doc, string_buffer(s));
    }
}

static string_ptr _get_res_name(int res)
{
    return string_alloc_format(
        "<color:%c>%s</color>",
        attr_to_attr_char(res_color(res)),
        res_name(res)
    );
}

static bool _easy_lore(monster_race *r_ptr)
{
	if (easy_lore) return TRUE;
    if (p_ptr->wizard) return TRUE;
    if (spoiler_hack) return TRUE;
    if (r_ptr->r_xtra1 & MR1_LORE) return TRUE; /* Probing */
    return FALSE;
}

static bool _know_armor_hp(monster_race *r_ptr)
{
    int level = r_ptr->level;
    int kills = r_ptr->r_tkills;

    if (_easy_lore(r_ptr)) return TRUE;

    if (kills > 304 / (4 + level)) return TRUE;
    else if ((r_ptr->flags1 & RF1_UNIQUE) && kills > 304 / (38 + (5 * level) / 4)) return TRUE;

    return FALSE;
}

/* XXX The following calculations are slightly tweaked from very
 * old original code (was using 80). Spreadsheeting things up showed
 * spell damage not quite right (divisor is a hack) and what I would
 * guess to be the effect of melee damage inflation over the years.
 * I'm not sure what the following is going for, or what advantage it
 * has over say, just requiring 30 sightings to know stuff. */
static bool _know_damage_need(int dam, int lvl, bool unique)
{
    int need = 50*dam/MAX(1, lvl); /* was 80 */
    if (unique) need /= 5;
    need = MIN(100, MAX(5, need));
    return need;
}

static bool _know_damage_aux(int ct, int dam, int lvl, bool unique)
{
    int need = _know_damage_need(dam, lvl, unique);
    return ct >= need;
}

static bool _know_melee_damage(mon_race_ptr race, mon_effect_ptr effect)
{
    if (_easy_lore(race)) return TRUE;
    return _know_damage_aux(effect->lore, effect->dd*effect->ds,
        race->level + 4, BOOL(race->flags1 & RF1_UNIQUE));
}

static bool _know_spell_damage(mon_race_ptr race, mon_spell_ptr spell)
{
    if (_easy_lore(race)) return TRUE;
    return _know_damage_aux(
        spell->lore, MIN(500, mon_spell_avg_dam(spell, race, FALSE))/10,
        race->level + 4, BOOL(race->flags1 & RF1_UNIQUE));
}

static bool _know_aura_damage(mon_race_ptr race, mon_effect_ptr effect)
{
    if (_easy_lore(race)) return TRUE;
    return _know_damage_aux(effect->lore, effect->dd*effect->ds*5,
        race->level + 4, BOOL(race->flags1 & RF1_UNIQUE));
}

static bool _know_alertness(monster_race *r_ptr)
{
    int wake = r_ptr->r_wake;
    int sleep = r_ptr->sleep;
    int ignore = r_ptr->r_ignore;

    if (_easy_lore(r_ptr)) return TRUE;
    if (wake * wake > sleep) return TRUE;
    if (ignore == MAX_UCHAR) return TRUE;
    if (!sleep && r_ptr->r_tkills >= 10) return TRUE;

    return FALSE;
}

/**************************************************************************
 * Basic Info
 **************************************************************************/
static char _speed_color(int speed)
{
    if (speed >= 30) return 'r';
    else if (speed >= 20) return 'o';
    else if (speed >= 15) return 'u';
    else if (speed >= 10) return 'R';
    else if (speed >= 1) return 'U';
    else if (speed == 0) return 'w';
    else return 'G';
}
static void _display_level(monster_race *r_ptr, doc_ptr doc)
{
    doc_insert(doc, "等级 :");
    if (r_ptr->level == 0)
        doc_insert(doc, "<color:G>城镇</color>");
    else if (_easy_lore(r_ptr) || r_ptr->r_tkills > 0)
    {
        if (r_ptr->max_level != 999)
            doc_printf(doc, "<color:G>%d 到 %d</color>", r_ptr->level, r_ptr->max_level);
        else
            doc_printf(doc, "<color:G>%d</color>", r_ptr->level);
    }
    else
        doc_insert(doc, "<color:y>?</color>");
    doc_newline(doc);
    if (spoiler_hack)
        doc_printf(doc, "稀有度 : <color:G>%d</color>\n", r_ptr->rarity);
}
static void _display_ac(monster_race *r_ptr, doc_ptr doc)
{
    doc_insert(doc, "护甲 :");
    if (_know_armor_hp(r_ptr))
        doc_printf(doc, "<color:G>%d</color>", r_ptr->ac);
    else
        doc_insert(doc, "<color:y>?</color>");
    doc_newline(doc);
}
static void _display_hp(monster_race *r_ptr, doc_ptr doc)
{
    doc_insert(doc, "生命 :");
    if (_know_armor_hp(r_ptr))
    {
        if ((r_ptr->flags1 & RF1_FORCE_MAXHP) || r_ptr->hside == 1)
        {
            int hp = r_ptr->hdice * r_ptr->hside;
            doc_printf(doc, "<color:G>%d</color>", hp);
        }
        else
        {
            doc_printf(doc, "<color:G>%dd%d</color>", r_ptr->hdice, r_ptr->hside);
        }
    }
    else
        doc_insert(doc, "<color:y>?</color>");
    doc_newline(doc);
}
static void _display_speed(monster_race *r_ptr, doc_ptr doc)
{                        /* v~~~~~~byte */
    int speed = r_ptr->speed - 110;
    int rand = 0;
    if (effective_speed) doc_printf(doc, "速度: <color:%c>%d.%dx</color>", _speed_color(speed), SPEED_TO_ENERGY(r_ptr->speed) / 10, SPEED_TO_ENERGY(r_ptr->speed) % 10);
    else doc_printf(doc, "速度: <color:%c>%+d</color>", _speed_color(speed), speed);

    if (r_ptr->flags1 & RF1_RAND_50) rand += 50;
    if (r_ptr->flags1 & RF1_RAND_25) rand += 25;
    if (rand == 75) doc_insert(doc, "<color:r>极其飘忽</color>");
    else if (rand == 50) doc_insert(doc, "<color:R>有些飘忽</color>");
    else if (rand == 25) doc_insert(doc, "<color:o>轻微飘忽</color>");

    if (r_ptr->flags1 & RF1_NEVER_MOVE) doc_insert(doc, ", <color:u>静止</color>");

    doc_newline(doc);
}
static void _display_alertness(monster_race *r_ptr, doc_ptr doc)
{
    if (_know_alertness(r_ptr))
    {
        doc_insert(doc, "警戒:");
        if (r_ptr->sleep > 200)
            doc_insert(doc, "<color:D>无视入侵者</color>");
        else if (r_ptr->sleep > 95)
            doc_insert(doc, "<color:w>非常不留心</color>");
        else if (r_ptr->sleep > 75)
            doc_insert(doc, "<color:W>不留心</color>");
        else if (r_ptr->sleep > 45)
            doc_insert(doc, "<color:U>容易忽略</color>");
        else if (r_ptr->sleep > 25)
            doc_insert(doc, "<color:y>熟视无睹</color>");
        else if (r_ptr->sleep > 10)
            doc_insert(doc, "<color:y>相当盲目</color>");
        else if (r_ptr->sleep > 5)
            doc_insert(doc, "<color:o>相当敏锐</color>");
        else if (r_ptr->sleep > 3)
            doc_insert(doc, "<color:R>敏锐</color>");
        else if (r_ptr->sleep > 1)
            doc_insert(doc, "<color:r>非常敏锐</color>");
        else if (r_ptr->sleep > 0)
            doc_insert(doc, "<color:r>警惕</color>");
        else
            doc_insert(doc, "<color:v>时刻警惕</color>");
        doc_printf(doc, " <color:G>(%d')</color>\n", 10 * r_ptr->aaf);
    }
}
static void _display_type(monster_race *r_ptr, doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);
    doc_insert(doc, "类型 : <indent><style:indent>");

    if (r_ptr->flags2 & RF2_ELDRITCH_HORROR)
        vec_add(v, string_copy_s("<color:v>精神冲击</color>"));
    if (r_ptr->flags3 & RF3_ANIMAL)
        vec_add(v, string_copy_s("<color:G>自然</color>"));
    if (r_ptr->flags3 & RF3_EVIL)
        vec_add(v, string_copy_s("<color:D>邪恶</color>"));
    if (r_ptr->flags3 & RF3_GOOD)
        vec_add(v, string_copy_s("<color:y>善良</color>"));
    if (r_ptr->flags3 & RF3_UNDEAD)
        vec_add(v, string_copy_s("<color:v>不死</color>"));
    if (r_ptr->flags3 & RF3_NONLIVING)
        vec_add(v, string_copy_s("<color:U>无生命</color>"));
    if (r_ptr->flags3 & RF3_AMBERITE)
        vec_add(v, string_copy_s("<color:v>安珀皇族</color>"));
    if (r_ptr->flags3 & RF3_DRAGON)
        vec_add(v, string_copy_s("<color:o>龙类</color>"));
    if (r_ptr->flags3 & RF3_DEMON)
        vec_add(v, string_copy_s("<color:v>恶魔</color>"));
    if (r_ptr->flags3 & RF3_GIANT)
        vec_add(v, string_copy_s("<color:U>巨人</color>"));
    if (r_ptr->flags3 & RF3_TROLL)
        vec_add(v, string_copy_s("<color:B>巨魔</color>"));
    if (r_ptr->flags3 & RF3_ORC)
        vec_add(v, string_copy_s("<color:u>兽人</color>"));
    if (r_ptr->flags2 & RF2_HUMAN)
        vec_add(v, string_copy_s("<color:W>人类</color>"));
    if (r_ptr->flags2 & RF2_THIEF)
        vec_add(v, string_copy_s("<color:D>盗贼</color>"));
    /*if (r_ptr->flags2 & RF2_QUANTUM)
        vec_add(v, string_copy_s("<color:v>Quantum</color>"));*/
    if (r_ptr->flags1 & RF1_MALE)
        vec_add(v, string_copy_s("<color:b>雄性</color>"));
    if (r_ptr->flags1 & RF1_FEMALE)
        vec_add(v, string_copy_s("<color:R>雌性</color>")); /* Pink? */
    if (p_ptr->pclass == CLASS_WARLOCK && warlock_is_pact_monster(r_ptr))
        vec_add(v, string_copy_s("<color:v>契约</color>"));

    _print_list(v, doc, ',', '\0');
    vec_free(v);
    doc_insert(doc, "</style></indent>\n");
}
static void _display_basic(monster_race *r_ptr, doc_ptr doc)
{
    doc_printf(doc, "名称 : <indent><style:indent><color:B>%s</color>", monster_race_display_name(r_ptr->id));
    assert(r_ptr->d_char);
    doc_printf(doc, "(<color:%c>%c</color>", attr_to_attr_char(r_ptr->d_attr), r_ptr->d_char);
    if (use_graphics && (r_ptr->x_char != r_ptr->d_char || r_ptr->x_attr != r_ptr->d_attr))
    {
        doc_insert(doc, " / ");
        doc_insert_char(doc, r_ptr->x_attr, r_ptr->x_char);
    }
    doc_insert(doc, ")</style></indent>\n");

    {
        doc_ptr cols[2];

        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(MAX(20, MIN(50, doc_width(doc) - 20))); /* Monster Recall Terminal */

        /* Column 1 */
        _display_level(r_ptr, cols[0]);
        _display_ac(r_ptr, cols[0]);
        _display_hp(r_ptr, cols[0]);

        /* Column 2 */
        _display_speed(r_ptr, cols[1]);
        if (!_possessor_hack) _display_alertness(r_ptr, cols[1]);
        _display_type(r_ptr, cols[1]);

        doc_insert_cols(doc, cols, 2, 0);

        doc_free(cols[0]);
        doc_free(cols[1]);
    }
}

/**************************************************************************
 * Resists
 **************************************************************************/
static void _display_resists(monster_race *r_ptr, doc_ptr doc)
{
    int        i;
    int        ct = 0;
    vec_ptr    v = vec_alloc((vec_free_f)string_free);
    const int  flags[RES_MAX] = {
        RFR_RES_ACID, RFR_RES_ELEC, RFR_RES_FIRE, RFR_RES_COLD, RFR_RES_POIS,
        RFR_RES_LITE, RFR_RES_DARK, -1, RFR_RES_NETH, RFR_RES_NEXU, RFR_RES_SOUN,
        RFR_RES_SHAR, RFR_RES_CHAO, RFR_RES_DISE, RFR_RES_TIME, -1, -1, -1};

    for (i = 0; i < RES_MAX; i++)
    {
        int which = flags[i];
        if (which >= 0 && (r_ptr->flagsr & which))
        {
            if ((flags[i] == RFR_RES_NETH) && (r_ptr->flags3 & RF3_UNDEAD)) continue; /* Hack - undead are immune to nether */
            vec_add(v, _get_res_name(i));
        }
    }
    if ((r_ptr->flagsr & RFR_RES_TELE) && !(r_ptr->flags1 & RF1_UNIQUE) && !(r_ptr->flagsr & RFR_RES_ALL))
        vec_add(v, string_copy_s("<color:o>传送</color>"));
    if (r_ptr->flagsr & RFR_RES_WATE)
        vec_add(v, string_copy_s("<color:b>水</color>"));
    if (r_ptr->flagsr & RFR_RES_PLAS)
        vec_add(v, string_copy_s("<color:R>等离子</color>"));
    if (r_ptr->flagsr & RFR_RES_WALL)
        vec_add(v, string_copy_s("<color:u>原力</color>"));
    if (r_ptr->flagsr & RFR_RES_GRAV)
        vec_add(v, string_copy_s("<color:s>重力</color>"));
    if (r_ptr->flagsr & RFR_RES_DISI)
        vec_add(v, string_copy_s("<color:s>分解</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "抵抗 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Immunities */
    vec_clear(v);
    if (r_ptr->flagsr & RFR_RES_ALL)
    {
        vec_add(v, string_copy_s("<color:y>全属性</color>"));
    }
    if (r_ptr->flagsr & RFR_IM_ACID)
        vec_add(v, _get_res_name(RES_ACID));
    if (r_ptr->flagsr & RFR_IM_ELEC)
        vec_add(v, _get_res_name(RES_ELEC));
    if (r_ptr->flagsr & RFR_IM_FIRE)
        vec_add(v, _get_res_name(RES_FIRE));
    if (r_ptr->flagsr & RFR_IM_COLD)
        vec_add(v, _get_res_name(RES_COLD));
    if (r_ptr->flagsr & RFR_IM_POIS)
        vec_add(v, _get_res_name(RES_POIS));
    if (r_ptr->flags3 & RF3_UNDEAD)
        vec_add(v, _get_res_name(RES_NETHER));
    if (r_ptr->flags3 & RF3_NO_FEAR)
        vec_add(v, string_copy_s("<color:s>恐惧</color>"));
    if (r_ptr->flags3 & RF3_NO_STUN)
        vec_add(v, string_copy_s("<color:o>震慑</color>"));
    if (r_ptr->flags3 & RF3_NO_CONF)
        vec_add(v, string_copy_s("<color:U>混乱</color>"));
    if (r_ptr->flags3 & RF3_NO_SLEEP)
        vec_add(v, string_copy_s("<color:b>睡眠</color>"));
    if ((r_ptr->flagsr & RFR_RES_TELE) && ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flagsr & RFR_RES_ALL)))
        vec_add(v, string_copy_s("<color:o>传送</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "免疫 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Vulnerabilities */
    vec_clear(v);
    if (r_ptr->flags3 & RF3_HURT_FIRE)
        vec_add(v, _get_res_name(RES_FIRE));
    if (r_ptr->flags3 & RF3_HURT_COLD)
        vec_add(v, _get_res_name(RES_COLD));
    if (r_ptr->flags3 & RF3_HURT_LITE)
        vec_add(v, _get_res_name(RES_LITE));
    if (r_ptr->flags3 & RF3_HURT_ROCK)
        vec_add(v, string_copy_s("<color:u>粉碎岩石</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "易伤 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    if (ct) doc_newline(doc);
    vec_free(v);
}

/**************************************************************************
 * Spells
 **************************************************************************/
static void _display_frequency(monster_race *r_ptr, doc_ptr doc)
{
    int pct = 0;
    assert(r_ptr->spells);
    if ( spoiler_hack
	  || easy_lore
      || (!r_ptr->r_spell_turns && (r_ptr->r_xtra1 & MR1_LORE)) ) /* probing */
    {
        pct = r_ptr->spells->freq * 100;
    }
    else if (r_ptr->r_spell_turns)
    {
        int total = r_ptr->r_spell_turns + r_ptr->r_move_turns;
        pct = r_ptr->r_spell_turns * 10000 / total;
    }
    if (pct)
    {
        vec_ptr v = vec_alloc((vec_free_f)string_free);

        doc_printf(doc, "法术 : <indent><color:G>%d.%02d%%</color>", pct/100, pct%100);
        if (!spoiler_hack && r_ptr->r_spell_turns + r_ptr->r_move_turns > 0)
            doc_printf(doc, "(%d 次施法 / %d 次行动)", r_ptr->r_spell_turns, r_ptr->r_spell_turns + r_ptr->r_move_turns);

        if (r_ptr->flags2 & RF2_SMART)
            vec_add(v, string_copy_s("<color:y>智能</color>"));
        _print_list(v, doc, ',', '\0');
        vec_free(v);
        doc_insert(doc, "</indent>");
    }
    else
    {
        doc_printf(doc, "法术 : <color:y>?%%</color>");
    }
    doc_newline(doc);
}
static bool _is_attack_spell(mon_spell_ptr spell)
{
    switch (spell->id.type)
    {
    case MST_BREATH: case MST_BALL: case MST_BOLT: case MST_BEAM:
    case MST_CURSE: return TRUE;
    }
    return FALSE;
}
static void _display_spell_group(mon_race_ptr r_ptr, mon_spell_group_ptr group, vec_ptr v)
{
    int i;
    if (!group) return;
    for (i = 0; i < group->count; i++)
    {
        mon_spell_ptr spell = &group->spells[i];
        if (_easy_lore(r_ptr) || spell->lore)
        {
            string_ptr s = string_alloc();
            mon_spell_display(spell, s);
            if (_know_spell_damage(r_ptr, spell))
            {
                if (spell->parm.tag && spell->id.type != MST_SUMMON)
                {
                    string_append_c(s, ' ');
                    string_append_c(s, '(');
                    if (spoiler_hack || !_is_attack_spell(spell))
                    {
                        mon_spell_parm_print(&spell->parm, s, r_ptr);
                        if (spell->id.type == MST_CURSE && spell->id.effect == GF_HAND_DOOM)
                            string_append_c(s, '%');
                    }
                    else if (show_damage_range)
                    {
                        mon_spell_dam_range(s, spell, r_ptr, !_possessor_hack);
                    }
                    else
                        string_printf(s, "%d", mon_spell_avg_dam(spell, r_ptr, !_possessor_hack));
                    if (!spoiler_hack && !_possessor_hack && spell->lore) /* XXX stop repeating yourself! */
                        string_printf(s, ", %dx", spell->lore);
                    string_append_c(s, ')');
                }
                else if (!spoiler_hack && !_possessor_hack && spell->lore) /* XXX stop repeating yourself! */
                    string_printf(s, " (%dx)", spell->lore);
            }
            else if (!spoiler_hack && !_possessor_hack && spell->lore) /* XXX stop repeating yourself! */
                string_printf(s, " (%dx)", spell->lore);
            vec_add(v, s);
        }
    }
}
static void _display_spells(monster_race *r_ptr, doc_ptr doc)
{
    int            ct = 0;
    vec_ptr        v = vec_alloc((vec_free_f)string_free);
    mon_spells_ptr spells = r_ptr->spells;

    assert(spells);
    if (!_possessor_hack) _display_frequency(r_ptr, doc);

    /* Breaths */
    _display_spell_group(r_ptr, spells->groups[MST_BREATH], v);
    if (vec_length(v))
    {
        doc_insert(doc, "喷吐 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Offense */
    vec_clear(v);
    _display_spell_group(r_ptr, spells->groups[MST_BALL], v);
    _display_spell_group(r_ptr, spells->groups[MST_BOLT], v);
    _display_spell_group(r_ptr, spells->groups[MST_BEAM], v);
    _display_spell_group(r_ptr, spells->groups[MST_CURSE], v);
    if (vec_length(v))
    {
        doc_insert(doc, "攻击 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Annoy */
    vec_clear(v);
    _display_spell_group(r_ptr, spells->groups[MST_ANNOY], v);
    if (vec_length(v))
    {
        doc_insert(doc, "干扰 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Defense */
    vec_clear(v);
    _display_spell_group(r_ptr, spells->groups[MST_BUFF], v);
    _display_spell_group(r_ptr, spells->groups[MST_BIFF], v);
    _display_spell_group(r_ptr, spells->groups[MST_HEAL], v);
    _display_spell_group(r_ptr, spells->groups[MST_ESCAPE], v);
    _display_spell_group(r_ptr, spells->groups[MST_TACTIC], v);
    _display_spell_group(r_ptr, spells->groups[MST_WEIRD], v);
    if (vec_length(v))
    {
        doc_insert(doc, "防御 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Summoning */
    vec_clear(v);
    _display_spell_group(r_ptr, spells->groups[MST_SUMMON], v);
    if (vec_length(v))
    {
        doc_insert(doc, "召唤 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    if (ct) doc_newline(doc);
    vec_free(v);
}

/**************************************************************************
 * Attacks
 **************************************************************************/
static cptr _method_desc(int method)
{
    switch (method)
    {
    case RBM_HIT:     return "击打";
    case RBM_TOUCH:   return "触摸";
    case RBM_PUNCH:   return "拳击";
    case RBM_KICK:    return "踢击";
    case RBM_CLAW:    return "爪击";
    case RBM_BITE:    return "咬击";
    case RBM_STING:   return "蛰刺";
    case RBM_SLASH:   return "斩击";
    case RBM_BUTT:    return "冲撞";
    case RBM_CRUSH:   return "粉碎";
    case RBM_ENGULF:  return "吞噬";
    case RBM_CHARGE:  return "冲锋";
    case RBM_CRAWL:   return "爬行";
    case RBM_DROOL:   return "流涎";
    case RBM_SPIT:    return "喷吐";
    case RBM_EXPLODE: return "自爆";
    case RBM_GAZE:    return "凝视";
    case RBM_WAIL:    return "哀嚎";
    case RBM_SPORE:   return "孢子";
    case RBM_BEG:     return "乞讨";
    case RBM_INSULT:  return "侮辱";
    case RBM_MOAN:    return "呻吟";
    case RBM_SHOW:    return "唱歌";
    }
    return "怪异";
}
static string_ptr _effect_desc(mon_race_ptr race, mon_effect_ptr effect)
{
    string_ptr s;

    switch (effect->effect)
    {
    case RBE_HURT:        s = string_copy_s("伤害"); break;
    case RBE_DRAIN_CHARGES: s = string_copy_s("吸取充能"); break;
    case RBE_EAT_GOLD:    s = string_copy_s("偷取金钱"); break;
    case RBE_EAT_ITEM:    s = string_copy_s("偷取物品"); break;
    case RBE_EAT_FOOD:    s = string_copy_s("偷取食物"); break;
    case RBE_EAT_LITE:    s = string_copy_s("吸收光亮"); break;
    case RBE_LOSE_STR:    s = string_copy_s("降低力量"); break;
    case RBE_LOSE_INT:    s = string_copy_s("降低智力"); break;
    case RBE_LOSE_WIS:    s = string_copy_s("降低感知"); break;
    case RBE_LOSE_DEX:    s = string_copy_s("降低敏捷"); break;
    case RBE_LOSE_CON:    s = string_copy_s("降低体质"); break;
    case RBE_LOSE_CHR:    s = string_copy_s("降低魅力"); break;
    case RBE_LOSE_ALL:    s = string_copy_s("降低所有属性"); break;
    case RBE_SHATTER:     s = string_copy_s("引发地震"); break;
    case RBE_DRAIN_EXP:   s = string_copy_s("<color:D>汲取经验</color>"); break;
    case RBE_DISEASE:     s = string_copy_s("疾病"); break;
    case RBE_VAMP:        s = string_copy_s("<color:D>吸血</color>"); break;
    case RBE_CUT:         s = string_copy_s("<color:r>割伤</color>"); break;
    case GF_MISSILE:      s = string_copy_s("伤害"); break;
    case GF_TURN_ALL:     s = string_copy_s("<color:r>恐吓</color>"); break;
    default:              s = string_copy_s(gf_name(effect->effect));
    }
    assert(s);
    if (_know_melee_damage(race, effect))
    {
        if (effect->pct && effect->dd && effect->ds)
            string_printf(s, " (%dd%d,%d%%)", effect->dd, effect->ds, effect->pct);
        else if (effect->dd && effect->ds)
            string_printf(s, " (%dd%d)", effect->dd, effect->ds);
        else if (effect->pct)
            string_printf(s, " (%d%%)", effect->pct);
    }
    return s;
}
static int _ct_known_attacks(monster_race *r_ptr)
{
    int ct = 0;
    int i;
    for (i = 0; i < MAX_MON_BLOWS; i++)
    {
        mon_blow_ptr blow = &r_ptr->blows[i];
        if (!blow->method) continue;
        if (blow->lore || _easy_lore(r_ptr)) ct++;
    }
    return ct;
}
static void _display_attacks(monster_race *r_ptr, doc_ptr doc)
{
    if (r_ptr->flags1 & RF1_NEVER_BLOW)
        doc_insert(doc, "近战 : <color:D>无</color>\n");
    else if (_ct_known_attacks(r_ptr))
    {
        int i,j;
        /* XXX Damage display needs some rethinking ... */
        doc_printf(doc, "近战 : <color:G>%s 效果</color>\n", "类型");
        for (i = 0; i < MAX_MON_BLOWS; i++)
        {
            mon_blow_ptr blow = &r_ptr->blows[i];
            vec_ptr      v;

            if (!blow->method) continue;
            if (!_easy_lore(r_ptr) && !blow->lore) continue;

            v = vec_alloc((vec_free_f)string_free);
            for (j = 0; j < MAX_MON_BLOW_EFFECTS; j++)
            {
                mon_effect_ptr effect = &blow->effects[j];
                if (!effect->effect) continue;
                if (!_easy_lore(r_ptr) && !effect->lore) continue;
                vec_add(v, _effect_desc(r_ptr, effect));
            }
            doc_insert(doc, "          ");
            doc_insert(doc, _method_desc(blow->method));
            if (vec_length(v))
            {
                doc_insert(doc, " <indent><style:indent>");
                _print_list(v, doc, ',', '\0');
                doc_insert(doc, "</style></indent>");
            }
            doc_newline(doc);
            vec_free(v);
        }
    }
    else
        doc_insert(doc, "近战 : <color:y>?</color>\n");

    doc_newline(doc);
}

/**************************************************************************
 * Other Info
 **************************************************************************/
static void _display_other(monster_race *r_ptr, doc_ptr doc)
{
    int        ct = 0, i;
    vec_ptr    v = vec_alloc((vec_free_f)string_free);

    if (r_ptr->flags2 & RF2_KILL_WALL)
        vec_add(v, string_copy_s("<color:U>摧毁墙壁</color>"));

    if (r_ptr->flags2 & RF2_PASS_WALL)
        vec_add(v, string_copy_s("<color:B>穿透墙壁</color>"));

    if (r_ptr->flags2 & RF2_REFLECTING)
        vec_add(v, string_copy_s("<color:o>反射</color>"));

    if (r_ptr->flags7 & (RF7_SELF_LITE_1 | RF7_SELF_LITE_2))
        vec_add(v, string_copy_s("<color:y>发光</color>"));

    if (r_ptr->flags7 & (RF7_SELF_DARK_1 | RF7_SELF_DARK_2))
        vec_add(v, string_copy_s("<color:D>暗影笼罩</color>"));

    if (r_ptr->flags2 & RF2_INVISIBLE)
        vec_add(v, string_copy_s("<color:B>隐形</color>"));

    if (r_ptr->flags2 & RF2_COLD_BLOOD)
        vec_add(v, string_copy_s("<color:w>冷血</color>"));

    if (r_ptr->flags2 & RF2_EMPTY_MIND)
        vec_add(v, string_copy_s("<color:o>屏蔽心灵感应</color>"));

    if (r_ptr->flags2 & RF2_WEIRD_MIND)
        vec_add(v, string_copy_s("<color:w>诡异心智</color>"));

    if (r_ptr->flags3 & RF3_CLEAR_HEAD)
        vec_add(v, string_copy_s("<color:v>清晰心智</color>"));

    if (r_ptr->flags2 & RF2_MULTIPLY)
        vec_add(v, string_copy_s("<color:U>爆炸式增殖</color>"));

    if (r_ptr->flags2 & RF2_REGENERATE)
        vec_add(v, string_copy_s("<color:r>再生</color>"));

    if (r_ptr->flags7 & RF7_RANGED_MELEE)
        vec_add(v, string_copy_s("<color:o>长武器</color>"));

    if (r_ptr->flags7 & RF7_RIDING)
        vec_add(v, string_copy_s("<color:s>适合骑乘</color>"));

    if ((r_ptr->flags7 & RF7_GUARDIAN) && !no_wilderness)
        vec_add(v, string_copy_s("<color:R>地下城守卫</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "信息 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    /* Auras */
    vec_clear(v);

    if (r_ptr->flags2 & RF2_AURA_REVENGE)
        vec_add(v, string_copy_s("<color:v>反击</color>"));
    if (r_ptr->flags2 & RF2_AURA_FEAR)
        vec_add(v, string_copy_s("<color:v>恐惧</color>"));
    if (r_ptr->flags2 & RF2_AURA_FIRE)
        vec_add(v, _get_res_name(RES_FIRE));
    if (r_ptr->flags3 & RF3_AURA_COLD)
        vec_add(v, _get_res_name(RES_COLD));
    if (r_ptr->flags2 & RF2_AURA_ELEC)
        vec_add(v, _get_res_name(RES_ELEC));

    for (i = 0; i < MAX_MON_AURAS; i++)
    {
        mon_effect_ptr aura = &r_ptr->auras[i];
        gf_info_ptr    gf;
        if (!aura->effect) continue;
        if (!_easy_lore(r_ptr) && !aura->lore) continue;
        gf = gf_lookup(aura->effect);
        if (gf)
        {
            string_ptr s = string_alloc_format("<color:%c>%s</color>", attr_to_attr_char(gf->color), gf->name);
            if (_know_aura_damage(r_ptr, aura))
                string_printf(s, " (%dd%d)", aura->dd, aura->ds);
            vec_add(v, s);
        }
    }

    if (vec_length(v))
    {
        doc_insert(doc, "光环 : <indent><style:indent>");
        _print_list(v, doc, ',', '\0');
        doc_insert(doc, "</style></indent>\n");
        ct += vec_length(v);
    }

    if (ct) doc_newline(doc);
    vec_free(v);
}

/**************************************************************************
 * Kills and Drops
 **************************************************************************/
static void _display_drops(monster_race *r_ptr, doc_ptr doc)
{
    int ct_gold = 0;
    int ct_obj = 0;

    if (_easy_lore(r_ptr))
    {
        if (r_ptr->flags1 & RF1_DROP_4D2) ct_gold += 8;
        if (r_ptr->flags1 & RF1_DROP_3D2) ct_gold += 6;
        if (r_ptr->flags1 & RF1_DROP_2D2) ct_gold += 4;
        if (r_ptr->flags1 & RF1_DROP_1D2) ct_gold += 2;
        if (r_ptr->flags1 & RF1_DROP_90) ct_gold += 1;
        if (r_ptr->flags1 & RF1_DROP_60) ct_gold += 1;

        ct_obj = ct_gold;

        /* Hack -- but only "valid" drops */
        if (r_ptr->flags1 & RF1_ONLY_GOLD) ct_obj = 0;
        if (r_ptr->flags1 & RF1_ONLY_ITEM) ct_gold = 0;
    }
    else
    {
        ct_gold = r_ptr->r_drop_gold;
        ct_obj = r_ptr->r_drop_item;
    }

    if (ct_gold || ct_obj)
    {
        int ct = MAX(ct_gold, ct_obj);
        cptr obj_text = (ct_obj > 1) ? "多件物品" : "物品";
        cptr gold_text = (ct_gold > 1) ? "多件财宝" : "财宝";

        doc_insert(doc, "掉落 :");

        if (ct == 1)
            doc_insert(doc, "1 ");
        else if (ct == 2)
            doc_insert(doc, "1 或 2 件");
        else
            doc_printf(doc, "最多 %d 件", ct);

        if (r_ptr->flags1 & RF1_DROP_GREAT)
            doc_insert(doc, "<color:v>极好的</color>");
        else if (r_ptr->flags1 & RF1_DROP_GOOD)
            doc_insert(doc, "<color:r>优秀的</color>");

        if (ct_gold && ct_obj)
        {
            doc_insert(doc, obj_text);
            doc_insert(doc, "或");
            doc_insert(doc, gold_text);
        }
        else if (ct_obj)
            doc_insert(doc, obj_text);
        else if (ct_gold)
            doc_insert(doc, gold_text);

        doc_newline(doc);
    }
}
static void _display_kills(monster_race *r_ptr, doc_ptr doc)
{
    if (r_ptr->flags1 & RF1_UNIQUE)
    {
        if (spoiler_hack)
            doc_insert(doc, "状态 : <color:v>唯一</color>");
        else
        {
            doc_insert(doc, "状态 :");
            if (r_ptr->max_num == 0)
                doc_insert(doc, "<color:D>死亡</color>");
            else if (mon_is_wanted(r_ptr->id))
                doc_insert(doc, "<color:v>通缉</color>");
            else
                doc_insert(doc, "<color:y>存活</color>");
        }
        doc_newline(doc);
    }
    else if (!spoiler_hack)
    {
        doc_printf(doc, "击杀 : <color:G>%d</color>\n", r_ptr->r_pkills);
    }

    if (_easy_lore(r_ptr) || r_ptr->r_tkills)
    {
        int plev = spoiler_hack ? 50 : p_ptr->max_plv;
        int xp = r_ptr->mexp * r_ptr->level / (plev + 2);
        char buf[10];

        if (r_ptr->r_akills > (coffee_break ? 49 : 99))
        {
            xp *= 2;
            xp /= divide_exp_by(r_ptr->r_akills);
        }

        big_num_display(xp, buf);
        doc_printf(doc, "经验 : <color:G>%s</color> (角色等级CL%d时)\n", buf, plev);
    }

    _display_drops(r_ptr, doc);
    doc_newline(doc);
}

/**************************************************************************
 * Desc
 **************************************************************************/
static void _display_desc(monster_race *r_ptr, doc_ptr doc)
{
    doc_insert(doc, r_text + r_ptr->text);
    doc_newline(doc);
}

/**************************************************************************
 * Public
 **************************************************************************/
void mon_display(monster_race *r_ptr)
{
    mon_display_rect(r_ptr, ui_menu_rect());
}
void mon_display_rect(monster_race *r_ptr, rect_t display)
{
    doc_ptr doc = doc_alloc(MIN(display.cx, 72));

    if (display.cx > 80)
        display.cx = 80;

    mon_display_doc(r_ptr, doc);

    screen_save();
    if (doc_cursor(doc).y < display.cy - 3)
    {
        doc_insert(doc, "\n<color:B>[按任意键继续]</color>\n\n");
        doc_sync_term(doc, doc_range_all(doc), doc_pos_create(display.x, display.y));
        inkey();
    }
    else
    {
        doc_display_aux(doc, "怪物信息", 0, display);
    }
    screen_load();

    doc_free(doc);
}
void mon_display_doc(monster_race *r_ptr, doc_ptr doc)
{
    /* XXX Struct copy is a bad idea ... for example, mon_race->spells is
     * not copied, but the pointer is. Try to see why we need to copy
     * to correctly display lore. I mean, can't the other code just use
     * the r_flags instead of the flags? */
    monster_race copy = *r_ptr;
    if (!_easy_lore(r_ptr))
    {
        /* Wipe flags to their 'known' values */
        copy.flags1 &= copy.r_flags1;
        copy.flags2 &= copy.r_flags2;
        copy.flags3 &= copy.r_flags3;
        copy.flagsr &= copy.r_flagsr;

        /* Assume some "obvious" flags */
        if (r_ptr->flags1 & RF1_UNIQUE)  copy.flags1 |= RF1_UNIQUE;
        if (r_ptr->flags1 & RF1_MALE)    copy.flags1 |= RF1_MALE;
        if (r_ptr->flags1 & RF1_FEMALE)  copy.flags1 |= RF1_FEMALE;

        /* Assume some "creation" flags */
        if (r_ptr->flags1 & RF1_FRIENDS) copy.flags1 |= RF1_FRIENDS;
        if (r_ptr->flags1 & RF1_ESCORT)  copy.flags1 |= RF1_ESCORT;

        /* Killing a monster reveals some properties */
        if (r_ptr->r_tkills)
        {
            /* Know "race" flags */
            if (r_ptr->flags3 & RF3_ORC)      copy.flags3 |= RF3_ORC;
            if (r_ptr->flags3 & RF3_TROLL)    copy.flags3 |= RF3_TROLL;
            if (r_ptr->flags3 & RF3_GIANT)    copy.flags3 |= RF3_GIANT;
            if (r_ptr->flags3 & RF3_DRAGON)   copy.flags3 |= RF3_DRAGON;
            if (r_ptr->flags3 & RF3_DEMON)    copy.flags3 |= RF3_DEMON;
            if (r_ptr->flags3 & RF3_UNDEAD)   copy.flags3 |= RF3_UNDEAD;
            if (r_ptr->flags3 & RF3_EVIL)     copy.flags3 |= RF3_EVIL;
            if (r_ptr->flags3 & RF3_GOOD)     copy.flags3 |= RF3_GOOD;
            if (r_ptr->flags3 & RF3_ANIMAL)   copy.flags3 |= RF3_ANIMAL;
            if (r_ptr->flags3 & RF3_AMBERITE) copy.flags3 |= RF3_AMBERITE;
            if (r_ptr->flags2 & RF2_HUMAN)    copy.flags2 |= RF2_HUMAN;
            if (r_ptr->flags2 & RF2_QUANTUM)  copy.flags2 |= RF2_QUANTUM;

            /* Know "forced" flags */
            if (r_ptr->flags1 & RF1_FORCE_DEPTH) copy.flags1 |= RF1_FORCE_DEPTH;
            if (r_ptr->flags1 & RF1_FORCE_MAXHP) copy.flags1 |= RF1_FORCE_MAXHP;
        }

    }

    _display_basic(&copy, doc);
    _display_resists(&copy, doc);
    if (copy.spells && copy.spells->freq) _display_spells(&copy, doc);
    _display_attacks(&copy, doc);
    _display_other(&copy, doc);
    if (!_possessor_hack) _display_kills(&copy, doc);

    _display_desc(&copy, doc);
}

void mon_display_possessor(monster_race *r_ptr, doc_ptr doc)
{
    _possessor_hack = TRUE;
    mon_display_doc(r_ptr, doc);
    _possessor_hack = FALSE;
}

