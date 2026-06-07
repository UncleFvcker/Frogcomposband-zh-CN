#include "angband.h"
#include "equip.h"

#define REW_POLY_SLF    1
#define REW_GAIN_EXP    2
#define REW_LOSE_EXP    3
#define REW_GOOD_OBJ    4
#define REW_GREA_OBJ    5
#define REW_CHAOS_WP    6
#define REW_GOOD_OBS    7
#define REW_GREA_OBS    8
#define REW_TY_CURSE    9
#define REW_SUMMON_M    10
#define REW_H_SUMMON    11
#define REW_DO_HAVOC    12
#define REW_GAIN_ABL    13
#define REW_LOSE_ABL    14
#define REW_RUIN_ABL    15
#define REW_AUGM_ABL    16
#define REW_POLY_WND    17
#define REW_HEAL_FUL    18
#define REW_HURT_LOT    19
#define REW_CURSE_WP    20
#define REW_CURSE_AR    21
#define REW_PISS_OFF    22
#define REW_WRATH       23
#define REW_DESTRUCT    24
#define REW_GENOCIDE    25
#define REW_MASS_GEN    26
#define REW_DISPEL_C    27
#define REW_UNUSED_1    28
#define REW_UNUSED_2    29
#define REW_UNUSED_3    30
#define REW_UNUSED_4    31
#define REW_UNUSED_5    32
#define REW_IGNORE      33
#define REW_SER_UNDE    34
#define REW_SER_DEMO    35
#define REW_SER_MONS    36

int chaos_stats[MAX_CHAOS_PATRON] =
{
    A_CON,  /* Slortar */
    A_CON,  /* Mabelode */
    A_STR,  /* Chardros */
    A_STR,  /* Hionhurn */
    A_STR,  /* Xiombarg */

    A_INT,  /* Pyaray */
    A_STR,  /* Balaan */
    A_INT,  /* Arioch */
    A_CON,  /* Eequor */
    A_CHR,  /* Narjhan */

    -1,     /* Balo */
    A_STR,  /* Khorne */
    A_CHR,  /* Slaanesh */
    A_CON,  /* Nurgle */
    A_INT,  /* Tzeentch */

    A_STR,  /* Khaine */
};

int chaos_rewards[MAX_CHAOS_PATRON][20] =
{
    /* Slortar the Old: */
    {
        REW_WRATH, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL, REW_LOSE_ABL,
        REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_POLY_WND, REW_POLY_SLF,
        REW_POLY_SLF, REW_POLY_SLF, REW_GAIN_ABL, REW_GAIN_ABL, REW_GAIN_EXP,
        REW_GOOD_OBJ, REW_CHAOS_WP, REW_GREA_OBJ, REW_AUGM_ABL, REW_AUGM_ABL
    },

    /* Mabelode the Faceless: */
    {
        REW_WRATH, REW_CURSE_WP, REW_CURSE_AR, REW_H_SUMMON, REW_SUMMON_M,
        REW_SUMMON_M, REW_IGNORE, REW_IGNORE, REW_POLY_WND, REW_POLY_WND,
        REW_POLY_SLF, REW_HEAL_FUL, REW_HEAL_FUL, REW_GAIN_ABL, REW_SER_UNDE,
        REW_CHAOS_WP, REW_GOOD_OBJ, REW_GOOD_OBJ, REW_GOOD_OBS, REW_GOOD_OBS
    },

    /* Chardros the Reaper: */
    {
        REW_WRATH, REW_WRATH, REW_HURT_LOT, REW_PISS_OFF, REW_H_SUMMON,
        REW_SUMMON_M, REW_IGNORE, REW_IGNORE, REW_DESTRUCT, REW_SER_UNDE,
        REW_GENOCIDE, REW_MASS_GEN, REW_MASS_GEN, REW_DISPEL_C, REW_GOOD_OBJ,
        REW_CHAOS_WP, REW_GOOD_OBS, REW_GOOD_OBS, REW_AUGM_ABL, REW_AUGM_ABL
    },

    /* Hionhurn the Executioner: */
    {
        REW_WRATH, REW_WRATH, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL,
        REW_IGNORE, REW_IGNORE, REW_SER_UNDE, REW_DESTRUCT, REW_GENOCIDE,
        REW_MASS_GEN, REW_MASS_GEN, REW_HEAL_FUL, REW_GAIN_ABL, REW_GAIN_ABL,
        REW_CHAOS_WP, REW_GOOD_OBS, REW_GOOD_OBS, REW_AUGM_ABL, REW_AUGM_ABL
    },

    /* Xiombarg the Sword-Queen: */
    {
        REW_TY_CURSE, REW_TY_CURSE, REW_PISS_OFF, REW_RUIN_ABL, REW_LOSE_ABL,
        REW_IGNORE, REW_POLY_SLF, REW_POLY_SLF, REW_POLY_WND, REW_POLY_WND,
        REW_GENOCIDE, REW_DISPEL_C, REW_GOOD_OBJ, REW_GOOD_OBJ, REW_SER_MONS,
        REW_GAIN_ABL, REW_CHAOS_WP, REW_GAIN_EXP, REW_AUGM_ABL, REW_GOOD_OBS
    },


    /* Pyaray the Tentacled Whisperer of Impossible Secretes: */
    {
        REW_WRATH, REW_TY_CURSE, REW_PISS_OFF, REW_H_SUMMON, REW_H_SUMMON,
        REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_POLY_WND, REW_POLY_SLF,
        REW_POLY_SLF, REW_SER_DEMO, REW_HEAL_FUL, REW_GAIN_ABL, REW_GAIN_ABL,
        REW_CHAOS_WP, REW_DO_HAVOC, REW_GOOD_OBJ, REW_GREA_OBJ, REW_GREA_OBS
    },

    /* Balaan the Grim: */
    {
        REW_TY_CURSE, REW_HURT_LOT, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL,
        REW_SUMMON_M, REW_LOSE_EXP, REW_POLY_SLF, REW_POLY_SLF, REW_POLY_WND,
        REW_SER_UNDE, REW_HEAL_FUL, REW_HEAL_FUL, REW_GAIN_EXP, REW_GAIN_EXP,
        REW_CHAOS_WP, REW_GOOD_OBJ, REW_GOOD_OBS, REW_GREA_OBS, REW_AUGM_ABL
    },

    /* Arioch, Duke of Hell: */
    {
        REW_WRATH, REW_PISS_OFF, REW_RUIN_ABL, REW_LOSE_EXP, REW_H_SUMMON,
        REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_POLY_SLF,
        REW_POLY_SLF, REW_MASS_GEN, REW_SER_DEMO, REW_HEAL_FUL, REW_CHAOS_WP,
        REW_CHAOS_WP, REW_GOOD_OBJ, REW_GAIN_EXP, REW_GREA_OBJ, REW_AUGM_ABL
    },

    /* Eequor, Blue Lady of Dismay: */
    {
        REW_WRATH, REW_TY_CURSE, REW_PISS_OFF, REW_CURSE_WP, REW_RUIN_ABL,
        REW_IGNORE, REW_IGNORE, REW_POLY_SLF, REW_POLY_SLF, REW_POLY_WND,
        REW_GOOD_OBJ, REW_GOOD_OBJ, REW_SER_MONS, REW_HEAL_FUL, REW_GAIN_EXP,
        REW_GAIN_ABL, REW_CHAOS_WP, REW_GOOD_OBS, REW_GREA_OBJ, REW_AUGM_ABL
    },

    /* Narjhan, Lord of Beggars: */
    {
        REW_WRATH, REW_CURSE_AR, REW_CURSE_WP, REW_CURSE_WP, REW_CURSE_AR,
        REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_POLY_SLF, REW_POLY_SLF,
        REW_POLY_WND, REW_HEAL_FUL, REW_HEAL_FUL, REW_GAIN_EXP, REW_AUGM_ABL,
        REW_GOOD_OBJ, REW_GOOD_OBJ, REW_CHAOS_WP, REW_GREA_OBJ, REW_GREA_OBS
    },

    /* Balo the Jester: */
    {
        REW_WRATH, REW_SER_DEMO, REW_CURSE_WP, REW_CURSE_AR, REW_LOSE_EXP,
        REW_GAIN_ABL, REW_LOSE_ABL, REW_POLY_WND, REW_POLY_SLF, REW_IGNORE,
        REW_DESTRUCT, REW_MASS_GEN, REW_CHAOS_WP, REW_GREA_OBJ, REW_HURT_LOT,
        REW_AUGM_ABL, REW_RUIN_ABL, REW_H_SUMMON, REW_GREA_OBS, REW_AUGM_ABL
    },

    /* Khorne the Bloodgod: */
    {
        REW_WRATH, REW_HURT_LOT, REW_HURT_LOT, REW_H_SUMMON, REW_H_SUMMON,
        REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_SER_MONS, REW_SER_DEMO,
        REW_POLY_SLF, REW_POLY_WND, REW_HEAL_FUL, REW_GOOD_OBJ, REW_GOOD_OBJ,
        REW_CHAOS_WP, REW_GOOD_OBS, REW_GOOD_OBS, REW_GREA_OBJ, REW_GREA_OBS
    },

    /* Slaanesh: */
    {
        REW_WRATH, REW_PISS_OFF, REW_PISS_OFF, REW_RUIN_ABL, REW_LOSE_ABL,
        REW_LOSE_EXP, REW_IGNORE, REW_IGNORE, REW_POLY_WND, REW_SER_DEMO,
        REW_POLY_SLF, REW_HEAL_FUL, REW_HEAL_FUL, REW_GOOD_OBJ, REW_GAIN_EXP,
        REW_GAIN_EXP, REW_CHAOS_WP, REW_GAIN_ABL, REW_GREA_OBJ, REW_AUGM_ABL
    },

    /* Nurgle: */
    {
        REW_WRATH, REW_PISS_OFF, REW_HURT_LOT, REW_RUIN_ABL, REW_LOSE_ABL,
        REW_LOSE_EXP, REW_IGNORE, REW_IGNORE, REW_IGNORE, REW_POLY_SLF,
        REW_POLY_SLF, REW_POLY_WND, REW_HEAL_FUL, REW_GOOD_OBJ, REW_GAIN_ABL,
        REW_GAIN_ABL, REW_SER_UNDE, REW_CHAOS_WP, REW_GREA_OBJ, REW_AUGM_ABL
    },

    /* Tzeentch: */
    {
        REW_WRATH, REW_CURSE_WP, REW_CURSE_AR, REW_RUIN_ABL, REW_LOSE_ABL,
        REW_LOSE_EXP, REW_IGNORE, REW_POLY_SLF, REW_POLY_SLF, REW_POLY_SLF,
        REW_POLY_SLF, REW_POLY_WND, REW_HEAL_FUL, REW_CHAOS_WP, REW_GREA_OBJ,
        REW_GAIN_ABL, REW_GAIN_ABL, REW_GAIN_EXP, REW_GAIN_EXP, REW_AUGM_ABL
    },

    /* Khaine: */
    {
        REW_WRATH, REW_HURT_LOT, REW_PISS_OFF, REW_LOSE_ABL, REW_LOSE_EXP,
        REW_IGNORE,   REW_IGNORE,   REW_DISPEL_C, REW_DO_HAVOC, REW_DO_HAVOC,
        REW_POLY_SLF, REW_POLY_SLF, REW_GAIN_EXP, REW_GAIN_ABL, REW_GAIN_ABL,
        REW_SER_MONS, REW_GOOD_OBJ, REW_CHAOS_WP, REW_GREA_OBJ, REW_GOOD_OBS
    }
};

/* Bad things, but not ty_curse bad (or at least not ty_curse insta-deadly) */
void nonlethal_ty_substitute(bool do_dec)
{
    bool old_nos = no_scrambling;
    no_scrambling = TRUE;
    mutate_player();
    no_scrambling = old_nos;
    if (do_dec) dec_stat(randint0(MAX_STATS), 12 + randint1(6), TRUE);
    if (!(p_ptr->cursed & OFC_BY_CURSE))
    {
        if (randint0(p_ptr->max_plv + 40) > 45)
        {
            if ((one_in_(3)) && (randint0(50) < p_ptr->max_plv))
            {
                curse_equipment(100, 25);
            }
            else if ((one_in_(2)) || (randint0(50) >= p_ptr->max_plv))
            {
                curse_equipment(50, 0);
            }
            else /* The BFC sometimes puts itself on equipment */
            {
                int slot = equip_random_slot(object_is_art_or_ego);
                if (slot)
                {
                    object_type *o_ptr = equip_obj(slot);
                    u32b oflgs[OF_ARRAY_SIZE];
                    char o_name[MAX_NLEN];
                    if ((!o_ptr) || (!o_ptr->k_idx)) return;
                    obj_flags(o_ptr, oflgs);
                    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

                    if (have_flag(oflgs, OF_BLESSED))
                    {
                        msg_format("你的%s抵抗了诅咒！", o_name);
                        return;
                    }
                    o_ptr->curse_flags |= OFC_HEAVY_CURSE;
                    o_ptr->curse_flags |= OFC_CURSED;
                    o_ptr->curse_flags |= OFC_BY_CURSE;
                    msg_format("有一股恶意的黑色光环包围着%s……", o_name);
                    o_ptr->feeling = FEEL_NONE;
                    p_ptr->update |= PU_BONUS;
                    p_ptr->window |= (PW_EQUIP | PW_INVEN);
                    p_ptr->redraw |= PR_EFFECTS;
                }
            }
        }
    }
}


void chaos_warrior_reward(void)
{
    if (p_ptr->is_dead) return;
    if (one_in_(6))
    {
        msg_format("%^s赐予了你一项变异作为奖赏！",
            chaos_patrons[p_ptr->chaos_patron]);

        mut_gain_random(NULL);
    }
    else
    {
        char        wrath_reason[32] = "";
        int         nasty_chance = 6;
        int         dummy = 0, dummy2 = 0;
        int         type, effect;
        int         count = 0;

        if (p_ptr->lev == 13) nasty_chance = 2;
        else if (!(p_ptr->lev % 13)) nasty_chance = 3;
        else if (!(p_ptr->lev % 14)) nasty_chance = 12;

        if (one_in_(nasty_chance))
            type = randint1(20); /* Allow the 'nasty' effects */
        else
            type = randint1(15) + 5; /* Or disallow them */

        if (type < 1) type = 1;
        if (type > 20) type = 20;
        type--;

        sprintf(wrath_reason, "the Wrath of %s",
            chaos_patrons[p_ptr->chaos_patron]);

        /* Extra chance to avoid the worst stuff */
        if ((type < 5) && (p_ptr->lev % 13))
        {
            while (type < 5) 
            {
                if (!one_in_(type + ((p_ptr->lev < 13) ? 1 : 2))) break;
                type++;
            }
        }

        effect = chaos_rewards[p_ptr->chaos_patron][type];
        switch (effect)
        {
        case REW_POLY_SLF:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“凡人，你需要一个新的形态！”");

            do_poly_self();
            break;
        case REW_GAIN_EXP:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“干得好，凡人！继续前进！”");
            if (p_ptr->prace == RACE_ANDROID)
                msg_print("但是，什么也没有发生。");
            else if (p_ptr->exp < PY_MAX_EXP)
            {
                s32b ee = (p_ptr->exp / 2) + 10;
                if (ee > 100000L) ee = 100000L;
                msg_print("你感觉更有经验了。");
                gain_exp(ee);
            }
            break;
        case REW_LOSE_EXP:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“奴隶，你不配得到那个。”");

            if (p_ptr->prace == RACE_ANDROID)
                msg_print("但是，什么也没有发生。");
            else
            {
                lose_exp(p_ptr->exp / 6);
            }
            break;
        case REW_GOOD_OBJ:
            msg_format("%s的声音低语道：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“明智地使用我的礼物。”");
            acquirement(py, px, 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBJ:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“明智地使用我的礼物。”");

            acquirement(py, px, 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_CHAOS_WP:
        {
            object_type forge;

            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“你的事迹为你赢得了一把利刃。”");

            dummy = TV_SWORD;
            switch (randint1(p_ptr->lev))
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
                default:
                    dummy2 = SV_BLADE_OF_CHAOS;
            }

            object_prep(&forge, lookup_kind(dummy, dummy2));
            forge.to_h = 3 + randint1(dun_level) % 10;
            forge.to_d = 3 + randint1(dun_level) % 10;
            one_resistance(&forge);
            object_origins(&forge, ORIGIN_PATRON);
            forge.name2 = EGO_WEAPON_CHAOS;

            drop_near(&forge, -1, py, px);
            break;
        }
        case REW_GOOD_OBS:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“你的事迹为你赢得了应得的奖赏。”");

            acquirement(py, px, randint1(2) + 1, FALSE, FALSE, ORIGIN_PATRON);
            break;
        case REW_GREA_OBS:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“看吧，凡人，我多么慷慨地奖赏你的忠诚。”");

            acquirement(py, px, randint1(2) + 1, TRUE, FALSE, ORIGIN_PATRON);
            break;
        case REW_TY_CURSE:
            msg_format("%s的声音如雷鸣般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“凡人，你变得越来越傲慢了。”");

            if (p_ptr->lev < 35) nonlethal_ty_substitute(TRUE);
            else activate_ty_curse(FALSE, &count);
            break;
        case REW_SUMMON_M:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“我的宠物们，摧毁这个傲慢的凡人！”");
            for (dummy = 0; dummy < randint1(5) + 1; dummy++)
                summon_specific(0, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
            break;
        case REW_H_SUMMON:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“你需要更有价值的对手！”");
            activate_hi_summon(py, px, FALSE);
            break;
        case REW_DO_HAVOC:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“死亡与毁灭！这正合我意！”");
            call_chaos(100);
            break;
        case REW_GAIN_ABL:
            msg_format("%s的声音回荡着：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“留下来，凡人，让我来塑造你。”");
            if (one_in_(3) && !(chaos_stats[p_ptr->chaos_patron] < 0))
                do_inc_stat(chaos_stats[p_ptr->chaos_patron]);
            else
                do_inc_stat(randint0(6));
            break;
        case REW_LOSE_ABL:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“凡人，我对你感到厌倦了。”");

            if (one_in_(3) && !(chaos_stats[p_ptr->chaos_patron] < 0))
                do_dec_stat(chaos_stats[p_ptr->chaos_patron]);
            else
                do_dec_stat(randint0(6));
            break;
        case REW_RUIN_ABL:
            msg_format("%s的声音如雷鸣般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“凡人，你需要学学什么是谦卑！”");
            msg_print("你感觉力量流失了！");

            for (dummy = 0; dummy < 6; dummy++)
                dec_stat(dummy, 10 + randint1(15), TRUE);
            break;
        case REW_POLY_WND:
            msg_format("你感觉到%s的力量触碰了你。", chaos_patrons[p_ptr->chaos_patron]);
            do_poly_wounds();
            break;
        case REW_AUGM_ABL:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“收下我这份微薄的礼物吧！”");
            for (dummy = 0; dummy < 6; dummy++)
                do_inc_stat(dummy);
            break;
        case REW_HURT_LOT:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“受苦吧，可悲的蠢货！”");
            fire_ball(GF_DISINTEGRATE, 0, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), 4);
            take_hit(DAMAGE_NOESCAPE, MIN(p_ptr->lev * 4, p_ptr->mhp * 2 / 5), wrath_reason);
            break;
       case REW_HEAL_FUL:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“站起来，我的仆人！”");
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
            if (prace_is_(RACE_MON_SWORD) || prace_is_(RACE_MON_RING) || prace_is_(RACE_MON_ARMOR))
            {
                msg_format("%s的声音如洪钟般响起：",
                    chaos_patrons[p_ptr->chaos_patron]);
                msg_print("“现在你要为惹怒我付出代价。”");
                nonlethal_ty_substitute(TRUE);
                break;
            }
            if (slot)
            {
                msg_format("%s的声音如洪钟般响起：",
                    chaos_patrons[p_ptr->chaos_patron]);
                msg_print("“你太依赖你的武器了。”");
                curse_weapon(FALSE, slot);
            }
            break;
        }
        case REW_CURSE_AR:
        {
            int slot = equip_random_slot(object_is_armour);
            if (prace_is_(RACE_MON_SWORD) || prace_is_(RACE_MON_RING) || prace_is_(RACE_MON_ARMOR))
            {
                msg_format("%s的声音如洪钟般响起：",
                    chaos_patrons[p_ptr->chaos_patron]);
                msg_print("“现在你要为惹怒我付出代价。”");
                nonlethal_ty_substitute(TRUE);
                break;
            }
            if (slot)
            {
                msg_format("%s的声音如洪钟般响起：",
                    chaos_patrons[p_ptr->chaos_patron]);
                msg_print("“你太依赖你的装备了。”");
                curse_armor(slot);
            }
            break;
        }
        case REW_PISS_OFF:
            msg_format("%s的声音低语道：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“现在你要为惹怒我付出代价。”");
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
                    if (prace_is_(RACE_MON_SWORD) || prace_is_(RACE_MON_RING) || prace_is_(RACE_MON_ARMOR))
                    {
                        nonlethal_ty_substitute(TRUE);
                    }
                    else if (one_in_(2))
                    {
                        int slot = equip_random_slot(object_is_melee_weapon);
                        if (slot)
                            curse_weapon(FALSE, slot);
                    }
                    else
                    {
                        int slot = equip_random_slot(object_is_armour);
                        if (slot)
                            curse_armor(slot);
                    }
                    break;
                default:
                    for (dummy = 0; dummy < 6; dummy++)
                        dec_stat(dummy, 10 + randint1(15), TRUE);
                    break;
            }
            break;
        case REW_WRATH:
            msg_format("%s的声音如雷鸣般响起：", chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“去死吧，凡人！”");

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
        case REW_DESTRUCT:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“死亡与毁灭！这正合我意！”");
            destroy_area(py, px, 25, 3 * p_ptr->lev);
            break;
        case REW_GENOCIDE:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“让我为你清除这些压迫者吧！”");
            symbol_genocide(0, FALSE);
            break;
        case REW_MASS_GEN:
            msg_format("%s的声音如洪钟般响起：",
                chaos_patrons[p_ptr->chaos_patron]);
            msg_print("“让我为你清除这些压迫者吧！”");
            mass_genocide(0, FALSE);
            break;
        case REW_DISPEL_C:
            msg_format("你能感觉到%s的力量袭击了你的敌人！",
                chaos_patrons[p_ptr->chaos_patron]);
            dispel_monsters(p_ptr->lev * 4);
            break;
        case REW_IGNORE:
            msg_format("%s无视了你。",
                chaos_patrons[p_ptr->chaos_patron]);
            break;
        case REW_SER_DEMO:
            msg_format("%s赐予了你一个恶魔仆从作为奖赏！",chaos_patrons[p_ptr->chaos_patron]);
            if (!summon_specific(-1, py, px, dun_level, SUMMON_DEMON, PM_FORCE_PET))
                msg_print("没有任何人出现……");
            break;
        case REW_SER_MONS:
            msg_format("%s赐予了你一个仆从作为奖赏！",chaos_patrons[p_ptr->chaos_patron]);
            if (!summon_specific(-1, py, px, dun_level, 0, PM_FORCE_PET))
                msg_print("没有任何人出现……");
            break;
        case REW_SER_UNDE:
            msg_format("%s赐予了你一个不死仆从作为奖赏！",chaos_patrons[p_ptr->chaos_patron]);
            if (!summon_specific(-1, py, px, dun_level, SUMMON_UNDEAD, PM_FORCE_PET))
                msg_print("没有任何人出现……");
            break;
        default:
            msg_format("%s的声音结结巴巴地说：", chaos_patrons[p_ptr->chaos_patron]);
            msg_format("“呃……呃……答案是%d/%d，问题是什么来着？”", type, effect);
        }
    }
}


static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 30) 
        res_add(RES_CHAOS);
    if (p_ptr->lev >= 40) 
        res_add(RES_FEAR);
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (p_ptr->lev >= 30)
        add_flag(flgs, OF_RES_CHAOS);
    if (p_ptr->lev >= 40)
        add_flag(flgs, OF_RES_FEAR);
}

static power_info _get_powers[] =
{
    { A_INT, {40, 50, 80, confusing_lights_spell}},
    { -1, {-1, -1, -1, NULL}}
};

static void _gain_level(int new_level)
{
    if ((new_level > 1) && (!mut_present(MUT_CHAOS_GIFT))) /* avoid double reward */
        chaos_warrior_reward();
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 450;
        me.encumbrance.weapon_pct = 20;
        me.encumbrance.enc_wgt = 1200;
        me.min_fail = 5;
        me.min_level = 2;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_BROAD_SWORD, 1);
    py_birth_obj_aux(TV_HARD_ARMOR, SV_METAL_SCALE_MAIL, 1);
    py_birth_spellbooks();
}

class_t *chaos_warrior_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 20,  25,  34,   1,  14,  12,  65,  40};
    skills_t xs = {  7,  11,  10,   0,   0,   0,  20,  17};

        me.name = "混沌战士";
        me.desc = "混沌战士是可怕的混沌魔王们令人畏惧的仆从。每个混沌战士都有一位庇护神，神明可能会在他们升级时赐予奖励；混沌战士可能会被治疗或被变形，其属性可能会增加，或者被赐予一把可怕的武器。但另一方面，他也可能会受到严厉的惩罚，或者干脆被神明无视；混沌魔王确实变幻莫测，不过谢天谢地，奖励通常比惩罚更常见。确切的奖励不取决于玩家做出的任何行为，而是完全取决于随机概率和对应的神明；每位神明都会给出不同的奖励和惩罚。\n \n混沌战士只能选择一个法术领域，混沌(Chaos)或恶魔(Daemon)；他们对其他形式的魔法不感兴趣。他们可以学会所选领域中的每一个法术。在40级时，他们会获得发射迷乱之光的强大能力，有可能击晕、惊吓和困惑视线内的每一个生物。";

        me.stats[A_STR] =  2;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  0;
        me.stats[A_CON] =  1;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 109;
        me.base_hp = 12;
        me.exp = 125;
        me.pets = 40;
        me.flags = CLASS_SENSE1_SLOW | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_powers = _get_powers;
        me.gain_level = _gain_level;
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
