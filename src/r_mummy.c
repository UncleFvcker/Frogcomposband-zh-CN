#include "angband.h"

/* Largely based on Maledicts from PosChengband R */

static int _curse_boost = 0;
static int _curse_boost_capped = 0;
static int _curse_boost_removable = 0;

static byte UglyBitTable[256] = {
0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 
2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 
4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 
3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 
4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

typedef struct
{
    u32b flag;
    int lev;
    char desc[25];
    byte attr;
    char *help;
} _curse_type;

#define _MAX_MUMMY_CURSE 25

static _curse_type _mummy_curses[_MAX_MUMMY_CURSE] =
{
    { OFC_NORMALITY, 10, "驱散魔法", TERM_L_BLUE, "力量等级为1的轻度诅咒。偶尔会驱散临时的状态增益。" },
    { OFC_CALL_ANIMAL, 10, "召唤动物", TERM_GREEN, "力量等级为1的轻度诅咒。偶尔会召唤出敌对的动物。" },
    { OFC_COWARDICE, 10, "懦弱", TERM_YELLOW, "力量等级为1的轻度诅咒。偶尔会让你感到非常害怕。" },
    { OFC_CATLIKE, 10, "猫之步", TERM_RED, "力量等级为1的轻度诅咒。降低你的潜行能力。" },
    { OFC_FAST_DIGEST, 10, "快速消化", TERM_ORANGE, "力量等级为1的轻度诅咒。增加你的食物消耗速度。" },
    { OFC_OPEN_WOUNDS, 10, "开放性伤口", TERM_RED, "力量等级为1的轻度诅咒。降低割伤的愈合速度。" },
    { OFC_ADD_L_CURSE, 20, "增加弱诅咒", TERM_WHITE, "力量等级为1的轻度诅咒。偶尔会给你的装备增加新的轻度诅咒。" },
    { OFC_LOW_AC, 20, "低护甲", TERM_L_RED, "力量等级为1的轻度诅咒。降低你的护甲等级(AC)。" },
    { OFC_LOW_MELEE, 20, "攻击失误", TERM_L_GREEN, "力量等级为1的轻度诅咒。降低你的近战命中率。" },
    { OFC_DRAIN_HP, 20, "吸取生命", TERM_ORANGE, "力量等级为1的轻度诅咒。偶尔会吸取你的生命值。" },
    { OFC_DRAIN_MANA, 20, "吸取法力", TERM_L_BLUE, "力量等级为1的轻度诅咒。偶尔会吸取你的法力值。" },
    { OFC_DRAIN_PACK, 20, "吸取背包", TERM_GREEN, "力量等级为1的轻度诅咒。偶尔会吸取你魔法装置的能量。" },
    { OFC_LOW_MAGIC, 20, "诱发法术失败", TERM_YELLOW, "力量等级为1的轻度诅咒。增加你的法术失败率。" },
    { OFC_LOW_DEVICE, 20, "诱发装置失败", TERM_YELLOW, "力量等级为1的轻度诅咒。增加你的魔法装置使用失败率。" },
    { OFC_CALL_DEMON, 30, "召唤恶魔", TERM_L_RED, "力量等级为2的重度诅咒。偶尔会召唤出敌对的恶魔。" },
    { OFC_CALL_DRAGON, 30, "召唤龙", TERM_L_GREEN, "力量等级为2的重度诅咒。偶尔会召唤出敌对的龙。" },
    { OFC_TELEPORT, 30, "随机传送", TERM_L_BLUE, "力量等级为2的重度诅咒。偶尔会将你随机传送。" },
    { OFC_DRAIN_EXP, 30, "吸取经验", TERM_YELLOW, "力量等级为2的重度诅咒。偶尔会吸取你的经验值。" },
    { OFC_ADD_H_CURSE, 30, "增加重度诅咒", TERM_BLUE, "力量等级为2的重度诅咒。偶尔会给你的装备增加新的重度诅咒。" },
    { OFC_CRAPPY_MUT, 30, "诱发变异", TERM_L_RED, "力量等级为2的重度诅咒。偶尔会引发有害的变异。" },
    { OFC_AGGRAVATE, 40, "激怒", TERM_RED, "力量等级为3的重度诅咒。让你处于激怒怪物的状态。" },
    { OFC_DANGER, 40, "招惹危险", TERM_L_RED, "力量等级为3的重度诅咒。导致你会遇到更危险的怪物。" },
    { OFC_BY_CURSE, 40, "弱邪恶诅咒", TERM_PINK, "力量等级为3的重度诅咒。偶尔会触发弱邪恶诅咒效果。" },
    { OFC_TY_CURSE, 45, "*远古邪恶诅咒*", TERM_VIOLET, "力量等级为4的重度诅咒。偶尔会触发远古邪恶诅咒效果。" },
    { OFC_PERMA_CURSE, 50, "永久诅咒", TERM_VIOLET, "力量等级为1的永久诅咒。除了将物品变为世俗物品外无法被移除。防止普通诅咒和重度诅咒被移除或被吸取，但不防止特定诅咒的移除。" },
};

/* The first two are just the flaggy mask; Allergy has no effect on mummies
 * anyway; Slow Regen, well, not ignoring it might just encourage some poor
 * soul to actually put it on something... */
#define _IGNORE_MASK \
    (OFC_TELEPORT_SELF | OFC_CHAINSWORD | OFC_SLOW_REGEN | OFC_ALLERGY)

/* Not sure why we don't just use count_bits()... this *is* somewhat faster
 * with a high number of curses, but count_bits() is "fast enough" */
static int _count_curses(u32b flg)
{
    int tulos = 0;
    while (flg)
    {
        tulos += UglyBitTable[(flg & 0xff)];
        flg>>=8;
    }
    return tulos;
}

static int _get_toggle(void)
{
    return p_ptr->magic_num1[0];
}

static int _set_toggle(s32b toggle)
{
    int result = p_ptr->magic_num1[0];

    if (toggle == result) return result;

    p_ptr->magic_num1[0] = toggle;

    p_ptr->redraw |= PR_STATUS;
    p_ptr->update |= PU_BONUS;
    handle_stuff();
    return result;
}


static void _toggle_spell(int which, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_get_toggle() == which)
            _set_toggle(TOGGLE_NONE);
        else
            _set_toggle(which);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void _nether_ball_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 / 2 + 39 + (_curse_boost_capped * 8) + p_ptr->to_d_spell);
    int rad = spell_power(p_ptr->lev / 20 + 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "地狱球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的地狱球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_NETHER, dir, dam, rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static byte _boost_cap(void)
{
    return 5 + (p_ptr->lev / 5);
}

static bool _purge_curse_which(object_type *o_ptr)
{
    char o_name[MAX_NLEN];

    object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_NAME_ONLY | OD_OMIT_PREFIX);

    if ((o_ptr->curse_flags & OFC_PERMA_CURSE) && (p_ptr->lev >= 25))
    {
        if (o_ptr->curse_flags == (o_ptr->curse_flags & (OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED)))
        {
            msg_format("你的%s没有可移除的诅咒。", o_name, object_plural(o_ptr) ? "有" : "有");
            return FALSE;
        }
        o_ptr->curse_flags = OFC_PERMA_CURSE;
        o_ptr->known_curse_flags = OFC_PERMA_CURSE; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("你%s上的诅咒是永久的。较弱的诅咒已被剥离。", o_name);
        return TRUE;
    }
    else if (!mummy_can_remove(o_ptr))
    {
        msg_format("你不够强大，无法解开这件物品的诅咒。");
        return FALSE;
    }
    else if (o_ptr->curse_flags & OFC_HEAVY_CURSE)
    {
        o_ptr->curse_flags = 0;
        o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("你感觉一个重度诅咒从你的%s上解除了。", o_name);
        return TRUE;
    }
    else if (o_ptr->curse_flags & OFC_CURSED)
    {
        o_ptr->curse_flags = 0;
        o_ptr->known_curse_flags = 0; /* Forget lore in preparation for next cursing */
        o_ptr->ident |= IDENT_SENSE;
        o_ptr->feeling = FEEL_NONE;
        p_ptr->update |= PU_BONUS;
        p_ptr->window |= PW_EQUIP;
        p_ptr->redraw |= PR_EFFECTS;
        msg_format("你感觉一个诅咒从你的%s上解除了。", o_name);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool _purge_curse(void){

    obj_prompt_t prompt = {0};
    prompt.prompt = "解咒哪件物品？";
    prompt.error = "没有什么可以解咒的。";
    prompt.filter = object_is_cursed;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    obj_prompt(&prompt);
    if ((!prompt.obj) || (!prompt.obj->number)) return FALSE;
    _purge_curse_which(prompt.obj);
    
    return TRUE;
}

static void _mummy_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    static char _mummy_menu_options[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int idx = ((int*)cookie)[which];
    switch (cmd)
    {
    case MENU_KEY:
    {
        var_set_int(res, _mummy_menu_options[which]);
        break;
    }
    case MENU_TEXT:
    {
        var_set_string(res, format("%-20.20s (等级 %2d)", _mummy_curses[idx].desc, _mummy_curses[idx].lev));
        break;
    }
    case MENU_COLOR:
    {
        int attr = TERM_L_BLUE;
        if (p_ptr->cursed & _mummy_curses[idx].flag) attr = TERM_SLATE;
        else if (_mummy_curses[idx].lev > 40) attr = TERM_VIOLET;
        else if (_mummy_curses[idx].lev > 30) attr = TERM_L_RED;
        else if (_mummy_curses[idx].lev > 20) attr = TERM_ORANGE;
        else if (_mummy_curses[idx].lev > 10) attr = TERM_YELLOW;
        var_set_int(res, attr);
        break;
    }
    case MENU_HELP:
    {
        char buf[255];
        strcpy(buf, _mummy_curses[idx].help);
        var_set_string(res, buf);
        break;
    }
    case MENU_COLUMN:
    {
        var_set_int(res, (which >= Term->hgt - 6) ? 44 : 8);
        break;
    }
    default:
        default_menu(cmd, which, cookie, res);
    }
}

static bool _mummy_pick_curse(object_type *o_ptr)
{
    int choices[_MAX_MUMMY_CURSE];
    int i, ct = 0;
    menu_t menu = { "增加哪种诅咒？", "浏览哪种诅咒？", NULL,
                    _mummy_menu_fn, choices, 0, Term->hgt - 6};

    if ((!o_ptr) || (!o_ptr->k_idx)) return FALSE;

    for (i = 0; i < _MAX_MUMMY_CURSE; i++)
    {
        _curse_type *_curse = &_mummy_curses[i];
        if ((_curse->flag == OFC_LOW_MELEE) && (!object_is_melee_weapon(o_ptr))) continue;
        if ((_curse->flag == OFC_LOW_AC) && (!object_is_armour(o_ptr))) continue;
        if (_curse->lev > p_ptr->lev) continue;
        if (o_ptr->curse_flags & _curse->flag) continue;
        choices[ct++] = i;
    }

    if (ct == 0)
    {
        msg_print("目前你无法再给这件物品增加任何诅咒。");
        return FALSE;
    }

    menu.count = ct;

    for (;;)
    {
        i = menu_choose(&menu);
        if (i >= 0)
        {
            char o_name[MAX_NLEN];
            int idx = choices[i];
            _curse_type *_curse = &_mummy_curses[idx];
            char c, buf[256];
            strcpy(buf, format("Add the curse of <color:%c>%s</color>? <color:y>[y/n]</color>", attr_to_attr_char(_curse->attr), _curse->desc));
            c = msg_prompt(buf, "ny", PROMPT_NEW_LINE | PROMPT_ESCAPE_DEFAULT);
            if (c == 'n') continue;

            object_desc(o_name, o_ptr, OD_COLOR_CODED | OD_OMIT_PREFIX | OD_NAME_ONLY);
            o_ptr->curse_flags |= _curse->flag;
            o_ptr->known_curse_flags |= _curse->flag;

            if (_curse->lev >= 30)
            {
                msg_format("一股可怕的黑色光环冲击了你的%s！", o_name);
                o_ptr->curse_flags |= (OFC_HEAVY_CURSE | OFC_CURSED);
                o_ptr->known_curse_flags |= (OFC_HEAVY_CURSE | OFC_CURSED);
            }
            else
            {
                msg_format("一股黑色光环包围了你的%s！", o_name);
                o_ptr->curse_flags |= (OFC_CURSED);
                o_ptr->known_curse_flags |= (OFC_CURSED);
            }
            if ((!object_is_known(o_ptr)) && (o_ptr->ident & IDENT_SENSE))
            {
                o_ptr->feeling = value_check_aux1(o_ptr, TRUE);
            }
            p_ptr->update |= PU_BONUS;
            p_ptr->window |= (PW_EQUIP | PW_INVEN);
            p_ptr->redraw |= PR_EFFECTS;
            return TRUE;
        }
        return FALSE;
    }
}

static bool _curse_item_aux(void)
{
    obj_prompt_t prompt = {0};
    prompt.prompt = "诅咒哪件物品？";
    prompt.error = "没有什么可以诅咒的。";
    prompt.filter = object_is_equipment;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;

    obj_prompt(&prompt);
    if (!prompt.obj || !prompt.obj->number) return FALSE;
    if (!_mummy_pick_curse(prompt.obj)) return FALSE;

    p_ptr->update |= (PU_BONUS);
    p_ptr->window |= (PW_INVEN | PW_EQUIP);
    p_ptr->redraw |= (PR_EFFECTS);
    handle_stuff();
    return TRUE;
}

static void _purge_curse_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "清除诅咒");break;
    case SPELL_DESC: var_set_string(res, "移除单件物品上的诅咒。在更高等级时，你甚至可以驱散重度诅咒。"); break;
    case SPELL_CAST: var_set_bool(res, _purge_curse()); break;
    default: default_spell(cmd, res);break;
    }
}

static void _curse_item_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "诅咒物品"); break;
    case SPELL_DESC: var_set_string(res, "诅咒单件物品，消耗3个回合。"); break;
    case SPELL_CAST: var_set_bool(res,_curse_item_aux()); break;
    case SPELL_ENERGY: var_set_int(res, 300); break;
    default: default_spell(cmd, res);break;}
}

static int _curse_plev(void)
{
    switch (p_ptr->current_r_idx)
    {
        case MON_MUMMY_SORC:
            return (p_ptr->lev + 50) / 2;
        case MON_MUMMY_KING:
        case MON_GHOUL:
        case MON_GREATER_MUMMY:
            return p_ptr->lev;
        default:
            return MAX((p_ptr->lev + 1) / 2, p_ptr->lev - 10);
    }
    return 1;
}

static int _get_curse_rolls(int pow)
{
    int plev = _curse_plev();
    if (pow == 2) return 4 + (plev / 10) + (_curse_boost_capped / 3); // max 4 + 5 + 5 = 14
    else if (pow == 1) return 2 + (plev / 10) + (_curse_boost_capped * 4 / 15); // max 2 + 5 + 4 = 11
    return 2 + (plev + 10) / 20 + (_curse_boost_capped / 5); // max 2 + 3 + 3 = 8
}


static bool _inflict_curse_aux(int pow, monster_type *m_ptr, int m_idx, bool DoDamage){
    int ct = 0; int p = 0;
    int highest_power = 0;
    int dType = -1;
    int refunds = 0;
    int dmg = 0;
    int rolls = 1;
    int plev = _curse_plev();
    byte nopat[15];

    char m_name[MAX_NLEN];
    monster_desc(m_name, m_ptr, 0);

    rolls = _get_curse_rolls(pow);
    if (one_in_(3)) rolls++;

    while (ct < rolls)
    {
        if (one_in_(666) && plev > 40) nopat[ct] = 13;
        else if (one_in_(66) && plev > 40) nopat[ct] = 11;
        else if (one_in_(22) && plev > 35) nopat[ct] = 9;
        else if (one_in_(18) && plev > 30) nopat[ct] = 7;
        else if (one_in_(15) && plev > 10) nopat[ct] = 6;
        else if (one_in_(12) && plev > 10) nopat[ct] = 5;
        else if (one_in_(10)) nopat[ct] = 4;
        else if (one_in_(8)) nopat[ct] = 3;
        else if (one_in_(6)) nopat[ct] = 2;
        else if (one_in_(4)) nopat[ct] = 1;
        else nopat[ct] = 0;
        if (nopat[ct] > highest_power) highest_power = MAX(highest_power, (nopat[ct] + 1) / 2);
        ct++;
    }

    switch (highest_power){
    case 1: msg_format("%^s 被诅咒了。", m_name); break;
    case 2: msg_format("一个邪恶的诅咒向 %s 伸出魔爪。", m_name); break;
    case 3: msg_format("%^s 受到了一种邪恶的诅咒。", m_name); break;
    case 4: msg_format("%^s 因诅咒而付出了沉重的代价。", m_name); break;
    case 5: msg_format("%^s 被一个强大的诅咒击中了！", m_name); break;
    case 6: msg_format("<color:D>黑色凶兆！</color> 可怕的命运正等待着 %s！", m_name); break;
    case 7: msg_format("死亡之手伸向了 %s！", m_name); break;
    default: msg_format("一圈微弱的黑色光环短暂地包围了 %s。", m_name); break;
    }

    ct = 0;
//    msg_format("Rolls: %d", rolls);
    while (ct < rolls)
    {
        if ((!m_ptr) || (!m_ptr->r_idx)) return TRUE;
        dmg = plev + _curse_boost_capped;
        dType = -1;
        switch (nopat[ct])
        {
            case 13: dType = GF_DEATH_RAY; dmg = spell_power(plev * 200); break;
            case 11: dType = GF_BLOOD_CURSE; break;
            case 9:  dType = GF_HAND_DOOM; break;
            case 7:  dType = GF_ANTIMAGIC; break;
            case 6:  dType = GF_STASIS; break;
            case 5:  dType = GF_PARALYSIS; break;
            case 4:  dType = GF_OLD_CONF; break;
            case 3: dType = GF_STUN; break;
            case 2: dType = GF_TURN_ALL; break;
            case 1: dType = GF_OLD_SLOW; break;
            default: break;
        }
        p = (nopat[ct] + 1) / 2;
        ct++;

//        msg_format("Roll %d: %d", ct, nopat[ct - 1]);

        if (r_info[m_ptr->r_idx].flags1 & RF1_UNIQUE){ // if it is an unique, give some refund for high-powered ones...
            if (p == 5 || p == 3 || p == 7){ refunds++; continue; }
        }
        if (dType > 0){
            u32b liput = PROJECT_KILL | PROJECT_HIDE | PROJECT_JUMP;
            if ((ct < rolls) || (DoDamage)) liput |= PROJECT_NO_PAIN;
            project(0, 0, m_ptr->fy, m_ptr->fx, dmg, dType, liput);
        }
    }
    // In addition to these things, we also have some damage
    if ((DoDamage) && (m_ptr) && (m_ptr->r_idx))
    {
        dmg = (plev / 2) * refunds + plev * (pow + 1) + _curse_boost_capped;
        project(0, 0, m_ptr->fy, m_ptr->fx, dmg, GF_NETHER, (PROJECT_KILL | PROJECT_HIDE | PROJECT_JUMP));
    }

    return TRUE;
}

static bool _inflict_curse(int pow){ 
    int m_idx = 0;
    monster_type *m_ptr;
    char m_name[MAX_NLEN];

    if (!get_direct_target()) return FALSE;

    m_idx = cave[target_row][target_col].m_idx;
    if (!m_idx) return FALSE;
    if (m_idx == p_ptr->riding) return FALSE;
    if (!player_has_los_bold(target_row, target_col)) return FALSE;
    m_ptr = &m_list[m_idx];

    if ((m_ptr) && (m_ptr->r_idx))
    {
        monster_desc(m_name, m_ptr, 0);

        if(pow==0) msg_format("<color:R>你诅咒了 %s。</color>", m_name);
        else if (pow == 1) msg_format("<color:R>你诅咒了 %s。</color>", m_name);
        else msg_format("<color:R>你诅咒了 %s。</color>", m_name);
        _inflict_curse_aux(pow, m_ptr, m_idx, TRUE);
        energy_use = 100;
        return TRUE;
    }

    return FALSE;
}

static bool _blasphemy(void)
{
    int i, afflicted = 0;
    monster_type *m_ptr;
    msg_print("你吐出了一个古老而可怕的词语。");

    for (i = 1; i < m_max; i++)
    {
        if (!m_list[i].r_idx) continue;
        m_ptr = &m_list[i];
        if (player_has_los_bold(m_ptr->fy, m_ptr->fx)){ // Not seen
            _inflict_curse_aux(1, m_ptr, i, FALSE);
            afflicted++;
        }

    }
    if (afflicted == 0){ msg_print("没有人听到……"); return FALSE; }
    energy_use = 100;
    return TRUE;
}

/* BLASPHEMY */
static void _blasphemy_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "亵渎"); break;
    case SPELL_INFO: var_set_string(res, format("%d*诅咒 + 伤害 0; 伤害 %dd%d",  _get_curse_rolls(1), _curse_plev(), 8)); break;
    case SPELL_DESC: var_set_string(res, "吐出一个被诅咒的词语，对视线内的所有怪物施加诅咒，并以自身为中心产生一个地狱球。"); break;
    case SPELL_CAST: 
        if (_blasphemy()){
            project(0, 4, py, px, damroll(_curse_plev(), 8), GF_NETHER, (PROJECT_FULL_DAM | PROJECT_KILL));
            var_set_bool(res, TRUE);
        } else var_set_bool(res, FALSE);
        break;
    default:default_spell(cmd, res); break;
    }
}

static int _curse_pow(int pow)
{
    return _curse_plev() * (pow + 1) + _curse_boost_capped;
}

/* MINOR CURSE */
static void _minor_curse(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "次级诅咒"); break;
    case SPELL_INFO: var_set_string(res, format("%d*诅咒 + 伤害 %d", _get_curse_rolls(0), _curse_pow(0))); break;
    case SPELL_DESC: var_set_string(res, "对单一怪物施加轻微的诅咒。"); break;
    case SPELL_COST_EXTRA: var_set_int(res, p_ptr->lev / 20); break;
    case SPELL_CAST: var_set_bool(res, _inflict_curse(0)); break;
    default:default_spell(cmd, res);break;}
}

/* CURSE */
static void _curse_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "邪恶诅咒"); break;
    case SPELL_INFO: var_set_string(res, format("%d*诅咒 + 伤害 %d", _get_curse_rolls(1), _curse_pow(1))); break;
    case SPELL_DESC: var_set_string(res, "对单一怪物施加诅咒。"); break;
    case SPELL_COST_EXTRA: var_set_int(res, MIN(4, p_ptr->lev / 10)); break;
    case SPELL_CAST: var_set_bool(res, _inflict_curse(1)); break;
    default:default_spell(cmd, res); break;}
}

/* MAJOR CURSE */
static void _major_curse(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "强力诅咒"); break;
    case SPELL_INFO: var_set_string(res, format("%d*诅咒 + 伤害 %d", _get_curse_rolls(2), _curse_pow(2))); break;
    case SPELL_DESC: var_set_string(res, "对单一怪物施加可怕的诅咒。"); break;
    case SPELL_COST_EXTRA: var_set_int(res, MIN(12, p_ptr->lev / 4)); break;
    case SPELL_CAST: var_set_bool(res,_inflict_curse(2)); break;
    default:default_spell(cmd, res); break;
    }
}
/*static void _sense_misfortune(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "Sense Misfortune"); break;
    case SPELL_DESC: var_set_string(res, "Senses monsters and traps in range. At high level, also maps the area."); break;
    case SPELL_INFO: var_set_string(res, info_radius(26 + _curse_boost_capped)); break;
    case SPELL_CAST: 
        msg_print("You attempt to sense misfortune... \n");
        detect_monsters_evil( 26 + _curse_boost_capped );
        detect_traps(26 + _curse_boost_capped, FALSE);
        if (p_ptr->lev > 40){ map_area(26 + _curse_boost_capped); }
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}*/
static void _curse_of_impotence(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "无力诅咒"); break;
    case SPELL_DESC: var_set_string(res, "用无力感诅咒所有生物。"); break;
    case SPELL_CAST: 
        num_repro += MAX_REPRO; 
        msg_print("你感到一种切实的节欲感在增加……");
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}

bool mummy_cast_antitele(void)
{
    variant res;
    var_init(&res);
    _toggle_spell(MUMMY_TOGGLE_ANTITELE, SPELL_CAST, &res);
    var_clear(&res);
    if (_get_toggle() == MUMMY_TOGGLE_ANTITELE) msg_print("空间中的一切都被封锁了。");
    else msg_print("次元锚定消失了。");
    return TRUE;
}

static void _dimensional_anchor(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "次元锁"); break;
    case SPELL_DESC: var_set_string(res, "将事物锁定在原地，防止几乎所有的传送。"); break;
    case SPELL_CAST:{
        var_set_bool(res, mummy_cast_antitele());
        break;
    }
    default:default_spell(cmd, res); break;
    }
}

static void _absorb_curse_pow(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "吸收诅咒力量"); break;
    case SPELL_DESC: var_set_string(res, "清除装备上的所有诅咒以治疗自己。在更高力量下，成功施放消耗的时间更少（最低降至0.25个回合）。"); break;
    case SPELL_INFO: var_set_string(res, info_heal(0, 0, _curse_boost_removable * 70)); break;
    case SPELL_CAST:{
        int old_cursepow = _curse_boost_removable;
        if (old_cursepow == 0){ msg_print("你身上没有携带任何可驱散的诅咒。"); var_set_bool(res, FALSE); break; }
        msg_print("你吸收了邪恶诅咒的力量！");
        hp_player(old_cursepow *70);
        remove_all_curse();
        if (old_cursepow >= 5)
        {
            set_stun(0, TRUE);
            set_cut(0, TRUE);
            set_blind(0, TRUE);
            energy_use = 100 - (old_cursepow * 5); // super-quick too.
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:default_spell(cmd, res); break;
    }
}

static void _drain_curse_pow(int cmd, variant *res)
{
    int pow = _curse_plev() / 2;
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "吸取诅咒力量"); break;
    case SPELL_DESC: var_set_string(res, "吸取被诅咒装备的能量以补充法力。"); break;
    case SPELL_INFO: var_set_string(res, format("力量 %d+1d%d", pow, pow)); break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        prompt.prompt = "吸取哪件物品？";
        prompt.error = "你没有可吸取的受诅咒装备。";
        prompt.filter = object_is_cursed;
        prompt.where[0] = INV_EQUIP;
        obj_prompt(&prompt);
        var_set_bool(res, FALSE);
        if ((!prompt.obj) || (!(prompt.obj->number))) break;

        if (_purge_curse_which(prompt.obj))
        {
            p_ptr->csp += (pow) + randint1(pow);
            if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;
            var_set_bool(res, TRUE);
        }
        else
        {
            var_set_bool(res, FALSE);
        }
        break;
    }
    default: default_spell(cmd, res); break;
    }
}

static void _umbra_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "暗影"); break;
    case SPELL_DESC: var_set_string(res, "将你笼罩在阴影中，使你更具隐蔽性，并减轻激怒效果。"); break;
    case SPELL_INFO: var_set_string(res, info_duration(p_ptr->lev / 3 + _curse_boost_capped * 3, p_ptr->lev / 3 + _curse_boost_capped * 3)); break;
    case SPELL_CAST:
        if (p_ptr->cur_lite > 0)
        {
            msg_print("你携带的光源太亮了，无法将自己隐藏在阴影中！");
            var_set_bool(res, FALSE);
        }
        set_tim_dark_stalker(spell_power(p_ptr->lev / 3 + _curse_boost_capped * 3 + randint1(p_ptr->lev / 3 + _curse_boost_capped * 3)), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:default_spell(cmd, res); break;
    }
}

static void _assess_curses_spell(int cmd, variant *res)
{
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "评估诅咒"); break;
    case SPELL_DESC: var_set_string(res, "分析环绕着你的邪恶附魔的力量。"); break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (_curse_boost < 1)
        {
            msg_print("你目前没有从被诅咒的装备中获得任何加成。");
            break;
        }
        else
        {
            int hat = _boost_cap();
            if (_curse_boost_capped == hat)
            {
                msg_format("你装备上的诅咒总力量为 <color:o>%d</color>，为你提供了 <color:o>等级 %d</color> 的提升，这是%s所能达到的最高加成。", _curse_boost, _curse_boost_capped, (p_ptr->lev == 50) ? "" : "目前");
            }
            else
            {
                if (p_ptr->cur_lite > 0) msg_format("你装备上诅咒的标称总力量为 <color:o>%d</color>，但这被你周围永久的光源削弱了。你获得了 <color:o>等级 %d</color> 的提升。", _curse_boost, _curse_boost_capped);
                else msg_format("你装备上诅咒的总力量为 <color:o>%d</color>，为你提供了 <color:o>等级 %d</color> 的提升。若有更多诅咒，你的加成可达到 <color:o>等级 %d</color>。", _curse_boost, _curse_boost_capped, hat);
            }
            var_set_bool(res, TRUE);
        }
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:default_spell(cmd, res); break;
    }
}

static bool _unleash(void)
{    
    msg_print("所有的恶意都被释放了！");

    if(_curse_boost_removable >= 6) cast_destruction();
    if(_curse_boost_removable >= 5) project_hack(GF_BLOOD_CURSE, _curse_boost_removable * 25);
    project(0, 10, py, px, damroll(_curse_boost_removable, 25), GF_MANA, (PROJECT_FULL_DAM | PROJECT_KILL));
    hp_player(_curse_boost_removable * 50);
    set_stun(0, TRUE);
    set_cut(0, TRUE);
    set_blind(0, TRUE);

    remove_all_curse();
    energy_use = 100 - (_curse_boost_removable * 5);
    return TRUE;
}

static void _unleash_spell(int cmd, variant *res){
    switch (cmd){
    case SPELL_NAME: var_set_string(res, "释放恶意"); break;
    case SPELL_DESC: var_set_string(res, "释放你装备上诅咒的邪恶力量。在高力量下会摧毁周围区域。在更高力量下，成功施放消耗的时间更少（最低降至0.25个回合）。"); break;
    case SPELL_INFO: {
        if (_curse_boost_removable > 2){
            if (_curse_boost_removable >= 5)
            var_set_string(res, format("伤害 %dd25+%d; 治疗 %d", _curse_boost_removable, _curse_boost_removable * 25, _curse_boost_removable * 50));
            else var_set_string(res, format("伤害 %dd25; 治疗 %d", _curse_boost_removable, _curse_boost_removable * 50));
        }
        else var_set_string(res, "");
        break;
    }
    case SPELL_CAST:{ 
        if (_curse_boost_removable > 2) var_set_bool(res, _unleash());
        else
        {
            msg_print("你体内的恶意不足……");
            var_set_bool(res, FALSE);
        }
        break;
    }
    default:default_spell(cmd, res); break;
    }
}
static power_info _powers[] = {
    { A_CHR, { 3,  0, 20, _purge_curse_spell } },
    { A_CHR, { 10,  0, 35, _curse_item_spell } },
    { A_CHR, { 10,  0, 0, _assess_curses_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _draugr_powers[] = {
    { A_STR, { 36, 15, 30, building_up_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info _sorc_powers[] = {
    { A_CHR, { 36, 30, 30, _nether_ball_spell } },
    {    -1, { -1, -1, -1, NULL}}
};
static power_info *_get_powers(void) {
    static power_info spells[MAX_SPELLS];
    int max = MAX_SPELLS;
    int ct = get_powers_aux(spells, max, _powers, FALSE);
    if (p_ptr->current_r_idx == MON_DRAUGR)
        ct += get_powers_aux(spells + ct, max - ct, _draugr_powers, FALSE);
    else if (p_ptr->current_r_idx == MON_MUMMY_SORC)
        ct += get_powers_aux(spells + ct, max - ct, _sorc_powers, FALSE);
    spells[ct].spell.fn = NULL;
    return spells;
}

bool mummy_ty_protection(void){
    if (p_ptr->prace != RACE_MON_MUMMY) return FALSE;
    if (p_ptr->lev > 30){
        if (one_in_(2)) return TRUE;
    }
    return FALSE;
}

bool mummy_can_remove(object_type *o_ptr)
{
    if (p_ptr->prace != RACE_MON_MUMMY) return FALSE;
    if (o_ptr->curse_flags & OFC_PERMA_CURSE) return FALSE;
    if ((o_ptr->curse_flags & OFC_HEAVY_CURSE) && (p_ptr->lev < 25)) return FALSE;
    if (p_ptr->lev < 3) return FALSE;
    return TRUE;
}

int mummy_get_toggle(void)
{
    int result = TOGGLE_NONE;
    if (p_ptr->prace == RACE_MON_MUMMY)
        result = _get_toggle();
    return result;
}

static spell_info _get_spells[] =
{
    /*lvl cst fail spell */
    { 1, 8, 30, _minor_curse }, // debuff
//    { 5, 5, 40, _sense_misfortune }, // detect traps / monsters
    { 10, 15, 40, _curse_of_impotence }, // no breeding
    { 15, 8, 45,  _drain_curse_pow }, // attempts to remove curse from one equipment, restores mana based on curses.
    { 20, 20, 45, _umbra_spell}, // Stealth buff, cancels out aggravation
    { 24, 16, 45, _curse_spell}, // stronger debuff + damage
    { 28, 0, 0, _dimensional_anchor }, // -TELE on self and everyone on sight
    { 32, 20, 40, animate_dead_spell },
    { 36, 30, 45, _absorb_curse_pow }, // Remove curse & heal
    { 40, 110, 45, _blasphemy_spell },
    { 45, 48, 45, _major_curse }, // crippling debuff
    { 50, 100, 50, _unleash_spell }, // *remove curse, heal, LOS effects depending on the curse_power. Requires curses to be present.
    { -1, -1, -1, NULL }
};

/**********************************************************************
 * Mummy Equipment
 **********************************************************************/
static void _birth(void)
{
    object_type    forge;

    p_ptr->current_r_idx = MON_ZOMBIE_H;
    equip_on_change_race();
    skills_innate_init("爪击", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("撕咬", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("凝视", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("粉碎", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("猛击", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    object_prep(&forge, lookup_kind(TV_SOFT_ARMOR, SV_ROBE));
    py_birth_obj(&forge);
    py_birth_obj_aux(TV_STAFF, EFFECT_NOTHING, 1);
}

/**********************************************************************
 * Mummy Attacks
 **********************************************************************/

/* Not a physical attack, so undo physical bonuses */
void _gaze_adjustments(innate_attack_ptr a)
{
    a->to_d -= ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
    a->to_h -= ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);
    a->to_h -= ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
    if (IS_HERO()) a->to_h -= 12;
    if (IS_SHERO())
    {
        a->to_d -= 3+(p_ptr->lev/5);
        a->to_h -= 12;
    }
}

void _zombie_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Hit */
    {
        innate_attack_t    a = {0};

        a.dd = 3 + l / 15;
        a.ds = 3 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += 10 + _curse_boost_capped;

        a.effect[0] = GF_MISSILE;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You hit.";
        a.name = "猛击";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _ghoul_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 13;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;

        a.weight = 100;
        calc_innate_blows(&a, 164);
        a.blows += py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "爪击";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 2 + l / 12;
        a.ds = 2 + l / 12;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_PARALYSIS;
        a.effect_chance[1] = 25;

        a.weight = 100;
        calc_innate_blows(&a, 164);
        a.blows += py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You bite.";
        a.name = "撕咬";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _sorc_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 12;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;
        a.effect_chance[1] = 50;
        a.effect[2] = GF_DISENCHANT;
        a.effect_chance[2] = 25;
        a.effect[3] = GF_BLIND;
        a.effect_chance[3] = 10;

        a.weight = 100;
        calc_innate_blows(&a, 200);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "爪击";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT | INNATE_NO_DAM;

        a.weight = 150;
        a.effect[0] = GF_TURN_ALL;

        a.blows = 100 + py_prorata_level_aux(_curse_boost_capped*100/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "凝视";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _draugr_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 100;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 40;
        a.effect[2] = GF_DRAIN_MANA;
        a.effect_chance[2] = 20;

        a.blows = 100 + py_prorata_level_aux(_curse_boost_capped*5, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "凝视";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 2 + l / 9;
        a.ds = 2 + l / 9;
        a.to_d += _curse_boost_capped * 3 / 2;
        a.to_h += _curse_boost_capped * 3 / 2;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_POIS;

        a.weight = 150;
        calc_innate_blows(&a, 150);
        a.blows += py_prorata_level_aux(_curse_boost_capped*5, 1, 1, 0);
        a.msg = "You bite.";
        a.name = "撕咬";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Crush */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 9;
        a.ds = 2 + l / 8;
        a.to_d += _curse_boost_capped;
        a.to_h += 5 + _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_BABY_SLOW;
        a.effect_chance[1] = 40;
        a.effect[2] = GF_STUN;
        a.effect_chance[2] = 20;

        a.weight = 250;
        a.blows = 100;
        a.msg = "You crush.";
        a.name = "粉碎";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _gmum_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_OLD_DRAIN;
        a.effect_chance[1] = 50;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "爪击";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 150;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 50;

        a.blows = 200 + py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "凝视";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

void _king_innate_attacks(void)
{
    int l = p_ptr->lev;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 12;
        a.ds = 2 + l / 11;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;

        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_OLD_DRAIN;
        a.effect[2] = GF_BLIND;
        a.effect_chance[2] = 10;

        a.weight = 150;
        calc_innate_blows(&a, 242);
        a.blows += py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You claw.";
        a.name = "爪击";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Gaze */
    {
        innate_attack_t a = {0};

        a.dd = 1 + l / 11;
        a.ds = 1 + l / 10;
        a.to_d += _curse_boost_capped;
        a.to_h += _curse_boost_capped;
        _gaze_adjustments(&a);
        a.flags = INNATE_NO_CRIT;

        a.weight = 150;
        a.effect[0] = GF_MISSILE;
        a.effect[1] = GF_TURN_ALL;
        a.effect_chance[1] = 50;
        a.effect[2] = GF_DRAIN_MANA;
        a.effect_chance[2] = 25;

        a.blows = 200 + py_prorata_level_aux(_curse_boost_capped*121/15, 1, 1, 0);
        a.msg = "You gaze.";
        a.name = "凝视";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}
 
void _calc_innate_attacks(void)
{
    if ((p_ptr->weapon_ct > 0) || (!equip_find_empty_hand())) return;
    switch (p_ptr->current_r_idx)
    {
        case MON_ZOMBIE_H:
        case MON_MUMMY_H:
            _zombie_innate_attacks();
            break;
        case MON_GHOUL:
            _ghoul_innate_attacks();
            break;
        case MON_GREATER_MUMMY:
            _gmum_innate_attacks();
            break;
        case MON_DRAUGR:
            _draugr_innate_attacks();
            break;
        case MON_MUMMY_SORC:
            _sorc_innate_attacks();
            break;
        case MON_MUMMY_KING:
            _king_innate_attacks();
            break;
        default: break;
    }
}

static void _calc_bonuses_aux(void)
{
    int slot;
    u32b checklist = 0, perm_flags = 0;
    int basePow = 0;
    int perm_ct = 0;
    int osumat = 0;
    int boost_cap = _boost_cap();
    int boost = 0;
    int removable = 0;

    if (!p_ptr->cursed)
    {
        _curse_boost = 0;
        _curse_boost_capped = 0;
        _curse_boost_removable = 0;
        return; /* Easy */
    }

    for (slot = equip_find_first(object_is_cursed); slot; slot = equip_find_next(object_is_cursed, slot))
    {
        object_type *o_ptr = equip_obj(slot);
        u32b flgs[OF_ARRAY_SIZE];
        u32b liput = o_ptr->curse_flags;

        obj_flags(o_ptr, flgs);

        if (o_ptr->curse_flags & OFC_PERMA_CURSE)
        {
            basePow += 3;
            perm_ct += 3;
            liput &= ~(OFC_PERMA_CURSE | OFC_HEAVY_CURSE | OFC_CURSED);
        }
        else if (o_ptr->curse_flags & OFC_HEAVY_CURSE) basePow += 2;
        else if (o_ptr->curse_flags & OFC_CURSED) basePow++;
        if (obj_is_blessed(o_ptr)) basePow -= 2;
        if (have_flag(flgs, OF_TY_CURSE)) perm_flags |= OFC_TY_CURSE;
        if (have_flag(flgs, OF_AGGRAVATE)) perm_flags |= OFC_AGGRAVATE;
        if (have_flag(flgs, OF_DRAIN_EXP)) perm_flags |= OFC_DRAIN_EXP;
        checklist |= (liput);
    }
    checklist &= ~(_IGNORE_MASK);
    osumat = _count_curses(checklist);
    /* Special curses
     * Note: We don't reward perma-curses very heavily to prevent constant
     * high power from curses that aren't actually removed */
    if (checklist & OFC_TY_CURSE) osumat += 2;
    else if (perm_flags & OFC_TY_CURSE) boost += 4; /* 2 for TY bonus, 1 for extra curse and 1 for extra heavy curse */
    if (checklist & OFC_AGGRAVATE) osumat += 1;
    else if (perm_flags & OFC_AGGRAVATE) boost += 3;
    if (checklist & OFC_DANGER) osumat += 1;
    if (checklist & OFC_BY_CURSE) osumat += 1;
    if ((perm_flags & OFC_DRAIN_EXP) && (!(checklist & OFC_DRAIN_EXP))) boost += 2;
    boost += osumat + (basePow / 2);
    removable += osumat + ((basePow - perm_ct) / 2);
    if (perm_ct > 0)
    {
        /* Check for curses that were hidden but are actually present */
        boost++; /* Perma-curse itself is such a curse, it's always hidden! */
        if (!(checklist & OFC_HEAVY_CURSE)) boost++;
        if (!(checklist & OFC_CURSED)) boost++;
    }
    checklist &= (TRC_HEAVY_MASK);
    osumat = _count_curses(checklist);
    boost += osumat;
    removable += osumat;

    _curse_boost = boost; // save up the uncapped boost.
    boost = MAX(0, MIN(boost_cap, boost / 3));
    removable = MAX(0, MIN(boost_cap, removable / 3));
    if (p_ptr->cur_lite > 0)
    {
        int malus = p_ptr->cur_lite + 5;
        if (malus >= 10)
        {
            boost = 0;
            removable = 0;
        }
        else if (boost)
        {
            boost -= (boost * malus / 10);
            removable -= (removable * malus / 10);
        }
        if (p_ptr->tim_dark_stalker)
        {
            set_tim_dark_stalker(0, TRUE);
        }
    }
    _curse_boost_capped = boost;
    _curse_boost_removable = removable;
}

static void _calc_bonuses(void)
{
    int boost;

    _calc_bonuses_aux();

    boost = _curse_boost_capped;
    p_ptr->pspeed += boost / 3;
    if (p_ptr->current_r_idx == MON_MUMMY_SORC) p_ptr->pspeed += boost / 3;

    p_ptr->weapon_info[0].xtra_blow += py_prorata_level_aux(boost*10, 1, 1, 1);
    p_ptr->weapon_info[1].xtra_blow += py_prorata_level_aux(boost*10, 1, 1, 1);

    p_ptr->weapon_info[0].to_h += boost; p_ptr->weapon_info[1].to_h += boost;
    p_ptr->to_h_m += boost;
    p_ptr->weapon_info[0].dis_to_h += boost; p_ptr->weapon_info[1].dis_to_h += boost;

    p_ptr->weapon_info[0].to_d += boost; p_ptr->weapon_info[1].to_d += boost;
    p_ptr->to_d_m += boost;
    p_ptr->weapon_info[0].dis_to_d += boost; p_ptr->weapon_info[1].dis_to_d += boost;
    p_ptr->see_nocto = TRUE;
    p_ptr->hold_life++;

    res_add(RES_POIS);
    res_add(RES_COLD);
    res_add(RES_NETHER);
    if (p_ptr->lev < 10) p_ptr->regen -= (p_ptr->regen / 4);
    if (p_ptr->lev >= 27)
    {
        res_add(RES_ELEC);
        p_ptr->slow_digest = TRUE;
    }
    if (p_ptr->lev >= 36)
    {
        res_add(RES_DARK);
        res_add(RES_ACID);
    }
    switch (p_ptr->current_r_idx)
    {
        case MON_MUMMY_SORC:
            p_ptr->skills.dev += 16;
            res_add_vuln(RES_LITE);
            break;
        case MON_MUMMY_KING:
            p_ptr->skills.dev += 8;
            break;
        case MON_DRAUGR:
            p_ptr->skills.dev -= 8;
            break;
        case MON_GHOUL:
            res_add_vuln(RES_LITE);
            break;
        default: break;
    }
    if (mummy_get_toggle() == MUMMY_TOGGLE_ANTITELE) p_ptr->anti_tele = TRUE;
    p_ptr->regen += _curse_boost_capped * 10;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_NETHER);
    add_flag(flgs, OF_NIGHT_VISION);
    add_flag(flgs, OF_HOLD_LIFE);
    if (p_ptr->lev >= 27)
    {
        add_flag(flgs, OF_RES_ELEC);
        add_flag(flgs, OF_SLOW_DIGEST);
    }
    if (p_ptr->lev >= 36)
    {
        add_flag(flgs, OF_RES_DARK);
        add_flag(flgs, OF_RES_ACID);
    }
    if ((p_ptr->current_r_idx == MON_MUMMY_SORC) || (p_ptr->current_r_idx == MON_GHOUL))
    {
        add_flag(flgs, OF_VULN_LITE);
    }
    if (_curse_boost_capped) add_flag(flgs, OF_REGEN);
}

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_ZOMBIE_H && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_MUMMY_H;
        msg_print("你进化成了木乃伊化人类(Mummified human)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MUMMY_H && new_level >= 18)
    {
        p_ptr->current_r_idx = MON_GHOUL;
        msg_print("你进化成了食尸鬼(Ghoul)。");
    }
    if ((p_ptr->current_r_idx == MON_MUMMY_H || p_ptr->current_r_idx == MON_GHOUL) && (new_level >= 27))
    {
        p_ptr->current_r_idx = MON_GREATER_MUMMY;
        msg_print("你进化成了高等木乃伊(Greater mummy)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_GREATER_MUMMY && new_level >= 36)
    {
        if (one_in_(2))
        {
            p_ptr->current_r_idx = MON_DRAUGR;
            msg_print("你进化成了尸鬼(Draugr)。");
        }
        else
        {
            p_ptr->current_r_idx = MON_MUMMY_SORC;
            msg_print("你进化成了木乃伊术士(Mummified sorcerer)。");
        }
        p_ptr->redraw |= PR_MAP;
    }
    if ((p_ptr->current_r_idx != MON_MUMMY_KING) && (new_level >= 45))
    {
        p_ptr->current_r_idx = MON_MUMMY_KING;
        msg_print("你进化成了木乃伊王(Mummy king)。");
        p_ptr->redraw |= PR_MAP;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = { 0 };
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "诅咒力量";
        me.which_stat = A_CHR;
        me.encumbrance.max_wgt = 420;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.min_fail = 5;
        init = TRUE;
    }
    if ((me.min_fail == 5) && ((p_ptr->current_r_idx == MON_MUMMY_SORC) ||
        (p_ptr->current_r_idx == MON_MUMMY_KING)))
    {
        me.min_fail = 3;
    }
    return &me;
}

race_t *mon_mummy_get_race(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static int    init_race = -1;

    if (!init)
    {   /* dis, dev, sav, stl, srh, fos, thn, thb */
        skills_t bs = { 20, 18, 32, 1, 12, 6, 60, 35 };
        skills_t xs = { 7,  6,  12, 0,  0, 0, 20, 17 };

        me.skills = bs;
        me.extra_skills = xs;

        me.name = "木乃伊";
        me.desc = "僵尸和木乃伊是所有不死种族中最令人畏惧的存在之一。邪恶诅咒是木乃伊赖以生存的根本；它们不仅能对敌人施加恶咒，还能随意给自己的装备增加或解除诅咒；它们身上缠绕的诅咒越强、越多，其魔力就越强大。当然，它们也必须承受被诅咒的负面影响……\n\n你以卑微的僵尸化人类开始你的不死生涯，几乎没有诅咒或其他能力可用；但很快你就会进化成木乃伊化人类，情况将大有起色。木乃伊的进化在一定程度上是随机的——你可能会发现自己变成了强壮但愚笨的尸鬼，或者是狡猾但虚弱的木乃伊术士——但最终，你将在令人毛骨悚然的最终形态“木乃伊王”中把力量与魔法结合在一起。\n\n作为陵墓和黑夜的生物，僵尸和木乃伊在没有光的情况下也能看得很清楚；事实上，携带光源会削弱它们诅咒的黑暗能量。在近战中，它们更倾向于依赖自身的天生攻击；装备武器只会分散它们真正的战斗技巧，仅在游戏极初期算是一个可行的进攻选择。";

        me.infra = 5;
        me.exp = 135;
        me.base_hp = 22;
        me.shop_adjust = 120;

        me.calc_innate_attacks = _calc_innate_attacks;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers_fn = _get_powers;
        me.get_spells = _get_spells;
        me.get_flags = _get_flags;
        me.gain_level = _gain_level;
        me.caster_info = _caster_info;
        me.birth = _birth;
        me.boss_r_idx = MON_OSIRIS;

        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING | RACE_IS_UNDEAD | RACE_NIGHT_START | RACE_EATS_DEVICES;
        me.pseudo_class_idx = CLASS_RAGE_MAGE;

        init = TRUE;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);
    if ((init_race != p_ptr->current_r_idx) || (birth_hack) || (spoiler_hack))
    {
        int _my_race = p_ptr->current_r_idx;
        if (spoiler_hack) _my_race = MON_GREATER_MUMMY;
        else if (birth_hack)
        {
            _my_race = MON_ZOMBIE_H;
            init_race = -1; /* paranoia */
        }
        else if (_my_race) init_race = _my_race;
        switch (_my_race)
        {
            case MON_DRAUGR:
                me.life = 110;
                me.stats[A_STR] = 2;
                me.stats[A_INT] = -3;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -1;
                me.stats[A_CON] = 2;
                me.stats[A_CHR] = -3;
            break;
            case MON_MUMMY_SORC:
                me.life = 92;
                me.stats[A_STR] = -3;
                me.stats[A_INT] = 2;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -2;
                me.stats[A_CON] = -2;
                me.stats[A_CHR] = 1;
            break;
            case MON_MUMMY_KING:
                me.life = 105;
                me.stats[A_STR] = 1;
                me.stats[A_INT] = 0;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -1;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = 0;
            break;
            case MON_GREATER_MUMMY:
                me.life = 100;
                me.stats[A_STR] = -1;
                me.stats[A_INT] = -2;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -2;
                me.stats[A_CON] = 0;
                me.stats[A_CHR] = -1;
            break;
            case MON_GHOUL:
                me.life = 95;
                me.stats[A_STR] = -2;
                me.stats[A_INT] = 0;
                me.stats[A_WIS] = -3;
                me.stats[A_DEX] = -3;
                me.stats[A_CON] = -2;
                me.stats[A_CHR] = -1;
            break;
            case MON_MUMMY_H:
                me.life = 100;
                me.stats[A_STR] = 0;
                me.stats[A_INT] = -3;
                me.stats[A_WIS] = -4;
                me.stats[A_DEX] = 0;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = -3;
            break;
            default:
                me.life = 100;
                me.stats[A_STR] = 0;
                me.stats[A_INT] = -4;
                me.stats[A_WIS] = -6;
                me.stats[A_DEX] = 0;
                me.stats[A_CON] = 1;
                me.stats[A_CHR] = -3;
            break;
        }
    }

    return &me;
}
