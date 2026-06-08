#include "angband.h"

#include <assert.h>

static bool ang_sort_comp_pet(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    int w1 = who[a];
    int w2 = who[b];

    monster_type *m_ptr1 = &m_list[w1];
    monster_type *m_ptr2 = &m_list[w2];
    monster_race *r_ptr1 = &r_info[m_ptr1->r_idx];
    monster_race *r_ptr2 = &r_info[m_ptr2->r_idx];

    /* Unused */
    (void)v;

    if (m_ptr1->nickname && !m_ptr2->nickname) return TRUE;
    if (m_ptr2->nickname && !m_ptr1->nickname) return FALSE;

    if ((r_ptr1->flags1 & RF1_UNIQUE) && !(r_ptr2->flags1 & RF1_UNIQUE)) return TRUE;
    if ((r_ptr2->flags1 & RF1_UNIQUE) && !(r_ptr1->flags1 & RF1_UNIQUE)) return FALSE;

    if (r_ptr1->level > r_ptr2->level) return TRUE;
    if (r_ptr2->level > r_ptr1->level) return FALSE;

    if (m_ptr1->hp > m_ptr2->hp) return TRUE;
    if (m_ptr2->hp > m_ptr1->hp) return FALSE;

    return w1 <= w2;
}

bool class_uses_spell_scrolls(int mika)
{
    if (mika == CLASS_WARRIOR ||
      mika == CLASS_MINDCRAFTER ||
      mika == CLASS_PSION ||
      mika == CLASS_SORCERER ||
      mika == CLASS_ARCHER ||
      mika == CLASS_MAGIC_EATER ||
      mika == CLASS_DEVICEMASTER ||
      mika == CLASS_RED_MAGE ||
      mika == CLASS_SAMURAI ||
      mika == CLASS_CAVALRY ||
      mika == CLASS_BERSERKER ||
      mika == CLASS_WEAPONSMITH ||
      mika == CLASS_MIRROR_MASTER ||
      mika == CLASS_TIME_LORD ||
      mika == CLASS_BLOOD_KNIGHT ||
      mika == CLASS_WARLOCK ||
      mika == CLASS_ARCHAEOLOGIST ||
      mika == CLASS_DUELIST ||
      mika == CLASS_RUNE_KNIGHT ||
      mika == CLASS_WILD_TALENT ||
      mika == CLASS_BLUE_MAGE ||
      mika == CLASS_NINJA ||
      mika == CLASS_NINJA_LAWYER ||
      mika == CLASS_SCOUT ||
      mika == CLASS_MYSTIC ||
      mika == CLASS_MAULER ||
      mika == CLASS_POLITICIAN ||
      mika == CLASS_ALCHEMIST ||
      mika == CLASS_DISCIPLE ||
      mika == CLASS_SKILLMASTER )
        return FALSE;
    return TRUE;
}

/* Devices: We are following the do_spell() pattern which is quick and dirty,
   but not my preferred approach ... */

/* Fail Rates ... Scaled by 10 (95.2% returned as 952)
 * cf design/devices.ods */
int _difficulty(int d)
{
    /* A non-linear difficulty protects the high level end game
     * devices from inappropriate usage. -Rockets are now much
     * harder to use (and _Healing only marginally more difficult).
     * Formerly, OF_MAGIC_MASTERY was useless to device classes
     * like the mage. No longer! 100 -> 130, but with cubic weighting.
     * Two versions: The first is more punishing than the second, depending
     * on the chosen cutoff. As usual, see design/devices.ods or ^A"2.
    return d + 30 * d * d / 100 * d / 10000; */
    int cutoff = 40;
    int xtra   = 30;
    if (d > cutoff)
    {
        int l = d - cutoff;
        int m = 100 - cutoff;
        assert(cutoff < 100);
        return d + xtra * l * l / m * l / (m * m);
    }
    return d;
}
/* in progress: I find this calculation hard to grok ... let's
 * rephrase in terms of skill vs. difficulty and expose a simple
 * api so I can view fail(s,d) (a function of 2 variables).
 * cf ^A"2 for online spoiler tables (wizard1.c) */
int device_calc_fail_rate_aux(int skill, int difficulty)
{
    int min = USE_DEVICE;
    int fail = 0;
    difficulty = _difficulty(difficulty);
    if (skill > difficulty) difficulty -= (skill - difficulty)*2;
    else skill -= (difficulty - skill)*2;
    if (difficulty < min) difficulty = min;
    if (skill < min) skill = min;
    if (skill > difficulty)
        fail = difficulty * 500 / skill;
    else
        fail = 1000 - skill * 500 / difficulty;
    return fail;
}

/* XXX design is sloppy atm ... I want devicemasters to have a skill
 * boost with their speciality device type (e.g. wands), and less
 * magic skills overall. But the "effect" api layer has no info about
 * the obj->tval ... so we'll need to pass that along. The "effect" layer
 * is public and is also used for equipment with activations. */
static int effect_calc_fail_rate_aux(effect_t *effect, int skill_boost)
{
    int skill = p_ptr->skills.dev + skill_boost;
    int fail;

    if (p_ptr->pclass == CLASS_BERSERKER) return 1000;
    if (beorning_is_(BEORNING_FORM_BEAR)) return 1000;

    if (p_ptr->confused) skill = 3 * skill / 4;

    fail = device_calc_fail_rate_aux(skill, effect->difficulty);
    if ((p_ptr->stun) && (fail < 950))
    {
        fail += 500 * p_ptr->stun / 100;
        if (fail > 950) fail = 950;
    }
    return fail;
}

int effect_calc_fail_rate(effect_t *effect)
{
    return effect_calc_fail_rate_aux(effect, 0);
}

int device_calc_fail_rate(object_type *o_ptr)
{
    int lev, chance, fail;

    if (o_ptr->activation.type)
    {
        effect_t effect = o_ptr->activation;
        u32b     flgs[OF_ARRAY_SIZE];
        int      skill_boost = 0;

        if (devicemaster_is_speciality(o_ptr))
            skill_boost = 5 + 3*p_ptr->lev/5; /* 40+15 base = 115 -> 150 */

        if ((mut_present(MUT_IMPOTENCE)) && (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_ROD))
        {
            skill_boost -= 10;
            if (effect.type == EFFECT_SPEED || effect.type == EFFECT_SPEED_HERO || effect.type == EFFECT_BALL_FIRE || o_ptr->name2 == EGO_DEVICE_QUICKNESS)
                skill_boost -= 20;
        }

        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_EASY_SPELL))
        {
            int d = effect.difficulty;

            d -= MAX(o_ptr->pval, effect.difficulty * 10 * o_ptr->pval / 300);
            if (d <= 1) d = 1;
            effect.difficulty = d;
        }

        if (o_ptr->curse_flags & OFC_CURSED)
            effect.difficulty += effect.difficulty / 5;

        return effect_calc_fail_rate_aux(&effect, skill_boost);
    }
    if (p_ptr->pclass == CLASS_BERSERKER) return 1000;
    if (beorning_is_(BEORNING_FORM_BEAR)) return 1000;

    lev = k_info[o_ptr->k_idx].level;
    if (lev > 50) lev = 50 + (lev - 50)/2;
    chance = p_ptr->skills.dev;
    if (p_ptr->confused) chance = chance / 2;
    chance = chance - lev;
    if (chance < USE_DEVICE)
        fail = 1000 - 1000/(3 * (USE_DEVICE - chance + 1));
    else
        fail = (USE_DEVICE-1)*1000/chance;

    if ((p_ptr->stun) && (fail < 950))
    {
        fail += 500 * p_ptr->stun / 100;
        if (fail > 950) fail = 950;
    }
    if (o_ptr->tval == TV_SCROLL && fail > 500) fail = 500;
    return fail;
}

/* Hack: When using an unknown rod we force the user to target. Also
   Trap Location should not spoil with the view_unsafe_grids option. */
bool device_known = FALSE;

/* Hack: Allow player to learn device power thru actual use. They can
 * also learn the fail rate (i.e. difficulty) by failing enough times,
 * but that is handled elsewhere. We deal solely with OFL_DEVICE_POWER. */
bool device_lore = FALSE;

/* Hack: When using an unknown device, was there an observable effect?
   If so, identify the device. */
bool device_noticed = FALSE;

/* Hack for Device Master's desperation. This power uses all the charges
   in a device at once (with diminishing returns) and potentially destroys
   the device as well. */
int  device_extra_power = 0;

/* Hack for identifying all relevant objects in a single action.
   It's ugly, but worthwhile! */
int  device_available_charges = 0; /* How many can we do? */
int  device_used_charges = 0;      /* How many did we do? */
static bool _use_charges = FALSE;
static bool _multi_charge_lock = FALSE;

static void _do_identify_aux(obj_ptr obj)
{
    char name[MAX_NLEN];
    bool old_known;

    if ((_use_charges) && (device_used_charges >= device_available_charges)) return;

    old_known = identify_item(obj);
    object_desc(name, obj, OD_COLOR_CODED);
    switch (obj->loc.where)
    {
    case INV_EQUIP:
        msg_format("%^s: %s (%c).", equip_describe_slot(obj->loc.slot),
                name, slot_label(obj->loc.slot));
        break;
/* case INV_PACK:
        msg_format("In your pack: %s.", name);
        break;
    case INV_QUIVER:
        msg_format("In your quiver: %s.", name);
        break;*/
    case INV_PACK:
    case INV_QUIVER:
        obj->marked |= OM_DELAYED_MSG;
        p_ptr->notice |= PN_CARRY;
        break;
    case INV_FLOOR:
        msg_format("在地上: %s。", name);
        break;
    }
    autopick_alter_obj(obj, destroy_identify && !old_known);
    obj_release(obj, OBJ_RELEASE_ID | OBJ_RELEASE_QUIET);
    if (_use_charges) device_used_charges++;
}

void mass_identify(bool use_charges) /* shared with Sorcery spell */
{
    inv_ptr floor = inv_filter_floor(point(px, py), obj_exists);

    _use_charges = use_charges;
    _multi_charge_lock = TRUE;
    pack_for_each_that(_do_identify_aux, obj_is_unknown);
    equip_for_each_that(_do_identify_aux, obj_is_unknown);
    quiver_for_each_that(_do_identify_aux, obj_is_unknown);
    inv_for_each_that(floor, _do_identify_aux, obj_is_unknown);
    _multi_charge_lock = FALSE;

    inv_free(floor);
}

static int _cmd_handler(obj_prompt_context_ptr context, int cmd)
{
    if (cmd == '*')
        return OP_CMD_DISMISS;
    return OP_CMD_SKIPPED;
}

static bool _do_identify(void)
{
    obj_prompt_t prompt = {0};

    assert(device_used_charges == 0);

    prompt.prompt = "鉴定哪件物品 <color:w>(<color:keypress>*</color> 鉴定全部)</color>？";
    prompt.error = "所有物品均已鉴定。";
    prompt.filter = obj_is_unknown;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.cmd_handler = _cmd_handler;
    obj_prompt_add_special_packs(&prompt);

    switch (obj_prompt(&prompt))
    {
    case OP_CUSTOM:
        mass_identify(TRUE);
        return TRUE;
    case OP_SUCCESS:
        _use_charges = TRUE;
        _do_identify_aux(prompt.obj);
        return TRUE;
    }
    return FALSE;
}

/* Using Devices
      if (!device_try(o_ptr)) ... "You failed to use the device" ...
      if (device_use(o_ptr)) ... Decrement Charges/Unstack/Etc. ...
*/
bool device_try(object_type *o_ptr)
{
    int fail = device_calc_fail_rate(o_ptr);
    if (randint0(1000) < fail)
        return FALSE;
    return TRUE;
}

bool device_use(object_type *o_ptr, int boost)
{
    device_known = object_is_known(o_ptr);
    if (do_device(o_ptr, SPELL_CAST, boost))
        return TRUE;
    return FALSE;
}

static int _scroll_power(int val)
{
    if (devicemaster_is_(DEVICEMASTER_SCROLLS))
    {
        val += val * device_extra_power / 100;
        return device_power_aux(val, /*p_ptr->device_power + */p_ptr->lev/10);
    }
    return val;
}

static int _potion_power(int val)
{
    if (devicemaster_is_(DEVICEMASTER_POTIONS))
    {
        val += val * device_extra_power / 100;
        return device_power_aux(val, /*p_ptr->device_power + */p_ptr->lev/10);
    }
    return val;
}

static cptr _do_potion(int sval, int mode)
{
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    switch (sval)
    {
    case SV_POTION_WATER:
        if (desc) return "这只是水。";
        if (cast)
        {
            msg_print("你觉得不那么渴了。");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_APPLE_JUICE:
        if (desc) return "它尝起来很甜。";
        if (cast)
        {
            msg_print("你觉得不那么渴了。");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SLIME_MOLD:
        if (desc) return "它尝起来很怪异。";
        if (cast)
        {
            msg_print("你觉得不那么渴了。");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SLOWNESS:
        if (desc) return "饮用它会让你暂时减速。";
        if (cast)
        {
            if (set_slow(randint1(25) + 15, FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_POTION_SALT_WATER:
        if (desc) return "饮用它会让你几乎饿晕并麻痹你，但同时也会解毒。";
        if (cast)
        {
            if (( !(get_race()->flags & RACE_IS_NONLIVING)
              && !prace_is_(RACE_MON_JELLY) ) || prace_is_(RACE_EINHERI))
            {
                msg_print("这药水让你作呕！");
                set_food(PY_FOOD_STARVE - 1);
                set_paralyzed(randint1(4), FALSE);
                set_poisoned(0, TRUE);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_POISON:
        if (desc) return "饮用它会让你中毒。";
        if (cast)
        {
            if (!res_save_default(RES_POIS))
            {
                if (set_poisoned(p_ptr->poisoned + randint0(15) + 10, FALSE))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_BLINDNESS:
        if (desc) return "饮用它会让你致盲。";
        if (cast)
        {
            if (!res_save_default(RES_BLIND))
            {
                if (set_blind(p_ptr->blind + randint0(100) + 100, FALSE))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_CONFUSION: /* Booze */
        if (desc) return "饮用它会让你混乱并产生幻觉。如果你是一名武僧，你可能会成为一名醉拳大师。";
        if (cast)
        {
            if (p_ptr->pclass != CLASS_MONK)
                virtue_add(VIRTUE_HARMONY, -1);
            if (!res_save_default(RES_CONF))
            {
                if (p_ptr->pclass == CLASS_MONK)
                    p_ptr->special_attack |= ATTACK_SUIKEN;
                if (set_confused(randint0(20) + 15, FALSE))
                    device_noticed = TRUE;
            }

            if (!res_save_default(RES_CHAOS))
            {
                if (one_in_(2))
                {
                    if (set_image(p_ptr->image + randint0(25) + 25, FALSE))
                        device_noticed = TRUE;
                }
                if (one_in_(13) && (p_ptr->pclass != CLASS_MONK))
                {
                    device_noticed = TRUE;
                    if (one_in_(3)) lose_all_info();
                    else wiz_dark();
                    teleport_player_aux(100, TELEPORT_NONMAGICAL | TELEPORT_PASSIVE);
                    wiz_dark();
                    msg_print("你头痛欲裂地在某个地方醒来……");
                    msg_print("你什么都不记得了，也不知道自己是怎么到这里来的！");
                }
            }
        }
        break;
    case SV_POTION_SLEEP:
        if (desc) return "饮用它会麻痹你。";
        if (cast)
        {
            if (!free_act_save_p(0))
            {
                msg_print("你睡着了。");

                if (ironman_nightmare)
                {
                    msg_print("一个可怕的幻象进入了你的脑海。");
                    get_mon_num_prep(get_nightmare, NULL);
                    have_nightmare(get_mon_num(MAX_DEPTH));
                    get_mon_num_prep(NULL, NULL);
                }
                if (set_paralyzed(randint1(4), FALSE))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case SV_POTION_LIQUID_LOGRUS:
        if (desc) return "It has unpredictable effects when you quaff it, either "
                         "1) curing 5000 HP and all temporary ailments, "
                         "2) increasing all stats, 3) decreasing all stats, "
                         "4) providing temporary light speed, "
                         "5) temporarily turning your skin to stone, "
                         "6) giving or curing mutations randomly, "
                         "7) repeatedly giving or curing mutations randomly, "
                         "8) temporarily making you gigantic, "
                         "9) triggering the Baby Foul Curse, "
                         "10) temporarily polymorphing you into an Ent, "
                         "11) making you hallucinate temporarily, "
                         "12) curing 420 HP and all ailments but making you "
                         "hallucinate temporarily, "
                         "or 13) exploding in your mouth for 11d66 damage, "
                         "massive stunning and a dangerous wound.";
        if (cast)
        {
            int noppa = randint1(13);
            if (p_ptr->good_luck) noppa--;
            else if (mut_present(MUT_BAD_LUCK)) noppa++;
            switch (noppa)
            {
                case 0:
                case 1:
                    virtue_add(VIRTUE_CHANCE, 5);
                    (void)_do_potion(SV_POTION_LIFE, SPELL_CAST);
                     break;
                case 2:
                    virtue_add(VIRTUE_CHANCE, 5);
                    (void)_do_potion(SV_POTION_AUGMENTATION, SPELL_CAST);
                    break;
                case 3:
                    virtue_add(VIRTUE_CHANCE, -5);
                    msg_print("你觉得力量变弱了！");
                    dec_stat(A_DEX, 10, TRUE);
                    dec_stat(A_WIS, 10, TRUE);
                    dec_stat(A_CON, 10, TRUE);
                    dec_stat(A_STR, 10, TRUE);
                    dec_stat(A_CHR, 10, TRUE);
                    dec_stat(A_INT, 10, TRUE);
                    break;
                case 4:
                    virtue_add(VIRTUE_CHANCE, 2);
                    set_lightspeed(_potion_power(25 + randint1(50)), FALSE);
                    break;
                case 5:
                    (void)_do_potion(SV_POTION_STONE_SKIN, SPELL_CAST);
                    break;
                case 6:
                case 7:
                {
                    int ii, ct = 1;
                    if (noppa == 7) ct += randint1(3);
                    for (ii = 0; ii < ct; ii++)
                    {
                        (void)_do_potion(SV_POTION_POLYMORPH, SPELL_CAST);
                    }
                    break;
                }
                case 8:
                    (void)_do_potion(SV_POTION_GIANT_STRENGTH, SPELL_CAST);
                    break;
                case 9:
                    virtue_add(VIRTUE_CHANCE, -2);
                    nonlethal_ty_substitute(TRUE);
                    break;
                case 10:
                    if (player_obviously_poly_immune(TRUE) || mut_present(MUT_DRACONIAN_METAMORPHOSIS))
                        nonlethal_ty_substitute(TRUE);
                    else
                        set_mimic(20 + randint1(20), RACE_ENT, TRUE);
                    break;
                case 11:
                    set_image(50 + randint1(50), TRUE);
                    break;
                case 12:
                    virtue_add(VIRTUE_CHANCE, 1);
                    virtue_add(VIRTUE_VITALITY, 1);
                    virtue_add(VIRTUE_UNLIFE, -5);
                    msg_print("你感觉生命力在你的体内流淌！");
                    restore_level();
                    lp_player(1000);
                    set_poisoned(0, TRUE);
                    set_blind(0, TRUE);
                    set_confused(0, TRUE);
                    set_image(0, TRUE);
                    set_stun(0, TRUE);
                    set_cut(0, TRUE);
                    set_unwell(0, TRUE);
                    do_res_stat(A_STR);
                    do_res_stat(A_CON);
                    do_res_stat(A_DEX);
                    do_res_stat(A_WIS);
                    do_res_stat(A_INT);
                    do_res_stat(A_CHR);
                    set_shero(0,TRUE);
                    (void)p_inc_minislow(-10);
                    p_ptr->slow = 0;
                    update_stuff();
                    hp_player(_potion_power(420));
                    set_image(50 + randint1(50), TRUE);
                    break;
                default:
                    virtue_add(VIRTUE_CHANCE, -10);
                    msg_print("剧烈的爆炸撕裂了你的身体！");
                    take_hit(DAMAGE_NOESCAPE, damroll(11, 66), "液态罗格鲁斯药水");

                    set_stun(MAX(p_ptr->stun, STUN_MASSIVE), FALSE);
                    set_cut(p_ptr->cut + 3630, FALSE);
                    device_noticed = TRUE;
                    break;
            }
        }
        break;
    case SV_POTION_LOSE_MEMORIES:
        if (desc) return "饮用它会使你失去经验。";
        if (cast)
        {
            if (!p_ptr->hold_life && (p_ptr->exp > 0))
            {
                msg_print("你感觉自己的记忆正在消退。");
                virtue_add(VIRTUE_KNOWLEDGE, -5);
                lose_exp(p_ptr->exp / 4);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_MEAD_OF_POETRY:
        if (desc) return "由被谋杀的智者克瓦希尔（Kvasir）之血酿造而成，饮用后会暂时获得极大的智慧与雄辩能力。";
        if (info) return info_duration(_potion_power(75), _potion_power(75));
        if (cast)
        {
            int dur = _potion_power(100 + randint1(100));
            if (set_tim_poet(p_ptr->tim_poet + dur, FALSE))
            {
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_RUINATION:
        if (desc) return "饮用它不仅会使你受到伤害，还会永久降低你的所有属性。";
        if (cast)
        {
            msg_print("你的神经和肌肉感觉软弱无力！");
            take_hit(DAMAGE_LOSELIFE, damroll(10, 10), "毁灭药水");

            dec_stat(A_DEX, 25, TRUE);
            dec_stat(A_WIS, 25, TRUE);
            dec_stat(A_CON, 25, TRUE);
            dec_stat(A_STR, 25, TRUE);
            dec_stat(A_CHR, 25, TRUE);
            dec_stat(A_INT, 25, TRUE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_STR:
        if (desc) return "饮用它会降低你的力量。";
        if (cast)
        {
            if (do_dec_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_INT:
        if (desc) return "饮用它会降低你的智力。";
        if (cast)
        {
            if (do_dec_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_WIS:
        if (desc) return "饮用它会降低你的感知。";
        if (cast)
        {
            if (do_dec_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_DEX:
        if (desc) return "饮用它会降低你的敏捷。";
        if (cast)
        {
            if (do_dec_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_CON:
        if (desc) return "饮用它会降低你的体质。";
        if (cast)
        {
            if (do_dec_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEC_CHR:
        if (desc) return "饮用它会降低你的魅力。";
        if (cast)
        {
            if (do_dec_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_DETONATIONS:
        if (desc) return "饮用它时会在你嘴里爆炸。";
        if (cast)
        {
            msg_print("剧烈的爆炸撕裂了你的身体！");
            take_hit(DAMAGE_NOESCAPE, damroll(50, 20), "爆炸药水");

            set_stun(MAX(p_ptr->stun, STUN_MASSIVE), FALSE);
            set_cut(p_ptr->cut + 5000, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_DEATH:
        if (desc) return "饮用它会让你当场死亡。";
        if (cast)
        {
            virtue_add(VIRTUE_VITALITY, -1);
            virtue_add(VIRTUE_UNLIFE, 5);
            msg_print("一种死亡的感觉流遍你的全身。");
            take_hit(DAMAGE_LOSELIFE, 5000, "死亡药水");
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SIGHT:
        if (desc) return "饮用它会使你暂时获得识破隐形和红外视力，并治愈致盲。";
        if (info) return info_duration(_potion_power(100), _potion_power(100));
        if (cast)
        {
            int dur = _potion_power(100 + randint1(100));
            if (set_tim_infra(p_ptr->tim_infra + dur, FALSE))
            {
                device_noticed = TRUE;
            }
			if (set_tim_invis(p_ptr->tim_invis + dur, FALSE))
			{
				device_noticed = TRUE;
			}
			if (set_blind(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_POISON: //anti-toxin
        if (desc) return "饮用它会解毒，并赋予临时的毒素抗性。";
        if (cast)
        {
			int dur = _potion_power(10 + randint1(10));
			if (set_poisoned(p_ptr->poisoned - MAX(400, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
			if (set_oppose_pois(p_ptr->oppose_pois + dur, FALSE))
			{
				device_noticed = TRUE;
			}
        }
        break;
    case SV_POTION_BOLDNESS:
        if (desc) return "饮用它会消除恐惧。";
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_SPEED:
        if (desc) return "饮用它会使你暂时加速。";
        if (info) return format("持续 d%d+%d", _potion_power(25), _potion_power(15));
        if (cast)
        {
            if (!p_ptr->fast)
            {
                int dur = _potion_power(randint1(25) + 15);
                if (set_fast(dur, FALSE)) device_noticed = TRUE;
            }
            else if (p_ptr->pclass == CLASS_MAULER)
                set_fast(p_ptr->fast + 10, FALSE);
            else
                set_fast(p_ptr->fast + 5, FALSE);
        }
        break;
    case SV_POTION_THERMAL:
        if (desc) return "饮用它会使你暂时获得对火焰和寒冷的抗性。这种抗性可以与装备叠加。";
        if (info) return format("持续 d%d+%d", _potion_power(10), _potion_power(10));
        if (cast)
        {
            int dur = _potion_power(10 + randint1(10));
            if (set_oppose_fire(p_ptr->oppose_fire + dur, FALSE))
            {
                device_noticed = TRUE;
            }
			if (set_oppose_cold(p_ptr->oppose_cold + dur, FALSE))
			{
				device_noticed = TRUE;
			}
        }
        break;
    case SV_POTION_VIGOR:
        if (desc) return "饮用它能治愈所有的震慑状态和暂时的减速效果。";
        if (cast)
        {
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_slow(0, TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-10)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_HEROISM:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "饮用它能使你暂时获得英雄气概，并提供特殊的炼金术士加成（+2 速度以及基于等级的额外命中、伤害和射击次数加成）。" : "饮用它能使你暂时获得英雄气概。";
        if (info) return format("持续 d%d+%d", _potion_power(25), _potion_power(25));
        if (cast)
        {
            int dur = _potion_power(25 + randint1(25));
            if (set_hero(p_ptr->hero + dur, FALSE)) device_noticed = TRUE;
            if (p_ptr->pclass == CLASS_ALCHEMIST)
            {
                alchemist_set_hero(&device_noticed, p_ptr->hero + dur, TRUE);
                if (device_noticed)
                {
                    p_ptr->update |= (PU_BONUS);
                    handle_stuff();
                }
            }
        }
        break;
    case SV_POTION_BERSERK_STRENGTH:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "饮用它能使你陷入狂暴，并提供特殊的炼金术士加成（+4 速度以及基于等级的额外命中、伤害、攻击次数和射击次数加成）。" : "饮用它能使你陷入狂暴。";
        if (info) return format("持续 d%d+%d", _potion_power(25), _potion_power(25));
        if (cast)
        {
            int dur = _potion_power(25 + randint1(25));
            if (set_shero(p_ptr->shero + dur, FALSE)) device_noticed = TRUE;
            if (hp_player(30)) device_noticed = TRUE;
            if (p_ptr->pclass == CLASS_ALCHEMIST)
            {
                alchemist_set_hero(&device_noticed, p_ptr->shero + dur, FALSE);
                if (device_noticed)
                {
                    p_ptr->update |= (PU_BONUS);
                    handle_stuff();
                }
            }
        }
        break;
    case SV_POTION_CURE_LIGHT:
        if (desc) return "饮用它能稍微治愈你的生命值，解除狂暴状态并减轻割伤。";
        if (info) return info_heal(4, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(4, 8)))) device_noticed = TRUE;
            if (set_cut(p_ptr->cut - 15, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_SERIOUS:
        if (desc) return "饮用它能治愈部分生命值，解除狂暴状态并减轻割伤。";
        if (info) return info_heal(8, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(8, 8)))) device_noticed = TRUE;
            if (set_cut((p_ptr->cut / 2) - 50, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURE_CRITICAL:
        if (desc) return "饮用它能治愈你的生命值，并解除震慑、割伤和狂暴状态。";
        if (info) return info_heal(12, _potion_power(8), 0);
        if (cast)
        {
            if (hp_player(_potion_power(damroll(12, 8)))) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
//	if (set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_BLOOD:
        if (desc) return "一场急需的输液！饮用它能治愈你部分生命值，并解除致盲、混乱和震慑状态。";
        if (info) return info_heal(0, 0, _potion_power(200));
        if (cast)
        {
            if (hp_player(_potion_power(200))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_HEALING: {
        int amt = 300;
        if (desc) return "饮用它能治愈你的生命值，并解除致盲、混乱、震慑、割伤和狂暴状态。";
        if (info) return info_heal(0, 0, _potion_power(amt));
        if (cast)
        {
            if (hp_player(_potion_power(amt))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
        }
        break; }
    case SV_POTION_STAR_HEALING:
        if (desc) return "饮用它能治愈你的生命值，并解除致盲、混乱、中毒、震慑、割伤、疾病和狂暴状态。";
        if (info) return info_heal(0, 0, _potion_power(1000));
        if (cast)
        {
            if (hp_player(_potion_power(1000))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0, TRUE)) device_noticed = TRUE;
            if (set_unwell(0, TRUE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
            update_stuff(); /* hp may change if the player was unwell ... */
        }
        break;
    case SV_POTION_LIFE:
        if (desc) return "饮用它能完全治愈你的生命值，恢复你的生命力、经验值及所有属性，并解除致盲、混乱、中毒、幻觉、震慑、割伤、减速、疾病和狂暴状态。";
        if (info) return info_heal(0, 0, _potion_power(5000));
        if (cast)
        {
            virtue_add(VIRTUE_VITALITY, 1);
            virtue_add(VIRTUE_UNLIFE, -5);
            msg_print("你感觉生命力在你的体内流淌！");
            restore_level();
            lp_player(1000);
            set_poisoned(0, TRUE);
            set_blind(0, TRUE);
            set_confused(0, TRUE);
            set_image(0, TRUE);
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            set_unwell(0, TRUE);
            do_res_stat(A_STR);
            do_res_stat(A_CON);
            do_res_stat(A_DEX);
            do_res_stat(A_WIS);
            do_res_stat(A_INT);
            do_res_stat(A_CHR);
            set_shero(0,TRUE);
            (void)p_inc_minislow(-10);
            p_ptr->slow = 0;
            update_stuff();
            hp_player(_potion_power(5000));
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_CLARITY:
        if (desc) return "饮用它能使你的头脑变得清醒，解除混乱并恢复部分法力。";
        if (info) return format("3d%d + %d", _potion_power(6), _potion_power(3));
        if (cast)
        {
            int amt = _potion_power(damroll(3, 6) + 3);

            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("你不受影响。");
            else if (sp_player(amt))
            {
                msg_print("你感觉头脑清醒了。");
                device_noticed = TRUE;
            }
		if (set_confused(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_GREAT_CLARITY:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "饮用它能极大程度地清醒你的头脑，解除混乱、震慑和幻觉状态。" : "饮用它能极大程度地清醒你的头脑，解除混乱和幻觉状态。";
        if (info) return format("10d%d + %d", _potion_power(10), _potion_power(15));
        if (cast)
        {
            int amt = _potion_power(damroll(10, 10) + 15);

            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("你不受影响。");
            else if (sp_player(amt))
            {
                msg_print("你感觉头脑清醒了。");
                device_noticed = TRUE;
            }
			if (set_confused(0, TRUE)) device_noticed = TRUE;
			if (set_image(0, TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RESTORE_MANA:
        if (desc) return "饮用它能将法力恢复至满，并解除狂暴状态。它还能部分地为你背包中的所有装置进行充能。";
        if (cast)
        {
            if (restore_mana()) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RESTORE_EXP:
        if (desc) return "饮用它能恢复你的生命力与经验值。";
        if (cast)
        {
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_STR:
        if (desc) return "饮用它能恢复你的力量。";
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_INT:
        if (desc) return "饮用它能恢复你的智力。";
        if (cast)
        {
            if (do_res_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_WIS:
        if (desc) return "饮用它能恢复你的感知。";
        if (cast)
        {
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_DEX:
        if (desc) return "饮用它能恢复你的敏捷。";
        if (cast)
        {
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_CON:
        if (desc) return "饮用它能恢复你的体质。";
        if (cast)
        {
            if (do_res_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_RES_CHR:
        if (desc) return "饮用它能恢复你的魅力。";
        if (cast)
        {
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
	case SV_POTION_RES_ALL:
		if (desc) return "饮用它能恢复你的属性、生命力与经验值。";
		if (cast)
		{
			if (do_res_stat(A_STR)) device_noticed = TRUE;
			if (do_res_stat(A_INT)) device_noticed = TRUE;
			if (do_res_stat(A_WIS)) device_noticed = TRUE;
			if (do_res_stat(A_DEX)) device_noticed = TRUE;
			if (do_res_stat(A_CON)) device_noticed = TRUE;
			if (do_res_stat(A_CHR)) device_noticed = TRUE;
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
		}
		break;
    case SV_POTION_INC_STR:
        if (desc) return "饮用它能提升你的力量。";
        if (cast)
        {
            if (do_inc_stat(A_STR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_INT:
        if (desc) return "饮用它能提升你的智力。";
        if (cast)
        {
            if (do_inc_stat(A_INT)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_WIS:
        if (desc) return "饮用它能提升你的感知。";
        if (cast)
        {
            if (do_inc_stat(A_WIS)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_DEX:
        if (desc) return "饮用它能提升你的敏捷。";
        if (cast)
        {
            if (do_inc_stat(A_DEX)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_CON:
        if (desc) return "饮用它能提升你的体质。";
        if (cast)
        {
            if (do_inc_stat(A_CON)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_INC_CHR:
        if (desc) return "饮用它能提升你的魅力。";
        if (cast)
        {
            if (do_inc_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_AUGMENTATION:
        if (desc) return "饮用它能提升你的所有属性。";
        if (cast)
        {
            if (do_inc_stat(A_STR)) device_noticed = TRUE;
            if (do_inc_stat(A_INT)) device_noticed = TRUE;
            if (do_inc_stat(A_WIS)) device_noticed = TRUE;
            if (do_inc_stat(A_DEX)) device_noticed = TRUE;
            if (do_inc_stat(A_CON)) device_noticed = TRUE;
            if (do_inc_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_ENLIGHTENMENT:
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "饮用它能为整个楼层绘制地图、永久照亮并探测所有物品，同时还能提供临时的心灵感应。" : "饮用它能为整个楼层绘制地图、永久照亮并探测所有物品。";
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            msg_print("你脑海中浮现出周围环境的影像……");
            wiz_lite(p_ptr->tim_superstealth > 0);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_STAR_ENLIGHTENMENT: /* warning - long line ahead */
        if (desc) return (p_ptr->pclass == CLASS_ALCHEMIST) ? "饮用它能为整个楼层绘制地图、永久照亮并探测所有物品，提升你的智力与感知，探测你附近的所有陷阱、门、楼梯和宝藏，鉴定你背包中的所有物品，提供临时的心灵感应，并给予你关于自身的启示信息。" : "饮用它能为整个楼层绘制地图、永久照亮并探测所有物品，提升你的智力与感知，探测你附近的所有陷阱、门、楼梯和宝藏，鉴定你背包中的所有物品，并给予你关于自身的启示信息。";
        if (cast)
        {
            msg_print("你开始觉得受到了启示……");
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 2);
            msg_print(NULL);
            wiz_lite(p_ptr->tim_superstealth > 0);
            do_inc_stat(A_INT);
            do_inc_stat(A_WIS);
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            detect_treasure(DETECT_RAD_DEFAULT);
            detect_objects_gold(DETECT_RAD_DEFAULT);
            detect_objects_normal(DETECT_RAD_DEFAULT);
            identify_pack();
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_SELF_KNOWLEDGE:
        if (desc) return "饮用它会给予你关于自身的启示信息。";
        if (cast)
        {
            msg_print("你开始对自己有了更深入的了解……");
            msg_print(NULL);
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_EXPERIENCE:
        if (desc) return "饮用它能使你获得更多经验。";
        if (cast)
        {
            if (p_ptr->prace == RACE_ANDROID) break;
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            if (p_ptr->exp < PY_MAX_EXP)
            {
                s32b ee = _potion_power((p_ptr->exp / 2) + 10);
                s32b max = _potion_power(100000);
                if (mut_present(MUT_FAST_LEARNER))
                {
                    ee = ee * 5/3;
                    max = max * 5/3;
                }
                if (ee > max) ee = max;
                msg_print("你感觉自己更有经验了。");
                gain_exp(ee);
                device_noticed = TRUE;
            }
        }
        break;
    case SV_POTION_RESISTANCE:
        if (desc) return "You get temporary resistance to the elements and poison when you quaff it. ";
        if (info) return format("持续 d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            int dur = _potion_power(20 + randint1(20));
            set_oppose_base(dur, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_CURING: {
        if (desc) return "饮用它能治愈致盲、混乱、震慑、割伤和幻觉状态，并减轻中毒程度。";
        if (cast)
        {
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(200, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_image(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break; }
    case SV_POTION_INVULNERABILITY:
        if (desc) return "饮用它能使你暂时处于无敌状态。";
        if (info) return format("持续 d%d+%d", _potion_power(4), _potion_power(4));
        if (cast)
        {
            int dur = _potion_power(4 + randint1(4));
            set_invuln(p_ptr->invuln + dur, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_NEW_LIFE:
        if (desc) return "饮用它能改变你的生命评级和所有属性的最大值，并治愈所有变异。";
        if (cast)
        {
            do_cmd_rerate(FALSE);
            lp_player(1000);
            get_max_stats();
            p_ptr->update |= PU_BONUS;
            mut_lose_all();
            device_noticed = TRUE;
            if (p_ptr->personality == PERS_SPLIT)
                split_shuffle(2);
            if (p_ptr->pclass == CLASS_WILD_TALENT)
                wild_talent_new_life();
            /* XXX Originally, this was here as an act of mercy for players new to this class.
             * However, it is hugely scummable since you can pick the powers useful in early play
             * and then new life to switch over to those useful in late game. Several psion powers
             * are huge in the early game, but not so much later on. Players will abuse this. Also,
             * the Skillmaster has exactly the same permanent-irreversible-don't-screw-up-your-choices
             * game mechanic and receive no such love. Keeping things for the Wild Talent makes
             * sense though, since they are random (and don't get a say in the matter anyway).
            if (p_ptr->pclass == CLASS_PSION && get_check("Relearn Powers? "))
                psion_relearn_powers();*/
        }
        break;
    case SV_POTION_NEO_TSUYOSHI:
        if (desc) return "饮用它能治愈幻觉并暂时提升你的力量和体质，但在效果结束后，你的力量和体质会永久下降至低于之前的水平。";
        if (cast)
        {
            set_image(0, TRUE);
            set_tsuyoshi(p_ptr->tsuyoshi + randint1(100) + 100, FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_TSUYOSHI:
        if (desc) return "饮用它会永久降低你的力量和体质，并使你产生幻觉。";
        if (cast)
        {
            msg_print("OKURE 兄弟！");
            msg_print(NULL);
            p_ptr->tsuyoshi = 1;
            set_tsuyoshi(0, TRUE);
            if (!res_save_default(RES_CHAOS))
                set_image(50 + randint1(50), FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_POTION_GIANT_STRENGTH:
        if (desc) return "饮用它会暂时极大地增加你的体型。";
        if (info) return format("持续 d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            if (set_tim_building_up(_potion_power(20 + randint1(20)), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_POTION_POLYMORPH:
        if (desc) return "饮用它会使你发生变异。在极少数情况下，它会治愈你所有的变异。";
        if (cast)
        {
            int count = mut_count(mut_unlocked_pred);
            if (count > 1 && one_in_(23))
            {
                mut_lose_all();
                if (p_ptr->pclass == CLASS_WILD_TALENT)
                    wild_talent_new_life();
            }
            else
            {
                do
                {
                    if (one_in_(2))
                    {
                        if(mut_gain_random(NULL))
                        {
                            count++;
                            device_noticed = TRUE;
                        }
                    }
                    else if (count > 5 || one_in_(6 - count))
                    {
                        if (mut_lose_random(NULL))
                        {
                            count--;
                            device_noticed = TRUE;
                        }
                    }
                } while (!device_noticed || one_in_(2));

                if (p_ptr->pclass == CLASS_WILD_TALENT && one_in_(2))
                    wild_talent_scramble();
            }
        }
        break;
    case SV_POTION_STONE_SKIN:
        if (desc) return "当你喝下它时，它会暂时将你的皮肤变成石头，从而提高你的护甲等级。";
        if (info) return format("持续 d%d+%d", _potion_power(20), _potion_power(20));
        if (cast)
        {
            if (set_shield(_potion_power(20 + randint1(20)), FALSE)) device_noticed = TRUE;
        }
        break;
    }
    return "";
}

static bool _scroll_check_no_effect(int sval)
{
    int k_idx = lookup_kind(TV_SCROLL, sval);
    if (!k_info[k_idx].aware) return TRUE;
    if (msg_prompt("这张卷轴在这里没有效果。无论如何都要阅读吗？<color:y>[y/n]</color>", "ny", PROMPT_DEFAULT) == 'y') return TRUE;
    return FALSE;
}

static cptr _do_scroll(int sval, int mode)
{
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

    switch (sval)
    {
    case SV_SCROLL_DARKNESS:
        if (desc) return "阅读它会使附近区域或当前房间变暗，并使你致盲。";
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                if (set_blind(p_ptr->blind + 3 + randint1(5), FALSE)) device_noticed = TRUE;
            }
            if (unlite_area(10, 3)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_AGGRAVATE_MONSTER:
        if (desc) return "阅读它会激怒你附近的怪物。";
        if (cast)
        {
            msg_print("发出了一阵尖锐的嗡嗡声。");
            aggravate_monsters(0);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_CURSE_ARMOR:
        if (desc) return "阅读它会使你当前穿戴的护甲变成(损坏)(Blasted)状态。";
        if (cast)
        {
            int slot = equip_random_slot(object_is_armour);
            if (slot && curse_armor(slot)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_CURSE_WEAPON:
        if (desc) return "阅读它会破坏你当前装备的近战武器。";
        if (cast)
        {
            int slot = equip_random_slot(object_is_melee_weapon);
            if (slot && curse_weapon(FALSE, slot)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SUMMON_MONSTER:
        if (desc) return "阅读它会召唤几个怪物作为你的敌人。";
        if (cast)
        {
            int i;
            for (i = 0; i < randint1(3); i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_SUMMON_UNDEAD:
        if (desc) return "阅读它会召唤几个不死怪物作为你的敌人。";
        if (cast)
        {
            int i;
            for (i = 0; i < randint1(3); i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_UNDEAD, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_SUMMON_PET:
        if (desc)
        {
            if (p_ptr->prace == RACE_MON_RING)
                return "阅读它会召唤一名持戒者（戒灵）作为你的宠物。";
            else
                return "阅读它会召唤一个怪物作为你的宠物。";
        }
        if (cast)
        {
            int type = 0;
            if (p_ptr->prace == RACE_MON_RING)
                type = SUMMON_RING_BEARER;
            if (p_ptr->inside_arena && !type && !prace_is_(RACE_MON_QUYLTHULG) && !_scroll_check_no_effect(sval)) return NULL;
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, _scroll_power(dun_level), type, (PM_ALLOW_GROUP | PM_FORCE_PET)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SUMMON_KIN:
        if (desc) return "当你阅读它时，它会召唤一个与你种族相同的怪物作为你的宠物。";
        if (cast)
        {
            if (p_ptr->inside_arena && !prace_is_(RACE_MON_QUYLTHULG) && !_scroll_check_no_effect(sval)) return NULL;
            if (summon_kin_player(_scroll_power(p_ptr->lev), py, px, (PM_FORCE_PET | PM_ALLOW_GROUP)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TRAP_CREATION:
        if (desc) return "阅读它会在你相邻的方格上生成陷阱。";
        if (cast)
        {
            if (trap_creation(py, px))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_PHASE_DOOR:
        if (desc) return "阅读它会将你传送到短距离之外。";
        if (cast)
        {
            teleport_player(10, 0L);
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TELEPORT:
        if (desc) return "阅读它会将你传送到长距离之外。";
        if (cast)
        {
            teleport_player(100, 0L);
            energy_use = energy_use * 3 / 2;
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_TELEPORT);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_TELEPORT_LEVEL:
        if (desc) return "阅读它会立刻将你向上传送或向下传送到相邻的地下城楼层。";
        if (cast)
        {
            if ((TELE_LEVEL_IS_INEFF(-1)) && (!_scroll_check_no_effect(sval))) return NULL;
            device_noticed = TRUE;
            if (k_info[lookup_kind(TV_SCROLL, sval)].aware)
            {
                if (!py_teleport_level(NULL)) return NULL;
            }
            else teleport_level(0);
        }
        break;
    case SV_SCROLL_WORD_OF_RECALL:
        if (desc) return "阅读它会将你召回城镇，或者将你送回你之前所在的地下城深处。";
        if (cast)
        {
            device_noticed = TRUE;
            if (!word_of_recall(k_info[lookup_kind(TV_SCROLL, sval)].aware)) return NULL;
        }
        break;
    case SV_SCROLL_IDENTIFY:
        if (desc) return "阅读它能鉴定一件物品。";
        if (cast)
        {
            device_noticed = TRUE;
            if (!_do_identify()) return NULL;
        }
        break;
    case SV_SCROLL_STAR_IDENTIFY:
        if (desc) return "阅读它能揭示一件物品的所有信息（全面鉴定）。";
        if (cast)
        {
            device_noticed = TRUE;
            if (!identify_fully(NULL)) return NULL;
        }
        break;
    case SV_SCROLL_REMOVE_CURSE:
        if (desc) return "阅读它能解除已装备物品上的普通诅咒。";
        if (cast)
        {
            if (remove_curse())
            {
                msg_print("你感觉好像有人在守护着你。");
                device_noticed = TRUE;
            }
        }
        break;
    case SV_SCROLL_STAR_REMOVE_CURSE:
        if (desc) return "阅读它能解除已装备物品上的普通诅咒和重度诅咒。";
        if (cast)
        {
            if (remove_all_curse())
                msg_print("你感觉好像有人在守护着你。");
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_ARMOR:
        if (desc) return "阅读它能提升一件护甲的防御加值(to-AC)。";
        if (cast)
        {
            if (!enchant_spell(0, 0, 1)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
        if (desc) return "阅读它能提升一件武器的命中加值(to-hit)。";
        if (cast)
        {
            if (!enchant_spell(1, 0, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
        if (desc) return "阅读它能提升一件武器的伤害加值(to-dam)。";
        if (cast)
        {
            if (!enchant_spell(0, 1, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ENCHANT_ARMOR:
        if (desc) return "阅读它能强力地提升一件护甲的防御加值(to-AC)。";
        if (cast)
        {
            if (!enchant_spell(0, 0, randint1(3) + 3)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ENCHANT_WEAPON:
        if (desc) return "阅读它能强力地提升一件武器的命中加值和伤害加值。";
        if (cast)
        {
            if (!enchant_spell(randint1(3) + 3, randint1(3) + 3, 0)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RECHARGING:
        if (desc) return "阅读它会尝试使用源装置的法力为另一件魔法装置充能。这通常会摧毁那个源装置。";
        if (cast)
        {
            if (!recharge_from_device(_scroll_power(100))) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MUNDANITY:
        if (desc) return "这会消除一件物品的 Ego 或神器状态及其所有附魔。作为“附赠”，如果你对一整叠物品使用，多余的物品将会被摧毁。";
        if (cast)
        {
            if (!mundane_spell(FALSE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_LIGHT:
        if (desc) return "阅读它能永久照亮附近区域或当前房间。";
        if (cast)
        {
            if (lite_area(damroll(2, 8), 2)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MAPPING:
        if (desc) return "阅读它能为你附近的区域绘制地图。";
        if (cast)
        {
            map_area(_scroll_power(DETECT_RAD_MAP));
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_GOLD:
        if (desc) return "阅读它能探测你附近的所有宝藏。";
        if (cast)
        {
            if (detect_treasure(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
            if (detect_objects_gold(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_ITEM:
        if (desc) return "阅读它能探测你附近的所有物品。";
        if (cast)
        {
            if (detect_objects_normal(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_TRAP:
        if (desc) return "阅读它能探测你附近的所有陷阱。";
        if (cast)
        {
            if (detect_traps(_scroll_power(DETECT_RAD_DEFAULT), device_known)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_DOOR:
        if (desc) return "阅读它能探测你附近所有的门和楼梯。";
        if (cast)
        {
            if (detect_doors(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
            if (detect_stairs(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_INVIS:
        if (desc) return "阅读它能探测你附近所有隐形的怪物。";
        if (cast)
        {
            if (detect_monsters_invis(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_DETECT_MONSTERS:
        if (desc) return "阅读它能探测你附近所有的怪物。";
        if (cast)
        {
            if (detect_monsters_normal(_scroll_power(DETECT_RAD_DEFAULT))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SATISFY_HUNGER:
        if (desc) return "阅读它能满足你的饥饿（充饥）。";
        if (cast)
        {
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_BLESSING:
        if (desc) return "阅读它能使你暂时受到祝福。";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(12) + 6), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_HOLY_CHANT:
        if (desc) return "阅读它能使你暂时受到祝福。";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(24) + 12), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_HOLY_PRAYER:
        if (desc) return "阅读它能使你暂时受到祝福。";
        if (cast)
        {
            if (set_blessed(p_ptr->blessed + _scroll_power(randint1(48) + 24), FALSE)) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MONSTER_CONFUSION:
        if (desc) return "阅读它之后，你可以让你下一次击中的怪物陷入混乱（仅限一次）。";
        if (cast)
        {
            if (p_ptr->prace == RACE_MON_RING) /* no melee attacks */
            {
                msg_print("毫无效果。");
            }
            else if (!(p_ptr->special_attack & ATTACK_CONFUSE))
            {
                msg_print("你的双手开始发光。");
                p_ptr->special_attack |= ATTACK_CONFUSE;
                p_ptr->redraw |= (PR_STATUS);
            }
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_PROTECTION_FROM_EVIL:
        if (desc) return "阅读它能为你提供抵御次级邪恶生物的临时防护。";
        if (cast)
        {
            if (set_protevil(p_ptr->protevil + _scroll_power(randint1(25) + 3 * p_ptr->lev), FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RUNE_OF_PROTECTION:
        if (desc) return "阅读它会在你所站立的地面上生成一个结界符文。";
        if (cast)
        {
            device_noticed = TRUE;
            if ((!cave_clean_bold(py, px)) && (msg_prompt("守护结界符文只能被创造在空地上。无论如何都要阅读卷轴吗？<color:y>[y/n]</color>", "ny", PROMPT_DEFAULT) != 'y')) return NULL;
            warding_glyph();
        }
        break;
    case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
        if (desc) return "阅读它会摧毁你相邻地面上的所有陷阱。";
        if (cast)
        {
            if (destroy_doors_touch()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_DESTRUCTION:
        if (desc) return "阅读它会摧毁你附近的一切。";
        if (cast)
        {
            if (((!py_in_dungeon()) || (!quests_allow_all_spells()))
               && (!_scroll_check_no_effect(sval))) return NULL;
            if (destroy_area(py, px, 13 + randint0(5), _scroll_power(2000)))
                device_noticed = TRUE;
            else
                msg_print("大地在颤抖……");
        }
        break;
    case SV_SCROLL_DISPEL_UNDEAD:
        if (desc) return "阅读它会对视线内所有的不死怪物造成伤害。";
        if (info) return info_damage(0, 0, _scroll_power(80));
        if (cast)
        {
            if (dispel_undead(_scroll_power(80))) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_SPELL:
        if (desc) return "增加你可学习法术的数量。仅对需要学习法术的职业有效。";
        if (cast)
        {
            if (!class_uses_spell_scrolls(p_ptr->pclass))
            {
                msg_print("毫无效果。");
            }
            else
            {
                p_ptr->add_spells++;
                p_ptr->update |= (PU_SPELLS);
            }
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_GENOCIDE:
        if (desc) return "尝试将当前楼层内所有指定种族的怪物抹除，这会让你筋疲力尽。";
        if (cast)
        {
            if (((!quests_allow_all_spells()) || (p_ptr->inside_arena) || (p_ptr->inside_battle))
               && (!_scroll_check_no_effect(sval))) return NULL;
            if (!symbol_genocide(_scroll_power(300), TRUE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MASS_GENOCIDE:
        if (desc) return "尝试消灭附近所有的怪物，这会让你筋疲力尽。";
        if (cast)
        {
            if (((!quests_allow_all_spells()) || (p_ptr->inside_arena) || (p_ptr->inside_battle))
               && (!_scroll_check_no_effect(sval))) return NULL;
            mass_genocide(_scroll_power(300), TRUE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ACQUIREMENT:
        if (desc) return "阅读它能创造一件卓越的物品。在更深的楼层会有更好的结果。";
        if (cast)
        {
            acquirement(py, px, 1, TRUE, FALSE, ORIGIN_ACQUIRE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_STAR_ACQUIREMENT:
        if (desc) return "阅读它能创造数件卓越的物品。在更深的楼层会有更好的结果。";
        if (cast)
        {
            acquirement(py, px, _scroll_power(randint1(2) + 1), TRUE, FALSE, ORIGIN_ACQUIRE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_FOREST_CREATION:
        if (desc) return "它会在你周围生成植被。";
        if (cast)
        {
            if (tree_creation()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_WALL_CREATION:
        if (desc) return "它会在你周围生成岩石。";
        if (cast)
        {
            if (wall_stone()) device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_VENGEANCE:
        if (desc) return "在短时间内，攻击你的怪物将受到同等伤害的反击。";
        if (cast)
        {
            set_tim_eyeeye(_scroll_power(randint1(25) + 25), FALSE);
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_RUMOR:
        if (desc) return "里面记载着一个传闻。";
        if (cast)
        {
            char Rumor[1024];
            errr err = 0;

            switch (randint1(20))
            {
            case 1:
                err = get_rnd_line("chainswd.txt", 0, Rumor);
                break;
            case 2:
                err = get_rnd_line("error.txt", 0, Rumor);
                break;
            case 3:
            case 4:
            case 5:
                err = get_rnd_line("death.txt", 0, Rumor);
                break;
            default:
                err = get_rnd_line("rumors.txt", 0, Rumor);
            }

            if (err) strcpy(Rumor, "有些传闻是错的。");
            msg_format("<color:B>卷轴上有一条留言，写着：</color> %s", Rumor);
            msg_print("卷轴在一阵烟雾中消失了！");
            device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_ARTIFACT:
        if (desc) return "阅读它能将一件未命名（非神器）的武器或护甲变成神器。在更深的楼层会有更好的结果。不要太贪心——你只会获得一件神器。";
        if (cast)
        {
            device_noticed = TRUE;
            if (no_artifacts)
            {
                if (!brand_weapon(-1)) return NULL;
            }
            else
            {
                if (!artifact_scroll()) return NULL;
            }
        }
        break;
    case SV_SCROLL_MADNESS:
        if (desc) return "这似乎是一个处于伟大奥秘发现边缘的疯狂巫师匆忙写下的涂鸦。你完全摸不着头脑。你想读读看会发生什么吗？";
        if (cast)
        {
            int n = randint0(_scroll_power(100));
            device_noticed = TRUE;
            if (n < 2)
            {
                int curses = 1 + randint1(3);
                bool stop_ty = FALSE;
                int count = 0;

                cmsg_print(TERM_VIOLET, "这张卷轴带有一种古老而邪恶的诅咒！");
                curse_equipment(100, 50);
                do
                {
                    stop_ty = activate_ty_curse(stop_ty, &count);
                }
                while (--curses);
            }
            else if (n < 12)
            {
                msg_print("哎呀！那一点用都没有！");
                destroy_area(py, px, 13 + randint0(5), 300);
            }
            else if (n < 17)
            {
                msg_print("你隐约听到了一阵疯狂的笑声。");
                summon_cyber(-1, py, px);
            }
            else if (n < 27)
            {
                msg_print("卷轴猛烈地爆炸了！");
                project(0, 10, py, px, 300, GF_MANA, PROJECT_KILL | PROJECT_ITEM);
            }
            else if (n < 50)
            {
                _do_scroll(SV_SCROLL_CURSE_ARMOR, mode);
            }
            else if (n < 75)
            {
                _do_scroll(SV_SCROLL_CURSE_WEAPON, mode);
            }
            else if (n < 95)
            {
                if (!_do_scroll(SV_SCROLL_CRAFTING, mode)) return NULL;
            }
            else
            {
                _do_scroll(SV_SCROLL_ARTIFACT, mode);
            }
        }
        break;
    case SV_SCROLL_CRAFTING:
        if (desc) return "阅读它能使所选的武器、护甲或弹药变成一件高级(ego)物品。";
        if (cast)
        {
            device_noticed = TRUE;
            if (!cast_crafting())
                return NULL;
        }
        break;
    case SV_SCROLL_RESET_RECALL:
        if (desc) return "当你阅读它时，它会重置召回法术的目标地下城层数。";
        if (cast)
        {
            device_noticed = TRUE;
            if (!reset_recall()) return NULL;
        }
        break;
    case SV_SCROLL_FIRE:
        if (desc) return "它会以你为中心制造一个巨大的火球。";
        if (info) return info_damage(0, 0, _scroll_power(333));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_FIRE, 0, _scroll_power(666), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_FIRE))
            {
                int dam = res_calc_dam(RES_FIRE, 25 + randint1(25));
                take_hit(DAMAGE_NOESCAPE, dam, "火焰卷轴");
            }
        }
        break;
    case SV_SCROLL_ICE:
        if (desc) return "它会以你为中心制造一个巨大的冰球。";
        if (info) return info_damage(0, 0, _scroll_power(400));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_ICE, 0, _scroll_power(800), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_COLD))
            {
                int dam = res_calc_dam(RES_COLD, 30 + randint1(30));
                take_hit(DAMAGE_NOESCAPE, dam, "寒冰卷轴");
            }
        }
        break;
/*    case SV_SCROLL_CHAOS:
        if (desc) return "It creates a huge ball of logrus centered on you.";
        if (info) return info_damage(0, 0, _scroll_power(500));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_CHAOS, 0, _scroll_power(1000), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS) && !res_save_default(RES_CHAOS))
            {
                int dam = res_calc_dam(RES_CHAOS, 50 + randint1(50));
                take_hit(DAMAGE_NOESCAPE, dam, "a Scroll of Logrus");
            }
        }
        break;*/
    case SV_SCROLL_UNDERSTANDING:
        if (desc) return "阅读它能鉴定你背包里的所有物品，并赋予临时的自动鉴定能力。";
        if (cast)
        {
            if (set_tim_understanding(_scroll_power(40), FALSE))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_MANA:
        if (desc) return "它会以你为中心制造一个巨大的纯净法力球。";
        if (info) return info_damage(0, 0, _scroll_power(550));
        if (cast)
        {
            device_noticed = TRUE;
            fire_ball(GF_MANA, 0, _scroll_power(1100), 4);
            if (!devicemaster_is_(DEVICEMASTER_SCROLLS))
                take_hit(DAMAGE_NOESCAPE, 50 + randint1(50), "法力卷轴");
        }
        break;
    case SV_SCROLL_BANISHMENT:
        if (desc) return "除非被抵抗，否则它会将视线内的所有怪物传送走。";
        if (info) return info_power(_scroll_power(150));
        if (cast)
        {
            if (banish_monsters(_scroll_power(150)))
                device_noticed = TRUE;
        }
        break;
    case SV_SCROLL_INVEN_PROT:
        if (desc) return "它会在你的物品栏周围生成一层临时的防护盾（防止物品被毁）。";
        if (cast)
        {
            if (set_tim_inven_prot2(p_ptr->tim_inven_prot2 + _scroll_power(25), FALSE)) device_noticed = TRUE;
        }
        break;
    }
    return "";
}

cptr do_device(object_type *o_ptr, int mode, int boost)
{
    cptr result = NULL;

    device_noticed = FALSE;
    if (!_multi_charge_lock) device_used_charges = 0;
    device_lore = FALSE;

    if (o_ptr->activation.type)
    {
        u32b flgs[OF_ARRAY_SIZE];

        obj_flags(o_ptr, flgs);
        if (have_flag(flgs, OF_DEVICE_POWER))
            boost += device_power_aux(100, o_ptr->pval) - 100;

        result = do_effect(&o_ptr->activation, mode, boost);
    }
    else
    {
        switch (o_ptr->tval)
        {
            case TV_SCROLL: result = _do_scroll(o_ptr->sval, mode); break;
            case TV_POTION:
            {
                result = _do_potion(o_ptr->sval, mode);
                if ((p_ptr->pclass == CLASS_ALCHEMIST) && (mode == SPELL_CAST)) alchemist_super_potion_effect(o_ptr->sval);
                break;
            }
        }
    }
    device_known = FALSE;
    device_extra_power = 0;
    if (!_multi_charge_lock) device_available_charges = 0;
    return result;
}

/* Effects: We are following the do_spell() pattern which is quick and dirty,
   but not my preferred approach ... Also, we could conceivably merge all
   devices into effects, handling rods, staves, wands, potions, scrolls and
   activations uniformly. For the moment, effects are *just* activations,
   and I should mention that each type of effect has its own little quirky
   fail rate calculation ... sigh.

   Update: The Device Rewrite is merging Wands/Rods/Staves into the effect system!*/
effect_t obj_get_effect(object_type *o_ptr)
{
    if (o_ptr->activation.type)
        return o_ptr->activation;
    if (o_ptr->name1 && a_info[o_ptr->name1].activation.type)
        return a_info[o_ptr->name1].activation;
    if (o_ptr->name2 && e_info[o_ptr->name2].activation.type)
        return e_info[o_ptr->name2].activation;
    return k_info[o_ptr->k_idx].activation;
}

cptr obj_get_effect_msg(object_type *o_ptr)
{
    u32b offset;

    if (o_ptr->activation.type)
        return 0;

    if (o_ptr->name1 && a_info[o_ptr->name1].activation.type)
    {
        offset = a_info[o_ptr->name1].activation_msg;
        if (offset)
            return a_text + offset;
        else
            return 0;
    }
    if (o_ptr->name2 && e_info[o_ptr->name2].activation.type)
    {
        return 0;
    }

    offset = k_info[o_ptr->k_idx].activation_msg;
    if (offset)
        return k_text + offset;

    return 0;
}

bool obj_has_effect(object_type *o_ptr)
{
    effect_t e = obj_get_effect(o_ptr);
    if (e.type)
        return TRUE;
    return FALSE;
}

bool effect_try(effect_t *effect)
{
    int fail = effect_calc_fail_rate(effect);
    if (randint0(1000) < fail)
        return FALSE;
    return TRUE;
}

bool effect_use(effect_t *effect, int boost)
{
    device_noticed = FALSE;
    device_used_charges = 0;
    device_available_charges = 1;
    if (do_effect(effect, SPELL_CAST, boost))
        return TRUE;
    return FALSE;
}

int effect_value(effect_t *effect)
{
    int  result = 0;
    cptr hack = do_effect(effect, SPELL_VALUE, 0);
    if (hack)
        result = atoi(hack);
    return result;
}

int effect_cost_extra(effect_t *effect)
{
    int  result = 0;
    cptr hack = do_effect(effect, SPELL_COST_EXTRA, 0);
    if (hack)
        result = atoi(hack);
    return result;
}

byte effect_color(effect_t *effect)
{
    byte result = TERM_WHITE;
    cptr hack = do_effect(effect, SPELL_COLOR, 0);
    if (hack && strlen(hack))
        result = atoi(hack);
    return result;
}

typedef struct
{
    cptr text;
    int  type;
    int  level;
    int  cost;
    int  rarity;
    int  bias;
    bool known;
} _effect_info_t, *_effect_info_ptr;

/*  Allocation Table for Random Artifact Activations
    This also assists parsing a_info.txt, k_info.txt and e_info.txt.
    Order is irrelevant. Use Rarity 0 to exclude allocations.
*/
static _effect_info_t _effect_info[] =
{
    /* Detection:                                   Lv    T   R  Bias */
    {"LITE_AREA",       EFFECT_LITE_AREA,            1,  10,  1, BIAS_MAGE},
    {"LITE_MAP_AREA",   EFFECT_LITE_MAP_AREA,       20,  50,  3, 0},
    {"ENLIGHTENMENT",   EFFECT_ENLIGHTENMENT,       20,  50,  2, BIAS_PRIESTLY | BIAS_ARCHER},
    {"CLAIRVOYANCE",    EFFECT_CLAIRVOYANCE,        35, 100,  8, BIAS_MAGE},

    {"DETECT_TRAPS",    EFFECT_DETECT_TRAPS,         3,  10,  1, BIAS_ROGUE},
    {"DETECT_MONSTERS", EFFECT_DETECT_MONSTERS,      5,  20,  1, BIAS_MAGE | BIAS_ARCHER},
    {"DETECT_OBJECTS",  EFFECT_DETECT_OBJECTS,       8,  25,  1, BIAS_ROGUE},
    {"DETECT_ALL",      EFFECT_DETECT_ALL,          25,  50,  3, BIAS_ROGUE},
    {"DETECT_GOLD",     EFFECT_DETECT_GOLD,          5,  20,  1, BIAS_ROGUE},
    {"DETECT_INVISIBLE",EFFECT_DETECT_INVISIBLE,     5,  20,  1, 0},
    {"DETECT_DOOR_STAIRS",EFFECT_DETECT_DOOR_STAIRS,10,  25,  1, 0},
    {"DETECT_EVIL",     EFFECT_DETECT_EVIL,         20,  30,  1, BIAS_PRIESTLY},

    /* Utility:                                     Lv    T   R  Bias */
    {"PHASE_DOOR",      EFFECT_PHASE_DOOR,           8,  20,  1, BIAS_MAGE | BIAS_ARCHER},
    {"TELEPORT",        EFFECT_TELEPORT,            12,  20,  1, BIAS_ROGUE},
    {"TELEPORT_AWAY",   EFFECT_TELEPORT_AWAY,       20,  50,  2, BIAS_MAGE},
    {"STRAFING",        EFFECT_STRAFING,            20,  10,  3, BIAS_ARCHER},
    {"DIMENSION_DOOR",  EFFECT_DIMENSION_DOOR,      50, 100,  8, BIAS_MAGE | BIAS_ARCHER},
    {"ESCAPE",          EFFECT_ESCAPE,              30,  70,  3, BIAS_ROGUE},
    {"RECALL",          EFFECT_RECALL,              25, 100,  2, BIAS_MAGE},

    {"STONE_TO_MUD",    EFFECT_STONE_TO_MUD,        15,  20,  1, BIAS_STR | BIAS_RANGER},
    {"EARTHQUAKE",      EFFECT_EARTHQUAKE,          25, 100,  1, BIAS_STR},
    {"DESTRUCTION",     EFFECT_DESTRUCTION,         50, 250,  6, BIAS_DEMON},
    {"GENOCIDE",        EFFECT_GENOCIDE,            70, 500, 16, BIAS_NECROMANTIC},
    {"MASS_GENOCIDE",   EFFECT_MASS_GENOCIDE,       80, 750, 16, BIAS_NECROMANTIC},

    {"RECHARGE_FROM_DEVICE", EFFECT_RECHARGE_FROM_DEVICE, 35, 500,  3, BIAS_MAGE | BIAS_DEMON},
    {"RECHARGE_FROM_PLAYER", EFFECT_RECHARGE_FROM_PLAYER, 90, 500, 16, BIAS_MAGE | BIAS_DEMON},

    {"ENCHANTMENT",     EFFECT_ENCHANTMENT,         30, 900, 16, 0},
    {"IDENTIFY",        EFFECT_IDENTIFY,            15,  50,  1, BIAS_ROGUE | BIAS_MAGE},
    {"IDENTIFY_FULL",   EFFECT_IDENTIFY_FULL,       50, 200,  3, BIAS_ROGUE | BIAS_MAGE},
    {"PROBING",         EFFECT_PROBING,             30,  50,  1, BIAS_MAGE},
    {"RUNE_EXPLOSIVE",  EFFECT_RUNE_EXPLOSIVE,      30, 100,  2, BIAS_MAGE},
    {"RUNE_PROTECTION", EFFECT_RUNE_PROTECTION,     70, 500,  4, BIAS_PRIESTLY},

    {"SATISFY_HUNGER",  EFFECT_SATISFY_HUNGER,       5, 100,  1, BIAS_RANGER},
    {"DESTROY_TRAP",    EFFECT_DESTROY_TRAP,        20,  50,  1, 0},
    {"DESTROY_TRAPS",   EFFECT_DESTROY_TRAPS,       25,  50,  1, BIAS_ROGUE},
    {"WHIRLWIND_ATTACK",EFFECT_WHIRLWIND_ATTACK,    50, 100,  4, BIAS_WARRIOR},
    {"LIST_UNIQUES",    EFFECT_LIST_UNIQUES,        80, 250,  8, 0},
    {"LIST_ARTIFACTS",  EFFECT_LIST_ARTIFACTS,      80, 250,  8, 0},
    {"BANISH_EVIL",     EFFECT_BANISH_EVIL,         50, 100,  4, BIAS_PRIESTLY | BIAS_LAW},
    {"BANISH_ALL",      EFFECT_BANISH_ALL,          50, 100,  8, BIAS_MAGE},
    {"TELEKINESIS",     EFFECT_TELEKINESIS,         25, 100,  2, BIAS_MAGE},
    {"ALCHEMY",         EFFECT_ALCHEMY,             70, 100,  4, BIAS_MAGE},
    {"SELF_KNOWLEDGE",  EFFECT_SELF_KNOWLEDGE,      70, 500,  3, BIAS_MAGE},
    {"GENOCIDE_ONE",    EFFECT_GENOCIDE_ONE,        60, 250,  8, BIAS_NECROMANTIC},

    /* Timed Buffs:                                 Lv    T   R  Bias */
    {"STONE_SKIN",      EFFECT_STONE_SKIN,          25, 150,  2, BIAS_WARRIOR | BIAS_PROTECTION},
    {"RESIST_ACID",     EFFECT_RESIST_ACID,         15, 100,  1, BIAS_ACID | BIAS_PROTECTION},
    {"RESIST_ELEC",     EFFECT_RESIST_ELEC,         15, 100,  1, BIAS_ELEC | BIAS_PROTECTION},
    {"RESIST_FIRE",     EFFECT_RESIST_FIRE,         15, 100,  1, BIAS_FIRE | BIAS_DEMON | BIAS_PROTECTION},
    {"RESIST_COLD",     EFFECT_RESIST_COLD,         15, 100,  1, BIAS_COLD | BIAS_PROTECTION},
    {"RESIST_POIS",     EFFECT_RESIST_POIS,         30, 150,  2, BIAS_POIS | BIAS_PROTECTION},
    {"RESISTANCE",      EFFECT_RESISTANCE,          35, 200,  4, BIAS_RANGER | BIAS_PROTECTION},
    {"PROT_EVIL",       EFFECT_PROT_EVIL,           35, 200,  2, BIAS_PRIESTLY | BIAS_LAW | BIAS_PROTECTION},
    {"HOLY_GRAIL",      EFFECT_HOLY_GRAIL,          50, 500,  4, BIAS_PRIESTLY},
    {"BLESS",           EFFECT_BLESS,               10, 100,  1, BIAS_PRIESTLY | BIAS_DEMON},
    {"HEROISM",         EFFECT_HEROISM,             15, 100,  1, BIAS_WARRIOR | BIAS_PRIESTLY | BIAS_DEMON},
    {"BERSERK",         EFFECT_BERSERK,             20, 100,  2, BIAS_WARRIOR},
    {"SPEED",           EFFECT_SPEED,               25, 150,  4, BIAS_ROGUE | BIAS_MAGE | BIAS_ARCHER},
    {"SPEED_HERO",      EFFECT_SPEED_HERO,          35, 200,  6, BIAS_WARRIOR},
    {"SPEED_HERO_BLESS",EFFECT_SPEED_HERO_BLESS,    40, 250,  8, 0},
    {"LIGHT_SPEED",     EFFECT_LIGHT_SPEED,         99, 999, 99, 0},
    {"ENLARGE_WEAPON",  EFFECT_ENLARGE_WEAPON,      80, 900,  0, 0},
    {"TELEPATHY",       EFFECT_TELEPATHY,           30, 150,  8, BIAS_MAGE | BIAS_ARCHER},
    {"WRAITHFORM",      EFFECT_WRAITHFORM,          90, 666, 16, BIAS_NECROMANTIC},
    {"INVULNERABILITY", EFFECT_INVULNERABILITY,     90, 777, 16, BIAS_MAGE | BIAS_PROTECTION},

    /* Pets:                                        Lv    T   R  Bias */
    {"SUMMON_MONSTERS", EFFECT_SUMMON_MONSTERS,     30, 500,  1, BIAS_MAGE},
    {"SUMMON_HOUNDS",   EFFECT_SUMMON_HOUNDS,       35, 500,  1, BIAS_RANGER},
    {"SUMMON_ANTS",     EFFECT_SUMMON_ANTS,         20, 200,  1, BIAS_RANGER},
    {"SUMMON_HYDRAS",   EFFECT_SUMMON_HYDRAS,       40, 500,  1, BIAS_RANGER},
    {"SUMMON_OCTOPUS",  EFFECT_SUMMON_OCTOPUS,      30, 600, 16, 0},
    {"SUMMON_DAWN",     EFFECT_SUMMON_DAWN,         60, 888, 16, 0},
    {"SUMMON_PHANTASMAL",EFFECT_SUMMON_PHANTASMAL,  25, 150,  1, BIAS_MAGE},
    {"SUMMON_ELEMENTAL",EFFECT_SUMMON_ELEMENTAL,    30, 150,  1, BIAS_MAGE},
    {"SUMMON_DRAGON",   EFFECT_SUMMON_DRAGON,       60, 600,  2, BIAS_MAGE},
    {"SUMMON_UNDEAD",   EFFECT_SUMMON_UNDEAD,       60, 600,  2, BIAS_NECROMANTIC},
    {"SUMMON_DEMON",    EFFECT_SUMMON_DEMON,        66, 666,  2, BIAS_CHAOS | BIAS_DEMON},
    {"SUMMON_CYBERDEMON",EFFECT_SUMMON_CYBERDEMON,  90, 900, 16, BIAS_CHAOS},
    {"SUMMON_ANGEL",    EFFECT_SUMMON_ANGEL,        70, 777,  8, BIAS_PRIESTLY},
    {"SUMMON_KRAKEN",   EFFECT_SUMMON_KRAKEN,       90, 900,  8, 0},

    {"CHARM_ANIMAL",    EFFECT_CHARM_ANIMAL,        30, 200,  1, BIAS_RANGER},
    {"CHARM_DEMON",     EFFECT_CHARM_DEMON,         50, 500,  1, BIAS_CHAOS | BIAS_DEMON},
    {"CHARM_UNDEAD",    EFFECT_CHARM_UNDEAD,        50, 500,  1, BIAS_NECROMANTIC},
    {"CHARM_MONSTER",   EFFECT_CHARM_MONSTER,       30, 100,  1, BIAS_MAGE},

    {"RETURN_PETS",     EFFECT_RETURN_PETS,         10,   0,  0, 0},
    {"CAPTURE_PET",     EFFECT_CAPTURE_PET,         20,   0,  0, 0},

    /* Healing and Recovery:                        Lv    T   R  Bias */
    {"RESTORE_STATS",   EFFECT_RESTORE_STATS,       50, 600,  2, BIAS_PRIESTLY},
    {"RESTORE_EXP",     EFFECT_RESTORE_EXP,         40, 500,  1, BIAS_PRIESTLY},
    {"RESTORING",       EFFECT_RESTORING,           70, 800,  3, BIAS_PRIESTLY},
    {"HEAL",            EFFECT_HEAL,                40, 500,  1, BIAS_PRIESTLY},
    {"CURING",          EFFECT_CURING,              45, 200,  1, BIAS_PRIESTLY},
    {"HEAL_CURING",     EFFECT_HEAL_CURING,         60, 900,  4, BIAS_PRIESTLY},
    {"HEAL_CURING_HERO",EFFECT_HEAL_CURING_HERO,    70, 900,  4, BIAS_PRIESTLY},
    {"RESTORE_MANA",    EFFECT_RESTORE_MANA,        80, 900,  8, BIAS_MAGE},
    {"CURE_POIS",       EFFECT_CURE_POIS,           10,  50,  1, BIAS_POIS | BIAS_PRIESTLY | BIAS_RANGER},
    {"CURE_FEAR",       EFFECT_CURE_FEAR,           25, 100,  1, BIAS_PRIESTLY},
    {"CURE_FEAR_POIS",  EFFECT_CURE_FEAR_POIS,      30, 100,  1, BIAS_PRIESTLY},
    {"REMOVE_CURSE",    EFFECT_REMOVE_CURSE,        30, 200,  1, BIAS_PRIESTLY},
    {"REMOVE_ALL_CURSE",EFFECT_REMOVE_ALL_CURSE,    70, 500,  4, BIAS_PRIESTLY},
    {"CLARITY",         EFFECT_CLARITY,             20,  15, 12, BIAS_PRIESTLY | BIAS_MAGE},
    {"GREAT_CLARITY",   EFFECT_GREAT_CLARITY,       80,  75, 64, BIAS_PRIESTLY | BIAS_MAGE},

    /* Offense: Bolts                               Lv    T   R  Bias */
    {"BOLT_MISSILE",    EFFECT_BOLT_MISSILE,         1,  10,  1, BIAS_MAGE | BIAS_ARCHER},
    {"BOLT_ACID",       EFFECT_BOLT_ACID,           15,  20,  1, BIAS_ACID},
    {"BOLT_ELEC",       EFFECT_BOLT_ELEC,           15,  20,  1, BIAS_ELEC},
    {"BOLT_FIRE",       EFFECT_BOLT_FIRE,           15,  20,  1, BIAS_FIRE | BIAS_DEMON},
    {"BOLT_COLD",       EFFECT_BOLT_COLD,           15,  20,  1, BIAS_COLD},
    {"BOLT_POIS",       EFFECT_BOLT_POIS,           10,  10,  1, BIAS_POIS},
    {"BOLT_LITE",       EFFECT_BOLT_LITE,           20,  25,  1, 0},
    {"BOLT_DARK",       EFFECT_BOLT_DARK,           20,  25,  1, BIAS_NECROMANTIC},
    {"BOLT_CONF",       EFFECT_BOLT_CONF,           30,  50,  2, 0},
    {"BOLT_NETHER",     EFFECT_BOLT_NETHER,         20,  25,  1, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BOLT_NEXUS",      EFFECT_BOLT_NEXUS,          20,  25,  2, 0},
    {"BOLT_SOUND",      EFFECT_BOLT_SOUND,          40,  50,  2, BIAS_LAW},
    {"BOLT_SHARDS",     EFFECT_BOLT_SHARDS,         40,  50,  2, BIAS_LAW},
    {"BOLT_CHAOS",      EFFECT_BOLT_CHAOS,          50, 100,  2, BIAS_CHAOS},
    {"BOLT_DISEN",      EFFECT_BOLT_DISEN,          40,  50,  2, 0},
    {"BOLT_TIME",       EFFECT_BOLT_TIME,           60, 200, 90, 0},
    {"BOLT_WATER",      EFFECT_BOLT_WATER,          55, 150,  4, BIAS_MAGE},
    {"BOLT_MANA",       EFFECT_BOLT_MANA,           50, 100,  4, BIAS_MAGE},
    {"BOLT_ICE",        EFFECT_BOLT_ICE,            50, 100,  4, BIAS_MAGE | BIAS_COLD},
    {"BOLT_PLASMA",     EFFECT_BOLT_PLASMA,         50, 100,  4, BIAS_FIRE},

    /* Offense: Beams                               Lv    T   R  Bias */
    {"BEAM_LITE_WEAK",  EFFECT_BEAM_LITE_WEAK,      10,  20,  1, 0},
    {"BEAM_LITE",       EFFECT_BEAM_LITE,           40, 100,  2, 0},
    {"BEAM_GRAVITY",    EFFECT_BEAM_GRAVITY,        50, 150,  8, 0},
    {"BEAM_DISINTEGRATE",EFFECT_BEAM_DISINTEGRATE,  60, 200, 16, 0},
    {"BEAM_ACID",       EFFECT_BEAM_ACID,           20,  20,  2, BIAS_ACID},
    {"BEAM_ELEC",       EFFECT_BEAM_ELEC,           20,  20,  2, BIAS_ELEC},
    {"BEAM_FIRE",       EFFECT_BEAM_FIRE,           20,  20,  2, BIAS_FIRE | BIAS_DEMON},
    {"BEAM_COLD",       EFFECT_BEAM_COLD,           20,  20,  2, BIAS_COLD},
    {"BEAM_SOUND",      EFFECT_BEAM_SOUND,          45,  50,  3, BIAS_LAW},
    {"BEAM_CHAOS",      EFFECT_BEAM_CHAOS,          55, 100,  3, BIAS_CHAOS},

    /* Offense: Balls                               Lv    T   R  Bias */
    {"BALL_ACID",       EFFECT_BALL_ACID,           25,  50,  1, BIAS_ACID},
    {"BALL_ELEC",       EFFECT_BALL_ELEC,           25,  50,  1, BIAS_ELEC},
    {"BALL_FIRE",       EFFECT_BALL_FIRE,           25,  50,  1, BIAS_FIRE | BIAS_DEMON},
    {"BALL_COLD",       EFFECT_BALL_COLD,           25,  50,  1, BIAS_COLD},
    {"BALL_POIS",       EFFECT_BALL_POIS,           10,   5,  1, BIAS_POIS},
    {"BALL_LITE",       EFFECT_BALL_LITE,           65, 100,  2, 0},
    {"BALL_DARK",       EFFECT_BALL_DARK,           66, 100,  2, BIAS_NECROMANTIC},
    {"BALL_CONF",       EFFECT_BALL_CONF,           50, 150,  2, 0},
    {"BALL_NETHER",     EFFECT_BALL_NETHER,         40,  50,  2, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BALL_NEXUS",      EFFECT_BALL_NEXUS,          50, 100,  2, 0},
    {"BALL_SOUND",      EFFECT_BALL_SOUND,          60, 100,  2, BIAS_LAW},
    {"BALL_SHARDS",     EFFECT_BALL_SHARDS,         60, 100,  2, BIAS_LAW},
    {"BALL_CHAOS",      EFFECT_BALL_CHAOS,          65, 150,  4, BIAS_CHAOS},
    {"BALL_DISEN",      EFFECT_BALL_DISEN,          55, 100,  2, 0},
    {"BALL_TIME",       EFFECT_BALL_TIME,           80, 250, 32, 0},
    {"BALL_WATER",      EFFECT_BALL_WATER,          70, 200,  4, BIAS_MAGE},
    {"BALL_MANA",       EFFECT_BALL_MANA,           80, 200,  6, BIAS_MAGE},
    {"BALL_DISINTEGRATE", EFFECT_BALL_DISINTEGRATE, 60, 200, 16, 0},

    /* Offense: Breaths                             Lv    T   R  Bias */
    {"BREATHE_ACID",    EFFECT_BREATHE_ACID,        40, 100,  2, BIAS_ACID},
    {"BREATHE_ELEC",    EFFECT_BREATHE_ELEC,        40, 100,  2, BIAS_ELEC},
    {"BREATHE_FIRE",    EFFECT_BREATHE_FIRE,        40, 100,  2, BIAS_FIRE},
    {"BREATHE_COLD",    EFFECT_BREATHE_COLD,        40, 100,  2, BIAS_COLD},
    {"BREATHE_POIS",    EFFECT_BREATHE_POIS,        40, 100,  2, BIAS_POIS},
    {"BREATHE_LITE",    EFFECT_BREATHE_LITE,        50, 125,  3, 0},
    {"BREATHE_DARK",    EFFECT_BREATHE_DARK,        50, 125,  3, BIAS_NECROMANTIC},
    {"BREATHE_CONF",    EFFECT_BREATHE_CONF,        60, 200,  4, 0},
    {"BREATHE_NETHER",  EFFECT_BREATHE_NETHER,      50,  75,  2, BIAS_NECROMANTIC | BIAS_DEMON},
    {"BREATHE_NEXUS",   EFFECT_BREATHE_NEXUS,       60, 150,  4, 0},
    {"BREATHE_SOUND",   EFFECT_BREATHE_SOUND,       70, 200,  4, BIAS_LAW},
    {"BREATHE_SHARDS",  EFFECT_BREATHE_SHARDS,      70, 200,  4, BIAS_LAW},
    {"BREATHE_CHAOS",   EFFECT_BREATHE_CHAOS,       75, 250,  4, BIAS_CHAOS},
    {"BREATHE_DISEN",   EFFECT_BREATHE_DISEN,       60, 150,  8, 0},
	{"BREATHE_INERTIA", EFFECT_BREATHE_INERTIA,     60, 200,  8, 0},
	{"BREATHE_WATER",   EFFECT_BREATHE_WATER,       65, 150,  8, BIAS_MAGE },
    {"BREATHE_TIME",    EFFECT_BREATHE_TIME,        90, 500, 32, 0},
    {"BREATHE_ELEMENTS", EFFECT_BREATHE_ELEMENTS,   60, 100, 64, 0},

    {"BREATHE_ONE_MULTIHUED", EFFECT_BREATHE_ONE_MULTIHUED, 0, 0, 0, 0},
    {"BREATHE_ONE_CHAOS",EFFECT_BREATHE_ONE_CHAOS,   0, 0, 0, 0},
    {"BREATHE_ONE_LAW", EFFECT_BREATHE_ONE_LAW,      0, 0, 0, 0},
    {"BREATHE_ONE_BALANCE",EFFECT_BREATHE_ONE_BALANCE, 0, 0, 0, 0},
    {"BREATHE_ONE_SHINING",EFFECT_BREATHE_ONE_SHINING, 0, 0, 0, 0},

    /* Offense: Other                               Lv    T   R  Bias */
    {"DISPEL_EVIL",     EFFECT_DISPEL_EVIL,         50, 200,  2, BIAS_PRIESTLY | BIAS_LAW},
    {"DISPEL_EVIL_HERO",EFFECT_DISPEL_EVIL_HERO,    60, 250,  3, BIAS_PRIESTLY},
    {"DISPEL_GOOD",     EFFECT_DISPEL_GOOD,         50, 150,  1, BIAS_NECROMANTIC},
    {"DISPEL_LIFE",     EFFECT_DISPEL_LIFE,         55, 200,  1, BIAS_NECROMANTIC},
    {"DISPEL_DEMON",    EFFECT_DISPEL_DEMON,        60, 200,  2, BIAS_LAW},
    {"DISPEL_UNDEAD",   EFFECT_DISPEL_UNDEAD,       60, 200,  2, BIAS_PRIESTLY},
    {"DISPEL_MONSTERS", EFFECT_DISPEL_MONSTERS,     70, 250,  8, 0},
    {"DRAIN_LIFE",      EFFECT_DRAIN_LIFE,          40, 100,  2, BIAS_NECROMANTIC},
    {"STAR_BALL",       EFFECT_STAR_BALL,           80, 900, 64, BIAS_LAW},
    {"ROCKET",          EFFECT_ROCKET,              70, 200,  8, BIAS_DEMON},
    {"MANA_STORM",      EFFECT_MANA_STORM,          80, 250,  8, BIAS_MAGE},
    {"CONFUSING_LITE",  EFFECT_CONFUSING_LITE,      60, 100,  6, BIAS_CHAOS},
    {"ARROW",           EFFECT_ARROW,               30, 100,  2, BIAS_RANGER | BIAS_ARCHER},
    {"WRATH_OF_GOD",    EFFECT_WRATH_OF_GOD,        80, 250, 32, BIAS_LAW},
    {"METEOR",          EFFECT_METEOR,              55, 150,  8, 0},
    {"HOLINESS",        EFFECT_HOLINESS,            50, 250,  8, BIAS_PRIESTLY | BIAS_LAW},
    {"STARBURST",       EFFECT_STARBURST,           70, 500, 32, BIAS_LAW},
    {"DARKNESS_STORM",  EFFECT_DARKNESS_STORM,      70, 500, 32, BIAS_NECROMANTIC},
    {"PESTICIDE",       EFFECT_PESTICIDE,           10,   5,  8, 0},

    /* Misc                                         Lv    T   R  Bias */
    {"POLY_SELF",       EFFECT_POLY_SELF,           20, 500,  1, BIAS_CHAOS},
    {"ANIMATE_DEAD",    EFFECT_ANIMATE_DEAD,        25, 100,  1, BIAS_NECROMANTIC},
    {"SCARE_MONSTERS",  EFFECT_SCARE_MONSTERS,      20, 100,  1, BIAS_NECROMANTIC},
    {"SLEEP_MONSTERS",  EFFECT_SLEEP_MONSTERS,      25, 100,  1, BIAS_ROGUE},
    {"SLOW_MONSTERS",   EFFECT_SLOW_MONSTERS,       25, 100,  1, BIAS_RANGER},
    {"STASIS_MONSTERS", EFFECT_STASIS_MONSTERS,     50, 250,  8, BIAS_MAGE},
    {"CONFUSE_MONSTERS",EFFECT_CONFUSE_MONSTERS,    25, 100,  1, BIAS_CHAOS},
    {"FISHING",         EFFECT_FISHING,             10,   0,  0, 0},
    {"PIERCING_SHOT",   EFFECT_PIERCING_SHOT,       30, 100,  0, BIAS_ARCHER},
    {"CHARGE",          EFFECT_CHARGE,              15, 100,  0, 0},
    {"WALL_BUILDING",   EFFECT_WALL_BUILDING,       90, 750,  0, 0},
    {"SLEEP_MONSTER",   EFFECT_SLEEP_MONSTER,        5, 100,  1, 0},
    {"SLOW_MONSTER",    EFFECT_SLOW_MONSTER,         5, 100,  1, 0},
    {"CONFUSE_MONSTER", EFFECT_CONFUSE_MONSTER,      5, 100,  1, 0},
    {"SCARE_MONSTER",   EFFECT_SCARE_MONSTER,       10, 100,  1, 0},
    {"POLYMORPH",       EFFECT_POLYMORPH,           15, 100,  2, BIAS_CHAOS},
    {"STARLITE",        EFFECT_STARLITE,            20, 100,  2, 0},
    {"NOTHING",         EFFECT_NOTHING,              1,   1,  0, 0},
    {"ENDLESS_QUIVER",  EFFECT_ENDLESS_QUIVER,      50, 150,  0, BIAS_ARCHER},

    /* Bad Effects                                  Lv    T   R  Bias */
    {"AGGRAVATE",       EFFECT_AGGRAVATE,           10, 100,  1, BIAS_DEMON},
    {"HEAL_MONSTER",    EFFECT_HEAL_MONSTER,         2,  50,  0, 0},
    {"HASTE_MONSTER",   EFFECT_HASTE_MONSTER,       20, 100,  0, 0},
    {"HASTE_MONSTERS",  EFFECT_HASTE_MONSTERS,      10,  50,  0, 0},
    {"CLONE_MONSTER",   EFFECT_CLONE_MONSTER,       15, 100,  0, 0},
    {"DARKNESS",        EFFECT_DARKNESS,             5,  10,  0, 0},
    {"SUMMON_ANGRY_MONSTERS",
                        EFFECT_SUMMON_ANGRY_MONSTERS,10,200,  0, 0},
    {"SLOWNESS",        EFFECT_SLOWNESS,             5,  10,  0, 0},

    /* Specific Artifacts                           Lv    T   R  Bias */
    {"JEWEL",           EFFECT_JEWEL,                0,   0,  0, 0},
    {"HERMES",          EFFECT_HERMES,               0,   0,  0, 0},
    {"ARTEMIS",         EFFECT_ARTEMIS,              0,   0,  0, 0},
    {"DEMETER",         EFFECT_DEMETER,              0,   0,  0, 0},
    {"EYE_VECNA",       EFFECT_EYE_VECNA,            0,   0,  0, 0},
    {"ONE_RING",        EFFECT_ONE_RING,             0,   0,  0, 0},
    {"BLADETURNER",     EFFECT_BLADETURNER,          0,   0,  0, 0},
    {"MITO_KOUMON",     EFFECT_MITO_KOUMON,          0,   0,  0, 0},
    {"BLOODY_MOON",     EFFECT_BLOODY_MOON,          0,   0,  0, 0},
    {"SACRED_KNIGHTS",  EFFECT_SACRED_KNIGHTS,       0,   0,  0, 0},
    {"GONG",            EFFECT_GONG,                 0,   0,  0, 0},
    {"MURAMASA",        EFFECT_MURAMASA,             0,   0,  0, 0},
    {"EXPERTSEXCHANGE", EFFECT_EXPERTSEXCHANGE,      0,   0,  0, 0},
    {"EYE_HYPNO",       EFFECT_EYE_HYPNO,            0,   0,  0, 0},
    {"STUNNING_KICK",   EFFECT_STUNNING_KICK,        0,   0,  0, 0},
    {"RAMA_ARROW",      EFFECT_RAMA_ARROW,           0,   0,  0, 0},
    {"UNFOCUS_RAGE",    EFFECT_UNFOCUS_RAGE,         0,   0,  0, 0},

    {0}
};

_effect_info_ptr _get_effect_info(int type)
{
    int i;
    for (i = 0; ; i++)
    {
        _effect_info_ptr e = &_effect_info[i];
        if (!e->type) break;
        if (e->type == type) return e;
    }
    return NULL;
}

bool effect_is_known(int type)
{
    _effect_info_ptr e = _get_effect_info(type);
    if (e)
        return e->known;
    return FALSE;
}

bool effect_learn(int type)
{
    _effect_info_ptr e = _get_effect_info(type);
    if (e && !e->known)
    {
        e->known = TRUE;
        return TRUE;
    }
    return FALSE;
}

int effect_parse_type(cptr type)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].text) break;
        if (streq(type, _effect_info[i].text))
            return _effect_info[i].type;
    }
    return EFFECT_NONE;
}

errr effect_parse(char *line, effect_t *effect) /* LITE_AREA:<Lvl>:<Timeout>:<Extra> */
{
    char *tokens[5];
    int   num = tokenize(line, 5, tokens, 0);
    int   i;

    if (num < 1) return PARSE_ERROR_TOO_FEW_ARGUMENTS;

    WIPE(effect, effect_t);

    switch (num)
    {
    case 4: effect->extra = atoi(tokens[3]);
    case 3: effect->cost = atoi(tokens[2]);
    case 2: effect->power = atoi(tokens[1]);
            effect->difficulty = effect->power;
    case 1:
        for (i = 0; ; i++)
        {
            if (!_effect_info[i].text) break;
            if (streq(tokens[0], _effect_info[i].text))
            {
                effect->type = _effect_info[i].type;
                break;
            }
        }
    }
    if (!effect->type) return 1;
    return 0;
}

static int _choose_random_p(effect_p p)
{
    int i, n;
    int tot = 0;

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (!_effect_info[i].rarity) continue;
        if (p && !p(_effect_info[i].type)) continue;

        tot += MAX(255 / _effect_info[i].rarity, 1);
    }

    if (!tot) return -1;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (!_effect_info[i].rarity) continue;
        if (p && !p(_effect_info[i].type)) continue;

        n -= MAX(255 / _effect_info[i].rarity, 1);
        if (n <= 0) return i;
    }
    return -1;
}

static int _choose_random(int bias)
{
    int i, n;
    int tot = 0;

/*  if (one_in_(3)) bias = 0; */

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].level < object_level / 3) continue;
        if (bias && !(_effect_info[i].bias & bias)) continue;
        if (!_effect_info[i].rarity) continue;

        tot += MAX(255 / _effect_info[i].rarity, 1);
    }

    if (!tot) return -1;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].level < object_level / 3) continue;
        if (bias && !(_effect_info[i].bias & bias)) continue;
        if (!_effect_info[i].rarity) continue;

        n -= MAX(255 / _effect_info[i].rarity, 1);
        if (n <= 0) return i;
    }
    return -1;
}

static void _add_index(object_type *o_ptr, int index)
{
    if (index >= 0)
    {
        o_ptr->activation.type = _effect_info[index].type;
        o_ptr->activation.power = _effect_info[index].level;
        o_ptr->activation.difficulty = _effect_info[index].level;
        o_ptr->activation.cost = _effect_info[index].cost;
        o_ptr->activation.extra = 0;
        o_ptr->timeout = 0;
        add_flag(o_ptr->flags, OF_ACTIVATE); /* for object lore */
    }
}

bool effect_add_random_p(object_type *o_ptr, effect_p p)
{
    int i = _choose_random_p(p);
    if (i >= 0)
    {
        _add_index(o_ptr, i);
        return TRUE;
    }
    return FALSE;
}

bool effect_add_random(object_type *o_ptr, int bias)
{
    int i = _choose_random(bias);
    if (i >= 0)
    {
        _add_index(o_ptr, i);
        return TRUE;
    }
    return FALSE;
}

bool effect_add(object_type *o_ptr, int type)
{
    int i;
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].type == type)
        {
            _add_index(o_ptr, i);
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************
 * Redoing Devices (Wands, Staves and Rods)
 ***********************************************************************/

#define _DROP_GOOD       0x0001
#define _DROP_GREAT      0x0002
#define _NO_DESTROY      0x0004
#define _STOCK_TOWN      0x0008
#define _COMMON          0x0010
#define _RARE            0x0020

device_effect_info_t wand_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_BOLT_MISSILE,           1,   3,     1,  20,    10,  0, _STOCK_TOWN},
    {EFFECT_HEAL_MONSTER,           2,   3,     1,  50,     0,  0, 0},
    {EFFECT_BEAM_LITE_WEAK,         2,   3,     1,  20,    10,  0, _STOCK_TOWN},
    {EFFECT_BALL_POIS,              5,   4,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_SLEEP_MONSTER,          5,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_SLOW_MONSTER,           5,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_CONFUSE_MONSTER,        5,   5,     1,  25,    33,  0, _STOCK_TOWN},
    {EFFECT_SCARE_MONSTER,          7,   5,     1,  20,    33,  0, _STOCK_TOWN},
    {EFFECT_STONE_TO_MUD,          10,   5,     1,   0,    10,  0, _COMMON},
    {EFFECT_POLYMORPH,             12,   6,     1,  30,     0,  0, 0},
    {EFFECT_BOLT_COLD,             12,   7,     1,  30,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_ELEC,             15,   7,     1,  30,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_ACID,             17,   8,     1,  35,    33,  0, _STOCK_TOWN},
    {EFFECT_BOLT_FIRE,             19,   9,     1,  35,    33,  0, _STOCK_TOWN},
    {EFFECT_HASTE_MONSTER,         20,   3,     1,  50,     0,  0, 0},
    {EFFECT_TELEPORT_AWAY,         20,  10,     1,   0,    10,  0, _COMMON},
    {EFFECT_DESTROY_TRAPS,         20,  10,     1,   0,    10,  0, 0},
    {EFFECT_CHARM_MONSTER,         25,  11,     1,  50,    33,  0, 0},
    {EFFECT_BALL_COLD,             26,   5,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ELEC,             28,   5,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ACID,             29,   7,     1,   0,    50, 10, 0},
    {EFFECT_BALL_FIRE,             30,   8,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_WATER,            30,   9,     1,   0,    50, 10, 0},
    {EFFECT_DRAIN_LIFE,            32,  20,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_PLASMA,           38,  10,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_ICE,              40,  11,     1,   0,    50, 10, 0},
    {EFFECT_ARROW,                 45,  13,     1,   0,    50, 10, 0},
    {EFFECT_BALL_NEXUS,            47,  14,     1,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BREATHE_COLD,          50,  15,     1,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BREATHE_FIRE,          50,  16,     1,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
	{EFFECT_BREATHE_WATER,         50,  16,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BEAM_GRAVITY,          55,  32,     2,   0,    33,  0, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_METEOR,                55,  32,     2,   0,    50, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BREATHE_ONE_MULTIHUED, 60,  17,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_GENOCIDE_ONE,          65,  35,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BALL_WATER,            70,  20,     2,   0,    60, 10, _DROP_GOOD | _NO_DESTROY},
    {EFFECT_BALL_DISINTEGRATE,     75,  20,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_ROCKET,                85,  20,     3,   0,    70, 20, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_WALL_BUILDING,        100,  50,    16,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {0}
};

device_effect_info_t rod_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_PESTICIDE,              1,   7,     1,  30,    10,  0, 0},
    {EFFECT_DETECT_TRAPS,           5,   9,     1,  30,    10,  0, 0},
    {EFFECT_LITE_AREA,             10,  10,     1,  40,    10,  0, 0},
    {EFFECT_DETECT_DOOR_STAIRS,    12,  10,     1,  40,    10,  0, 0},
    {EFFECT_DETECT_MONSTERS,       15,  10,     1,  40,    10,  0, 0},
    {EFFECT_BEAM_ELEC,             17,   8,     1,  50,    33,  0, 0},
    {EFFECT_BEAM_COLD,             19,   8,     1,  50,    33,  0, 0},
    {EFFECT_BEAM_FIRE,             21,   9,     1,  60,    33,  0, 0},
    {EFFECT_BEAM_ACID,             23,   9,     1,  60,    33,  0, 0},
    {EFFECT_BEAM_LITE,             25,  12,     2,   0,    50, 10, 0},
    {EFFECT_RECALL,                27,  15,     1,   0,    10,  0, 0},
    {EFFECT_DETECT_ALL,            30,  17,     2,   0,    10,  0, _COMMON},
    {EFFECT_ESCAPE,                30,  20,     1,   0,    10,  0, 0},
    {EFFECT_BEAM_CHAOS,            32,  12,     2,  60,    33,  0, 0},
    {EFFECT_BEAM_SOUND,            32,  12,     2,  70,    33,  0, 0},
    {EFFECT_CLARITY,               35,  15,     3,  80,    33,  0, _DROP_GOOD},
    {EFFECT_BALL_ELEC,             40,  13,     1,   0,    50, 10, 0},
    {EFFECT_BALL_COLD,             40,  14,     1,   0,    50, 10, 0},
    {EFFECT_BALL_FIRE,             42,  15,     1,   0,    50, 10, 0},
    {EFFECT_BALL_ACID,             44,  16,     1,   0,    50, 10, 0},
    {EFFECT_BOLT_MANA,             45,  17,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BALL_NETHER,           45,  18,     1,   0,    50, 10, 0},
    {EFFECT_BALL_DISEN,            47,  19,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_ENLIGHTENMENT,         50,  33,     2,   0,    10,  0, _COMMON},
    {EFFECT_BALL_SOUND,            52,  22,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_BEAM_DISINTEGRATE,     60,  37,     2,   0,    33,  0, _DROP_GOOD},
    {EFFECT_SPEED_HERO,            70,  40,     2,   0,    10,  0, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_GREAT_CLARITY,         75,  60,     4,   0,    50, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_HEAL_CURING_HERO,      80,  60,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_RESTORING,             80,  60,     3,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_BALL_MANA,             80,  24,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BALL_SHARDS,           80,  25,     2,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BALL_CHAOS,            85,  27,     3,   0,    70, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_CLAIRVOYANCE,          90, 100,     3,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_BALL_LITE,             95,  27,     3,   0,    70, 10, _DROP_GOOD | _DROP_GREAT},
    {0}
};

device_effect_info_t staff_effect_table[] =
{
    /*                            Lvl Cost Rarity  Max  Difficulty Flags */
    {EFFECT_NOTHING,                1,   1,     0,   0,     0,  0, 0},
    {EFFECT_DARKNESS,               1,   3,     1,  15,     0,  0, 0},
    {EFFECT_LITE_AREA,              1,   3,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_GOLD,            5,   4,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_OBJECTS,         5,   4,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_INVISIBLE,       5,   4,     1,  30,    10,  0, 0},
    {EFFECT_DETECT_TRAPS,           5,   5,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_DOOR_STAIRS,     5,   5,     1,  30,    10,  0, _STOCK_TOWN},
    {EFFECT_DETECT_EVIL,            7,   5,     1,  30,    10,  0, 0},
    {EFFECT_HASTE_MONSTERS,        10,   5,     1,  30,    50, 10, 0},
    {EFFECT_SUMMON_ANGRY_MONSTERS, 10,   5,     1,  30,    50, 10, 0},
    {EFFECT_IDENTIFY,              10,   4,     1,   0,    10,  0, _STOCK_TOWN | _COMMON},
    {EFFECT_SLEEP_MONSTERS,        10,   6,     1,  40,    33,  0, 0},
    {EFFECT_SLOW_MONSTERS,         10,   6,     1,  40,    33,  0, 0},
    {EFFECT_CONFUSE_MONSTERS,      15,   8,     1,  40,    33,  0, 0},
    {EFFECT_TELEPORT,              20,  10,     1,   0,    10,  0, 0},
    {EFFECT_ENLIGHTENMENT,         20,  10,     1,  70,    10,  0, _STOCK_TOWN},
    {EFFECT_STARLITE,              20,  10,     1,  50,    33,  0, 0},
    {EFFECT_EARTHQUAKE,            20,  10,     2,   0,    10,  0, 0},
    {EFFECT_HEAL,                  20,  10,     2,  70,    33,  0,  _COMMON}, /* Cure Wounds for ~50hp */
    {EFFECT_CURING,                25,  12,     1,  70,    10,  0, 0}, /* Curing no longer heals */
    {EFFECT_SUMMON_HOUNDS,         27,  25,     2,   0,    10,  0, 0},
    {EFFECT_SUMMON_HYDRAS,         27,  25,     3,   0,    10,  0, 0},
    {EFFECT_SUMMON_ANTS,           27,  20,     2,   0,    10,  0, 0},
    {EFFECT_PROBING,               30,  15,     3,  70,    10,  0, 0},
    {EFFECT_TELEPATHY,             30,  16,     2,   0,    10,  0, 0},
    {EFFECT_SUMMON_MONSTERS,       32,  30,     2,   0,    33,  0, 0},
    {EFFECT_ANIMATE_DEAD,          35,  17,     2,  70,    33,  0, 0},
    {EFFECT_SLOWNESS,              40,  19,     3,  70,    50, 10, 0},
    {EFFECT_SPEED,                 40,  19,     2,   0,    10,  0, _COMMON},
    {EFFECT_IDENTIFY_FULL,         40,  20,     3,   0,    10,  0, _COMMON},
    {EFFECT_REMOVE_CURSE,          40,  20,     4,   0,    10,  0, 0},
    {EFFECT_DISPEL_DEMON,          45,  10,     2,   0,    50, 10, 0},
    {EFFECT_DISPEL_UNDEAD,         45,  10,     2,   0,    50, 10, 0},
    {EFFECT_DISPEL_LIFE,           50,  12,     3,   0,    50, 10, 0},
    {EFFECT_DISPEL_EVIL,           55,  13,     3,   0,    50, 10, 0},
    {EFFECT_DISPEL_MONSTERS,       55,  15,     5,   0,    50, 10, 0},
    {EFFECT_DESTRUCTION,           50,  15,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_CONFUSING_LITE,        55,  26,     2,   0,    50, 10, _DROP_GOOD},
    {EFFECT_HEAL_CURING,           55,  10,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT},
    {EFFECT_BANISH_EVIL,           60,  31,     2,   0,    33,  0, _DROP_GOOD},
    {EFFECT_BANISH_ALL,            70,  32,     3,   0,    33,  0, _DROP_GOOD},
    {EFFECT_MANA_STORM,            85,  10,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_STARBURST,             85,  12,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_DARKNESS_STORM,        85,  12,     3,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY},
    {EFFECT_GENOCIDE,              90,  50,     8,   0,    60, 10, _DROP_GOOD | _DROP_GREAT | _RARE},
    {EFFECT_RESTORE_MANA,         100, 100,    16,   0,     0,  0, _DROP_GOOD | _DROP_GREAT | _NO_DESTROY | _RARE},
    {0}
};

/* MAX(1, _rand_normal(1, 10)) is probablematic. Think about why! */
static int _bounds_check(int value, int min, int max)
{
    int result = value;
    if (result < min)
        result = min;
    if (result > max)
        result = max;
    return result;
}

/* I like to set my deviation as a percentage of the mean.
   Also, the scaling and rounding makes the distribution no
   longer normal, but I like small casting costs to vary as
   well (e.g. _rand_normal(1, 10) can give 2 (about 3% of the
   time) whereas randnor(1, 0.1), even if legal, would not. */
static int _rand_normal(int mean, int pct)
{
    int result = 0;
    int m = mean * 10;
    int d = m * pct / 100;
    int r = randnor(m, d);

    assert(mean >= 0);
    assert(pct >= 0);

    result = r/10;
    if (randint0(10) < (r%10))
        result++;

    return result;
}
/*static int _rand_normal_hi(int mean, int pct)
{
    int result = _rand_normal(mean, pct);
    if (result < mean)
        result = mean + (mean - result);
    return result;
}*/

static int _effect_rarity(device_effect_info_ptr entry, int level)
{
    int r = entry->rarity;
    if (!r) return 0;
    if (entry->max_depth && entry->max_depth < level) return 0;
    if (entry->flags & _RARE)
    {
        int n = entry->counts.found + entry->counts.bought;
        while (n--)
            r *= 2;
    }
    else if (level > entry->level)
    {
        int d = level - entry->level;
        int spread = entry->max_depth ? entry->max_depth - entry->level : 100 - entry->level;
        int n = (entry->flags & _COMMON) ? spread/2 : spread*2/7;
        while (d >= n)
        {
            r *= 2;
            d -= n;
        }
        r += d*r/n;
    }
    return r;
}

static void _device_adjust_activation_difficulty(object_type *o_ptr, device_effect_info_ptr entry)
{
    int d = 10*(o_ptr->activation.power - o_ptr->activation.difficulty);
    int b = entry->difficulty_base * d / 100;

    b += randint0(entry->difficulty_xtra * d / 100);
    o_ptr->activation.difficulty += b/10;
    if (randint0(10) < b%10)
        o_ptr->activation.difficulty++;
    if (o_ptr->activation.difficulty > o_ptr->activation.power) /* paranoia */
        o_ptr->activation.difficulty = o_ptr->activation.power;
}

static void _device_pick_effect(object_type *o_ptr, device_effect_info_ptr table, int level, int mode)
{
    int i, n;
    int tot = 0;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        int                    rarity;

        if (!entry->type) break;

        entry->prob = 0;
        rarity = _effect_rarity(entry, level);

        if (!rarity) continue;
        if (entry->level > device_level(o_ptr)) continue;
        if ((mode & AM_GOOD) && !(entry->flags & _DROP_GOOD)) continue;
        if ((mode & AM_GREAT) && !(entry->flags & _DROP_GREAT)) continue;
        if ((mode & AM_STOCK_TOWN) && !(entry->flags & _STOCK_TOWN)) continue;
		if (easy_id && entry->type == EFFECT_IDENTIFY_FULL) continue;
		if (easy_lore && entry->type == EFFECT_PROBING) continue;

        entry->prob = 64 / rarity;
        tot += entry->prob;
    }

    if (!tot) return;
    n = randint1(tot);

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];

        if (!entry->type) break;
        if (!entry->prob) continue;

        n -= entry->prob;
        if (n <= 0)
        {
            int cost;

            o_ptr->activation.type = entry->type;

            /* Power is the casting level of the device and determines damage or power of the effect.
               Difficulty is the level of the effect, and determines the fail rate of the effect.
               Difficulty is set to the base level of the effect, and then scaled based on the
               power of the effect using the difficulty_base and xtra percentages. */
            o_ptr->activation.power = device_level(o_ptr);
            o_ptr->activation.difficulty = entry->level;
            if (o_ptr->activation.power > o_ptr->activation.difficulty)
            {
                _device_adjust_activation_difficulty(o_ptr, entry);
            }

            cost = entry->cost;
            cost += effect_cost_extra(&o_ptr->activation);
            o_ptr->activation.cost = _bounds_check(_rand_normal(cost, 5), 1, 1000);

            if (entry->flags & _NO_DESTROY)
            {
                add_flag(o_ptr->flags, OF_IGNORE_ACID);
                add_flag(o_ptr->flags, OF_IGNORE_ELEC);
                add_flag(o_ptr->flags, OF_IGNORE_FIRE);
                add_flag(o_ptr->flags, OF_IGNORE_COLD);
            }

            return;
        }
    }
}

static bool _is_valid_device(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
    case TV_WAND:
    case TV_ROD:
    case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}

/* Initialize a device with a random effect for monster drops, dungeon objects, etc */
bool device_init(object_type *o_ptr, int level, int mode)
{
    if (!_is_valid_device(o_ptr))
        return FALSE;

    if (!(mode & (AM_STOCK_TOWN | AM_STOCK_BM)) && one_in_(GREAT_OBJ))
    {
        int boost = level;
        if (boost < 20)
            boost = 20;
        level += rand_range(boost/4, boost/2);
    }

    if (level > 100)
        level = 100;

    /* device_level
     * 90%+-10% means 84.13% <= level (modulo rounding, of course)
     * 95%+-10% means ~70% <= level. So ~30% at or *above* level.
     * See how generous I've become ;) */
    o_ptr->xtra3 = _bounds_check(_rand_normal(level*95/100, 10), 1, 100);

    switch (o_ptr->tval)
    {
    case TV_WAND:
        _device_pick_effect(o_ptr, wand_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3, 15), o_ptr->activation.cost*4, 1000);
        break;
    case TV_ROD:
        _device_pick_effect(o_ptr, rod_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp: rods have fewer sp but regen more quickly. */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3/2, 15), o_ptr->activation.cost*2, 1000);
        break;
    case TV_STAFF:
        _device_pick_effect(o_ptr, staff_effect_table, o_ptr->xtra3, mode);
        if (!o_ptr->activation.type)
            return FALSE;
        /* device_max_sp */
        o_ptr->xtra4 = _bounds_check(_rand_normal(3*o_ptr->xtra3, 15), o_ptr->activation.cost*4, 1000);
        break;
    }
    /* device_sp */
    o_ptr->xtra5 = _bounds_check(_rand_normal(o_ptr->xtra4/2, 25), o_ptr->activation.cost, o_ptr->xtra4);
    o_ptr->xtra5 *= 100; /* scale current sp by 100 for smoother regeneration */

    add_flag(o_ptr->flags, OF_ACTIVATE);

    /* cf obj_create_device in ego.c for egos */
    return TRUE;
}

static device_effect_info_ptr _device_find_effect(device_effect_info_ptr table, int effect)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];

        if (!entry->type) break;
        if (entry->type == effect) return entry;
    }

    return NULL;
}

bool device_is_valid_effect(int tval, int effect)
{
    switch (tval)
    {
    case TV_WAND:
        return _device_find_effect(wand_effect_table, effect) != NULL;
    case TV_ROD:
        return _device_find_effect(rod_effect_table, effect) != NULL;
    case TV_STAFF:
        return _device_find_effect(staff_effect_table, effect) != NULL;
    }
    return FALSE;
}

/* Initialize a device with a fixed effect. This is useful for birth objects, quest rewards, etc */
bool device_init_fixed(object_type *o_ptr, int effect)
{
    device_effect_info_ptr e_ptr = NULL;

    if (!_is_valid_device(o_ptr))
        return FALSE;

    switch (o_ptr->tval)
    {
    case TV_WAND:
        e_ptr = _device_find_effect(wand_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    case TV_ROD:
        e_ptr = _device_find_effect(rod_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    case TV_STAFF:
        e_ptr = _device_find_effect(staff_effect_table, effect);
        if (!e_ptr)
            return FALSE;
        break;
    }

    o_ptr->xtra3 = e_ptr->level;
    if (o_ptr->level < 0) /* Level hack */
    {
        o_ptr->xtra3 = MAX(e_ptr->level, MIN(e_ptr->level - o_ptr->level, (e_ptr->max_depth ? e_ptr->max_depth : 100)));
        o_ptr->level = 0;
    }

    if (o_ptr->xtra3 < 7)
        o_ptr->xtra3 = 7;

    o_ptr->activation.type = e_ptr->type;
    o_ptr->activation.power = o_ptr->xtra3;
    o_ptr->activation.difficulty = e_ptr->level;
    if (o_ptr->activation.power > o_ptr->activation.difficulty)
    {
        _device_adjust_activation_difficulty(o_ptr, e_ptr);
    }
    
    o_ptr->activation.cost = e_ptr->cost + effect_cost_extra(&o_ptr->activation);

    if (o_ptr->tval == TV_ROD)
    {
        o_ptr->xtra4 = _bounds_check(3 * o_ptr->xtra3 / 2, o_ptr->activation.cost*2, 1000);
    }
    else
    {
        o_ptr->xtra4 = _bounds_check(3 * o_ptr->xtra3, o_ptr->activation.cost*4, 1000);
    }
    o_ptr->xtra5 = o_ptr->xtra4; /* Fully Charged */
    o_ptr->xtra5 *= 100; /* scale current sp by 100 for smoother regeneration */

    if (e_ptr->flags & _NO_DESTROY)
    {
        add_flag(o_ptr->flags, OF_IGNORE_ACID);
        add_flag(o_ptr->flags, OF_IGNORE_ELEC);
        add_flag(o_ptr->flags, OF_IGNORE_FIRE);
        add_flag(o_ptr->flags, OF_IGNORE_COLD);
    }

    add_flag(o_ptr->flags, OF_ACTIVATE);

    return TRUE;
}

/* TODO: See wiz_obj.c for reliance on xtra fields */
int device_level(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra3;
    return 0;
}

int device_sp(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra5 / 100;
    return 0;
}

int device_charges(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr) && o_ptr->activation.cost)
        return  device_sp(o_ptr) / o_ptr->activation.cost;
    return 0;
}

int device_max_charges(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr) && o_ptr->activation.cost)
        return  device_max_sp(o_ptr) / o_ptr->activation.cost;
    return 0;
}

void device_decrease_sp(object_type *o_ptr, int amt)
{
    if (_is_valid_device(o_ptr))
    {
        int charges = device_charges(o_ptr);
        o_ptr->xtra5 -= amt * 100;
        if (o_ptr->xtra5 < 0)
            o_ptr->xtra5 = 0;
        if (device_charges(o_ptr) != charges)
            p_ptr->window |= PW_INVEN;
    }
}

void device_increase_sp(object_type *o_ptr, int amt)
{
    if (_is_valid_device(o_ptr))
    {
        int charges = device_charges(o_ptr);
        o_ptr->xtra5 += amt * 100;
        if (o_ptr->xtra5 > o_ptr->xtra4 * 100)
            o_ptr->xtra5 = o_ptr->xtra4 * 100;
        if (device_charges(o_ptr) != charges)
            p_ptr->window |= PW_INVEN;
    }
}

bool device_is_fully_charged(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        if (o_ptr->xtra5 == o_ptr->xtra4 * 100)
            return TRUE;
        else
            return FALSE;
    }
    return FALSE; /* ?? */
}

/* Note: Rods fire every 10 game turns; wands and staves fire every 100 game turns.*/
void device_regen_sp_aux(object_type *o_ptr, int per_mill)
{
    if (!device_is_fully_charged(o_ptr))
    {
        int div = 1000;
        int amt = o_ptr->xtra4 * 100 * per_mill;
        int charges = device_charges(o_ptr);

        o_ptr->xtra5 += amt / div;
        if (randint0(div) < (amt % div))
            o_ptr->xtra5++;

        if (o_ptr->xtra5 > o_ptr->xtra4 * 100)
            o_ptr->xtra5 = o_ptr->xtra4 * 100;

        if (device_is_fully_charged(o_ptr))
            recharged_notice(o_ptr, '!');

        if (device_charges(o_ptr) != charges)
        {
            if (!charges) recharged_notice(o_ptr, '1');
            p_ptr->window |= PW_INVEN;
        }
    }
}

void device_regen_sp(object_type *o_ptr, int base_per_mill)
{
    int  per_mill = base_per_mill;
    u32b flgs[OF_ARRAY_SIZE];

    if (!_is_valid_device(o_ptr))
        return;

    if (device_is_fully_charged(o_ptr))
        return;

    if (devicemaster_is_speciality(o_ptr))
        per_mill += base_per_mill;

    obj_flags(o_ptr, flgs);
    if (have_flag(flgs, OF_REGEN))
        per_mill += o_ptr->pval * base_per_mill;

    device_regen_sp_aux(o_ptr, per_mill);
}

int device_max_sp(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
        return o_ptr->xtra4;
    return 0;
}

int device_value(object_type *o_ptr, int options)
{
    int  result = 0;
    u32b flgs[OF_ARRAY_SIZE];
    int  pval = 0;

    if (!_is_valid_device(o_ptr))
        return 0;

    switch (o_ptr->tval)
    {
    case TV_WAND: result = 100; break;
    case TV_STAFF: result = 100; break;
    case TV_ROD: result = 250; break;
    }

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        pval = o_ptr->pval;
        if (o_ptr->activation.type)
        {
            int value = effect_value(&o_ptr->activation);

            if (o_ptr->activation.cost)
            {
                int base_charges = 40; /* scaled by 10 */
                int charges = device_max_sp(o_ptr) * 10 / o_ptr->activation.cost;

                if (o_ptr->tval == TV_ROD)
                {
                    base_charges /= 2;
                    value = value * 150 / 100; /* rods should value more than wands (durable+fast recharge) */
                }

                value = value * charges / base_charges;
            }
            result += value;
        }
    }

    if (options & COST_REAL)
        obj_flags(o_ptr, flgs);
    else
        obj_flags_known(o_ptr, flgs);

    if ((options & COST_REAL) || object_is_known(o_ptr))
    {
        if (o_ptr->name2 == EGO_DEVICE_RESISTANCE) /* I don't want artifacts to get an extra boost for TR_IGNORE_* */
            result += result * 25 / 100;
    }
    if (have_flag(flgs, OF_REGEN))
        result += result * 20 * pval / 100;

    if (have_flag(flgs, OF_EASY_SPELL))
        result += result * 10 * pval / 100;

    if (have_flag(flgs, OF_DEVICE_POWER))
        result += result * 20 * pval / 100;

    if (have_flag(flgs, OF_HOLD_LIFE))
        result += result * 40 / 100;

    if (have_flag(flgs, OF_SPEED))
        result += result * 25 * pval / 100;

    return result;
}

/* Statistics */
device_effect_info_ptr device_get_effect_info(int tval, int effect)
{
    switch (tval)
    {
    case TV_WAND:
        return _device_find_effect(wand_effect_table, effect);
        break;
    case TV_ROD:
        return _device_find_effect(rod_effect_table, effect);
        break;
    case TV_STAFF:
        return _device_find_effect(staff_effect_table, effect);
        break;
    }
    return NULL;
}

static void _device_stats_reset_imp(device_effect_info_ptr table)
{
    int i;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        WIPE(&entry->counts, counts_t);
    }
}

void device_stats_reset(void)
{
    _device_stats_reset_imp(wand_effect_table);
    _device_stats_reset_imp(rod_effect_table);
    _device_stats_reset_imp(staff_effect_table);
}

static void _device_stats_save_imp(savefile_ptr file, device_effect_info_ptr table)
{
    int i, ct = 0;

    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        ct++;
    }

    savefile_write_s32b(file, ct);
    for (i = 0; ; i++)
    {
        device_effect_info_ptr entry = &table[i];
        if (!entry->type) break;
        savefile_write_s32b(file, entry->type);

        savefile_write_s32b(file, entry->counts.generated);
        savefile_write_s32b(file, entry->counts.found);
        savefile_write_s32b(file, entry->counts.bought);
        savefile_write_s32b(file, entry->counts.used);
        savefile_write_s32b(file, entry->counts.destroyed);
    }

}

static void _device_stats_load_imp(savefile_ptr file, device_effect_info_ptr table)
{
    int ct, i;

    /* We reset since not every current table entry need exist in the savefile.
       In other words, code changes over time and I might add a new entry :) */
    _device_stats_reset_imp(table);

    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        int                     type = savefile_read_s32b(file);
        counts_t                counts;
        device_effect_info_ptr  entry = _device_find_effect(table, type);

        counts.generated = savefile_read_s32b(file);
        counts.found = savefile_read_s32b(file);
        counts.bought = savefile_read_s32b(file);
        counts.used = savefile_read_s32b(file);
        counts.destroyed = savefile_read_s32b(file);

        if (entry)
            entry->counts = counts;
    }
}

void device_stats_on_save(savefile_ptr file)
{
    int i, ct = 0;
    _device_stats_save_imp(file, wand_effect_table);
    _device_stats_save_imp(file, rod_effect_table);
    _device_stats_save_imp(file, staff_effect_table);

    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].known) ct++;
    }
    savefile_write_s32b(file, ct);
    for (i = 0; ; i++)
    {
        if (!_effect_info[i].type) break;
        if (_effect_info[i].known)
            savefile_write_s32b(file, _effect_info[i].type);
    }
}

void device_stats_on_load(savefile_ptr file)
{
    int i, ct;

    _device_stats_load_imp(file, wand_effect_table);
    _device_stats_load_imp(file, rod_effect_table);
    _device_stats_load_imp(file, staff_effect_table);

    ct = savefile_read_s32b(file);
    for (i = 0; i < ct; i++)
    {
        int type = savefile_read_s32b(file);
        _effect_info_ptr e = _get_effect_info(type);
        if (e)
            e->known = TRUE;
    }
}

void device_stats_on_find(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.found++;
    }
}

void device_stats_on_use(object_type *o_ptr, int num)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.used += num;
    }
}

void device_stats_on_destroy(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.destroyed++;
    }
}

void device_stats_on_purchase(object_type *o_ptr)
{
    if (_is_valid_device(o_ptr))
    {
        device_effect_info_ptr entry = device_get_effect_info(o_ptr->tval, o_ptr->activation.type);
        if (entry)
            entry->counts.bought++;
    }
}

static int _extra(effect_t *effect, int def)
{
    int result = effect->extra;
    if (!result)
        result = def;
    return result;
}

static int _boost(int value, int boost)
{
    return MAX(0, value * (100 + boost) / 100);
}

static int _avg_damroll(int dd, int ds)
{
    return dd * (ds + 1) / 2;
}

/* Device casting is non-linear in difficulty (cf design/devices.ods)
 * Yet device power (e.g. damage) is (or was?) linear. This is hardly fair! */
typedef struct { int w1, w2, w3; } _weights_t;
static _weights_t _weights(int w1, int w2, int w3)
{
    _weights_t w;
    w.w1 = w1;
    w.w2 = w2;
    w.w3 = w3;
    return w;
}
typedef struct { int lvl, max; } _level_t;
static _level_t _level_aux(int lvl, int max)
{
    _level_t l;
    l.lvl = lvl;
    l.max = max;
    return l;
}
static _level_t _level(int lvl)
{
    return _level_aux(lvl, 100);
}
static _level_t _level_offset(int lvl, int start)
{
    int l = MAX(0, lvl - start);
    return _level_aux(l, 100 - start);
}
static int _power_curve_aux(int amt, _level_t l, _weights_t w)
{
    int result = 0;
    int wt = w.w1 + w.w2 + w.w3;

    if (l.lvl == l.max)
        return amt;

    result += amt * l.lvl * w.w1 / (l.max*wt);
    result += amt * l.lvl * l.lvl * w.w2 / (l.max*l.max*wt);
    result += (amt * l.lvl * l.lvl / l.max) * l.lvl * w.w3 / (l.max*l.max*wt);

    return result;
}
static int _power_curve(int amt, int lvl)
{
    return _power_curve_aux(amt, _level(lvl), _weights(1, 1, 1));
}
static int _power_curve_offset(int amt, int lvl, int start)
{
    return _power_curve_aux(amt, _level_offset(lvl, start), _weights(1, 1, 1));
}

/************************************************************************
 * The Effects
 ***********************************************************************/
#define _BOOST(n) (_boost((n), boost))
cptr do_effect(effect_t *effect, int mode, int boost)
{
    bool name = (mode == SPELL_NAME);
    bool desc = (mode == SPELL_DESC);
    bool info = (mode == SPELL_INFO);
    bool cast = (mode == SPELL_CAST);
    bool value = (mode == SPELL_VALUE);
    bool color = (mode == SPELL_COLOR);
    bool cost = (mode == SPELL_COST_EXTRA);
    int  dir = 0;

    switch (effect->type)
    {
    case EFFECT_NONE:
        if (name) return "无";
        if (desc) return "它什么都不会做，完全没有效果。";
        if (cast)
        {
            msg_print("什么也没发生。");
            device_noticed = TRUE;
        }
        break;
    /* Detection */
    case EFFECT_LITE_AREA:
        if (name) return "照明";
        if (desc) return "它能永久照亮附近区域或当前房间。";
        if (info) return info_damage(2 + effect->power/20, _BOOST(15), 0);
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (lite_area(_BOOST(damroll(2 + effect->power/20, 15)), 3))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_LITE_MAP_AREA:
        if (name) return "魔法探知与照明";
        if (desc) return "它能为你附近的区域绘制地图，并照亮你当前的房间。";
        if (info) return info_damage(2 + effect->power/20, _BOOST(15), 0);
        if (value) return format("%d", 1300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            map_area(DETECT_RAD_MAP);
            lite_area(_BOOST(damroll(2 + effect->power/20, 15)), 3);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_ENLIGHTENMENT:
        if (name) return "启示";
        if (desc) return "它能为你附近的区域绘制地图。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            map_area(DETECT_RAD_MAP);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_CLAIRVOYANCE:
        if (name) return "透视";
        if (desc) return "它能为整个楼层绘制地图、永久照亮并探测所有物品。";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_TRAPS:
        if (name) return "探测陷阱";
        if (desc) return "它能探测你附近的所有陷阱。";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_traps(DETECT_RAD_DEFAULT, device_known))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_MONSTERS:
        if (name) return "探测怪物";
        if (desc) return "它能探测你附近所有的可见怪物。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_monsters_normal(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_OBJECTS:
        if (name) return "探测物品";
        if (desc) return "它能探测你附近的所有物品。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_objects_normal(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_ALL:
        if (name) return "探测";
        if (desc) return "它能探测你附近所有的陷阱、门、楼梯、宝藏、物品和怪物。";
        if (value) return format("%d", 2000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            detect_all(DETECT_RAD_DEFAULT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_GOLD:
        if (name) return "探测宝藏";
        if (desc) return "当你使用它时，它能探测你附近的所有宝藏。";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (detect_treasure(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
            if (detect_objects_gold(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_INVISIBLE:
        if (name) return "探测隐形";
        if (desc) return "当你使用它时，它能探测你附近所有隐形的怪物。";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_monsters_invis(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_DOOR_STAIRS:
        if (name) return "探测门与楼梯";
        if (desc) return "当你使用它时，它能探测你附近所有的门和楼梯。";
        if (value) return format("%d", 300);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (detect_doors(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
            if (detect_stairs(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DETECT_EVIL:
        if (name) return "探测邪恶";
        if (desc) return "当你使用它时，它能探测你附近所有的邪恶怪物。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (detect_monsters_evil(DETECT_RAD_DEFAULT))
                device_noticed = TRUE;
        }
        break;

    /* Utility */
    case EFFECT_PHASE_DOOR:
        if (name) return "相位门";
        if (desc) return "它会将你传送到短距离之外。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            teleport_player(10, 0L);
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
        }
        break;
    case EFFECT_TELEPORT:
        if (name) return "传送";
        if (desc) return "它会将你传送到长距离之外。";
        if (value) return format("%d", 1500);
        if (cast)
        {
            teleport_player(100, 0L);
            energy_use = energy_use * 3 / 2;
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            device_noticed = TRUE;
            if (disciple_is_(DISCIPLE_TROIKA)) troika_effect(TROIKA_TELEPORT);
        }
        break;
    case EFFECT_TELEPORT_AWAY:
        if (name) return "传送他人";
        if (desc) return "它会发射一道光束，将被击中的所有怪物传送走。";
        if (info) return format("距离 %d", MAX_SIGHT * 5);
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (teleport_monster(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_STRAFING:
        if (name) return "扫射";
        if (desc) return "它会将你传送到附近一个可见的位置。";
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (mut_present(MUT_ASTRAL_GUIDE))
                energy_use = energy_use / 3;
            teleport_player(10, TELEPORT_LINE_OF_SIGHT);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DIMENSION_DOOR:
        if (name) return "任意门";
        if (desc) return "它会将你传送到一个选定的位置。";
        if (info) return info_range(_BOOST(effect->power / 2 + 10));
        if (value) return format("%d", 10000);
        if (cast)
        {
            if (dimension_door(_BOOST(effect->power / 2 + 10)))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_ESCAPE:
        if (name) return "逃脱";
        if (desc) return "它会提供一种随机的逃生方式。";
        if (value) return format("%d", 2000);
        if (cast)
        {
            switch (randint1(13))
            {
            case 1: case 2: case 3: case 4: case 5:
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(10, 0L);
                break;
            case 6: case 7: case 8: case 9: case 10:
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use /= 3;
                teleport_player(222, 0L);
                break;
            case 11: case 12:
                stair_creation(FALSE);
                break;
            default:
                (void)py_teleport_level("Teleport Level? ");
            }
            device_noticed = TRUE;
        }
        break;
    case EFFECT_RECALL:
        if (name) return "召回";
        if (desc) return "它能将你召回地表，或者将你送回你之前所在的地下城。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!word_of_recall(TRUE)) return NULL;
        }
        break;

    case EFFECT_STONE_TO_MUD:
        if (name) return "化石为泥";
        if (desc) return "它能将门、岩石或墙壁化为泥土。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            if (wall_to_mud(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_EARTHQUAKE:
        if (name) return "地震";
        if (desc) return "它会在附近引发一场强烈的地震。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!earthquake(py, px, _extra(effect, 10)))
                msg_print("大地在颤抖。");
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTRUCTION:
    {
        int power = _extra(effect, 150 + _power_curve_offset(400, effect->power, 50));
        if (name) return "破坏";
        if (desc) return "它会摧毁你附近的一切。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power * 8 + ((100 - (MIN(100, 8000 / power))) * 125));
        if (color) return format("%d", TERM_RED);
        if (cost) return format("%d", power/15);
        if (cast)
        {
            if (destroy_area(py, px, 13 + randint0(5), _BOOST(power)))
                device_noticed = TRUE;
            else
                msg_print("大地在颤抖……");
        }
        break;
    }
    case EFFECT_GENOCIDE:
    {
        int power = _extra(effect, effect->power * 3);
        if (name) return "灭绝";
        if (desc) return "尝试将当前楼层内所有指定种族的怪物抹除，这会让你筋疲力尽。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power*50);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!symbol_genocide(_BOOST(power), TRUE)) return NULL;
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MASS_GENOCIDE:
    {
        int power = _extra(effect, 100 + effect->power * 3);
        if (name) return "群体灭绝";
        if (desc) return "尝试消灭附近所有的怪物，这会让你筋疲力尽。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power*60);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (mass_genocide(_BOOST(power), TRUE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RECHARGE_FROM_DEVICE:
    {
        int power = _extra(effect, 100);
        if (name) return "充能";
        if (desc) return "它会尝试使用源装置的法力为另一件魔法装置充能。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power*30);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!recharge_from_device(_BOOST(power))) return NULL;
        }
        break;
    }
    case EFFECT_RECHARGE_FROM_PLAYER:
    {
        int power = _extra(effect, 100 + effect->power);
        if (name) return "*充能*";
        if (desc) return "它会尝试消耗你的法力作为源泉，为一件魔法装置充能。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power*30);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            device_noticed = TRUE;
            if (!recharge_from_player(_BOOST(power))) return NULL;
        }
        break;
    }
    case EFFECT_ENCHANTMENT:
        if (name) return "附魔";
        if (desc) return "它会尝试为一件武器、弹药或护甲附魔。";
        if (value) return format("%d", 5000);
        if (cast)
        {
            enchantment_hack = TRUE; /* TODO: Hephaestus only ... */
            device_noticed = TRUE;
            cast_enchantment();
            enchantment_hack = FALSE;
        }
        break;
    case EFFECT_IDENTIFY:
        if (name) return "鉴定";
        if (desc) return "它能鉴定一件物品。";
        if (value) return format("%d", 500);
        if (cast)
        {
            device_noticed = TRUE;
            if (!_do_identify()) return NULL;
        }
        break;
    case EFFECT_IDENTIFY_FULL:
        if (name) return "*鉴定*";
        if (desc) return "它能揭示一件物品的所有信息（全面鉴定）。";
        if (value) return format("%d", 5000);
        if (cast)
        {
            device_noticed = TRUE;
            if (!identify_fully(NULL)) return NULL;
        }
        break;
    case EFFECT_PROBING:
        if (name) return "探知";
        if (desc) return "它能探明所有可见怪物的阵营、生命值、防御等级(AC)、速度、当前经验值以及真实性格。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (probing()) device_noticed = TRUE;
        }
        break;
    case EFFECT_RUNE_EXPLOSIVE:
        if (name) return "爆炸符文";
        if (desc) return "它会放置一个符文，当怪物经过时会爆炸。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (explosive_rune()) device_noticed = TRUE;
        }
        break;
    case EFFECT_RUNE_PROTECTION:
        if (name) return "保护符文";
        if (desc) return "它会生成一个能阻止怪物攻击你的结界符文。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (warding_glyph()) device_noticed = TRUE;
        }
        break;
    case EFFECT_SATISFY_HUNGER:
        if (name) return "充饥";
        if (desc) return "它能用营养丰富的食物填饱你的肚子。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTROY_TRAP:
        if (name) return "破坏陷阱与门";
        if (desc) return "它会摧毁所有相邻方格中的陷阱和门。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (destroy_doors_touch()) device_noticed = TRUE;
        }
        break;
    case EFFECT_DESTROY_TRAPS:
        if (name) return "开启通路";
        if (desc) return "它会发射一道光束，摧毁沿途的陷阱和门。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            if (destroy_door(dir)) device_noticed = TRUE;
        }
        break;
    case EFFECT_WHIRLWIND_ATTACK:
        if (name) return "旋风斩";
        if (desc) return "它能使你在一个回合内攻击所有相邻的怪物。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            int           y = 0, x = 0;
            cave_type    *c_ptr;
            monster_type *m_ptr;
            int           dir;

            for (dir = 0; dir < 8; dir++)
            {
                y = py + ddy_ddd[dir];
                x = px + ddx_ddd[dir];
                c_ptr = &cave[y][x];
                m_ptr = &m_list[c_ptr->m_idx];
                if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
                {
                    py_attack(y, x, 0);
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_LIST_UNIQUES:
        if (name) return "列出唯一怪物";
        if (desc) return "它能列出当前楼层中所有的唯一怪物。";
        if (value) return format("%d", 12000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int i;
            for (i = m_max - 1; i >= 1; i--)
            {
                if (!m_list[i].r_idx) continue;
                if (r_info[m_list[i].r_idx].flags1 & RF1_UNIQUE)
                {
                    msg_format("%s. ", r_name + r_info[m_list[i].r_idx].name);
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_LIST_ARTIFACTS:
        if (name) return "列出神器";
        if (desc) return "它能列出当前楼层中所有的神器。";
        if (value) return format("%d", 15000);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int i;
            for (i = 1; i < o_max; i++)
            {
                object_type *o_ptr = &o_list[i];
                if (!o_ptr->k_idx) continue;
                if (o_ptr->held_m_idx) continue;
                if (o_ptr->name1)
                {
                    msg_format("%s. ", artifact_display_name(o_ptr->name1));
                    device_noticed = TRUE;
                }
                if (o_ptr->art_name)
                {
                    msg_format("%s. ", quark_str(o_ptr->art_name));
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_BANISH_EVIL:
    {
        int power = _extra(effect, 100);
        if (name) return "驱逐邪恶";
        if (desc) return "它会尝试将所有可见的邪恶怪物传送走。";
        if (info) return format("距离 %d", _BOOST(power));
        if (value) return format("%d", 50*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (banish_evil(_BOOST(power)))
            {
                msg_print("神圣的力量驱逐了邪恶！");
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_BANISH_ALL:
    {
        int power = _extra(effect, 150);
        if (name) return "驱逐";
        if (desc) return "除非被抵抗，否则它会将视线内的所有怪物传送走。";
        if (info) return format("距离 %d", _BOOST(power));
        if (value) return format("%d", 70*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (banish_monsters(_BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_TELEKINESIS:
    {
        int weight = effect->power * 7;
        if (name) return "隔空取物";
        if (desc) return "它能将远处的物品拉到你身边。";
        if (info) return info_weight(_BOOST(weight));
        if (value) return format("%d", 8*weight);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            fetch(dir, _BOOST(weight), FALSE);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_ALCHEMY:
        if (name) return "炼金术";
        if (desc) return "它能将一件物品变成金币。";
        if (value) return format("%d", 2000);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (!alchemy()) return NULL;
            device_noticed = TRUE;
        }
        break;
    case EFFECT_SELF_KNOWLEDGE:
        if (name) return "自我知识";
        if (desc) return "它会揭示关于你的属性、抗性以及生命评级的信息。";
        if (value) return format("%d", 2500);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            self_knowledge();
            device_noticed = TRUE;
        }
        break;
    case EFFECT_GENOCIDE_ONE:
    {
        int power = _extra(effect, 50 + effect->power * 3);
        if (name) return "湮灭";
        if (desc) return "除非被抵抗，否则当你使用它时，它会将一个怪物从当前的地下城楼层中抹除。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", power*50);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball_hide(GF_GENOCIDE, dir, _BOOST(power), 0);
            device_noticed = TRUE;
        }
        break;
    }
    /* Timed Buffs */
    case EFFECT_STONE_SKIN:
    {
        int power = _extra(effect, 20);
        if (name) return "石肤术";
        if (desc) return "它会暂时将你的皮肤变成石头，从而提高你的护甲等级。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 4000 + 50*power);
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (set_shield(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_ACID:
    {
        int power = _extra(effect, 20);
        if (name) return "抗酸";
        if (desc) return "它能赋予你临时的酸性抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (set_oppose_acid(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_ELEC:
    {
        int power = _extra(effect, 20);
        if (name) return "抗电";
        if (desc) return "它能赋予你临时的闪电抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (set_oppose_elec(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_FIRE:
    {
        int power = _extra(effect, 20);
        if (name) return "抗火";
        if (desc) return "它能赋予你临时的火焰抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cast)
        {
            if (set_oppose_fire(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_COLD:
    {
        int power = _extra(effect, 20);
        if (name) return "抗寒";
        if (desc) return "它能赋予你临时的寒冷抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", res_color(RES_COLD));
        if (cast)
        {
            if (set_oppose_cold(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESIST_POIS:
    {
        int power = _extra(effect, 20);
        if (name) return "抗毒";
        if (desc) return "它能赋予你临时的毒素抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 2500 + 25*power);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (set_oppose_pois(_BOOST(power + randint1(power)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESISTANCE:
    {
        int power = _extra(effect, 20);
        if (name) return "元素抵抗";
        if (desc) return "它能赋予你临时的对各种元素和毒素的抗性。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 25*power);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            int dur = _BOOST(power + randint1(power));
            if (set_oppose_base(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_PROT_EVIL:
    {
        int power = _extra(effect, 100);
        if (name) return "防护邪恶";
        if (desc) return "它能为你提供抵御邪恶生物近战攻击的临时防护。";
        if (info) return format("持续 d%d+%d", 25, _BOOST(power));
        if (value) return format("%d", 2000 + 10*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (set_protevil(_BOOST(randint1(25) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HOLY_GRAIL:
        if (name) return "治疗与魔法抗性";
        if (desc) return "它能治愈你的生命值，并赋予临时的魔法抗性。";
        if (info) return format("持续 d%d+%d", 50, _BOOST(50));
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (hp_player(250))
                device_noticed = TRUE;
            if (set_resist_magic(_BOOST(50 + randint1(50)), FALSE))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_BLESS:
    {
        int power = _extra(effect, 24);
        if (name) return "神圣祈祷";
        if (desc) return "当你阅读它时，能使你暂时受到祝福。";
        if (info) return format("持续 d%d+%d", _BOOST(power), 6);
        if (value) return format("%d", 1000 + 25*power);
        if (color) return format("%d", TERM_WHITE);
        if (cast)
        {
            if (set_blessed(_BOOST(randint1(power) + 6), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEROISM:
    {
        int power = _extra(effect, 25);
        if (name) return "英雄气概";
        if (desc) return "它能赋予你临时的英雄气概。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1500 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (set_hero(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BERSERK:
    {
        int power = _extra(effect, 25);
        if (name) return "狂暴";
        if (desc) return "它能使你陷入狂暴状态，增强你的战斗力，但会降低你的潜行能力和各项技能。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 1500 + 25*power);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            if (set_shero(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
            if (hp_player(30))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED:
    {
        int power = _extra(effect, 20);
        if (name) return "加速";
        if (desc) return "它能提供临时的速度提升。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 2500 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (set_fast(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED_HERO:
    {
        int power = _extra(effect, effect->power/2);
        if (name) return "英雄极速";
        if (desc) return "它能赋予临时的速度提升和英雄气概。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 25*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            int dur = _BOOST(randint1(power) + power);
            if (set_fast(dur, FALSE)) device_noticed = TRUE;
            if (set_hero(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SPEED_HERO_BLESS:
    {
        int power = _extra(effect, 15);
        if (name) return "英雄之歌";
        if (desc) return "它能赋予临时的速度提升、祝福和英雄气概。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 5000 + 30*power);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            int dur = _BOOST(randint1(power) + power);
            if (set_fast(dur, FALSE)) device_noticed = TRUE;
            if (set_hero(dur, FALSE)) device_noticed = TRUE;
            if (set_blessed(dur, FALSE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_LIGHT_SPEED:
    {
        int power = _extra(effect, 16);
        if (name) return "光速";
        if (desc) return "它能暂时赋予你不可思议的运动能力。";
        if (info) return format("持续 %d", _BOOST(power));
        if (value) return format("%d", 5000 + 500*power);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (set_lightspeed(_BOOST(power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_ENLARGE_WEAPON:
    {
        int power = _extra(effect, 7);
        if (name) return "武器巨大化";
        if (desc) return "它能暂时提升你近战武器的伤害骰数。";
        if (info) return format("持续 %d", _BOOST(power));
        if (value) return format("%d", 2000 + 250*power);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            if (set_tim_enlarge_weapon(_BOOST(power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_TELEPATHY:
    {
        int power = _extra(effect, 30);
        if (name) return "心灵感应";
        if (desc) return "它会暂时赋予你心灵感应的能力。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(25));
        if (value) return format("%d", 2000 + 50*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (set_tim_esp(_BOOST(randint1(power) + 25), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_WRAITHFORM:
    {
        int power = _extra(effect, 25);
        if (name) return "幽灵形态";
        if (desc) return "它能将你变成幽灵，赋予你穿墙的能力，并减少你受到攻击时的物理伤害。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 10000 + 100*power);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (set_wraith_form(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_INVULNERABILITY:
    {
        int power = _extra(effect, 8);
        if (name) return "无敌结界";
        if (desc) return "它能生成一道屏障，使你完全免疫几乎所有的伤害。当屏障破裂或持续时间结束时，会消耗你几个回合的时间（僵直）。";
        if (info) return format("持续 d%d+%d", _BOOST(power), _BOOST(power));
        if (value) return format("%d", 10000 + 500*power);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (set_invuln(_BOOST(randint1(power) + power), FALSE))
                device_noticed = TRUE;
        }
        break;
    }

    /* Pets */
    case EFFECT_SUMMON_MONSTERS:
        if (name) return "召唤怪物";
        if (desc) return "它会尝试召唤怪物来协助你。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(10)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, 0, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("你感觉有些不对劲……");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, PM_FORCE_PET | PM_ALLOW_GROUP | PM_NO_SUMMONERS))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_HOUNDS:
        if (name) return "召唤猎犬";
        if (desc) return "它会尝试召唤猎犬来协助你。";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(4)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_HOUND, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("你感觉有些不对劲……");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_HOUND, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_ANTS:
        if (name) return "召唤蚂蚁";
        if (desc) return "它会尝试召唤蚂蚁来协助你。";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(3)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_ANT, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("你感觉有些不对劲……");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_ANT, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_HYDRAS:
        if (name) return "召唤多头蛇";
        if (desc) return "它会尝试召唤九头蛇来协助你。";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint1(3);
            int i;
            bool hostile = (one_in_(3)) ? TRUE : FALSE;
            for (i = 0; i < num; i++)
            {
                if ((hostile) && (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level + 5, SUMMON_REPTILE, PM_NO_PET | PM_NO_KAGE | PM_ALLOW_GROUP)))
                {
                    msg_print("你感觉有些不对劲……");
                    device_noticed = TRUE;
                }
                else if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_HYDRA, PM_FORCE_PET | PM_ALLOW_GROUP))
                {
                    device_noticed = TRUE;
                }
            }
        }
        break;
    case EFFECT_SUMMON_OCTOPUS:
        if (name) return "召唤章鱼";
        if (desc) return "它会尝试召唤章鱼来协助你。";
        if (value) return format("%d", 1200);
        if (cast)
        {
            int num = randint0(3);
            int i;
            for (i = 0; i < num; i++)
            {
                if (summon_named_creature(-1, py, px, MON_JIZOTAKO, PM_FORCE_PET | PM_ALLOW_GROUP))
                    device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_DAWN:
        if (name) return "召唤黎明军团";
        if (desc) return "它会尝试召唤黎明斗士来协助你。";
        if (value) return format("%d", 2500);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DAWN, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            {
                msg_print("你召唤了黎明军团。");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_PHANTASMAL:
        if (name) return "召唤幻影仆从";
        if (desc) return "它会尝试召唤一只幻影仆从（Phantasmal Servant）来协助你。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_PHANTOM, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            {
                msg_print("你召唤了一个幻影仆从。");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SUMMON_ELEMENTAL:
        if (name) return "召唤元素";
        if (desc) return "它会尝试召唤一只元素生物来为你服务。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, SUMMON_ELEMENTAL, mode))
            {
                device_noticed = TRUE;
                msg_print("一个元素实体化了……");
                if (pet)
                    msg_print("它似乎对你很服从。");
                else
                    msg_print("你未能控制住它！");
            }
        }
        break;
    case EFFECT_SUMMON_DRAGON:
        if (name) return "召唤龙";
        if (desc) return "它会尝试召唤一只龙来协助你。";
        if (value) return format("%d", 1500);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_DRAGON, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_UNDEAD:
        if (name) return "召唤不死生物";
        if (desc) return "它会尝试召唤一只不死怪物来为你服务。";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            int  type = lvl > 75 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, type, mode))
            {
                device_noticed = TRUE;
                msg_print("寒风开始在你周围吹拂，夹杂着腐烂的恶臭……");
                if (pet)
                    msg_print("古老、早已死去的躯体从地下爬起，听候你的差遣！");
                else
                    msg_print("'死者苏醒了……来惩罚你惊扰了他们！'");
            }
        }
        break;
    case EFFECT_SUMMON_DEMON:
        if (name) return "召唤恶魔";
        if (desc) return "它会尝试召唤一只恶魔来为你服务。";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            bool pet = one_in_(3);
            int  lvl = dun_level;
            u32b mode = pet ? PM_FORCE_PET : PM_NO_PET;
            int  who = SUMMON_WHO_PLAYER;

            if (!pet || lvl >= 50)
                mode |= PM_ALLOW_GROUP;

            if (summon_specific(who, py, px, lvl, SUMMON_DEMON, mode))
            {
                device_noticed = TRUE;
                msg_print("这片区域弥漫着硫磺和烈火的恶臭。");
                if (pet)
                    msg_print("'您有何吩咐……主人？'");
                else
                    msg_print("'我绝不服从！可怜虫！我要吞噬你凡人的灵魂！'");
            }
        }
        break;
    case EFFECT_SUMMON_CYBERDEMON:
        if (name) return "召唤机械恶魔";
        if (desc) return "它会尝试召唤一只赛博恶魔（Cyberdemon）来协助你。";
        if (value) return format("%d", 7500);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_CYBER, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_ANGEL:
        if (name) return "召唤天使";
        if (desc) return "它会尝试召唤一名天使来协助你。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_ANGEL, PM_FORCE_PET))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_KRAKEN:
        if (name) return "召唤海怪";
        if (desc) return "它会尝试召唤强大的海妖(kraken)来协助你。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            int num = randint0(3);
            int ct = 0;
            int i;
            fire_ball_hide(GF_WATER_FLOW, 0, 3, 3);
            device_noticed = TRUE;
            for (i = 0; i < num; i++)
                ct += summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, SUMMON_KRAKEN, PM_FORCE_PET);
            if (!ct)
                msg_print("没有援军到来。");
        }
        break;

    case EFFECT_CHARM_ANIMAL:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "魅惑动物";
        if (desc) return "它会尝试魅惑一只动物。";
        if (info) return format("强度 %d", _BOOST(lvl));
        if (value) return format("%d", 10*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (charm_animal(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_DEMON:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "支配恶魔";
        if (desc) return "它会尝试支配一只恶魔。";
        if (info) return format("强度 %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (control_one_demon(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_UNDEAD:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "奴役不死生物";
        if (desc) return "它会尝试奴役一只不死怪物。";
        if (info) return format("强度 %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (control_one_undead(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CHARM_MONSTER:
    {
        int lvl = _extra(effect, effect->power);
        if (name) return "魅惑怪物";
        if (desc) return "它会尝试魅惑一个怪物。";
        if (info) return format("强度 %d", _BOOST(lvl));
        if (value) return format("%d", 15*_extra(effect, 50));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return FALSE;
            if (charm_monster(dir, _BOOST(lvl)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RETURN_PETS:
        if (name) return "召回宠物";
        if (desc) return "它会将你的宠物召回到你身边。";
        if (value) return format("%d", 500);
        if (cast)
        {
            int pet_ctr, i;
            u16b *who;
            int max_pet = 0;
            u16b dummy_why;

            stop_mouth();

            C_MAKE(who, max_m_idx, u16b);

            for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
            {
                if (is_pet(&m_list[pet_ctr]) && (p_ptr->riding != pet_ctr))
                    who[max_pet++] = pet_ctr;
            }

            ang_sort_comp = ang_sort_comp_pet;
            ang_sort_swap = ang_sort_swap_hook;
            ang_sort(who, &dummy_why, max_pet);

            for (i = 0; i < max_pet; i++)
            {
                pet_ctr = who[i];
                teleport_monster_to(pet_ctr, py, px, 100, TELEPORT_PASSIVE);
                device_noticed = TRUE;
            }

            C_KILL(who, max_m_idx, u16b);
        }
        break;

    case EFFECT_CAPTURE_PET:
        if (name) return "捕获宠物";
        if (desc) return "它会尝试捕获目标怪物。";
        if (value) return format("%d", 500);
        if (cast)
        {
            /* TODO: This is handled elsewhere in cm6.c, since we need the object_type
               for the capture ball in order to "reconstitute" the captured pet.
             */
        }
        break;

    /* Healing and Recovery */
    case EFFECT_RESTORE_STATS:
        if (name) return "恢复属性";
        if (desc) return "它能恢复你的属性。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
            if (do_res_stat(A_INT)) device_noticed = TRUE;
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
            if (do_res_stat(A_CON)) device_noticed = TRUE;
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
        }
        break;
    case EFFECT_RESTORE_EXP:
        if (name) return "恢复生命";
        if (desc) return "它能恢复你的生命力与经验值。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(150)) device_noticed = TRUE;
        }
        break;
    case EFFECT_RESTORING:
        if (name) return "恢复";
        if (desc) return "它能恢复你的属性、生命力与经验值。";
        if (value) return format("%d", 6000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (do_res_stat(A_STR)) device_noticed = TRUE;
            if (do_res_stat(A_INT)) device_noticed = TRUE;
            if (do_res_stat(A_WIS)) device_noticed = TRUE;
            if (do_res_stat(A_DEX)) device_noticed = TRUE;
            if (do_res_stat(A_CON)) device_noticed = TRUE;
            if (do_res_stat(A_CHR)) device_noticed = TRUE;
            if (restore_level()) device_noticed = TRUE;
            if (lp_player(1000)) device_noticed = TRUE;
        }
        break;
    case EFFECT_HEAL:
    {
        int amt = _extra(effect, 25 + effect->power);
        if (name) return (amt < 100) ? "治疗伤势" : "治疗";
        if (desc) return "它能治愈你的生命值并消除割伤。";
        if (info) return info_heal(0, 0, _BOOST(amt));
        if (value) return format("%d", 15*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            amt = _BOOST(amt);

            if (hp_player(amt)) device_noticed = TRUE;
            if (amt >= 100)
            {
                if (set_cut(0, TRUE)) device_noticed = TRUE;
            }
            else
            {
                if (set_cut(p_ptr->cut - amt, TRUE)) device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_CURING:
    {
        if (name) return "治愈";
        if (desc) return "它能治愈致盲、混乱、震慑、割伤和幻觉状态，并减轻中毒程度。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE))
                device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_image(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEAL_CURING:
    {
        int amt = _extra(effect, 30 + 4*effect->power);
        if (amt < 100)
        {
            if (name) return "治疗伤势";
            if (desc) return "它能治愈你的生命值并消除致盲和割伤。";
        }
        else
        {
            if (name) return "治疗";
            if (desc) return "它能治愈你的生命值并消除你的异常状态(ailments)。";
        }
        if (info) return info_heal(0, 0, _BOOST(amt));
        if (value) return format("%d", ((amt > 100) ? 5000 : 1000) + 23*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", amt/10);
        if (cast)
        {
            amt = _BOOST(amt);

            if (hp_player(amt)) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (amt >= 100)
            {
                if (set_cut(0, TRUE)) device_noticed = TRUE;
                if (set_confused(0, TRUE)) device_noticed = TRUE;
                if (set_stun(0, TRUE)) device_noticed = TRUE;
            }
            else
            {
                if (set_cut(p_ptr->cut - amt, TRUE)) device_noticed = TRUE;
            }
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HEAL_CURING_HERO:
    {
        int amt = _extra(effect, 300 + _power_curve_offset(477, effect->power, 70));
        if (name) return "天使治愈";
        if (desc) return "它能治愈你的生命值，消除你的异常状态，并使你获得英雄气概。";
        if (info) return info_heal(0, 0, _BOOST(amt));
        /* XXX The following is too low for -AngelicHealing, but avoids over-valuing Lohengrin */
        if (value) return format("%d", 750 + 15*amt);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (hp_player(_BOOST(amt))) device_noticed = TRUE;
            if (set_blind(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
            if (set_confused(0, TRUE)) device_noticed = TRUE;
            if (set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
            if (set_hero(_BOOST(randint1(25) + 25), FALSE)) device_noticed = TRUE;
            if (p_inc_minislow(-1)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RESTORE_MANA:
        if (name) return "恢复法力";
        if (desc) return "它能完全恢复你的法力。同时还能为你背包中的任何魔法装置部分充能。";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if (restore_mana()) device_noticed = TRUE;
            if (set_shero(0,TRUE)) device_noticed = TRUE;
        }
        break;
    case EFFECT_CURE_POIS:
        if (name) return "解毒";
        if (desc) return "它能减轻中毒程度。";
        if (value) return format("%d", 500);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (set_poisoned(p_ptr->poisoned - MAX(300, p_ptr->poisoned / 2), TRUE))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_CURE_FEAR:
        if (name) return "无畏";
        if (desc) return "它能恢复你的勇气。";
        if (value) return format("%d", 750);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_CURE_FEAR_POIS:
        if (name) return "消除恐惧与解毒";
        if (desc) return "它能减轻中毒程度并恢复你在战斗中的勇气。";
        if (value) return format("%d", 1250);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE))
                device_noticed = TRUE;
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_REMOVE_CURSE:
        if (name) return "移除诅咒";
        if (desc) return "它能解除已装备物品上的普通诅咒。";
        if (value) return format("%d", 1000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (remove_curse())
            {
                msg_print("你感觉好像有人在守护着你。");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_REMOVE_ALL_CURSE:
        if (name) return "*解除诅咒*";
        if (desc) return "它能解除已装备物品上的普通和重度诅咒。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if (remove_all_curse())
            {
                msg_print("你感觉好像有人在守护着你。");
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_CLARITY:
    {
        int amt = _extra(effect, effect->cost);
        if (name) return "清明";
        if (desc) return "它能使你的头脑变得清醒，恢复部分法力。";
        if (info) return format("%dsp", _BOOST(amt));
        if (value) return format("%d", 1000 + 50*amt);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("你不受影响。");
            else if (sp_player(_BOOST(amt)))
            {
                msg_print("你感觉头脑清醒了。");
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_GREAT_CLARITY: /* FYI: This is a separate effect from EFFECT_CLARITY for purposes */
    {                          /*      of statistics tracking, which is by effect id. */
        int amt = _extra(effect, effect->cost);
        if (name) return "高等清明";
        if (desc) return "它能使你的头脑变得清醒，恢复部分法力。";
        if (info) return format("%dsp", _BOOST(amt));
        if (value) return format("%d", 1000 + 50*amt);
        if (color) return format("%d", TERM_L_BLUE);
        if (cast)
        {
            if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
                msg_print("你不受影响。");
            else if (sp_player(_BOOST(amt)))
            {
                msg_print("你感觉头脑清醒了。");
                device_noticed = TRUE;
            }
        }
        break;
    }

    /* Offense: Bolts */
    case EFFECT_BOLT_MISSILE:
    {
        int dd = _extra(effect, 2 + effect->power/10);
        int ds = 6;
        if (name) return "魔法飞弹";
        if (desc) return "它会发射一道微弱的魔法弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_MISSILE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ACID:
    {
        int dd = _extra(effect, 6 + effect->power/7);
        int ds = 8;
        if (name) return "酸之矢";
        if (desc) return "它会发射一道酸液弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ACID, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ELEC:
    {
        int dd = _extra(effect, 4 + effect->power/9);
        int ds = 8;
        if (name) return "闪电之矢";
        if (desc) return "它会发射一道闪电弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ELEC, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_FIRE:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "火之矢";
        if (desc) return "它会发射一道火焰弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_FIRE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_FIRE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_COLD:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "冰霜之矢";
        if (desc) return "它会发射一道寒冰弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_COLD));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_COLD, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_POIS:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "毒镖";
        if (desc) return "它会发射一枚毒镖。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_POIS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_LITE:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "光之矢";
        if (desc) return "它会发射一道闪光弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_LITE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_DARK:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "暗之矢";
        if (desc) return "它会发射一道暗黑弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 30*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_DARK));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_DARK, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_CONF:
    {
        int dd = _extra(effect, 5 + effect->power/8);
        int ds = 8;
        if (name) return "困惑之矢";
        if (desc) return "它会发射一道混乱弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 25*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_CONFUSION, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_NETHER:
    {
        int dd = _extra(effect, 10 + effect->power/6);
        int ds = 8;
        if (name) return "地狱之矢";
        if (desc) return "它会发射一道地狱弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_NETHER));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_NETHER, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_NEXUS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "时空之矢";
        if (desc) return "它会发射一道时空(nexus)弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_NEXUS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_SOUND:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "音波之矢";
        if (desc) return "它会发射一道音波弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_SOUND, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_SHARDS:
    {
        int dd = _extra(effect, 7 + effect->power/5);
        int ds = 8;
        if (name) return "碎片之矢";
        if (desc) return "它会发射一道破片弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_SHARDS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_CHAOS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "混沌之矢";
        if (desc) return "它会发射一道混沌弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_CHAOS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_DISEN:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "解除魔法之矢";
        if (desc) return "它会发射一道解除魔法弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 35*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_DISENCHANT, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_TIME:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "时间之矢";
        if (desc) return "它会发射一道时间弹。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 45*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_TIME, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_WATER:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 20;
        if (name) return "水之矢";
        if (desc) return "它会发射一道水流弹。";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", TERM_BLUE);
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_WATER, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_MANA:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(500, effect->power));
        int base = 50;
        if (name) return "法力之矢";
        if (desc) return "它会发射一道强大的法力弹。";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", TERM_L_BLUE);
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/8);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_MANA, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_ICE:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 30;
        if (name) return "寒冰之矢";
        if (desc) return "它会发射一道坚冰弹。";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ICE, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BOLT_PLASMA:
    {
        int dd = 1;
        int ds = _extra(effect, _power_curve(400, effect->power));
        int base = 40;
        if (name) return "等离子之矢";
        if (desc) return "它会发射一道等离子弹。";
        if (info) return info_damage(dd, _BOOST(ds), _BOOST(base));
        if (value) return format("%d", 40*(_avg_damroll(dd, ds) + base));
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", (_avg_damroll(dd, ds) + base)/7);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_PLASMA, dir, _BOOST(damroll(dd, ds) + base));
            device_noticed = TRUE;
        }
        break;
    }


    /* Offense: Beams */
    case EFFECT_BEAM_LITE_WEAK:
    {
        int dd = _extra(effect, 6);
        int ds = 8;
        if (name) return "月光射线";
        if (desc) return "它会发射一道微弱的光束，对惧光的生物造成伤害。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 20*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("一道苍白微光的射线出现了。");
            project_hook(GF_LITE_WEAK, dir, _BOOST(damroll(dd, ds)), PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_LITE:
    {
        int dam = _extra(effect, 10 + _power_curve(275, effect->power));
        if (name) return "光之射线";
        if (desc) return "它会发射一道强大的闪光射线。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cost) return format("%d", dam/7);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("一道纯白的射线出现了。");
            fire_beam(GF_LITE, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_GRAVITY:
    {
        int dd = _extra(effect, 9 + effect->power/8);
        int ds = 8;
        if (name) return "重力射线";
        if (desc) return "它会发射一道重力射线。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 55*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_L_UMBER);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_GRAVITY, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_DISINTEGRATE:
    {
        int dd = _extra(effect, 9 + effect->power/8);
        int ds = 8;
        if (name) return "解离射线";
        if (desc) return "它会发射一道分解射线。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (!get_fire_dir_aux(&dir, TARGET_DISI)) return NULL;
            fire_beam(GF_DISINTEGRATE, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_ACID:
    {
        int dam = _extra(effect, 5 + _power_curve(270, effect->power));
        if (name) return "射出酸液";
        if (desc) return "它会发射一道酸液射线。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_ACID, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_ELEC:
    {
        int dam = _extra(effect, 5 + _power_curve(250, effect->power));
        if (name) return "闪电打击";
        if (desc) return "它会发射一道闪电射线。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_ELEC, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_FIRE:
    {
        int dam = _extra(effect, 5 + _power_curve(280, effect->power));
        if (name) return "火焰射线";
        if (desc) return "它会发射一道火焰射线。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_FIRE, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_COLD:
    {
        int dam = _extra(effect, 5 + _power_curve(260, effect->power));
        if (name) return "寒冰射线";
        if (desc) return "它会发射一道寒冰射线。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/6);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_COLD, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_SOUND:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "音波打击";
        if (desc) return "它会发射一道音波射线。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 50*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_SOUND));
        if (cost) return format("%d", _avg_damroll(dd, ds)/5);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_SOUND, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BEAM_CHAOS:
    {
        int dd = _extra(effect, 7 + effect->power/6);
        int ds = 8;
        if (name) return "混沌打击";
        if (desc) return "它会发射一道混沌射线。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cost) return format("%d", _avg_damroll(dd, ds)/5);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_beam(GF_CHAOS, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Balls */
    case EFFECT_BALL_ACID:
    {
        int dam = _extra(effect, 20 + _power_curve(300, effect->power));
        if (name) return "酸之球";
        if (desc) return "它会发射一颗酸液球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ACID, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_ELEC:
    {
        int dam = _extra(effect, 20 + _power_curve(250, effect->power));
        if (name) return "闪电球";
        if (desc) return "它会发射一颗闪电球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ELEC, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_FIRE:
    {
        int dam = _extra(effect, 20 + _power_curve(350, effect->power));
        if (name) return "火球";
        if (desc) return "它会发射一颗火球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_FIRE, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_COLD:
    {
        int dam = _extra(effect, 20 + _power_curve(275, effect->power));
        if (name) return "冰霜球";
        if (desc) return "它会发射一颗寒冰球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_COLD, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_POIS:
    {
        int dam = _extra(effect, 12 + effect->power/4);
        if (name) return "臭气云";
        if (desc) return "它会发射一颗毒素球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_POIS, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_LITE:
    {
        int dam = _extra(effect, 200 + _power_curve_offset(350, effect->power, 80));
        if (name) return "星爆";
        if (desc) return "它会发射一颗巨大的强光球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_LITE, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DARK:
    {
        int dam = _extra(effect, 100 + 7*effect->power/2);
        if (name) return "黑暗风暴";
        if (desc) return "它会发射一颗巨大的暗黑球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_DARK));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DARK, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_CONF:
    {
        int dam = _extra(effect, 30 + effect->power);
        if (name) return "困惑球";
        if (desc) return "它会发射一颗混乱球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_NETHER:
    {
        int dam = _extra(effect, 125 + _power_curve_offset(250, effect->power, 30));
        if (name) return "地狱球";
        if (desc) return "它会发射一颗地狱球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 25*dam);
        if (color) return format("%d", res_color(RES_NETHER));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_NEXUS:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(200, effect->power, 40));
        if (name) return "时空球";
        if (desc) return "它会发射一颗时空(nexus)球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NEXUS, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_SOUND:
    {
        int dam = _extra(effect, 70 + _power_curve_offset(280, effect->power, 40));
        if (name) return "音波球";
        if (desc) return "它会发射一颗音波球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SOUND, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_SHARDS:
    {
        int dam = _extra(effect, 175 + _power_curve_offset(325, effect->power, 75));
        if (name) return "碎片球";
        if (desc) return "它会发射一颗破片球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SHARDS, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_CHAOS:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(350, effect->power, 70));
        if (name) return "唤起洛格鲁斯";
        if (desc) return "它会发射一颗巨大的混沌球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CHAOS, dir, _BOOST(dam), 5);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DISEN:
    {
        int dam = _extra(effect, 90 + _power_curve_offset(250, effect->power, 40));
        if (name) return "解除魔法球";
        if (desc) return "它会发射一颗解除魔法球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISENCHANT, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_TIME:
    {
        int dam = _extra(effect, 50 + effect->power);
        if (name) return "时间球";
        if (desc) return "它会发射一颗时间球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_TIME, dir, _BOOST(dam), 3);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_WATER:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(200, effect->power, 50));
        if (name) return "漩涡";
        if (desc) return "它会发射一颗巨大的水流球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_BLUE);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_WATER, dir, _BOOST(dam), 4);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_MANA:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(300, effect->power, 60));
        if (name) return "法力球";
        if (desc) return "它会发射一颗强大的法力球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_L_BLUE);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_ball(GF_MANA, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BALL_DISINTEGRATE:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(200, effect->power, 50));
        if (name) return "解离";
        if (desc) return "它会发射一颗强大的分解球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_SLATE);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (device_known && !get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISINTEGRATE, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Breaths */
    case EFFECT_BREATHE_ACID:
    {
        int dam = _extra(effect, 100 + effect->power*3);
        if (name) return "喷吐酸液";
        if (desc) return "它能喷吐酸液。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_ACID));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ACID, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ELEC:
    {
        int dam = _extra(effect, 70 + effect->power*3);
        if (name) return "喷吐闪电";
        if (desc) return "它能喷吐闪电。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_ELEC));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_ELEC, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_FIRE:
    {
        int dam = _extra(effect, 160 + _power_curve_offset(300, effect->power, 40));
        if (name) return "龙之火焰";
        if (desc) return "它能喷吐火焰。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_FIRE));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_FIRE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_COLD:
    {
        int dam = _extra(effect, 150 + _power_curve_offset(300, effect->power, 40));
        if (name) return "龙之寒霜";
        if (desc) return "它能喷吐寒霜。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_COLD));
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_COLD, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_POIS:
    {
        int dam = _extra(effect, 60 + effect->power*2);
        if (name) return "喷吐毒气";
        if (desc) return "它能喷吐毒素。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_POIS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_LITE:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐强光";
        if (desc) return "它能喷吐强光。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_LITE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_DARK:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐黑暗";
        if (desc) return "它能喷吐暗黑。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_DARK));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DARK, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_CONF:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐困惑气体";
        if (desc) return "它能喷吐混乱。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CONFUSION, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_NETHER:
    {
        int dam = _extra(effect, 100 + effect->power*3);
        if (name) return "喷吐地狱气体";
        if (desc) return "它能喷吐地狱能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", res_color(RES_NETHER));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NETHER, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_NEXUS:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐时空力量";
        if (desc) return "它能喷吐时空(nexus)能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_NEXUS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_NEXUS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_SOUND:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐音波";
        if (desc) return "它能喷吐音波。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SOUND, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_SHARDS:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "喷吐碎片";
        if (desc) return "它能喷吐破片。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SHARDS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_SHARDS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_CHAOS:
    {
        int dam = _extra(effect, 75 + effect->power*2);
        if (name) return "喷吐混沌";
        if (desc) return "它能喷吐混沌能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_CHAOS, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_DISEN:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐解除魔法能量";
        if (desc) return "它能喷吐解除魔法能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_DISENCHANT, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
	case EFFECT_BREATHE_INERTIA:
	{
		int dam = _extra(effect, 50 + effect->power * 2);
		if (name) return "喷吐惰性";
		if (desc) return "它能喷吐迟缓(inertia)能量。";
		if (info) return info_damage(0, 0, _BOOST(dam));
		if (value) return format("%d", 35 * dam);
		if (cast)
		{
			if (!get_fire_dir(&dir)) return NULL;
			fire_ball(GF_INERT, dir, _BOOST(dam), -2);
			device_noticed = TRUE;
		}
		break;
	}
	case EFFECT_BREATHE_WATER:
	{
		int dam = _extra(effect, 41 + effect->power * 7 / 4);
		if (name) return "海啸";
		if (desc) return "它能喷射出水流激流。";
		if (info) return info_damage(0, 0, _BOOST(dam));
		if (value) return format("%d", 30 * dam);
		if (cost) return format("%d", dam/32);
		if (cast)
		{
			if (!get_fire_dir(&dir)) return NULL;
			fire_ball(GF_WATER, dir, _BOOST(dam), -2);
			device_noticed = TRUE;
		}
		break;
	}
    case EFFECT_BREATHE_TIME:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐时间力量";
        if (desc) return "它能喷吐时间流。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_TIME));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_TIME, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_MULTIHUED:
    {
        int dam = _extra(effect, 170 + _power_curve_offset(300, effect->power, 40));
        if (name) return "龙之喷吐";
        if (desc) return "它能随机喷吐酸液、闪电、火焰、寒霜或毒素。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", TERM_ORANGE);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[5] = {
                { GF_ACID, "酸液"},
                { GF_ELEC, "闪电"},
                { GF_FIRE, "火焰"},
                { GF_COLD, "寒霜"},
                { GF_POIS, "毒素"},
            };
            int which = randint0(5);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("它喷吐出%s。", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_CHAOS:
    {
        int dam = _extra(effect, 75 + effect->power*2);
        if (name) return "喷吐";
        if (desc) return "它能随机喷吐混沌或解除魔法能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", res_color(RES_CHAOS));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_CHAOS, "混沌"},
                { GF_DISENCHANT, "解除魔法"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("它喷吐出%s。", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_LAW:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "喷吐";
        if (desc) return "它能随机喷吐音波或破片。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_SOUND));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_SOUND, "音波"},
                { GF_SHARDS, "破片"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("它喷吐出%s。", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_BALANCE:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "喷吐";
        if (desc) return "它能随机喷吐音波、破片、混沌或解除魔法能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", res_color(RES_DISEN));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[4] = {
                { GF_SOUND, "音波"},
                { GF_SHARDS, "破片"},
                { GF_CHAOS, "混沌"},
                { GF_DISENCHANT, "解除魔法"},
            };
            int which = randint0(4);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("它喷吐出%s。", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ONE_SHINING:
    {
        int dam = _extra(effect, 50 + effect->power*2);
        if (name) return "喷吐";
        if (desc) return "它能随机喷吐强光或暗黑。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", res_color(RES_LITE));
        if (cast)
        {
            struct { int  type; cptr desc; } _choices[2] = {
                { GF_LITE, "强光"},
                { GF_DARK, "暗黑"},
            };
            int which = randint0(2);

            if (!get_fire_dir(&dir)) return NULL;
            msg_format("它喷吐出%s。", _choices[which].desc);
            fire_ball(_choices[which].type, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_BREATHE_ELEMENTS:
    {
        int dam = _extra(effect, 100 + effect->power*2);
        if (name) return "喷吐元素";
        if (desc) return "它能喷吐所有元素能量。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 55*dam);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_ball(GF_MISSILE, dir, _BOOST(dam), -2);
            device_noticed = TRUE;
        }
        break;
    }

    /* Offense: Other */
    case EFFECT_DISPEL_EVIL:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(250, effect->power, 50));
        if (name) return "驱散邪恶";
        if (desc) return "它会对视线内的所有邪恶怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_EVIL_HERO:
    {
        int dam = _extra(effect, 2*effect->power);
        if (name) return "驱散邪恶";
        if (desc) return "它会对视线内的所有邪恶怪物造成伤害，并赋予你临时的英雄气概。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 500 + 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam)))
                device_noticed = TRUE;
            if (set_hero(_BOOST(25 + randint1(25)), FALSE))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_GOOD:
    {
        int dam = _extra(effect, 2*effect->power);
        if (name) return "驱散善良";
        if (desc) return "它会对视线内的所有善良怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/12);
        if (cast)
        {
            if (dispel_good(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_LIFE:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(250, effect->power, 50));
        if (name) return "驱散生命";
        if (desc) return "它会对视线内的所有活体怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/9);
        if (cast)
        {
            if (dispel_living(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_DEMON:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(400, effect->power, 50));
        if (name) return "驱散恶魔";
        if (desc) return "它会对视线内的所有恶魔类怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/15);
        if (cast)
        {
            if (dispel_demons(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_UNDEAD:
    {
        int dam = _extra(effect, 100 + _power_curve_offset(400, effect->power, 50));
        if (name) return "驱散不死生物";
        if (desc) return "它会对视线内的所有不死怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 20*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/15);
        if (cast)
        {
            if (dispel_undead(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DISPEL_MONSTERS:
    {
        int dam = _extra(effect, 50 + _power_curve_offset(200, effect->power, 50));
        if (name) return "驱散怪物";
        if (desc) return "它会对视线内的所有怪物造成伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 40*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (dispel_monsters(_BOOST(dam)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DRAIN_LIFE:
    {
        int dam = _extra(effect, 50 + effect->power/2);
        if (name) return "吸血";
        if (desc) return "当你使用它时，它会发射一道能从敌人身上吸取生命力的魔法弹。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 35*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            dam = _BOOST(dam);
            if (drain_life(dir, dam))
            {
                vamp_player(dam);
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_STAR_BALL:
    {
        int dam = _extra(effect, 150);
        if (name) return "星之球";
        if (desc) return "它会向随机的空旷方向发射出大量闪电球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            int num = _BOOST(damroll(5, 3));
            int y, x, i;
            int attempts;

            for (i = 0; i < num; i++)
            {
                attempts = 1000;
                while (attempts--)
                {
                    scatter(&y, &x, py, px, 4, 0);
                    if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                    if (!player_bold(y, x)) break;
                }
                project(0, 3, y, x, _BOOST(dam), GF_ELEC,
                    (PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
            }
        }
        break;
    }
    case EFFECT_WRATH_OF_GOD:
    {
        int dam = _extra(effect, 25 + effect->power*3/2);
        if (name) return "神之愤怒";
        if (desc) return "它会在目标附近落下许多分解球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (cast)
        {
            if (!cast_wrath_of_the_god(_BOOST(dam), 2)) return NULL;
        }
        break;
    }
    case EFFECT_ROCKET:
    {
        int dam = _extra(effect, 200 + _power_curve_offset(300, effect->power, 60));
        if (name) return "火箭";
        if (desc) return "它会发射一枚火箭弹。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (color) return format("%d", TERM_UMBER);
        if (cost) return format("%d", dam/11);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_rocket(GF_ROCKET, dir, _BOOST(dam), 2);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_METEOR:
    {
        int dd = _extra(effect, 15 + effect->power/5);
        int ds = 13;
        if (name) return "陨石";
        if (desc) return "当你使用它时，它会召唤一颗陨石。";
        if (info) return info_damage(_BOOST(dd), ds, 0);
        if (value) return format("%d", 40*_avg_damroll(dd, ds));
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_METEOR, dir, _BOOST(damroll(dd, ds)));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MANA_STORM:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "法力风暴";
        if (desc) return "它会以你为中心产生一颗巨大的法力球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_RED);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            msg_print("强大的魔法撕裂了你的敌人！");
            project(0, 5, py, px,
                _BOOST(dam*2),
                GF_MANA, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSING_LITE:
    {
        int pow = _extra(effect, effect->power*2);
        if (name) return "困惑之光";
        if (desc) return "它会发出耀眼的光芒，能使附近的怪物减速、震慑、混乱、恐惧甚至被冻结。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 60*pow);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            msg_print("你用令人目眩的困惑之光照射附近的怪物！");
            pow = _BOOST(pow);
            slow_monsters(pow);
            stun_monsters(5 + pow/10);
            confuse_monsters(pow);
            turn_monsters(pow);
            stasis_monsters(pow/3);
            device_noticed = TRUE; /* You see the dazzling lights, no? */
        }
        break;
    }
    case EFFECT_ARROW:
    {
        int dam = _extra(effect, 70 + effect->power);
        if (name) return "魔法箭";
        if (desc) return "它会发射一支强大的魔法箭。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 30*dam);
        if (color) return format("%d", TERM_SLATE);
        if (cost) return format("%d", dam/8);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            fire_bolt(GF_ARROW, dir, _BOOST(dam));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HOLINESS:
    {
        int dam = _extra(effect, effect->power*2);
        if (name) return "神圣";
        if (desc) return "当你使用它时，它会对视线内的所有邪恶怪物造成伤害，提供抵御次级邪恶生物的临时防护，解毒、解除震慑和割伤、消除恐惧，并治愈你的生命值。";
        if (info) return info_power(_BOOST(dam));
        if (value) return format("%d", 5000 + 30*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (dispel_evil(_BOOST(dam))) device_noticed = TRUE;
            if (set_protevil(p_ptr->protevil + _BOOST(dam/2), FALSE)) device_noticed = TRUE;
            if (hp_player(_BOOST(dam))) device_noticed = TRUE;
            if (set_stun(0, TRUE)) device_noticed = TRUE;
            if (set_cut(0, TRUE)) device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_STARBURST:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "星爆";
        if (desc) return "它会以你为中心产生一颗巨大的强光球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_YELLOW);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_LITE))
            {
                set_blind(p_ptr->blind + 3 + randint1(5), FALSE);
            }
            project(0, 5, py, px, _BOOST(dam*2),
                    GF_LITE, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_DARKNESS_STORM:
    {
        int dam = _extra(effect, 375 + _power_curve_offset(200, effect->power, 80));
        if (name) return "黑暗风暴";
        if (desc) return "它会以你为中心产生一颗巨大的暗黑球。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 45*dam);
        if (color) return format("%d", TERM_L_DARK);
        if (cost) return format("%d", dam/10);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                set_blind(p_ptr->blind + 3 + randint1(5), FALSE);
            }
            project(0, 5, py, px, _BOOST(dam*2),
                    GF_DARK, PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_PESTICIDE:
    {
        int dam = _extra(effect, 4);
        if (name) return "杀虫剂";
        if (desc) return "当你发射(zap)它时，它会对视线内的所有怪物造成轻微伤害。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 250);
        if (color) return format("%d", res_color(RES_POIS));
        if (cast)
        {
            if (dispel_monsters(_BOOST(4)))
                device_noticed = TRUE;
        }
        break;
    }
    /* Misc */
    case EFFECT_POLY_SELF:
        if (name) return "变形";
        if (desc) return "它会使你发生变异。警告：你可能不会喜欢这结果！";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            if (get_check("这可能有风险。你确定吗？"))
            {
                do_poly_self();
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_ANIMATE_DEAD:
        if (name) return "操纵死尸";
        if (desc) return "当你使用它时，它会复活你附近的尸体和骸骨，并让它们成为你的宠物。";
        if (value) return format("%d", 750);
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (animate_dead(0, py, px))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SCARE_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "恐吓怪物";
        if (desc) return "它会尝试恐吓附近所有的可见怪物。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 10*pow);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            if (turn_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLEEP_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "催眠怪物";
        if (desc) return "它会尝试催眠附近所有的可见怪物。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (sleep_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLOW_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "减速怪物";
        if (desc) return "它会尝试使附近所有的可见怪物减速。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (slow_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_STASIS_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "冻结怪物";
        if (desc) return "它会尝试冻结附近所有的可见怪物。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 30*pow);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (stasis_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_EYE_HYPNO:
    {
        int pow = _extra(effect, effect->power*2);
        if (name) return "催眠";
        if (desc) return "它会尝试冻结并魅惑附近所有的可见怪物。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 125*pow);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            (void)stasis_monsters(_BOOST(pow));
            (void)charm_monsters(MIN(56, _BOOST(pow) / 2));
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSE_MONSTERS:
    {
        int pow = _extra(effect, effect->power*3);
        if (name) return "困惑怪物";
        if (desc) return "它会尝试使附近所有的可见怪物陷入混乱。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 15*pow);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (confuse_monsters(_BOOST(pow)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_FISHING:
        if (name) return "钓鱼";
        if (desc) return "它能让你在冒险的重压之下得到放松和解压（钓鱼）。";
        if (value) return format("%d", 100);
        if (cast)
        {
            int x, y;
            if (!get_rep_dir2(&dir)) return NULL;
            y = py+ddy[dir];
            x = px+ddx[dir];
            tsuri_dir = dir;
            if (!cave_have_flag_bold(y, x, FF_WATER))
            {
                msg_print("这里没有可以钓鱼的地方。");
                break;
            }
            else if (cave[y][x].m_idx)
            {
                char m_name[MAX_NLEN];
                monster_desc(m_name, &m_list[cave[y][x].m_idx], 0);
                msg_format("%^s挡住了你的路。", m_name);
                energy_use = 0;
                break;
            }
            set_action(ACTION_FISH);
            p_ptr->redraw |= (PR_STATE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_CHARGE:
        if (name) return "冲锋";
        if (desc) return "如果在骑乘状态下，你可以向一个选定的敌人发起冲锋，造成额外伤害。";
        if (value) return format("%d", 5000);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            bool charged = FALSE;
            /* For the lance activation, the player really should be riding.
               At the moment, only the Heavy Lance 'Impaler' has this effect. */
            if (!p_ptr->riding)
            {
                msg_print("你必须在骑乘状态下才能冲锋。");
                return NULL;
            }
            charged = rush_attack(7, NULL);
            if (!charged) return NULL;
        }
        break;
    case EFFECT_PIERCING_SHOT:
        if (name) return "穿透射击";
        if (desc) return "它会发射一道能穿透多个敌人的弩箭。";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            bool fired = FALSE;
            msg_print("");
            shoot_hack = SHOOT_PIERCE;
            fired = do_cmd_fire();
            shoot_hack = SHOOT_NONE;
            if (!fired) return NULL;
            device_known = TRUE;
        }
        break;
    case EFFECT_ENDLESS_QUIVER: /* should only be on a quiver ... */
        if (name) return "无尽箭袋";
        if (desc) return "你的箭袋将被普通弹药重新填满。";
        if (value) return format("%d", 1500);
        if (color) return format("%d", TERM_L_RED);
        if (cast)
        {
            obj_t forge = {0};
            int   tval = p_ptr->shooter_info.tval_ammo;

            if ((!tval) || (tval == TV_NO_AMMO)) tval = TV_ARROW;
            if (tval == TV_ANY_AMMO) tval = TV_BOLT;

            object_prep(&forge, lookup_kind(tval, SV_ARROW)); /* Hack: SV_ARROW == SV_BOLT == SV_PEBBLE */
            forge.number = MAX(0, MIN(50, quiver_capacity() - quiver_count(NULL)));
            obj_identify_fully(&forge);
            object_origins(&forge, ORIGIN_ENDLESS);

            if (!forge.number)
                msg_print("你的箭袋满了。");
            else
            {
                msg_print("你的箭袋重新装满了。");
                quiver_carry(&forge);
            }
        }
        break;
    case EFFECT_WALL_BUILDING:
        if (name) return "造墙术";
        if (desc) return "它会生成一道石墙。";
        if (value) return format("%d", 50000);
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            fire_beam(GF_MAKE_WALL, dir, 0);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_SLEEP_MONSTER:
    {
        int power = _extra(effect, 10 + effect->power);
        if (name) return "催眠单一怪物";
        if (desc) return "当你使用它时，它会催眠一个怪物。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", 10*power);
        if (color) return format("%d", TERM_BLUE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (sleep_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SLOW_MONSTER:
    {
        int power = _extra(effect, MAX(10, effect->power * 2 - 6));
        if (name) return "减速单一怪物";
        if (desc) return "当你使用它时，它会使一个怪物减速。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", 15 * power);
        if (color) return format("%d", TERM_SLATE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (slow_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_CONFUSE_MONSTER:
    {
//        int power = _extra(effect, MIN(98, MAX(21, effect->power * 7 - 52)));
        int power = _extra(effect, MIN(105, 25 + _power_curve_offset(75, effect->power + 75, 80)));
        if (name) return "困惑单一怪物";
        if (desc) return "当你使用它时，它会使一个怪物混乱。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", 15*power);
        if (color) return format("%d", res_color(RES_CONF));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (confuse_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_SCARE_MONSTER:
    {
        int power = _extra(effect, 10 + effect->power);
        if (name) return "恐吓单一怪物";
        if (desc) return "当你使用它时，它会恐吓一个怪物。";
        if (info) return format("强度 %d", _BOOST(power));
        if (value) return format("%d", 10*power);
        if (color) return format("%d", res_color(RES_FEAR));
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (fear_monster(dir, _BOOST(power)))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_POLYMORPH:
        if (name) return "变形";
        if (desc) return "当你使用它时，它会将一个怪物变成另一个怪物（变形术）。";
        if (value) return format("%d", 500);
        if (color) return format("%d", TERM_ORANGE);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (poly_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_STARLITE:
    {
        int dd = _extra(effect, 6 + effect->power / 10);
        if (name) return "星光";
        if (desc) return "当你使用它时，它会向随机的空旷方向发射出多道微弱的闪光射线。";
        if (value) return format("%d", 750);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            int num = damroll(5, 3);
            int y = 0, x = 0, k;
            int attempts;

            for (k = 0; k < num; k++)
            {
                attempts = 1000;
                while (attempts--)
                {
                    scatter(&y, &x, py, px, 4, 0);
                    if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                    if (!player_bold(y, x)) break;
                }
                project(0, 0, y, x, _BOOST(damroll(dd, 10)), GF_LITE_WEAK,
                          PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL);
            }
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_NOTHING:
        if (name) return "无";
        if (desc) return "这就是你的食物。";
        if (value) return format("%d", 1);
        if (cast)
            msg_print("真可惜……你正在浪费食物！");
        break;

    /* Bad Effects */
    case EFFECT_AGGRAVATE:
        if (name) return "激怒怪物";
        if (desc) return "它会激怒附近的怪物。";
        if (value) return format("%d", 100); /* This actually *can* be useful ... */
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            aggravate_monsters(0);
            device_known = TRUE;
        }
        break;
    case EFFECT_HEAL_MONSTER:
    {
        int dd = _extra(effect, 10 + effect->power / 10);
        if (name) return "治疗怪物";
        if (desc) return "当你使用它时，它会治愈一个怪物的生命值。";
        if (info) return format("治疗 %dd10", dd);
        if (value) return format("%d", dd / 2);
        if (cast)
        {
            bool old_target_pet = target_pet;
            target_pet = TRUE;
            if (!get_fire_dir(&dir))
            {
                target_pet = old_target_pet;
                return NULL;
            }
            target_pet = old_target_pet;
            if (heal_monster(dir, _BOOST(damroll(dd, 10))))
                device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_HASTE_MONSTER:
        if (name) return "加速怪物";
        if (desc) return "当你使用它时，它会加速一个怪物。";
        if (color) return format("%d", TERM_VIOLET);
        if (value) return format("%d", 15);
        if (cast)
        {
            bool old_target_pet = target_pet;
            target_pet = TRUE;
            if (!get_fire_dir(&dir))
            {
                target_pet = old_target_pet;
                return NULL;
            }
            target_pet = old_target_pet;
            if (speed_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_HASTE_MONSTERS:
        if (name) return "加速群体怪物";
        if (desc) return "当你使用它时，它会加速视线内所有的怪物。";
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            if (speed_monsters())
                device_noticed = TRUE;
        }
        break;
    case EFFECT_CLONE_MONSTER:
        if (name) return "克隆怪物";
        if (desc) return "当你使用它时，它会克隆一个非唯一（非暗金）怪物。";
        if (value) return format("%d", 10);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            if (clone_monster(dir))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_DARKNESS:
        if (name) return "黑暗";
        if (desc) return "当你使用它时，它会使附近区域或当前房间变暗，并使你致盲。";
        if (color) return format("%d", TERM_L_DARK);
        if (cast)
        {
            if (!res_save_default(RES_BLIND) && !res_save_default(RES_DARK))
            {
                if (set_blind(p_ptr->blind + 3 + randint1(5), FALSE))
                    device_noticed = TRUE;
            }
            if (unlite_area(10, 3))
                device_noticed = TRUE;
        }
        break;
    case EFFECT_SUMMON_ANGRY_MONSTERS:
        if (name) return "召唤";
        if (desc) return "当你使用它时，它会召唤几个怪物作为你的敌人。";
        if (color) return format("%d", TERM_RED);
        if (cast)
        {
            int i;
            int num = randint1(4);
            for (i = 0; i < num; i++)
            {
                if (summon_specific(SUMMON_WHO_PLAYER, py, px, dun_level, 0, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET)))
                    device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SLOWNESS:
        if (name) return "迟缓";
        if (desc) return "当你使用它时，它会使你暂时减速。";
        if (color) return format("%d", TERM_UMBER);
        if (cast)
        {
            if (set_slow(p_ptr->slow + randint1(30) + 15, FALSE))
                device_noticed = TRUE;
        }
        break;

    /* Specific Artifacts ... Try to minimize! */
    case EFFECT_JEWEL:
        if (name) return "透视与召回";
        if (desc) return "它能为整个楼层绘制地图、永久照亮并探测所有物品。";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_VIOLET);
        if (cast)
        {
            virtue_add(VIRTUE_KNOWLEDGE, 1);
            virtue_add(VIRTUE_ENLIGHTENMENT, 1);
            wiz_lite(p_ptr->tim_superstealth > 0);
            msg_print("这颗宝石吸取了你的生命力……");
            take_hit(DAMAGE_LOSELIFE, damroll(3, 8), "审判宝石");
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
            if (get_check("激活召回？"))
                word_of_recall(TRUE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_HERMES:
        if (name) return "加速与任意门";
        if (desc) return "它能为你加速，并将你传送到附近一个选定的位置。";
        if (value) return format("%d", 15000);
        if (cast)
        {
            if (set_fast(_BOOST(randint1(75) + 75), FALSE)) device_noticed = TRUE;
            if (dimension_door(_BOOST(p_ptr->lev / 2 + 10))) device_noticed = TRUE;
        }
        break;
    case EFFECT_ARTEMIS:
        if (name) return "制造箭矢";
        if (desc) return "它能凭空用魔法变出箭矢！";
        if (value) return format("%d", 7500);
        if (cast)
        {
            object_type forge;
            char o_name[MAX_NLEN];

            object_prep(&forge, lookup_kind(TV_ARROW, m_bonus(1, p_ptr->lev)+ 1));
            forge.number = (byte)rand_range(5, 10);
            apply_magic(&forge, p_ptr->lev, AM_NO_FIXED_ART);
            obj_identify(&forge);
            object_origins(&forge, ORIGIN_ACQUIRE);
            
            forge.discount = 99;

            object_desc(o_name, &forge, 0);
            msg_format("它制造出了%s。", o_name);

            pack_carry(&forge);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_DEMETER:
        if (name) return "德墨忒尔之焰";
        if (desc) return "它能治愈你的生命值并填饱你的肚子。";
        if (info) return info_heal(0, 0, _BOOST(500));
        if (value) return format("%d", 10000);
        if (cast)
        {
            if (hp_player(_BOOST(500))) device_noticed = TRUE;
            if (set_food(PY_FOOD_MAX - 1)) device_noticed = TRUE;
        }
        break;
    case EFFECT_EYE_VECNA:
        if (name) return "维克那之眼";
        if (desc) return "它能打开你的天眼，让你看清周围的一切。";
        if (value) return format("%d", 10000);
        if (cast)
        {
            take_hit(DAMAGE_LOSELIFE, damroll(8, 8), "维克那之眼");
            wiz_lite(TRUE);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_ONE_RING:
        if (name) return "怪异之物";
        if (desc) return "它会产生一些完全不可预测的效果，并且可能非常糟糕。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            if (!get_fire_dir(&dir)) return NULL;
            ring_of_power(dir);
            device_noticed = TRUE;
        }
        break;
    case EFFECT_BLADETURNER:
        if (name) return "英雄气概、全抗性与元素吐息";
        if (desc) return "它能赋予你临时的英雄气概、祝福和元素抗性，并允许你喷吐元素能量。";
        if (value) return format("%d", 10000);
        if (cast)
        {
            int dur;
            if (!get_fire_dir(&dir)) return NULL;
            msg_print("你喷吐出元素。");

            fire_ball(GF_MISSILE, dir, _BOOST(300), 4);
            device_noticed = TRUE;

            msg_print("你的盔甲闪烁着五颜六色的光芒……");

            dur = _BOOST(randint1(50) + 50);

            set_hero(dur, FALSE);
            set_blessed(dur, FALSE);
            set_oppose_base(dur, FALSE);
        }
        break;
    case EFFECT_MITO_KOUMON:
        if (name) return "揭示身份";
        if (desc) return "它会揭示你的真实身份（真实性格）。";
        if (value) return format("%d", 1000);
        if (cast)
        {
            int count = 0, i;
            monster_type *m_ptr;
            cptr kakusan = "";

            if (summon_named_creature(0, py, px, MON_SUKE, PM_FORCE_PET))
            {
                msg_print("助三出现了。");
                kakusan = "助三";
                count++;
            }
            if (summon_named_creature(0, py, px, MON_KAKU, PM_FORCE_PET))
            {
                msg_print("格三出现了。");
                kakusan = "格三";
                count++;
            }
            if (!count)
            {
                for (i = m_max - 1; i > 0; i--)
                {
                    m_ptr = &m_list[i];
                    if (!m_ptr->r_idx) continue;
                    if (!((m_ptr->r_idx == MON_SUKE) || (m_ptr->r_idx == MON_KAKU))) continue;
                    if (!los(m_ptr->fy, m_ptr->fx, py, px)) continue;
                    if (!projectable(m_ptr->fy, m_ptr->fx, py, px)) continue;
                    count++;
                    break;
                }
            }
            if (count)
            {
                msg_format("%^s说：‘你以为这位大人是谁！还不快快跪下叩头！’", kakusan);

                sukekaku = TRUE;
                stun_monsters(15);
                confuse_monsters(120);
                turn_monsters(120);
                stasis_monsters(120);
                sukekaku = FALSE;
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_BLOODY_MOON:
        if (name) return "改变属性";
        if (desc) return "它会改变它自身带有的杀戮(slays)属性和抗性。";
        if (value) return format("%d", 5000);
        if (cast)
        {
            /* TODO: Again, we need the underlying object ...
               For now, we safely assume the artifact is Bloody Moon. */
            int slot = equip_find_art(ART_BLOOD);
            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                get_bloody_moon_flags(o_ptr);
                obj_identify_fully(o_ptr);
                obj_display(o_ptr);
                if (p_ptr->prace == RACE_ANDROID) android_calc_exp();
                p_ptr->update |= (PU_BONUS | PU_HP);
                device_noticed = TRUE;
            }
        }
        break;
    case EFFECT_SACRED_KNIGHTS:
        if (name) return "驱除诅咒与探知";
        if (desc) return "它能解除你装备上的所有普通诅咒，并探明附近的怪物。";
        if (value) return format("%d", 5000);
        if (cast)
        {
            if (remove_all_curse())
            {
                msg_print("你感觉好像有人在守护着你。");
                device_noticed = TRUE;
            }
            if (probing())
                device_noticed = TRUE;
        }
        break;
    case EFFECT_GONG:
    {
        int dam = _extra(effect, 3*effect->power);
        if (name) return "敲锣";
        if (desc) return "它会发出非常巨大的噪音。";
        if (info) return info_damage(0, 0, _BOOST(dam));
        if (value) return format("%d", 50*dam);
        if (cast)
        {
            if (!res_save_default(RES_SOUND))
                project(-1, 0, py, px, _BOOST(dam), GF_SOUND, PROJECT_KILL | PROJECT_HIDE);
            project(0, 18, py, px, _BOOST(dam*2), GF_SOUND, PROJECT_KILL | PROJECT_ITEM);
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_MURAMASA:
        if (name) return "增强力量";
        if (desc) return "它会尝试提升你的力量，但如果失败就会被摧毁。";
        if (value) return format("%d", 5000);
        if (cast)
        {
            if (get_check("你确定吗？！"))
            {
                msg_print("");
                do_inc_stat(A_STR);
                if (one_in_(2))
                {
                    /* TODO: We need to know the exact equipment slot being used so
                        we can destroy it. For now, we assume the artifact is Muramasa.
                        Note: Effects might someday be triggered by spells, so passing
                        an object to this routine won't always make sense!
                     */
                    int slot = equip_find_art(ART_MURAMASA);
                    if (slot)
                    {
                        msg_print("村正被摧毁了！");
                        curse_weapon(TRUE, slot);
                    }
                }
            }
        }
        break;
    case EFFECT_EXPERTSEXCHANGE: /* adapted from Frogspawn */
        if (name) return "改变性别";
        if (desc) return "它能改变你的生理性别。";
        if (value) return format("%d", 100);
        if (cast)
        {
            if ((prace_is_(RACE_MON_POSSESSOR)) || (prace_is_(RACE_MON_MIMIC)))
            {
                msg_print("什么也没发生。也许你应该直接换一具身体试试？");
                break;
            }
            p_ptr->psex = (SEX_MALE + SEX_FEMALE) - p_ptr->psex;
            if (p_ptr->psex == SEX_FEMALE)
            {
                mut_lose(MUT_IMPOTENCE);
                take_hit(DAMAGE_NOESCAPE, 10, "性别重置手术");
                msg_print("恭喜你！你现在是女性了！");
                switch (randint0(7))
                {
                    case 0: msg_print("(好吧，还挺快的，而且也不疼。不怎么疼。)"); break;
                    case 1: msg_print("(是时候开始大显身手了。)"); break;
                    case 2: msg_print("(你已经开始讨厌那些大男子主义的沙文猪了。)"); break;
                    case 3: msg_print("(你内心残存的那个肮脏男人已经开始动歪脑筋了……)"); break;
                    case 4: msg_print("(终于，你能够清晰地思考了，而没有那些奇怪的荷尔蒙在你体内流窜。)"); break;
                    case 5: msg_print("(不公平的性别！你可是期待已久了！)"); break;
                    default: msg_print("(你有点想念以前的身体，但你从来不怕做实验。)"); break;
                }
            }
            else
            {
                if ((one_in_(12)) && (p_ptr->prace != RACE_ENT))
                {
                    msg_print("恭喜你！你现在是个树人了！");
                    msg_print(NULL);
                    msg_print("不，开玩笑的。你现在是男性了。");
                    break;
                }
                else msg_print("恭喜你！你现在是男性了！");
                switch (randint0(7))
                {
                    case 0: msg_print("(好吧，还挺快的。现在轮到你让他们受苦了。)"); break;
                    case 1: msg_print("(你感觉睾酮在你体内流淌。是时候劫掠一番了！)"); break;
                    case 2: msg_print("(你现在要去对谁发号施令呢？)"); break;
                    case 3: msg_print("吼啊啊啊啊啊啊！"); break;
                    case 4: msg_print("你感觉非常强壮！(也许这只是你的错觉？要么就是你的属性还没正确更新。)"); break;
                    case 5: msg_print("(你希望其他雄性现在能停止纠缠你了。)"); break;
                    default: msg_print("(现在，你每赚八十个金币，他们最好付给你一百个。)"); break;
                }
            }
            device_noticed = TRUE;
        }
        break;
    case EFFECT_STUNNING_KICK:
    {
        int pow = effect->power * 2;
        if (name) return "致晕践踏";
        if (desc) return "用一次强力践踏强行震慑一个相邻的怪物。";
        if (info) return format("强度 %d", pow);
        if (value) return format("%d", 125*MAX(0, pow - 25));
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)
        {
            if ((d_info[dungeon_type].flags1 & DF1_NO_MELEE) || (no_melee_challenge))
            {
                msg_print("有什么东西阻止了你攻击。");
                break;
            }
            else
            {
                int x = 0, y = 0, m_idx = 0;

                if (!get_adjacent_target(&x, &y, &m_idx))
                {
                    break;
                }

                (void)project(0, 0, y, x, pow, GF_STUN, PROJECT_STOP | PROJECT_KILL | PROJECT_THRU);
            }
            device_noticed = TRUE;
        }
        break;
    }
    case EFFECT_RAMA_ARROW:
    {
        if (name) return "强力之箭";
        if (desc) return "射出一支带有额外威力的箭，造成三倍于正常水平的伤害。";
        if (info) return "伤害 x3";
        if (value) return format("%d", 10000);
        if (color) return format("%d", TERM_L_GREEN);
        if (cast)                         {
            if (melee_challenge)
            {
                msg_print("你想起了你仅依赖近战的庄严誓言。");
                break;
            }
            else
            {
                shoot_hack = SHOOT_RAMA;
                command_cmd = 'f'; /* hack for inscriptions */
                do_cmd_fire();
                shoot_hack = 0;
                device_noticed = TRUE;
            }
        }
        break;
    }
    case EFFECT_UNFOCUS_RAGE:
    {
        int pow = _extra(effect, effect->power * 2);
        if (name) return "狂怒解除";
        if (desc) return ((p_ptr->pclass == CLASS_RAGE_MAGE) ? "消除恐惧，解除狂暴力量，并将最多200点法力值(SP)转化为生命值(HP)。" : "消除恐惧并解除狂暴力量。");
        if ((info) && (p_ptr->pclass == CLASS_RAGE_MAGE)) return format("强度 %d", pow);
        if (value) return format("%d", 2500);
        if (color) return format("%d", TERM_YELLOW);
        if (cast)
        {
            if (p_ptr->afraid)
            {
                fear_clear_p();
                device_noticed = TRUE;
            }
            if (set_shero(0, TRUE)) device_noticed = TRUE;
            if ((p_ptr->csp > 0) && (p_ptr->pclass == CLASS_RAGE_MAGE))
            {
                int healing = MIN(pow, p_ptr->csp);
                hp_player(healing);
                sp_player(-healing);
                device_noticed = TRUE;
            }
        }
        break;
    }
    default:
        if (name) return format("无效的效果：%d", effect->type);
    }
    return "";
}
#undef _BOOST

