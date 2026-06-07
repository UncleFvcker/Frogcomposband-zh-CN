#include "angband.h"
#include <assert.h>

#define _NONE  -1
#define _OK     0
#define _CANCEL 1

static doc_ptr _doc = NULL;

static int _inkey(void)
{
    return inkey_special(TRUE);
}

/***********************************************************************
 * Object Creation
 **********************************************************************/
void wiz_obj_create(void)
{
}

/***********************************************************************
 * Object Modification (Smithing)
 **********************************************************************/

/* Low level helpers for bit twiddling */
typedef struct {
    int flag;
    cptr name;
    object_p pred;
} _flag_info_t, *_flag_info_ptr;

static void _obj_identify_fully(object_type *o_ptr)
{
    int old_hack_status = no_karrot_hack;
    no_karrot_hack = TRUE;
    obj_identify_fully(o_ptr);
    no_karrot_hack = old_hack_status;
}

static void _toggle(object_type *o_ptr, int flag)
{
    if (have_flag(o_ptr->flags, flag)) remove_flag(o_ptr->flags, flag);
    else add_flag(o_ptr->flags, flag);
    if (is_pval_flag(flag) && have_flag(o_ptr->flags, flag) && o_ptr->pval == 0)
        o_ptr->pval = 1;
    _obj_identify_fully(o_ptr);
}

/* Level 2 Smithing functions */
static int _smith_plusses(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        if (object_is_melee_weapon(o_ptr) || object_is_ammo(o_ptr))
        {
            doc_insert(_doc, "<color:y>x</color>/<color:y>X</color>) 调整伤害骰数\n");
            doc_insert(_doc, "<color:y>y</color>/<color:y>Y</color>) 调整伤害面数\n");
        }
        else if (o_ptr->tval == TV_BOW)
            doc_insert(_doc, "<color:y>x</color>/<color:y>X</color>) 调整倍率\n");
        else
            doc_insert(_doc, "<color:y>x</color>/<color:y>X</color>) 调整基础护甲(AC)\n");
        if (!object_is_ammo(o_ptr))
            doc_insert(_doc, "<color:y>a</color>/<color:y>A</color>) 调整护甲(AC)加成\n");
        doc_insert(_doc, "<color:y>h</color>/<color:y>H</color>) 调整近战命中\n");
        doc_insert(_doc, "<color:y>d</color>/<color:y>D</color>) 调整近战伤害\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'x':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.dd > 0) copy.dd--;
                else copy.dd = 99;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult > 0) copy.mult -= 5;
                else copy.mult = 700;
            }
            else
            {
                if (copy.ac > 0) copy.ac--;
                else copy.ac = 50;
            }
            break;
        case 'X':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.dd < 99) copy.dd++;
                else copy.dd = 0;
            }
            else if (copy.tval == TV_BOW)
            {
                if (copy.mult < 696) copy.mult += 5;
                else copy.mult = 0;
            }
            else
            {
                if (copy.ac < 50) copy.ac++;
                else copy.ac = 0;
            }
            break;
        case 'y':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.ds > 0) copy.ds--;
                else copy.ds = 99;
            }
            break;
        case 'Y':
            if (object_is_melee_weapon(&copy) || object_is_ammo(o_ptr))
            {
                if (copy.ds < 99) copy.ds++;
                else copy.ds = 0;
            }
            break;
        case 'h':
            if (copy.to_h > -50) copy.to_h--;
            else copy.to_h = 50;
            break;
        case 'H':
            if (copy.to_h < 50) copy.to_h++;
            else copy.to_h = -50;
            break;
        case 'd':
            if (copy.to_d > -50) copy.to_d--;
            else copy.to_d = 50;
            break;
        case 'D':
            if (copy.to_d < 50) copy.to_d++;
            else copy.to_d = -50;
            break;
        case 'a':
            if (!object_is_ammo(o_ptr))
            {
                if (copy.to_a > -50) copy.to_a--;
                else copy.to_a = 50;
            }
            break;
        case 'A':
            if (!object_is_ammo(o_ptr))
            {
                if (copy.to_a < 50) copy.to_a++;
                else copy.to_a = -50;
            }
            break;
        }
    }
}

static int _smith_stats(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int cmd, i;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < MAX_STATS; i++)
        {
            doc_printf(_doc, "   <color:y>%c</color>) %s\n", I2A(i), stat_name_true[i]);
        }
        doc_insert(_doc, "<color:y>p</color>/<color:y>P</color>) 调整属性附加值(pval)\n");
        doc_newline(_doc);
        doc_insert(_doc, "使用 SHIFT+选项 切换减益标志\n");
        doc_insert(_doc, "使用 CTRL+选项 切换维持标志\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        
        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'p':
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
            break;
        case 'P':
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
            break;
        }
       
        /* Toggle inc stat? */
        i = A2I(cmd);
        if (0 <= i && i < MAX_STATS)
        {
            _toggle(&copy, OF_STR + i);
            continue;
        }

        /* Toggle dec stat? */
        if (isupper(cmd))
        {
            i = A2I(tolower(cmd));
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_DEC_STR + i);
                continue;
            }
        }

        /* Toggle sustain stat? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            i = A2I(c);
            if (0 <= i && i < MAX_STATS)
            {
                _toggle(&copy, OF_SUST_STR + i);
                continue;
            }
        }
    }
}

static bool _blows_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && o_ptr->tval != TV_BOW;
}

static bool _shots_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr);
}

static bool _weaponmastery_p(object_type *o_ptr)
{
    return object_is_wearable(o_ptr)
        && !object_is_melee_weapon(o_ptr)
        && o_ptr->tval != TV_BOW;
}

typedef struct { /* Bonuses need to support DEC_* flags */
    int flag;
    int dec_flag;
    cptr name;
    object_p pred;
} _flagx_info_t, *_flagx_info_ptr;

static _flagx_info_t _bonus_flags[] = {
    { OF_BLOWS, OF_DEC_BLOWS, "攻击速度", _blows_p },
    { OF_MAGIC_MASTERY, OF_DEC_MAGIC_MASTERY, "装置技能" },
    { OF_DEVICE_POWER, OF_INVALID, "装置强度" },
    { OF_TUNNEL, OF_INVALID, "挖掘" },
    { OF_XTRA_MIGHT, OF_INVALID, "额外威力", _shots_p },
    { OF_XTRA_SHOTS, OF_INVALID, "额外射击", _shots_p },
    { OF_INFRA, OF_INVALID, "红外视力" },
    { OF_LIFE, OF_DEC_LIFE, "生命评级" },
    { OF_MAGIC_RESISTANCE, OF_INVALID, "魔法抗性" },
    { OF_SEARCH, OF_INVALID, "搜索" },
    { OF_SPEED, OF_DEC_SPEED, "速度" },
    { OF_SPELL_POWER, OF_DEC_SPELL_POWER, "法术强度" },
    { OF_SPELL_CAP, OF_DEC_SPELL_CAP, "法术容量" },
    { OF_STEALTH, OF_DEC_STEALTH, "潜行" },
    { OF_WEAPONMASTERY, OF_INVALID, "武器专精", _weaponmastery_p },
    { OF_INVALID }
};

static int _smith_bonuses(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    vec_ptr     v = vec_alloc(NULL);
    int         result = _NONE, i;

    for (i = 0; ; i++)
    {
        _flagx_info_ptr fi = &_bonus_flags[i];
        if (fi->flag == OF_INVALID) break;
        if (fi->pred && !fi->pred(o_ptr)) continue;
        vec_add(v, fi);
    }

    while (result == _NONE)
    {
        int     cmd, split = vec_length(v); /* default to no split ... cols[1] remains empty */
        doc_ptr cols[2];

        cols[0] = doc_alloc(23);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1) / 2;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _flagx_info_ptr fi = vec_get(v, i);
            doc_printf(
                cols[i < split ? 0 : 1],
                "   <color:y>%c</color>) %s%c\n",
                I2A(i),
                fi->name,
                fi->dec_flag != OF_INVALID ? '*' : ' '
            );
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, "<color:y>p</color>/<color:y>P</color>) 调整属性附加值(pval)\n");
        doc_insert(_doc, "(*)使用 SHIFT+选项 切换减益标志\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
        {
            *o_ptr = copy;
            result =  _OK;
        }
        else if (cmd == ESCAPE)
        {
            result = _CANCEL;
        }
        else if (cmd == 'p')
        {
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
        }
        else if (cmd == 'P')
        {
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
        }
        else if (isupper(cmd))
        {
            i = cmd - 'A';
            if (0 <= i && i < vec_length(v))
            {
                _flagx_info_ptr fi = vec_get(v, i);
                if (fi->dec_flag != OF_INVALID)
                    _toggle(&copy, fi->dec_flag);
            }
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                _flagx_info_ptr fi = vec_get(v, i);
                _toggle(&copy, fi->flag);
            }
        }
    }
    vec_free(v);
    return result;
}

static int _smith_flags(object_type* o_ptr, _flag_info_ptr flags)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    vec_ptr     v = vec_alloc(NULL);
    int         result = _NONE, i;

    for (i = 0; ; i++)
    {
        _flag_info_ptr fi = &flags[i];
        if (fi->flag == OF_INVALID) break;
        if (fi->pred && !fi->pred(o_ptr)) continue;
        vec_add(v, fi);
    }

    while (result == _NONE && vec_length(v) > 0)
    {
        int     cmd, split = vec_length(v); /* default to no split ... cols[1] remains empty */
        doc_ptr cols[2];

        cols[0] = doc_alloc(23);
        cols[1] = doc_alloc(30);

        if (split > 10)
            split = (split + 1) / 2;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _flag_info_ptr fi = vec_get(v, i);
            doc_printf(cols[i < split ? 0 : 1], "   <color:y>%c</color>) %s\n", I2A(i), fi->name);
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
        {
            *o_ptr = copy;
            result =  _OK;
        }
        else if (cmd == ESCAPE)
        {
            result = _CANCEL;
        }
        else
        {
            i = A2I(cmd);
            if (0 <= i && i < vec_length(v))
            {
                _flag_info_ptr fi = vec_get(v, i);
                _toggle(&copy, fi->flag);
            }
        }
    }
    vec_free(v);
    return result;
}

static _flag_info_t _ability_flags[] = {
    { OF_FREE_ACT, "行动自由" },
    { OF_SEE_INVIS, "识破隐形" },
    { OF_HOLD_LIFE, "维持生命" },
    { OF_SLOW_DIGEST, "消化缓慢" },
    { OF_REGEN, "生命再生" },
    { OF_DUAL_WIELDING, "双持武器", object_is_gloves },
    { OF_NO_MAGIC, "反魔法" },
    { OF_WARNING, "警戒" },
    { OF_LEVITATION, "浮空" },
    { OF_REFLECT, "反弹" },
    { OF_AURA_FIRE, "火焰光环" },
    { OF_AURA_ELEC, "闪电光环" },
    { OF_AURA_COLD, "冰霜光环" },
    { OF_AURA_SHARDS, "碎片光环" },
    { OF_AURA_REVENGE, "复仇光环" },
    { OF_LITE, "额外发光" },
    { OF_INVALID }
};

static int _smith_abilities(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _ability_flags);
}

static _flag_info_t _telepathy_flags[] = {
    { OF_TELEPATHY,     "全局心感" },
    { OF_ESP_ANIMAL,    "感应动物" },
    { OF_ESP_UNDEAD,    "感应死灵" },
    { OF_ESP_DEMON,     "感应恶魔" },
    { OF_ESP_ORC,       "感应兽人" },
    { OF_ESP_TROLL,     "感应巨魔" },
    { OF_ESP_GIANT,     "感应巨人" },
    { OF_ESP_DRAGON,    "感应龙类" },
    { OF_ESP_HUMAN,     "感应人类" },
    { OF_ESP_EVIL,      "感应邪恶" },
    { OF_ESP_GOOD,      "感应善良" },
    { OF_ESP_NONLIVING, "感应非活物" },
	{ OF_ESP_LIVING,    "感应活物" },
    { OF_ESP_UNIQUE,    "感应Unique" },
    { OF_INVALID }
};

static int _smith_telepathies(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _telepathy_flags);
}

static _flag_info_t _slay_flags[] = {
    { OF_SLAY_EVIL,   "屠杀邪恶" },
    { OF_SLAY_GOOD,   "屠杀善良" },
    { OF_SLAY_LIVING, "屠杀活物" },
    { OF_SLAY_UNDEAD, "屠杀死灵" },
    { OF_SLAY_DEMON,  "屠杀恶魔" },
    { OF_SLAY_DRAGON, "屠杀龙类" },
    { OF_SLAY_HUMAN,  "屠杀人类" },
    { OF_SLAY_ANIMAL, "屠杀动物" },
    { OF_SLAY_ORC,    "屠杀兽人" },
    { OF_SLAY_TROLL,  "屠杀巨魔" },
    { OF_SLAY_GIANT,  "屠杀巨人" },
    { OF_KILL_EVIL,   "克星：邪恶" },
	{ OF_KILL_GOOD,   "克星：善良" },
	{ OF_KILL_LIVING, "克星：活物" },
    { OF_KILL_UNDEAD, "克星：死灵" },
    { OF_KILL_DEMON,  "克星：恶魔" },
    { OF_KILL_DRAGON, "克星：龙类" },
    { OF_KILL_HUMAN,  "克星：人类" },
    { OF_KILL_ANIMAL, "克星：动物" },
    { OF_KILL_ORC,    "克星：兽人" },
    { OF_KILL_TROLL,  "克星：巨魔" },
    { OF_KILL_GIANT,  "克星：巨人" },
    { OF_INVALID }
};

static int _smith_slays(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _slay_flags);
}

static _flag_info_t _brand_flags[] = {
    { OF_BRAND_ACID,    "强酸烙印" },
    { OF_BRAND_ELEC,    "闪电烙印" },
    { OF_BRAND_FIRE,    "火焰烙印" },
    { OF_BRAND_COLD,    "冰霜烙印" },
    { OF_BRAND_POIS,    "毒素烙印" },
    { OF_BRAND_MANA,    "法力烙印", object_is_melee_weapon },
    { OF_BRAND_CHAOS,   "混乱", object_is_melee_weapon },
    { OF_BRAND_VAMP,    "吸血" },
    { OF_IMPACT,        "冲击", object_is_melee_weapon },
    { OF_STUN,          "震慑", object_is_melee_weapon },
    { OF_VORPAL,        "斩首", object_is_melee_weapon },
    { OF_VORPAL2,       "*Vorpal*", object_is_melee_weapon },
    { OF_INVALID }
};

static int _smith_brands(object_type *o_ptr)
{
    return _smith_flags(o_ptr, _brand_flags);
}

static void _reroll_aux(object_type *o_ptr, int flags, int min)
{
    int attempts = 1 * 1000; /* param?  better gcc -O2 if more than 1k */
    int i, score, best_score = -1; /* scores are never negative */
    object_type forge, best = {0};
    bool igor_hack = ((prace_is_(RACE_IGOR)) && (o_ptr->tval == TV_CORPSE));

    for (i = 0; i < attempts; i++)
    {
        object_prep(&forge, igor_hack ? lookup_kind(TV_CORPSE, SV_CORPSE) : o_ptr->k_idx);
        if (igor_hack)
        {
            forge.pval = o_ptr->pval;
            if ((o_ptr->sval != SV_CORPSE) && (o_ptr->sval != SV_BODY_EARS)) forge.pval = o_ptr->xtra4;
            igor_dissect_corpse(&forge);
        }
        else apply_magic(&forge, dun_level, AM_NO_FIXED_ART | flags);
        _obj_identify_fully(&forge);

        score = obj_value_real(&forge);
        if (score > min)
        {
            forge.number = o_ptr->number; /* ammo */
            *o_ptr = forge;
            return;
        }
        else if (object_is_melee_weapon(o_ptr))
        {
            if (forge.dd * forge.ds > best_score)
            {
                best_score = forge.dd * forge.ds;
                best = forge;
            }
        }
        else if (score > best_score)
        {
            best_score = score;
            best = forge;
        }
    }
    if (!igor_hack) assert(best.k_idx == o_ptr->k_idx);
    best.number = o_ptr->number; /* ammo */
    object_origins(&best, ORIGIN_CHEAT);
    *o_ptr = best;
}

static char _score_color(int score)  /* XXX duplicated in wizard2.c */
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

void wiz_create_objects(obj_create_f creator, u32b mode)
{
    doc_ptr doc = doc_alloc(120);
    inv_ptr inv = inv_alloc("临时", INV_PACK, 0);
    int     i;

    statistics_hack = TRUE;
    for (i = 1; i < 1000;)
    {
        object_type forge;
        if (creator(&forge, mode))
        {
            _obj_identify_fully(&forge);
            inv_add(inv, &forge);
            i++;
        }
    }
    inv_optimize(inv);
    doc_insert(doc, "<style:table>");
    for (i = 1; i <= inv_max(inv); i++)
    {
        obj_ptr obj = inv_obj(inv, i);
        int     score;
        char    name[MAX_NLEN];

        if (!obj) continue;
        score = obj_value_real(obj);
        object_desc(name, obj, OD_COLOR_CODED);

        doc_printf(doc, "%3d <color:%c>%5d</color>) <indent><style:indent>%s</style></indent>\n",
            i, _score_color(score), score, name);
    }
    doc_insert(doc, "</style>");
    statistics_hack = FALSE;

    doc_display(doc, "物品", 0);
    inv_free(inv);
    doc_free(doc);
}

static obj_ptr _reroll_obj = NULL;
static bool _reroll_creator(obj_ptr obj, u32b mode)
{
    bool igor_hack = ((prace_is_(RACE_IGOR)) && (_reroll_obj->tval == TV_CORPSE));
    object_prep(obj, igor_hack ? lookup_kind(TV_CORPSE, SV_CORPSE) : _reroll_obj->k_idx);
    object_origins(obj, ORIGIN_CHEAT);
    if (igor_hack)
    {
        if ((_reroll_obj->sval == SV_CORPSE) || (_reroll_obj->sval == SV_BODY_EARS)) obj->pval = _reroll_obj->pval;
        else obj->pval = _reroll_obj->xtra4;
    }
    if ((prace_is_(RACE_IGOR)) && (obj->tval == TV_CORPSE)) return igor_dissect_corpse(obj);
    return apply_magic(obj, dun_level, mode);
}
static void _reroll_stats_aux(object_type *o_ptr, int flags)
{
    _reroll_obj = o_ptr;
    wiz_create_objects(_reroll_creator, AM_NO_FIXED_ART | flags);
    _reroll_obj = NULL;
}

static int _smith_reroll(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t      r = ui_map_rect();
    static int  min = 0;

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "<color:y>w</color>) 糟糕(Awful)\n");
        doc_insert(_doc, "<color:y>b</color>) 劣质(Bad)\n");
        doc_insert(_doc, "<color:y>a</color>) 普通(Average)*\n");
        doc_insert(_doc, "<color:y>g</color>) 良好(Good)*\n");
        doc_insert(_doc, "<color:y>e</color>) 优秀(Excellent)*\n");
        doc_insert(_doc, "<color:y>r</color>) 随机神器(Random Artifact)*\n");
        if (o_ptr->name1 || o_ptr->name3)
            doc_insert(_doc, "<color:y>X</color>) 替代神器(Replacement Artifact)\n");

        doc_newline(_doc);
        doc_printf(_doc, "<color:y>m</color>) 最小分数 = %d\n", min);

        doc_insert(_doc, "\n (*)SHIFT+选项 以查看统计信息\n");
        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'm':
        {
            char buf[51];
            sprintf(buf, "%d", min);
            if (get_string("最小分数：", buf, 50))
            {
                min = atoi(buf);
                if (min < 0) min = 0;
                else if (min > 500000) min = 500000;
            }
            break;
        }
        case 'w': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_CURSED, min); break;
        case 'b': _reroll_aux(&copy, AM_GOOD | AM_CURSED, min); break;
        case 'a': _reroll_aux(&copy, AM_AVERAGE, min); break;
        case 'A': _reroll_stats_aux(&copy, AM_AVERAGE); break; /* only makes sense for devices */
        case 'g': _reroll_aux(&copy, AM_GOOD, min); break;
        case 'G': _reroll_stats_aux(&copy, AM_GOOD); break; /* only makes sense for devices */
        case 'e': _reroll_aux(&copy, AM_GOOD | AM_GREAT, min); break;
        case 'E': _reroll_stats_aux(&copy, AM_GOOD | AM_GREAT); break;
        case 'r': _reroll_aux(&copy, AM_GOOD | AM_GREAT | AM_SPECIAL, min); break;
        case 'R': _reroll_stats_aux(&copy, AM_GOOD | AM_GREAT | AM_SPECIAL); break;
        case 'X': {
            int which = o_ptr->name1;
            if (!which) which = o_ptr->name3;
            create_replacement_art(which, &copy, ORIGIN_CHEAT);
            _obj_identify_fully(&copy);
            break;}
        }
    }
}

static int _smith_resistances(object_type *o_ptr)
{
    object_type copy = *o_ptr;
    rect_t r = ui_map_rect();

    for (;;)
    {
        int  cmd, which;
        doc_ptr cols[2];

        cols[0] = doc_alloc(20);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        for (which = RES_BEGIN; which < RES_END; which++)
        {
            doc_printf(cols[which < RES_NEXUS ? 0 : 1], "   <color:y>%c</color>) %s%c\n",
                I2A(which - RES_BEGIN), res_name(which),
                res_get_object_immune_flag(which) != OF_INVALID ? '*' : ' ');
        }

        doc_insert_cols(_doc, cols, 2, 0);
        doc_free(cols[0]);
        doc_free(cols[1]);

        doc_insert(_doc, "SHIFT+选项 切换易伤标志\n");
        doc_insert(_doc, "(*)CTRL+选项 切换免疫标志\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();

        /* Note: iscntrl('\r') is true ... so we need to check this first*/
        if (cmd == '\r')
        {
            *o_ptr = copy;
            return _OK;
        }
        else if (cmd == ESCAPE)
            return _CANCEL;

        /* Toggle resistance? */
        which = A2I(cmd) + RES_BEGIN;
        if (RES_BEGIN <= which && which < RES_END)
        {
            _toggle(&copy, res_get_object_flag(which));
            continue;
        }

        /* Toggle vulnerability? */
        if (isupper(cmd))
        {
            which = A2I(tolower(cmd)) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flag = res_get_object_vuln_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
                    continue;
                }
            }
        }

        /* Toggle immunity? */
        if (iscntrl(cmd))
        {
            char c = 'a' + cmd - KTRL('A');
            which = A2I(c) + RES_BEGIN;
            if (RES_BEGIN <= which && which < RES_END)
            {
                int  flag = res_get_object_immune_flag(which);
                if (flag != OF_INVALID)
                {
                    _toggle(&copy, flag);
                    continue;
                }
            }
        }
    }
}

/* Devices */
static device_effect_info_ptr _device_find_effect(device_effect_info_ptr tbl, int effect)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &tbl[i];

        if (!entry->type) break;
        if (entry->type == effect) return entry;
    }

    return NULL;
}

static device_effect_info_ptr _device_effect_tbl(object_type *o_ptr)
{
    assert(object_is_device(o_ptr));
    switch (o_ptr->tval)
    {
    case TV_WAND: return wand_effect_table;
    case TV_STAFF: return staff_effect_table;
    case TV_ROD: return rod_effect_table;
    }
    assert(FALSE);
    return NULL;
}

static device_effect_info_ptr _choose_effect(object_type *o_ptr)
{
    rect_t                 r = ui_map_rect();
    device_effect_info_ptr tbl = _device_effect_tbl(o_ptr);
    device_effect_info_ptr effect = _device_find_effect(tbl, o_ptr->activation.type);

    for (;;)
    {
        int cmd, i, ct = 0, split;
        doc_ptr cols[2];

        cols[0] = doc_alloc(30);
        cols[1] = doc_alloc(30);

        doc_clear(_doc);
        /*obj_display_smith(o_ptr, _doc);*/
        doc_insert(_doc, "<color:G>选择一个效果：</color>\n");

        for (i = 0; ; i++)
        {
            if (!tbl[i].type) break;
            ct++;
        }
        split = ct;
        if (split > 10)
            split = (ct + 1) / 2;

        for (i = 0; ; i++)
        {
            device_effect_info_ptr e = &tbl[i];
            effect_t               dummy = {0};
            char                   choice = '?';
            char                   color = 'w';

            if (!e->type) break;
            dummy.type = e->type;
            if (i < 26) choice = 'a' + i;
            else if (i < 52) choice = 'A' + (i - 26);
            if (e->type == o_ptr->activation.type) color = 'B';
            doc_printf(cols[i < split ? 0 : 1], "   <color:y>%c</color>) <color:%c>%s</color>\n",
                choice, color, do_effect(&dummy, SPELL_NAME, 0));
        }
        doc_insert_cols(_doc, cols, 2, 1);
        doc_free(cols[0]);
        doc_free(cols[1]);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == ESCAPE)
            break;
        else
        {
            if (isupper(cmd)) i = cmd - 'A' + 26;
            else i = cmd - 'a';
            if (0 <= i && i < ct)
            {
                effect = &tbl[i];
                break;
            }
        }
    }
    return effect;
}

static int _smith_device_effect(object_type *o_ptr)
{
    rect_t                 r = ui_map_rect();
    object_type            copy = *o_ptr;
    device_effect_info_ptr tbl = _device_effect_tbl(o_ptr);
    device_effect_info_ptr effect = _device_find_effect(tbl, copy.activation.type);

    for (;;)
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "<color:y>e</color>) 更改效果\n");

        doc_insert(_doc, "<color:y>p</color>/<color:y>P</color>) 调整强度(power)\n");
        doc_insert(_doc, "<color:y>m</color>/<color:y>M</color>) 调整法力\n");
        doc_insert(_doc, "<color:y>l</color>/<color:y>L</color>) 调整效果等级\n");
        doc_insert(_doc, "<color:y>c</color>/<color:y>C</color>) 调整效果消耗\n");
        doc_insert(_doc, "<color:y>r</color>) 充能\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'l':
            if (copy.activation.difficulty > 1) copy.activation.difficulty--;
            else copy.activation.difficulty = 100;
            break;
        case 'L':
            if (copy.activation.difficulty < 100) copy.activation.difficulty++;
            else copy.activation.difficulty = 1;
            break;
        case 'c':
            if (copy.activation.cost > 1) copy.activation.cost--;
            else copy.activation.cost = 150;
            break;
        case 'C':
            if (copy.activation.cost < 150) copy.activation.cost++;
            else copy.activation.cost = 1;
            break;
        /* Note: We break encapsulation here ... */
        case 'p':
            if (copy.xtra3 > effect->level) copy.xtra3--;
            else copy.xtra3 = 100;
            copy.activation.power = copy.xtra3;
            break;
        case 'P':
            if (copy.xtra3 < 100) copy.xtra3++;
            else copy.xtra3 = effect->level;
            copy.activation.power = copy.xtra3;
            break;
        case 'm':
            if (copy.xtra4 > copy.activation.cost + 4) copy.xtra4 -= 5;
            else copy.xtra4 = 500;
            if (copy.xtra5/100 > copy.xtra4)
                copy.xtra5 = copy.xtra4*100;
            break;
        case 'M':
            if (copy.xtra4 < 496) copy.xtra4 += 5;
            else copy.xtra4 = copy.activation.cost;
            if (copy.xtra5/100 > copy.xtra4)
                copy.xtra5 = copy.xtra4*100;
            break;
        case 'e':
        {
            device_effect_info_ptr new_effect = _choose_effect(&copy);
            if (new_effect->type != effect->type)
            {
                effect = new_effect;
                copy.activation.type = effect->type;
                if (effect->level > copy.xtra3)
                {
                    copy.xtra3 = effect->level;
                    copy.activation.power = effect->level;
                }
                copy.activation.difficulty = effect->level;
                copy.activation.cost = effect->cost;
                if (effect->cost > copy.xtra4)
                {
                    copy.xtra4 = effect->cost;
                    copy.xtra5 = effect->cost * 100;
                }
            }
            break;
        }
        case 'r':
            device_regen_sp(&copy, 1000);
            break;
        }
    }
}

static int _smith_device_bonus(object_type *o_ptr)
{
    rect_t      r = ui_map_rect();
    object_type copy = *o_ptr;

    for (;;)
    {
        int cmd;

        doc_clear(_doc);
        obj_display_smith(&copy, _doc);

        doc_insert(_doc, "<color:y>q</color>) 快捷(Quickness)\n");
        doc_insert(_doc, "<color:y>p</color>) 威力(Power)\n");
        doc_insert(_doc, "<color:y>e</color>) 易用(Easy Use)\n");
        doc_insert(_doc, "<color:y>r</color>) 再生(Regeneration)\n");
        doc_insert(_doc, "<color:y>h</color>) 维持充能(Hold Charges)\n");
        doc_insert(_doc, "<color:y>x</color>/<color:y>X</color>) 调整加成数量\n");

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);

        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        switch (cmd)
        {
        case '\r':
            *o_ptr = copy;
            return _OK;
        case ESCAPE: return _CANCEL;
        case 'q':
            _toggle(&copy, OF_SPEED);
            break;
        case 'p':
            _toggle(&copy, OF_DEVICE_POWER);
            break;
        case 'e':
            _toggle(&copy, OF_EASY_SPELL);
            if (!copy.pval) /* not normally a pval flag */
                copy.pval = 1;
            break;
        case 'r':
            _toggle(&copy, OF_REGEN);
            if (!copy.pval) /* not normally a pval flag */
                copy.pval = 1;
            break;
        case 'h':
            _toggle(&copy, OF_HOLD_LIFE);
            break;
        case 'x':
            if (copy.pval > 0) copy.pval--;
            else copy.pval = 15;
            break;
        case 'X':
            if (copy.pval < 15) copy.pval++;
            else copy.pval = 0;
            break;
        }
    }
}

/* Top Level Smithing UI */
typedef int (*_smith_fn)(object_type *o_ptr);

typedef struct {
    char choice;
    cptr name;
    _smith_fn smithee;
    object_p pred;
} _command_t, *_command_ptr;

static bool _slays_p(object_type *o_ptr)
{
    return object_is_melee_weapon(o_ptr)
        || object_is_ammo(o_ptr);
}

static bool _brands_p(object_type *o_ptr)
{
    return object_is_melee_weapon(o_ptr)
        || object_is_ammo(o_ptr)
        || object_is_bow(o_ptr)
        || o_ptr->tval == TV_RING;
}

static _command_t _commands[] = {
    { 'p', "附加加成", _smith_plusses, object_is_weapon_armour_ammo },
    { 's', "属性", _smith_stats, object_is_wearable },
    { 'b', "特殊加成", _smith_bonuses, object_is_wearable },
    { 'r', "抗性", _smith_resistances, object_is_wearable },
    { 'a', "能力", _smith_abilities, object_is_wearable },
    { 't', "心灵感应", _smith_telepathies, object_is_wearable },
    { 'S', "屠杀", _smith_slays, _slays_p },
    { 'B', "烙印", _smith_brands, _brands_p },
/*  { 'A', "Activation", _smith_activation, object_is_wearable },*/
    { 'e', "效果", _smith_device_effect, object_is_device },
    { 'b', "特殊加成", _smith_device_bonus, object_is_device },
    { 'R', "重新生成", _smith_reroll },
/*  { 'i', "Ignore", _smith_ignore },*/
    { 0 }
};

static int _smith_object_aux(object_type *o_ptr)
{
    rect_t  r = ui_map_rect();
    vec_ptr v = vec_alloc(NULL);
    int     result = _NONE, i; 

    for (i = 0; ; i++)
    {
        _command_ptr c_ptr = &_commands[i];
        if (!c_ptr->smithee) break;
        if (c_ptr->pred && !c_ptr->pred(o_ptr)) continue;
        vec_add(v, c_ptr);
    }

    while (result == _NONE && vec_length(v))
    {
        int  cmd;

        doc_clear(_doc);
        obj_display_smith(o_ptr, _doc);

        for (i = 0; i < vec_length(v); i++)
        {
            _command_ptr c_ptr = vec_get(v, i);
            doc_printf(_doc, "   <color:y>%c</color>) %s\n",
                c_ptr->choice, c_ptr->name);
        }

        doc_newline(_doc);
        doc_insert(_doc, "<color:y>回车</color>) 接受更改\n");
        doc_insert(_doc, "<color:y>ESC</color>) 取消更改\n");
        doc_newline(_doc);
        Term_load();
        doc_sync_term(_doc, doc_range_all(_doc), doc_pos_create(r.x, r.y));

        cmd = _inkey();
        if (cmd == '\r')
            result = _OK;
        else if (cmd == ESCAPE)
            result = _CANCEL;
        else
        {
            for (i = 0; i < vec_length(v); i++)
            {
                _command_ptr c_ptr = vec_get(v, i);
                if (c_ptr->choice == cmd)
                {
                    assert(c_ptr->smithee);
                    c_ptr->smithee(o_ptr);
                    break;
                }
            }
        }
    }
    vec_free(v);
    return result;
}

static int _smith_object(object_type *o_ptr)
{
    int result = _OK;
    assert(!_doc);
    _doc = doc_alloc(72);
    msg_line_clear();
    Term_save();

    result = _smith_object_aux(o_ptr);

    Term_load();
    doc_free(_doc);
    _doc = NULL;
    return result;
}

static bool _smith_p(object_type *o_ptr)
{
    if (object_is_wearable(o_ptr)) return TRUE;
    if (object_is_ammo(o_ptr)) return TRUE;
    if (object_is_device(o_ptr)) return TRUE;
    if ((prace_is_(RACE_IGOR)) && (object_is_(o_ptr, TV_CORPSE, SV_CORPSE))) return TRUE;
    return FALSE;
}

void wiz_obj_smith(void)
{
    obj_t        copy;
    obj_prompt_t prompt = {0};

    prompt.prompt = "锻造哪件物品？";
    prompt.error = "你没有可以处理的东西。";
    prompt.filter = _smith_p;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    copy = *prompt.obj;
    _obj_identify_fully(&copy);

    msg_line_clear();
    if (_smith_object(&copy) == _OK)
    {
        obj_loc_t loc = prompt.obj->loc; /* re-roll will erase this ... */
        *prompt.obj = copy;
        prompt.obj->loc = loc;
        obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
    }
}

