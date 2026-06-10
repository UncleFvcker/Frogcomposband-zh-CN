#include "angband.h"

#include <stdlib.h>
#include <assert.h>

/* Build & Display the "Character Sheet" */

extern void py_display(void);
extern void py_display_birth(void);
extern void py_display_spells(doc_ptr doc, power_info *table, int ct);
extern void py_display_powers(doc_ptr doc, power_info *table, int ct);
extern void py_display_character_sheet(doc_ptr doc);
extern void py_display_dungeons(doc_ptr doc);

static void _build_general(doc_ptr doc);
static void _build_equipment(doc_ptr doc);
static void _build_melee(doc_ptr doc);
static void _build_shooting(doc_ptr doc);
static void _build_powers(doc_ptr doc);
static void _build_spells(doc_ptr doc);
static void _build_dungeons(doc_ptr doc);
static void _build_uniques(doc_ptr doc);
static void _build_virtues(doc_ptr doc);
static void _build_race_history(doc_ptr doc);
static void _build_mutations(doc_ptr doc);
static void _build_pets(doc_ptr doc);
static void _build_allies(doc_ptr doc);
static void _build_inventory(doc_ptr doc);
static void _build_quiver(doc_ptr doc);
static void _build_home(doc_ptr doc);
static void _build_museum(doc_ptr doc);
static void _build_statistics(doc_ptr doc);

static void _build_messages(doc_ptr doc);
static void _build_options(doc_ptr doc);

static cptr _stat_label_zh(int stat)
{
    switch (stat)
    {
    case A_STR: return "力量";
    case A_INT: return "智力";
    case A_WIS: return "感知";
    case A_DEX: return "敏捷";
    case A_CON: return "体质";
    case A_CHR: return "魅力";
    }
    return "";
}

/********************************** Page 1 ************************************/
static void _build_general1(doc_ptr doc)
{
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    bool             patron_listed = FALSE;

    doc_printf(doc, "名字 : <color:B>%s</color>\n", player_name);
    doc_printf(doc, "性别 : <color:B>%s</color>\n", sex_info[p_ptr->psex].title);
    if (p_ptr->personality == PERS_SPLIT)
        split_dump(doc, 0);
    else doc_printf(doc, "性格 : <color:B>%s</color>\n", pers_ptr->name);

    if (race_ptr->mimic)
        doc_printf(doc, "种族 : <color:B>[%s]</color>\n", race_ptr->name);
    else
        doc_printf(doc, "种族 : <color:B>%s</color>\n", race_ptr->name);

    if (race_ptr->subname)
    {
        char nimi[MAX_NLEN];
        int paikka;
        bool ok_name = FALSE;
        strcpy(nimi, get_race()->subname);
        if (strlen(get_race()->subname) < 25) ok_name = TRUE;
        while (!ok_name)
        {
            paikka = strpos(",", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" the ", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" the", nimi);
            if (paikka >= 20)
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            paikka = strpos(" of ", nimi);
            if (paikka) 
            {
                nimi[paikka - 1] = '\0';
                break;
            }
            break;
        }

        if (p_ptr->prace == RACE_MON_RING)
            doc_printf(doc, "控制 : <color:B>%s</color>\n", nimi);
        else if (p_ptr->prace == RACE_MON_MIMIC)
        {
            if (p_ptr->current_r_idx == MON_MIMIC)
                doc_printf(doc, "模仿 : <color:B>%s</color>\n", "无");
            else
                doc_printf(doc, "模仿 : <color:B>%s</color>\n", nimi);
        }
        else
            doc_printf(doc, "分支种族 : <color:B>%s</color>\n", nimi);
    }
    else
        doc_printf(doc, "分支种族 : <color:B>%s</color>\n", "无");

    doc_printf(doc, "职业 : <color:B>%s</color>\n", class_ptr->name);

    /* Assume Subclass and Magic are mutually exclusive ... */
    if (class_ptr->subname)
        doc_printf(doc, "副职业 : <color:B>%s</color>\n", class_ptr->subname);
    else if (p_ptr->prace == RACE_MON_DRAGON)
    {
        dragon_realm_ptr realm = dragon_get_realm(p_ptr->dragon_realm);
        doc_printf(doc, "领域 : <color:B>%s</color>\n", realm->name);
    }
    else if ((p_ptr->realm1) && (p_ptr->pclass != CLASS_LAWYER) && (p_ptr->pclass != CLASS_NINJA_LAWYER))
    {
        if (p_ptr->realm2)
            doc_printf(doc, "领域 : <color:B>%s, %s</color>\n", realm_names[p_ptr->realm1], realm_names[p_ptr->realm2]);
        else
            doc_printf(doc, "领域 : <color:B>%s</color>\n", realm_names[p_ptr->realm1]);
    }
    else if ((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || mut_present(MUT_CHAOS_GIFT))
    {
        doc_printf(doc, "庇护者 : <color:B>%s</color>\n", chaos_patrons[p_ptr->chaos_patron]);
        patron_listed = TRUE;
    }
    else
        doc_newline(doc);

    if (((p_ptr->pclass == CLASS_CHAOS_WARRIOR) || mut_present(MUT_CHAOS_GIFT)) && (!patron_listed))
        doc_printf(doc, "庇护者 : <color:B>%s</color>\n", chaos_patrons[p_ptr->chaos_patron]);
    else
        doc_newline(doc);


    doc_printf(doc, "等级 : <color:G>%8d</color>\n", p_ptr->lev);
    if (p_ptr->prace == RACE_ANDROID)
    {
        doc_printf(doc, "构造进度 : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_newline(doc);
    }
    else
    {
        doc_printf(doc, "当前经验 : <color:%c>%8d</color>\n", p_ptr->exp >= p_ptr->max_exp ? 'G' : 'y', p_ptr->exp);
        doc_printf(doc, "最大经验 : <color:G>%8d</color>\n", p_ptr->max_exp);
    }
    doc_printf(doc, "升级需验 : <color:G>%8.8s</color>\n", p_ptr->lev >= PY_MAX_LEVEL ? "*****" : format("%d", exp_requirement(p_ptr->lev)));
    doc_newline(doc);
    doc_newline(doc);

    doc_printf(doc, "金币 : <color:G>%8d</color>\n", p_ptr->au);
    doc_printf(doc, "击杀数 : <color:G>%8d</color>\n", ct_kills());
    doc_printf(doc, "唯一怪 : <color:G>%8d</color>\n", ct_uniques(CTU_INCLUDE_SUPPRESSED | CTU_INCLUDE_RARE | CTU_COUNT_DEAD));
    doc_printf(doc, "神器数 : <color:G>%8.8s</color>\n",
                            no_artifacts ? "N/A" : format("%d+%d" , ct_artifacts(), stats_rand_art_counts.found));
    doc_newline(doc);

    {
        int day, hour, min;
        extract_day_hour_min(&day, &hour, &min);

        doc_printf(doc, "游戏天数 : <color:G>%8d</color>\n", day);
        doc_printf(doc, "游戏时间 : <color:G>%8.8s</color>\n", format("%d:%02d", hour, min));
    }

    update_playtime();
    doc_printf(doc, "游玩时间 : <color:G>%8.8s</color>\n",
                            format("%.2lu:%.2lu", playtime/(60*60), (playtime/60)%60));
}

static void _build_label(doc_ptr doc, cptr name);
static void _build_skill_label(doc_ptr doc, cptr name);

static void _display_skill(doc_ptr doc, cptr name, int amt, int div)
{
    skill_desc_t desc = skills_describe(amt, div);
    _build_skill_label(doc, name);
    doc_printf(doc, "<color:%c>%s</color>", attr_to_attr_char(desc.color), desc.desc);
    if (p_ptr->wizard || display_skill_num || 0)
        doc_printf(doc, " (%d)", amt);
    doc_newline(doc);
}

static char _stat_color(int i, int mode)
{
    if ((!p_ptr->unwell) || ((i != A_DEX) && (i != A_CON)) || (!unwell_effect(p_ptr->unwell)))
        return ((mode == 2) || ((p_ptr->stat_use[i] < p_ptr->stat_top[i]) && (mode != 1))) ? 'y' : 'G';
    else
        return ((mode == 2) || ((p_ptr->stat_use[i] < p_ptr->stat_top[i]) && (mode != 1))) ? 'B' : 'W';
}

static void _build_general2(doc_ptr doc)
{
    string_ptr s = string_alloc();
    char       buf[255];
    int        i;

    doc_insert(doc, "========== 属性 ==========\n");
    for (i = 0; i < MAX_STATS; i++)
    {
        doc_printf(doc, "<tab:9>%s", _stat_label_zh(i));

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        cnv_stat(p_ptr->stat_use[i], buf);
        doc_printf(doc, "<color:%c>%9.9s</color>\n", _stat_color(i, 0), buf);
    }

    doc_newline(doc);

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->chp , p_ptr->mmhp);
    doc_printf(doc, "<tab:9>生命 : <color:%c>%9.9s</color>\n",
                    p_ptr->chp >= p_ptr->mhp ? 'G' :
                        p_ptr->chp > (p_ptr->mmhp * hitpoint_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    string_clear(s);
    string_printf(s, "%d/%d", p_ptr->csp , p_ptr->msp);
    if (elemental_is_(ELEMENTAL_WATER))
        doc_printf(doc, "<tab:9>流派 : <color:G>%9.9s</color>\n", string_buffer(s));
    else
        doc_printf(doc, "<tab:9>法力 : <color:%c>%9.9s</color>\n",
                    p_ptr->csp >= p_ptr->msp ? 'G' :
                        p_ptr->csp > (p_ptr->msp * mana_warn) / 10 ? 'y' : 'r',
                    string_buffer(s));

    doc_printf(doc, "<tab:9>护甲 : <color:G>%9d</color>\n", p_ptr->dis_ac + p_ptr->dis_to_a);

    /* Dump speed ... What a monster! */
    {
        int  tmp_speed = 0;
        byte attr;
        int  speed = p_ptr->pspeed-110;

        /* Hack -- Visually "undo" the Search Mode Slowdown */
        if (p_ptr->action == ACTION_SEARCH) speed += 10;

        if (speed > 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_GREEN;
            else
                attr = TERM_GREEN;
        }
        else if (i == 0)
        {
            if (!p_ptr->riding)
                attr = TERM_L_BLUE;
            else
                attr = TERM_GREEN;
        }
        else
        {
            if (!p_ptr->riding)
                attr = TERM_L_UMBER;
            else
                attr = TERM_RED;
        }

        if (!p_ptr->riding)
        {
            if (IS_FAST()) tmp_speed += 10;
            tmp_speed -= player_slow();
            if (IS_LIGHT_SPEED()) tmp_speed = 99;
        }
        else
        {
            if (MON_FAST(&m_list[p_ptr->riding])) tmp_speed += 10;
            tmp_speed -= monster_slow(&m_list[p_ptr->riding]);
        }

        string_clear(s);
        if (tmp_speed)
        {
            string_printf(s, "%+d%+d", speed-tmp_speed, tmp_speed);
            if (tmp_speed > 0)
                attr = TERM_YELLOW;
            else
                attr = TERM_VIOLET;
        }
        else
        {
            string_printf(s, "%+d", speed);
        }

        doc_printf(doc, "<tab:9>速度 : <color:%c>%9.9s</color>\n", attr_to_attr_char(attr), string_buffer(s));
    }

    doc_newline(doc);
    doc_insert(doc, "========== 技能 =========\n");

    {
        skills_t skills = p_ptr->skills;
        int      slot = equip_find_obj(TV_BOW, SV_ANY);

        /* Patch Up Skills a bit */
        skills.thn += p_ptr->to_h_m * BTH_PLUS_ADJ;
        if (slot)
        {
            object_type *bow = equip_obj(slot);
            if (bow)
                skills.thb += (p_ptr->shooter_info.to_h + bow->to_h) * BTH_PLUS_ADJ;
        }
        if (!skills.stl)
            skills.stl = -1; /* Force "Very Bad" */

        /* Display */
        _display_skill(doc, "近战", skills.thn, 12);
        _display_skill(doc, "箭术", skills.thb, 12);
        _display_skill(doc, "豁免", skills.sav, 7);
        _display_skill(doc, "潜行", skills.stl, 1);
        _display_skill(doc, "察觉", skills.fos, 6);
        _display_skill(doc, "搜索", skills.srh, 6);
        _display_skill(doc, "解除陷阱", skills.dis, 8);
        _display_skill(doc, "装置", skills.dev, 7);
    }

    string_free(s);
}

static void _build_general(doc_ptr doc)
{
    doc_ptr cols[2];

    cols[0] = doc_alloc(40);
    cols[1] = doc_alloc(40);

    _build_general1(cols[0]);
    _build_general2(cols[1]);
    doc_insert_cols(doc, cols, 2, 0);

    doc_free(cols[0]);
    doc_free(cols[1]);
}

/********************************** Equipment *********************************/
static void _equippy_chars(doc_ptr doc, int col)
{
    if (equippy_chars)
    {
        int i;
        doc_printf(doc, "<tab:%d>", col);
        for (i = 1; i <= equip_max(); i++)
        {
            object_type *o_ptr = equip_obj(i);

            if (o_ptr)
            {
                byte a;
                char c;

                a = object_attr(o_ptr);
                c = object_char(o_ptr);

                doc_insert_char(doc, a, c);
            }
            else
                doc_insert_char(doc, TERM_WHITE, ' ');
        }
        doc_newline(doc);
    }
}

static void _build_label(doc_ptr doc, cptr name)
{
    doc_insert_space(doc, 1);
    doc_insert(doc, name);
    if (doc_cursor(doc).x < 12)
        doc_insert_space(doc, 12 - doc_cursor(doc).x);
    else
        doc_insert_space(doc, 1);
    doc_insert(doc, ": ");
}

static void _build_skill_label(doc_ptr doc, cptr name)
{
    doc_insert_space(doc, 3);
    doc_insert(doc, name);
    if (doc_cursor(doc).x < 14)
        doc_insert_space(doc, 14 - doc_cursor(doc).x);
    else
        doc_insert_space(doc, 1);
    doc_insert(doc, ": ");
}

static void _equippy_heading_aux(doc_ptr doc, cptr heading, int col)
{
    int i;
    doc_printf(doc, " <color:G>%s</color><tab:%d>", heading, col);
    for (i = 1; i <= equip_max(); i++)
        doc_insert_char(doc, TERM_WHITE, 'a' + i - 1);
    doc_insert_char(doc, TERM_WHITE, '@');
}

static void _equippy_heading(doc_ptr doc, cptr heading, int col)
{
    _equippy_heading_aux(doc, heading, col);
    doc_newline(doc);
}

typedef struct {
    u32b py_flgs[OF_ARRAY_SIZE];
    u32b tim_py_flgs[OF_ARRAY_SIZE];
    u32b obj_flgs[EQUIP_MAX + 1][OF_ARRAY_SIZE];
} _flagzilla_t, *_flagzilla_ptr;

static _flagzilla_ptr _flagzilla_alloc(void)
{
    _flagzilla_ptr flagzilla = malloc(sizeof(_flagzilla_t));
    int            i;

    memset(flagzilla, 0, sizeof(_flagzilla_t));

    player_flags(flagzilla->py_flgs);
    tim_player_flags(flagzilla->tim_py_flgs);
    for (i = 1; i <= equip_max(); i++)
    {
        object_type *o_ptr = equip_obj(i);

        if (o_ptr)
        {
            obj_flags_display(o_ptr, flagzilla->obj_flgs[i]);
            remove_opposite_flags(flagzilla->obj_flgs[i]);
            switch (o_ptr->rune)
            {
            case RUNE_ABSORPTION:
                add_flag(flagzilla->obj_flgs[i], OF_MAGIC_RESISTANCE);
                break;
            case RUNE_SHADOW:
                if (object_is_body_armour(o_ptr) || o_ptr->tval == TV_CLOAK)
                    add_flag(flagzilla->obj_flgs[i], OF_STEALTH);
                break;
            case RUNE_HASTE:
                if (o_ptr->tval == TV_BOOTS)
                    add_flag(flagzilla->obj_flgs[i], OF_SPEED);
                break;
            }
        }
    }

    return flagzilla;
}

static void _flagzilla_free(_flagzilla_ptr flagzilla)
{
    free(flagzilla);
}

static void _build_res_flags(doc_ptr doc, int which, _flagzilla_ptr flagzilla)
{
    int i;
    int flg = res_get_object_flag(which);
    int im_flg = res_get_object_immune_flag(which);
    int vuln_flg = res_get_object_vuln_flag(which);
    int pct = res_pct_known(which);
    char color = 'w';

    _build_label(doc, res_name(which));

    for (i = 1; i <= equip_max(); i++)
    {
        if (im_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], im_flg))
            doc_insert_char(doc, TERM_VIOLET, '*');
        else if (vuln_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], vuln_flg) && have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_L_DARK, '.');
        else if (vuln_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], vuln_flg))
            doc_insert_char(doc, TERM_L_RED, '-');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }

    if (im_flg != OF_INVALID && have_flag(flagzilla->py_flgs, im_flg))
        doc_insert_char(doc, TERM_VIOLET, '*');
    else if (im_flg != OF_INVALID && have_flag(flagzilla->tim_py_flgs, im_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
    {
        if (vuln_flg != OF_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
            doc_insert_char(doc, TERM_ORANGE, '#');
        else
            doc_insert_char(doc, TERM_YELLOW, '#');
    }
    else if (vuln_flg != OF_INVALID && have_flag(flagzilla->py_flgs, vuln_flg))
        doc_insert_char(doc, TERM_RED, 'v');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    if (pct == 100)
        color = 'v';
    else if (pct < 0)
        color = 'D';
    else if (res_is_low(which))
    {
        if (pct >= 72)
            color =  'r';
        else if (pct >= 65)
            color =  'R';
        else if (pct >= 50)
            color =  'y';
    }
    else
    {
        if (pct >= 45)
            color =  'r';
        else if (pct >= 40)
            color =  'R';
        else if (pct >= 30)
            color =  'y';
    }

    if (which == RES_FEAR && res_pct(RES_FEAR) < 100)
        doc_printf(doc, " %3dx", res_ct_known(which));
    else
        doc_printf(doc, " <color:%c>%3d%%</color>", color, pct);
    doc_newline(doc);
}

static void _build_curse_flags(doc_ptr doc, cptr name)
{
    int i;
    _build_label(doc, name);
    for (i = 1; i <= equip_max(); i++)
    {
        object_type *o_ptr = equip_obj(i);

        if (o_ptr)
        {
            if (o_ptr->curse_flags & OFC_PERMA_CURSE)
                doc_insert_char(doc, TERM_VIOLET, '*');
            else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
                doc_insert_char(doc, TERM_L_RED, '+');
            else if (o_ptr->curse_flags & OFC_CURSED)
                doc_insert_char(doc, TERM_WHITE, '+');
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    doc_insert_char(doc, TERM_L_DARK, '.');
    doc_newline(doc);
}

static void _build_slays_imp(doc_ptr doc, cptr name, int flg, int kill_flg, _flagzilla_ptr flagzilla)
{
    int i;
    _build_label(doc, name);
    for (i = 1; i <= equip_max(); i++)
    {
        if (kill_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], kill_flg))
            doc_insert_char(doc, TERM_RED, '*');
        else if (have_flag(flagzilla->obj_flgs[i], flg))
            doc_insert_char(doc, TERM_WHITE, '+');
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (kill_flg != OF_INVALID && have_flag(flagzilla->tim_py_flgs, kill_flg))
        doc_insert_char(doc, TERM_YELLOW, '*');
    else if (have_flag(flagzilla->tim_py_flgs, flg))
        doc_insert_char(doc, TERM_YELLOW, '+');
    else if (kill_flg != OF_INVALID && have_flag(flagzilla->py_flgs, kill_flg))
        doc_insert_char(doc, TERM_RED, '*');
    else if (have_flag(flagzilla->py_flgs, flg))
        doc_insert_char(doc, TERM_WHITE, '+');
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    doc_newline(doc);
}

static int _build_flags_imp(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    int result = 0;
    int i;
    _build_label(doc, name);
    for (i = 1; i <= equip_max(); i++)
    {
        if (have_flag(flagzilla->obj_flgs[i], flg))
        {
            doc_insert_char(doc, TERM_WHITE, '+');
            result++;
        }
        else if (dec_flg != OF_INVALID && have_flag(flagzilla->obj_flgs[i], dec_flg))
        {
            doc_insert_char(doc, TERM_L_RED, '-');
            result--;
        }
        else
            doc_insert_char(doc, TERM_L_DARK, '.');
    }
    if (have_flag(flagzilla->tim_py_flgs, flg))
    {
       doc_insert_char(doc, TERM_YELLOW, '#');
       result++;
    }
    else if (have_flag(flagzilla->py_flgs, flg))
    {
        doc_insert_char(doc, TERM_WHITE, '+');
        result++;
    }
    else if (dec_flg != OF_INVALID && have_flag(flagzilla->py_flgs, dec_flg))
    {
        doc_insert_char(doc, TERM_L_RED, '-');
        result++;
    }
    else
        doc_insert_char(doc, TERM_L_DARK, '.');

    return result;
}

static void _build_flags_aura(doc_ptr doc, cptr name, int flg, _flagzilla_ptr flagzilla)
{
    if (_build_flags_imp(doc, name, flg, OF_INVALID, flagzilla))
    {
        if (flg == OF_AURA_FIRE)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_fire - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_COLD)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_cold - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_ELEC)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_elec - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else if (flg == OF_AURA_SHARDS)
             doc_printf(doc, " %dd%d+2", 2 * p_ptr->sh_shards - 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
        else doc_printf(doc, " %dd%d+2", 1 + p_ptr->lev/10, 2 + p_ptr->lev/ 10);
    }
    doc_newline(doc);
}

static void _build_flags(doc_ptr doc, cptr name, int flg, int dec_flg, _flagzilla_ptr flagzilla)
{
    _build_flags_imp(doc, name, flg, dec_flg, flagzilla);
    doc_newline(doc);
}

static void _build_flags1(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int i;
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "抗性", 14);

    for (i = RES_BEGIN; i < RES_END; i++)
        _build_res_flags(doc, i, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "光环", 14);
    _build_flags_aura(doc, "闪电光环", OF_AURA_ELEC, flagzilla);
    _build_flags_aura(doc, "火焰光环", OF_AURA_FIRE, flagzilla);
    _build_flags_aura(doc, "寒冷光环", OF_AURA_COLD, flagzilla);
    _build_flags_aura(doc, "碎片光环", OF_AURA_SHARDS, flagzilla);
    _build_flags(doc, "反击光环", OF_AURA_REVENGE, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "斩杀", 14);
    _build_slays_imp(doc, "斩杀邪恶", OF_SLAY_EVIL, OF_KILL_EVIL, flagzilla);
    _build_slays_imp(doc, "斩杀不死", OF_SLAY_UNDEAD, OF_KILL_UNDEAD, flagzilla);
    _build_slays_imp(doc, "斩杀恶魔", OF_SLAY_DEMON, OF_KILL_DEMON, flagzilla);
    _build_slays_imp(doc, "斩杀龙类", OF_SLAY_DRAGON, OF_KILL_DRAGON, flagzilla);
    _build_slays_imp(doc, "斩杀人类", OF_SLAY_HUMAN, OF_KILL_HUMAN, flagzilla);
    _build_slays_imp(doc, "斩杀动物", OF_SLAY_ANIMAL, OF_KILL_ANIMAL, flagzilla);
    _build_slays_imp(doc, "斩杀兽人", OF_SLAY_ORC, OF_KILL_ORC, flagzilla);
    _build_slays_imp(doc, "斩杀巨魔", OF_SLAY_TROLL, OF_KILL_TROLL, flagzilla);
    _build_slays_imp(doc, "斩杀巨怪", OF_SLAY_GIANT, OF_KILL_GIANT, flagzilla);
    _build_slays_imp(doc, "斩杀善良", OF_SLAY_GOOD, OF_KILL_GOOD, flagzilla);
    _build_slays_imp(doc, "斩杀活物", OF_SLAY_LIVING, OF_KILL_LIVING, flagzilla);
    _build_slays_imp(doc, "酸液烙印", OF_BRAND_ACID, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "闪电烙印", OF_BRAND_ELEC, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "火焰烙印", OF_BRAND_FIRE, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "寒冷烙印", OF_BRAND_COLD, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "毒素烙印", OF_BRAND_POIS, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "法力烙印", OF_BRAND_MANA, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "暗影烙印", OF_BRAND_DARK, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "锋锐", OF_VORPAL, OF_VORPAL2, flagzilla);
    _build_slays_imp(doc, "震慑", OF_STUN, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "地震", OF_IMPACT, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "吸血", OF_BRAND_VAMP, OF_INVALID, flagzilla);
    _build_slays_imp(doc, "混乱", OF_BRAND_CHAOS, OF_INVALID, flagzilla);
    _build_flags(doc, "额外攻击", OF_BLOWS, OF_DEC_BLOWS, flagzilla);
    _build_flags(doc, "额外射击", OF_XTRA_SHOTS, OF_INVALID, flagzilla);
    _build_flags(doc, "额外威力", OF_XTRA_MIGHT, OF_INVALID, flagzilla);
    _build_flags(doc, "受祝福", OF_BLESSED, OF_INVALID, flagzilla);
    _build_flags(doc, "骑乘", OF_RIDING, OF_INVALID, flagzilla);
    _build_flags(doc, "挖掘", OF_TUNNEL, OF_INVALID, flagzilla);
    _build_flags(doc, "投掷", OF_THROWING, OF_INVALID, flagzilla);
}

static void _display_known_count(doc_ptr doc, int total, int flg)
{
    int ct = total;
    slot_t slot;
    for (slot = 1; slot <= equip_max(); slot++)
    {
        obj_ptr obj = equip_obj(slot);
        u32b    flgs[OF_ARRAY_SIZE];
        u32b    flgs_known[OF_ARRAY_SIZE];

        if (!obj) continue;
        obj_flags(obj, flgs);
        obj_flags_known(obj, flgs_known);
        if (have_flag(flgs, flg) && !have_flag(flgs_known, flg))
            ct--;
    }
    if (ct)
        doc_printf(doc, " %3dx", ct);
    doc_newline(doc);
}
static void _build_flags2(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int _tmp;
    s32b _regen = (s32b)p_ptr->regen * py_food_regen();
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "能力", 14);

    _build_flags(doc, "速度", OF_SPEED, OF_DEC_SPEED, flagzilla);

    _build_flags_imp(doc, "行动自如", OF_FREE_ACT, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->free_act, OF_FREE_ACT);

    _build_flags_imp(doc, "识破隐形", OF_SEE_INVIS, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->see_inv, OF_SEE_INVIS);

    _build_flags(doc, "危险预警", OF_WARNING, OF_INVALID, flagzilla);
    _build_flags(doc, "减缓消化", OF_SLOW_DIGEST, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "快速恢复", OF_REGEN, OF_INVALID, flagzilla);
    doc_printf(doc, " %3d%%\n", p_ptr->regen);

    _build_flags_imp(doc, "生命再生", OF_INVALID, OF_SLOW_REGEN, flagzilla);
    doc_printf(doc, " %3d%%\n", _regen / PY_REGEN_NORMAL * mutant_regenerate_mod / 100);

    _build_flags_imp(doc, "法力再生", OF_REGEN_MANA, OF_INVALID, flagzilla);
    if (p_ptr->msp) doc_printf(doc, " %3d%%", ((p_ptr->pclass == CLASS_RUNE_KNIGHT || p_ptr->pclass == CLASS_RAGE_MAGE || mimic_no_regen()) ? 0 : (p_ptr->mana_regen ? _regen * 2 : _regen) / PY_REGEN_NORMAL));
    doc_newline(doc);

    _build_flags_imp(doc, "保持生命", OF_HOLD_LIFE, OF_INVALID, flagzilla);
    _display_known_count(doc, p_ptr->hold_life, OF_HOLD_LIFE);

    _build_flags(doc, "悬浮", OF_LEVITATION, OF_INVALID, flagzilla);
    _build_flags(doc, "永久光明", OF_LITE, OF_DARKNESS, flagzilla);
    _build_flags(doc, "反射", OF_REFLECT, OF_INVALID, flagzilla);
    _build_flags(doc, "夜视", OF_NIGHT_VISION, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "生命乘数", OF_LIFE, OF_DEC_LIFE, flagzilla);
    _tmp = (p_ptr->life - adj_con_mhp[p_ptr->stat_ind[A_CON]]);
    if (_tmp != 0)
    {
        doc_printf(doc, " %+3d%%", _tmp);
    }
    doc_newline(doc);

    _build_flags(doc, "减少法耗", OF_DEC_MANA, OF_INVALID, flagzilla);
    _build_flags(doc, "简易施法", OF_EASY_SPELL, OF_INVALID, flagzilla);
    _build_flags(doc, "反魔法", OF_NO_MAGIC, OF_INVALID, flagzilla);

    _build_flags_imp(doc, "魔法技能", OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY, flagzilla);
    if (p_ptr->device_power)
    {
        _tmp = device_power_aux(100, p_ptr->device_power) - 100;
        doc_printf(doc, " %+3d%%", _tmp);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "法术强度", OF_SPELL_POWER, OF_DEC_SPELL_POWER, flagzilla))
    {
        _tmp = spell_power_aux(100, p_ptr->spell_power) - 100;
        doc_printf(doc, " %+3d%%", _tmp);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "法术上限", OF_SPELL_CAP, OF_DEC_SPELL_CAP, flagzilla))
    {
        _tmp = spell_cap_aux(100, p_ptr->spell_cap) - 100;
        doc_printf(doc, " %+3d%%", _tmp);
    }
    doc_newline(doc);

    if (_build_flags_imp(doc, "魔法抗性", OF_MAGIC_RESISTANCE, OF_INVALID, flagzilla))
            doc_printf(doc, " %+3d%%", p_ptr->magic_resistance);
    doc_newline(doc);

    if (_build_flags_imp(doc, "红外视力", OF_INFRA, OF_INVALID, flagzilla))
        doc_printf(doc, " %3d'", p_ptr->see_infra * 10);
    doc_newline(doc);

    _build_flags(doc, "潜行", OF_STEALTH, OF_DEC_STEALTH, flagzilla);
    _build_flags(doc, "搜索", OF_SEARCH, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "维持", 14);
    _build_flags(doc, "维持力量", OF_SUST_STR, OF_INVALID, flagzilla);
    _build_flags(doc, "维持智力", OF_SUST_INT, OF_INVALID, flagzilla);
    _build_flags(doc, "维持感知", OF_SUST_WIS, OF_INVALID, flagzilla);
    _build_flags(doc, "维持敏捷", OF_SUST_DEX, OF_INVALID, flagzilla);
    _build_flags(doc, "维持体质", OF_SUST_CON, OF_INVALID, flagzilla);
    _build_flags(doc, "维持魅力", OF_SUST_CHR, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "感应", 14);
    _build_flags(doc, "心灵感应", OF_TELEPATHY, OF_INVALID, flagzilla);
    _build_flags(doc, "感应邪恶", OF_ESP_EVIL, OF_INVALID, flagzilla);
    _build_flags(doc, "感应无生命", OF_ESP_NONLIVING, OF_INVALID, flagzilla);
	_build_flags(doc, "感应活物", OF_ESP_LIVING, OF_INVALID, flagzilla);
    _build_flags(doc, "感应善良", OF_ESP_GOOD, OF_INVALID, flagzilla);
    _build_flags(doc, "感应不死", OF_ESP_UNDEAD, OF_INVALID, flagzilla);
    _build_flags(doc, "感应恶魔", OF_ESP_DEMON, OF_INVALID, flagzilla);
    _build_flags(doc, "感应龙类", OF_ESP_DRAGON, OF_INVALID, flagzilla);
    _build_flags(doc, "感应人类", OF_ESP_HUMAN, OF_INVALID, flagzilla);
    _build_flags(doc, "感应动物", OF_ESP_ANIMAL, OF_INVALID, flagzilla);
    _build_flags(doc, "感应兽人", OF_ESP_ORC, OF_INVALID, flagzilla);
    _build_flags(doc, "感应巨魔", OF_ESP_TROLL, OF_INVALID, flagzilla);
    _build_flags(doc, "感应巨怪", OF_ESP_GIANT, OF_INVALID, flagzilla);

    doc_newline(doc);
    _equippy_chars(doc, 14);
    _equippy_heading(doc, "诅咒", 14);
    _build_curse_flags(doc, "被诅咒");
    _build_flags(doc, "随机传送", OF_TELEPORT, OF_INVALID, flagzilla);
    _build_flags(doc, "无法传送", OF_NO_TELE, OF_INVALID, flagzilla);
    _build_flags(doc, "吸取经验", OF_DRAIN_EXP, OF_INVALID, flagzilla);
    _build_flags(doc, "激怒怪物", OF_AGGRAVATE, OF_INVALID, flagzilla);
    _build_flags(doc, "古神诅咒", OF_TY_CURSE, OF_INVALID, flagzilla);
}

static void _build_stats(doc_ptr doc, _flagzilla_ptr flagzilla)
{
    int              i, j;
    char             buf[255];
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    s16b             stats[MAX_STATS] = {0};
    s16b             tim_stats[MAX_STATS] = {0};

    mut_calc_stats(stats);
    if (race_ptr->calc_stats)
        race_ptr->calc_stats(stats);
    if (class_ptr->calc_stats)
        class_ptr->calc_stats(stats);

    tim_player_stats(tim_stats);

    _equippy_chars(doc, 14);
    _equippy_heading_aux(doc, "", 14);
    doc_insert(doc, "基础 种 职 性 装 总计\n");

    for (i = 0; i < MAX_STATS; i++)
    {
        int flg = OF_STR + i;
        int dec_flg = OF_DEC_STR + i;
        int sust_flg = OF_SUST_STR + i;
        int e_adj = 0;

        doc_printf(doc, "<tab:7>%s", _stat_label_zh(i));

        if (p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
            doc_insert(doc, "! : ");
        else
            doc_insert(doc, "  : ");

        /* abcdefghijkl */
        for (j = 1; j <= equip_max(); j++)
        {
            object_type *o_ptr = equip_obj(j);

            if (o_ptr)
            {
                int adj = 0;
                bool slipping = (o_ptr->marked & OM_SLIPPING) ? TRUE : FALSE;

                if (o_ptr->rune)
                {
                    s16b stats[MAX_STATS] = {0};
                    rune_calc_stats(o_ptr, stats);
                    adj += stats[i];
                }
                if (have_flag(flagzilla->obj_flgs[j], dec_flg))
                    adj = -o_ptr->pval;
                else if (have_flag(flagzilla->obj_flgs[j], flg))
                {
                    adj += o_ptr->pval;
                    if (p_ptr->prace == RACE_MON_ARMOR) adj += rag_effect_pval(o_ptr, -1, flg, FALSE);
                }

                if (adj)
                {
                    byte a = TERM_WHITE;
                    char c = '*';
                    if (abs(adj) < 10)
                        c = '0' + abs(adj);

                    if (adj < 0)
                        a = TERM_RED;
                    else
                    {
                        a = TERM_L_GREEN;
                        if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                            a = TERM_GREEN;
                    }
                    if (slipping)
                    {
                        switch (a)
                        {
                            case TERM_L_GREEN: a = TERM_L_BLUE; break;
                            case TERM_GREEN: a = TERM_BLUE; break;
                            case TERM_RED: a = TERM_L_RED; break;
                            default: break;
                        }
                    }
                    doc_insert_char(doc, a, c);
                }
                else
                {
                    byte a = TERM_L_DARK;
                    char c = '.';

                    if (have_flag(flagzilla->obj_flgs[j], sust_flg))
                    {
                        a = TERM_GREEN;
                        c = 's';
                    }
                    doc_insert_char(doc, a, c);
                }
                if (!slipping) e_adj += adj;
            }
            else
                doc_insert_char(doc, TERM_L_DARK, '.');
        }
        /* @ */
        if (stats[i] + tim_stats[i] != 0)
        {
            byte a = TERM_WHITE;
            char c = '*';
            int  adj = stats[i] + tim_stats[i];

            if (abs(adj) < 10)
                c = '0' + abs(adj);

            if (tim_stats[i])
                a = TERM_YELLOW;
            else if (adj > 0)
            {
                a = TERM_L_GREEN;
                if (have_flag(flagzilla->tim_py_flgs, sust_flg) || have_flag(flagzilla->py_flgs, sust_flg))
                    a = TERM_GREEN;
            }
            else if (adj < 0)
                a = TERM_RED;

            doc_insert_char(doc, a, c);
        }
        else
        {
            byte a = TERM_L_DARK;
            char c = '.';

            if (have_flag(flagzilla->tim_py_flgs, sust_flg))
            {
                a = TERM_YELLOW;
                c = 's';
            }
            else if (have_flag(flagzilla->py_flgs, sust_flg))
            {
                a = TERM_GREEN;
                c = 's';
            }
            doc_insert_char(doc, a, c);
        }

        /* Base R C P E */
        cnv_stat(p_ptr->stat_max[i], buf);
        doc_printf(doc, " <color:B>%6.6s%3d%3d%3d%3d</color>",
                    buf, race_ptr->stats[i], class_ptr->stats[i], pers_ptr->stats[i], e_adj);

        /* Total */
        cnv_stat(p_ptr->stat_top[i], buf);
        doc_printf(doc, " <color:%c>%6.6s</color>", _stat_color(i, 1), buf);

        /* Current */
        if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
        {
            cnv_stat(p_ptr->stat_use[i], buf);
            doc_printf(doc, " <color:%c>%6.6s</color>", _stat_color(i, 2), buf);
        }

        doc_newline(doc);
    }
    doc_newline(doc);
}

static void _build_equipment(doc_ptr doc)
{
    bool old_use_graphics = use_graphics;

    /* It always bugged me that equippy characters didn't show in character dumps!*/
    if (equippy_chars && old_use_graphics)
    {
        use_graphics = FALSE;
        reset_visuals();
    }

    if (equip_count_used())
    {
        int slot;
        char o_name[MAX_NLEN];
        _flagzilla_ptr flagzilla = 0;

        doc_insert(doc, "<topic:Equipment>=============================== 角色装备(<color:keypress>E</color>) ===============================\n\n");
        for (slot = 1; slot <= equip_max(); slot++)
        {
            object_type *o_ptr = equip_obj(slot);
            if (!o_ptr) continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED);
            doc_printf(doc, " %c) <indent><style:indent>%s</style></indent>\n", slot - 1 + 'a', o_name);
            if (((always_dump_origins) || ((final_dump_origins) && ((p_ptr->total_winner) || (p_ptr->is_dead))))
              && (o_ptr->origin_type != ORIGIN_NONE) && (o_ptr->origin_type != ORIGIN_MIXED)
              && (!prace_is_(RACE_MON_SWORD)) && (!prace_is_(RACE_MON_ARMOR)) && (!prace_is_(RACE_MON_RING)))
            {
                doc_printf(doc, "    <indent><style:indent><color:W>");
                (void)display_origin(o_ptr, doc);
                doc_printf(doc, "</color></style></indent>\n");
            }
        }
        doc_newline(doc);


        /* Flags */
        flagzilla = _flagzilla_alloc();
        {
            doc_ptr cols[2];

            cols[0] = doc_alloc(40);
            cols[1] = doc_alloc(40);
            _build_flags1(cols[0], flagzilla);
            _build_flags2(cols[1], flagzilla);
            doc_insert_cols(doc, cols, 2, 0);
            doc_free(cols[0]);
            doc_free(cols[1]);
        }

        /* Stats */
        _build_stats(doc, flagzilla);

        _flagzilla_free(flagzilla);
    }

    if (equippy_chars && old_use_graphics)
    {
        use_graphics = TRUE;
        reset_visuals();
    }
}

/****************************** Combat ************************************/
static void _build_melee(doc_ptr doc)
{
    if (p_ptr->prace == RACE_MON_RING) return;
    if (possessor_can_attack() && !p_ptr->weapon_ct && !p_ptr->innate_attack_ct) return;
    {
        int i;
        doc_insert(doc, "<topic:Melee>==================================== 近战(<color:keypress>M</color>) ====================================\n\n");
        for (i = 0; i < MAX_HANDS; i++)
        {
            if (p_ptr->weapon_info[i].wield_how == WIELD_NONE) continue;
            if (p_ptr->weapon_info[i].bare_hands)
                monk_display_attack_info(doc, i);
            else
                display_weapon_info(doc, i);
        }

        for (i = 0; i < p_ptr->innate_attack_ct; i++)
        {
            display_innate_attack_info(doc, i);
        }
    }
}

static void _build_shooting(doc_ptr doc)
{
    if (equip_find_obj(TV_BOW, SV_ANY) && !prace_is_(RACE_MON_JELLY) && p_ptr->shooter_info.tval_ammo != TV_NO_AMMO)
    {
        doc_insert(doc, "<topic:Shooting>=================================== 射击(<color:keypress>S</color>) ==================================\n\n");
        display_shooter_info(doc);
    }
}

/****************************** Magic ************************************/
void py_display_powers(doc_ptr doc, power_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;
    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    doc_printf(doc, "<topic:Powers>==================================== 能力(<color:keypress>P</color>) ===================================\n\n");
    doc_printf(doc, "<style:table><color:G>名称<tab:22>等级 消耗 失败率 描述<tab:50>施放 失败率</color>\n");
    for (i = 0; i < ct; i++)
    {
        power_info     *power = &table[i];
        spell_info     *spell = &power->spell;
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
        spell->fn(SPELL_COST_EXTRA, &vc);
        spell->fn(SPELL_FAIL_MIN, &vfm);

        doc_printf(doc, "%s<tab:22>%3d %4d %3d%% %s<tab:50>%4d %4d %3d%%\n",
            var_get_string(&vn),
            spell->level, calculate_cost(spell->cost + var_get_int(&vc)), MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail,
            spell_stats_fail(stats)
        );
    }
    doc_insert(doc, "</style>");

    var_clear(&vn);
    var_clear(&vd);
    var_clear(&vc);
    var_clear(&vfm);

    doc_newline(doc);
}

static void _build_powers(doc_ptr doc)
{
    power_info spells[MAX_SPELLS];
    int ct = get_power_table(spells);

    py_display_powers(doc, spells, ct);
}

void py_display_spells(doc_ptr doc, power_info *table, int ct)
{
    if (!ct) return;
    doc_printf(doc, "<topic:Spells>=================================== 法术(<color:keypress>S</color>) ====================================\n\n");
    py_display_spells_aux(doc, table, ct);
}

void py_dump_spells(doc_ptr doc)
{
    power_info spells[MAX_SPELLS];
    class_t *class_ptr = get_class();
    int ct = 0;
    if (class_ptr->get_spells)
        ct += get_spells_aux(spells, MAX_SPELLS, class_ptr->get_spells, TRUE);
    else if (class_ptr->get_spells_fn)
        ct += get_spells_aux(spells, MAX_SPELLS, class_ptr->get_spells_fn(), TRUE);

    if (ct > 0) py_display_spells(doc, spells, ct);
}

void py_dump_spells_aux(doc_ptr doc)
{
    power_info spells[MAX_SPELLS];
    class_t *class_ptr = get_class();
    int ct = 0;
    if (class_ptr->get_spells)
        ct += get_spells_aux(spells, MAX_SPELLS, class_ptr->get_spells, TRUE);
    else if (class_ptr->get_spells_fn)
        ct += get_spells_aux(spells, MAX_SPELLS, class_ptr->get_spells_fn(), TRUE);

    if (ct > 0) py_display_spells_aux(doc, spells, ct);
}

void py_display_spells_aux(doc_ptr doc, power_info *table, int ct)
{
    int i;
    variant vn, vd, vc, vfm;

    if (!ct) return;

    var_init(&vn);
    var_init(&vd);
    var_init(&vc);
    var_init(&vfm);

    if (prace_is_(RACE_MON_MUMMY))
        doc_printf(doc, "<style:table><color:G>名称<tab:30>等级 消耗 失败率 描述<tab:59>施放 失败率</color>\n");
    else
        doc_printf(doc, "<style:table><color:G>名称<tab:30>等级 消耗 失败率 描述<tab:56>施放 失败率</color>\n");

    for (i = 0; i < ct; i++)
    {
        spell_info     *spell = &table[i].spell;
        spell_stats_ptr stats = spell_stats(spell);

        spell->fn(SPELL_NAME, &vn);
        spell->fn(SPELL_INFO, &vd);
/*      spell->fn(SPELL_COST_EXTRA, &vc); */
        spell->fn(SPELL_FAIL_MIN, &vfm);

        if (prace_is_(RACE_MON_MUMMY))
        {
            doc_printf(doc, " %c) %s<tab:30>%3d %4d %3d%% %s<tab:59>%5d %4d %3d%%\n",
            multicase[i],
            var_get_string(&vn),
            spell->level, spell->cost, MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail, spell_stats_fail(stats));
        }
        else doc_printf(doc, " %c) %s<tab:30>%3d %4d %3d%% %s<tab:56>%5d %4d %3d%%\n",
            multicase[i],
            var_get_string(&vn),
            spell->level, spell->cost, MAX(spell->fail, var_get_int(&vfm)),
            var_get_string(&vd),
            stats->ct_cast, stats->ct_fail,
            spell_stats_fail(stats)
        );
    }
    doc_insert(doc, "</style>");

    var_clear(&vn);
    var_clear(&vd);
    var_clear(&vc);
    var_clear(&vfm);

    doc_newline(doc);
}

static void _build_spells(doc_ptr doc)
{
    power_info spells[MAX_SPELLS];
    int        ct = get_spell_table(spells, MAX_SPELLS, FALSE);

    py_display_spells(doc, spells, ct);
}

/****************************** Miscellaneous ************************************/
static void _build_race_history(doc_ptr doc)
{
    if (p_ptr->old_race1 || p_ptr->old_race2 || p_ptr->old_race3)
    {
        int i;
        const char *slaji = get_race_aux(p_ptr->start_race, 0)->name;

        if (p_ptr->psex == p_ptr->start_sex) doc_printf(doc, "\n 你出生时是一个%s%s。\n", is_a_vowel(slaji[0]) ? "an" : "a", slaji);
        else doc_printf(doc, "\n 你出生时是一个%s%s。\n", p_ptr->start_sex == SEX_FEMALE ? "女性" : "男性", slaji);
        for (i = 0; i < MAX_RACES; i++)
        {
            if (p_ptr->start_race == i) continue;
            if (i < 32)
            {
                if (!(p_ptr->old_race1 & 1L << i)) continue;
            }
            else if (i < 64)
            {
                if (!(p_ptr->old_race2 & 1L << (i-32))) continue;
            }
            else
            {
                if (!(p_ptr->old_race3 & 1L << (i-64))) continue;
            }
            {
                const char *laji = get_race_aux(i, 0)->name;
                doc_printf(doc, "你以前是一个%s%s。\n", is_a_vowel(laji[0]) ? "an" : "a", laji);
            }
        }
        doc_newline(doc);
    }
}

static int _compare_monsters_counts(monster_race *left, monster_race *right)
{
    if (left->r_pkills < right->r_pkills)
        return -1;
    if (left->r_pkills > right->r_pkills)
        return 1;
    if (left->level < right->level)
        return -1;
    if (left->level > right->level)
        return 1;
    if (left->mexp < right->mexp)
        return -1;
    if (left->mexp > right->mexp)
        return 1;
    return 0;
}
static int _compare_monsters(monster_race *left, monster_race *right)
{
    if (left->level < right->level)
        return -1;
    if (left->level > right->level)
        return 1;
    if (left->mexp < right->mexp)
        return -1;
    if (left->mexp > right->mexp)
        return 1;
    if (left->r_pkills < right->r_pkills)
        return -1;
    if (left->r_pkills > right->r_pkills)
        return 1;
    return 0;
}
static void _build_uniques(doc_ptr doc)
{
    int ct = ct_kills();

    if (ct)
    {
        int     i;
        vec_ptr v = vec_alloc(NULL);
        int     ct_uniques_dead, ct_uniques_alive = 0;

        doc_printf(doc, "<topic:Kills>================================ 怪物击杀(<color:keypress>K</color>) ================================\n\n");

        for (i = 0; i < max_r_idx; i++)
        {
            monster_race *r_ptr = &r_info[i];
            if (!r_ptr->name) continue;
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                if (r_ptr->max_num == 0)
                    vec_add(v, r_ptr);
                /* When playing with reduce_uniques, it is helpful to know just
                 * how many you are dealing with. Skip the Arena uniques and any
                 * uniques suppressed this game, but count up the rest. */
                else if ( 0 < r_ptr->rarity && r_ptr->rarity <= 100
                       && !(r_ptr->flagsx & RFX_SUPPRESS) && ((!r_ptr->dungeon) ||
                        (!(d_info[r_ptr->dungeon].flags1 & DF1_SUPPRESSED))))
                {
                    ct_uniques_alive++;
                }
            }
        }

        ct_uniques_dead = vec_length(v);
        if (ct_uniques_dead)
        {
            doc_printf(doc, "你总共击败了 %d 个%s，其中包括 %d 个唯一怪%s",
                ct, ct == 1 ? "敌人" : "敌人",
                ct_uniques_dead, ct_uniques_dead == 1 ? "" : "s");

            if ((coffee_break == SPEED_INSTA_COFFEE) && (p_ptr->lv_kills))
            {
                doc_printf(doc, "，以及这一层的 %d 个怪物%s。", p_ptr->lv_kills, p_ptr->lv_kills == 1 ? "" : "s");
            }
            else doc_printf(doc, ". ");

            if (ct_uniques_alive == 1)
                doc_insert(doc, "还剩下 1 个唯一怪。");
            else
                doc_printf(doc, "还剩下 %d 个唯一怪。", ct_uniques_alive);

            doc_insert(doc, "\n\n");

            vec_sort(v, (vec_cmp_f)_compare_monsters);

            doc_printf(doc, "<style:table>  <color:G>唯一怪<tab:46><color:R>等级</color></color>\n");
            for (i = ct_uniques_dead - 1; i >= 0 && i >= ct_uniques_dead - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(doc, "  %s<tab:46>%3d\n", monster_race_display_name(r_ptr->id), r_ptr->level);
            }
            doc_insert(doc, "</style>");
        }
        else
            doc_printf(doc,"你击败了 %d 个%s。\n", ct, ct == 1 ? "敌人" : "敌人");

        doc_newline(doc);


        vec_clear(v);
        for (i = 0; i < max_r_idx; i++)
        {
            monster_race *r_ptr = &r_info[i];
            if (!r_ptr->name) continue;
            if (!(r_ptr->flags1 & RF1_UNIQUE))
            {
                if (r_ptr->r_pkills)
                    vec_add(v, r_ptr);
            }
        }
        ct = vec_length(v);
        if (ct)
        {
            doc_ptr cols[2] = {0};

            cols[0] = doc_alloc(40);
            cols[1] = doc_alloc(40);

            vec_sort(v, (vec_cmp_f)_compare_monsters);

            doc_printf(cols[0], "<style:table>  <color:G>普通怪物<tab:27><color:R>等级</color>  数量</color>\n");
            for (i = ct - 1; i >= 0 && i >= ct - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(cols[0], "  %s<tab:27>%3d %5d\n", monster_race_display_name(r_ptr->id), r_ptr->level, r_ptr->r_pkills);
            }
            doc_insert(cols[0], "</style>");
            doc_newline(cols[0]);

            vec_sort(v, (vec_cmp_f)_compare_monsters_counts);
            doc_printf(cols[1], "<style:table>  <color:G>普通怪物<tab:27>等级  <color:R>数量</color></color>\n");
            for (i = ct - 1; i >= 0 && i >= ct - 20; i--)
            {
                monster_race *r_ptr = vec_get(v, i);
                doc_printf(cols[1], "  %s<tab:27>%3d %5d\n", monster_race_display_name(r_ptr->id), r_ptr->level, r_ptr->r_pkills);
            }
            doc_insert(cols[1], "</style>");
            doc_newline(cols[1]);

            doc_insert_cols(doc, cols, 2, 0);
            doc_free(cols[0]);
            doc_free(cols[1]);
        }

        vec_free(v);
    }
}

static void _build_virtues(doc_ptr doc)
{
    if (enable_virtues)
    {	
        doc_printf(doc, "<topic:Virtues>=================================== 美德(<color:keypress>V</color>) ===================================\n\n");
        virtue_display(doc);
        doc_newline(doc);
    }	
}

static void _build_mutations(doc_ptr doc)
{
    if (mut_count(NULL))
    {
        doc_printf(doc, "<topic:Mutations>================================== 突变(<color:keypress>M</color>) ==================================\n\n");
        mut_display(doc);
        doc_newline(doc);
    }
}

static void _build_pets(doc_ptr doc)
{
    int i;
    bool pet = FALSE;
    bool pet_settings = FALSE;
    char pet_name[MAX_NLEN];

    for (i = m_max - 1; i >= 1; i--)
    {
        monster_type *m_ptr = &m_list[i];

        if (!m_ptr->r_idx) continue;
        if (!is_pet(m_ptr)) continue;

        pet_settings = TRUE;
        /*if (!m_ptr->nickname && (p_ptr->riding != i)) continue;*/
        if (!pet)
        {
            doc_printf(doc, "<topic:Pets>==================================== 宠物(<color:keypress>P</color>) =====================================\n\n");
            doc_printf(doc, "<color:G>主要宠物</color>\n");
            pet = TRUE;
        }
        monster_desc(pet_name, m_ptr, MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE | MD_NO_PET_ABBREV);
        doc_printf(doc, "  <indent><style:indent>%s</style></indent>\n", pet_name);
    }

    if (pet_settings)
    {
        doc_printf(doc, "\n <color:G>选项</color>\n");
        doc_printf(doc, "宠物可以开门: %s\n", (p_ptr->pet_extra_flags & PF_OPEN_DOORS) ? "开启" : "关闭");
        doc_printf(doc, "宠物可以拾取物品: %s\n", (p_ptr->pet_extra_flags & PF_PICKUP_ITEMS) ? "开启" : "关闭");
        doc_printf(doc, "允许宠物传送: %s\n", (p_ptr->pet_extra_flags & PF_TELEPORT) ? "开启" : "关闭");
        doc_printf(doc, "允许宠物施放攻击法术: %s\n", (p_ptr->pet_extra_flags & PF_ATTACK_SPELL) ? "开启" : "关闭");
        doc_printf(doc, "允许宠物施放召唤法术: %s\n", (p_ptr->pet_extra_flags & PF_SUMMON_SPELL) ? "开启" : "关闭");
        doc_printf(doc, "允许范围法术波及玩家: %s\n", (p_ptr->pet_extra_flags & PF_BALL_SPELL) ? "开启" : "关闭");
        if (p_ptr->wizard || easy_damage)
        {
            doc_printf(doc, "骑乘技能: %d\n", skills_riding_current());
        }

        doc_newline(doc);
    }
}

static void _build_allies(doc_ptr doc)
{
    int i;
    int ally_counter = 0, guardian_counter = 0;
    vec_ptr v = vec_alloc(NULL);

    if (p_ptr->lev < 2)
    {
        vec_free(v);
        return;
    }

    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
        if (!unique_is_friend(i)) continue;
        vec_add(v, r_ptr);
        ally_counter++; 
        if (r_ptr->flags7 & RF7_GUARDIAN)
        {
            int j;
            for (j = 1; j < max_d_idx; j++)
            {
                dungeon_info_type *d_ptr = &d_info[j];
                if ((!d_ptr) || (!d_ptr->final_guardian)) continue;
                if (r_ptr->name == r_info[d_ptr->final_guardian].name)
                {
                    guardian_counter++;
                    break;
                }
            }
        }
    }

    if (ally_counter)
    {
        doc_printf(doc, "<topic:Allies>=================================== 盟友(<color:keypress>A</color>) ====================================\n\n");

        if ((!guardian_counter) || (ally_counter == 1))
        {
            doc_printf(doc, "<color:w>%d 个唯一怪%s与你结盟：\n\n", ally_counter, (ally_counter == 1) ? "</color>" : "</color>");
        }
        else
        {
            doc_printf(doc, "%d 个唯一怪，包括 <color:w>%d 个地下城守卫%s</color> 与你结盟：\n\n", ally_counter, guardian_counter, (guardian_counter == 1) ? "," : "，");
        }

        doc_printf(doc, "<style:table><color:G>唯一怪<tab:44>等级</color>\n");
        vec_sort(v, (vec_cmp_f)_compare_monsters);

        for (i = ally_counter - 1; i >= 0; i--)
        {
            monster_race *r_ptr = vec_get(v, i);
            bool osuma = FALSE;
            int j;

            doc_printf(doc, "  %s<tab:44>%3d", monster_race_display_name(r_ptr->id), r_ptr->level);
            if (r_ptr->flags7 & RF7_GUARDIAN)
            {
                for (j = 1; j < max_d_idx && !osuma; j++)
                {
                    dungeon_info_type *d_ptr = &d_info[j];
                    if ((!d_ptr) || (!d_ptr->final_guardian)) continue;
                    if (r_ptr->name == r_info[d_ptr->final_guardian].name)
                    {
                        doc_printf(doc, " (%s)", dungeon_display_name(j));
                        osuma = TRUE;
                    }
                }
            }
            if (!osuma)
            {
                vec_ptr qv = quests_get_random();
                for (j = 0; j < vec_length(qv) && !osuma; j++)
                {
                    quest_ptr q_ptr = vec_get(qv, j);
                    if ((!q_ptr) || (q_ptr->goal != QG_KILL_MON)) continue;
                    if (r_ptr->name == r_info[q_ptr->goal_idx].name)
                    {
                        doc_printf(doc, "(安格班 %d)", q_ptr->level);
                        osuma = TRUE;
                    }
                }
                vec_free(qv);
            }
            doc_newline(doc);
        }
        doc_insert(doc, "</style>");

        doc_newline(doc);

    }
    vec_free(v);
}

/****************************** Objects ************************************/
static void _build_inventory(doc_ptr doc)
{
    slot_t slot;
    char o_name[MAX_NLEN];

    doc_printf(doc, "<topic:Inventory>=============================== 角色物品栏(<color:keypress>I</color>) ==============================\n\n");

    for (slot = 1; slot <= pack_max(); slot++)
    {
        obj_ptr obj = pack_obj(slot);
        if (!obj) continue;
        object_desc(o_name, obj, OD_COLOR_CODED);
        doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", o_name);
    }

    doc_newline(doc);
}

static void _build_quiver(doc_ptr doc)
{
    if (quiver_count(NULL))
    {
        slot_t slot;
        char o_name[MAX_NLEN];

        doc_printf(doc, "<topic:vQuiver>=============================== 角色箭袋(<color:keypress>v</color>) ===============================\n\n");

        for (slot = 1; slot <= quiver_max(); slot++)
        {
            obj_ptr obj = quiver_obj(slot);
            if (!obj) continue;
            object_desc(o_name, obj, OD_COLOR_CODED);
            doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", o_name);
        }

        doc_newline(doc);
    }
}

static void _build_home(doc_ptr doc)
{
    if (home_count(NULL))
    {
        doc_printf(doc, "<topic:Home>================================ 家里物品(<color:keypress>H</color>) ===============================\n");
        doc_newline(doc);
        home_display(doc, obj_exists, 0);
        doc_newline(doc);
    }
}

static void _build_museum(doc_ptr doc)
{
    if (museum_count(NULL))
    {
        doc_printf(doc, "<topic:Museum>==================================== 博物馆(<color:keypress>M</color>) ===================================\n");
        doc_newline(doc);
        museum_display(doc, obj_exists, 0);
        doc_newline(doc);
    }
}

/****************************** Statistics ************************************/
static void _object_counts_imp(doc_ptr doc, int tval, int sval)
{
    int          k_idx = lookup_kind(tval, sval);
    object_kind *k_ptr = &k_info[k_idx];

    if (k_ptr->counts.found || k_ptr->counts.bought || k_ptr->counts.used || k_ptr->counts.destroyed)
    {
        doc_printf(
            doc,
            "  %s<tab:22>%5d %6d %5d %5d\n",
            k_name + k_ptr->name,
            k_ptr->counts.found,
            k_ptr->counts.bought,
            k_ptr->counts.used,
            k_ptr->counts.destroyed
        );
    }
}

static void _device_counts_imp(doc_ptr doc, int tval, int effect)
{
    device_effect_info_ptr entry = device_get_effect_info(tval, effect);

    if (!entry)
        return;

    if (entry->counts.found || entry->counts.bought || entry->counts.used || entry->counts.destroyed)
    {
        effect_t effect;

        effect.power = entry->level;
        effect.difficulty = entry->level;
        effect.type = entry->type;
        effect.extra = 0;

        doc_printf(
            doc,
            "  %s<tab:22>%5d %6d %5d %5d\n",
            do_effect(&effect, SPELL_NAME, 0),
            entry->counts.found,
            entry->counts.bought,
            entry->counts.used,
            entry->counts.destroyed
        );
    }
}

typedef bool (*_kind_p)(int k_idx);
bool _kind_is_third_book(int k_idx) {
    if (k_info[k_idx].tval == TV_ARCANE_BOOK) return FALSE;
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END
      && k_info[k_idx].sval == 2 )
    {
        return TRUE;
    }
    return FALSE;
}
bool _kind_is_fourth_book(int k_idx) {
    if (k_info[k_idx].tval == TV_ARCANE_BOOK) return FALSE;
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END
      && k_info[k_idx].sval == 3 )
    {
        return TRUE;
    }
    return FALSE;
}

static bool _kind_is_equipment(int i) {
    int tval = k_info[i].tval;
    if ( kind_is_weapon(i)
      || tval == TV_SHIELD
      || tval == TV_BOW
      || tval == TV_RING
      || tval == TV_AMULET
      || tval == TV_LITE
      || kind_is_body_armor(i)
      || tval == TV_HELM
      || tval == TV_CLOAK
      || kind_is_helm(i)
      || tval == TV_GLOVES
      || tval == TV_BOOTS )
    {
        return TRUE;
    }
    return FALSE;
}

static bool _kind_is_device(int i) {
    int tval = k_info[i].tval;
    if ( tval == TV_WAND
      || tval == TV_ROD
      || tval == TV_STAFF
      || tval == TV_POTION
      || tval == TV_SCROLL )
    {
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_corpse(int k_idx) {
    if (k_info[k_idx].tval == TV_CORPSE && k_info[k_idx].sval == SV_CORPSE)
        return TRUE;
    return FALSE;
}
static bool _kind_is_skeleton(int k_idx) {
    if (k_info[k_idx].tval == TV_CORPSE && k_info[k_idx].sval == SV_SKELETON)
        return TRUE;
    return FALSE;
}
static bool _kind_is_spellbook(int k_idx) {
    if ( TV_BOOK_BEGIN <= k_info[k_idx].tval
      && k_info[k_idx].tval <= TV_BOOK_END )
    {
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_other(int k_idx) {
    int tval = k_info[k_idx].tval;
    if ( tval == TV_FOOD
      || _kind_is_corpse(k_idx)
      || _kind_is_skeleton(k_idx)
      || _kind_is_spellbook(k_idx)
      || tval == TV_SHOT
      || tval == TV_ARROW
      || tval == TV_BOLT 
      || tval == TV_CHEST )
    {
        return TRUE;
    }
    return FALSE;
}

static void _group_counts_imp(doc_ptr doc, _kind_p p, cptr text)
{
    int i;
    counts_t totals = {0};
    for (i = 0; i < max_k_idx; i++)
    {
        if (p(i))
        {
            totals.generated += k_info[i].counts.generated;
            totals.found += k_info[i].counts.found;
            totals.bought += k_info[i].counts.bought;
            totals.used += k_info[i].counts.used;
            totals.destroyed += k_info[i].counts.destroyed;
        }
    }

    if (totals.found || totals.bought || totals.used || totals.destroyed)
    {
        doc_printf(
            doc,
            "  %s<tab:22>%5d %6d %5d %5d\n",
            text,
            totals.found,
            totals.bought,
            totals.used,
            totals.destroyed
        );
    }
}

static void _group_counts_tval_imp(doc_ptr doc, int tval, cptr text)
{
    int i;
    counts_t totals = {0};
    for (i = 0; i < max_k_idx; i++)
    {
        if (k_info[i].tval == tval)
        {
            totals.generated += k_info[i].counts.generated;
            totals.found += k_info[i].counts.found;
            totals.bought += k_info[i].counts.bought;
            totals.used += k_info[i].counts.used;
            totals.destroyed += k_info[i].counts.destroyed;
        }
    }

    if (totals.found || totals.bought || totals.used || totals.destroyed)
    {
        doc_printf(
            doc,
            "  %s<tab:22>%5d %6d %5d %5d\n",
            text,
            totals.found,
            totals.bought,
            totals.used,
            totals.destroyed
        );
    }
}

static void _ego_counts_imp(doc_ptr doc, int idx, cptr text)
{
    ego_type *e_ptr = &e_info[idx];

    if (e_ptr->counts.found || e_ptr->counts.bought || e_ptr->counts.destroyed)
    {
        doc_printf(
            doc,
            "  %s<tab:22>%5d %6d %5d\n",
            text,
            e_ptr->counts.found,
            e_ptr->counts.bought,
            e_ptr->counts.destroyed
        );
    }
}

typedef bool (*_mon_p)(int r_idx);
static bool _mon_is_animal(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_ANIMAL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_breeder(int r_idx)
{
    if (r_info[r_idx].flags2 & RF2_MULTIPLY)
        return TRUE;
    return FALSE;
}
static bool _mon_is_demon(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_DEMON)
        return TRUE;
    return FALSE;
}
static bool _mon_is_dragon(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_DRAGON)
        return TRUE;
    return FALSE;
}
static bool _mon_is_evil(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_EVIL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_giant(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GIANT)
        return TRUE;
    return FALSE;
}
static bool _mon_is_good(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GOOD)
        return TRUE;
    return FALSE;
}
static bool _mon_is_hound(int r_idx)
{
    if (r_info[r_idx].d_char == 'Z')
        return TRUE;
    return FALSE;
}
static bool _mon_is_human(int r_idx)
{
    if (r_info[r_idx].flags2 & RF2_HUMAN)
        return TRUE;
    return FALSE;
}
static bool _mon_is_neutral(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_GOOD)
        return FALSE;
    if (r_info[r_idx].flags3 & RF3_EVIL)
        return FALSE;
    return TRUE;
}
static bool _mon_is_orc(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_ORC)
        return TRUE;
    return FALSE;
}
static bool _mon_is_troll(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_TROLL)
        return TRUE;
    return FALSE;
}
static bool _mon_is_undead(int r_idx)
{
    if (r_info[r_idx].flags3 & RF3_UNDEAD)
        return TRUE;
    return FALSE;
}
static bool _mon_is_unique(int r_idx)
{
    if (r_info[r_idx].flags1 & RF1_UNIQUE)
        return TRUE;
    return FALSE;
}
static bool _mon_is_pact(int r_idx)
{
    return warlock_is_pact_monster(&r_info[r_idx]);
}
static void _kill_counts_imp(doc_ptr doc, _mon_p p, cptr text, int total)
{
    int i;
    int kills = 0;
    for (i = 0; i < max_r_idx; i++)
    {
        if (p(i))
        {
            if (_mon_is_unique(i))
            {
                if (r_info[i].max_num == 0)
                    kills++;   /* Perhaps The Cloning Pits is messing up r_akills? */
            }
            else
                kills += r_info[i].r_pkills;
        }
    }

    if (kills)
    {
        doc_printf(
            doc,
            "  %s<tab:22>%5d %3d.%1d%%\n",
            text,
            kills,
            kills*100/total,
            (kills*1000/total)%10
        );
    }
}

static void _build_mon_kill_stats(doc_ptr doc)
{
    int total_kills = ct_kills_all();
    doc_printf(doc, "<color:G>怪物类别 击杀数 占比</color>\n");
    _kill_counts_imp(doc, _mon_is_animal, "动物", total_kills);
    _kill_counts_imp(doc, _mon_is_breeder, "增殖者", total_kills);
    _kill_counts_imp(doc, _mon_is_demon, "恶魔", total_kills);
    _kill_counts_imp(doc, _mon_is_dragon, "龙类", total_kills);
    _kill_counts_imp(doc, _mon_is_giant, "巨怪", total_kills);
    _kill_counts_imp(doc, _mon_is_hound, "猎犬", total_kills);
    _kill_counts_imp(doc, _mon_is_human, "人类", total_kills);
    _kill_counts_imp(doc, _mon_is_orc, "兽人", total_kills);
    _kill_counts_imp(doc, _mon_is_troll, "巨魔", total_kills);
    _kill_counts_imp(doc, _mon_is_undead, "不死生物", total_kills);
    _kill_counts_imp(doc, _mon_is_unique, "唯一怪", total_kills);
    if (p_ptr->pclass == CLASS_WARLOCK)
        _kill_counts_imp(doc, _mon_is_pact, "契约怪物", total_kills);
    doc_newline(doc);
    _kill_counts_imp(doc, _mon_is_evil, "邪恶怪物", total_kills);
    _kill_counts_imp(doc, _mon_is_good, "善良怪物", total_kills);
    _kill_counts_imp(doc, _mon_is_neutral, "中立怪物", total_kills);
    doc_printf(doc, "\n  总计<tab:22>%5d\n", total_kills);
}

static void _spell_count_imp(doc_ptr doc, cptr heading, int ct, int total)
{
    if (!total) return;
    doc_printf(doc, "  %s<tab:22>%5d %3d.%1d%%\n", heading, ct,
        ct*100/total,
        (ct*1000/total)%10);
}
static void _build_mon_spell_stats(doc_ptr doc, cptr heading, mon_race_p filter)
{
    int ct_total_moves = 0;
    int ct_spell_moves = 0;
    int ct_spells = 0;
    int total_freq = 0;
    double expected_freq;
    int expected_spells;
    int allocation[MST_COUNT] = {0};
    int i, j, k;

    for (i = 1; i < max_r_idx; i++)
    {
        mon_race_ptr race = &r_info[i];
        int          moves;
        if (!race->name) continue;
        if (!race->spells) continue;
        if (filter && !filter(race)) continue;
        moves = race->r_move_turns + race->r_spell_turns;
        ct_total_moves += moves;
        ct_spell_moves += race->r_spell_turns;
        total_freq += race->spells->freq * moves;
        for (j = 0; j < MST_COUNT; j++)
        {
            mon_spell_group_ptr group = race->spells->groups[j];
            if (!group) continue;
            for (k = 0; k < group->count; k++)
            {
                mon_spell_ptr spell = &group->spells[k];
                allocation[j] += spell->lore;
                ct_spells += spell->lore; /* ct_spell_moves includes spell failures */
            }
        }
    }
    if (!ct_total_moves) return;
    doc_printf(doc, "<color:G>%s<tab:22>数量 占比</color>\n", heading);
    _spell_count_imp(doc, "实际观察", ct_spell_moves, ct_total_moves);
    expected_freq = (double)total_freq/(ct_total_moves * 100.0);
    expected_spells = ct_total_moves * expected_freq; 
    doc_printf(doc, "  期望<tab:22>%5d %5.1f%%\n", expected_spells, expected_freq * 100.0);
    _spell_count_imp(doc, "失败", ct_spell_moves - ct_spells, ct_spell_moves);
    _spell_count_imp(doc, "召唤", allocation[MST_SUMMON], ct_spell_moves);
    _spell_count_imp(doc, "治疗", allocation[MST_HEAL], ct_spell_moves);
    _spell_count_imp(doc, "逃跑", allocation[MST_ESCAPE], ct_spell_moves);
    _spell_count_imp(doc, "攻击",
        allocation[MST_BREATH] + allocation[MST_BALL] + allocation[MST_BOLT]
            + allocation[MST_BEAM] + allocation[MST_CURSE],
        ct_spell_moves);
    _spell_count_imp(doc, "其他",
        allocation[MST_BUFF] + allocation[MST_BIFF] + allocation[MST_ANNOY]
            + allocation[MST_TACTIC] + allocation[MST_WEIRD],
        ct_spell_moves);
}
static bool _is_unique(mon_race_ptr race) { return BOOL(race->flags1 & RF1_UNIQUE); }
/*static bool _is_hound(mon_race_ptr race) { return race->d_char == 'Z'; }*/
/*static bool _is_deep(mon_race_ptr race) { return race->level >= 60; }*/
static void _build_monster_stats(doc_ptr doc)
{
    doc_ptr cols[2];
    cols[0] = doc_alloc(40);
    cols[1] = doc_alloc(40);
    _build_mon_kill_stats(cols[0]);
    _build_mon_spell_stats(cols[1], "法术", NULL);
    doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "专属法术", _is_unique);
    /*doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "Hound Spells", _is_hound);
    doc_newline(cols[1]);
    _build_mon_spell_stats(cols[1], "Deep Spells", _is_deep);*/
    doc_insert_cols(doc, cols, 2, 0);
    doc_free(cols[0]);
    doc_free(cols[1]);
}
static void _build_monster_histogram(doc_ptr doc)
{
    int histogram[20] = {0};
    int i, max_bucket = 0, total = 0, running = 0;

    for (i = 0; i < max_r_idx; i++)
    {
        mon_race_ptr race = &r_info[i];
        int          bucket = MIN(19, race->level/5);
        int          amt = 0;
        if (!race->name) continue;

        if (race->flags1 & RF1_UNIQUE) /* XXX problem with r_pkills and uniques */
        {
            if (race->max_num == 0)
                amt = 1;
        }
        else
            amt = race->r_pkills;

        if (amt)
        {
            histogram[bucket] += amt;
            total += amt;
            max_bucket = MAX(bucket, max_bucket);
        }
    }
    if (!total) return;

    doc_insert(doc, "<color:G>等级 数量</color>\n");
    for (i = 0; i <= max_bucket; i++)
    {
        int min = i*5;
        int max = min + 4;
        int ct = histogram[i];
        running += ct;
        doc_printf(doc, "  %2d - ", min);
        if (i < 19)
            doc_printf(doc, "%2d", max);
        else
            doc_insert(doc, "**");
        doc_printf(doc, " %5d %2d.%02d%% %3d.%02d%%\n", ct,
            ct * 100 / total, (ct * 10000 / total) % 100,
            running * 100 / total, (running * 10000 / total) % 100);
    }
    doc_newline(doc);
}
static void _build_statistics(doc_ptr doc)
{
    int i;
    counts_t totals = {0};

    doc_printf(doc, "<topic:Statistics>================================== 统计(<color:keypress>S</color>) =================================\n\n");

    /* Gold */
    doc_insert(doc, "<color:y> 金币</color>\n");
    doc_printf(doc, "找到 : <color:w>%8d</color>\n", stats_gold_counts.found);
    if (!no_selling)
        doc_printf(doc, "售出 : <color:w>%8d</color>\n", stats_gold_counts.selling);
    else
        doc_printf(doc, "炼金 : <color:w>%8d</color>\n", stats_gold_counts.selling);
    doc_printf(doc, "获胜赢取 : <color:w>%8d</color> <color:G>%8d</color>\n",
        stats_gold_counts.winnings,
        stats_gold_counts.found + stats_gold_counts.selling + stats_gold_counts.winnings);
    doc_printf(doc, "购买 : <color:w>%8d</color>\n", stats_gold_counts.buying);
    doc_printf(doc, "服务消费 : <color:w>%8d</color>\n", stats_gold_counts.services);
    doc_printf(doc, "被偷走 : <color:w>%8d</color> <color:R>%8d</color>\n",
        stats_gold_counts.stolen,
        stats_gold_counts.buying + stats_gold_counts.services + stats_gold_counts.stolen);
    cornucopia_print_stats(doc);
    doc_printf(doc, "                      <color:y>%8d</color>\n\n", p_ptr->au);

    /* Objects */
    for (i = 0; i < max_k_idx; i++)
    {
        totals.generated += k_info[i].counts.generated;
        totals.found += k_info[i].counts.found;
        totals.bought += k_info[i].counts.bought;
        totals.used += k_info[i].counts.used;
        totals.destroyed += k_info[i].counts.destroyed;
    }

    doc_printf(doc, "<color:R>找到物品 :</color> %6d\n", totals.found);
    doc_printf(doc, "<color:R>买入物品 :</color> %6d\n", totals.bought);
    doc_printf(doc, "<color:R>摧毁物品 :</color> %6d\n", totals.destroyed);


    doc_printf(doc, "\n <color:G>装备分类 找到 买入 使用 摧毁</color>\n");
    _group_counts_imp(doc, kind_is_weapon, "武器");
    _group_counts_tval_imp(doc, TV_SHIELD, "盾牌");
    _group_counts_tval_imp(doc, TV_BOW, "弓");
    _group_counts_tval_imp(doc, TV_QUIVER, "箭袋");
    _group_counts_tval_imp(doc, TV_RING, "戒指");
    _group_counts_tval_imp(doc, TV_AMULET, "护身符");
    _group_counts_tval_imp(doc, TV_LITE, "光源");
    _group_counts_imp(doc, kind_is_body_armor, "身体护甲");
    _group_counts_tval_imp(doc, TV_CLOAK, "披风");
    _group_counts_imp(doc, kind_is_helm, "头盔");
    _group_counts_tval_imp(doc, TV_GLOVES, "手套");
    _group_counts_tval_imp(doc, TV_BOOTS, "靴子");
    _group_counts_imp(doc, _kind_is_equipment, "总计");

    doc_printf(doc, "\n <color:G>魔法装置 找到 买入 使用 摧毁</color>\n");
    _group_counts_tval_imp(doc, TV_WAND, "魔杖");
    _group_counts_tval_imp(doc, TV_STAFF, "法杖");
    _group_counts_tval_imp(doc, TV_ROD, "魔棒");
    _group_counts_tval_imp(doc, TV_POTION, "药水");
    _group_counts_tval_imp(doc, TV_SCROLL, "卷轴");
    _group_counts_imp(doc, _kind_is_device, "总计");

    doc_printf(doc, "\n <color:G>其他物品 找到 买入 使用 摧毁</color>\n");
    _group_counts_tval_imp(doc, TV_SHOT, "弹丸");
    _group_counts_tval_imp(doc, TV_ARROW, "箭矢");
    _group_counts_tval_imp(doc, TV_BOLT, "弩箭");
    if (player_is_ninja)
        _group_counts_tval_imp(doc, TV_SPIKE, "铁蒺藜");
    _group_counts_imp(doc, _kind_is_spellbook, "法术书");
    _group_counts_tval_imp(doc, TV_FOOD, "食物");
    _group_counts_imp(doc, _kind_is_corpse, "尸体");
    _group_counts_imp(doc, _kind_is_skeleton, "骨骸");
    _group_counts_tval_imp(doc, TV_CHEST, "箱子");
    _group_counts_imp(doc, _kind_is_other, "总计");

    doc_printf(doc, "\n <color:G>药水种类 找到 买入 使用 摧毁</color>\n");
    _object_counts_imp(doc, TV_POTION, SV_POTION_CURE_CRITICAL);
    _object_counts_imp(doc, TV_POTION, SV_POTION_CURING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_SPEED);
    _object_counts_imp(doc, TV_POTION, SV_POTION_CLARITY);
    _object_counts_imp(doc, TV_POTION, SV_POTION_HEALING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_STAR_HEALING);
    _object_counts_imp(doc, TV_POTION, SV_POTION_LIFE);
    _object_counts_imp(doc, TV_POTION, SV_POTION_RESTORE_MANA);
    _object_counts_imp(doc, TV_POTION, SV_POTION_RESTORE_EXP);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_STR);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_INT);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_WIS);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_DEX);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_CON);
    _object_counts_imp(doc, TV_POTION, SV_POTION_INC_CHR);
    _object_counts_imp(doc, TV_POTION, SV_POTION_LIQUID_LOGRUS);
    _object_counts_imp(doc, TV_POTION, SV_POTION_NEW_LIFE);
    _object_counts_imp(doc, TV_POTION, SV_POTION_EXPERIENCE);
    _group_counts_tval_imp(doc, TV_POTION, "总计");

    doc_printf(doc, "\n <color:G>卷轴种类 找到 买入 使用 摧毁</color>\n");
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_PHASE_DOOR);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_IDENTIFY);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_IDENTIFY);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_REMOVE_CURSE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_REMOVE_CURSE);
    if (class_uses_spell_scrolls(p_ptr->pclass))
        _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_SPELL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_TELEPORT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_TELEPORT_LEVEL);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_DESTRUCTION);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_GENOCIDE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_MASS_GENOCIDE);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_FOREST_CREATION);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_BANISHMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_CRAFTING);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_ACQUIREMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_STAR_ACQUIREMENT);
    _object_counts_imp(doc, TV_SCROLL, SV_SCROLL_ARTIFACT);
    _group_counts_tval_imp(doc, TV_SCROLL, "总计");

    doc_printf(doc, "\n <color:G>魔杖种类 找到 买入 使用 摧毁</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &wand_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_WAND, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_WAND, EFFECT_STONE_TO_MUD);
        _device_counts_imp(doc, TV_WAND, EFFECT_TELEPORT_AWAY);
        _device_counts_imp(doc, TV_WAND, EFFECT_DRAIN_LIFE);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_COLD);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_FIRE);
		_device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_WATER);
        _device_counts_imp(doc, TV_WAND, EFFECT_BREATHE_ONE_MULTIHUED);
        _device_counts_imp(doc, TV_WAND, EFFECT_METEOR);
        _device_counts_imp(doc, TV_WAND, EFFECT_GENOCIDE_ONE);
        _device_counts_imp(doc, TV_WAND, EFFECT_BALL_WATER);
        _device_counts_imp(doc, TV_WAND, EFFECT_BALL_DISINTEGRATE);
        _device_counts_imp(doc, TV_WAND, EFFECT_ROCKET);
        _device_counts_imp(doc, TV_WAND, EFFECT_WALL_BUILDING);
    }
    _group_counts_tval_imp(doc, TV_WAND, "总计");

    doc_printf(doc, "\n <color:G>法杖种类 找到 买入 使用 摧毁</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &staff_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_STAFF, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_STAFF, EFFECT_IDENTIFY);
        _device_counts_imp(doc, TV_STAFF, EFFECT_ENLIGHTENMENT);
        _device_counts_imp(doc, TV_STAFF, EFFECT_CURING);
        _device_counts_imp(doc, TV_STAFF, EFFECT_HEAL);
        _device_counts_imp(doc, TV_STAFF, EFFECT_TELEPATHY);
        _device_counts_imp(doc, TV_STAFF, EFFECT_SPEED);
        _device_counts_imp(doc, TV_STAFF, EFFECT_IDENTIFY_FULL);
        _device_counts_imp(doc, TV_STAFF, EFFECT_DESTRUCTION);
        _device_counts_imp(doc, TV_STAFF, EFFECT_HEAL_CURING);
        _device_counts_imp(doc, TV_STAFF, EFFECT_GENOCIDE);
        _device_counts_imp(doc, TV_STAFF, EFFECT_CONFUSING_LITE);
        _device_counts_imp(doc, TV_STAFF, EFFECT_MANA_STORM);
        _device_counts_imp(doc, TV_STAFF, EFFECT_STARBURST);
        _device_counts_imp(doc, TV_STAFF, EFFECT_DARKNESS_STORM);
        _device_counts_imp(doc, TV_STAFF, EFFECT_RESTORE_MANA);
    }
    _group_counts_tval_imp(doc, TV_STAFF, "总计");

    doc_printf(doc, "\n <color:G>魔棒种类 找到 买入 使用 摧毁</color>\n");
    if (p_ptr->wizard)
    {
        for (i = 0; ; i++)
        {
        device_effect_info_ptr entry = &rod_effect_table[i];

            if (!entry->type) break;
            _device_counts_imp(doc, TV_ROD, entry->type);
        }
    }
    else
    {
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_TRAPS);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_DOOR_STAIRS);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_MONSTERS);
        _device_counts_imp(doc, TV_ROD, EFFECT_LITE_AREA);
        _device_counts_imp(doc, TV_ROD, EFFECT_RECALL);
        _device_counts_imp(doc, TV_ROD, EFFECT_DETECT_ALL);
        _device_counts_imp(doc, TV_ROD, EFFECT_ENLIGHTENMENT);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_SOUND);
        _device_counts_imp(doc, TV_ROD, EFFECT_SPEED_HERO);
        _device_counts_imp(doc, TV_ROD, EFFECT_HEAL_CURING_HERO);
        _device_counts_imp(doc, TV_ROD, EFFECT_RESTORING);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_MANA);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_SHARDS);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_CHAOS);
        _device_counts_imp(doc, TV_ROD, EFFECT_CLAIRVOYANCE);
        _device_counts_imp(doc, TV_ROD, EFFECT_BALL_LITE);
    }
    _group_counts_tval_imp(doc, TV_ROD, "总计");

    doc_printf(doc, "\n <color:G>法术书种类 找到 买入 使用 摧毁</color>\n");
    _group_counts_imp(doc, _kind_is_third_book, "第三法术书");
    _group_counts_imp(doc, _kind_is_fourth_book, "第四法术书");
    _group_counts_imp(doc, kind_is_book, "总计");

    /* Egos */
    WIPE(&totals, counts_t);
    for (i = 0; i < max_e_idx; i++)
    {
        totals.generated += e_info[i].counts.generated;
        totals.found += e_info[i].counts.found;
        totals.bought += e_info[i].counts.bought;
        totals.destroyed += e_info[i].counts.destroyed;
    }

    doc_printf(doc, "\n <color:R>找到Ego装备 :</color> %6d\n", totals.found);
    doc_printf(doc,   "<color:R>买入Ego装备 :</color> %6d\n", totals.bought);
    doc_printf(doc,   "<color:R>摧毁Ego装备 :</color> %6d\n", totals.destroyed);

    doc_printf(doc, "\n <color:G>特殊Ego装备 找到 买入 摧毁</color>\n");
    _ego_counts_imp(doc, EGO_RING_SPEED, "速度戒指");
    _ego_counts_imp(doc, EGO_JEWELRY_DEFENDER, "珠宝 (护卫)");
    _ego_counts_imp(doc, EGO_BOOTS_ELVENKIND, "精灵之靴");
    _ego_counts_imp(doc, EGO_BOOTS_SPRITE, "妖精之靴");
    _ego_counts_imp(doc, EGO_BOOTS_SPEED, "速度之靴");
    _ego_counts_imp(doc, EGO_BOOTS_FEANOR, "费诺之靴");

    doc_newline(doc);
    _build_monster_stats(doc);
    if (0 || p_ptr->wizard)
        _build_monster_histogram(doc);

    if ((p_ptr->is_dead) || (p_ptr->knowledge & KNOW_HPRATE))
    {
        doc_printf(doc, "<color:G>生命评分</color>: %s\n\n", life_rating_desc(TRUE));
    }
}

/****************************** Dungeons ************************************/
typedef dungeon_info_type *dun_ptr;
static int _cmp_d_lvl(dun_ptr l, dun_ptr r)
{
    int ld = (l->id == DUNGEON_MYSTERY) ? max_dlv[l->id] : l->maxdepth;
    int rd = (r->id == DUNGEON_MYSTERY) ? max_dlv[r->id] : r->maxdepth;
    if (ld < rd) return -1;
    if (ld > rd) return 1;
    if (l->id < r->id) return -1;
    if (l->id > r->id) return 1;
    return 0;
}
void py_display_dungeons(doc_ptr doc)
{
    int     i;
    vec_ptr v = vec_alloc(NULL);
    for (i = 1; i < max_d_idx; i++)
    {
        dun_ptr d_ptr = &d_info[i];
        if (!d_ptr->maxdepth) continue;
        if (!max_dlv[i]) continue;
        vec_add(v, d_ptr);
    }
    vec_sort(v, (vec_cmp_f)_cmp_d_lvl);
    for (i = 0; i < vec_length(v); i++)
    {
        bool    conquered;
        dun_ptr d_ptr = vec_get(v, i);

        conquered = dungeon_conquered(d_ptr->id);

        if (conquered)
            doc_printf(doc, "!<color:G>%-16s</color>: 第 %3d 层\n", d_name+d_ptr->name, max_dlv[d_ptr->id]);
        else
            doc_printf(doc, "%-16s: 第 %3d 层\n", d_name+d_ptr->name, max_dlv[d_ptr->id]);
    }
    vec_free(v);
    doc_newline(doc);

    if (p_ptr->is_dead)
    {
        if ((p_ptr->total_winner) && ((strpos("切腹", p_ptr->died_from)) || (strpos("寿终正寝", p_ptr->died_from))))
        {
            doc_printf(doc, "<color:v>你在获胜后%s。</color>\n",
                streq(p_ptr->died_from, "切腹自尽了") ? "切腹自尽了" : "退出了冒险");
        }
        else if (py_on_surface())
        {
            doc_printf(doc, "你在%s被%s杀死了。\n", map_name(), p_ptr->died_from);
        }
        else if (py_in_dungeon())
        {
            doc_printf(doc, "你在%s的第 %d 层被%s杀死了。\n",
                map_name(), dun_level, p_ptr->died_from);
        }
        else if (quests_get_current())
        {
            cptr quest_name;
            doc_printf(doc, "你在任务“%s”中被%s杀死了。\n",
                lyhytnimi(quests_get_current(), &quest_name), p_ptr->died_from);
            free((vptr)quest_name);
        }
        else /* ??? */
        {
            doc_printf(doc, "你在%s被%s杀死了。\n", map_name(), p_ptr->died_from);
        }
    }
    else if (character_dungeon)
    {
        if (py_on_surface())
            doc_printf(doc, "现在，你在%s。\n", map_name());
        else if (py_in_dungeon())
            doc_printf(doc, "现在，你正在探索%s的第 %d 层。\n", map_name(), dun_level);
        else if (quests_get_current())
        {
            cptr quest_name;
            doc_printf(doc, "现在，你正在执行任务“%s”。\n", lyhytnimi(quests_get_current(), &quest_name));
            free((vptr)quest_name);
        }
        else if (p_ptr->inside_arena)
            doc_printf(doc, "现在，你在%s的%s里。\n", map_name(), town_name(p_ptr->town_num));
        else
            doc_insert(doc, "嗯……你在哪里？");
    }

    if (p_ptr->last_message)
    {
        if (p_ptr->is_dead)
            doc_printf(doc, "\n 最后的信息: %s\n", p_ptr->last_message);
        else if (p_ptr->total_winner)
            doc_printf(doc, "\n *获胜*的信息: %s\n", p_ptr->last_message);
    }
    doc_newline(doc);
}

static void _build_dungeons(doc_ptr doc)
{
    doc_printf(doc, "<topic:Dungeons>================================== 地下城(<color:keypress>D</color>) =================================\n\n");
    py_display_dungeons(doc);
}

/****************************** Messages ************************************/
static void _build_messages(doc_ptr doc)
{
    int i;
    int current_turn = 0;
    int current_row = 0;

    doc_insert(doc, "<topic:LastMessages>=============================== 最近的信息(<color:keypress>L</color>) ===============================\n");
    doc_insert(doc, "<style:normal>");
    for (i = MIN(msg_count() - 1, 30); i >= 0; i--)
    {
        msg_ptr m = msg_get(i);

        if (m->turn != current_turn)
        {
            if (doc_cursor(doc).y > current_row + 1)
                doc_newline(doc);
            current_turn = m->turn;
            current_row = doc_cursor(doc).y;
        }

        doc_insert_text(doc, m->color, string_buffer(m->msg));
        if (m->count > 1)
        {
            char buf[10];
            sprintf(buf, " (x%d)", m->count);
            doc_insert_text(doc, m->color, buf);
        }
        doc_newline(doc);
    }
    doc_insert(doc, "</style>\n");
}

/******************************** Options ************************************/
static cptr _game_mode_text[GAME_MODE_MAX] = {
    "<color:G>新手</color>",
    "普通",
    "XXX", 
    "<color:r>怪物</color>"
};
static cptr _game_speed_text[GAME_SPEED_MAX] = {
    "普通",
    "<color:U>茶歇</color>",
    "<color:U>速溶</color>"
};

static void _build_options(doc_ptr doc)
{
    int i, loydetty = 0;
    doc_printf(doc, "<topic:Options>==================================== 选项(<color:keypress>O</color>) ===================================\n\n");

    if (game_mode != GAME_MODE_NORMAL)
        doc_printf(doc, "游戏模式: %s\n", _game_mode_text[game_mode]);

    if (coffee_break)
        doc_printf(doc, "游戏速度: %s\n", _game_speed_text[coffee_break]);

    if ((p_ptr->coffee_lv_revisits) || (coffee_break && p_ptr->total_winner))
    {
        if (!p_ptr->coffee_lv_revisits)
            doc_printf(doc, "深度重访: 无\n");
        else if (p_ptr->coffee_lv_revisits > 250)
            doc_printf(doc, "深度重访: 250+\n");
        else
            doc_printf(doc, "深度重访: %d\n", p_ptr->coffee_lv_revisits);
    }

    if (thrall_mode)
        doc_printf(doc, "奴役模式: 开启\n");

    doc_printf(doc, "保留模式: %s\n", preserve_mode ? "开启" : "关闭");

    if (small_level_type <= SMALL_LVL_MAX)
         doc_printf(doc, "关卡大小: %s\n", lv_size_options[small_level_type]);

    if (easy_damage)
		doc_printf(doc, "简易伤害信息: 开启\n");

	if (easy_id)
		doc_printf(doc, "简易鉴定: 开启\n");
	
	if (easy_lore)
		doc_printf(doc, "简易传说: 开启\n");

    if (no_wilderness)
        doc_printf(doc, "大地图: 关闭\n");

    if (ironman_shops)
        doc_printf(doc, "无商店: 开启\n");

    if ((ironman_downward) && (!coffee_break))
        doc_printf(doc, "强制下潜: 开启\n");

    if (wacky_rooms)
        doc_printf(doc, "搞怪房间: 开启\n");

    if (melee_challenge)
        doc_printf(doc, "纯近战挑战: 开启\n");

    if (no_melee_challenge)
        doc_printf(doc, "无近战挑战: 开启\n");

    if (increase_density)
        doc_printf(doc, "密集小关卡: 开启\n");

    if (no_big_dungeons)
        doc_printf(doc, "大型地下城: 仅限竞技场\n");

    if (ironman_nightmare)
        doc_printf(doc, "<color:v>噩梦模式</color>: 开启\n");

    doc_printf(doc, "竞技场关卡: %s\n", empty_lv_description[generate_empty]);

    doc_printf(doc, "万神殿%s", ((pantheon_count == 1) ? ": " : ":"));
    for (i = 1; i < PANTHEON_MAX; i++)
    {
        if (is_active_pantheon(i))
        {
            if (loydetty) doc_printf(doc, ", ");
            doc_printf(doc, "%s", pant_list[i].name);
            loydetty++;
        }
    }
    doc_printf(doc, "\n");

    if (no_artifacts)
        doc_printf(doc, "无神器模式: 开启\n");
    else if (random_artifacts)
        doc_printf(doc, "随机神器概率: %d%%\n", random_artifact_pct);

    if (no_egos)
        doc_printf(doc, "无Ego模式: 开启\n");

    if (reduce_uniques)
        doc_printf(doc, "减少唯一怪: %d%%\n", reduce_uniques_pct);

    if (no_selling)
        doc_printf(doc, "禁止售卖: 开启\n");

    if (comp_mode)
        doc_printf(doc, "竞赛模式: 开启\n");

    {
        int mult = score_mult();
        doc_printf(doc, "\n 评分乘数: %d.%02d%%\n", mult / 100, mult % 100);
        if (p_ptr->noscore) doc_printf(doc, "调整后评分: %d\n", hof_score());
        else doc_printf(doc, "调整后评分: <color:B>%d</color>\n", hof_score());
    }

    if (p_ptr->noscore)
        doc_printf(doc, "\n <color:v>你做了一些违规的事情(作弊)。</color>\n");

    doc_newline(doc);
}

/****************************** Character Sheet ************************************/
static void _build_quests(doc_ptr doc)
{
    doc_printf(doc, "<topic:uQuests>==================================== 任务(<color:keypress>u</color>) ===================================\n\n");
    quests_doc(doc);
/*     if (!no_wilderness || !ironman_downward || coffee_break)*/
    {
        if (p_ptr->arena_number < 0)
        {
            if (p_ptr->arena_number <= ARENA_DEFEATED_OLD_VER)
            {
                doc_printf(doc, "<color:G>竞技场</color>: <color:v>被击败</color>\n");
            }
            else
            {
                doc_printf(doc, "<color:G>竞技场</color>: 在第 %d%s 场战斗中 <color:v>被</color> %s <color:v>击败</color>\n",
                    -p_ptr->arena_number, get_ordinal_number_suffix(-p_ptr->arena_number),
                    monster_race_display_name(arena_info[-1 - p_ptr->arena_number].r_idx));
            }
        }
        else if (p_ptr->arena_number > MAX_ARENA_MONS + 2)
        {
            doc_printf(doc, "<color:G>竞技场</color>: <color:B>真正的冠军</color>\n");
        }
        else if (p_ptr->arena_number > MAX_ARENA_MONS - 1)
        {
            doc_printf(doc, "<color:G>竞技场</color>: <color:R>冠军</color>\n");
        }
        else
        {
            doc_printf(doc, "<color:G>竞技场</color>: %2d 场胜利%s\n",
                p_ptr->arena_number > MAX_ARENA_MONS ? MAX_ARENA_MONS : p_ptr->arena_number,
                p_ptr->arena_number != 1 ? "ies" : "y");
        }
    }
    doc_newline(doc);
}

static int _max_depth(void)
{
    int result = 0;
    int i;

    for(i = 1; i < max_d_idx; i++)
    {
        if (!d_info[i].maxdepth) continue;
        if (d_info[i].flags1 & DF1_RANDOM) continue;
        if (!max_dlv[i]) continue;
        result = MAX(result, max_dlv[i]);
    }

    return result;
}

static bool _is_retired(void)
{
    if (p_ptr->total_winner && p_ptr->is_dead && strcmp(p_ptr->died_from, "寿终正寝") == 0)
        return TRUE;
    return FALSE;
}

int oook_score(void)
{
    int tulos = p_ptr->max_max_exp;
    if ((easy_damage) && ((p_ptr->total_winner) || (tulos > 6500000L))) tulos = ((tulos - 6500000L) / 2) + 6500000L;
    return tulos;
}

s32b hof_score(void)
{
    u32b tulos = (u32b)p_ptr->max_max_exp;
    u32b mult = (u32b)score_mult();
    s32b bigtulos = 0L;
    if (!tulos) return 0;
    s64b_mul(&bigtulos, &tulos, 0L, mult);
    s64b_div(&bigtulos, &tulos, 0L, 10000L);
    return (s32b)tulos;
}

char *version_modifier(void)
{
    return format("%s%s%s", coffee_break ? (coffee_break == SPEED_INSTA_COFFEE ? " (insta-coffee)" : " (coffee)") : "", thrall_mode ? " (thrall)" : "", wacky_rooms ? " (wacky)" : "");
}

static void _add_html_header(doc_ptr doc)
{
    string_ptr s = string_alloc_format("%s.html", player_base);
    string_ptr header = string_alloc();

    doc_change_name(doc, string_buffer(s));

    string_append_s(header, "<head>\n");
    string_append_s(header, " <meta name='filetype' value='character dump'>\n");
    string_printf(header,  " <meta name='variant' value='%s'>\n", VERSION_NAME);
    string_printf(header,  " <meta name='variant_version' value='%s%s'>\n", VERSION_STRING, version_modifier());
    string_printf(header,  " <meta name=\"character_name\" value=\"%s\">\n", player_name);
    string_printf(header,  " <meta name='race' value='%s'>\n", get_true_race()->name);
    string_printf(header,  " <meta name='class' value='%s'>\n", get_class()->name);
    string_printf(header,  " <meta name='level' value='%d'>\n", p_ptr->max_plv);
    string_printf(header,  " <meta name='experience' value='%d'>\n", oook_score());
    string_printf(header,  " <meta name='turncount' value='%d'>\n", turn_real(game_turn));
    string_printf(header,  " <meta name='max_depth' value='%d'>\n", _max_depth());
    string_printf(header,  " <meta name='score' value='%d'>\n", hof_score()); /* ?? Does oook need this? */
    string_printf(header,  " <meta name='fame' value='%d'>\n", p_ptr->fame);

    /* For angband.oook.cz ... I'm not sure what is best for proper display of html dumps so I'll need to ask pav
     * Note: A retired winning player is_dead, but has died_from set to 'Ripe Old Age'.
     * Approach #1: Give oook a string status field */
    string_printf(header,  " <meta name='status' value='%s'>\n",
        p_ptr->total_winner ? "通关者" : (p_ptr->is_dead ? "死亡" : "存活"));
    /* Approach #2: Give oook some boolean fields */
    string_printf(header,  " <meta name='winner' value='%d'>\n", p_ptr->total_winner ? 1 : 0);
    string_printf(header,  " <meta name='dead' value='%d'>\n", p_ptr->is_dead ? 1 : 0);
    string_printf(header,  " <meta name='retired' value='%d'>\n", _is_retired() ? 1 : 0);

    if (p_ptr->is_dead)
        string_printf(header,  " <meta name='killer' value='%s'>\n", p_ptr->died_from);
    string_append_s(header, "</head>");

    doc_change_html_header(doc, string_buffer(header));

    string_free(s);
    string_free(header);
}

void py_display_character_sheet(doc_ptr doc)
{
    _add_html_header(doc);

    doc_insert(doc, "<style:wide> [RoguelikeFansBand <$:version> 角色信息转储]\n");
    if (p_ptr->total_winner)
        doc_insert(doc, "<color:B>***通关者***</color>\n");
    else if (p_ptr->is_dead)
        doc_insert(doc, "<color:v>***失败者***</color>\n");
    else
        doc_newline(doc);
    doc_newline(doc);

    _build_general(doc);
    if (p_ptr->personality == PERS_SPLIT) split_dump(doc, 1);
    _build_equipment(doc);
    _build_melee(doc);
    _build_shooting(doc);
    _build_powers(doc);
    _build_spells(doc);

    {
        class_t *class_ptr = get_class();
        race_t  *race_ptr = get_race();
        if (class_ptr->character_dump)
            (class_ptr->character_dump)(doc);
        if (race_ptr && race_ptr->character_dump)
            race_ptr->character_dump(doc);
    }

    _build_dungeons(doc);
    _build_quests(doc);
    _build_uniques(doc);
    _build_virtues(doc);
    _build_race_history(doc);
    _build_mutations(doc);
    _build_pets(doc);
    _build_allies(doc);
    _build_inventory(doc);
    _build_quiver(doc);
    _build_home(doc);
    _build_museum(doc);
    _build_statistics(doc);
    _build_messages(doc);
    _build_options(doc);

    doc_insert(doc, "</style>");
}

void py_display(void)
{
    doc_ptr d = doc_alloc(80);

    py_display_character_sheet(d);

    screen_save();
    doc_display(d, "角色面板", 0);
    screen_load();

    doc_free(d);
}

/* This is used by the birth process ... Note that there
   is birth code that assumes the location of fields on the screen,
   and probably that should be cleaned up some day */
void py_display_birth(void)
{
    doc_ptr d = doc_alloc(80);
    _build_general(d);
    doc_sync_term(d, doc_range_all(d), doc_pos_create(0, 1));
    doc_free(d);
}
