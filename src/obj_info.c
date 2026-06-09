#include "angband.h"

#include <stdlib.h>
#include <assert.h>

/* Display detailed object information to the user */
extern void obj_display(object_type *o_ptr);
extern void obj_display_rect(object_type *o_ptr, rect_t display);
extern void obj_display_doc(object_type *o_ptr, doc_ptr doc);
extern void obj_display_smith(object_type *o_ptr, doc_ptr doc);
extern void device_display_doc(object_type *o_ptr, doc_ptr doc);
extern void device_display_smith(object_type *o_ptr, doc_ptr doc);
extern bool display_origin(object_type *o_ptr, doc_ptr doc);

static void _display_name(object_type *o_ptr, doc_ptr doc);
static void _display_desc(object_type *o_ptr, doc_ptr doc);
static void _display_stats(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_sustains(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_other_pval(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_brands(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_slays(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_resists(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_abilities(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_auras(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_extra(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_insurance(object_type *o_ptr, doc_ptr doc);
static void _display_curses(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_activation(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_activation_aux(effect_t *effect, bool full_info, doc_ptr doc, bool cb_hack);
static void _display_ignore(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _display_autopick(object_type *o_ptr, doc_ptr doc);
static void _display_score(object_type *o_ptr, doc_ptr doc);
static void _lite_display_doc(object_type *o_ptr, doc_ptr doc);
static void _custom_book_display_doc(object_type *o_ptr, doc_ptr doc);

/* Ego Object Knowledge is very similar to obj_display() ... I had to hack a bit
   to make it work, though :( */
extern void ego_display(ego_type *e_ptr);
extern void ego_display_rect(ego_type *e_ptr, rect_t display);
extern void ego_display_doc(ego_type *e_ptr, doc_ptr doc);

static void _ego_display_name(ego_type *e_ptr, doc_ptr doc);
static void _ego_display_stats(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _ego_display_other_pval(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);
static void _ego_display_extra(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc);

static int _calc_net_bonus(int amt, u32b flgs[OF_ARRAY_SIZE], int flg, int flg_dec);

static void _print_list(vec_ptr v, doc_ptr doc, char sep, char term);
static string_ptr _get_res_name(int res);

/* Low level helpers.
   Note: We have bonus flags, and penalty flags. For example
   TR_STR and TR_DEC_STR. With Rand-arts and the new Cursed implementation, it
   is possible for an object to have both. Also, pvals can be negative and use
   the bonus flag (rather than being positive and use the penalty flag).
   Note: vec_ptr is always vec<string_ptr> */
static int _calc_net_bonus(int amt, u32b flgs[OF_ARRAY_SIZE], int flg, int flg_dec)
{
    int net = 0;

    if (flg != OF_INVALID && have_flag(flgs, flg))
        net += amt;
    if (flg_dec != OF_INVALID && have_flag(flgs, flg_dec))
        net -= amt;

    return net;
}

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

static void _custom_book_display_doc(object_type *o_ptr, doc_ptr doc)
{
    int i;
    int cap = custom_book_capacity(o_ptr);
    int ct = custom_book_count(o_ptr);

    doc_printf(doc, "<color:U>容量:</color><tab:12><color:B>%d</color>/<color:G>%d</color>\n", ct, cap);
    if (!ct)
    {
        doc_insert(doc, "这本书还是空白的。按 <color:keypress>A</color> 可以将你已经学会的法术从普通法术书抄写进去。\n");
        return;
    }

    doc_insert(doc, "<color:U>法术:</color>\n");
    for (i = 0; i < cap; i++)
    {
        int realm = o_ptr->custom_book_realm[i];
        int spell = o_ptr->custom_book_spell[i];

        if (realm && spell < 32)
            doc_printf(doc, "  <color:y>%d.</color> <color:B>%s</color> <color:D>(%s)</color>\n",
                i + 1, do_spell(realm, spell, SPELL_NAME), realm_names[realm]);
        else
            doc_printf(doc, "  <color:y>%d.</color> <color:D>空白</color>\n", i + 1);
    }
    doc_insert(doc, "按 <color:keypress>A</color> 可以继续抄写；写满后可以覆盖已有法术。\n");
}

static string_ptr _get_res_name(int res)
{
    return string_alloc_format(
        "<color:%c>%s</color>",
        attr_to_attr_char(res_color(res)),
        res_name(res)
    );
}

/* Mid level helpers. Output one block of information at a time. */
static void _display_name(object_type *o_ptr, doc_ptr doc)
{
    char o_name[MAX_NLEN];
    char o_name2[MAX_NLEN];
    int leveys = MIN(72, doc->width);

    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_AND_ENCHANT | OD_NO_FLAVOR);
    object_desc(o_name2, o_ptr, OD_NAME_AND_ENCHANT | OD_NO_FLAVOR);
    if ((int)strlen(o_name2) > leveys - 10)
    {
        doc_printf(doc, "%s\n", o_name);
    }
    else doc_printf(doc, "%s%*c%2d.%d 磅\n", o_name, leveys - (strlen(o_name2) + 9), ' ', o_ptr->weight / 10, o_ptr->weight % 10);
}

static void _selita_paikka(char *paikka_text, byte paikka, byte taso, byte origin)
{
    bool tehtava = ((origin == ORIGIN_QUEST) || (origin == ORIGIN_QUEST_DROP) || (origin == ORIGIN_QUEST_REWARD));
    bool kaupunki = ((!tehtava) && (paikka < ORIGIN_DUNGEONS_AFTER));
    if (paikka > ORIGIN_QUESTS_AFTER)
    {
        paikka -= ORIGIN_QUESTS_AFTER;
        tehtava = TRUE;
    }

    if (kaupunki)
    {
        byte mika = ORIGIN_DUNGEONS_AFTER - paikka;
        strcpy(paikka_text, "in ");
        if (mika > TOWN_MAX)
        {
            strcat(paikka_text, "一个奇异的城镇里");
        }
        else
        {
            strcat(paikka_text, town_name(mika));
        }
        return;
    }
    else if (tehtava)
    {
        if (!paikka)
        {
            strcpy(paikka_text, "在一个离奇的任务中");
            return;
        }
        else
        {
            quest_ptr q = quests_get(paikka);
            cptr nimi;
            if ((!q) || (!q->id))
            {
                strcpy(paikka_text, "在一个离奇的任务中");
                return;
            }
            strcpy(paikka_text, (origin == ORIGIN_QUEST_REWARD) ? "任务“" : "在任务“");
            strcat(paikka_text, lyhytnimi(q, &nimi));
            free((vptr)nimi);
            strcat(paikka_text, "'");
            return;
        }
    }
    else if ((paikka == ORIGIN_DUNGEONS_AFTER) || (!taso))
    {
        strcpy(paikka_text, "在野外");
        return;
    }
    else /* originated in a regular dungeon */
    {
        int dung = paikka - ORIGIN_DUNGEONS_AFTER;
        int aitotaso = taso;
        string_ptr apu;
        char *dung_name;
        if (dung == DUNGEON_HEAVEN) aitotaso += 512;
        else if (dung == DUNGEON_HELL) aitotaso += 640;
        if (dung > DUNGEON_MAX) dung = DUNGEON_MAX; /* desperation */
        dung_name = (char *)dungeon_display_name(dung);
        apu = string_alloc_format("在 %s 的第 %d 层", dung_name, aitotaso);
        strcpy(paikka_text, string_buffer(apu));
        string_free(apu);
        return;
    }
}

bool display_origin(object_type *o_ptr, doc_ptr doc)
{
    byte origin = o_ptr->origin_type;
    byte paikka = o_ptr->origin_place / ORIGIN_MODULO;
    byte taso = o_ptr->origin_place % ORIGIN_MODULO;
    char paikka_text[MAX_NLEN];
    char pudottaja[MAX_NLEN];

    if ((origin == ORIGIN_NONE) || (origin == ORIGIN_MIXED)) return FALSE;
    if ((!show_discovery) && (o_ptr->mitze_type & MITZE_MIXED)) return FALSE;
    if (origin == ORIGIN_MYSTERY) o_ptr->origin_xtra = d_info[DUNGEON_MYSTERY].final_guardian;
    if ((origin != ORIGIN_CHEAT) && (origin != ORIGIN_PLAYER_MADE) && (origin != ORIGIN_GAMBLE) && (origin != ORIGIN_ENDLESS)
     && ((origin != ORIGIN_REFORGE) || (p_ptr->dragon_realm == DRAGON_REALM_CRAFT)) && (origin != ORIGIN_BIRTH) && (origin != ORIGIN_ARENA_REWARD) && (origin != ORIGIN_WANTED)
     && (origin != ORIGIN_CORNUCOPIA))
    {
        _selita_paikka(paikka_text, paikka, taso, origin);
    }
    if ((origin == ORIGIN_DROP) || (origin == ORIGIN_QUEST_DROP) || (origin == ORIGIN_STOLEN) || (origin == ORIGIN_ARENA_REWARD)
     || (origin == ORIGIN_WANTED) || (origin == ORIGIN_MYSTERY))
    {
        monster_race *r_ptr = &r_info[o_ptr->origin_xtra];
        if ((o_ptr->origin_xtra) && (o_ptr->origin_xtra < max_r_idx) && (r_ptr) && (r_ptr->name))
        {
            cptr mon_name = monster_race_display_name(o_ptr->origin_xtra);
            if (r_ptr->flags1 & RF1_UNIQUE)
            {
                strcpy(pudottaja, mon_name);
            }
            else
            {
                strcpy(pudottaja, is_a_vowel(mon_name[0]) ? "an " : "a ");
                strcat(pudottaja, mon_name);
            }
        }
        else strcpy(pudottaja, "一个奇异的怪物");
    }

    switch (origin)
    {
        case ORIGIN_FLOOR:
        case ORIGIN_QUEST:
        {
            doc_printf(doc, "%s时发现掉落在地上。", paikka_text);
            break;
        }
        case ORIGIN_DROP:
        case ORIGIN_QUEST_DROP:
        {
            doc_printf(doc, "%s时由 %s 掉落。", paikka_text, pudottaja);
            break;
        }
        case ORIGIN_CHEST:
        {
            doc_printf(doc, "%s时在箱子里找到的。", paikka_text);
            break;
        }
        case ORIGIN_SPECIAL:
        {
            doc_printf(doc, "%s时发现掉落在一个特殊房间的地上。", paikka_text);
            break;
        }
        case ORIGIN_VAULT:
        {
            doc_printf(doc, "%s时发现掉落在一个宝库的地上。", paikka_text);
            break;
        }
        case ORIGIN_RUBBLE:
        {
            doc_printf(doc, "%s时在碎石下找到的。", paikka_text);
            break;
        }
        case ORIGIN_ACQUIRE:
        {
            if ((o_ptr->tval == TV_FOOD) && (o_ptr->sval == SV_FOOD_RATION)) /* Produce Food/Create Food */
            {
                if (p_ptr->prace == RACE_HOBBIT) doc_printf(doc, "%s时由专业厨师变出来的。", paikka_text);
                else doc_printf(doc, "天赐之物。");
            }
            else
                doc_printf(doc, "%s时通过魔法变出来的。", paikka_text);
            break;
        }
        case ORIGIN_STORE:
        {
            doc_printf(doc, "%s时从商店买到的。", paikka_text);
            break;
        }
        case ORIGIN_BIRTH:
        {
            if ((p_ptr->pclass == CLASS_MONSTER) && (have_flag(o_ptr->flags, OF_NO_REMOVE))) return FALSE; /* Hack - Death-Swords, Rings, Filthy Rags */
            else if (!object_plural(o_ptr)) doc_printf(doc, "你已经记不清是什么时候拥有它的了。");
            else doc_printf(doc, "你已经记不清是什么时候拥有它们的了。");
            break;
        }
        case ORIGIN_DROP_UNKNOWN:
        {
            doc_printf(doc, "%s时由未知的生物掉落。", paikka_text);
            break;
        }
        case ORIGIN_CHEAT:
        {
            doc_printf(doc, "通过调试选项创建。");
            break;
        }
        case ORIGIN_QUEST_REWARD:
        {
            doc_printf(doc, "作为完成%s的奖励获得。", paikka_text);
            break;
        }
        case ORIGIN_ANGBAND_REWARD:
        {
            doc_printf(doc, "%s时由楼层守卫掉落。", paikka_text);
            break;
        }
        case ORIGIN_ARENA_REWARD:
        {
            if (!no_wilderness) doc_printf(doc, "在萨罗斯(Thalos)竞技场由%s掉落。", pudottaja);
            else doc_printf(doc, "在竞技场由%s掉落。", pudottaja);
            break;
        }
        case ORIGIN_NAGA:
        {
            doc_printf(doc, "%s时作为灵魂娜迦公司的礼物获得。", paikka_text);
            break;
        }
        case ORIGIN_PATTERN:
        {
            doc_printf(doc, "%s时从源质图印(Pattern)处获得。", paikka_text);
            break;
        }
        case ORIGIN_PLAYER_MADE:
        {
            doc_printf(doc, "你自己亲手制作的作品。", paikka_text);
            break;
        }
        case ORIGIN_ART_CREATION:
        {
            doc_printf(doc, "%s时被卷轴魔法般地强化了。", paikka_text);
            break;
        }
        case ORIGIN_STOLEN:
        {
            doc_printf(doc, "%s时从 %s 处偷来的。", paikka_text, pudottaja);
            break;
        }
        case ORIGIN_REFORGE:
        {
            if (p_ptr->dragon_realm == DRAGON_REALM_CRAFT)
            {
                doc_printf(doc, "%s时重铸的。", paikka_text);
                break;
            }
            if (!no_wilderness) doc_printf(doc, "在莫里万特重铸的。");
            else doc_printf(doc, "在战士公会重铸的。");
            break;
        }
        case ORIGIN_GAMBLE:
        {
            doc_printf(doc, "你在物品彩票中赢得了它。");
            break;
        }
        case ORIGIN_ENDLESS:
        {
            doc_printf(doc, "由无尽箭袋变出来的。");
            break;
        }
        case ORIGIN_PATRON:
        {
            if (o_ptr->origin_xtra >= MAX_PATRON) /* bizarre patron */
                 doc_printf(doc, "%s时作为你的混沌庇护者的礼物获得。", paikka_text);
            else if (disciple_is_(DISCIPLE_TROIKA)) doc_printf(doc, "%s时作为三驾马车(Troika)的礼物获得。", paikka_text);
            else doc_printf(doc, "%s时作为 %s 的礼物获得。", paikka_text, chaos_patrons[o_ptr->origin_xtra]);
            break;
        }
        case ORIGIN_PHOTO:
        {
            doc_printf(doc, "%s时取得的。", paikka_text);
            break;
        }
        case ORIGIN_KAWARIMI:
        {
            doc_printf(doc, "%s时被遗留下来的。", paikka_text);
            break;
        }
        case ORIGIN_WANTED:
        {
            doc_printf(doc, "作为上交%s尸体的奖励获得。", pudottaja);
            break;
        }
        case ORIGIN_CAN_OF_TOYS:
        {
            doc_printf(doc, "%s时在玩具罐里找到的。", paikka_text);
            break;
        }
        case ORIGIN_BLOOD:
        {
            if (object_is_(o_ptr, TV_POTION, SV_POTION_BLOOD))
                doc_printf(doc, "%s时用你自己的鲜血创造出来的。", paikka_text);
            else
                doc_printf(doc, "%s时由%s变质而来。", paikka_text, object_plural(o_ptr) ? "几瓶鲜血药水" : "一瓶鲜血药水");
            break;
        }
        case ORIGIN_CORNUCOPIA:
        {
            doc_printf(doc, "从阿南巴的丰饶之角获得的替换物品。");
            break;
        }
        case ORIGIN_CRAFTING:
        {
            doc_printf(doc, "%s时制作的。", paikka_text);
            break;
        }
        case ORIGIN_MUNDANITY:
        {
            doc_printf(doc, "%s时被剥夺了魔法属性（凡化）。", paikka_text);
            break;
        }
        case ORIGIN_MYSTERY:
        {
            doc_printf(doc, "%s时神秘地由 %s 掉落。", paikka_text, pudottaja);
            break;
        }
    }
    if ((show_discovery) && (o_ptr->mitze_type) && (o_ptr->mitze_turn) && (origin != ORIGIN_BIRTH))
    {
        int day = 0, hour = 0, min = 0;
        doc_printf(doc, "\n");
        if (origin == ORIGIN_STORE) doc_printf(doc, "购买时间:");
        else if (origin == ORIGIN_PHOTO) doc_printf(doc, "拍摄时间:");
        else if (o_ptr->mitze_type & MITZE_REFORGE) doc_printf(doc, "收到时间:");
        else if (o_ptr->mitze_type & MITZE_ID) doc_printf(doc, "鉴定时间:");
        else if (o_ptr->mitze_type & MITZE_PICKUP) doc_printf(doc, "拾取时间:");
        extract_day_hour_min_imp(o_ptr->mitze_turn, &day, &hour, &min);
        doc_printf(doc, "第%d天，%d:%02d，角色等级(CL) %d。", day, hour, min, o_ptr->mitze_level);
        if (o_ptr->mitze_type & MITZE_MIXED) doc_printf(doc, "\n(详情可能仅适用于一堆相似物品中的第一件。)");
    }

    return TRUE;
}


static void _display_desc(object_type *o_ptr, doc_ptr doc)
{
    cptr text;

    if (o_ptr->name1 && object_is_known(o_ptr))
        text = a_text + a_info[o_ptr->name1].text;
    else
        text = k_text + k_info[o_ptr->k_idx].text;

    if (strlen(text))
        doc_printf(doc, "%s\n\n", text);
}

/* For convenience, the stats section will include a few other bonuses, like stealh,
   searching, tunneling, and speed. These are things where the pval is readily
   interpreted by the user (unlike, say, device power). */
struct _flag_info_s
{
    int flg;
    int flg_dec;
    cptr name;
};
typedef struct _flag_info_s _flag_info_t, *_flag_info_ptr;
static _flag_info_t _stats_flags[] =
{
    { OF_STR,           OF_DEC_STR,             "力量" },
    { OF_INT,           OF_DEC_INT,             "智力" },
    { OF_WIS,           OF_DEC_WIS,             "感知" },
    { OF_DEX,           OF_DEC_DEX,             "敏捷" },
    { OF_CON,           OF_DEC_CON,             "体质" },
    { OF_CHR,           OF_DEC_CHR,             "魅力" },
    { OF_SPEED,         OF_DEC_SPEED,           "速度" },
    { OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY,   "Devices" },
    { OF_STEALTH,       OF_DEC_STEALTH,         "潜行" },
    { OF_SEARCH,        OF_INVALID,             "搜索" },
    { OF_INFRA,         OF_INVALID,             "红外视力" },
    { OF_TUNNEL,        OF_INVALID,             "挖掘" },
    { OF_XTRA_SHOTS,    OF_INVALID,             "射击速度" },
    { OF_INVALID,       OF_INVALID,             NULL }
};

static int _opposite_flag(int i)
{
    static s16b loydetty[OF_COUNT - 1] = {0};
    _flag_info_ptr table = _stats_flags;
    if (i < 1) return OF_INVALID;
    if (i >= OF_COUNT) return OF_INVALID;
    if (loydetty[i - 1]) return loydetty[i - 1];
    while (table->flg != OF_INVALID)
    {
        if (i == table->flg)
        {
            loydetty[i - 1] = table->flg_dec;
            return table->flg_dec;
        }
        if (i == table->flg_dec)
        {
            loydetty[i - 1] = table->flg;
            return table->flg;
        }
        table++;
    }
    if (i >= OF_RES_ACID)
    {
        int tulos = resist_opposite_flag(i);
        loydetty[i - 1] = tulos;
        return tulos;
    }
    if (i == OF_LIFE)
    {
        loydetty[i - 1] = OF_DEC_LIFE;
        return OF_DEC_LIFE;
    }
    if (i == OF_DEC_LIFE)
    {
        loydetty[i - 1] = OF_LIFE;
        return OF_LIFE;
    }
  
    loydetty[i - 1] = OF_INVALID;
    return OF_INVALID;
}

void remove_opposite_flags(u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    for (i = 1; i < OF_COUNT; i++)
    {
        int j;
        if (!have_flag(flgs, i)) continue;
        j = _opposite_flag(i);
        if ((j != OF_INVALID) && have_flag(flgs, j))
        {
            remove_flag(flgs, i);
            remove_flag(flgs, j);
        }
    }
}

static void _build_bonus_list(int pval, u32b flgs[OF_ARRAY_SIZE], _flag_info_ptr table, vec_ptr v)
{
    while (table->flg != OF_INVALID)
    {
        int net = _calc_net_bonus(pval, flgs, table->flg, table->flg_dec);
        if (net > 0)
            vec_add(v, string_copy_s(table->name));

        table++;
    }
}

static void _build_penalty_list(int pval, u32b flgs[OF_ARRAY_SIZE], _flag_info_ptr table, vec_ptr v)
{
    while (table->flg != OF_INVALID)
    {
        int net = _calc_net_bonus(pval, flgs, table->flg, table->flg_dec);
        if (net < 0)
            vec_add(v, string_copy_s(table->name));

        table++;
    }
}

static void _display_stats(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Pass 1: Positive Bonus */
    _build_bonus_list(o_ptr->pval, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_printf(doc, "<color:G>%+d</color> 的", abs(o_ptr->pval));
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Pass 2: Penalty Phase */
    vec_clear(v);
    _build_penalty_list(o_ptr->pval, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_printf(doc, "<color:r>%+d</color> 的", -abs(o_ptr->pval));
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}
static void _ego_display_stats(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Pass 1: Positive Bonus */
    _build_bonus_list(1, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:G>增加</color>");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Pass 2: Penalty Phase */
    vec_clear(v);
    _build_penalty_list(1, flgs, _stats_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:r>减少</color>");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static _flag_info_t _sustains_flags[] =
{
    { OF_SUST_STR, OF_INVALID, "力量" },
    { OF_SUST_INT, OF_INVALID, "智力" },
    { OF_SUST_WIS, OF_INVALID, "感知" },
    { OF_SUST_DEX, OF_INVALID, "敏捷" },
    { OF_SUST_CON, OF_INVALID, "体质" },
    { OF_SUST_CHR, OF_INVALID, "魅力" },
    { OF_INVALID,  OF_INVALID, NULL }
};

static void _display_sustains(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    _build_bonus_list(1, flgs, _sustains_flags, v);
    if (vec_length(v) > 0)
    {
        if (vec_length(v) == 6)
            doc_insert(doc, "<color:B>维持所有属性</color>");
        else
        {
            doc_insert(doc, "<color:B>维持</color>");
            _print_list(v, doc, ',', '\0');
        }
        doc_newline(doc);
    }
    vec_free(v);
}

static _flag_info_t _other_flags[] =
{
    { OF_BLOWS,             OF_INVALID, "攻击速度" },
//    { OF_XTRA_SHOTS,        OF_INVALID, "Shooting Speed" },
    { OF_DEVICE_POWER,      OF_INVALID, "装置能量" },
    { OF_MAGIC_RESISTANCE,  OF_INVALID, "魔法抗性" },
    { OF_SPELL_POWER,       OF_INVALID, "法术强度" },
    { OF_SPELL_CAP,         OF_INVALID, "法术容量" },
    { OF_LIFE,              OF_INVALID, "生命评级" },
    { OF_INVALID,           OF_INVALID, NULL }
};
static void _ego_display_other_pval(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);
    _build_bonus_list(1, flgs, _other_flags, v);
    if (vec_length(v) > 0)
    {
        doc_insert(doc, "<color:G>增加</color>");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }
    vec_free(v);
}
/* These pval flags require detailed explanations (e.g. +21% to Spell Power or -1.50 to Attack Speed) */
static void _display_other_pval(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int net = 0;

    if (!o_ptr->pval) return;

    if (have_flag(flgs, OF_BLOWS) || have_flag(flgs, OF_DEC_BLOWS))
    {
        int num = 0;

        if (have_flag(flgs, OF_BLOWS))
            num +=  o_ptr->pval * 50;
        if (have_flag(flgs, OF_DEC_BLOWS))
            num -=  o_ptr->pval * 100;

        doc_printf(doc, "<color:%c>%+d.%2.2d</color> 攻击速度\n",
                    (net > 0) ? 'G' : 'r', num / 100, num % 100);
    }
/*    if (have_flag(flgs, OF_XTRA_SHOTS))
    {
        int num = o_ptr->pval * 15;
        doc_printf(doc, "<color:%c>%+d.%2.2d</color> to Shooting Speed\n",
                    (net > 0) ? 'G' : 'r', num / 100, num % 100);
    }*/

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_DEC_MAGIC_MASTERY);
    if (net)
    {
        int pct = device_power_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> 装置能量\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    if (have_flag(flgs, OF_MAGIC_RESISTANCE))
    {
        int pct = o_ptr->pval * 5;
        doc_printf(doc, "<color:%c>%+d%%</color> 魔法抗性\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPELL_POWER, OF_DEC_SPELL_POWER);
    if (net)
    {
        int pct = spell_power_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> 法术强度\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPELL_CAP, OF_DEC_SPELL_CAP);
    if (net)
    {
        int pct = spell_cap_aux(100, net) - 100;
        doc_printf(doc, "<color:%c>%+d%%</color> 法术容量\n",
                    (net > 0) ? 'G' : 'r', pct);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_LIFE, OF_DEC_LIFE);
    if (net)
    {
        int pct = 3 * net;
        doc_printf(doc, "<color:%c>%+d%%</color> 生命倍率\n",
                    (net > 0) ? 'G' : 'r', pct);
    }
}

static void _display_brands(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_BRAND_ACID))
        vec_add(v, string_copy_s("<color:g>酸性烙印</color>"));
    if (have_flag(flgs, OF_BRAND_ELEC))
        vec_add(v, string_copy_s("<color:b>闪电烙印</color>"));
    if (have_flag(flgs, OF_BRAND_FIRE))
        vec_add(v, string_copy_s("<color:r>火焰之舌</color>"));
    if (have_flag(flgs, OF_BRAND_COLD))
        vec_add(v, string_copy_s("<color:W>冰霜烙印</color>"));
    if (have_flag(flgs, OF_BRAND_POIS))
        vec_add(v, string_copy_s("<color:G>毒蛇之牙</color>"));
    if (have_flag(flgs, OF_BRAND_CHAOS))
        vec_add(v, string_copy_s("<color:v>混沌印记</color>"));
    if (have_flag(flgs, OF_BRAND_VAMP))
        vec_add(v, string_copy_s("<color:D>吸血</color>"));
    if (have_flag(flgs, OF_BRAND_DARK))
        vec_add(v, string_copy_s("<color:D>暗影横扫</color>"));
    if (have_flag(flgs, OF_IMPACT))
        vec_add(v, string_copy_s("<color:U>引发地震</color>"));
    if (have_flag(flgs, OF_VORPAL2))
        vec_add(v, string_copy_s("<color:v>*锋锐*</color>"));
    else if (have_flag(flgs, OF_VORPAL))
        vec_add(v, string_copy_s("<color:R>锋锐</color>"));
    if (have_flag(flgs, OF_STUN))
        vec_add(v, string_copy_s("<color:o>震慑</color>"));
    if (have_flag(flgs, OF_BRAND_ORDER))
        vec_add(v, string_copy_s("<color:W>秩序武器</color>"));
    if (have_flag(flgs, OF_BRAND_WILD))
        vec_add(v, string_copy_s("<color:o>狂野武器</color>"));
    if (have_flag(flgs, OF_BRAND_MANA))
        vec_add(v, string_copy_s("<color:B>法力烙印</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_slays(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_KILL_EVIL))
        vec_add(v, string_copy_s("<color:y>*邪恶*</color>"));
    else if (have_flag(flgs, OF_SLAY_EVIL))
        vec_add(v, string_copy_s("<color:y>邪恶</color>"));

	if (have_flag(flgs, OF_KILL_GOOD))
		vec_add(v, string_copy_s("<color:y>*善良*</color>"));
	else if (have_flag(flgs, OF_SLAY_GOOD))
        vec_add(v, string_copy_s("<color:W>善良</color>"));

	if (have_flag(flgs, OF_KILL_LIVING))
		vec_add(v, string_copy_s("<color:y>*活物*</color>"));
	else if (have_flag(flgs, OF_SLAY_LIVING))
        vec_add(v, string_copy_s("<color:o>活物</color>"));

    if (have_flag(flgs, OF_KILL_DRAGON))
        vec_add(v, string_copy_s("<color:r>*龙类*</color>"));
    else if (have_flag(flgs, OF_SLAY_DRAGON))
        vec_add(v, string_copy_s("<color:r>龙类</color>"));

    if (have_flag(flgs, OF_KILL_DEMON))
        vec_add(v, string_copy_s("<color:R>*恶魔*</color>"));
    else if (have_flag(flgs, OF_SLAY_DEMON))
        vec_add(v, string_copy_s("<color:R>恶魔</color>"));

    if (have_flag(flgs, OF_KILL_UNDEAD))
        vec_add(v, string_copy_s("<color:D>*不死*</color>"));
    else if (have_flag(flgs, OF_SLAY_UNDEAD))
        vec_add(v, string_copy_s("<color:D>不死</color>"));

    if (have_flag(flgs, OF_KILL_ANIMAL))
        vec_add(v, string_copy_s("<color:g>*动物*</color>"));
    else if (have_flag(flgs, OF_SLAY_ANIMAL))
        vec_add(v, string_copy_s("<color:g>动物</color>"));

    if (have_flag(flgs, OF_KILL_HUMAN))
        vec_add(v, string_copy_s("<color:s>*人类*</color>"));
    else if (have_flag(flgs, OF_SLAY_HUMAN))
        vec_add(v, string_copy_s("<color:s>人类</color>"));

    if (have_flag(flgs, OF_KILL_ORC))
        vec_add(v, string_copy_s("<color:U>*兽人*</color>"));
    else if (have_flag(flgs, OF_SLAY_ORC))
        vec_add(v, string_copy_s("<color:U>兽人</color>"));

    if (have_flag(flgs, OF_KILL_TROLL))
        vec_add(v, string_copy_s("<color:g>*巨魔*</color>"));
    else if (have_flag(flgs, OF_SLAY_TROLL))
        vec_add(v, string_copy_s("<color:g>巨魔</color>"));

    if (have_flag(flgs, OF_KILL_GIANT))
        vec_add(v, string_copy_s("<color:u>*巨人*</color>"));
    else if (have_flag(flgs, OF_SLAY_GIANT))
        vec_add(v, string_copy_s("<color:u>巨人</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "斩杀:");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_resists(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int     i;
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    /* Immunities */
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg = res_get_object_immune_flag(i);
        if (flg != OF_INVALID && have_flag(flgs, flg))
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "免疫:");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Resistances */
    vec_clear(v);
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg_im = res_get_object_immune_flag(i);
        int net = 0;

        if (flg_im != OF_INVALID && have_flag(flgs, flg_im)) continue;
        net = _calc_net_bonus(1, flgs, res_get_object_flag(i), res_get_object_vuln_flag(i));
        if (net > 0)
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "抵抗:");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    /* Vulnerabilities */
    vec_clear(v);
    for (i = 0; i < RES_TELEPORT; i++)
    {
        int flg_im = res_get_object_immune_flag(i);
        int net = 0;

        if (flg_im != OF_INVALID && have_flag(flgs, flg_im)) continue;
        net = _calc_net_bonus(1, flgs, res_get_object_flag(i), res_get_object_vuln_flag(i));
        if (net < 0)
            vec_add(v, _get_res_name(i));
    }
    if (vec_length(v))
    {
        doc_insert(doc, "脆弱:");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_abilities(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_LORE2))
        vec_add(v, string_copy_s("<color:B>自动鉴定</color>"));

    if (have_flag(flgs, OF_FREE_ACT))
        vec_add(v, string_copy_s("<color:R>行动自如</color>"));
    if (have_flag(flgs, OF_SEE_INVIS))
        vec_add(v, string_copy_s("<color:B>识破隐形</color>"));
    if (have_flag(flgs, OF_REGEN))
        vec_add(v, string_copy_s("<color:g>生命再生</color>"));
    if (have_flag(flgs, OF_REGEN_MANA))
        vec_add(v, string_copy_s("<color:G>法力回复</color>"));
    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>维持生命</color>"));
    if (have_flag(flgs, OF_REFLECT))
        vec_add(v, string_copy_s("<color:o>反射</color>"));
    if (have_flag(flgs, OF_LEVITATION))
        vec_add(v, string_copy_s("<color:B>悬浮</color>"));
    if (have_flag(flgs, OF_SLOW_DIGEST))
        vec_add(v, string_copy_s("<color:g>缓慢消化</color>"));
    if (have_flag(flgs, OF_WARNING))
        vec_add(v, string_copy_s("<color:y>预警</color>"));
    if (have_flag(flgs, OF_NO_MAGIC))
        vec_add(v, string_copy_s("<color:r>反魔法</color>"));
    if (have_flag(flgs, OF_NO_SUMMON))
        vec_add(v, string_copy_s("<color:v>阻止召唤</color>"));
    if (have_flag(flgs, OF_NO_TELE))
        vec_add(v, string_copy_s("<color:r>阻止传送</color>"));
    if (have_flag(flgs, OF_THROWING))
        vec_add(v, string_copy_s("<color:D>投掷</color>"));
    if (have_flag(flgs, OF_BLESSED))
        vec_add(v, string_copy_s("<color:B>受祝福的</color>"));
    if (have_flag(flgs, OF_RIDING))
        vec_add(v, string_copy_s("<color:o>骑乘</color>"));
    if (have_flag(flgs, OF_DARKNESS))
        vec_add(v, string_copy_s("<color:D>永久黑暗</color>"));
    else if (have_flag(flgs, OF_LITE))
        vec_add(v, string_copy_s("<color:y>永久光明</color>"));
    if (have_flag(flgs, OF_NIGHT_VISION))
        vec_add(v, string_copy_s("<color:D>夜视</color>"));
    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_clear(v);
    if (have_flag(flgs, OF_TELEPATHY))
        vec_add(v, string_copy_s("<color:y>心灵感应</color>"));
    if (have_flag(flgs, OF_ESP_ANIMAL))
        vec_add(v, string_copy_s("<color:B>感知动物</color>"));
    if (have_flag(flgs, OF_ESP_UNDEAD))
        vec_add(v, string_copy_s("<color:D>感知不死生物</color>"));
    if (have_flag(flgs, OF_ESP_DEMON))
        vec_add(v, string_copy_s("<color:R>感知恶魔</color>"));
    if (have_flag(flgs, OF_ESP_ORC))
        vec_add(v, string_copy_s("<color:U>感知兽人</color>"));
    if (have_flag(flgs, OF_ESP_TROLL))
        vec_add(v, string_copy_s("<color:g>感知巨魔</color>"));
    if (have_flag(flgs, OF_ESP_GIANT))
        vec_add(v, string_copy_s("<color:u>感知巨人</color>"));
    if (have_flag(flgs, OF_ESP_DRAGON))
        vec_add(v, string_copy_s("<color:r>感知龙类</color>"));
    if (have_flag(flgs, OF_ESP_HUMAN))
        vec_add(v, string_copy_s("<color:s>感知人类</color>"));
    if (have_flag(flgs, OF_ESP_EVIL))
        vec_add(v, string_copy_s("<color:y>感知邪恶</color>"));
    if (have_flag(flgs, OF_ESP_GOOD))
        vec_add(v, string_copy_s("<color:w>感知善良</color>"));
    if (have_flag(flgs, OF_ESP_NONLIVING))
        vec_add(v, string_copy_s("<color:B>感知无生命生物</color>"));
	if (have_flag(flgs, OF_ESP_LIVING))
		vec_add(v, string_copy_s("<color:W>感知活物</color>"));
    if (have_flag(flgs, OF_ESP_UNIQUE))
        vec_add(v, string_copy_s("<color:v>感知唯一怪物</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_auras(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v = vec_alloc((vec_free_f)string_free);

    if (have_flag(flgs, OF_AURA_FIRE))
        vec_add(v, _get_res_name(RES_FIRE));
    if (have_flag(flgs, OF_AURA_COLD))
        vec_add(v, _get_res_name(RES_COLD));
    if (have_flag(flgs, OF_AURA_ELEC))
        vec_add(v, _get_res_name(RES_ELEC));
    if (have_flag(flgs, OF_AURA_SHARDS))
        vec_add(v, string_copy_s("<color:U>碎片</color>"));
    if (have_flag(flgs, OF_AURA_REVENGE))
        vec_add(v, string_copy_s("<color:v>反击</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "光环:");
        _print_list(v, doc, ',', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_extra(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    int net = 0;

    switch (o_ptr->name2)
    {
    case EGO_AMMO_RETURNING:
        doc_insert(doc, "它在被发射后经常会回到你的背包中。\n");
        break;
    case EGO_AMMO_ENDURANCE:
        doc_insert(doc, "它能承受几乎任何破坏而不会被摧毁。\n");
        break;
    case EGO_QUIVER_PHASE:
        doc_insert(doc, "这个箭袋及其里面的东西重量绝对为零。\n");
        break;
    case EGO_QUIVER_PROTECTION:
        doc_insert(doc, "这个箭袋能保护其里面的东西免遭意外破坏。\n");
        break;
    case EGO_QUIVER_HOLDING:
        doc_insert(doc, "这个箭袋拥有增加的携带容量。\n");
        break;
    }

    if (have_flag(flgs, OF_EASY_SPELL))
        doc_insert(doc, "它会影响你施放法术的能力。\n");

    if (have_flag(flgs, OF_DEC_MANA))
    {
        doc_insert(doc, "它能减少你的法力消耗。\n");
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_WEAPONMASTERY, OF_INVALID);
    if (net)
    {
        doc_printf(doc, "它%s你近战武器的伤害面骰。\n",
            (net > 0) ? "提升" : "<color:R>降低</color>");
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_XTRA_MIGHT, OF_INVALID);
    if (net)
    {
        doc_printf(doc, "它%s你弓箭的倍率。\n",
            (net > 0) ? "提升" : "<color:R>降低</color>");
    }

    switch (o_ptr->name1)
    {
    case ART_STONE_OF_NATURE:
        doc_insert(doc, "它极大地强化自然魔法。\n");
        break;
    case ART_STONE_OF_LIFE:
        doc_insert(doc, "它极大地强化生命魔法。\n");
        break;
    case ART_STONE_OF_SORCERY:
        doc_insert(doc, "它极大地强化咒术魔法。\n");
        break;
    case ART_STONE_OF_CHAOS:
        doc_insert(doc, "它极大地强化混沌魔法。\n");
        break;
    case ART_STONE_OF_DEATH:
        doc_insert(doc, "它极大地强化死亡魔法。\n");
        break;
    case ART_STONE_OF_TRUMP:
        doc_insert(doc, "它极大地强化王牌魔法。\n");
        break;
    case ART_STONE_OF_DAEMON:
        doc_insert(doc, "它极大地强化恶魔魔法。\n");
        break;
    case ART_STONE_OF_CRUSADE:
        doc_insert(doc, "它极大地强化圣战魔法。\n");
        break;
    case ART_STONE_OF_CRAFT:
        doc_insert(doc, "它极大地强化工匠魔法。\n");
        break;
    case ART_STONE_OF_ARMAGEDDON:
        doc_insert(doc, "它极大地强化毁灭魔法。\n");
        break;
    case ART_STONEMASK:
        doc_insert(doc, "它会让你永久变成吸血鬼。\n");
        break;
    }

    if (object_is_(o_ptr, TV_SWORD, SV_POISON_NEEDLE))
        doc_insert(doc, "它会尝试瞬间杀死一只怪物。\n");

    if (object_is_(o_ptr, TV_POLEARM, SV_DEATH_SCYTHE))
        doc_insert(doc, "它有时会让你击中自己。\n它总是能穿透无敌屏障。\n");

    if (have_flag(flgs, OF_IGNORE_INVULN))
        doc_insert(doc, "它能抵消无敌结界。\n");

    if (have_flag(flgs, OF_DUAL_WIELDING))
        doc_insert(doc, "它会影响你双持武器时的命中能力。\n");

    if (o_ptr->tval == TV_STATUE)
    {
        if (o_ptr->pval == MON_BULLGATES)
            doc_insert(doc, "它是可耻的。\n");
        else if (r_info[o_ptr->pval].flags2 & RF2_ELDRITCH_HORROR)
            doc_insert(doc, "它是可怕的。\n");
        else
            doc_insert(doc, "它是令人愉快的。\n");
    }

    if (o_ptr->tval == TV_FIGURINE)
        doc_insert(doc, "它在被投掷时会变成一只宠物。\n");

    if (o_ptr->name3)
    {
        char nimi[MAX_NLEN];
        my_strcpy(nimi, artifact_display_name(o_ptr->name3), sizeof(nimi));
        (void)clip_and_locate("~", nimi);
        doc_printf(doc, "它让你想起了神器 <color:R>%s</color>。\n", ((nimi[0] == '&') && (strlen(nimi) > 2)) ? nimi + 2 : nimi);
    }
}

static void _display_insurance(object_type *o_ptr, doc_ptr doc)
{
    if (cornucopia_item_policy(o_ptr) >= 0)
    {
        int vakuutettu = (o_ptr->insured % 100);
        if (o_ptr->number == vakuutettu) doc_printf(doc, "\n你已经为%s买了保险。\n", (vakuutettu == 1) ? "它" : "它们");
        else if (vakuutettu) doc_printf(doc, "\n这堆物品中有 %d 件%s购买了保险。\n", vakuutettu, (vakuutettu == 1) ? "已被": "已被");
    }
}

static void _ego_display_extra(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (have_flag(flgs, OF_EASY_SPELL))
        doc_insert(doc, "它会影响你施放法术的能力。\n");

    if (have_flag(flgs, OF_DEC_MANA))
    {
        caster_info *caster_ptr = get_caster_info();
        if (caster_ptr && (caster_ptr->options & CASTER_ALLOW_DEC_MANA))
            doc_insert(doc, "它能减少你的法力消耗。\n");
    }

    if (have_flag(flgs, OF_WEAPONMASTERY))
        doc_insert(doc, "它能增加你近战武器的伤害面骰。\n");

    if (have_flag(flgs, OF_XTRA_MIGHT))
        doc_insert(doc, "它能增加你弓箭的倍率。\n");

    if (have_flag(flgs, OF_DUAL_WIELDING))
        doc_insert(doc, "它会影响你双持武器时的命中能力。\n");
}

static void _display_curses(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    vec_ptr v;

    if (object_is_device(o_ptr)) return;
    if (!(o_ptr->ident & (IDENT_KNOWN | IDENT_SENSE | IDENT_STORE))) return;

    v = vec_alloc((vec_free_f)string_free);

    /* Note: Object may not actually be cursed, but still might have
       Aggravate or TY Curse. */
    if ((obj_is_identified(o_ptr)) || (o_ptr->loc.where == INV_EQUIP))
    {
        /* Basic Curse Status is always obvious (light, heavy, permanent) */
        if (o_ptr->curse_flags & OFC_PERMA_CURSE)
            doc_insert(doc, "它是 <color:v>永久诅咒的</color>。\n");
        else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
            doc_insert(doc, "它是 <color:r>重度诅咒的</color>。\n");
        else if (o_ptr->curse_flags & OFC_CURSED)
            doc_insert(doc, "它是 <color:D>被诅咒的</color>。\n");
        if ((o_ptr->loc.where == INV_EQUIP) && (o_ptr->curse_flags != o_ptr->known_curse_flags))
            doc_insert(doc, "它带有 <color:v>未知的诅咒</color>。\n");
    }
    else if (o_ptr->curse_flags & OFC_CURSED)
        doc_insert(doc, "它带有 <color:v>未知的诅咒</color>。\n");

    /* The precise nature of the curse, however, must be learned either by
       experience or by *identification* */
    if (have_flag(flgs, OF_TY_CURSE) || o_ptr->known_curse_flags & OFC_TY_CURSE)
        vec_add(v, string_copy_s("<color:v>*远古邪恶诅咒*</color>"));
    if (o_ptr->known_curse_flags & OFC_BY_CURSE)
        vec_add(v, string_copy_s("<color:P>微型邪恶诅咒</color>"));
    if (have_flag(flgs, OF_AGGRAVATE) || o_ptr->known_curse_flags & OFC_AGGRAVATE)
        vec_add(v, string_copy_s("<color:r>激怒怪物</color>"));
    if (have_flag(flgs, OF_DRAIN_EXP) || o_ptr->known_curse_flags & OFC_DRAIN_EXP)
        vec_add(v, string_copy_s("<color:y>吸取经验</color>"));
    if (o_ptr->known_curse_flags & OFC_SLOW_REGEN)
        vec_add(v, string_copy_s("<color:o>缓慢再生</color>"));
    if (o_ptr->known_curse_flags & OFC_DANGER)
        vec_add(v, string_copy_s("<color:R>招惹危险</color>"));
    if (o_ptr->known_curse_flags & OFC_ADD_L_CURSE)
        vec_add(v, string_copy_s("<color:w>附加弱效诅咒</color>"));
    if (o_ptr->known_curse_flags & OFC_ADD_H_CURSE)
        vec_add(v, string_copy_s("<color:b>附加重度诅咒</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_ANIMAL)
        vec_add(v, string_copy_s("<color:g>吸引动物</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_DEMON)
        vec_add(v, string_copy_s("<color:R>吸引恶魔</color>"));
    if (o_ptr->known_curse_flags & OFC_CALL_DRAGON)
        vec_add(v, string_copy_s("<color:r>吸引龙类</color>"));
    if (o_ptr->known_curse_flags & OFC_COWARDICE)
        vec_add(v, string_copy_s("<color:y>怯懦</color>"));
    if (o_ptr->known_curse_flags & OFC_CATLIKE)
        vec_add(v, string_copy_s("<color:r>猫之步伐</color>"));
    if (o_ptr->known_curse_flags & OFC_CRAPPY_MUT)
        vec_add(v, string_copy_s("<color:R>有害突变</color>"));
    if (have_flag(flgs, OF_TELEPORT) || o_ptr->known_curse_flags & OFC_TELEPORT)
        vec_add(v, string_copy_s("<color:B>随机传送</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_MELEE)
        vec_add(v, string_copy_s("<color:G>攻击失误</color>"));
    if (o_ptr->known_curse_flags & OFC_NORMALITY)
        vec_add(v, string_copy_s("<color:B>驱散魔法</color>"));
    if (o_ptr->name2 == EGO_ROBE_TWILIGHT)
        vec_add(v, string_copy_s("<color:v>零护甲</color>"));
    else if (o_ptr->known_curse_flags & OFC_LOW_AC)
        vec_add(v, string_copy_s("<color:R>低护甲</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_MAGIC)
        vec_add(v, string_copy_s("<color:y>额外法术失败率</color>"));
    if (o_ptr->known_curse_flags & OFC_LOW_DEVICE)
        vec_add(v, string_copy_s("<color:y>装置易失效</color>"));
    if (o_ptr->known_curse_flags & OFC_FAST_DIGEST)
        vec_add(v, string_copy_s("<color:r>快速消化</color>"));
    if (o_ptr->known_curse_flags & OFC_OPEN_WOUNDS)
        vec_add(v, string_copy_s("<color:r>伤口撕裂</color>"));
    if (o_ptr->known_curse_flags & OFC_ALLERGY)
        vec_add(v, string_copy_s("<color:y>刺激气道</color>"));
    if (o_ptr->known_curse_flags & OFC_DRAIN_HP)
        vec_add(v, string_copy_s("<color:o>吸取生命</color>"));
    if (o_ptr->known_curse_flags & OFC_DRAIN_MANA)
        vec_add(v, string_copy_s("<color:B>吸取法力</color>"));
    if (o_ptr->known_curse_flags & OFC_DRAIN_PACK)
        vec_add(v, string_copy_s("<color:g>吸取装置</color>"));
    if (object_is_unenchantable(o_ptr))
        vec_add(v, string_copy_s("<color:r>无法附魔</color>"));

    if (vec_length(v))
    {
        _print_list(v, doc, ';', '\0');
        doc_newline(doc);
    }

    vec_free(v);
}

static void _display_activation_aux(effect_t *effect, bool full_info, doc_ptr doc, bool cb_hack)
{
    cptr res = cb_hack ? "释放宠物" : do_effect(effect, SPELL_NAME, 0);

    doc_newline(doc);
    doc_printf(doc, "<color:U>激活:</color><tab:12><color:B>%s</color>\n", res);

    if (full_info)
    {
        int fail = effect_calc_fail_rate(effect);

        res = do_effect(effect, SPELL_INFO, 0);
        if (res && strlen(res))
            doc_printf(doc, "<color:U>信息:</color><tab:12>%s\n", res);
        doc_printf(doc, "<color:U>失败率:</color><tab:12>%d.%d%%\n", fail/10, fail%10);
        if (effect->cost)
            doc_printf(doc, "<color:U>冷却:</color><tab:12>%d\n", effect->cost);
    }

}

static void _display_activation(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (obj_has_effect(o_ptr))
    {
        if (have_flag(flgs, OF_ACTIVATE))
        {
            effect_t e = obj_get_effect(o_ptr);
            bool capture_ball_hack = ((o_ptr->tval == TV_CAPTURE) && (o_ptr->pval > 0));
            _display_activation_aux(&e, obj_is_identified_fully(o_ptr), doc, capture_ball_hack);
            doc_newline(doc);
        }
        else
        {
            doc_newline(doc);
            doc_insert(doc, "<color:U>激活:</color><tab:12><color:y>?</color>\n");
            doc_newline(doc);
        }
    }
}

static void _display_ignore(u32b flgs[OF_ARRAY_SIZE], doc_ptr doc)
{
    if (have_flag(flgs, OF_IGNORE_ACID) &&
        have_flag(flgs, OF_IGNORE_ELEC) &&
        have_flag(flgs, OF_IGNORE_FIRE) &&
        have_flag(flgs, OF_IGNORE_COLD))
    {
        doc_insert(doc, "<color:B>它不会受到元素伤害的破坏。</color>\n");
    }
    else
    {
        vec_ptr v = vec_alloc((vec_free_f)string_free);

        if (have_flag(flgs, OF_IGNORE_ACID))
            vec_add(v, _get_res_name(RES_ACID));
        if (have_flag(flgs, OF_IGNORE_ELEC))
            vec_add(v, _get_res_name(RES_ELEC));
        if (have_flag(flgs, OF_IGNORE_FIRE))
            vec_add(v, _get_res_name(RES_FIRE));
        if (have_flag(flgs, OF_IGNORE_COLD))
            vec_add(v, _get_res_name(RES_COLD));

        if (vec_length(v) > 0)
        {
            doc_insert(doc, "它不会被此种伤害破坏:");
            _print_list(v, doc, ',', '.');
            doc_newline(doc);
        }

        vec_free(v);
    }
}

static void _display_autopick(object_type *o_ptr, doc_ptr doc)
{
    if (destroy_debug)
    {
        int idx = is_autopick(o_ptr);
        if (idx >= 0)
        {
            string_ptr s = autopick_line_from_entry(&autopick_list[idx], AUTOPICK_COLOR_CODED);
            doc_printf(doc, "<color:r>自动拾取:</color> <indent><style:indent>%s</style></indent>\n", string_buffer(s));
            string_free(s);
        }
    }
}

#if 0
/* Debugging Object Pricing */
static doc_ptr _dbg_doc = NULL;
static void _cost_dbg_hook(cptr msg)
{
    doc_printf(_dbg_doc, "%s\n", msg);
}
#else
static char _score_color(int score)
{
    if (score < 1000)
        return 'D';
    if (score < 10000)
        return 'w';
    if (score < 20000)
        return 'W';
    if (score < 40000)
        return 'u';
    if (score < 60000)
        return 'y';
    if (score < 80000)
        return 'o';
    if (score < 100000)
        return 'R';
    if (score < 150000)
        return 'r';
    return 'v';
}
#endif

static void _display_score(object_type *o_ptr, doc_ptr doc)
{
#if 0
    _dbg_doc = doc;
    cost_calc_hook = _cost_dbg_hook;

    doc_newline(doc);
    new_object_cost(o_ptr, 0);

    cost_calc_hook = NULL;
    _dbg_doc = NULL;
#else
    int score = obj_value(o_ptr);
    char buf[10];
    big_num_display(score, buf);
    doc_printf(doc, "<color:B>评分:</color> <color:%c>%s</color>", _score_color(score), buf);
    if (o_ptr->level)
        doc_printf(doc, " (L%d)", o_ptr->level);
    if (o_ptr->discount)
        doc_printf(doc, "(%d%% 折扣)", o_ptr->discount);
    doc_newline(doc);

    if (p_ptr->prace == RACE_ANDROID && obj_is_identified(o_ptr))
    {
        score = android_obj_exp(o_ptr);
        big_num_display(score, buf);
        if (score)
            doc_printf(doc, "<color:B>常数:</color> <color:%c>%s</color>\n", _score_color(score), buf);
    }

#endif
}

static void _lite_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];
    if (o_ptr->tval != TV_LITE) return;
    obj_flags_known(o_ptr, flgs);
    if (have_flag(flgs, OF_DARKNESS))
    {
        doc_insert(doc, "它不提供光源。\n");

        if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "它使光源半径减少 2。\n");
        else if (o_ptr->sval == SV_LITE_TORCH)
            doc_insert(doc, "它使光源半径减少 1。\n");
        else
            doc_insert(doc, "它使光源半径减少 3。\n");
    }
    else if (have_flag(flgs, OF_NIGHT_VISION))
    {
        doc_insert(doc, "它让你能在黑暗中视物。\n");
    }
    else if (o_ptr->name1 || o_ptr->art_name)
    {
        doc_insert(doc, "它永久提供光源 (半径 3)。\n");
    }
    else if (o_ptr->name2 == EGO_LITE_EXTRA_LIGHT)
    {
        if (o_ptr->sval == SV_LITE_FEANOR)
            doc_insert(doc, "它永久提供光源 (半径 3)。\n");
        else if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "加注燃料后它提供光源 (半径 3)。\n");
        else if (o_ptr->sval == SV_LITE_TORCH)
            doc_insert(doc, "加注燃料后它提供光源 (半径 2)。\n");
    }
    else
    {
        if (o_ptr->sval == SV_LITE_FEANOR)
            doc_insert(doc, "它永久提供光源 (半径 2)。\n");
        else if (o_ptr->sval == SV_LITE_LANTERN)
            doc_insert(doc, "加注燃料后它提供光源 (半径 2)。\n");
        else if (o_ptr->sval == SV_LITE_TORCH)
            doc_insert(doc, "加注燃料后它提供光源 (半径 1)。\n");
    }
    if (o_ptr->name2 == EGO_LITE_DURATION)
        doc_insert(doc, "它提供照明的时间要长得多。\n");
}

/* Public Interface */
void obj_display(object_type *o_ptr)
{
    obj_display_rect(o_ptr, ui_menu_rect());
}

void obj_display_rect(object_type *o_ptr, rect_t display)
{
    doc_ptr doc = doc_alloc(MIN(display.cx, 72));

    if (display.cx > 80)
        display.cx = 80;

    obj_display_doc(o_ptr, doc);

    screen_save();
    if (doc_cursor(doc).y < display.cy - 3)
    {
        doc_insert(doc, "\n<color:B>[按<color:y>任意键</color>继续]</color>\n\n");
        doc_sync_term(doc, doc_range_all(doc), doc_pos_create(display.x, display.y));
        inkey();
    }
    else
    {
        doc_display_aux(doc, "物品信息", 0, display);
    }
    screen_load();

    doc_free(doc);
}

void obj_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];

    /* Devices need special handling. For one thing, they cannot be equipped, so
       that most flags are not used, and those that are generally mean something
       different (e.g. TR_HOLD_LIFE means no charge draining). */
    if (object_is_device(o_ptr) || o_ptr->tval == TV_POTION || o_ptr->tval == TV_SCROLL)
    {
        device_display_doc(o_ptr, doc);
        return;
    }

    obj_flags_display(o_ptr, flgs);

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");
    if (show_origins || show_discovery)
    {
        if (display_origin(o_ptr, doc)) doc_printf(doc, "\n\n");

    }
    _display_desc(o_ptr, doc);
    if (obj_is_custom_book(o_ptr))
        _custom_book_display_doc(o_ptr, doc);
    doc_insert(doc, "<style:indent>"); /* Indent a bit when word wrapping long lines */

    if (o_ptr->tval == TV_LITE)
        _lite_display_doc(o_ptr, doc);

    _display_stats(o_ptr, flgs, doc);
    _display_sustains(flgs, doc);
    _display_other_pval(o_ptr, flgs, doc);
    _display_slays(flgs, doc);
    _display_brands(flgs, doc);
    _display_resists(flgs, doc);
    _display_abilities(flgs, doc);
    _display_auras(flgs, doc);
    _display_extra(o_ptr, flgs, doc);
    _display_insurance(o_ptr, doc);
    _display_activation(o_ptr, flgs, doc);
    _display_curses(o_ptr, flgs, doc);
    _display_ignore(flgs, doc);
    _display_score(o_ptr, doc);

    // printf object level
    // doc_printf(doc, "<color:U>Level:</color> <color:B>%d</color>\n", o_ptr->level);

    if (object_is_wearable(o_ptr))
    {
        doc_newline(doc);
        if (obj_is_identified(o_ptr))
        {
            if (!obj_is_identified_fully(o_ptr))
            {
                if (object_is_artifact(o_ptr))
                    doc_printf(doc, "此物品是神器，一件独特的物品，你必须通过直接体验、或通过“高级鉴定(*identifying*)”、或出售此物品来了解其力量。\n");
                else
                    doc_printf(doc, "此物品可能具有额外的力量，你可以通过体验、或通过“高级鉴定(*identifying*)”、或出售此物品来了解它们。\n");
            } else {
                if (object_is_artifact(o_ptr)) {
                    if (object_is_true_artifact(o_ptr))
                        doc_printf(doc, "此物品是一件真正的神器。\n");
                    else
                        doc_printf(doc, "此物品是一件随机生成的神器。\n");
                }
            }
        }
        else
        {
            doc_printf(doc, "此物品处于未知状态。你应该鉴定它，或者，如果你足够大胆（愚蠢？），你也可以通过装备它来了解更多！\n");
        }
    }
    _display_autopick(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

void obj_display_smith(object_type *o_ptr, doc_ptr doc)
{
    u32b flgs[OF_ARRAY_SIZE];

    if (object_is_device(o_ptr))
    {
        device_display_smith(o_ptr, doc);
        return;
    }

    obj_flags_display(o_ptr, flgs);

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent><style:indent>");

    if (!object_is_known(o_ptr) && (o_ptr->ident & IDENT_SENSE))
    {
        switch (o_ptr->feeling)
        {
        case FEEL_TERRIBLE:
            doc_insert(doc, "这件物品似乎是某种真正 <color:v>可怕的(terrible)</color> 东西。\n");
            break;
        case FEEL_SPECIAL:
            doc_insert(doc, "这件物品似乎是某种非常 <color:B>特殊的(special)</color> 东西。\n");
            break;
        case FEEL_AWFUL:
            doc_insert(doc, "这件物品似乎是某种 <color:r>糟糕的(awful)</color> 东西。\n");
            break;
        case FEEL_EXCELLENT:
            doc_insert(doc, "这件物品似乎是一件 <color:y>优秀的(excellent)</color> 杰作。\n");
            break;
        case FEEL_BAD:
            doc_insert(doc, "这件物品似乎是一件 <color:R>劣质的(shoddy)</color> 破烂。\n");
            break;
        case FEEL_GOOD:
            doc_insert(doc, "这件物品展现出 <color:G>良好的(good)</color> 工艺。\n");
            break;
        case FEEL_AVERAGE:
            doc_insert(doc, "这件物品看起来很普通。\n");
            break;
        }
    }


    _display_stats(o_ptr, flgs, doc);
    _display_sustains(flgs, doc);
    _display_other_pval(o_ptr, flgs, doc);
    _display_slays(flgs, doc);
    _display_brands(flgs, doc);
    _display_resists(flgs, doc);
    _display_abilities(flgs, doc);
    _display_auras(flgs, doc);
    _display_extra(o_ptr, flgs, doc);
    _display_curses(o_ptr, flgs, doc);
    _display_ignore(flgs, doc);
    _display_score(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

void device_display_doc(object_type *o_ptr, doc_ptr doc)
{
    u32b    flgs[OF_ARRAY_SIZE];
    int     net = 0;
    int     boost = 0;
    vec_ptr v = NULL;

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");
    if (show_origins || show_discovery)
    {
        if (display_origin(o_ptr, doc)) doc_printf(doc, "\n\n");

    }
    _display_desc(o_ptr, doc);

    if (o_ptr->tval == TV_SCROLL || o_ptr->tval == TV_POTION)
    {
        if (!object_is_known(o_ptr)) doc_printf(doc, "你对这件物品没有特殊的了解。\n\n");
        else doc_printf(doc, "%s\n\n", do_device(o_ptr, SPELL_DESC, 0));
        if (obj_is_identified_fully(o_ptr))
        {
            cptr info = do_device(o_ptr, SPELL_INFO, 0);
            if (info && strlen(info))
                doc_printf(doc, "<color:U>信息: </color>%s\n", info);
            if (o_ptr->tval == TV_SCROLL)
            {
                int fail = device_calc_fail_rate(o_ptr);
                doc_printf(doc, "<color:U>失败率: </color>%d.%d%%\n", fail/10, fail%10);
            }
        }
        _display_insurance(o_ptr, doc);
        _display_autopick(o_ptr, doc);
        doc_insert(doc, "</indent>\n");
        return;
    }

    if (!object_is_device(o_ptr) || !object_is_known(o_ptr))
    {
        _display_insurance(o_ptr, doc);
        _display_autopick(o_ptr, doc);
        doc_insert(doc, "</indent>\n");
        return;
    }

    obj_flags_known(o_ptr, flgs);
    if (devicemaster_is_speciality(o_ptr))
        boost = device_power_aux(100, p_ptr->device_power + p_ptr->lev/10) - 100;
    else
        boost = device_power(100) - 100;

    doc_insert(doc, "<color:U>该装置具有以下魔法强度:</color>\n");
    if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_POWER))
        doc_printf(doc, "能量 : <color:G>%d</color>\n", device_level(o_ptr));
    else
        doc_insert(doc, "能量 : <color:G>?</color>\n");
    if (obj_is_identified_fully(o_ptr))
    {
        int sp = device_sp(o_ptr);
        int max_sp = device_max_sp(o_ptr);

        doc_printf(doc, "法力 : <color:%c>%d</color>/<color:G>%d</color>\n",
                    (sp < max_sp) ? 'y' : 'G', sp, max_sp);
    }
    else
    {
        doc_insert(doc, "法力 : <color:G>?</color>/<color:G>?</color>\n");
    }

    v = vec_alloc((vec_free_f)string_free);
    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPEED, OF_DEC_SPEED);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d 迅捷</color>", (net > 0) ? 'G' : 'r', net));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_INVALID);
    if (net)
    {
        int        pct = device_power_aux(100, net) - 100;
        string_ptr s = string_alloc_format("<color:%c>%+d%% 能量</color>", (net > 0) ? 'G' : 'r', pct);
        vec_add(v, s);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_EASY_SPELL, OF_INVALID);
    if (net > 0)
        vec_add(v, string_alloc_format("<color:G>%+d 易于使用</color>", net));
    else if (net < 0)
        vec_add(v, string_copy_s("<color:r>难以使用</color>"));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_REGEN, OF_INVALID);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d 再生</color>", (net > 0) ? 'G' : 'r', net));

    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>维持充能</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "额外 : <indent>");
        _print_list(v, doc, ';', '\0');
        doc_insert(doc, "</indent>\n\n");
    }
    else
        doc_newline(doc);

    vec_free(v);

    if (o_ptr->activation.type != EFFECT_NONE)
    {
        doc_insert(doc, "<color:U>该装置装载了一个法术:</color>\n");
        doc_printf(doc, "法术 : <color:B>%s</color>\n", do_device(o_ptr, SPELL_NAME, boost));
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_POWER))
        {
            cptr desc;
            desc = do_device(o_ptr, SPELL_INFO, boost);
            if (desc && strlen(desc))
                doc_printf(doc, "信息 : <color:w>%s</color>\n", desc);
        }
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_FAIL))
            doc_printf(doc, "等级 : <color:G>%d</color>\n", o_ptr->activation.difficulty);

        if (obj_is_identified_fully(o_ptr))
        {
            int  charges = device_sp(o_ptr) / o_ptr->activation.cost;
            int  max_charges = device_max_sp(o_ptr) / o_ptr->activation.cost;
            doc_printf(doc, "消耗 : <color:G>%d</color>\n", o_ptr->activation.cost);
            doc_printf(doc, "充能 : <color:%c>%d</color>/<color:G>%d</color>\n",
                        (charges < max_charges) ? 'y' : 'G', charges, max_charges);
            if (p_ptr->pclass == CLASS_MAGIC_EATER)
            {
                int       per_mill = magic_eater_regen_amt(o_ptr->tval);
                const int scale = 100;
                int       cost = o_ptr->activation.cost * scale;
                int       sp, turns;

                if (have_flag(flgs, OF_REGEN))
                    per_mill += o_ptr->pval * per_mill / 5;

                sp = device_max_sp(o_ptr) * per_mill * scale / 1000;
                turns = cost * scale / sp;

                doc_printf(doc, "恢复 : 每发充能需 <color:G>%d.%02d</color> 回合\n", turns / scale, turns % scale);
            }
        }
        if (obj_is_identified_fully(o_ptr) || (o_ptr->known_xtra & OFL_DEVICE_FAIL))
        {
            int  fail = device_calc_fail_rate(o_ptr);
            doc_printf(doc, "失败率 : <color:G>%d.%d%%</color>\n", fail/10, fail%10);
        }
        doc_printf(doc, "描述 : <indent>%s</indent>\n\n", do_device(o_ptr, SPELL_DESC, boost));
    }

    _display_insurance(o_ptr, doc);
        
    doc_insert(doc, "<style:indent>"); /* Indent a bit when word wrapping long lines */
    _display_score(o_ptr, doc);

    if (!obj_is_identified_fully(o_ptr))
        doc_printf(doc, "\n你可以进行“高级鉴定(*identify*)”或出售此物品以了解关于该装置的更多信息。此外，对于许多类型的装置，你也可以通过实际使用它足够长的时间来了解更多。\n");

    _display_ignore(flgs, doc);
    _display_autopick(o_ptr, doc);

    doc_insert(doc, "</style></indent>\n");
}

/* For wiz_smithing ... TODO: Consolidate common code with device_display_doc */
void device_display_smith(object_type *o_ptr, doc_ptr doc)
{
    u32b    flgs[OF_ARRAY_SIZE];
    int     net = 0;
    int     boost = 0;
    vec_ptr v = NULL;

    assert(object_is_device(o_ptr));
    assert(obj_is_identified_fully(o_ptr));

    _display_name(o_ptr, doc);
    doc_insert(doc, "  <indent>");

    obj_flags_known(o_ptr, flgs);
    if (devicemaster_is_speciality(o_ptr))
        boost = device_power_aux(100, p_ptr->device_power + p_ptr->lev/10) - 100;
    else
        boost = device_power(100) - 100;

    {
        int sp = device_sp(o_ptr);
        int max_sp = device_max_sp(o_ptr);

        doc_insert(doc, "<color:U>该装置具有以下魔法强度:</color>\n");
        doc_printf(doc, "能量 : <color:G>%d</color>\n", device_level(o_ptr));
        doc_printf(doc, "法力 : <color:%c>%d</color>/<color:G>%d</color>\n",
                    (sp < max_sp) ? 'y' : 'G', sp, max_sp);
    }

    v = vec_alloc((vec_free_f)string_free);
    net = _calc_net_bonus(o_ptr->pval, flgs, OF_SPEED, OF_DEC_SPEED);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d 迅捷</color>", (net > 0) ? 'G' : 'r', net));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_DEVICE_POWER, OF_INVALID);
    if (net)
    {
        int        pct = device_power_aux(100, net) - 100;
        string_ptr s = string_alloc_format("<color:%c>%+d%% 能量</color>", (net > 0) ? 'G' : 'r', pct);
        vec_add(v, s);
    }

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_EASY_SPELL, OF_INVALID);
    if (net > 0)
        vec_add(v, string_alloc_format("<color:G>%+d 易于使用</color>", net));
    else if (net < 0)
        vec_add(v, string_copy_s("<color:r>难以使用</color>"));

    net = _calc_net_bonus(o_ptr->pval, flgs, OF_REGEN, OF_INVALID);
    if (net)
        vec_add(v, string_alloc_format("<color:%c>%+d 再生</color>", (net > 0) ? 'G' : 'r', net));

    if (have_flag(flgs, OF_HOLD_LIFE))
        vec_add(v, string_copy_s("<color:y>维持充能</color>"));

    if (vec_length(v))
    {
        doc_insert(doc, "额外 : <indent>");
        _print_list(v, doc, ';', '\0');
        doc_insert(doc, "</indent>\n\n");
    }
    else
        doc_newline(doc);

    vec_free(v);

    if (o_ptr->activation.type != EFFECT_NONE)
    {
        int  fail = device_calc_fail_rate(o_ptr);
        cptr desc;

        doc_insert(doc, "<color:U>该装置装载了一个法术:</color>\n");
        doc_printf(doc, "法术 : <color:B>%s</color>\n", do_device(o_ptr, SPELL_NAME, boost));
        desc = do_device(o_ptr, SPELL_INFO, boost);
        if (desc && strlen(desc))
            doc_printf(doc, "信息 : <color:w>%s</color>\n", desc);
        doc_printf(doc, "等级 : <color:G>%d</color>\n", o_ptr->activation.difficulty);
        doc_printf(doc, "消耗 : <color:G>%d</color>\n", o_ptr->activation.cost);
        doc_printf(doc, "失败率 : <color:G>%d.%d%%</color>\n", fail/10, fail%10);
    }

    _display_score(o_ptr, doc);
    _display_ignore(flgs, doc);
    doc_insert(doc, "</indent>\n");
}

void ego_display(ego_type *e_ptr)
{
    ego_display_rect(e_ptr, ui_menu_rect());
}

void ego_display_rect(ego_type *e_ptr, rect_t display)
{
    doc_ptr doc = doc_alloc(MIN(display.cx, 72));

    if (display.cx > 80)
        display.cx = 80;

    ego_display_doc(e_ptr, doc);

    screen_save();
    if (doc_cursor(doc).y < display.cy - 3)
    {
        doc_insert(doc, "\n[按任意键继续]\n\n");
        doc_sync_term(doc, doc_range_all(doc), doc_pos_create(display.x, display.y));
        inkey();
    }
    else
    {
        doc_display_aux(doc, "Ego信息", 0, display);
    }
    screen_load();

    doc_free(doc);
}

static bool _have_flag(u32b flgs[OF_ARRAY_SIZE])
{
    int  i;
    for (i = 0; i < OF_ARRAY_SIZE; i++)
    {
        if (flgs[i])
            return TRUE;
    }
    return FALSE;
}

static void _ego_display_name(ego_type *e_ptr, doc_ptr doc)
{
    char name[255];

    /* Major hackage. Many egos can span multiple types, and
       I'd rather not always list them all. For example, 'Armor of Protection'
       is enough, rather than 'Body Armor/Shield/Cloak/Helmet/Gloves/Boots of Protection' */
    if (e_ptr->type & EGO_TYPE_WEAPON)
        doc_insert(doc, "武器");
    else if (e_ptr->type & EGO_TYPE_DIGGER)
        doc_insert(doc, "挖掘工具");
    else if (e_ptr->type & EGO_TYPE_BODY_ARMOR)
        doc_insert(doc, "护甲");
    else if (e_ptr->type & EGO_TYPE_DRAGON_ARMOR)
        doc_insert(doc, "龙铠");
    else if (e_ptr->type == (EGO_TYPE_CLOAK | EGO_TYPE_BOOTS))
        doc_insert(doc, "披风/靴子");
    else if (e_ptr->type == (EGO_TYPE_GLOVES | EGO_TYPE_BOOTS))
        doc_insert(doc, "手套/靴子");
    else if (e_ptr->type == (EGO_TYPE_HELMET | EGO_TYPE_CROWN))
        doc_insert(doc, "头盔/王冠");
    else if (e_ptr->type & EGO_TYPE_SHIELD)
        doc_insert(doc, "盾牌");
    else if (e_ptr->type & EGO_TYPE_ROBE)
        doc_insert(doc, "法袍");
    else if (e_ptr->type & EGO_TYPE_CLOAK)
        doc_insert(doc, "披风");
    else if (e_ptr->type & EGO_TYPE_HELMET)
        doc_insert(doc, "头盔");
    else if (e_ptr->type & EGO_TYPE_CROWN)
        doc_insert(doc, "王冠");
    else if (e_ptr->type & EGO_TYPE_GLOVES)
        doc_insert(doc, "手套");
    else if (e_ptr->type & EGO_TYPE_BOOTS)
        doc_insert(doc, "靴子");
    else if (e_ptr->type & EGO_TYPE_BOW)
        doc_insert(doc, "弓");
    else if (e_ptr->type & EGO_TYPE_AMMO)
        doc_insert(doc, "弹药");
    else if (e_ptr->type & EGO_TYPE_HARP)
        doc_insert(doc, "竖琴");
    else if (e_ptr->type == (EGO_TYPE_RING | EGO_TYPE_AMULET))
        doc_insert(doc, "戒指/护身符");
    else if (e_ptr->type & EGO_TYPE_RING)
        doc_insert(doc, "戒指");
    else if (e_ptr->type & EGO_TYPE_AMULET)
        doc_insert(doc, "护身符");
    else if (e_ptr->type & EGO_TYPE_LITE)
        doc_insert(doc, "光源");
    else if (e_ptr->type & EGO_TYPE_DEVICE)
        doc_insert(doc, "魔法装置");

    strip_name_aux(name, e_name + e_ptr->name);
    doc_printf(doc, "%s\n\n", name);
}

void ego_display_doc(ego_type *e_ptr, doc_ptr doc)
{
    int  i;
    u32b flgs[OF_ARRAY_SIZE];

    _ego_display_name(e_ptr, doc);

    /* First, the fixed flags always present */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = e_ptr->known_flags[i] & e_ptr->flags[i];
    remove_flag(flgs, OF_HIDE_TYPE);
    remove_flag(flgs, OF_SHOW_MODS);
    remove_flag(flgs, OF_FULL_NAME);

    if (_have_flag(flgs))
    {
        doc_insert(doc, "<color:B>固定加成</color>\n");
        doc_insert(doc, "  <indent><style:indent>");
        _ego_display_stats(flgs, doc);
        _display_sustains(flgs, doc);
        _ego_display_other_pval(flgs, doc);
        _display_slays(flgs, doc);
        _display_brands(flgs, doc);
        _display_resists(flgs, doc);
        _display_abilities(flgs, doc);
        _display_auras(flgs, doc);
        _ego_display_extra(flgs, doc);
        if (have_flag(flgs, OF_ACTIVATE))
            _display_activation_aux(&e_ptr->activation, FALSE, doc, FALSE); /* no ego capture balls */
        _display_ignore(flgs, doc);
        doc_insert(doc, "</style></indent>\n");
    }

    /* Next, the optional flags */
    for (i = 0; i < OF_ARRAY_SIZE; i++)
        flgs[i] = e_ptr->known_flags[i] & e_ptr->xtra_flags[i];
    remove_flag(flgs, OF_HIDE_TYPE);
    remove_flag(flgs, OF_SHOW_MODS);
    remove_flag(flgs, OF_FULL_NAME);

    if (_have_flag(flgs))
    {
        doc_insert(doc, "<color:B>可选加成</color>\n");
        doc_insert(doc, "  <indent><style:indent>");
        _ego_display_stats(flgs, doc);
        _display_sustains(flgs, doc);
        _ego_display_other_pval(flgs, doc);
        _display_slays(flgs, doc);
        _display_brands(flgs, doc);
        _display_resists(flgs, doc);
        _display_abilities(flgs, doc);
        _display_auras(flgs, doc);
        _ego_display_extra(flgs, doc);
        if (have_flag(flgs, OF_ACTIVATE))
            doc_insert(doc, "它提供一次额外的激活。\n");
        _display_ignore(flgs, doc);
        doc_insert(doc, "</style></indent>\n");
    }
}
