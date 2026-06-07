#include "angband.h"

typedef struct
{
    u16b goal_idx;
    u16b dungeon;
    u16b level;
    u16b danger_level;
    byte goal_ct;
    byte killed;
    byte start_lev;
    byte completed_lev;
    u32b completed_turn;
} dpl_quest;

#define MAX_TK_QUEST 15
#define MAX_TK_SPELL 20
#define MAX_TK_TIMEOUT 18
#define _MAX_PACK_SLOTS 12

static byte _q_idx = 0;
static dpl_quest _tk_quests[MAX_TK_QUEST];
static byte _my_spell_taso[2][MAX_TK_SPELL];
static byte _slay_timeouts[MAX_TK_TIMEOUT];
static char *_troika_names[3] = {"冈图扬特", "乌克西普", "索霍格利斯"};
static byte _sohoglyth_reward_level = 0;

#define REW_WRATH       1
#define REW_TY_CURSE    2
#define REW_HURT_LOT    3
#define REW_PISS_OFF    4
#define REW_CURSE_WP    5
#define REW_CURSE_AR    6
#define REW_LOSE_ABL    7
#define REW_RUIN_ABL    8
#define REW_LOSE_EXP    9
#define REW_SUMMON_M    10
#define REW_H_SUMMON    11
#define REW_AFC_WP      12
#define REW_STICKY      13
#define REW_BY_CURSE    14
#define REW_CURSE_EQ    15

#define REW_TROIKA_W    16
#define REW_HEAL_FUL    17
#define REW_DISPEL_C    18
#define REW_POLY_SLF    19
#define REW_GAIN_EXP    20
#define REW_POLY_WND    21
#define REW_DO_HAVOC    22
#define REW_DESTRUCT    23
#define REW_GENOCIDE    24
#define REW_MASS_GEN    25
#define REW_NO_BUFFS    26

#define REW_AUGM_ABL    27
#define REW_GOOD_OBJ    28
#define REW_SHARP_WP    29
#define REW_GREA_OBJ    30
#define REW_GOOD_OBS    31
#define REW_GREA_OBS    32
#define REW_GAIN_ABL    33

#define _MAX_PUN 22
#define _MAX_RND 17
#define _MAX_REW 15

int _troika_punishments[_MAX_PUN] =
{
    REW_WRATH, REW_WRATH, REW_TY_CURSE, REW_HURT_LOT, REW_PISS_OFF,
    REW_CURSE_AR, REW_CURSE_WP, REW_CURSE_AR, REW_CURSE_WP, REW_CURSE_AR,
    REW_CURSE_WP, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL, REW_LOSE_ABL,
    REW_LOSE_EXP, REW_H_SUMMON, REW_SUMMON_M, REW_AFC_WP, REW_CURSE_EQ,
    REW_BY_CURSE, REW_STICKY
};
int _troika_random[_MAX_RND] =
{
    REW_SUMMON_M, REW_TROIKA_W, REW_POLY_WND, REW_POLY_WND, REW_NO_BUFFS,
    REW_POLY_SLF, REW_HEAL_FUL, REW_HEAL_FUL, REW_GAIN_ABL,
    REW_GAIN_EXP, REW_LOSE_EXP, REW_GOOD_OBJ, REW_DESTRUCT,
    REW_GENOCIDE, REW_MASS_GEN, REW_MASS_GEN, REW_DISPEL_C
};
int _troika_rewards[_MAX_REW] =
{
    REW_GOOD_OBJ, REW_GOOD_OBS, REW_TROIKA_W, REW_GREA_OBJ, REW_AUGM_ABL,
    REW_GREA_OBS, REW_GAIN_ABL, REW_GOOD_OBJ, REW_GOOD_OBS, REW_TROIKA_W,
    REW_GREA_OBJ, REW_AUGM_ABL, REW_GREA_OBS, REW_GAIN_ABL, REW_SHARP_WP
};

static int _slay_flags[MAX_TK_TIMEOUT] =
{
    OF_SLAY_ORC, OF_SLAY_ANIMAL, OF_SLAY_TROLL, OF_SLAY_HUMAN, OF_BRAND_FIRE,
    OF_BRAND_COLD, OF_BRAND_ACID, OF_BRAND_ELEC, OF_SLAY_DRAGON, OF_SLAY_GIANT,
    OF_SLAY_DEMON, OF_SLAY_UNDEAD, OF_VORPAL, OF_SLAY_EVIL, OF_BRAND_VAMP,
    OF_BRAND_CHAOS, OF_STUN, OF_BRAND_MANA
};

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_STR;
        me.encumbrance.max_wgt = 500;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 0;
        me.options = CASTER_ALLOW_DEC_MANA;
        init = TRUE;
    }
    return &me;
}

static void _troika_ini_quests(void)
{
    int i;
    _q_idx = 0;
    _sohoglyth_reward_level = 0;
    for (i = 0; i < MAX_TK_QUEST; i++)
    {
        _tk_quests[i].goal_idx = 0;
        _tk_quests[i].dungeon = 0;
        _tk_quests[i].level = 0;
        _tk_quests[i].danger_level = 0;
        _tk_quests[i].goal_ct = 0;
        _tk_quests[i].killed = 0;
        _tk_quests[i].start_lev = 0;
        _tk_quests[i].completed_lev = 0;
        _tk_quests[i].completed_turn = 0;
    }
}

static void _troika_ini_spells(void)
{
    int i;
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        _my_spell_taso[0][i] = 0;
        _my_spell_taso[1][i] = 0;
    }
}

static int _locate_slay_flag(int which)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if (_slay_flags[i] == which) return i;
    }
    return -1;
}

bool troika_timeout_flag(int which)
{
    if (_slay_timeouts[_locate_slay_flag(which)] > 0) return TRUE;
    return FALSE;
}

static void _inc_purple_timeout(int which, int delta)
{
    int vanha, uusi, mika = _locate_slay_flag(which);
    if (mika < 0) return;
    vanha = _slay_timeouts[mika];
    uusi = vanha + delta;
    if (uusi < 0) uusi = 0;
    else if (uusi > 150) uusi = 150;
    _slay_timeouts[mika] = uusi;
    if ((vanha > 0) && (uusi > 0)) return;
    if ((!vanha) && (!uusi)) return;
    if (uusi)
    {
        switch (which)
        {
             case OF_SLAY_ORC: msg_print("你成为了兽人的巨大克星！"); break;
             case OF_SLAY_ANIMAL: msg_print("你成为了动物的巨大克星！"); break;
             case OF_SLAY_TROLL: msg_print("你成为了巨魔的巨大克星！"); break;
             case OF_SLAY_HUMAN: msg_print("你成为了人类的巨大克星！"); break;
             case OF_SLAY_GIANT: msg_print("你成为了巨人的巨大克星！"); break;
             case OF_SLAY_DRAGON: msg_print("你成为了龙的巨大克星！"); break;
             case OF_SLAY_DEMON: msg_print("你成为了恶魔的巨大克星！"); break;
             case OF_SLAY_UNDEAD: msg_print("你成为了死灵的巨大克星！"); break;
             case OF_SLAY_EVIL: msg_print("你以巨大的力量重击邪恶怪物！"); break;
             case OF_BRAND_FIRE: msg_print("你的武器被火焰包裹！"); break;
             case OF_BRAND_COLD: msg_print("你的武器被冰霜包裹！"); break;
             case OF_BRAND_ACID: msg_print("你的武器被强酸包裹！"); break;
             case OF_BRAND_POIS: msg_print("你的武器被毒液包裹！"); break;
             case OF_BRAND_ELEC: msg_print("你的武器闪烁着电光！"); break;
             case OF_BRAND_VAMP: msg_print("你的武器变得吸血！"); break;
             case OF_BRAND_CHAOS: msg_print("你的武器变得混乱！"); break;
             case OF_VORPAL: msg_print("你的武器突然感觉非常锋利！"); break;
             case OF_STUN: msg_print("你的武器变得极其令人震慑！"); break;
             default: break; /* ?? */
        }
    }
    else
    {
        switch (which)
        {
             case OF_SLAY_ORC: msg_print("你不再是兽人的巨大克星。"); break;
             case OF_SLAY_ANIMAL: msg_print("你不再是动物的巨大克星。"); break;
             case OF_SLAY_TROLL: msg_print("你不再是巨魔的巨大克星。"); break;
             case OF_SLAY_HUMAN: msg_print("你不再是人类的巨大克星。"); break;
             case OF_SLAY_GIANT: msg_print("你不再是巨人的巨大克星。"); break;
             case OF_SLAY_DRAGON: msg_print("你不再是龙的巨大克星。"); break;
             case OF_SLAY_DEMON: msg_print("你不再是恶魔的巨大克星。"); break;
             case OF_SLAY_UNDEAD: msg_print("你不再是死灵的巨大克星。"); break;
             case OF_SLAY_EVIL: msg_print("你不再以巨大的力量重击邪恶怪物。"); break;
             case OF_BRAND_FIRE: msg_print("你武器周围的火焰消失了。"); break;
             case OF_BRAND_COLD: msg_print("你的武器不再被冰霜包裹。"); break;
             case OF_BRAND_ACID: msg_print("你的武器不再被强酸包裹。"); break;
             case OF_BRAND_POIS: msg_print("你的武器不再被毒液包裹。"); break;
             case OF_BRAND_ELEC: msg_print("你的武器不再闪烁电光。"); break;
             case OF_BRAND_VAMP: msg_print("你的武器不再吸血。"); break;
             case OF_BRAND_CHAOS: msg_print("你的武器不再混乱。"); break;
             case OF_VORPAL: msg_print("你的武器突然感觉没那么锋利了。"); break;
             case OF_STUN: msg_print("你的武器不再震慑敌人。"); break;
             default: break; /* ?? */
        }
    }
    p_ptr->redraw |= PR_STATUS;
}

void troika_wipe_timeouts(void)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++) _slay_timeouts[i] = 0;
}

static void _birth(void)
{
    disciple_birth(); 
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_CHAIN_MAIL, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_CONFUSION, 2);
    _troika_ini_quests();
    _troika_ini_spells();
    troika_wipe_timeouts();
}

static void _make_troika_weapon(int sval)
{
    int k_idx = lookup_kind(TV_SWORD, sval);
    object_type forge;
    if (k_idx < 1) return;
    object_prep(&forge, k_idx);
    apply_magic_ego = EGO_WEAPON_TROIKA;
    apply_magic(&forge, (p_ptr->max_plv * 5 / 3) + 8, AM_GOOD | AM_GREAT | AM_FORCE_EGO);
    object_origins(&forge, ORIGIN_PATRON);
    drop_near(&forge, -1, py, px);
}

static void _troika_event(int effect)
{
    char wrath_reason[24] = "the Wrath of Guntujant";
    int count = 0, dummy;

    if (p_ptr->is_dead) return;

    switch (effect)
    {
        case REW_POLY_SLF:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你需要一个新的形态，凡人！”");

            do_poly_self();
            break;
        case REW_GAIN_EXP:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“干得好，凡人！继续前进！”");
            if (p_ptr->prace == RACE_ANDROID)
                msg_print("但是，什么也没有发生。");
            else if (p_ptr->exp < PY_MAX_EXP)
            {
                s32b ee = (p_ptr->exp / 2) + 10;
                if (ee > 100000L) ee = 100000L;
                msg_print("你感觉经验增加了。");
                gain_exp(ee);
            }
            break;
        case REW_LOSE_EXP:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你不配拥有那个，奴隶。”");

            if (p_ptr->prace == RACE_ANDROID)
            {
                nonlethal_ty_substitute(TRUE);
            }
            else
            {
                lose_exp(p_ptr->exp / 6);
            }
            break;
        case REW_GOOD_OBJ:
            msg_format("%s低语道：",_troika_names[0]);
            cmsg_print(TERM_VIOLET, "“明智地使用我的礼物。”");
            acquirement(py, px, 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBJ:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“明智地使用我的礼物。”");

            acquirement(py, px, 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_TROIKA_W:
        {
            int dummy2;
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你的行为为你赢得了一把配得上你的利刃。”");
            switch (randint1(p_ptr->lev - (p_ptr->lev / 5)))
            {
                case 0: case 1:
                    dummy2 = SV_DAGGER;
                    break;
                case 2: case 3:
                    dummy2 = SV_MAIN_GAUCHE;
                    break;
                case 4:
                    dummy2 = SV_TANTO;
                    break;
                case 5: case 6:
                    dummy2 = SV_RAPIER;
                    break;
                case 7: case 8:
                    dummy2 = SV_SMALL_SWORD;
                    break;
                case 9: case 10:
                    dummy2 = SV_BASILLARD;
                    break;
                case 11: case 12: case 13:
                    dummy2 = SV_SHORT_SWORD;
                    break;
                case 14: case 15:
                    dummy2 = SV_SABRE;
                    break;
                case 16: case 17:
                    dummy2 = SV_CUTLASS;
                    break;
                case 18:
                    dummy2 = SV_WAKIZASHI;
                    break;
                case 19:
                    dummy2 = SV_KHOPESH;
                    break;
                case 20:
                    dummy2 = SV_TULWAR;
                    break;
                case 21:
                    dummy2 = SV_BROAD_SWORD;
                    break;
                case 22: case 23:
                    dummy2 = SV_LONG_SWORD;
                    break;
                case 24: case 25:
                    dummy2 = SV_SCIMITAR;
                    break;
                case 26:
                    dummy2 = SV_NINJATO;
                    break;
                case 27:
                    dummy2 = SV_KATANA;
                    break;
                case 28: case 29:
                    dummy2 = SV_BASTARD_SWORD;
                    break;
                case 30:
                    dummy2 = SV_GREAT_SCIMITAR;
                    break;
                case 31:
                    dummy2 = SV_CLAYMORE;
                    break;
                case 32:
                    dummy2 = SV_ESPADON;
                    break;
                case 33:
                    dummy2 = SV_TWO_HANDED_SWORD;
                    break;
                case 34:
                    dummy2 = SV_FLAMBERGE;
                    break;
                case 35:
                    dummy2 = SV_NO_DACHI;
                    break;
                case 36:
                    dummy2 = SV_EXECUTIONERS_SWORD;
                    break;
                case 37:
                    dummy2 = SV_ZWEIHANDER;
                    break;
                case 38:
                    dummy2 = SV_HAYABUSA;
                    break;
                case 39:
                    dummy2 = SV_BLADE_OF_CHAOS;
                    break;
                default:
                    dummy2 = (one_in_(3)) ? SV_DIAMOND_EDGE : SV_BLADE_OF_CHAOS;
                    break;
            }
            _make_troika_weapon(dummy2);
            break;
        }
        case REW_GOOD_OBS:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你的行为为你赢得了应得的奖励。”");

            acquirement(py, px, randint1(2) + 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBS:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“看吧，凡人，我多么慷慨地奖励你的忠诚。”");

            acquirement(py, px, randint1(2) + 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_TY_CURSE:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你变得越来越傲慢了，凡人。”");

            if (p_ptr->lev < 30) nonlethal_ty_substitute(TRUE);
            else activate_ty_curse(FALSE, &count);
            break;
        case REW_BY_CURSE:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你变得越来越傲慢了，凡人。”");

            nonlethal_ty_substitute(TRUE);
            break;
        case REW_SUMMON_M:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“我的宠物们，消灭这个傲慢的凡人！”");
            for (dummy = 0; dummy < randint1(5) + 1; dummy++)
                summon_specific(0, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            break;
        case REW_H_SUMMON:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你需要更强的对手！”");
            activate_hi_summon(py, px, FALSE);
            break;
        case REW_DO_HAVOC:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“死亡与毁灭！这让我很高兴！”");
            call_chaos(100);
            break;
        case REW_GAIN_ABL:
            msg_format("%s的声音回荡着：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“留下，凡人，让我来塑造你。”");
                do_inc_stat(randint0(6));
            break;
        case REW_LOSE_ABL:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“我已经厌倦你了，凡人。”");
                do_dec_stat(randint0(6));
            break;
        case REW_RUIN_ABL:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“你需要学会谦卑，凡人！”");
            msg_print("你感觉力量减弱了！");

            for (dummy = 0; dummy < 6; dummy++)
                dec_stat(dummy, 10 + randint1(15), TRUE);
            break;
        case REW_POLY_WND:
            msg_format("你感觉到%s的力量触碰了你。", _troika_names[0]);
            do_poly_wounds();
            break;
        case REW_AUGM_ABL:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“收下我这份微薄的礼物吧！”");
            for (dummy = 0; dummy < 6; dummy++)
                do_inc_stat(dummy);
            break;
        case REW_HURT_LOT:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“受苦吧，可悲的蠢货！”");
            fire_ball(GF_DISINTEGRATE, 0, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), 4);
            take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), wrath_reason);
            break;
       case REW_HEAL_FUL:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“站起来，我的仆人！”");
            restore_level();
            set_poisoned(0, TRUE);
            set_blind(0, TRUE);
            set_confused(0, TRUE);
            set_image(0, TRUE);
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            hp_player(5000);
            for (dummy = 0; dummy < 6; dummy++)
                do_res_stat(dummy);
            break;
        case REW_CURSE_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                msg_format("%s的声音如雷鸣般响起：",
                    _troika_names[0]);
                cmsg_print(TERM_VIOLET, "“你太依赖你的武器了。”");
                curse_weapon(FALSE, slot);
            }
            else nonlethal_ty_substitute(TRUE);
            break;
        }
        case REW_SHARP_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                if (one_in_(4))
                {
                    object_type *o_ptr = equip_obj(slot);
                    if ((o_ptr) && (o_ptr->k_idx))
                    {
                         u32b flgs[OF_ARRAY_SIZE];
                         obj_flags(o_ptr, flgs);
                         if (!have_flag(flgs, OF_VORPAL))
                         {
                             msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
                             cmsg_print(TERM_VIOLET, "“你需要一把更锋利的武器！”");
                             add_flag(o_ptr->flags, OF_VORPAL);
                             add_flag(o_ptr->known_flags, OF_VORPAL);
                         }
                    }
                }
                else
                {
                    msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
                    cmsg_print(TERM_VIOLET, "“你需要一把更锋利的武器！”");
                    _inc_purple_timeout(OF_VORPAL, 50);
                }
            }
            break;
        }
        case REW_AFC_WP:
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if ((o_ptr) && (o_ptr->k_idx) && (!(o_ptr->curse_flags & OFC_TY_CURSE)))
                {
                    bool _perm = (one_in_(4));
                    if (_perm)
                    {
                        add_flag(o_ptr->flags, OF_TY_CURSE);
                    }
                    o_ptr->curse_flags |= (OFC_HEAVY_CURSE | OFC_TY_CURSE | OFC_CURSED);
                    o_ptr->known_curse_flags |= (OFC_HEAVY_CURSE | OFC_TY_CURSE | OFC_CURSED);
                    msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
                    cmsg_format(TERM_VIOLET, "“我在此对你的武器降下鲜血诅咒；它将%s你敌人的希望灯塔，并在你需要时背叛你。”", _perm ? "永远是" : "成为");
                    p_ptr->update |= PU_BONUS;
                }
            }
            break;
        }
        case REW_STICKY:
        {
            int slot = equip_random_slot(obj_exists);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if ((o_ptr) && (o_ptr->k_idx) && (!(o_ptr->curse_flags & OFC_PERMA_CURSE)))
                {
                    char o_name[MAX_NLEN];
                    strip_name(o_name, o_ptr->k_idx);
                    if ((o_ptr->name2 == EGO_SPECIAL_BLASTED) && (!one_in_(13))) /* Let's be nice and not perma-curse blasted equipment... */
                    {
                        msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
                        cmsg_print(TERM_VIOLET, "“受苦吧，可悲的凡人！”");
                        take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->mhp * 2 / 3, p_ptr->lev * 4), wrath_reason);
                        nonlethal_ty_substitute(TRUE);
                        break;
                    }
                    o_ptr->curse_flags |= (OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED);
                    msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
                    cmsg_format(TERM_VIOLET, "“我在此对你降下诅咒，凡人：你将终生%s这件相同的%s，直到你的末日，并且永远无法将其卸下。”", object_is_weapon(o_ptr) ? "挥舞" : "穿戴", o_name);
                    p_ptr->update |= PU_BONUS;
                }
            }
            break;
        }
        case REW_CURSE_AR:
        {
            int slot = equip_random_slot(object_is_armour);
            if (slot)
            {
                msg_format("%s的声音如雷鸣般响起：",
                    _troika_names[0]);
                cmsg_print(TERM_VIOLET, "“你太依赖你的装备了。”");
                curse_armor(slot);
            }
            else nonlethal_ty_substitute(TRUE);
            break;
        }
        case REW_CURSE_EQ:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“让诅咒降临在你的装备上！”");
            curse_equipment(100, 50);
            break;
        case REW_PISS_OFF:
            msg_format("%s低语道：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“现在你将为惹恼我付出代价。”");
            switch (randint1(4))
            {
                case 1:
                    if ((p_ptr->lev < 39) || (one_in_(2))) nonlethal_ty_substitute(TRUE);
                    else activate_ty_curse(FALSE, &count);
                    break;
                case 2:
                    activate_hi_summon(py, px, FALSE);
                    break;
                case 3:
                    if (one_in_(2))
                    {
                        int slot = equip_random_slot(object_is_melee_weapon);
                        if (slot)
                            curse_weapon(FALSE, slot);
                        else
                            nonlethal_ty_substitute(TRUE);
                    }
                    else
                    {
                        int slot = equip_random_slot(object_is_armour);
                        if (slot)
                            curse_armor(slot);
                        else
                            nonlethal_ty_substitute(TRUE);
                    }
                    break;
                default:
                    for (dummy = 0; dummy < 6; dummy++)
                        dec_stat(dummy, 10 + randint1(15), TRUE);
                    break;
            }
            break;
        case REW_WRATH:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“死吧，凡人！”");

            take_hit(DAMAGE_LOSELIFE, MIN(p_ptr->mhp * 2 / 3, p_ptr->lev * 4), wrath_reason);
            if (p_ptr->chp < 0) break; /* We've probably done enough */
            for (dummy = 0; dummy < 6; dummy++)
                dec_stat(dummy, 10 + randint1(15), FALSE);
            activate_hi_summon(py, px, FALSE);
            if ((p_ptr->lev < 39) || (!one_in_(8))) nonlethal_ty_substitute(TRUE);
            else activate_ty_curse(FALSE, &count);
            if (one_in_(2))
            {
                int slot = equip_random_slot(object_is_melee_weapon);
                if (slot)
                    curse_weapon(FALSE, slot);
            }
            if (one_in_(2))
            {
                int slot = equip_random_slot(object_is_armour);
                if (slot)
                    curse_armor(slot);
            }
            break;
        case REW_NO_BUFFS:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“回归常态！这让我很高兴！”");
            reset_tim_flags();
            break;
        case REW_DESTRUCT:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“死亡与毁灭！这让我很高兴！”");
            destroy_area(py, px, 25, 3 * p_ptr->lev);
            break;
        case REW_GENOCIDE:
            msg_format("%s的声音如雷鸣般响起：",
                _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“让我来为你解除压迫吧！”");
            symbol_genocide(0, FALSE);
            break;
        case REW_MASS_GEN:
            msg_format("%s的声音如雷鸣般响起：", _troika_names[0]);
            cmsg_print(TERM_VIOLET, "“让我来为你解除压迫吧！”");
            mass_genocide(0, FALSE);
            break;
        case REW_DISPEL_C:
            msg_format("你感觉到%s的力量袭击了你的敌人！", _troika_names[0]);
            dispel_monsters(p_ptr->lev * 4);
            break;
/*        case REW_IGNORE:
            msg_format("%s ignores you.", _troika_names[0]);
            break;*/
        default:
            msg_format("%s结结巴巴地说：", _troika_names[0]);
            msg_format("“呃……呃……答案是 %d，问题是什么来着？”", effect);
            break;
    }
}

static void _troika_punish(void)
{
    int type = randint0(_MAX_PUN);
    if ((type == _MAX_PUN - 1) && (one_in_(2))) type = randint0(_MAX_PUN);
    _troika_event(_troika_punishments[type]);
}
static void _troika_random_effect(void)
{
    int type = randint0(_MAX_RND);
    _troika_event(_troika_random[type]);
}

static void _troika_reward(void)
{
    int type = randint0(_MAX_REW * 5 / 3);
    if (p_ptr->is_dead) return;
    if (type >= _MAX_REW)
    {
        int count;
        count = mut_count(mut_unlocked_pred);
        if (count>randint1(5))
        {
            if (one_in_(3))
            {
                int newmuts = 1;
                cmsg_format(TERM_VIOLET, "%s用新的变异奖励了你！", _troika_names[0]);
                mut_gain_random(NULL);
                while (one_in_(newmuts)) 
                {
                    mut_lose_random(NULL);
                    mut_gain_random(NULL);
                    newmuts++;
                }
            }
            else
            {
                cmsg_format(TERM_VIOLET, "%s用一个新的变异奖励了你！", _troika_names[0]);
                mut_gain_random(NULL);
            }
        }
        else
        {
            {
                cmsg_format(TERM_VIOLET, "%s用一个变异奖励了你！", _troika_names[0]);
                mut_gain_random(NULL);
            }
        }
    }
    else 
    {
        _troika_event(_troika_rewards[type]);
    }
}

void troika_effect(int reason)
{
    int punish_chance = 0;
    int reward_chance = 0;
    if (reason) switch (reason)
    {
    case TROIKA_HIT:
        if (one_in_(343)) /* 343 */
        {
            punish_chance = 39; /* 39 */
            reward_chance = 21; /* 21 */
        }
        break;
    case TROIKA_KILL_WEAK:
        if (one_in_(216))
        {
            punish_chance = 13;
            reward_chance = 169;
        }
        break;
    case TROIKA_KILL:
        if (one_in_(81))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_KILL_UNIQUE:
        if (one_in_(7))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_KILL_FAMOUS:
        if (one_in_(2))
        {
            punish_chance = 666;
            reward_chance = 3;
        }
        break;
    case TROIKA_KILL_GOOD:
        if (one_in_(7))
        {
            punish_chance = 666;
            reward_chance = 3;
        }
        break;
    case TROIKA_KILL_DEMON:
        if (one_in_(27))
        {
            punish_chance = 13;
            reward_chance = 7;
        }
        break;
    case TROIKA_CAST:
        if (one_in_(131))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_VILLAINY:
        if (one_in_(21))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_CHANCE:
        if (one_in_(14))
        {
            punish_chance = 39;
            reward_chance = 7;
        }
        break;
    case TROIKA_TAKE_HIT:
        if (one_in_(7))
        {
            punish_chance = 39;
            reward_chance = 21;
        }
        break;
    case TROIKA_TELEPORT:
        if ((one_in_(7)) && (p_ptr->chp > p_ptr->mhp / 4) && (p_ptr->chp != p_ptr->mhp))
        {
            punish_chance = (randint1(100) < (p_ptr->chp * 100 / p_ptr->mhp)) ? 2 : 77;
            reward_chance = 28;
        }
        break;
    default:
        break;
    }
    if (punish_chance && reward_chance) 
    {
        if (one_in_(punish_chance))
        {
            _troika_punish();
        }
        else if (one_in_(reward_chance))
        {
            _troika_reward();
        }
        else
        {
            _troika_random_effect();
        }
    }
}

static void _gain_level(int new_level)
{
    if (new_level > 1)
    {
        if (one_in_(2))
        {
            _troika_reward();
        }
        else if (one_in_(13))
        {
            _troika_punish();
        }
        else
        {
            _troika_random_effect();
        }
    }
}

static void _troika_quest_cleanup(quest_ptr q)
{
    q->status = QS_UNTAKEN;
    q->level = 0;
    q->completed_lev = 0;
    q->completed_turn = 0;
    q->goal_current = 0;
    q->goal_idx = 0;
    q->goal_count = 0;
}

void troika_punish_quest_fail(void)
{
    msg_print("一个遥远的声音咆哮道：");
    cmsg_print(TERM_VIOLET, "“这种懦弱是对乌克西普的亵渎！”");
    take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 6, p_ptr->mhp / 2), "the Wrath of Uxip");
    switch (randint0(5))
    {
        case 0:
        case 1:
            _troika_event(REW_CURSE_WP);
            break;
        case 2:
        case 3:
            _troika_event(REW_CURSE_AR);
            break;
        default:
            _troika_event(REW_STICKY);
            break;
    }
}

static void _give_reward(void)
{
    int voitot = 0, tappiot = 0, hyvaksy, i;
    for (i = 0; i < _q_idx - 1; i++)
    {
        if ((_tk_quests[i].completed_turn) && (_tk_quests[i].killed == _tk_quests[i].goal_ct)) voitot++;
        else tappiot++;
    }
    hyvaksy = (_q_idx / 5) - tappiot;
    if (hyvaksy > _sohoglyth_reward_level)
    {
        _sohoglyth_reward_level++;
        msg_format("%s的声音充满了地下城：", _troika_names[2]);
        switch (_sohoglyth_reward_level)
        {
            case 1:
                cmsg_print(TERM_VIOLET, "“今日你在战斗中证明了自己，并在为我们服务的过程中克服了巨大的危险。看吧，这是你的奖励：我将赐予你神圣的视力，使你能够察觉盘踞在这个地下城中的邪恶灵魂，即使它们还在靠近，也没有任何敌人能出其不意地袭击你。”");
                break;
            case 2:
                if (p_ptr->personality != PERS_MUNCHKIN) cmsg_print(TERM_VIOLET, "“在我们的仆人中，你是值得称道的。你今日的作为为你赢得了丰厚的奖励。看吧，这是我赐予你的礼物：你现在无需触碰便能察觉物品的品质，哪怕是在洞穴的另一端，仿佛你就是一个‘踢门团玩家(Munchkin)’一样。”");
                else cmsg_print(TERM_VIOLET, "“我们本来为你准备了一份丰厚的奖励和一段华丽的辞藻，但你并不真的需要它们，对吧？”");
                break;
            case 3:
                if (!prace_is_(RACE_SPECTRE)) cmsg_print(TERM_VIOLET, "“确实，你是我们真正的信徒，在所有凡人中脱颖而出，成为了传说中的英雄。那么，这就是你的奖励：你不再受凡人生命的限制，可以穿透坚硬的岩石，仿佛你是一个虚无的幽灵，除了永久的墙壁，没有什么能阻挡你。”");
                else cmsg_print(TERM_VIOLET, "“我们本来为你准备了一份丰厚的奖励和一段华丽的辞藻，但你并不真的需要它们，对吧？");
                break;
            default:
                cmsg_print(TERM_VIOLET, "“好像有什么地方不对劲……”");
                break;
        }
        p_ptr->update |= PU_BONUS;
    }
    else
    {
        int maali = (_sohoglyth_reward_level + 1) * 5 + (tappiot * 4);
        int tarve = MAX(1, maali - voitot - 1);
        if (maali > MAX_TK_QUEST - tappiot) return;
        msg_format("%s的声音回荡着：", _troika_names[2]);
        if (tarve == 1)
        {
            cmsg_print(TERM_VIOLET, "“干得好，凡人！再完成一个任务，我将给你丰厚的奖励。”");
        }
        else cmsg_format(TERM_VIOLET, "“干得好，凡人！现在再完成 %d 个任务，你将获得极大的奖励。”", tarve);
    }
}

void troika_quest_finished(quest_ptr q, bool success)
{
    int _tq = _q_idx - 1;
    _tk_quests[_tq].killed = q->goal_current;
    if (success) _tk_quests[_tq].killed = q->goal_count; /* paranoia */
    _tk_quests[_tq].completed_lev = (prace_is_(RACE_ANDROID)) ? p_ptr->lev : p_ptr->max_plv;
    _tk_quests[_tq].completed_turn = game_turn;
    _troika_quest_cleanup(q);
    if (!success)
    {
        msg_format("%s低语道：<color:v>你是故意让任务失败的吗？我们将你置于所有其他凡人之上，赐予了你多数人宁愿出卖灵魂也想换取的力量……这就是你表达感激的方式吗？</color>", _troika_names[2]);
        return;
    }
    else
    {
        /* No need to create stairs because purple quests allow normal stair generation
         * (indeed, generating stairs would be risky because purple quests can appear
         * at the bottom of a dungeon) */
        cmsg_print(TERM_L_BLUE, "你刚刚完成了你的任务！");
        msg_add_tiny_screenshot(50, 24);
        p_ptr->redraw |= PR_DEPTH;
        _give_reward();
    }
}

quest_ptr troika_get_quest(int dungeon, int level)
{
    point_t tbl[2] = { {26, 134}, {50, 182} };
    int mahis = (no_wilderness ? 5 : 9);
    int verrokki = isompi(p_ptr->max_plv + 5, (p_ptr->max_plv * interpolate(p_ptr->max_plv, tbl, 2) / 100) + randint1(mahis) - 3);
    quest_ptr q;

    /* No quest if level too low */
    if (level < verrokki) return NULL;

    /* Not too early */
    if (p_ptr->max_plv < 3) return NULL;

    /* Not too many quests */
    if (_q_idx >= MAX_TK_QUEST) return NULL;

    /* Paranoia */
    if (!dungeon) return NULL;
    if ((level == d_info[dungeon].maxdepth) && (!dungeon_conquered(dungeon))) return NULL;

    /* Not in Chameleon Cave */
    if (dungeon == DUNGEON_CHAMELEON) return NULL;

    if (_q_idx > 2)
    {
        int i, monesko = 0;
        for (i = 0; i < _q_idx; i++)
        {
            if ((_tk_quests[i].completed_turn) && (_tk_quests[i].killed != _tk_quests[i].goal_ct)) monesko++;
        }
        if (monesko > 2 - _sohoglyth_reward_level) return NULL; /* Side-quests stop appearing once it's impossible to gain further rewards */
    }

    /* Not too many quests in one dungeon */
    if (!no_wilderness)
    {
        int i, osumat = 0, limit = (dungeon == DUNGEON_ANGBAND) ? 5 : 2;
        for (i = 0; i < _q_idx; i++)
        {
            if (_tk_quests[i].dungeon == dungeon) osumat++;
            if (osumat >= limit) break;
        }
        if (osumat >= limit) return NULL;
        /* Never guarantee a quest */
        if (magik(60))
        {
            return NULL;
        }
    }

    /* Not too many quests at this clvl, especially if the danger level is decreasing */
    {
        int i, osumat = 0, huippu = 0;
        for (i = 0; i < _q_idx; i++)
        {
            if (_tk_quests[i].start_lev == p_ptr->max_plv)
            {
                osumat++;
                if (_tk_quests[i].level > huippu) huippu = _tk_quests[i].level;
            }
        }
        if (osumat >= 4) return NULL;
        if ((osumat >= 1) && (level < huippu + osumat * 3)) return NULL;
    }

    /* Roll a new quest */
    q = quests_get(PURPLE_QUEST);
    q->level = MIN(88, (MAX(p_ptr->max_plv + 5, (p_ptr->max_plv * interpolate(p_ptr->max_plv, tbl, 2) / 100)) + level) / 2);
    q->dungeon = dungeon;
    get_purple_questor(q);
    q->goal = QG_KILL_MON;
    q->status = QS_IN_PROGRESS;
    q->level = level;
    q->danger_level = level;

    /* Store the details permanently */
    _tk_quests[_q_idx].goal_idx = q->goal_idx;
    _tk_quests[_q_idx].goal_ct = q->goal_count;
    _tk_quests[_q_idx].dungeon = q->dungeon;
    _tk_quests[_q_idx].start_lev = p_ptr->max_plv;
    _tk_quests[_q_idx].killed = 0;
    _tk_quests[_q_idx].completed_lev = 0;
    _tk_quests[_q_idx].completed_turn = 0;
    _tk_quests[_q_idx].level = level;
    _tk_quests[_q_idx].danger_level = r_info[q->goal_idx].level;

    _q_idx++;

    return q;
}

static void _plasma_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 6);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "等离子箭");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发等离子箭。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_PLASMA, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _chaos_bolt_spell(int cmd, variant *res)
{
    int dice = 8 + (p_ptr->lev / 6);
    int sides = 8;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混沌箭");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发混沌箭。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_CHAOS, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_burst_spell(int cmd, variant *res)
{
    int dice = 3;
    int sides = 5;
    int rad = spell_power((p_ptr->lev < 30) ? 2 : 3);
    int base = p_ptr->lev + p_ptr->lev / 3;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力爆发");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dice), sides, spell_power(p_ptr->to_d_spell + base)));
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一颗魔法球。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_MISSILE, dir, spell_power(damroll(dice, sides) + p_ptr->to_d_spell + p_ptr->lev + base), rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _triple_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "三重攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "同时向三个相邻方向攻击。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int cdir;
        int y, x;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        if (dir == 5) break;

        for (cdir = 0;cdir < 8; cdir++)
        {
            if (cdd[cdir] == dir) break;
        }

        if (cdir == 8) break;

        y = py + ddy_cdd[cdir];
        x = px + ddx_cdd[cdir];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("你攻击了空气。");
        y = py + ddy_cdd[(cdir + 7) % 8];
        x = px + ddx_cdd[(cdir + 7) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("你攻击了空气。");
        y = py + ddy_cdd[(cdir + 1) % 8];
        x = px + ddx_cdd[(cdir + 1) % 8];
        if (cave[y][x].m_idx)
            py_attack(y, x, 0);
        else
            msg_print("你攻击了空气。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wave_of_death_spell(int cmd, variant *res)
{
    int sides = p_ptr->lev * 3;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "死亡之波");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, spell_power(sides), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害视线内的所有活物。");
        break;
    case SPELL_CAST:
    {
        dispel_living(spell_power(randint1(sides) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _berserk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂战士之怒");
        break;
    case SPELL_DESC:
        var_set_string(res, "使你陷入狂战士般的狂热状态，获得极大的战斗加成，但失去清晰思考的能力。此外还会为你提供祝福，并在高等级时短暂提供急速效果。");
        break;
    case SPELL_CAST:
    {
        int pituus = 10 + randint1(p_ptr->lev);
        int pituus2 = p_ptr->lev - 32;
        msg_print("嗷啊！你想打点什么。");
        set_shero(pituus, FALSE);
        set_blessed(pituus, FALSE);
        if (pituus2 > 0) set_fast(p_ptr->fast + pituus2, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rock_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎岩击");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁一堵墙，或对石头制成的怪物进行强力攻击。");
        break;
    case SPELL_CAST:
    {
        int dir, y, x;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        if (dir == 5) break;

        y = py + ddy[dir];
        x = px + ddx[dir];

        if (cave[y][x].m_idx)
        {
            py_attack(y, x, HISSATSU_HAGAN);
            var_set_bool(res, TRUE);
        }
    
        if (!cave_have_flag_bold(y, x, FF_HURT_ROCK)) break;

        cave_alter_feat(y, x, FF_HURT_ROCK);
        p_ptr->update |= (PU_FLOW);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_vamp_spell(int cmd, variant *res)
{
    int dam = spell_power(70 + p_ptr->to_d_spell/3);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫光吸血");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射 3 发魔法箭。每发箭都会从活着的怪物身上吸收一些生命值并将其转移给你。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("伤害 3*%d", dam));
        break;
    case SPELL_CAST:
    {
        int i, dir;

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) break;

        virtue_add(VIRTUE_SACRIFICE, -1);
        virtue_add(VIRTUE_VITALITY, -1);

        for (i = 0; i < 3; i++)
        {
            if (drain_life(dir, dam) && p_ptr->pclass != CLASS_BLOOD_MAGE)
                 vamp_player(dam);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _wall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "石墙术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在周围所有开阔的方格上制造墙壁。");
        break;
    case SPELL_CAST:
        wall_stone();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slaughter_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫光杀戮");
        break;
    case SPELL_DESC:
        var_set_string(res, "执行一系列冲锋攻击，只要能击杀怪物且你还有法力值(SP)就会持续进行。");
        break;
    case SPELL_CAST:
    {
        const int mana_cost_per_monster = 8;
        bool uusi = TRUE;
        bool mdeath;
        var_set_bool(res, FALSE);
        do
        {
            if (!rush_attack(5, &mdeath)) break;
            if (uusi)
            {
                /* Reserve needed mana point */
                p_ptr->csp -= calculate_cost(40);
                uusi = FALSE;
            }
            else
                p_ptr->csp -= mana_cost_per_monster;

            if (!mdeath) break;
            command_dir = 0;

            p_ptr->redraw |= PR_MANA;
            handle_stuff();
        }
        while (p_ptr->csp > mana_cost_per_monster);

        if (uusi) break;
    
        /* Restore reserved mana */
        p_ptr->csp += calculate_cost(40);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _nether_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幽冥风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一颗巨大的幽冥球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 9 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        fire_ball(GF_NETHER, dir, spell_power(p_ptr->lev * 9 + 50 + p_ptr->to_d_spell), 3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一颗巨大的纯粹法力球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(10), spell_power(p_ptr->lev * 6 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        msg_print("你施放了法力风暴。");
        fire_ball(GF_MANA, dir, spell_power(p_ptr->lev * 6 + 50 + damroll(10, 10) + p_ptr->to_d_spell), 4);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_orc_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀兽人");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀兽人的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_ORC, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_animal_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀动物");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀动物的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_ANIMAL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_troll_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀巨魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀巨魔的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_TROLL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_human_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀人类");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀人类的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_HUMAN, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_fire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰之舌");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时附带火焰烙印。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_FIRE, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_cold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "凛冬之寒");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时附带冰霜烙印。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_COLD, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_acid_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "腐蚀涂层");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时附带强酸烙印。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_ACID, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _brand_elec_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "雷霆之击");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时附带闪电烙印。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_ELEC, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_giant_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀巨人");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀巨人的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_GIANT, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀龙类");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀龙类的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_DRAGON, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _long_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "延伸攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "攻击两格外的怪物。");
        break;
    case SPELL_CAST:
        {
            int dir = 5;
            bool b = FALSE;

            project_length = 2;
            if (get_fire_dir(&dir))
            {
                project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
                b = TRUE;
            }
            var_set_bool(res, b);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rush_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "冲锋攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "靠近怪物并攻击它。");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(2, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀恶魔的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_DEMON, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _slay_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "屠杀死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀死灵的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_UNDEAD, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_sharp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫光锋锐");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时变得锋利。（已经具备“锋锐”或“*锋锐*”属性的武器不会进一步提升。）");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_VORPAL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smite_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "破邪斩");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得屠杀邪恶的能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_SLAY_EVIL, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血汲取");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时变得具有吸血属性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_VAMP, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mark_chaos_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混沌印记");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时获得混沌属性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_CHAOS, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _purple_stun_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫光震慑");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时能震慑怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_STUN, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mana_brand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力烙印");
        break;
    case SPELL_DESC:
        var_set_string(res, "你的近战武器将暂时从你的法力中获得额外力量。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
    {
        _inc_purple_timeout(OF_BRAND_MANA, 20 + randint1(20));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _troika_spells[2][MAX_TK_SPELL] =
{
    { /* Guntujant's spells */
    /* id cost fail spell (note that level is determined on learning) */
    { 1, 3, 25, heroism_spell},
    { 2, 1, 25, magic_missile_spell},
    { 3, 5, 30, _mana_burst_spell},
    { 4, 10, 30, _triple_attack_spell},
    { 5, 5, 35, light_area_spell},
    { 6, 5, 35, detect_monsters_spell},
    { 7, 10, 40, _berserk_spell},
    { 8, 10, 35, building_up_spell},
    { 9, 10, 40, wonder_spell},
    { 10, 10, 40, _rock_smash_spell},
    { 11, 13, 45, _plasma_bolt_spell},
    { 12, 13, 45, _chaos_bolt_spell},
    { 13, 25, 50, nether_ball_spell},
    { 14, 30, 50, _wave_of_death_spell},
    { 15, 50, 55, _purple_vamp_spell},
    { 16, 45, 55, drain_mana_spell},
    { 17, 40, 60, _slaughter_spell},
    { 18, 50, 60, _wall_spell},
    { 19, 60, 65, _nether_storm_spell},
    { 20, 60, 65, _mana_storm_spell}

    },

    { /* Uxip's spells */
    { 1, 5, 25, _slay_orc_spell},
    { 2, 10, 25, _slay_animal_spell},
    { 3, 10, 25, _slay_troll_spell},
    { 4, 20, 35, _slay_human_spell},
    { 5, 20, 40, _brand_fire_spell},
    { 6, 20, 40, _brand_cold_spell},
    { 7, 25, 45, _brand_acid_spell},
    { 8, 25, 45, _brand_elec_spell},
    { 9, 35, 55, _slay_dragon_spell},
    { 10, 25, 50, _slay_giant_spell},
    { 11, 10, 55, _long_attack_spell},
    { 12, 20, 55, _rush_attack_spell},
    { 13, 55, 60, _slay_demon_spell},
    { 14, 55, 60, _slay_undead_spell},
    { 15, 55, 65, _purple_sharp_spell},
    { 16, 55, 65, _smite_evil_spell},
    { 17, 75, 65, _vampirism_spell},
    { 18, 75, 65, _mark_chaos_spell},
    { 19, 75, 65, _purple_stun_spell},
    { 20, 50, 65, _mana_brand_spell}
    }
};

void troika_bonus_flags(object_type *o_ptr, u32b flgs[OF_ARRAY_SIZE])
{
    int i;
    if ((!o_ptr) || (!o_ptr->k_idx)) return; /* paranoia */
    if (o_ptr->loc.where != INV_EQUIP) return;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if ((_slay_timeouts[i] > 0) && (!have_flag(flgs, _slay_flags[i]))) add_flag(flgs, _slay_flags[i]);
    }
}

static int _troika_get_spells_learned(spell_info* spells, int alku, int kumpi)
{
    int ct = 0, i;
    bool dummy = (!spells) ? TRUE : FALSE;
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        if (_my_spell_taso[kumpi][i] == 0) continue;
        if (dummy)
        {
            ct++;
            continue;
        }
        else
        {
            spell_info *src, *dest;
            src = &_troika_spells[kumpi][i];
            dest = &spells[alku + (ct++)];
            dest->level = _my_spell_taso[kumpi][i];
            dest->cost = src->cost;
            dest->fail = src->fail;
            dest->fn = src->fn;
        }
    }
    return ct;
}

void troika_learn_spell(monster_race *r_ptr)
{
    bool unique = (r_ptr->flags1 & RF1_UNIQUE) ? TRUE : FALSE;
    byte kumpi, opittu = 0, uusi;
    int taso = MAX(10, MIN(r_ptr->level, p_ptr->max_plv * 2));
    s32b imp = MIN(400, (r_ptr->hdice * r_ptr->hside) * 100L / (taso * taso));
    if (taso < p_ptr->max_plv + 4) return;
    if (randint1(555) > imp) return;
    kumpi = randint0(2);
    opittu = _troika_get_spells_learned(NULL, 0, kumpi);
    if (opittu >= (MAX_TK_SPELL / 2)) return;
    if ((opittu > 3) && (taso < p_ptr->max_plv * 4 / 3)) return;
    if (randint1(MAX(6, r_ptr->level)) < MIN(p_ptr->max_plv, opittu * 5 + 5)) return;
    if ((!unique) && (randint1(847 + (242 * opittu)) > imp)) return;
    if (r_ptr->level < (opittu * 8) + 8) return;
    else
    {
        uusi = (opittu * 2) + randint0(2);
        _my_spell_taso[kumpi][uusi] = p_ptr->max_plv;
        msg_format("%s传授你<color:v>%s</color>法术。", _troika_names[kumpi], get_spell_name(_troika_spells[kumpi][uusi].fn));
        msg_print(NULL);
    }
}

static void _dump_quests(doc_ptr doc)
{
    int i;
    doc_insert(doc, "<topic:Purple Quests>================================ <color:keypress>P</color> 紫色任务 ================================\n\n");
    for (i = 0; i < _q_idx; i++)
    {
        int vari = TERM_L_GREEN;
        int day = 0, hour = 0, min = 0;
        monster_race *r_ptr = &r_info[_tk_quests[i].goal_idx];
        if (!r_ptr) continue; /* paranoia */
        if ((_tk_quests[i].start_lev) && (!_tk_quests[i].completed_lev)) vari = TERM_YELLOW;
        else if (_tk_quests[i].killed < _tk_quests[i].goal_ct) vari = TERM_RED;
        if (_tk_quests[i].goal_ct > 1)
        {
            char name[MAX_NLEN];
            strcpy(name, r_name + r_ptr->name);
            plural_aux(name);
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s，等级 %d - 击杀 %d 只 %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_tk_quests[i].dungeon].name, _tk_quests[i].level, _tk_quests[i].goal_ct, name);
        }
        else
            doc_printf(doc, "%2d) <indent><style:indent><color:%c>%s，等级 %d - 击杀 %s\n", i + 1, attr_to_attr_char(vari), d_name + d_info[_tk_quests[i].dungeon].name, _tk_quests[i].level, r_name + r_ptr->name);
        switch (vari)
        {
            case TERM_YELLOW:
                doc_printf(doc, "进行中\n");
                break;
            case TERM_RED:
                extract_day_hour_min_imp(_tk_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "失败：第 %d 天，%d:%02d，玩家等级(CL) %d", day, hour, min, _tk_quests[i].completed_lev);
                if (_tk_quests[i].goal_ct > 1) doc_printf(doc, "（已击杀 %d）", _tk_quests[i].killed);
                doc_printf(doc, "\n");
                break;
            default:
                extract_day_hour_min_imp(_tk_quests[i].completed_turn, &day, &hour, &min);
                doc_printf(doc, "完成：第 %d 天，%d:%02d，玩家等级(CL) %d\n", day, hour, min, _tk_quests[i].completed_lev);
                break;
        }
        doc_printf(doc, "</color></style></indent>\n");
    }
}

static void _calc_bonuses(void)
{
    int hand;
    if (_sohoglyth_reward_level > 0) p_ptr->telepathy = TRUE;
    if (_sohoglyth_reward_level > 1) p_ptr->munchkin_pseudo_id = TRUE;
    if (_sohoglyth_reward_level > 2)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->levitation = TRUE;
    }
    for (hand = 0; hand < MAX_HANDS; hand++)
    {
        if (p_ptr->weapon_info[hand].wield_how == WIELD_TWO_HANDS)
        {
            p_ptr->weapon_info[hand].to_d += (p_ptr->lev / 3) + (_sohoglyth_reward_level * 3);
            p_ptr->weapon_info[hand].dis_to_d += (p_ptr->lev / 3) + (_sohoglyth_reward_level * 3);
        }
    }
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (_sohoglyth_reward_level > 0) add_flag(flgs, OF_TELEPATHY);
    if (_sohoglyth_reward_level > 2) add_flag(flgs, OF_LEVITATION);
}

static void _troika_load(savefile_ptr file)
{
    int i;
    _troika_ini_quests();
    _troika_ini_spells();
    troika_wipe_timeouts();
    _sohoglyth_reward_level = savefile_read_byte(file);
    _q_idx = savefile_read_byte(file);
    for (i = 0; i < _q_idx; i++)
    {
        int j = MIN(i, MAX_TK_QUEST - 1); /* paranoia */
        _tk_quests[j].goal_idx = savefile_read_u16b(file);
        _tk_quests[j].dungeon = savefile_read_u16b(file);
        _tk_quests[j].level = savefile_read_u16b(file);
        _tk_quests[j].danger_level = savefile_read_u16b(file);
        _tk_quests[j].goal_ct = savefile_read_byte(file);
        _tk_quests[j].killed = savefile_read_byte(file);
        _tk_quests[j].start_lev = savefile_read_byte(file);
        _tk_quests[j].completed_lev = savefile_read_byte(file);
        _tk_quests[j].completed_turn = savefile_read_u32b(file);
    }
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        _slay_timeouts[i] = savefile_read_byte(file);
    }
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        _my_spell_taso[0][i] = savefile_read_byte(file);
        _my_spell_taso[1][i] = savefile_read_byte(file);
    }
}

static void _troika_save(savefile_ptr file)
{
    int i;
    savefile_write_byte(file, _sohoglyth_reward_level);
    savefile_write_byte(file, _q_idx);
    for (i = 0; i < _q_idx; i++)
    {
        savefile_write_u16b(file, _tk_quests[i].goal_idx);
        savefile_write_u16b(file, _tk_quests[i].dungeon);
        savefile_write_u16b(file, _tk_quests[i].level);
        savefile_write_u16b(file, _tk_quests[i].danger_level);
        savefile_write_byte(file, _tk_quests[i].goal_ct);
        savefile_write_byte(file, _tk_quests[i].killed);
        savefile_write_byte(file, _tk_quests[i].start_lev);
        savefile_write_byte(file, _tk_quests[i].completed_lev);
        savefile_write_u32b(file, _tk_quests[i].completed_turn);
    }
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        savefile_write_byte(file, _slay_timeouts[i]);
    }
    for (i = 0; i < MAX_TK_SPELL; i++)
    {
        savefile_write_byte(file, _my_spell_taso[0][i]);
        savefile_write_byte(file, _my_spell_taso[1][i]);
    }
}

static spell_info *_get_spells(void)
{
    static spell_info spells[MAX_SPELLS];
    int laskuri = _troika_get_spells_learned(spells, 0, 0);
    troika_spell_hack = laskuri;
    laskuri += _troika_get_spells_learned(spells, laskuri, 1);
    spells[laskuri].fn = NULL;
    return spells;
}

static void _troika_dump(doc_ptr doc)
{
    _dump_quests(doc);
    py_dump_spells(doc);
}

bool troika_dispel_timeouts(void)
{
    bool tulos = FALSE;
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if ((_slay_timeouts[i] > 0) && (one_in_(2)))
        {
            _inc_purple_timeout(_slay_flags[i], -500);
            tulos = TRUE;
        }
    }
    return tulos;
}

void troika_reduce_timeouts(void)
{
    int i;
    for (i = 0; i < MAX_TK_TIMEOUT; i++)
    {
        if (_slay_timeouts[i] > 0) _inc_purple_timeout(_slay_flags[i], -1);
    }
}

bool troika_allow_equip_item(object_type *o_ptr)
{
    u32b flgs[OF_ARRAY_SIZE];
    if ((!o_ptr) || (!o_ptr->k_idx)) return FALSE;
    if (!disciple_is_(DISCIPLE_TROIKA)) return TRUE; /* paranoia */
    if (o_ptr->tval == TV_SHIELD)
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("一个遥远的声音咆哮道：");
            cmsg_print(TERM_VIOLET, "“装备盾牌是对乌克西普的亵渎！”");
        }
        else msg_print("盾牌是对乌克西普的亵渎！");
        viesti = TRUE;
        return FALSE;
    }
    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_NO_MAGIC))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("乌克西普的声音咆哮道：");
            cmsg_print(TERM_VIOLET, "“你试图压制我们赐予你的魔法？你应该拥抱它，并心存感激。”");
        }
        else msg_print("反魔法物品是对乌克西普的亵渎！");
        obj_learn_flag(o_ptr, OF_NO_MAGIC);
        viesti = TRUE;
        return FALSE;
    }
    if ((object_is_helmet(o_ptr)) && (o_ptr->name2 == EGO_ARMOR_SEEING) && (p_ptr->max_plv < 50))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("乌克西普的声音咆哮道：");
            cmsg_print(TERM_VIOLET, "“你不配拥有如此强大的物品！”");
        }
        else msg_print("乌克西普认为只有 50 级的角色才配得上全知头盔。你戴上它是一种亵渎。");
        viesti = TRUE;
        return FALSE;
    }
    return TRUE;
}

bool troika_allow_use_device(object_type *o_ptr)
{
    if ((!o_ptr) || (!o_ptr->k_idx) || (!object_is_device(o_ptr))) return FALSE;
    if (!disciple_is_(DISCIPLE_TROIKA)) return TRUE; /* paranoia */
    if (!o_ptr->activation.type) return TRUE;
    if ((o_ptr->tval == TV_WAND) && (o_ptr->activation.type == EFFECT_DRAIN_LIFE) && (p_ptr->max_plv < 40))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("一个遥远的声音咆哮道：");
            cmsg_print(TERM_VIOLET, "“如此弱者竟使用这样的魔杖，这是对乌克西普的亵渎！”");
        }
        else
            msg_print("在达到 40 级之前，你做任何耍酷的事都是对乌克西普的亵渎。");
        viesti = TRUE;
        return FALSE;
    }
    if ((o_ptr->tval == TV_ROD) && (o_ptr->activation.type == EFFECT_DETECT_ALL) && (p_ptr->max_plv < 40))
    {
        static bool viesti = FALSE;
        if (!viesti)
        {
            msg_print("乌克西普的声音咆哮道：");
            cmsg_print(TERM_VIOLET, "“如此弱者竟使用这样的魔棒，这是对乌克西普的亵渎！”");
        }
        else
            msg_print("在达到 40 级之前，你使用如此超模的物品是对乌克西普的亵渎。");
        viesti = TRUE;
        return FALSE;
    }
    return TRUE;
}

class_t *troika_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  17,  30,   3,  16,  20,  64,  42};
    skills_t xs = { 12,   7,   8,   0,   0,   0,  20,  15};

        me.name = "特罗伊卡";
 me.subdesc = "The Troika is a group of three closely related Purples working together - "
 "Guntujant, Uxip and Sohoglyth. Guntujant is a capricious Purple who follows "
 "the careers of his servants very closely, and rewards and punishes them "
 "seemingly at random; the powers gained by his followers are unpredictable, "
 "but generally quite strong. Uxip delights in combat, and grants Troika "
 "disciples their prodigious endurance and skill with two-handed weapons; "
 "however, a number of common items are an abomination unto Uxip, and cannot "
 "be used by followers of the Troika. Sohoglyth is a mysterious spirit, but "
 "a profoundly powerful one. She occasionally sets bonus quests for Troika "
 "disciples; if enough of the quests are completed with no failures, she will "
 "reward the questor with the power of telepathy, remote object quality "
 "sensing, or even walking through walls.\n\n"
 "Troika Disciples are in general very strong, especially in combat but also "
 "sorcery, yet the limitations placed upon them by Uxip and Guntujant also make "
 "them in some ways vulnerable. In particular, they frequently suffer random "
 "punishments, risk being punished if they teleport from a fight while not at low health, "
 "cannot wear a shield or cure poison with mushrooms, and must earn the right to "
 "use a rod of Detection. The spells of Troika disciples are unique in that they "
 "are not learned at any particular character level; instead, Guntujant and Uxip "
 "will teach them as a reward for killing dangerous enemies, especially uniques. "
 "Strength determines the spellcasting ability of a Troika disciple.";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  0;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  1;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  0;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.base_hp = 23;
        me.exp = 150;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.gain_level = _gain_level;
        me.character_dump = _troika_dump;
        me.get_spells_fn = _get_spells;
        me.load_player = _troika_load;
        me.save_player = _troika_save;
        init = TRUE;
    }
    me.life = 100 + (p_ptr->lev / 4);

    return &me;
}

