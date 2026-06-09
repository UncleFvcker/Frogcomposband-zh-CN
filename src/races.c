#include "angband.h"

#include <assert.h>

/****************************************************************
 * Public Entrypoints
 ****************************************************************/
bool prace_is_(int which)
{
    if (p_ptr->mimic_form == which)
        return TRUE;
    else if (p_ptr->mimic_form == MIMIC_NONE && p_ptr->prace == which)
        return TRUE;

    return FALSE;
}

typedef struct {
    cptr name;
    int id;
} _race_alias_t;

static _race_alias_t _race_aliases[] = {
        { "Amberite", RACE_AMBERITE },
        { "Android", RACE_ANDROID },
        { "Archon", RACE_ARCHON },
        { "Balrog", RACE_BALROG },
        { "Barbarian", RACE_BARBARIAN },
        { "Beastman", RACE_BEASTMAN },
        { "Beorning", RACE_BEORNING },
        { "Boit", RACE_BOIT },
        { "Centaur", RACE_CENTAUR },
        { "Cyclops", RACE_CYCLOPS },
        { "Dark-Elf", RACE_DARK_ELF },
        { "Demigod", RACE_DEMIGOD },
        { "Doppelganger", RACE_DOPPELGANGER },
        { "Draconian", RACE_DRACONIAN },
        { "Dunadan", RACE_DUNADAN },
        { "Dwarf", RACE_DWARF },
        { "Einheri", RACE_EINHERI },
        { "Ent", RACE_ENT },
        { "Gnome", RACE_GNOME },
        { "Golem", RACE_GOLEM },
        { "Half-Giant", RACE_HALF_GIANT },
        { "Half-Orc", RACE_HALF_ORC },
        { "Half-Titan", RACE_HALF_TITAN },
        { "Half-Troll", RACE_HALF_TROLL },
        { "High-Elf", RACE_HIGH_ELF },
        { "Hobbit", RACE_HOBBIT },
        { "Human", RACE_HUMAN },
        { "Igor", RACE_IGOR },
        { "Imp", RACE_IMP },
        { "Klackon", RACE_KLACKON },
        { "Kobold", RACE_KOBOLD },
        { "Kutar", RACE_KUTAR },
        { "Maia", RACE_MAIA },
        { "Mindflayer", RACE_MIND_FLAYER },
        { "Angel", RACE_MON_ANGEL },
        { "Beholder", RACE_MON_BEHOLDER },
        { "Centipede", RACE_MON_CENTIPEDE },
        { "Demon", RACE_MON_DEMON },
        { "Dragon", RACE_MON_DRAGON },
        { "Elemental", RACE_MON_ELEMENTAL },
        { "Filthy-Rag", RACE_MON_ARMOR },
        { "Giant", RACE_MON_GIANT },
        { "Golem", RACE_MON_GOLEM },
        { "Hound", RACE_MON_HOUND },
        { "Hydra", RACE_MON_HYDRA },
        { "Jelly", RACE_MON_JELLY },
        { "Leprechaun", RACE_MON_LEPRECHAUN },
        { "Lich", RACE_MON_LICH },
        { "Mimic", RACE_MON_MIMIC },
        { "Orc", RACE_MON_ORC },
        { "Possessor", RACE_MON_POSSESSOR },
        { "Quylthulg", RACE_MON_QUYLTHULG },
        { "Spider", RACE_MON_SPIDER },
        { "Death-Sword", RACE_MON_SWORD },
        { "Ring", RACE_MON_RING },
        { "Pumpkin", RACE_MON_PUMPKIN },
        { "Mummy", RACE_MON_MUMMY },
        { "Troll", RACE_MON_TROLL },
        { "Vampire", RACE_MON_VAMPIRE },
        { "Vortex", RACE_MON_VORTEX },
        { "Xorn", RACE_MON_XORN },
        { "Clay-Golem", MIMIC_CLAY_GOLEM },
        { "Colossus", MIMIC_COLOSSUS },
        { "Bat", MIMIC_BAT },
        { "Mist", MIMIC_MIST },
        { "Wolf", MIMIC_WOLF },
        { "Demon", MIMIC_DEMON },
        { "Demon-Lord", MIMIC_DEMON_LORD },
        { "Iron-Golem", MIMIC_IRON_GOLEM },
        { "Mithril-Golem", MIMIC_MITHRIL_GOLEM },
        { "Vampire-Lord", MIMIC_VAMPIRE },
        { "Small-Kobold", MIMIC_SMALL_KOBOLD },
        { "Mangy-Leper", MIMIC_MANGY_LEPER },
        { "Karrot", MIMIC_DRAGON },
        { "Nibelung", RACE_NIBELUNG },
        { "Ogre", RACE_OGRE },
        { "Half-Ogre", RACE_OGRE },
        { "Shadow-Fairy", RACE_SHADOW_FAIRY },
        { "Skeleton", RACE_SKELETON },
        { "Snotling", RACE_SNOTLING },
        { "Spectre", RACE_SPECTRE },
        { "Sprite", RACE_SPRITE },
        { "Tomte", RACE_TOMTE },
        { "Tonberry", RACE_TONBERRY },
        { "Vampire", RACE_VAMPIRE },
        { "Werewolf", RACE_WEREWOLF },
        { "Wood-Elf", RACE_WOOD_ELF },
        { "Yeek", RACE_YEEK },
        { "Zombie", RACE_ZOMBIE },
        { NULL, -1 }
};

static bool _is_player_monster_or_mimic(int prace)
{
    switch (prace)
    {
    case RACE_MON_JELLY:
    case RACE_MON_SPIDER:
    case RACE_MON_DRAGON:
    case RACE_MON_LICH:
    case RACE_MON_XORN:
    case RACE_MON_ANGEL:
    case RACE_MON_HOUND:
    case RACE_MON_GIANT:
    case RACE_MON_BEHOLDER:
    case RACE_MON_DEMON:
    case RACE_MON_HYDRA:
    case RACE_MON_LEPRECHAUN:
    case RACE_MON_TROLL:
    case RACE_MON_ELEMENTAL:
    case RACE_MON_SWORD:
    case RACE_MON_RING:
    case RACE_MON_GOLEM:
    case RACE_MON_QUYLTHULG:
    case RACE_MON_POSSESSOR:
    case RACE_MON_MIMIC:
    case RACE_MON_VAMPIRE:
    case RACE_MON_ARMOR:
    case RACE_MON_ORC:
    case RACE_MON_PUMPKIN:
    case RACE_MON_MUMMY:
    case RACE_MON_CENTIPEDE:
    case RACE_MON_VORTEX:
    case MIMIC_CLAY_GOLEM:
    case MIMIC_COLOSSUS:
    case MIMIC_BAT:
    case MIMIC_MIST:
    case MIMIC_WOLF:
    case MIMIC_DEMON:
    case MIMIC_DEMON_LORD:
    case MIMIC_IRON_GOLEM:
    case MIMIC_MITHRIL_GOLEM:
    case MIMIC_VAMPIRE:
    case MIMIC_SMALL_KOBOLD:
    case MIMIC_MANGY_LEPER:
    case MIMIC_DRAGON:
        return TRUE;
    }
    return FALSE;
}

static int _lookup_race_alias(cptr name)
{
    int i;

    for (i = 0; _race_aliases[i].name; i++)
    {
        if (!initialized && _is_player_monster_or_mimic(_race_aliases[i].id))
            continue;
        if (strcmp(name, _race_aliases[i].name) == 0)
            return _race_aliases[i].id;
    }
    return -1;
}

int get_race_idx(cptr name)
{
    int i;
    for (i = 0; i < MAX_RACES; i++)
    {
        race_t *race_ptr;

        /* r_info.txt contains racial references, but player monster races
           might try to index r_info as well! */
        if (!initialized)
        {
            switch (i)
            {
            case RACE_MON_JELLY:
            case RACE_MON_SPIDER:
            case RACE_MON_DRAGON:
            case RACE_MON_LICH:
            case RACE_MON_XORN:
            case RACE_MON_ANGEL:
            case RACE_MON_HOUND:
            case RACE_MON_GIANT:
            case RACE_MON_BEHOLDER:
            case RACE_MON_DEMON:
            case RACE_MON_HYDRA:
            case RACE_MON_LEPRECHAUN:
            case RACE_MON_TROLL:
            case RACE_MON_ELEMENTAL:
            case RACE_MON_SWORD:
            case RACE_MON_RING:
            case RACE_MON_GOLEM:
            case RACE_MON_QUYLTHULG:
            case RACE_MON_POSSESSOR:
            case RACE_MON_MIMIC:
            case RACE_MON_VAMPIRE:
            case RACE_MON_ARMOR:
            case RACE_MON_ORC:
            case RACE_MON_PUMPKIN:
            case RACE_MON_MUMMY:
                continue;
            }
        }

        race_ptr = get_race_aux(i, 0);
        if (race_ptr && strcmp(name, race_ptr->name) == 0)
            return i;
    }
    return _lookup_race_alias(name);
}

cptr get_race_internal_name(int prace)
{
    int i;
    race_t *race_ptr;

    for (i = 0; _race_aliases[i].name; i++)
    {
        if (_race_aliases[i].id == prace)
            return _race_aliases[i].name;
    }
    race_ptr = get_race_aux(prace, 0);
    if (race_ptr && race_ptr->name)
        return race_ptr->name;
    return "Unknown";
}

race_t *get_race_aux(int prace, int psubrace)
{
    race_t *result = NULL;

    switch (prace)
    {
    /* Player Races */
    case RACE_AMBERITE:
        result = amberite_get_race();
        break;
    case RACE_ANDROID:
        result = android_get_race();
        break;
    case RACE_ARCHON:
        result = archon_get_race();
        break;
    case RACE_BARBARIAN:
        result = barbarian_get_race();
        break;
    case RACE_BEASTMAN:
        result = beastman_get_race();
        break;
    case RACE_BEORNING:
        result = beorning_get_race();
        break;
    case RACE_BOIT:
        result = boit_get_race();
        break;
    case RACE_CENTAUR:
        result = centaur_get_race();
        break;
    case RACE_CYCLOPS:
        result = cyclops_get_race();
        break;
    case RACE_DARK_ELF:
        result = dark_elf_get_race();
        break;
    case RACE_DEMIGOD:
        result = demigod_get_race(psubrace);
        break;
    case RACE_BALROG:
        result = balrog_get_race();
        break;
    case RACE_DOPPELGANGER:
        result = doppelganger_get_race();
        break;
    case RACE_DUNADAN:
        result = dunadan_get_race();
        break;
    case RACE_DRACONIAN:
        result = draconian_get_race(psubrace);
        break;
    case RACE_DWARF:
        result = dwarf_get_race();
        break;
    case RACE_EINHERI:
        result = einheri_get_race();
        break;
    case RACE_ENT:
        result = ent_get_race();
        break;
    case RACE_GNOME:
        result = gnome_get_race();
        break;
    case RACE_GOLEM:
        result = golem_get_race();
        break;
    case RACE_HALF_GIANT:
        result = half_giant_get_race();
        break;
    case RACE_HALF_ORC:
	result = half_orc_get_race();
	break;
    case RACE_HALF_TITAN:
        result = half_titan_get_race();
        break;
    case RACE_HALF_TROLL:
        result = half_troll_get_race();
        break;
    case RACE_HIGH_ELF:
        result = high_elf_get_race();
        break;
    case RACE_HOBBIT:
        result = hobbit_get_race();
        break;
    case RACE_HUMAN:
        result = human_get_race();
        break;
    case RACE_IGOR:
        result = igor_get_race();
        break;
    case RACE_IMP:
        result = imp_get_race();
        break;
    case RACE_KLACKON:
        result = klackon_get_race();
        break;
    case RACE_KOBOLD:
        result = kobold_get_race();
        break;
    case RACE_KUTAR:
        result = kutar_get_race();
        break;
    case RACE_MAIA:
        result = maia_get_race(psubrace);
        break;
    case RACE_MIND_FLAYER:
        result = mindflayer_get_race();
        break;
    case RACE_MON_ANGEL:
        result = mon_angel_get_race();
        break;
    case RACE_MON_BEHOLDER:
        result = mon_beholder_get_race();
        break;
    case RACE_MON_CENTIPEDE:
        result = mon_centipede_get_race();
        break;
    case RACE_MON_DEMON:
        result = mon_demon_get_race(psubrace);
        break;
    case RACE_MON_DRAGON:
        result = mon_dragon_get_race(psubrace);
        break;
    case RACE_MON_ELEMENTAL:
        result = mon_elemental_get_race(psubrace);
        break;
    case RACE_MON_GIANT:
        result = mon_giant_get_race(psubrace);
        break;
    case RACE_MON_GOLEM:
        result = mon_golem_get_race(psubrace);
        break;
    case RACE_MON_HOUND:
        result = mon_hound_get_race();
        break;
    case RACE_MON_HYDRA:
        result = mon_hydra_get_race();
        break;
    case RACE_MON_JELLY:
        result = mon_jelly_get_race();
        break;
    case RACE_MON_LEPRECHAUN:
        result = mon_leprechaun_get_race();
        break;
    case RACE_MON_LICH:
        result = mon_lich_get_race();
        break;
    case RACE_MON_MIMIC:
        result = mon_mimic_get_race();
        break;
    case RACE_MON_ORC:
        result = mon_orc_get_race(psubrace);
        break;
    case RACE_MON_POSSESSOR:
        result = mon_possessor_get_race();
        break;
    case RACE_MON_QUYLTHULG:
        result = mon_quylthulg_get_race();
        break;
    case RACE_MON_SPIDER:
        result = mon_spider_get_race(psubrace);
        break;
    case RACE_MON_SWORD:
        result = mon_sword_get_race();
        break;
    case RACE_MON_RING:
        result = mon_ring_get_race();
        break;
    case RACE_MON_ARMOR:
        result = mon_armor_get_race();
        break;
    case RACE_MON_PUMPKIN:
        result = mon_pumpkin_get_race();
        break;
    case RACE_MON_MUMMY:
        result = mon_mummy_get_race();
        break;
    case RACE_MON_TROLL:
        result = mon_troll_get_race(psubrace);
        break;
    case RACE_MON_VAMPIRE:
        result = mon_vampire_get_race();
        break;
    case RACE_MON_VORTEX:
        result = mon_vortex_get_race();
        break;
    case RACE_MON_XORN:
        result = mon_xorn_get_race();
        break;
    case RACE_NIBELUNG:
        result = nibelung_get_race();
        break;
    case RACE_OGRE:
        result = ogre_get_race();
        break;
    case RACE_SHADOW_FAIRY:
        result = shadow_fairy_get_race();
        break;
    case RACE_SKELETON:
        result = skeleton_get_race();
        break;
    case RACE_SNOTLING:
        result = snotling_get_race();
        break;
    case RACE_SPECTRE:
        result = spectre_get_race();
        break;
    case RACE_SPRITE:
        result = sprite_get_race();
        break;
    case RACE_TOMTE:
        result = tomte_get_race();
        break;
    case RACE_TONBERRY:
        result = tonberry_get_race();
        break;
    case RACE_VAMPIRE:
        result = vampire_get_race();
        break;
    case RACE_WEREWOLF:
        result = werewolf_get_race();
        break;
    case RACE_WOOD_ELF:
        result = wood_elf_get_race();
        break;
    case RACE_YEEK:
        result = yeek_get_race();
        break;
    case RACE_ZOMBIE:
        result = zombie_get_race();
        break;
    /* Mimic Races */
    case MIMIC_CLAY_GOLEM:
        result = clay_golem_get_race();
        break;
    case MIMIC_COLOSSUS:
        result = colossus_get_race();
        break;
    case MIMIC_BAT:
        result = bat_get_race();
        break;
    case MIMIC_MIST:
        result = mist_get_race();
        break;
    case MIMIC_WOLF:
        result = wolf_get_race();
        break;
    case MIMIC_DEMON:
        result = demon_get_race();
        break;
    case MIMIC_DEMON_LORD:
        result = demon_lord_get_race();
        break;
    case MIMIC_IRON_GOLEM:
        result = iron_golem_get_race();
        break;
    case MIMIC_MITHRIL_GOLEM:
        result = mithril_golem_get_race();
        break;
    case MIMIC_VAMPIRE:
        result = vampire_lord_get_race();
        break;
    case MIMIC_SMALL_KOBOLD:
        result = small_kobold_get_race();
        break;
    case MIMIC_MANGY_LEPER:
        result = mangy_leper_get_race();
        break;
    case MIMIC_DRAGON:
        result = karrot_dragon_get_race();
        break;
    }

    assert(result);
    result->id = prace;
    result->subid = psubrace;
    return result;
}

race_t *get_true_race(void)
{
    return get_race_aux(p_ptr->prace, p_ptr->psubrace);
}

race_t *get_race(void)
{
    race_t *result;
    if (p_ptr->mimic_form != MIMIC_NONE)
    {
        result = get_race_aux(p_ptr->mimic_form, 0);
        result->mimic = TRUE;
    }
    else
    {
        result = get_race_aux(p_ptr->prace, p_ptr->psubrace);
        result->mimic = FALSE;
    }
    return result;
}
