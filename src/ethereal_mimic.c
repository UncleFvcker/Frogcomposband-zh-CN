#include "angband.h"

#include <stdlib.h>

static bool _can_use_body(int r_idx)
{
    mon_race_ptr race;

    if (r_idx <= 0 || r_idx >= max_r_idx) return FALSE;
    race = &r_info[r_idx];
    if (!race->name) return FALSE;
    if (race->flags1 & RF1_UNIQUE) return FALSE;
    if (race->flagsx & RFX_SUPPRESS) return FALSE;
    if (!race->body.life) return FALSE;
    if (r_idx == MON_TANUKI) return FALSE;
    if (r_idx == MON_KAGE) return FALSE;
    return TRUE;
}

int ethereal_mimic_kill_requirement(int r_idx)
{
    if (r_idx <= 0 || r_idx >= max_r_idx) return 0;
    return MAX(1, r_info[r_idx].level);
}

bool ethereal_mimic_can_mimic(int r_idx)
{
    return _can_use_body(r_idx);
}

bool ethereal_mimic_is_learned(int r_idx)
{
    if (!_can_use_body(r_idx)) return FALSE;
    return r_info[r_idx].r_pkills >= ethereal_mimic_kill_requirement(r_idx);
}

static void _set_current_r_idx(int r_idx)
{
    if (r_idx == p_ptr->current_r_idx) return;

    disturb(1, 0);
    if (!r_idx && p_ptr->current_r_idx)
        msg_format("你停止模仿%s。", monster_race_display_name(p_ptr->current_r_idx));

    possessor_set_current_r_idx(r_idx);

    if (r_idx)
        msg_format("你开始模仿%s。", monster_race_display_name(r_idx));

    equip_shuffle("@mimic1");
    equip_shuffle("@mimic2");
    equip_shuffle("@mimic3");
    equip_shuffle("@mimic4");
    equip_shuffle("@mimic");
}

static void _birth(void)
{
    possessor_on_birth();
    p_ptr->current_r_idx = 0;
    equip_on_change_race();

    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_LEATHER_SCALE_MAIL, 1);
    py_birth_obj_aux(TV_BOW, SV_SHORT_BOW, 1);
    py_birth_obj_aux(TV_ARROW, SV_ARROW, rand_range(15, 30));
    py_birth_food();
    py_birth_light();
}

static void _mimic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->current_r_idx)
            var_set_string(res, "停止模仿");
        else
            var_set_string(res, "模仿");
        break;
    case SPELL_DESC:
        if (p_ptr->current_r_idx)
            var_set_string(res, "返回你的原生形态，并恢复普通人形装备栏位。");
        else
            var_set_string(res, "输入怪物序号，拟态为你已经学会的非唯一怪物形态。");
        break;
    case SPELL_CAST:
    {
        char buf[16] = "";
        int  r_idx;

        var_set_bool(res, FALSE);

        if (p_ptr->current_r_idx)
        {
            _set_current_r_idx(0);
            var_set_bool(res, TRUE);
            return;
        }

        if (!get_string("怪物序号: ", buf, sizeof(buf))) return;
        r_idx = atoi(buf);

        if (r_idx <= 0 || r_idx >= max_r_idx || !r_info[r_idx].name)
        {
            msg_print("没有这个怪物序号。");
            return;
        }
        if (!_can_use_body(r_idx))
        {
            msg_format("你无法模仿这个形态(%s)。", monster_race_display_name(r_idx));
            return;
        }
        if (!ethereal_mimic_is_learned(r_idx))
        {
            msg_format("你还没有学会这个形态(%s: %d/%d)。",
                monster_race_display_name(r_idx),
                r_info[r_idx].r_pkills,
                ethereal_mimic_kill_requirement(r_idx));
            return;
        }

        _set_current_r_idx(r_idx);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _cmp_form(const void *a, const void *b)
{
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    mon_race_ptr ra = &r_info[ia];
    mon_race_ptr rb = &r_info[ib];

    if (ra->level != rb->level) return ra->level - rb->level;
    return strcmp(monster_race_display_name(ia), monster_race_display_name(ib));
}

static void _browse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "浏览形态");
        break;
    case SPELL_DESC:
        var_set_string(res, "浏览已学会的形态和对应怪物序号。");
        break;
    case SPELL_CAST:
    {
        doc_ptr doc = doc_alloc(80);
        int *forms;
        int ct = 0, i;

        C_MAKE(forms, max_r_idx, int);
        for (i = 1; i < max_r_idx; i++)
        {
            if (ethereal_mimic_is_learned(i))
                forms[ct++] = i;
        }
        qsort(forms, ct, sizeof(int), _cmp_form);

        doc_insert(doc, "<topic:Forms><style:heading>已学会的形态</style>\n\n");
        if (!ct)
            doc_insert(doc, "你还没有学会任何形态。\n");
        else
        {
            doc_insert(doc, "序号  等级  击杀  名称\n");
            doc_insert(doc, "========================================\n");
            for (i = 0; i < ct; i++)
            {
                int r_idx = forms[i];
                doc_printf(doc, "%4d  %4d  %4d  %s\n",
                    r_idx, r_info[r_idx].level, r_info[r_idx].r_pkills,
                    monster_race_display_name(r_idx));
            }
        }

        doc_display(doc, "已学会的形态", 0);
        doc_free(doc);
        C_KILL(forms, max_r_idx, int);

        p_ptr->redraw |= PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY | PR_MSG_LINE;
        p_ptr->window |= PW_OVERHEAD | PW_DUNGEON;
        handle_stuff();

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _add_power(power_info *power, int lvl, int cost, int fail, ang_spell fn, int stat)
{
    power->spell.level = lvl;
    power->spell.cost = cost;
    power->spell.fail = fail;
    power->spell.fn = fn;
    power->stat = stat;
}

static power_info *_get_powers(void)
{
    static power_info spells[MAX_SPELLS];
    int ct = 0;
    int max = MAX_SPELLS;

    if (ct < max)
        _add_power(&spells[ct++], 1, 0, 0, _mimic_spell, A_DEX);
    if (ct < max)
        _add_power(&spells[ct++], 1, 0, 0, _browse_spell, A_INT);

    if (p_ptr->current_r_idx)
        ct += possessor_get_powers(spells + ct, max - ct);

    spells[ct].spell.fn = NULL;
    return spells;
}

static void _calc_stats(s16b stats[MAX_STATS])
{
    int i;
    mon_race_ptr race;

    if (!p_ptr->current_r_idx) return;
    race = &r_info[p_ptr->current_r_idx];
    for (i = 0; i < MAX_STATS; i++)
        stats[i] += race->body.stats[i];
}

static void _calc_bonuses(void)
{
    mon_race_ptr race;

    if (!p_ptr->current_r_idx) return;
    race = &r_info[p_ptr->current_r_idx];

    possessor_calc_bonuses();
    if (race->body.life)
        p_ptr->life += race->body.life - 100;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->current_r_idx)
        possessor_get_flags(flgs);
}

static caster_info *_caster_info(void)
{
    if (!p_ptr->current_r_idx) return NULL;
    return possessor_caster_info();
}

static void _character_dump(doc_ptr doc)
{
    int i, ct = 0;

    if (p_ptr->current_r_idx)
    {
        mon_race_ptr race = &r_info[p_ptr->current_r_idx];
        bool old_use_graphics = use_graphics;

        use_graphics = FALSE;
        doc_printf(doc, "<topic:CurrentForm>================================ 当 前 的 形 态 (<color:keypress>C</color>) =================================\n\n");
        mon_display_possessor(race, doc);
        doc_newline(doc);
        use_graphics = old_use_graphics;
    }

    for (i = 1; i < max_r_idx; i++)
    {
        if (ethereal_mimic_is_learned(i))
        {
            if (!ct)
                doc_printf(doc, "<topic:LearnedForms>================================ 已 学 会 的 形 态 (<color:keypress>L</color>) ================================\n\n");
            doc_printf(doc, "%4d  %s\n", i, monster_race_display_name(i));
            ct++;
        }
    }
    if (ct) doc_newline(doc);
}

class_t *ethereal_mimic_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {
        skills_t bs = { 25,  25,  32,   2,  16,   8,  60,  45};
        skills_t xs = { 10,   8,  10,   0,   0,   0,  25,  20};

        me.name = "缥缈模仿者";
        me.desc = "缥缈模仿者是能够学习怪物形态的战士。他们通过击杀非唯一怪物来理解其形体；当击杀数量达到该怪物等级后，就能永久模仿该形态。使用职业能力输入怪物序号即可变身，怪物序号可在怪物知识界面查询。未变身时他们使用普通人形装备栏位；变身后则继承目标形态的身体、抗性、法术、吐息和天然攻击。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 105;
        me.base_hp = 16;
        me.exp = 175;
        me.pets = 35;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_STRONG;

        me.birth = _birth;
        me.get_powers_fn = _get_powers;
        me.calc_stats = _calc_stats;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.character_dump = _character_dump;

        init = TRUE;
    }

    return &me;
}
