#include "angband.h"

#include <assert.h>

static cptr _desc = 
    "Dragons are powerful winged serpents. They are strong in combat, "
    "with razor-sharp claws and a bone-crushing bite; but even more legendary "
    "is their breath, which grows ever more deadly as the dragon matures.\n \n"
    "Due to their non-humanoid bodies, dragons are unable to wear armor, gloves "
    "or boots; however, being creatures of magic, they may equip up to six rings. "
    "They can also wear a helmet, a light source, a cloak and an amulet. Because "
    "of these equipment restrictions, dragons may have a difficult time covering "
    "all resistances; but each dragon has at least one innate resistance, maybe "
    "even an immunity.\n \n"
    "Dragons begin life in a weak form, being very young. As their bodies mature, "
    "their scales grow tough and their claws sharp. Their breaths become stronger and "
    "stronger, and they frequently gain additional magical powers and resistances. "
    "All dragons can fly, but younger dragons are not so quick as their elders.\n \n"
    "With the exception of Steel Dragons, who are far less intelligent than their kin "
    "and rely on brute force only, each dragon may choose to specialize in a "
    "specific type of magic. These special dragon-magic realms do not require books "
    "to learn. The selection of a realm has a direct influence on the stats and skills of the dragon.";

static dragon_realm_ptr _get_realm(void);

/**********************************************************************
 * Dragon Equipment
 **********************************************************************/
static void _dragon_birth(void) 
{ 
    object_type    forge;

    equip_on_change_race();
    skills_innate_init("爪击", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);
    skills_innate_init("撕咬", WEAPON_EXP_BEGINNER, WEAPON_EXP_MASTER);

    
    object_prep(&forge, lookup_kind(TV_RING, 0));
    forge.name2 = EGO_RING_COMBAT;
    forge.to_h = 3;
    forge.to_d = 3;
    forge.pval = 1;
    add_flag(forge.flags, OF_STR);
    add_flag(forge.flags, OF_DEX);
    py_birth_obj(&forge);

    py_birth_food();
    py_birth_light();
}

/**********************************************************************
 * Dragon Breath
 **********************************************************************/

static void _do_breathe(int effect, int dir, int dam)
{
    /* Dragon breath changes shape with maturity */
    if (p_ptr->lev < 20)
        fire_bolt(effect, dir, dam);
    else if (p_ptr->lev < 30)
        fire_beam(effect, dir, dam);
    else
        fire_ball(effect, dir, dam, -1 - (p_ptr->lev / 20));
}

cptr gf_name(int which)
{
    gf_info_ptr gf;
    switch (which)
    {
    case GF_ELDRITCH_HOWL: return "<color:R>Fear</color>";
    case GF_ANIM_DEAD: return "<color:D>Reanimation</color>";
    case GF_OLD_DRAIN: return "<color:D>Vampirism</color>";
    case GF_GENOCIDE: return "<color:D>Death</color>";
    case GF_OLD_POLY: return "<color:v>Change</color>";
    }
    gf = gf_lookup(which);
    if (gf)
        return format("<color:%c>%s</color>", attr_to_attr_char(gf->color), gf->name);
    return "something";
}

static int _count(int list[])
{
    int i;
    for (i = 0; ; i++)
    {
        if (list[i] == -1) return i;
    }
    /* return 0;  error: missing sentinel ... unreachable */
}

static int _random(int list[])
{
    return list[randint0(_count(list))];
}

static void _effect_menu_fn(int cmd, int which, vptr cookie, variant *res)
{
    int  idx = ((int*)cookie)[which];

    switch (cmd)
    {
    case MENU_TEXT:
        var_set_string(res, format("%s", gf_name(idx)));
        break;
    }
}

static int _choose_effect(int list[])
{
    int i;
    int ct = _count(list);

    if (REPEAT_PULL(&i))
    {
        if (i >= 0 && i < ct)
            return list[i];
    }

    {
        menu_t menu = { "选择哪个效果？", NULL, NULL,
                        _effect_menu_fn, list, ct, 0};
        
        i = menu_choose(&menu);
        if (i >= 0)
        {
            REPEAT_PUSH(i);
            i = list[i];
        }
    }
    return i;
}

static int _get_effect(int list[]) /* va_args is probably a better sig ... */
{
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH && p_ptr->lev >= 35)
        return _choose_effect(list);
    else
        return _random(list);
}

static int _breath_effect(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return GF_FIRE;
    case DRAGON_WHITE: return GF_COLD;
    case DRAGON_BLUE: return GF_ELEC;
    case DRAGON_BLACK: return GF_ACID;
    case DRAGON_GREEN: return GF_POIS;
    case DRAGON_BRONZE: return GF_CONFUSION;
    case DRAGON_GOLD: return GF_SOUND;
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 45)
        {
            int effects[] = { GF_NETHER, GF_NEXUS, GF_DISENCHANT, -1 };
            return _get_effect(effects);
        }
        return GF_NETHER;
    case DRAGON_LAW: 
    {
        int effects[] = { GF_SOUND, GF_SHARDS, -1 };
        return _get_effect(effects);
    }
    case DRAGON_CHAOS: 
    {
        int effects[] = { GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    case DRAGON_ETHEREAL: 
        if (p_ptr->lev < 40)
        {
            int effects[] = { GF_LITE, GF_DARK, -1 };
            return _get_effect(effects);
        }
        else
        {
            int effects[] = { GF_LITE, GF_DARK, GF_CONFUSION, -1 };
            return _get_effect(effects);
        }
    case DRAGON_CRYSTAL: return GF_SHARDS;
    case DRAGON_BALANCE:
    {
        int effects[] = { GF_SOUND, GF_SHARDS, GF_CHAOS, GF_DISENCHANT, -1 };
        return _get_effect(effects);
    }
    }
    return 0;
}
static int _breath_amount(void)
{
    int l = p_ptr->lev;
    int amt = 0;
    dragon_realm_ptr realm = _get_realm();

    switch (p_ptr->psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        amt = MIN(600, p_ptr->chp * (25 + l*l*l/2500) / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        amt = MIN(450, p_ptr->chp * (20 + l*l*l*30/125000) / 100);
        break;

    case DRAGON_BALANCE:
    case DRAGON_NETHER:
    case DRAGON_ETHEREAL:
        amt = MIN(400, p_ptr->chp * (20 + l*l*l*25/125000) / 100);
        break;

    case DRAGON_STEEL:
        return 0;
    }
    amt = MAX(1, amt * realm->breath / 100);
    return amt;
}

static int _breath_cost(void)
{
    int l = p_ptr->lev;
    int cost = l/2 + l*l*15/2500;
    if (p_ptr->dragon_realm == DRAGON_REALM_BREATH)
    {
        if (p_ptr->lev >= 40)
            cost = cost * 3 / 4;
    }
    else
    {
    /*    cost += 5; */
    }
    return MAX(cost, 1);
}

static cptr _breath_desc(void)
{
    switch (p_ptr->psubrace)
    {
    case DRAGON_RED: return "fire";
    case DRAGON_WHITE: return "cold";
    case DRAGON_BLUE: return "lightning";
    case DRAGON_BLACK: return "acid";
    case DRAGON_GREEN: return "poison";
    case DRAGON_BRONZE: return "confusion";
    case DRAGON_GOLD: return "sound";
    case DRAGON_NETHER: 
        if (p_ptr->lev >= 40) return "nether, nexus or disenchantment";
        return "nether";
    case DRAGON_LAW: return "sound or shards";
    case DRAGON_CHAOS: return "chaos or disenchantment";
    case DRAGON_ETHEREAL: return "light, dark or confusion";
    case DRAGON_CRYSTAL: return "shards";
    case DRAGON_BALANCE: return "sound, shards, chaos or disenchantment";
    }
    return 0;
}

static void _breathe_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐%s。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            _do_breathe(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

/**********************************************************************
 * Dragon Melee
 * cf design/dragons.ods
 **********************************************************************/
static int _attack_level(void)
{
    int l = p_ptr->lev * 2;
    switch (p_ptr->psubrace)
    {
    case DRAGON_STEEL:
        l = MAX(1, l * 115 / 100);
        break;

    case DRAGON_RED:
    case DRAGON_WHITE:
        l = MAX(1, l * 105 / 100);
        break;

    case DRAGON_BLACK:
    case DRAGON_GREEN:
        break;

    case DRAGON_BLUE:
        l = MAX(1, l * 95 / 100);
        break;

    case DRAGON_ETHEREAL:
    case DRAGON_CRYSTAL:
    case DRAGON_BRONZE:
    case DRAGON_GOLD:
        l = MAX(1, l * 90 / 100);
        break;

    case DRAGON_LAW:
    case DRAGON_CHAOS:
    case DRAGON_NETHER:
    case DRAGON_BALANCE:
        l = MAX(1, l * 85 / 100);
        break;
    }

    l = MAX(1, l * _get_realm()->attack / 100);
    return l;
}

static void _calc_innate_attacks(void)
{
    int l = _attack_level();
    int l2 = p_ptr->lev; /* Note: Using attack_level() for both dd and ds gives too much variation */
    int to_d = 0;
    int to_h = l2*3/5;

    /* Claws */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l / 15;
        a.ds = 3 + l2 / 16; /* d6 max for everybody */
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 100 + l;
        calc_innate_blows(&a, 400);
        a.msg = "You claw.";
        a.name = "爪击";

        /*if (p_ptr->dragon_realm == DRAGON_REALM_ATTACK && p_ptr->lev >= 40)
            a.flags |= INNATE_VORPAL;*/

        if (p_ptr->dragon_realm == DRAGON_REALM_DEATH && p_ptr->lev >= 45)
            a.effect[1] = GF_OLD_DRAIN;

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
    /* Bite */
    {
        innate_attack_t    a = {0};

        a.dd = 1 + l2 / 10; /* 6d max for everybody */
        a.ds = 4 + l / 6;
        a.to_h += to_h;
        a.to_d += to_d;

        a.weight = 200 + 2 * l;

        if (p_ptr->lev >= 40)
            calc_innate_blows(&a, 200);
        else if (p_ptr->lev >= 35)
            calc_innate_blows(&a, 150);
        else
            a.blows = 100;
        a.msg = "You bite.";
        a.name = "撕咬";

        /*if (p_ptr->dragon_realm == DRAGON_REALM_ATTACK && p_ptr->lev >= 40)
            a.flags |= INNATE_VORPAL;*/

        if (p_ptr->dragon_realm == DRAGON_REALM_DEATH && p_ptr->lev >= 45)
            a.effect[1] = GF_OLD_DRAIN;

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

/**********************************************************************
 * Dragon Realms
 **********************************************************************/
static dragon_realm_t _realms[DRAGON_REALM_MAX] = {
    { DRAGON_REALM_NONE, "无", 
        "",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0,  0,  0,  0,  0,  0}, {   0,   0,   0,   0,   0,   0,  0,  0}, 100, 100,   100,   100, A_NONE},

    { DRAGON_REALM_LORE, "传说", 
        "Dragons specializing in lore are seekers of knowledge. They are the most "
        "intelligent of dragonkind, and it is intellect that drives their magic. "
        "Armed with a vast array of detection and knowledge spells, dragons of lore "
        "seek power through knowledge. They eventually gain powers of telepathy and "
        "automatic object identification. Lore dragons are quick learners, and gain "
        "maturity much more rapidly than the rest of their kind.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, +3,  0, -1, -1,  0}, {   3,   8,   2,   0,   5,   5, -8,  0}, 100,  80,   100,   100, A_INT},

    { DRAGON_REALM_BREATH, "吐息", 
        "龙息是传说中令人敬畏的力量，而这个领域旨在强化龙族这一最强大的特性。掌握此专精后，你能够塑造并控制你的吐息，在不同情况下最大化其杀伤力。此外，该领域的龙如果条件允许，还可以选择它们的吐息类型，并且随着它们的成长，吐息消耗的精力也会减少。这种专注需要极大的毅力才能掌握，并在一定程度上削弱了龙的防御和近战能力。",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      { 0, -1, -1,  0, +3, +1}, {   0,   0,   3,  -1,   0,   0,  0,  0}, 103, 105,    90,   115, A_CON},

    { DRAGON_REALM_ATTACK, "攻击", 
        "攻击领域的龙追求近战的绝对霸权。该领域提供了强大的攻击法术来支持这个本来就已经是近战精英的种族，其结果往往是毁灭与死亡。在这个领域的支持下，龙可以用极其锋利的爪子撕裂对手，可以用强有力的巨颚抓起相邻的对手并像布娃娃一样把他们扔出去，甚至可以将它们的吐息元素附加在撕咬攻击上！一头横冲直撞的龙是真正令人敬畏的景象，很少有人能目睹，或者说，很少有人能在目睹后幸存下来。这种专注视力量高于一切。",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+3, -2, -2, +1, -1,  0}, {  -5,  -5,  -3,  -1,  -2,  -2, 15,  0},  97, 105,   115,    80, A_STR},

    { DRAGON_REALM_CRAFT, "工匠", 
        "The most powerful magical items have long been believed forged by dragonflame. The "
        "craft dragon gains powers of enchantment, and may even reforge artifacts into the objects "
        "of their choosing! Otherwise, craft dragons are not particularly powerful, as they trade "
        "melee and breath prowess for magical understanding. This focus requires great wisdom.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, +3, -1, -1, -1}, {   3,  15,   3,   0,   0,   0, -5,  0}, 100,  95,   100,   100, A_WIS},

    { DRAGON_REALM_ARMOR, "护甲", 
        "Dragon scales have thwarted many a would-be dragonslayer. Naturally tough and resistant, "
        "the dragon's armor is even further enhanced by this realm. This specialization gives enhanced "
        "armor class, reflection, resistance to cuts and stunning, resistance to poison "
        "and life draining, and sustaining of several key stats, albeit not all at once. With all "
        "of these extra innate bonuses, the magic spells of this realm are few in number but serve "
        "to offer temporary defensive augmentations. Unlike their kin, dragons of this order prize "
        "agility above all else.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, +3, +1, +1}, {  -2,  -3,   7,   1,   0,   0,-10,  0}, 102, 105,    90,    90, A_DEX},

    { DRAGON_REALM_DOMINATION, "支配", 
        "All dragons have a formidable presence and make fearsome opponents; but Domination dragons "
        "are truly a breed apart, seeking to bend and control the will of all they meet. Convinced "
        "of their right to rule, these dragons may subjugate the weak, terrify the uncertain, "
        "and stun the unwary with their awesome presence. They are the best dragons at controlling "
        "minions, and can summon mighty aid. Initially hostile monsters summoned to battle may switch sides "
        "when they notice whom they have been commanded to fight; and in the end, the dragon of "
        "domination may sever all oaths of allegiance, returning enemy summons to where they came from. "
        "Needless to say, dragons of this order value force of will above all else.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {-1, -1, -1, -1, -1, +3}, {  -2,  -3,  -2,   0,   0,   0, -7,  0},  95, 105,    95,    90, A_CHR},

    { DRAGON_REALM_CRUSADE, "圣战", 
        "圣战领域的龙肩负着毁灭邪恶势力的使命。因此，该领域仅对金龙和律法龙开放。由于目标专一，圣战龙在近战和吐息方面不如其他龙强大，但它们的法术足以弥补这一缺陷——至少在对抗邪恶敌人时是如此。圣战龙能够喷吐通常无法获得的神圣元素。一见到邪恶，它们就会被激怒并可能加速进入战斗。它们甚至能进行一些治疗；最终，它们可以在近战和吐息中用神圣力量惩击邪恶势力。这种龙个性坚毅，甚至能为最终的决战召唤志同道合的同族。",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+1, -1, -1, +1, -1, +2}, {  -5,   0,  -2,   0,  -2,  -2,  7,  0},  95, 107,    90,    90, A_CHR},

    { DRAGON_REALM_DEATH, "死亡", 
        "Death dragons are enemies of life itself, seeking to destroy all living creatures. With this "
        "realm, the dragon may bend their breath weapon to suit their necromantic desires, eventually "
        "breathing mastery over both death and life. At high levels, the death dragon's melee attacks "
        "gain a powerful draining effect against living creatures. This focus values strength above "
        "all else. This foul realm is only available to Shadow and Chaos dragons.",
    /*  S   I   W   D   C   C    Dsrm Dvce Save Stlh Srch Prcp Thn Thb  Life  Exp Attack Breath*/
      {+2, -2, -2,  0, -2, +1}, {  -5,  -3,  -3,   2,  -2,  -2,  5,  0},  95, 105,    90,    90, A_STR},
};

dragon_realm_ptr dragon_get_realm(int which)
{
    assert(0 <= which && which < DRAGON_REALM_MAX);
    return &_realms[which];
}

dragon_realm_ptr _get_realm(void)
{
    return dragon_get_realm(p_ptr->dragon_realm);
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "龙之法术";
        me.encumbrance.max_wgt = 750;
        me.encumbrance.weapon_pct = 0;
        me.encumbrance.enc_wgt = 800;
        init = TRUE;
    }
    me.which_stat = _get_realm()->spell_stat; /* Careful: Birthing may invoke this multiple times with different realms */
    return &me;
}

/* Lore */
static spell_info _lore_spells[] = {
    {  1,  1, 30, detect_traps_spell },
    {  3,  2, 30, detect_treasure_spell },
    {  5,  3, 40, detect_monsters_spell },
    {  7,  5, 50, detect_objects_spell },
    { 12, 10, 60, identify_spell },
    { 15, 12, 60, sense_surroundings_spell },
    { 20, 15, 60, detection_spell },
    { 22, 17, 60, probing_spell },
    { 25, 20, 65, self_knowledge_spell },
    { 30, 25, 70, identify_fully_spell },
    { 40, 50, 90, clairvoyance_spell },
    { -1, -1, -1, NULL}
};

/* Breath */
static void _bolt_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吐息箭");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐一道%s箭。随着你变得更加强大，这种吐息的速度会变得更快，尽管它不如普通吐息致命。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, MAX(1, _breath_cost()/2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_bolt(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 101 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "射线吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐一道%s射线。随着你变得更加强大，这种吐息的速度会变得更快。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_beam(e, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 110 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _cone_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "锥形吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐锥形的%s。随着你变得更加强大，这种吐息的速度会变得更快。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_ball(e, dir, dam, -2);
            var_set_bool(res, TRUE);
        }
        break;
    }
    case SPELL_ENERGY:
        var_set_int(res, 120 - p_ptr->lev);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _split_beam_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "分裂射线");
        break;
    case SPELL_DESC:
        var_set_string(res, format("你向两个选定的目标喷吐%s射线，但伤害会有所降低。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, MAX(1, _breath_amount()/2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = MAX(1, _breath_amount()/2);
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_beam(e, dir, dam);
            
            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */
            if (get_fire_dir(&dir))
                fire_beam(e, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _retreating_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "撤退吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手喷吐%s，然后进行一次移动。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount();
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_ball(e, dir, dam, -2);

            command_dir = 0; /* Code is buggy asking for a direction 2x in a single player action! */
            target_who = 0;  /* TODO: Repeat command is busted ... */

            if (get_rep_dir2(&dir) && dir != 5)
            {
                int y, x;
                y = py + ddy[dir];
                x = px + ddx[dir];
                if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
                    move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
            }

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _deadly_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "致命吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, format("向你的对手进行强力的%s喷吐。", _breath_desc()));
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount() * 125 / 100));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost() * 125 / 100);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int e = _breath_effect();
            int dam = _breath_amount() * 125 / 100;
            var_set_bool(res, FALSE);
            if (e < 0) return;
            msg_format("你喷吐出%s。", gf_name(e));
            fire_ball(e, dir, dam, -3);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _star_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "星辰球");
        break;
    case SPELL_DESC:
        var_set_string(res, "不可控制地随机释放你的吐息，其效果具有毁灭性的破坏力。");
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int num = damroll(5, 3);
        int dam = MAX(1, _breath_amount()/3);
        int e = _breath_effect();
        int y = py, x = px, i;
        int attempts;

        var_set_bool(res, FALSE);
        if (e < 0) return;

        for (i = 0; i < num; i++)
        {
            attempts = 1000;
            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);
                if (!cave_have_flag_bold(y, x, FF_PROJECT)) continue;
                if (!player_bold(y, x)) break;
            }
            project(0, 3, y, x, dam, e,
                (PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL));
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _breath_spells[] = {
    {  1,  0, 30, _bolt_spell },
    { 10,  0, 30, _beam_spell },
    { 20,  0, 30, _cone_spell },
    { 25, 10, 50, _split_beam_spell },
    { 30, 15, 50, _retreating_breath_spell },
    { 40, 15, 60, _deadly_breath_spell },
    { 50, 50, 70, _star_ball_spell },
    { -1, -1, -1, NULL}
};

/* Craft */
static void _detect_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测魔法");
        break;
    case SPELL_DESC:
        var_set_string(res, "定位附近的魔法物品。");
        break;
    case SPELL_CAST:
        detect_objects_magic(DETECT_RAD_DEFAULT);    
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static obj_ptr _get_reforge_src(int max_power)
{
    obj_prompt_t prompt = {0};
    char buf[255];

    sprintf(buf, "使用哪件神器进行重铸 (最大力量 = %d)？", max_power);
    prompt.prompt = buf;
    prompt.error = "你没有可用于重铸的神器。";
    prompt.filter = object_is_artifact;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_FLOOR;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static obj_ptr _get_reforge_dest(int max_power)
{
    obj_prompt_t prompt = {0};
    char buf[255];

    sprintf(buf, "重铸哪件物品 (最大力量 = %d)？", max_power);
    prompt.prompt = buf;
    prompt.error = "你没有可以重铸的物品。";
    prompt.filter = item_tester_hook_nameless_weapon_armour;
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    prompt.where[3] = INV_FLOOR;
    prompt.flags = INV_SHOW_VALUE;

    obj_prompt(&prompt);
    return prompt.obj;
}

static void _reforging_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "重铸");
        break;
    case SPELL_DESC:
        var_set_string(res, "重铸一件选定的神器。");
        break;
    case SPELL_CAST:
    {
        int cost;
        char o_name[MAX_NLEN];
        obj_ptr src, dest;
        int power = p_ptr->lev * 5 / 2 + (p_ptr->lev >= 50 ? 5 : 0);
        int src_max_power = power * power * 10;
        int dest_max_power = 0;

        var_set_bool(res, FALSE);
        src = _get_reforge_src(src_max_power);
        if (!src) return;
        if (!object_is_artifact(src)) /* paranoia */
        {
            msg_print("你必须选择一件神器来进行重铸。");
            return;
        }
        if (obj_value_real(src) > src_max_power)
        {
            msg_print("你现在的力量不足以重铸这件物品。");
            return;
        }

        cost = obj_value_real(src);
    
        dest_max_power = cost / 2;
        if (dest_max_power < 1000) /* Reforging won't try to power match weak stuff ... */
            dest_max_power = 1000;

        object_desc(o_name, src, OD_NAME_ONLY);    
        if (!get_check(format("确认要使用 %s 吗？(它将被摧毁！)", o_name))) 
            return;

        dest = _get_reforge_dest(dest_max_power);
        if (!dest) return;

        if (dest->number > 1)
        {
            msg_print("不要贪心！你每次只能重铸一件物品。");
            return;
        }

        if (object_is_artifact(dest))
        {
            msg_print("这件物品已经是神器了！");
            return;
        }

        if (object_is_ego(dest))
        {
            msg_print("这件物品已经是Ego装备(名品)了！");
            return;
        }

        if (!equip_first_slot(dest))
        {
            msg_print("你只能制造你实际能使用的物品。");
            return;
        }

        if (obj_value_real(dest) > dest_max_power)
        {
            msg_print("对于你所选择的源神器来说，这件物品太过强大了。");
            return;
        }

        if (!reforge_artifact(src, dest, power))
        {
            msg_print("重铸失败了！");
            return;
        }

        src->number--;
        obj_release(src, 0);

        dest->mitze_type = 0;
        object_mitze(dest, MITZE_REFORGE);

        obj_identify_fully(dest);

        p_ptr->update |= PU_BONUS;
        p_ptr->window |= (PW_INVEN | PW_EQUIP);
        handle_stuff();

        obj_display(dest);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _craft_spells[] = {
    {  1,  1, 30, _detect_magic_spell },
    {  5,  7, 60, minor_enchantment_spell },
    { 12, 10, 60, remove_curse_I_spell },
    { 17, 15, 60, identify_spell },
    { 30, 25, 70, enchantment_spell },
    { 32, 30, 70, recharging_spell },
    { 35, 90, 70, _reforging_spell },
    { 40, 30, 70, dispel_magic_spell },
    { -1, -1, -1, NULL}
};

/* Attack */
static void _war_cry_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "巨龙咆哮");
        break;
    case SPELL_DESC:
        var_set_string(res, "你将发出一声巨大的咆哮，提醒附近所有的怪物你的存在。");
        break;
    case SPELL_CAST:
        msg_print("你咆哮了起来！");
        project_hack(GF_SOUND, randint1(p_ptr->lev));
        aggravate_monsters(0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rend_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "撕裂");
        break;
    case SPELL_DESC:
        var_set_string(res, "对相邻的对手进行切割攻击。");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].flags |= INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags |= INNATE_VORPAL;
        var_set_bool(res, do_blow(DRAGON_REND));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].flags |= INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags |= INNATE_VORPAL;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].flags &= ~INNATE_VORPAL;
        p_ptr->innate_attacks[1].flags &= ~INNATE_VORPAL;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rage_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂暴");
        break;
    default:
        berserk_spell(cmd, res);
        break;
    }
}

static void _three_way_attack_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "三向攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一次行动中，攻击选定方向，以及该方向的两侧。");
        break;
    case SPELL_CAST:
    {
        int cdir, dir;
        int y, x;

        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        for (cdir = 0;cdir < 8; cdir++)
        {
            if (cdd[cdir] == dir) break;
        }

        if (cdir == 8) return;

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

static void _deadly_bite_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "致命撕咬");
        break;
    case SPELL_DESC:
        var_set_string(res, "像往常一样攻击相邻的对手，但会在你的撕咬攻击中附加你的吐息元素。");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[1].effect[1] = _breath_effect();
        var_set_bool(res, do_blow(DRAGON_DEADLY_BITE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[1].effect[1] = _breath_effect();
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[1].effect[1] = 0;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _snatch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抓取");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试用巨颚抓取相邻的对手。如果成功，你可以将该怪物扔出去。");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].flags |= INNATE_SKIP;
        var_set_bool(res, do_blow(DRAGON_SNATCH));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _charge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "冲锋");
        break;
    case SPELL_DESC:
        var_set_string(res, "冲向附近的怪物并在一次行动中进行攻击。");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rapid_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "连环打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "对相邻的对手进行额外的攻击。");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].blows += 50;
        p_ptr->innate_attacks[1].blows += 25;
        var_set_bool(res, do_blow(DRAGON_RAPID_STRIKE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].blows += 50;
        p_ptr->innate_attacks[1].blows += 25;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].blows -= 50;
        p_ptr->innate_attacks[1].blows -= 25;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _power_strike_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "强力打击");
        break;
    case SPELL_DESC:
        var_set_string(res, "用更加强力的攻击打击相邻的对手。");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].dd += 2;
        p_ptr->innate_attacks[1].dd += 2;
        var_set_bool(res, do_blow(DRAGON_POWER_STRIKE));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;

    case SPELL_ON_BROWSE:
    {
        bool screen_hack = screen_is_saved();
        if (screen_hack) screen_load();

        p_ptr->innate_attacks[0].dd += 2;
        p_ptr->innate_attacks[1].dd += 2;
        do_cmd_knowledge_weapon();
        p_ptr->innate_attacks[0].dd -= 2;
        p_ptr->innate_attacks[1].dd -= 2;

        if (screen_hack) screen_save();
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _attack_spells[] = {
    {  1,  1, 20, _war_cry_spell },
    {  5,  3, 40, detect_menace_spell },
    {  7,  7,  0, _rend_spell },
    {  9,  9, 50, _rage_spell },
    { 12, 10,  0, _three_way_attack_spell },
    { 20, 15,  0, _deadly_bite_spell },
    { 22, 15,  0, _snatch_spell },
    { 25, 20, 50, _charge_spell },
    { 30, 25, 50, _rapid_strike_spell },
    { 40, 30, 60, _power_strike_spell },
    { -1, -1, -1, NULL}
};

/* Armor */
static void _shard_skin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "碎片肌肤");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得一层碎片光环，对任何攻击你的怪物造成伤害。");
        break;
    case SPELL_CAST:
        set_tim_sh_shards(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _dragon_cloak_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "巨龙披风");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得多种保护性的元素光环。");
        break;
    case SPELL_CAST:
        set_tim_sh_elements(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _magic_resistance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法抗性");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时提升对魔法的抗性。");
        break;
    case SPELL_CAST:
        set_resist_magic(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _armor_spells[] = {
    { 10, 10, 50, stone_skin_spell },
    { 15, 12, 50, _shard_skin_spell },
    { 20, 15, 60, _dragon_cloak_spell },
    { 25, 20, 60, resistance_spell },
    { 30, 30, 70, _magic_resistance_spell },
    { -1, -1, -1, NULL}
};

/* Crusade */
static void _breathe_spell_aux(int effect, int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = _breath_amount();

            msg_format("你喷吐出%s。", gf_name(effect));
            _do_breathe(effect, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_retribution_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "惩戒吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐闪电。");
        break;
    default:
        _breathe_spell_aux(GF_ELEC, cmd, res);
        break;
    }
}

static void _breathe_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光明吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定目标喷吐光芒。");
        break;
    default:
        _breathe_spell_aux(GF_LITE, cmd, res);
        break;
    }
}

static void _healing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗");
        break;
    case SPELL_DESC:
        var_set_string(res, "强大的治疗魔法：恢复生命值，并治愈割伤和震慑状态。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, 200));
        break;
    case SPELL_CAST:
        hp_player(200);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_holiness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神圣吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐神圣之火。这会对邪恶怪物造成巨大的伤害，但非邪恶的怪物会抵抗它。");
        break;
    default:
        _breathe_spell_aux(GF_HOLY_FIRE, cmd, res);
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
        var_set_string(res, "以神圣的狂怒攻击相邻的邪恶对手！");
        break;
    case SPELL_CAST:
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attacks[0].effect[1] = GF_HOLY_FIRE;
        p_ptr->innate_attacks[1].effect[1] = GF_HOLY_FIRE;
        var_set_bool(res, do_blow(DRAGON_SMITE_EVIL));
        p_ptr->innate_attack_lock = FALSE;
        p_ptr->update |= PU_BONUS;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static spell_info _crusade_spells[] = {
    {  1,  1, 30, bless_spell },
    {  5,  3, 30, remove_fear_spell },
    {  7,  4, 40, detect_evil_spell },
    { 10,  5, 50, _breathe_retribution_spell },
    { 15,  7, 50, heroism_spell },
    { 25, 15, 60, haste_self_spell },
    { 30, 20, 60, curing_spell },
    { 32, 10, 60, _breathe_light_spell },
    { 35, 30, 60, _healing_spell },
    { 37, 30, 70, _breathe_holiness_spell },
    { 40, 60,  0, _smite_evil_spell },
    { 45,100, 90, summon_hi_dragon_spell },
    { -1, -1, -1, NULL}
};

/* Death */
static void _breathe_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毒素吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐毒素。");
        break;
    default:
        _breathe_spell_aux(GF_POIS, cmd, res);
        break;
    }
}

static void _breathe_fear_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恐惧吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐恐惧。");
        break;
    default:
        _breathe_spell_aux(GF_ELDRITCH_HOWL, cmd, res);
        break;
    }
}

static void _breathe_dark_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑暗吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐黑暗。");
        break;
    default:
        _breathe_spell_aux(GF_DARK, cmd, res);
        break;
    }
}

static void _breathe_nether_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "地狱吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐地狱能量。");
        break;
    default:
        _breathe_spell_aux(GF_NETHER, cmd, res);
        break;
    }
}

static void _breathe_reanimation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "复生吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "喷吐复生能量。任何被这种吐息击中的尸体或骨骸都可能复活并为你效劳。");
        break;
    case SPELL_INFO:
        break;
    default:
        _breathe_spell_aux(GF_ANIM_DEAD, cmd, res);
        break;
    }
}

int dragon_vamp_amt = 0;
bool dragon_vamp_hack = FALSE;

static void _breathe_vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "喷吐吸血能量。任何被这种吐息击中的活物都会被吸取生命能量，以恢复你的生命值。");
        break;
    default:
        dragon_vamp_hack = TRUE;
        dragon_vamp_amt = 0;
        _breathe_spell_aux(GF_OLD_DRAIN, cmd, res);
        dragon_vamp_hack = FALSE;
        if (dragon_vamp_amt)
        {
            int amt = MIN(500, (dragon_vamp_amt + 1) / 2);
            hp_player(amt);
        }
        break;
    }
}

static void _breathe_unholiness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "不洁吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定目标喷吐地狱火。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _breath_amount() * 3 / 2));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost() * 3 / 2);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = _breath_amount() * 3 / 2;

            msg_print("你喷吐出地狱火。");
            fire_ball(GF_HELL_FIRE, dir, dam, -1 - (p_ptr->lev / 20));

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _breathe_genocide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "灭绝吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "喷吐灭绝能量。任何被这种吐息击中的怪物都可能从当前层被移除，但每移除一个怪物你都会受到伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(_breath_amount()));
        break;
    default:
        _breathe_spell_aux(GF_GENOCIDE, cmd, res);
        break;
    }
}

static spell_info _death_spells[] = {
    {  1,  1, 30, evil_bless_spell },
    {  5,  2, 40, detect_evil_spell },
    { 10,  5, 50, _breathe_poison_spell },
    { 15,  5, 50, _breathe_fear_spell },
    { 20, 10, 60, _breathe_dark_spell },
    { 25, 20, 60, restore_life_spell },
    { 27, 10, 60, _breathe_nether_spell },
    { 30, 25, 65, battle_frenzy_spell },
    { 32, 20, 65, _breathe_reanimation_spell },
    { 35, 20, 65, _breathe_vampirism_spell },
    { 37, 20, 70, _breathe_unholiness_spell },
    { 40, 35, 75, _breathe_genocide_spell },
    { -1, -1, -1, NULL}
};

/* Domination */
static void _frightful_presence_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恐怖威压");
        break;
    default:
        scare_spell(cmd, res);
        break;
    }
}

static void _detect_minions_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "探测随从");
        break;
    default:
        detect_monsters_spell(cmd, res);
        break;
    }
}

static void _baffling_presence_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "迷惑威压");
        break;
    default:
        confuse_spell(cmd, res);
        break;
    }
}

static void _enslave_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "奴役");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试强迫一个怪物为你效劳。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        charm_monster(dir, p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _plev(void)
{
    if (p_ptr->lev <= 40)
        return 5 + p_ptr->lev;

    return 45 + (p_ptr->lev - 40)*2;
}

int subjugation_power(void)
{
    return MAX(1, _plev() + adj_stat_save[p_ptr->stat_ind[A_CHR]]);
}

static void _breathe_subjugation_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "征服吐息");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定的目标喷吐支配之力。受影响的怪物可能会被迫为你效劳，或者被你可怕的威压所震慑或惊吓。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(subjugation_power()));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, _breath_cost());
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int dam = subjugation_power();

            msg_print("你喷吐出征服之力。");
            _do_breathe(GF_SUBJUGATION, dir, dam);

            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _aura_of_domination_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "支配光环");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得一层支配光环，它会试图奴役、惊吓或震慑任何攻击你的怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(10, 20));
        break;
    case SPELL_CAST:
        set_tim_sh_domination(randint1(20) + 10, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _banish_summons_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱逐召唤物");
        break;
    case SPELL_DESC:
        var_set_string(res, "将视野内所有敌方的召唤物送回它们来的地方。");
        break;
    case SPELL_CAST:
    {
        int i;
        msg_print("你粉碎了所有的效忠誓言！");
        for (i = 1; i < m_max; i++)
        {
            monster_type *m_ptr = &m_list[i];

            if (!m_ptr->r_idx) continue;
            if (!m_ptr->parent_m_idx) continue;
            if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) continue;
            if (is_pet(m_ptr)) continue;

            delete_monster_idx(i);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}


static spell_info _domination_spells[] = {
    {  1,  1, 30, _frightful_presence_spell },
    {  5,  2, 35, _detect_minions_spell }, 
    { 10,  5, 40, _baffling_presence_spell },
    { 15, 10, 50, slow_spell },
    { 20, 15, 55, _enslave_spell },
    { 25, 25, 60, summon_kin_spell },
    { 30, 10, 60, _breathe_subjugation_spell },
    { 35, 30, 60, _aura_of_domination_spell },
    { 40, 70, 90, summon_hi_dragon_spell },
    { 45, 80, 85, _banish_summons_spell },
    { -1, -1, -1, NULL}
};

static spell_info *_realm_get_spells(void)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        return _lore_spells;
    case DRAGON_REALM_BREATH:
        return _breath_spells;
    case DRAGON_REALM_ATTACK:
        return _attack_spells;
    case DRAGON_REALM_ARMOR:
        return _armor_spells;
    case DRAGON_REALM_CRAFT:
        return _craft_spells;
    case DRAGON_REALM_CRUSADE:
        return _crusade_spells;
    case DRAGON_REALM_DEATH:
        return _death_spells;
    case DRAGON_REALM_DOMINATION:
        return _domination_spells;
    }
    return NULL;
}

static void _realm_calc_bonuses(void)
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            p_ptr->telepathy = TRUE;
        if (p_ptr->lev >= 40)
            p_ptr->auto_id = TRUE;
        else
            p_ptr->auto_pseudo_id = TRUE;
        break;
    case DRAGON_REALM_BREATH:
        p_ptr->to_a -= p_ptr->lev/2;
        p_ptr->dis_to_a -= p_ptr->lev/2;
        break;
    case DRAGON_REALM_ATTACK:
        res_add(RES_FEAR);
        break;
    case DRAGON_REALM_ARMOR:
        p_ptr->to_a += p_ptr->lev;
        p_ptr->dis_to_a += p_ptr->lev;
        if (p_ptr->lev >= 5)
            p_ptr->sustain_dex = TRUE;
        if (p_ptr->lev >= 10)
            p_ptr->sustain_str = TRUE;
        if (p_ptr->lev >= 15)
            p_ptr->sustain_con = TRUE;
        if (p_ptr->lev >= 20)
            p_ptr->sustain_chr = TRUE;
        if (p_ptr->lev >= 25)
            p_ptr->hold_life++;
        if (p_ptr->lev >= 30)
            p_ptr->no_cut = TRUE;
        if (p_ptr->lev >= 35)
            res_add(RES_POIS);
        if (p_ptr->lev >= 40)
        {
            p_ptr->reflect = TRUE;
            p_ptr->no_stun = TRUE;
        }         /* v---- This is a timer but is not following the naming convention! */
        if (p_ptr->resist_magic && p_ptr->lev >= 30) 
            p_ptr->magic_resistance = 5 + (p_ptr->lev - 30) / 2;
        break;
    case DRAGON_REALM_CRUSADE:
        p_ptr->align += 200;
        if (p_ptr->lev >= 15)
            p_ptr->hold_life++;
        if (p_ptr->lev >= 30)
            res_add(RES_FEAR);
        break;
    case DRAGON_REALM_DEATH:
        p_ptr->align -= 200;
        break;
    case DRAGON_REALM_DOMINATION:
        res_add(RES_FEAR);
        if (p_ptr->lev >= 30)
            p_ptr->cult_of_personality = TRUE;
        if (p_ptr->lev >= 50)
            res_add_immune(RES_FEAR);
        break;
    }
}

static void _realm_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    switch (p_ptr->dragon_realm)
    {
    case DRAGON_REALM_LORE:
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_TELEPATHY);
        break;
    case DRAGON_REALM_ATTACK:
        add_flag(flgs, OF_RES_FEAR);
        break;
    case DRAGON_REALM_ARMOR:
        if (p_ptr->lev >= 5)
            add_flag(flgs, OF_SUST_DEX);
        if (p_ptr->lev >= 10)
            add_flag(flgs, OF_SUST_STR);
        if (p_ptr->lev >= 15)
            add_flag(flgs, OF_SUST_CON);
        if (p_ptr->lev >= 20)
            add_flag(flgs, OF_SUST_CHR);
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_HOLD_LIFE);
        /*if (p_ptr->lev >= 30)
            add_flag(flgs, TR_NO_CUT);*/
        if (p_ptr->lev >= 35)
            add_flag(flgs, OF_RES_POIS);
        if (p_ptr->lev >= 40)
        {
            add_flag(flgs, OF_REFLECT);
            /*add_flag(flgs, TR_NO_STUN);*/
        }
        if (p_ptr->resist_magic && p_ptr->lev >= 30) 
            add_flag(flgs, OF_MAGIC_RESISTANCE); /* s/b a temp flag ... */
        break;
    case DRAGON_REALM_CRUSADE:
        if (p_ptr->lev >= 15)
            add_flag(flgs, OF_HOLD_LIFE);
        if (p_ptr->lev >= 30)
            add_flag(flgs, OF_RES_FEAR);
        break;
    case DRAGON_REALM_DOMINATION:
        add_flag(flgs, OF_RES_FEAR);
        if (p_ptr->lev >= 50)
            add_flag(flgs, OF_IM_FEAR);
        break;
    }
}

/**********************************************************************
 * Dragon Bonuses (Common to all Types)
 **********************************************************************/
static void _dragon_calc_bonuses(void) 
{
    p_ptr->skill_dig += 100;
    p_ptr->levitation = TRUE;
    if (p_ptr->lev >= 20)
    {
        p_ptr->free_act++;
        p_ptr->see_inv++;
    }
    if (p_ptr->lev >= 30)
    {
        res_add(RES_CONF);
        /*Attack, Crusade, and Domination Realms: res_add(RES_FEAR);*/
    }
    _realm_calc_bonuses();
}

static void _dragon_get_flags(u32b flgs[OF_ARRAY_SIZE]) 
{
    add_flag(flgs, OF_LEVITATION);
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, OF_FREE_ACT);
        add_flag(flgs, OF_SEE_INVIS);
    }
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_RES_CONF);
        /*Attack, Crusade, and Domination Realms: add_flag(flgs, TR_RES_FEAR);*/
    }
    _realm_get_flags(flgs);
}

/**********************************************************************
 * Dragon Powers (Common to all Types)
 **********************************************************************/
void dragon_reach_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "触及攻击");
        break;
    case SPELL_DESC:
        var_set_string(res, "伸长脖子撕咬远处的怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_range(2 + p_ptr->lev/40));
        break;
    case SPELL_CAST:
        {
            int dir = 5;
            var_set_bool(res, FALSE);
            project_length = 2 + p_ptr->lev/40;
            if (!get_fire_dir(&dir)) return;
            p_ptr->innate_attacks[0].flags |= INNATE_SKIP;
            project_hook(GF_ATTACK, dir, 0, PROJECT_STOP | PROJECT_KILL);
            p_ptr->innate_attacks[0].flags &= ~INNATE_SKIP;
            var_set_bool(res, TRUE);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dragon_tail_sweep_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "扫尾");
        break;
    case SPELL_DESC:
        var_set_string(res, "从选定方向开始，顺时针呈半圆形挥动你的尾巴。如果怪物被击中，可能会被击退一两格。");
        break;
    case SPELL_CAST:
        /* Hack: Replace normal tooth and claw attacks with a tail attack */
        p_ptr->innate_attack_lock = TRUE;
        p_ptr->innate_attack_ct = 0;
        {
            int             l = _attack_level();
            innate_attack_t a = {0};

            a.dd = 1 + l / 30;
            a.ds = 3 + l / 10;
            a.to_h += l / 2;

            a.weight = 100 + l;
            a.blows = 100;
            a.msg = "You hit.";
            a.name = "尾击";

            p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        }     
        var_set_bool(res, do_blow(DRAGON_TAIL_SWEEP));
        p_ptr->innate_attack_lock = FALSE;
        /* Hack: Restore normal attacks */
        p_ptr->innate_attack_ct = 0;
        _calc_innate_attacks();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dragon_wing_storm_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "风翼风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "这项天赋利用你的双翼制造巨大的阵风。附近的敌人会受到伤害、被震慑，并被吹飞。");
        break;
    case SPELL_CAST:
        msg_print("你强有力地挥下双翼！");
        project(0, 5, py, px, randint1(p_ptr->lev * 3), GF_STORM, PROJECT_KILL | PROJECT_ITEM);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static power_info _dragon_get_powers[] = {
    { A_CON, {  1,  0, 30, _breathe_spell}},
    { A_DEX, { 20,  7,  0, dragon_reach_spell}},
    { A_DEX, { 25, 10,  0, dragon_tail_sweep_spell}},
    { A_DEX, { 30, 20,  0, dragon_wing_storm_spell}},
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _steel_get_powers[] = {
    { A_DEX, { 20,  7,  0, dragon_reach_spell}},
    { A_DEX, { 25, 10,  0, dragon_tail_sweep_spell}},
    { A_DEX, { 30, 20,  0, dragon_wing_storm_spell}},
    {    -1, { -1, -1, -1, NULL} }
};

/**********************************************************************
 * Elemental Dragon (Red, White, Blue, Black, Green)
 *   Baby -> Young -> Mature -> Ancient -> Great Foo Wyrm
 **********************************************************************/
 typedef struct {
    int  r_idx[5];
    cptr r_name[5];
    int  which_res;
    cptr name; /* For Birth and Helpfiles ... */
    cptr desc;
} _elemental_info_t;

static _elemental_info_t _elemental_info[5] = { /* relies on #define DRAGON_RED 0 ... */
    { {167, 563, 589, 644, 756},
      {"红龙宝宝", "幼年红龙", "成年红龙", "远古红龙", "地狱巨龙"},
      RES_FIRE, "红龙",
        "Red Dragons are elemental dragons of fire, and the second-strongest fighters among "
        "dragons. Their fiery breaths are the stuff of legends with damage unsurpassed. Even their "
        "bites are likely to burn their opponents, further enhancing their deadliness in melee. "
        "As the Red Dragon matures, it becomes more and more resistant to fire, eventually gaining "
        "total immunity." },
    { {164, 460, 549, 617, 741},
      {"白龙宝宝", "幼年白龙", "成年白龙", "远古白龙", "寒冰巨龙"},
      RES_COLD, "白龙",
        "White Dragons are to cold what Red Dragons are to fire. Their melee is awe-inspiring, "
        "and their icy breath can be felt even in their bite. Together with Red Dragons, they "
        "have the deadliest breaths among dragonkind; and they become more and more "
        "resistant to cold as they mature, eventually attaining immunity." },
    { {163, 459, 560, 601, 728},
      {"蓝龙宝宝", "幼年蓝龙", "成年蓝龙", "远古蓝龙", "风暴巨龙"},
      RES_ELEC, "蓝龙",
        "Blue Dragons are elemental dragons of lightning. Their breaths are as "
        "strong as those of their Red and White brethren, their melee somewhat weaker but still mighty, "
        "and their bites eventually shock their foes. Blue Dragons become more and more "
        "resistant to lightning as they mature, eventually gaining total immunity." },
    { {166, 546, 592, 624, 1066},
      {"黑龙宝宝", "幼年黑龙", "成年黑龙", "远古黑龙", "强酸巨龙"},
      RES_ACID, "黑龙",
        "Black Dragons are to acid what Blue Dragons are to lightning. Their breaths are as strong "
        "as those of their Red, White and Blue kin; and fewer monsters resist acid, giving them an extra "
        "opening. As the Black Dragon matures, its bites become corrosive, and it grows more resistant to acid "
        "until it reaches immunity." },
    { {165, 461, 561, 618, 890},
      {"绿龙宝宝", "幼年绿龙", "成年绿龙", "远古绿龙", "剧毒巨龙"},
      RES_POIS, "绿龙",
        "Green Dragons are elemental dragons of venom. They are not quite as strong as Red or White dragons, "
        "but are still fearsome opponents. As they mature, their bites poison their enemies. "
        "Green Dragons become more and more resistant to poison over time, eventually achieving immunity." },
};

static void _elemental_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*5;
    int res = _elemental_info[p_ptr->psubrace].which_res;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(res);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(res);
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add_immune(res);
        res_add(RES_BLIND);
        switch (res)
        {
        case RES_FIRE: p_ptr->sh_fire++; break;
        case RES_COLD: p_ptr->sh_cold++; break;
        case RES_ELEC: p_ptr->sh_elec++; break;
        }
    }
    _dragon_calc_bonuses();
}
static void _elemental_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    int res = _elemental_info[p_ptr->psubrace].which_res;
    add_flag(flgs, res_get_object_flag(res));
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_RES_BLIND);
        switch (res)
        {
        case RES_FIRE: add_flag(flgs, OF_AURA_FIRE); break;
        case RES_COLD: add_flag(flgs, OF_AURA_COLD); break;
        case RES_ELEC: add_flag(flgs, OF_AURA_ELEC); break;
        }
        add_flag(flgs, res_get_object_immune_flag(res));
    }
    _dragon_get_flags(flgs);
}
static void _elemental_birth(void) { 
    p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[0]; 
    _dragon_birth();
}
static void _elemental_gain_level(int new_level) {
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[0] && new_level >= 10)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[1];
        msg_format("你进化成了%s。", _elemental_info[p_ptr->psubrace].r_name[1]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[1] && new_level >= 20)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[2];
        msg_format("你进化成了%s。", _elemental_info[p_ptr->psubrace].r_name[2]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[2] && new_level >= 30)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[3];
        msg_format("你进化成了%s。", _elemental_info[p_ptr->psubrace].r_name[3]);
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == _elemental_info[p_ptr->psubrace].r_idx[3] && new_level >= 40)
    {
        p_ptr->current_r_idx = _elemental_info[p_ptr->psubrace].r_idx[4];
        msg_format("你进化成了%s。", _elemental_info[p_ptr->psubrace].r_name[4]);
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_elemental_get_race_t(int subrace)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = 0;

    if (p_ptr->lev >= 10) rank++;
    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  70,  30};
    skills_t xs = {  8,   9,  10,   0,   0,   0,  20,   7};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _elemental_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _elemental_calc_bonuses;
        me.get_flags = _elemental_get_flags;
        me.gain_level = _elemental_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = _elemental_info[subrace].name;
    else
        me.subname = _elemental_info[subrace].r_name[rank];
    me.subdesc = _elemental_info[subrace].desc;
    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -1 + rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] =  0 + rank;
    me.stats[A_CHR] = -1 + rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Nether: Shadow Drake -> Death Drake -> Spectral Wyrm
 **********************************************************************/
static void _nether_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_NETHER);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
        res_add(RES_COLD);
        res_add(RES_TELEPORT);
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 45)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 2;
        p_ptr->sh_cold++;
        res_add(RES_POIS);
        res_add_immune(RES_NETHER);
        res_add(RES_NEXUS);
        res_add(RES_DISEN);
        res_add(RES_TELEPORT);
    }
    _dragon_calc_bonuses();
}
static void _nether_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_NETHER);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
        add_flag(flgs, OF_RES_COLD);
    }
    if (p_ptr->lev >= 45)
    {
        add_flag(flgs, OF_AURA_COLD);
        add_flag(flgs, OF_RES_POIS);
        add_flag(flgs, OF_RES_NEXUS);
        add_flag(flgs, OF_RES_DISEN);
        add_flag(flgs, OF_IM_NETHER);
    }
    _dragon_get_flags(flgs);
}
static void _nether_birth(void) { 
    p_ptr->current_r_idx = MON_SHADOW_DRAKE; 
    _dragon_birth();
}

static void _nether_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_SHADOW_DRAKE && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_DEATH_DRAKE;
        msg_print("你进化成了死亡龙(Death Drake)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_DEATH_DRAKE && new_level >= 45)
    {
        p_ptr->current_r_idx = MON_SPECTRAL_WYRM;
        msg_print("你进化成了幽灵巨龙(Spectral Wyrm)。");
        p_ptr->redraw |= PR_MAP;
    }
}

static race_t *_nether_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"暗影龙", "死亡龙", "幽灵巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 30) rank++;
    if (p_ptr->lev >= 45) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   4,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Shadow Drakes are among the stealthiest of dragons. They are creatures of nether, "
            "and eventually evolve into Death Drakes and Spectral Wyrms. Their melee is the weakest among dragonkind and "
            "their breaths are also somewhat lacking, but they still make fearsome opponents. As they advance, "
            "these dragons gain the ability to pass through walls, and acquire several resistances including "
            "immunity to nether.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _nether_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _nether_calc_bonuses;
        me.get_flags = _nether_get_flags;
        me.gain_level = _nether_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "暗影龙";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + rank;
    me.stats[A_CON] = -1 + rank;
    me.stats[A_CHR] = -1 + 3*rank;
    me.life = 90 + 5*rank;

    return &me;
}

/**********************************************************************
 * Law: Law Drake -> Great Wyrm of Law
 **********************************************************************/
static void _law_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);

    if (p_ptr->lev >= 40)
    {
        p_ptr->align += 200;
        p_ptr->pspeed += 5;
        res_add(RES_SOUND);
        res_add(RES_SHARDS);
    }

    _dragon_calc_bonuses();
}
static void _law_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_SHARDS);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _law_birth(void) { 
    p_ptr->current_r_idx = MON_LAW_DRAKE; 
    _dragon_birth();
}
static void _law_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_LAW_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_LAW;
        msg_print("你进化成了律法巨龙(Great Wyrm of Law)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_law_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"律法龙", "律法巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Law Drakes are powerful dragons of order. They can breathe sound or shards and eventually "
                    "evolve into Great Wyrms of Law, though not as quickly as you might hope. Their breaths "
                    "are much weaker than those of the elemental dragons, but far fewer monsters resist sound "
                    "or shards. Their melee is also on the weak side by dragon standards, but most lesser creatures "
                    "would be quite happy with it!";
        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 280;

        me.birth = _law_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _law_calc_bonuses;
        me.get_flags = _law_get_flags;
        me.gain_level = _law_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "律法龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Chaos: Chaos Drake -> Great Wyrm of Chaos
 **********************************************************************/
static void _chaos_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->align -= 200;
        p_ptr->pspeed += 5;
        res_add(RES_CHAOS);
        res_add(RES_DISEN);
    }

    _dragon_calc_bonuses();
}
static void _chaos_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_CHAOS);
    add_flag(flgs, OF_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _chaos_birth(void) { 
    p_ptr->current_r_idx = MON_CHAOS_DRAKE; 
    _dragon_birth();
}
static void _chaos_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CHAOS_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_CHAOS;
        msg_print("你进化成了混沌巨龙(Great Wyrm of Chaos)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_chaos_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"混沌龙", "混沌巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  40,  40,   2,  25,  26,  55,  30};
    skills_t xs = {  8,  11,  11,   0,   0,   0,  15,   7};

        me.subdesc = "Chaos Drakes are powerful dragons of chaos. They can breathe chaos or disenchantment and eventually "
        "evolve into Great Wyrms of Chaos, though not as quickly as you might hope. Their breaths "
        "are much weaker than those of the elemental dragons, but far fewer monsters resist chaos "
        "or disenchantment; and though their melee is weak for a dragon, by more typical standards "
        "they fight quite well.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 280;

        me.birth = _chaos_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _chaos_calc_bonuses;
        me.get_flags = _chaos_get_flags;
        me.gain_level = _chaos_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "混沌龙";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 4*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 100 + 10*rank;

    return &me;
}

/**********************************************************************
 * Balance: Balance Drake -> Great Wyrm of Balance
 **********************************************************************/
static void _balance_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 10 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    res_add(RES_SHARDS);
    res_add(RES_CHAOS);
    res_add(RES_DISEN);
    
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
    }
    _dragon_calc_bonuses();
}
static void _balance_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    add_flag(flgs, OF_RES_SHARDS);
    add_flag(flgs, OF_RES_CHAOS);
    add_flag(flgs, OF_RES_DISEN);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _balance_birth(void) { 
    p_ptr->current_r_idx = MON_BALANCE_DRAKE; 
    _dragon_birth();
}
static void _balance_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_BALANCE_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_WYRM_OF_BALANCE;
        msg_print("你进化成了平衡巨龙(Great Wyrm of Balance)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_balance_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"平衡龙", "平衡巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  35,   2,  25,  26,  50,  30};
    skills_t xs = {  8,  10,  10,   0,   0,   0,  15,   7};

        me.subdesc = "Balance Drakes are a blend of Chaos and Law Drakes. They can breathe sound, shards, "
        "chaos or disenchantment and eventually evolve into Great Wyrms of Balance. "
        "Their breaths and attacks are weaker than those of Chaos and Law Drakes, but not by much.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 300;

        me.birth = _balance_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _balance_calc_bonuses;
        me.get_flags = _balance_get_flags;
        me.gain_level = _balance_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "平衡龙";
    else
        me.subname = titles[rank];
    me.stats[A_STR] =  0 + 4*rank;
    me.stats[A_INT] = -1 + 4*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] = -2 + 3*rank;
    me.stats[A_CON] = -1 + 3*rank;
    me.stats[A_CHR] = -1 + 5*rank;
    me.life = 95 + 10*rank;

    return &me;
}

/**********************************************************************
 * Ethereal: Pseudo Dragon -> Ethereal Drake -> Ethereal Dragon
 **********************************************************************/
static void _ethereal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_LITE);
    res_add(RES_DARK);
    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pass_wall = TRUE;
        p_ptr->no_passwall_dam = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 5;
        res_add(RES_LITE);
        res_add(RES_DARK);
        res_add(RES_CONF);
    }
    _dragon_calc_bonuses();
}
static void _ethereal_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_LITE);
    add_flag(flgs, OF_RES_DARK);
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _ethereal_birth(void) { 
    p_ptr->current_r_idx = MON_PSEUDO_DRAGON; 
    _dragon_birth();
}
static void _ethereal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_PSEUDO_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAKE;
        msg_print("你进化成了以太龙(Ethereal Drake)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_ETHEREAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_ETHEREAL_DRAGON;
        msg_print("你进化成了以太巨龙(Ethereal Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_ethereal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"伪龙", "以太龙", "以太巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  37,   4,  25,  26,  52,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc =
        "Ethereal Drakes are dragons of light and darkness. They begin life as modest Pseudo "
        "Dragons, but quickly evolve into Ethereal Drakes and then Ethereal Dragons. As they "
        "mature, they gain the ability to pass through walls and become more and more resistant "
        "to light, darkness and confusion. They are fairly weak fighters and have the weakest "
        "breaths in all of dragonkind, except for Steel Dragons which cannot breathe at all.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _ethereal_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _ethereal_calc_bonuses;
        me.get_flags = _ethereal_get_flags;
        me.gain_level = _ethereal_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "以太龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 95 + 7*rank;

    return &me;
}

/**********************************************************************
 * Crystal: Crystal Drake -> Great Crystal Drake
 **********************************************************************/
static void _crystal_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level_aux(125, 1, 2, 2);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_COLD);
    res_add(RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        p_ptr->pspeed++;
    }    
    if (p_ptr->lev >= 20)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed++;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
        res_add(RES_SHARDS);
        p_ptr->reflect = TRUE;
    }
    _dragon_calc_bonuses();
}
static void _crystal_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_COLD);
    add_flag(flgs, OF_RES_SHARDS);
    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, OF_SPEED);
    }
    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_REFLECT);
    }
    _dragon_get_flags(flgs);
}
static void _crystal_birth(void) { 
    p_ptr->current_r_idx = MON_CRYSTAL_DRAKE; 
    _dragon_birth();
}
static void _crystal_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_CRYSTAL_DRAKE && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_GREAT_CRYSTAL_DRAKE;
        msg_print("你进化成了水晶巨龙(Great Crystal Drake)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_crystal_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"水晶龙", "水晶巨龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  40,   1,  25,  26,  70,  30};
    skills_t xs = {  8,   7,  12,   0,   0,   0,  22,   7};

        me.subdesc =
        "Crystal Drakes are dragons of a strange crystalline form. They breathe shards and fight "
        "powerfully with their razor-sharp claws and teeth. At high levels, they gain the power of "
        "reflection.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 275;

        me.birth = _crystal_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _crystal_calc_bonuses;
        me.get_flags = _crystal_get_flags;
        me.gain_level = _crystal_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "水晶龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  1 + 5*rank;
    me.stats[A_INT] = -1 + 5*rank;
    me.stats[A_WIS] = -2 + 2*rank;
    me.stats[A_DEX] =  0 + 3*rank;
    me.stats[A_CON] =  0 + 4*rank;
    me.stats[A_CHR] =  0 + 3*rank;
    me.life = 100 + 15*rank;

    return &me;
}

/**********************************************************************
 * Bronze: Young -> Mature -> Ancient
 **********************************************************************/
static void _bronze_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_CONF);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _bronze_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_CONF);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _bronze_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_BRONZE_DRAGON; 
    _dragon_birth();
}
static void _bronze_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_BRONZE_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_BRONZE_DRAGON;
        msg_print("你进化成了成年青铜龙(Mature Bronze Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_BRONZE_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_BRONZE_DRAGON;
        msg_print("你进化成了远古青铜龙(Ancient Bronze Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_bronze_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"幼年青铜龙", "成年青铜龙", "远古青铜龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   3,  25,  26,  55,  30};
    skills_t xs = {  8,  10,  11,   0,   0,   0,  15,   7};

        me.subdesc =
        "Bronze Dragons are wyrms of perplexity. While they are not quite as strong as most other "
        "dragons, they eventually confuse monsters with their bite attack; also, they become "
        "more and more resistant to confusion as they mature.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _bronze_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _bronze_calc_bonuses;
        me.get_flags = _bronze_get_flags;
        me.gain_level = _bronze_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "青铜龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Gold: Young -> Mature -> Ancient
 **********************************************************************/
static void _gold_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(75);
    int ac = 15 + (l/10)*2;

    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    res_add(RES_SOUND);
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->pspeed += 3;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SOUND);
        p_ptr->pspeed += 2;
    }
    _dragon_calc_bonuses();
}
static void _gold_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_SOUND);
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, OF_SPEED);
    }
    _dragon_get_flags(flgs);
}
static void _gold_birth(void) { 
    p_ptr->current_r_idx = MON_YOUNG_GOLD_DRAGON; 
    _dragon_birth();
}
static void _gold_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_YOUNG_GOLD_DRAGON && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_MATURE_GOLD_DRAGON;
        msg_print("你进化成了成年金龙(Mature Gold Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MATURE_GOLD_DRAGON && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_ANCIENT_GOLD_DRAGON;
        msg_print("你进化成了远古金龙(Ancient Gold Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_gold_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[3] =  {"幼年金龙", "成年金龙", "远古金龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 20) rank++;
    if (p_ptr->lev >= 30) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  35,  38,   2,  25,  26,  55,  30};
    skills_t xs = {  8,   9,  11,   0,   0,   0,  20,   7};

        me.subdesc =
        "Gold Dragons are wyrms of sound. While they are not quite as strong as most other "
        "dragons, they are able to breathe sound on command, stunning their foes; also, they become "
        "more and more resistant to sound as they mature.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _gold_birth;
        me.get_powers = _dragon_get_powers;
        me.calc_bonuses = _gold_calc_bonuses;
        me.get_flags = _gold_get_flags;
        me.gain_level = _gold_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "金龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  0 + 2*rank;
    me.stats[A_INT] = -1 + 2*rank;
    me.stats[A_WIS] = -2 + rank;
    me.stats[A_DEX] = -2 + 2*rank;
    me.stats[A_CON] = -1 + 2*rank;
    me.stats[A_CHR] = -1 + 2*rank;
    me.life = 100 + 5*rank;

    return &me;
}

/**********************************************************************
 * Steel: Stone Dragon -> Steel Dragon
 **********************************************************************/
static void _steel_calc_bonuses(void) {
    int l = p_ptr->lev;
    int to_a = py_prorata_level(144);
    int ac = 15 + (l/10)*2;

    p_ptr->skill_dig += 100;
    
    p_ptr->ac += ac;
    p_ptr->dis_ac += ac;

    p_ptr->to_a += to_a;
    p_ptr->dis_to_a += to_a;

    if (p_ptr->lev < 40)
        res_add_vuln(RES_COLD);

    res_add(RES_FIRE);
    res_add(RES_ELEC);
    res_add(RES_POIS);
    p_ptr->no_cut = TRUE;
    p_ptr->pspeed -= 1;
    
    if (p_ptr->lev >= 30)
    {
        p_ptr->no_stun = TRUE;
    }
    if (p_ptr->lev >= 40)
    {
        res_add(RES_SHARDS);
    }
    _dragon_calc_bonuses();
}
static void _steel_get_flags(u32b flgs[OF_ARRAY_SIZE]) {
    add_flag(flgs, OF_RES_FIRE);
    add_flag(flgs, OF_RES_ELEC);
    add_flag(flgs, OF_RES_POIS);
    add_flag(flgs, OF_SPEED);

    if (p_ptr->lev >= 40)
    {
        add_flag(flgs, OF_RES_SHARDS);
    }
    if (p_ptr->lev < 40)
        add_flag(flgs, OF_VULN_COLD);

    _dragon_get_flags(flgs);
}
static void _steel_birth(void) { 
    p_ptr->current_r_idx = MON_STONE_DRAGON; 
    _dragon_birth();
}
static void _steel_gain_level(int new_level) {
    if (p_ptr->current_r_idx == MON_STONE_DRAGON && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_STEEL_DRAGON;
        msg_print("你进化成了钢龙(Steel Dragon)。");
        p_ptr->redraw |= PR_MAP;
    }
}
static race_t *_steel_get_race_t(void)
{
    static race_t me = {0};
    static bool   init = FALSE;
    static cptr   titles[2] =  {"石龙", "钢龙"};    
    int           rank = 0;

    if (p_ptr->lev >= 40) rank++;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 28,  18,  40,   0,  10,   7,  72,  30};
    skills_t xs = {  8,   7,  15,   0,   0,   0,  27,   7};

        me.subdesc =
        "Steel Dragons are magical dragons formed from rock. As they mature, their form hardens "
        "from stone into steel. Needless to say, their armor class is phenomenal, but their "
        "dexterity actually decreases with maturity. Steel dragons begin life being susceptible "
        "to cold damage, though they will eventually outgrow this vulnerability. They are not "
        "as fast as other dragons and have no powers whatsoever, not even the ubiquitous "
        "dragon breath! But their fighting is impossibly strong, putting all the other dragons "
        "to complete and utter shame. They also have extremely high hitpoints.";

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        me.exp = 250;

        me.birth = _steel_birth;
        me.calc_bonuses = _steel_calc_bonuses;
        me.get_flags = _steel_get_flags;
        me.get_powers = _steel_get_powers;
        me.gain_level = _steel_gain_level;
        init = TRUE;
    }

    if (spoiler_hack || birth_hack)
        me.subname = "钢龙";
    else
        me.subname = titles[rank];

    me.stats[A_STR] =  5 + (p_ptr->lev / 10);
    me.stats[A_INT] = -6;
    me.stats[A_WIS] = -6;
    me.stats[A_DEX] =  0 - (p_ptr->lev / 10);
    me.stats[A_CON] =  4 + (p_ptr->lev / 10);
    me.stats[A_CHR] =  0 + (p_ptr->lev / 10);
    me.life = 120 + (p_ptr->lev / 2);

    return &me;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_dragon_get_race(int psubrace)
{
    race_t *result = NULL;

    switch (psubrace)
    {
    case DRAGON_RED:
    case DRAGON_WHITE:
    case DRAGON_BLUE:
    case DRAGON_BLACK:
    case DRAGON_GREEN:
        result = _elemental_get_race_t(psubrace);
        break;
    case DRAGON_NETHER:
        result = _nether_get_race_t();
        break;
    case DRAGON_LAW:
        result = _law_get_race_t();
        break;
    case DRAGON_CHAOS:
        result = _chaos_get_race_t();
        break;
    case DRAGON_BALANCE:
        result = _balance_get_race_t();
        break;
    case DRAGON_ETHEREAL:
        result = _ethereal_get_race_t();
        break;
    case DRAGON_CRYSTAL:
        result = _crystal_get_race_t();
        break;
    case DRAGON_BRONZE:
        result = _bronze_get_race_t();
        break;
    case DRAGON_GOLD:
        result = _gold_get_race_t();
        break;
    case DRAGON_STEEL:
        result = _steel_get_race_t();
        break;
    default: /* Birth Menus */
        result = _nether_get_race_t();
    }

    if (p_ptr->dragon_realm && !birth_hack && !spoiler_hack)
    {
        dragon_realm_ptr realm = _get_realm();
        int              i;

        for (i = 0; i < MAX_STATS; i++)
            result->stats[i] += realm->stats[i];

        result->caster_info = _caster_info;
        result->get_spells_fn = _realm_get_spells;
    }
    else
    {
        result->caster_info = NULL;
        result->get_spells_fn = NULL;
    }

    result->name = "龙";
    result->desc = _desc;
    result->flags = RACE_IS_MONSTER;
    result->calc_innate_attacks = _calc_innate_attacks;
    result->equip_template = mon_get_equip_template();
    result->base_hp = 40;
    result->pseudo_class_idx = CLASS_BEASTMASTER;
    result->shop_adjust = 130;

    result->boss_r_idx = MON_GLAURUNG;
    return result;
}


