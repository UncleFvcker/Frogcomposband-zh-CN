#include "angband.h"

bool spoiler_hack = FALSE;

#ifdef ALLOW_SPOILERS

typedef void(*_file_fn)(FILE*);
static bool _preserve_localized_help_file(cptr name)
{
    FILE *fp = NULL;
    char  buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "r");
    if (fp)
    {
        my_fclose(fp);
        if (character_dungeon) msg_format("保留已汉化帮助文件 %s", buf);
        return TRUE;
    }

    return FALSE;
}

static void _help_file(cptr name, _file_fn fn)
{
    FILE    *fp = NULL;
    char    buf[1024];

    if (_preserve_localized_help_file(name))
        return;

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "w");

    if (!fp)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
        fp = my_fopen(buf, "w");

        if (!fp)
        {
            prt("失败！", 0, 0);
            (void)inkey();
            return;
        }
    }

    fn(fp);
    fprintf(fp, "\n\n<color:s>自动生成于 RoguelikeFansBand %s。</color>\n",
            VERSION_STRING);

    my_fclose(fp);
    if (character_dungeon) msg_format("已创建 %s", buf);
}

static void _csv_file(cptr name, _file_fn fn)
{
    FILE    *fp = NULL;
    char    buf[1024];

    path_build(buf, sizeof(buf), ANGBAND_DIR_HELP, name);
    fp = my_fopen(buf, "w");

    if (!fp)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
        fp = my_fopen(buf, "w");

        if (!fp)
        {
            prt("失败！", 0, 0);
            (void)inkey();
            return;
        }
    }

    fn(fp);

    my_fclose(fp);
    if (character_dungeon) msg_format("已创建 %s", buf);
}

/******************************************************************************
 * Skill Descriptions
 * Rather then displaying a meaningless and perhaps spoilerish number to the user,
 * let's display a description instead. These descriptions are only for comparison
 * purposes. For example, Warriors have "Bad" device skills while a Mage is "Superb".
 * Of course, a CL50 Warrior's Character Sheet might list their device skill as
 * "Superb", but it really is "Bad" ... Trust me!
 *
 * We use the 17/10 scale factor for personality modifiers because personality
 * skill modifiers are counted twice, through a normal component and through a
 * scaling component (this probably needs to be revisited some day...);
 * so at CL 35 you get (50+35)/50=1.7 times the original modifier.
 ******************************************************************************/
static cptr _skill_desc(int amt, int div)
{
    static char buf[255];
    skill_desc_t desc = skills_describe(amt, div);
    if (birth_hack) sprintf(buf, "<color:%c>%-10.10s</color>", attr_to_attr_char(desc.color), desc.desc);
    else sprintf(buf, "<color:%c>%-13.13s</color>", attr_to_attr_char(desc.color), desc.desc);
    return buf;
}

/* Disarming */
static cptr _dis_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra - 30, 7); }
static cptr _class_dis_skill_desc(class_t *class_ptr) { return _dis_skill_desc(class_ptr->base_skills.dis, class_ptr->extra_skills.dis); }
static cptr _mon_race_dis_skill_desc(race_t *race_ptr) { return _dis_skill_desc(race_ptr->skills.dis, race_ptr->extra_skills.dis); }

static cptr _dis_skill_desc2(int base) { return _skill_desc(base + 8, 2); }
static cptr _race_dis_skill_desc(race_t *race_ptr) { return _dis_skill_desc2(race_ptr->skills.dis); }
static cptr _pers_dis_skill_desc(personality_ptr pers_ptr) { return _dis_skill_desc2(pers_ptr->skills.dis * 17 / 10); }
static cptr _realm_dis_skill_desc(dragon_realm_ptr realm_ptr) { return _dis_skill_desc2(realm_ptr->skills.dis); }

/* Devices */
static cptr _dev_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra - 45, 4); }
static cptr _class_dev_skill_desc(class_t *class_ptr) { return _dev_skill_desc(class_ptr->base_skills.dev, class_ptr->extra_skills.dev); }
static cptr _mon_race_dev_skill_desc(race_t *race_ptr) { return _dev_skill_desc(race_ptr->skills.dev, race_ptr->extra_skills.dev); }

static cptr _dev_skill_desc2(int base) { return _skill_desc(base + 8, 2); }
static cptr _race_dev_skill_desc(race_t *race_ptr) { return _dev_skill_desc2(race_ptr->skills.dev); }
static cptr _pers_dev_skill_desc(personality_ptr pers_ptr) { return _dev_skill_desc2(pers_ptr->skills.dev * 17 / 10); }
static cptr _realm_dev_skill_desc(dragon_realm_ptr realm_ptr) { return _dev_skill_desc2(realm_ptr->skills.dev); }

/* Saving Throws */
static cptr _sav_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra - 56, 4); }
static cptr _class_sav_skill_desc(class_t *class_ptr) { return _sav_skill_desc(class_ptr->base_skills.sav, class_ptr->extra_skills.sav); }
static cptr _mon_race_sav_skill_desc(race_t *race_ptr) { return _sav_skill_desc(race_ptr->skills.sav, race_ptr->extra_skills.sav); }

static cptr _sav_skill_desc2(int base) { return _skill_desc(base + 12, 3); }
static cptr _race_sav_skill_desc(race_t *race_ptr) { return _sav_skill_desc2(race_ptr->skills.sav); }
static cptr _pers_sav_skill_desc(personality_ptr pers_ptr) { return _sav_skill_desc2(pers_ptr->skills.sav * 17 / 10); }
static cptr _realm_sav_skill_desc(dragon_realm_ptr realm_ptr) { return _sav_skill_desc2(realm_ptr->skills.sav); }

/* Melee */
static cptr _thn_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra - 50, 10); }
static cptr _class_thn_skill_desc(class_t *class_ptr) { return _thn_skill_desc(class_ptr->base_skills.thn, class_ptr->extra_skills.thn); }
static cptr _mon_race_thn_skill_desc(race_t *race_ptr) { return _thn_skill_desc(race_ptr->skills.thn, race_ptr->extra_skills.thn); }

static cptr _thn_skill_desc2(int base) { return _skill_desc(base + 15, 3); }
static cptr _race_thn_skill_desc(race_t *race_ptr) { return _thn_skill_desc2(race_ptr->skills.thn); }
static cptr _pers_thn_skill_desc(personality_ptr pers_ptr) { return _thn_skill_desc2(pers_ptr->skills.thn * 17 / 10); }
static cptr _realm_thn_skill_desc(dragon_realm_ptr realm_ptr) { return _thn_skill_desc2(realm_ptr->skills.thn); }

/* Bows */
static cptr _thb_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra - 48, 8); }
static cptr _class_thb_skill_desc(class_t *class_ptr) { return _thb_skill_desc(class_ptr->base_skills.thb, class_ptr->extra_skills.thb); }
static cptr _mon_race_thb_skill_desc(race_t *race_ptr) { return _thb_skill_desc(race_ptr->skills.thb, race_ptr->extra_skills.thb); }

static cptr _thb_skill_desc2(int base) { return _skill_desc(base + 15, 3); }
static cptr _race_thb_skill_desc(race_t *race_ptr) { return _thb_skill_desc2(race_ptr->skills.thb); }
static cptr _pers_thb_skill_desc(personality_ptr pers_ptr) { return _thb_skill_desc2(pers_ptr->skills.thb * 17 / 10); }
static cptr _realm_thb_skill_desc(dragon_realm_ptr realm_ptr) { return _thb_skill_desc2(realm_ptr->skills.thb); }

/* Stealth */
static cptr _stl_skill_desc(int base, int xtra) { return _skill_desc(base * 3 + xtra * 12 + 1, 2); }
static cptr _class_stl_skill_desc(class_t *class_ptr) { return _stl_skill_desc(class_ptr->base_skills.stl, class_ptr->extra_skills.stl); }
static cptr _mon_race_stl_skill_desc(race_t *race_ptr) { return _stl_skill_desc(race_ptr->skills.stl, race_ptr->extra_skills.stl); }

static cptr _stl_skill_desc2(int base, int xtra) { return _skill_desc(base * 3 + xtra * 12 + 9, 2); }
static cptr _race_stl_skill_desc(race_t *race_ptr) { return _stl_skill_desc2(race_ptr->skills.stl, 0); }
static cptr _pers_stl_skill_desc(personality_ptr pers_ptr) { return _stl_skill_desc2(pers_ptr->skills.stl * 17 / 10, 0); }
static cptr _realm_stl_skill_desc(dragon_realm_ptr realm_ptr) { return _stl_skill_desc2(realm_ptr->skills.stl, 0); }

/* Searching */
static cptr _srh_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra, 6); }
static cptr _class_srh_skill_desc(class_t *class_ptr) { return _srh_skill_desc(class_ptr->base_skills.srh, class_ptr->extra_skills.srh); }
static cptr _mon_race_srh_skill_desc(race_t *race_ptr) { return _srh_skill_desc(race_ptr->skills.srh, race_ptr->extra_skills.srh); }

static cptr _srh_skill_desc2(int base) { return _skill_desc(base + 4, 1); }
static cptr _race_srh_skill_desc(race_t *race_ptr) { return _srh_skill_desc2(race_ptr->skills.srh); }
static cptr _pers_srh_skill_desc(personality_ptr pers_ptr) { return _srh_skill_desc2(pers_ptr->skills.srh * 17 / 10); }
static cptr _realm_srh_skill_desc(dragon_realm_ptr realm_ptr) { return _srh_skill_desc2(realm_ptr->skills.srh); }

/* Perception */
static cptr _fos_skill_desc(int base, int xtra) { return _skill_desc(base + 4*xtra, 6); }
static cptr _class_fos_skill_desc(class_t *class_ptr) { return _fos_skill_desc(class_ptr->base_skills.fos, class_ptr->extra_skills.fos); }
static cptr _mon_race_fos_skill_desc(race_t *race_ptr) { return _fos_skill_desc(race_ptr->skills.fos, race_ptr->extra_skills.fos); }

static cptr _fos_skill_desc2(int base) { return _skill_desc(base - 4, 1); }
static cptr _race_fos_skill_desc(race_t *race_ptr) { return _fos_skill_desc2(race_ptr->skills.fos - 2); }
static cptr _pers_fos_skill_desc(personality_ptr pers_ptr) { return _fos_skill_desc2((pers_ptr->skills.fos * 17 / 10) + 8); }
static cptr _realm_fos_skill_desc(dragon_realm_ptr realm_ptr) { return _fos_skill_desc2(realm_ptr->skills.fos + 8); }

void skills_desc_class(class_t *class_ptr, skills_desc_t *skills)
{
    strcpy(skills->dis, _class_dis_skill_desc(class_ptr));
    strcpy(skills->dev, _class_dev_skill_desc(class_ptr));
    strcpy(skills->sav, _class_sav_skill_desc(class_ptr));
    strcpy(skills->stl, _class_stl_skill_desc(class_ptr));
    strcpy(skills->srh, _class_srh_skill_desc(class_ptr));
    strcpy(skills->fos, _class_fos_skill_desc(class_ptr));
    strcpy(skills->thn, _class_thn_skill_desc(class_ptr));
    strcpy(skills->thb, _class_thb_skill_desc(class_ptr));
}

void skills_desc_mon_race(race_t *race_ptr, skills_desc_t *skills)
{
    strcpy(skills->dis, _mon_race_dis_skill_desc(race_ptr));
    strcpy(skills->dev, _mon_race_dev_skill_desc(race_ptr));
    strcpy(skills->sav, _mon_race_sav_skill_desc(race_ptr));
    strcpy(skills->stl, _mon_race_stl_skill_desc(race_ptr));
    strcpy(skills->srh, _mon_race_srh_skill_desc(race_ptr));
    strcpy(skills->fos, _mon_race_fos_skill_desc(race_ptr));
    strcpy(skills->thn, _mon_race_thn_skill_desc(race_ptr));
    strcpy(skills->thb, _mon_race_thb_skill_desc(race_ptr));
}

void skills_desc_race(race_t *race_ptr, skills_desc_t *skills)
{
    strcpy(skills->dis, _race_dis_skill_desc(race_ptr));
    strcpy(skills->dev, _race_dev_skill_desc(race_ptr));
    strcpy(skills->sav, _race_sav_skill_desc(race_ptr));
    strcpy(skills->stl, _race_stl_skill_desc(race_ptr));
    strcpy(skills->srh, _race_srh_skill_desc(race_ptr));
    strcpy(skills->fos, _race_fos_skill_desc(race_ptr));
    strcpy(skills->thn, _race_thn_skill_desc(race_ptr));
    strcpy(skills->thb, _race_thb_skill_desc(race_ptr));
}

void skills_desc_pers(personality_t *pers_ptr, skills_desc_t *skills)
{
    strcpy(skills->dis, _pers_dis_skill_desc(pers_ptr));
    strcpy(skills->dev, _pers_dev_skill_desc(pers_ptr));
    strcpy(skills->sav, _pers_sav_skill_desc(pers_ptr));
    strcpy(skills->stl, _pers_stl_skill_desc(pers_ptr));
    strcpy(skills->srh, _pers_srh_skill_desc(pers_ptr));
    strcpy(skills->fos, _pers_fos_skill_desc(pers_ptr));
    strcpy(skills->thn, _pers_thn_skill_desc(pers_ptr));
    strcpy(skills->thb, _pers_thb_skill_desc(pers_ptr));
}

void skills_desc_realm(dragon_realm_ptr realm_ptr, skills_desc_t *skills)
{
    strcpy(skills->dis, _realm_dis_skill_desc(realm_ptr));
    strcpy(skills->dev, _realm_dev_skill_desc(realm_ptr));
    strcpy(skills->sav, _realm_sav_skill_desc(realm_ptr));
    strcpy(skills->stl, _realm_stl_skill_desc(realm_ptr));
    strcpy(skills->srh, _realm_srh_skill_desc(realm_ptr));
    strcpy(skills->fos, _realm_fos_skill_desc(realm_ptr));
    strcpy(skills->thn, _realm_thn_skill_desc(realm_ptr));
    strcpy(skills->thb, _realm_thb_skill_desc(realm_ptr));
}

void skills_desc_aux(skills_t *base, skills_t *xtra, skills_desc_t *skills)
{
    strcpy(skills->dis, _dis_skill_desc(base->dis, xtra->dis));
    strcpy(skills->dev, _dev_skill_desc(base->dev, xtra->dev));
    strcpy(skills->sav, _sav_skill_desc(base->sav, xtra->sav));
    strcpy(skills->stl, _stl_skill_desc(base->stl, xtra->stl));
    strcpy(skills->srh, _srh_skill_desc(base->srh, xtra->srh));
    strcpy(skills->fos, _fos_skill_desc(base->fos, xtra->fos));
    strcpy(skills->thn, _thn_skill_desc(base->thn, xtra->thn));
    strcpy(skills->thb, _thb_skill_desc(base->thb, xtra->thb));
}

/******************************************************************************
 * Racial Help
 ******************************************************************************/

static void _race_help_table(FILE *fp, race_t *race_ptr)
{
    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     %+3d        Disarming   %s\n",
        race_ptr->stats[A_STR],
        _race_dis_skill_desc(race_ptr));

    fprintf(fp, "Intelligence %+3d        Device      %s\n",
        race_ptr->stats[A_INT],
        _race_dev_skill_desc(race_ptr));

    fprintf(fp, "Wisdom       %+3d        Save        %s\n",
        race_ptr->stats[A_WIS],
        _race_sav_skill_desc(race_ptr));

    fprintf(fp, "Dexterity    %+3d        Stealth     %s\n",
        race_ptr->stats[A_DEX],
        _race_stl_skill_desc(race_ptr));

    fprintf(fp, "Constitution %+3d        Searching   %s\n",
        race_ptr->stats[A_CON],
        _race_srh_skill_desc(race_ptr));

    fprintf(fp, "Charisma     %+3d        Perception  %s\n",
        race_ptr->stats[A_CHR],
        _race_fos_skill_desc(race_ptr));

    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        race_ptr->life,
        _race_thn_skill_desc(race_ptr));

    fprintf(fp, "Base HP      %3d        Archery     %s\n",
        race_ptr->base_hp,
        _race_thb_skill_desc(race_ptr));

    fprintf(fp, "Experience   %3d%%       Infravision %d'\n", race_ptr->exp, race_ptr->infra*10);
    fputs("</style></indent>\n", fp);
}

static void _race_help(FILE *fp, int idx)
{
    race_t *race_ptr = get_race_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_DEMIGOD:
        fputs("有关半神血统的更多详细信息，请参阅 <link:Demigods.txt>。\n\n", fp);
        break;
    case RACE_DRACONIAN:
        fputs("有关龙人的更多详细信息，请参阅 <link:Draconians.txt>。\n\n", fp);
        break;
    }

    _race_help_table(fp, race_ptr);
}

static void _races_help(FILE* fp)
{
    int i, j;

    fputs("<style:title>种族</style>\n", fp);
    fputs("There are many races in the world, each, for the most part, with both "
          "strengths and weaknesses. In general, the stronger a race is, the higher "
          "its <color:keyword>Experience Penalty</color> and the longer it will take "
          "to gain levels.\n\n"
          "For details on the <color:keyword>Stats</color>, see "
          "<link:birth.txt#PrimaryStats>. For information about the <color:keyword>Skills</color>, see "
          "<link:birth.txt#PrimarySkills>. The skill descriptions in this document are "
          "<color:v>for comparison purposes only</color>; they will tell you how good a race is compared to other races, "
          "but should not be taken as a literal indicator of the skill level your fledgling character will "
          "be born with. In general, skills are influenced by level, race, class, stats and equipment. "
          "See the race tables <link:Races.txt#Tables> below for a quick comparison "
          "of the various races.\n\n", fp);
    for (i = 0; i < B_MAX_RACE_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", b_race_groups[i].name);
        for (j = 0; ; j++)
        {
            int race_idx = b_race_groups[i].ids[j];
            if (race_idx == -1) break;
            _race_help(fp, race_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>表 1 - 种族属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < B_MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                race_ptr->name,
                race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
                race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
                race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
            );
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 种族技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < B_MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 种族技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < B_MAX_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _race_srh_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_fos_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
            fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
            fprintf(fp, " %4d'", race_ptr->infra * 10);
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

struct _name_desc_s { string_ptr name; string_ptr desc; };
typedef struct _name_desc_s _name_desc_t, *_name_desc_ptr;
static int _compare_name_desc(const _name_desc_ptr left, const _name_desc_ptr right) {
    return string_compare(left->name, right->name);
}
static void _name_desc_free(_name_desc_ptr p) {
    string_free(p->name);
    string_free(p->desc);
    free(p);
}
static _name_desc_ptr _name_desc_alloc(void) {
    _name_desc_ptr result = malloc(sizeof(_name_desc_t));
    result->name = string_alloc();
    result->desc = string_alloc();
    return result;
}

static void _demigods_help(FILE* fp)
{
    int i;

    fputs("<style:title>半神血统</style>\n\n", fp);
    fputs(get_race_aux(RACE_DEMIGOD, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        _race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 半神属性加成表</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 半神技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 半神技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < DEMIGOD_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DEMIGOD, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_srh_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_fos_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<topic:Powers><style:heading>表 4 - 半神特殊能力</style>\n\n", fp);
        fputs("All demigods have access to special powers. When they reach level 20, they may choose "
                    "a single power from the following list. When they reach level, 40, they may choose another. "
                    "These powers can never be removed or changed, so you might want to study this list to "
                    "decide which powers you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_demigod_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<topic:Weaknesses><style:heading>表 5 - 人类弱点</style>\n\n", fp);
        fputs("Normal humans receive one demigod talent, but they also acquire a special human weakness on reaching level 35. "
                    "Each weakness corresponds to one of the six basic stats; which weakness a human gets depends on "
                    "their primary spellcasting stat. Characters with no spell stat are assumed to be "
                    "warrior-like and receive the Unbalancing Strikes weakness.\n\n", fp);

        for (i = MUT_HUMAN_STR; i <= MUT_HUMAN_CHR; i++)
        {
            char buf[1024];
            _name_desc_ptr nd = _name_desc_alloc();

            mut_name(i, buf);
            string_append_s(nd->name, buf);

            mut_help_desc(i, buf);
            string_append_s(nd->desc, buf);
            vec_add(vec, nd);
        }

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:R>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n", fp);
}

static void _draconians_help(FILE* fp)
{
    int i;

    fputs("<style:title>龙人</style>\n\n", fp);
    fputs(get_race_aux(RACE_DRACONIAN, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);

        _race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 龙人属性加成表</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp  Shop</color>\n", fp);

    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);

        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 龙人技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 龙人技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < DRACONIAN_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_DRACONIAN, i);
        fprintf(fp, "%-12.12s", race_ptr->subname);
        fprintf(fp, " %s", _race_srh_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_fos_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    {
        vec_ptr vec = vec_alloc((vec_free_f)_name_desc_free);

        fputs("<topic:Powers><style:heading>表 4 - 龙人特殊能力</style>\n\n", fp);
        fputs("All draconians have access to special powers. When they reach level 35, they may choose "
                "a single power from the following list. "
                "These powers can never be removed or changed, so you might want to study this list to "
                "decide which power you will choose for your character.\n\n", fp);

        for (i = 0; i < MAX_MUTATIONS; i++)
        {
            if (mut_draconian_pred(i))
            {
                char buf[1024];
                _name_desc_ptr nd = _name_desc_alloc();

                mut_name(i, buf);
                string_append_s(nd->name, buf);

                mut_help_desc(i, buf);
                string_append_s(nd->desc, buf);
                vec_add(vec, nd);
            }
        }

        vec_sort(vec, (vec_cmp_f)_compare_name_desc);

        for (i = 0; i < vec_length(vec); i++)
        {
            _name_desc_ptr nd = vec_get(vec, i);
            /*fprintf(fp, "<color:G>%s: </color>%s\n",*/
            fprintf(fp, "  <indent><color:G>%s</color>\n%s</indent>\n\n",
                string_buffer(nd->name), string_buffer(nd->desc));
        }

        vec_free(vec);
    }
    fputs("\n\n", fp);
}

/******************************************************************************
 * Monster Mode Help
 ******************************************************************************/

static void _mon_race_help_table(FILE *fp, race_t *race_ptr)
{
    caster_info *caster_ptr = NULL;

    if (race_ptr->caster_info)
        caster_ptr = race_ptr->caster_info();

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_STR) ? 'v' : 'w',
        race_ptr->stats[A_STR],
        _mon_race_dis_skill_desc(race_ptr));
    fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
        (caster_ptr && caster_ptr->which_stat == A_INT) ? 'v' : 'w',
        race_ptr->stats[A_INT],
        _mon_race_dev_skill_desc(race_ptr));
    fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
        (caster_ptr && caster_ptr->which_stat == A_WIS) ? 'v' : 'w',
        race_ptr->stats[A_WIS],
        _mon_race_sav_skill_desc(race_ptr));
    fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
        (caster_ptr && caster_ptr->which_stat == A_DEX) ? 'v' : 'w',
        race_ptr->stats[A_DEX],
        _mon_race_stl_skill_desc(race_ptr));
    fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CON) ? 'v' : 'w',
        race_ptr->stats[A_CON],
        _mon_race_srh_skill_desc(race_ptr));
    fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CHR) ? 'v' : 'w',
        race_ptr->stats[A_CHR],
        _mon_race_fos_skill_desc(race_ptr));
    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        race_ptr->life,
        _mon_race_thn_skill_desc(race_ptr));
    fprintf(fp, "Base HP      %3d        Archery     %s\n",
        race_ptr->base_hp,
        _mon_race_thb_skill_desc(race_ptr));
    fprintf(fp, "Experience   %3d%%       Infravision %d'\n", race_ptr->exp, race_ptr->infra*10);
    fputs("</style></indent>\n", fp);
}

static void _mon_race_help(FILE *fp, int idx)
{
    race_t *race_ptr = get_race_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->name, race_ptr->name);
    fprintf(fp, "%s\n\n", race_ptr->desc);
    switch(idx)
    {
    case RACE_MON_RING:
        fputs("有关戒指的更多详细信息，请参阅 <link:rings.txt>。\n\n", fp);
        break;
    case RACE_MON_DRAGON:
        fputs("有关龙族的更多详细信息，请参阅 <link:Dragons.txt>。\n", fp);
        fputs("有关龙族领域的更多详细信息，请参阅 <link:DragonRealms.txt>。\n\n", fp);
        break;
    case RACE_MON_ORC:
        fputs("有关兽人的更多详细信息，请参阅 <link:Orcs.txt>。\n\n", fp);
        break;
    case RACE_MON_DEMON:
        fputs("有关恶魔的更多详细信息，请参阅 <link:Demons.txt>。\n\n", fp);
        return;
    }

    _mon_race_help_table(fp, race_ptr);
}


static void _monster_races_help(FILE* fp)
{
    int i, j;

    fprintf(fp, "<style:title>怪物种族</style>\n\n");
    fputs("So, you feel ready to play as a monster? There are many monster races and "
            "subraces to choose from, and the various races are loosely grouped "
            "by type below: Animals, Dragons, Demons, and so forth.\n\n"
            "As a monster, <color:v>you will not be able to choose a normal class;</color> your race will "
            "essentially be your class as well. Monsters do not use book spells, but many of them are "
            "magical in other ways; for example, fire dragons can breathe fire and "
            "may gain access to a range of non-book spells. Most monster types gain "
            "new spells, powers or abilities with experience; check out both "
            "the magic command (<color:keypress>m</color>) and the racial "
            "power command (<color:keypress>U</color>/<color:keypress>O</color>) "
            "to see what powers are available to you as you play.\n\n"
            "Most monster races have custom body types, which may "
            "severely constrain the amount and kind of equipment you may wear. "
            "For example, a <color:keyword>Beholder</color> cannot wear armor or wield a sword... that "
            "would be an odd sight indeed! But to make up for this, they can wear up to "
            "eight rings on their eyestalks. Generally, details of this kind are described below.\n\n"
            "Finally, all monsters <color:keyword>evolve</color>. When they gain enough experience, they "
            "will assume a more powerful form. To continue with our example of dragons, "
            "you might evolve from Baby to Young to Mature to Ancient and finally to a Great Wyrm, "
            "becoming vastly more powerful in the process. The stats and skills listed "
            "below assume a level 35 character.\n\n"
            "For details on the <color:keyword>Stats</color>, see "
            "<link:birth.txt#PrimaryStats>. For information about the "
            "<color:keyword>Skills</color>, see <link:birth.txt#PrimarySkills>. "
            "To compare the various races at a glance, take a "
            "look at the race tables below (<link:MonsterRaces.txt#Tables>).\n\n", fp);

    for (i = 0; i < B_MAX_MON_RACE_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", b_mon_race_groups[i].name);
        for (j = 0; ; j++)
        {
            int race_idx = b_mon_race_groups[i].ids[j];
            if (race_idx == -1) break;
            _mon_race_help(fp, race_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>表 1 - 种族属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-12.12s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < B_MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
                race_ptr->name,
                race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
                race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
                race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
            );
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 种族技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < B_MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 种族技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < B_MAX_MON_RACE_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     race_idx = b_mon_race_groups[i].ids[j];
            race_t *race_ptr;

            if (race_idx == -1) break;
            if (race_idx == RACE_MON_DEMON) continue;

            race_ptr = get_race_aux(race_idx, 0);
            fprintf(fp, "%-12.12s", race_ptr->name);
            fprintf(fp, " %s", _mon_race_srh_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_fos_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
            fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
            fprintf(fp, " %4d'", race_ptr->infra * 10);
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

static void _demons_help(FILE* fp)
{
    int i;
    fputs("<style:title>恶魔</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_DEMON, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 恶魔属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 恶魔技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 恶魔技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < DEMON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DEMON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_srh_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_fos_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _dragons_help(FILE* fp)
{
    int i;
    fputs("<style:title>龙族</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_DRAGON, 0)->desc, fp);
    fputs("\n\n", fp);
    fputs("有关<color:keyword>龙族领域</color>的更多信息，请参阅 <link:DragonRealms.txt>。\n\n", fp);

    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 龙族属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 龙族技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 龙族技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < DRAGON_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_DRAGON, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_srh_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_fos_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _dragon_realms_help(FILE* fp)
{
    int i, j;
    fputs("<style:title>龙族领域</style>\n\n", fp);
    fputs("Dragons are magical creatures and may choose to learn a particular branch of "
           "dragon magic. Dragons do not need spellbooks to cast spells or learn powers; "
           "instead, they simply gain spells as they mature. Each "
           "realm of dragon magic has a direct impact on the player's stats and skills, and "
           "each realm also requires a different stat for casting purposes.\n\n", fp);
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "<topic:%s><color:o>%s</color>\n", realm->name, realm->name);
        fputs(realm->desc, fp);
        fputs("\n\n", fp);

        fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
        fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
            (realm->spell_stat == A_STR) ? 'v' : 'w',
            realm->stats[A_STR],
            _realm_dis_skill_desc(realm));
        fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
            (realm->spell_stat == A_INT) ? 'v' : 'w',
            realm->stats[A_INT],
            _realm_dev_skill_desc(realm));
        fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
            (realm->spell_stat == A_WIS) ? 'v' : 'w',
            realm->stats[A_WIS],
            _realm_sav_skill_desc(realm));
        fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
            (realm->spell_stat == A_DEX) ? 'v' : 'w',
            realm->stats[A_DEX],
            _realm_stl_skill_desc(realm));
        fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
            (realm->spell_stat == A_CON) ? 'v' : 'w',
            realm->stats[A_CON],
            _realm_srh_skill_desc(realm));
        fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
            (realm->spell_stat == A_CHR) ? 'v' : 'w',
            realm->stats[A_CHR],
            _realm_fos_skill_desc(realm));
        fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
            realm->life,
            _realm_thn_skill_desc(realm));
        fprintf(fp, "Experience   %3d%%       Archery     %s\n",
            realm->exp,
            _realm_thb_skill_desc(realm));
        fprintf(fp, "攻击 %3d%%\n", realm->attack);
        fprintf(fp, "喷吐 %3d%%\n", realm->breath);
        fputs("</style></indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>表 1 - 龙之领域属性加成表</style>\n\n", fp);
    fprintf(fp, "<style:table><color:G>%-14.14s STR  INT  WIS  DEX  CON  CHR  Life  Exp  Attack  Breath</color>\n", "");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        char             line[255];
        char             tmp[255];

        sprintf(line, "%-14.14s", realm->name);
        for (j = 0; j < 6; j++)
        {
            if (j == realm->spell_stat)
                sprintf(tmp, "<color:v> %+3d </color>", realm->stats[j]);
            else
                sprintf(tmp, " %+3d ", realm->stats[j]);
            strcat(line, tmp);
        }
        sprintf(tmp, " %3d%%  %3d%% %5d%%  %5d%%", realm->life, realm->exp, realm->attack, realm->breath);
        strcat(line, tmp);
        fprintf(fp, "%s\n", line);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 龙之领域技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-14.14s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "%-14.14s", realm->name);
        fprintf(fp, " %s", _realm_dis_skill_desc(realm));
        fprintf(fp, " %s", _realm_dev_skill_desc(realm));
        fprintf(fp, " %s", _realm_sav_skill_desc(realm));
        fprintf(fp, " %s", _realm_stl_skill_desc(realm));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 龙之领域技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-14.14s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = 1; i < DRAGON_REALM_MAX; i++)
    {
        dragon_realm_ptr realm = dragon_get_realm(i);
        fprintf(fp, "%-14.14s", realm->name);
        fprintf(fp, " %s", _realm_srh_skill_desc(realm));
        fprintf(fp, " %s", _realm_fos_skill_desc(realm));
        fprintf(fp, " %s", _realm_thn_skill_desc(realm));
        fprintf(fp, " %s", _realm_thb_skill_desc(realm));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _orcs_help(FILE* fp)
{
    int i;
    fputs("<style:title>兽人</style>\n\n", fp);
    fputs(get_race_aux(RACE_MON_ORC, ORC_FIGHTER)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", race_ptr->subname, race_ptr->subname);
        fprintf(fp, "%s\n\n", race_ptr->subdesc);
        _mon_race_help_table(fp, race_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 兽人属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp  Shop</color>\n", "");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%% %4d%%\n",
            race_ptr->subname,
            race_ptr->stats[A_STR], race_ptr->stats[A_INT], race_ptr->stats[A_WIS],
            race_ptr->stats[A_DEX], race_ptr->stats[A_CON], race_ptr->stats[A_CHR],
            race_ptr->life, race_ptr->base_hp, race_ptr->exp, race_ptr->shop_adjust
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 兽人技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_dis_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_dev_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_sav_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_stl_skill_desc(race_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 兽人技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s %s</color>\n", "", "搜索", "察觉", "近战", "箭术", "红外");
    for (i = 0; i < ORC_MAX; i++)
    {
        race_t *race_ptr = get_race_aux(RACE_MON_ORC, i);
        fprintf(fp, "%-17.17s", race_ptr->subname);
        fprintf(fp, " %s", _mon_race_srh_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_fos_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thn_skill_desc(race_ptr));
        fprintf(fp, " %s", _mon_race_thb_skill_desc(race_ptr));
        fprintf(fp, " %4d'", race_ptr->infra * 10);
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Class Help
 ******************************************************************************/
static void _class_help_table(FILE *fp, class_t *class_ptr)
{
    caster_info *caster_ptr = NULL;

    if (class_ptr->caster_info && class_ptr->id != CLASS_PSION && class_ptr->id != CLASS_WILD_TALENT)
        caster_ptr = class_ptr->caster_info();

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     <color:%c>%+3d</color>        Disarming   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_STR) ? 'v' : 'w',
        class_ptr->stats[A_STR],
        _class_dis_skill_desc(class_ptr));
    fprintf(fp, "Intelligence <color:%c>%+3d</color>        Device      %s\n",
        (caster_ptr && caster_ptr->which_stat == A_INT) ? 'v' : 'w',
        class_ptr->stats[A_INT],
        _class_dev_skill_desc(class_ptr));
    fprintf(fp, "Wisdom       <color:%c>%+3d</color>        Save        %s\n",
        (caster_ptr && caster_ptr->which_stat == A_WIS) ? 'v' : 'w',
        class_ptr->stats[A_WIS],
        _class_sav_skill_desc(class_ptr));
    fprintf(fp, "Dexterity    <color:%c>%+3d</color>        Stealth     %s\n",
        (caster_ptr && caster_ptr->which_stat == A_DEX) ? 'v' : 'w',
        class_ptr->stats[A_DEX],
        _class_stl_skill_desc(class_ptr));
    fprintf(fp, "Constitution <color:%c>%+3d</color>        Searching   %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CON) ? 'v' : 'w',
        class_ptr->stats[A_CON],
        _class_srh_skill_desc(class_ptr));
    fprintf(fp, "Charisma     <color:%c>%+3d</color>        Perception  %s\n",
        (caster_ptr && caster_ptr->which_stat == A_CHR) ? 'v' : 'w',
        class_ptr->stats[A_CHR],
        _class_fos_skill_desc(class_ptr));
    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        class_ptr->life,
        _class_thn_skill_desc(class_ptr));
    fprintf(fp, "Base HP      %3d        Archery     %s\n",
        class_ptr->base_hp,
        _class_thb_skill_desc(class_ptr));
    fprintf(fp, "Experience   %3d%%\n", class_ptr->exp);
    fputs("</style></indent>\n", fp);
}

static void _class_help(FILE *fp, int idx)
{
    class_t *class_ptr = get_class_aux(idx, 0);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->name, class_ptr->name);
    if (idx == CLASS_ETHEREAL_MIMIC)
    {
        fputs("缥缈模仿者是能够学习怪物形态的战士。他们在未变身时使用普通人形身体和装备栏位；一旦变身，就会继承目标形态的身体结构、抗性、法术、吐息和天然攻击，从而用敌人的形体反过来适应地下城。\n\n", fp);
        fputs("缥缈模仿者通过击杀非唯一怪物来理解其形体；当击杀数量达到该怪物等级后，就能永久学会该形态。使用职业能力时可以输入怪物序号来变身，怪物序号可在知识菜单的怪物知识中查询。由于可学习形态数量可能很多，这种方式也便于玩家为常用形态编写稳定的宏。唯一怪物不能被学习或模仿。", fp);
    }
    else
    {
        fputs(class_ptr->desc, fp);
    }
    fputs("\n\n", fp);

    switch(idx)
    {
    case CLASS_WARLOCK:
        fputs("有关邪术师契约的更多详细信息，请参阅 <link:Warlocks.txt>。\n\n", fp);
        break;
    case CLASS_WEAPONMASTER:
        fputs("有关武器大师的更多详细信息，请参阅 <link:Weaponmasters.txt>。\n\n", fp);
        break;
    case CLASS_SKILLMASTER:
        fputs("有关技能大师的更多详细信息，请参阅 <link:Skillmasters.txt>。\n\n", fp);
        break;
    case CLASS_DISCIPLE:
        fputs("有关门徒的更多详细信息，请参阅 <link:Disciples.txt>。\n\n", fp);
        break;
    case CLASS_RUNE_KNIGHT:
        fputs("有关符文骑士的更多详细信息，请参阅 <link:Runeknights.txt>。\n\n", fp);
        break;
    }

    _class_help_table(fp, class_ptr);
}

#define _MAX_CLASSES_PER_GROUP 20
#define _MAX_CLASS_GROUPS      11
typedef struct _class_group_s {
    cptr name;
    int ids[_MAX_CLASSES_PER_GROUP];
} _class_group_t;
static _class_group_t _class_groups[_MAX_CLASS_GROUPS] = {
    { "近战", {CLASS_BERSERKER, CLASS_BLOOD_KNIGHT, CLASS_DUELIST, CLASS_MAULER,
                    CLASS_RUNE_KNIGHT, CLASS_SAMURAI, CLASS_WARRIOR, CLASS_WEAPONMASTER,
                    CLASS_WEAPONSMITH, -1} },
    { "箭术", {CLASS_ARCHER, CLASS_SNIPER, -1} },
    { "武术", {CLASS_FORCETRAINER, CLASS_MONK, CLASS_MYSTIC, -1} },
    { "魔法", {CLASS_BLOOD_MAGE, CLASS_BLUE_MAGE, CLASS_GRAY_MAGE, CLASS_HIGH_MAGE, CLASS_MAGE,
                    CLASS_NECROMANCER, CLASS_SORCERER, CLASS_YELLOW_MAGE, -1} },
    { "装置", {CLASS_ALCHEMIST, CLASS_DEVICEMASTER, CLASS_MAGIC_EATER, -1} },
    { "祈祷", {CLASS_PRIEST, -1} },
    { "潜行", {CLASS_NINJA, CLASS_ROGUE, CLASS_SCOUT, -1} },
    { "混合", {CLASS_CHAOS_WARRIOR, CLASS_DISCIPLE, CLASS_NINJA_LAWYER, CLASS_PALADIN,
                    CLASS_RANGER, CLASS_RED_MAGE, CLASS_WARRIOR_MAGE, CLASS_ETHEREAL_MIMIC, -1} },
    { "骑乘", {CLASS_BEASTMASTER, CLASS_CAVALRY, -1} },
    { "心灵", {CLASS_MINDCRAFTER, CLASS_MIRROR_MASTER, CLASS_PSION,
                    CLASS_TIME_LORD, CLASS_WARLOCK, -1} },
    { "其他", {CLASS_ARCHAEOLOGIST, CLASS_BARD, CLASS_LAWYER, CLASS_POLITICIAN,
                    CLASS_RAGE_MAGE, CLASS_SKILLMASTER, CLASS_TOURIST, CLASS_WILD_TALENT, -1} },
};

static void _classes_help(FILE* fp)
{
    int i, j, k;

    fputs("<style:title>职业</style>\n\n", fp);
    fputs("没有什么决定比选择玩哪个职业更重要了。在下面，众多可用的职业大致按其主要优势或游戏风格进行了分类：近战、箭术、武术、魔法、装置、混合等。每个职业的主要施法属性都已<color:v>高亮</color>显示。\n\n有关<color:keyword>属性</color>的详细信息，请参阅 <link:birth.txt#PrimaryStats>。有关<color:keyword>技能</color>的信息，请参阅 <link:birth.txt#PrimarySkills>。本文档中的技能描述<color:v>仅供比较之用</color>；例如，你初出茅庐的狂战士并不会真正天生就带有<color:v>琥珀色[25]</color>的近战技能。通常来说，技能受等级、种族、职业、属性和装备的影响。请查看此文件底部的表格（<link:Classes.txt#Tables>）以一目了然地比较各种职业的属性和技能加成。\n\n", fp);

    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        fprintf(fp, "<style:heading>%s</style>\n  <indent>", _class_groups[i].name);
        for (j = 0; ; j++)
        {
            int class_idx = _class_groups[i].ids[j];
            if (class_idx == -1) break;
            _class_help(fp, class_idx);
        }
        fputs("</indent>\n", fp);
    }

    fputs("<topic:Tables><style:heading>表 1 - 职业属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int          class_idx = _class_groups[i].ids[j];
            class_t     *class_ptr;
            caster_info *caster_ptr = NULL;
            char         line[255];
            char         tmp[255];

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            if (class_ptr->caster_info)
                caster_ptr = class_ptr->caster_info();

            sprintf(line, "%-13.13s", class_ptr->name);
            for (k = 0; k < 6; k++)
            {
                if (caster_ptr && k == caster_ptr->which_stat && class_idx != CLASS_PSION && class_idx != CLASS_WILD_TALENT)
                    sprintf(tmp, "<color:v> %+3d </color>", class_ptr->stats[k]);
                else
                    sprintf(tmp, " %+3d ", class_ptr->stats[k]);
                strcat(line, tmp);
            }
            sprintf(tmp, " %3d%%  %+3d  %3d%%", class_ptr->life, class_ptr->base_hp, class_ptr->exp);
            strcat(line, tmp);
            fprintf(fp, "%s\n", line);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 职业技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     class_idx = _class_groups[i].ids[j];
            class_t *class_ptr;

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            fprintf(fp, "%-13.13s", class_ptr->name);
            fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 职业技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-13.13s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = 0; i < _MAX_CLASS_GROUPS; i++)
    {
        for (j = 0; ; j++)
        {
            int     class_idx = _class_groups[i].ids[j];
            class_t *class_ptr;

            if (class_idx == -1) break;
            class_ptr = get_class_aux(class_idx, 0);
            fprintf(fp, "%-13.13s", class_ptr->name);
            fprintf(fp, " %s", _class_srh_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_fos_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
            fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
            fputc('\n', fp);
        }
    }
    fputs("\n</style>\n", fp);
}

static void _disciples_help(FILE *fp)
{
    int i;
    fputs("<style:title>门徒</style>\n\n", fp);
    fputs(get_class_aux(CLASS_DISCIPLE, 0)->desc, fp);
    fputs("\n\n", fp);

    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fprintf(fp, "<color:B>TIP</color>: The special random quests for Karrot and Troika disciples can only appear ");
    fprintf(fp, "如果你所在的层数足够深，使得该任务具有挑战性。按下 <color:y>CTRL+F</color>");
    fprintf(fp, "reveals your patron's opinion of your current depth. The Purples also like you to visit ");
    fprintf(fp, "many different dungeons; do not expect a large number of assignments in a single dungeon, unless ");
    fprintf(fp, "你在没有荒野的模式下游玩。\n\n");

    fputs("<topic:Tables><style:heading>表 1 - 门徒属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 门徒技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 门徒技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = MIN_PURPLE_PATRON; i < MAX_PURPLE_PATRON; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_DISCIPLE, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_srh_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_fos_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
    yeqrezh_help(fp);
    fputs("\n</style>\n", fp);
}

static void _weaponmasters_help(FILE *fp)
{
    int i;
    fputs("<style:title>武器大师</style>\n\n", fp);
    fputs(get_class_aux(CLASS_WEAPONMASTER, WEAPONMASTER_AXES)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 武器大师属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 武器大师技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 武器大师技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = 0; i < WEAPONMASTER_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WEAPONMASTER, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_srh_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_fos_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

static void _warlocks_help(FILE *fp)
{
    int i;
    fputs("<style:title>邪术师</style>\n\n", fp);
    fputs(get_class_aux(CLASS_WARLOCK, WARLOCK_UNDEAD)->desc, fp);
    fputs("\n\n", fp);

    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);

        fprintf(fp, "<topic:%s><color:o>%s</color>\n", class_ptr->subname, class_ptr->subname);
        fprintf(fp, "%s\n\n", class_ptr->subdesc);
        _class_help_table(fp, class_ptr);
    }

    fputs("<topic:Tables><style:heading>表 1 - 邪术师属性加成表</style>\n<style:table>\n", fp);
    fprintf(fp, "<color:G>%-17.17s</color> <color:G>STR  INT  WIS  DEX  CON  CHR  Life  BHP  Exp</color>\n", "");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %+3d  %3d%%\n",
            class_ptr->subname,
            class_ptr->stats[A_STR], class_ptr->stats[A_INT], class_ptr->stats[A_WIS],
            class_ptr->stats[A_DEX], class_ptr->stats[A_CON], class_ptr->stats[A_CHR],
            class_ptr->life, class_ptr->base_hp, class_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 邪术师技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_dis_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_dev_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_sav_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_stl_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 邪术师技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-17.17s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = 0; i < WARLOCK_MAX; i++)
    {
        class_t *class_ptr = get_class_aux(CLASS_WARLOCK, i);
        fprintf(fp, "%-17.17s", class_ptr->subname);
        fprintf(fp, " %s", _class_srh_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_fos_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thn_skill_desc(class_ptr));
        fprintf(fp, " %s", _class_thb_skill_desc(class_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Personality Help
 ******************************************************************************/
static void _personality_help(FILE *fp, int idx)
{
    personality_ptr pers_ptr = get_personality_aux(idx);

    fprintf(fp, "<topic:%s><color:o>%s</color>\n", pers_ptr->name, pers_ptr->name);
    fprintf(fp, "%s\n\n", pers_ptr->desc);

    fputs("  <indent><style:table><color:G>Stats                   Skills</color>\n", fp);
    fprintf(fp, "Strength     %+3d        Disarming   %s\n",
        pers_ptr->stats[A_STR],
        _pers_dis_skill_desc(pers_ptr));

    fprintf(fp, "Intelligence %+3d        Device      %s\n",
        pers_ptr->stats[A_INT],
        _pers_dev_skill_desc(pers_ptr));

    fprintf(fp, "Wisdom       %+3d        Save        %s\n",
        pers_ptr->stats[A_WIS],
        _pers_sav_skill_desc(pers_ptr));

    fprintf(fp, "Dexterity    %+3d        Stealth     %s\n",
        pers_ptr->stats[A_DEX],
        _pers_stl_skill_desc(pers_ptr));

    fprintf(fp, "Constitution %+3d        Searching   %s\n",
        pers_ptr->stats[A_CON],
        _pers_srh_skill_desc(pers_ptr));

    fprintf(fp, "Charisma     %+3d        Perception  %s\n",
        pers_ptr->stats[A_CHR],
        _pers_fos_skill_desc(pers_ptr));

    fprintf(fp, "Life Rating  %3d%%       Melee       %s\n",
        pers_ptr->life,
        _pers_thn_skill_desc(pers_ptr));

    fprintf(fp, "Experience   %3d%%       Archery     %s\n",
        pers_ptr->exp,
        _pers_thb_skill_desc(pers_ptr));
    fputs("</style></indent>\n", fp);
}

static void _personalities_help(FILE* fp)
{
    int i;

    fprintf(fp, "<style:title>性格</style>\n\n");
    fputs("Your personality is the way you see and act in the world, and has "
            "a small but significant effect on your stats and skills. In general, "
            "you should pick a personality that complements the strengths and "
            "weaknesses inherent in your race and class. Most personalities only make "
            "a minor difference, but a few can change the game dramatically.\n\n"
            "For details on the <color:keyword>Stats</color>, see "
            "<link:birth.txt#PrimaryStats>. For information about the <color:keyword>Skills</color>, see "
            "<link:birth.txt#PrimarySkills>. "
            "The skill descriptions in this document are <color:v>for comparison purposes only</color>; "
            "for example, your fledgling munchkin will not be born with <color:v>Amber[25]</color> "
            "melee skill. In general, skills are also influenced by level, race, class, stats and equipment. "
            "To compare the stat and skill boosts of each personality, take a look "
            "at the <link:Personalities.txt#Tables> personality tables below.\n\n", fp);
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        _personality_help(fp, i);
    }

    fputs("<topic:Tables><style:heading>表 1 - 性格属性加成表</style>\n\n", fp);
    fputs("<style:table><color:G>               STR  INT  WIS  DEX  CON  CHR  Life  Exp</color>\n", fp);

    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);

        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-14s %+3d  %+3d  %+3d  %+3d  %+3d  %+3d  %3d%%  %3d%%\n",
            pers_ptr->name,
            pers_ptr->stats[0], pers_ptr->stats[1], pers_ptr->stats[2],
            pers_ptr->stats[3], pers_ptr->stats[4], pers_ptr->stats[5],
            pers_ptr->life, pers_ptr->exp
        );
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills1><style:heading>表 2 - 性格技能加成表 I</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "解除陷阱", "使用装置", "豁免", "潜行");
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-12.12s", pers_ptr->name);
        fprintf(fp, " %s", _pers_dis_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_dev_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_sav_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_stl_skill_desc(pers_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);

    fputs("<topic:Skills2><style:heading>表 3 - 性格技能加成表 II</style>\n<style:table>\n", fp);
    fprintf(fp, "%-12.12s <color:w>%-13.13s %-13.13s %-13.13s %-13.13s</color>\n", "", "搜索", "察觉", "近战", "箭术");
    for (i = 0; i < MAX_PERSONALITIES; i++)
    {
        personality_ptr pers_ptr = get_personality_aux(i);
        if (pers_ptr->flags & DEPRECATED) continue;
        fprintf(fp, "%-12.12s", pers_ptr->name);
        fprintf(fp, " %s", _pers_srh_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_fos_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_thn_skill_desc(pers_ptr));
        fprintf(fp, " %s", _pers_thb_skill_desc(pers_ptr));
        fputc('\n', fp);
    }
    fputs("\n</style>\n", fp);
}

/******************************************************************************
 * Spoilers: All the various possessor body types
 ******************************************************************************/
static void _possessor_stats_table(FILE* fp)
{
    int i;
    fprintf(fp, "Name,Idx,Lvl,Speed,AC,Attacks,Dam,Body,Str,Int,Wis,Dex,Con,Chr,Life,Disarm,Device,Save,Stealth,Search,Perception,Melee,Bows\n");
    for (i = 0; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /*XXX if (r_ptr->flags9 & RF9_DROP_CORPSE)*/
        {
            int ac = 0, dam = 0, attacks = 0/*, j*/;

            if (r_ptr->flags9 & RF9_POS_GAIN_AC)
                ac = r_ptr->ac;

            #if 0
            for (j = 0; j < 4; j++)
            {
                if (!r_ptr->blow[j].effect) continue;
                if (r_ptr->blow[j].method == RBM_EXPLODE) continue;

                dam += r_ptr->blow[j].d_dice * (r_ptr->blow[j].d_side + 1) / 2;
                attacks++;
            }
            #endif

            fprintf(fp, "\"%s\",%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,=\"%d+%d\",=\"%d+%d\",=\"%d+%d\",%d,%d,%d,=\"%d+%d\",=\"%d+%d\"\n",
                i == MON_ECHIZEN ? "战斗越前" : monster_race_display_name(i), i, r_ptr->level,
                r_ptr->speed - 110, ac, attacks, dam,
                body_display_name(r_ptr->body.body_idx),
                r_ptr->body.stats[A_STR], r_ptr->body.stats[A_INT], r_ptr->body.stats[A_WIS],
                r_ptr->body.stats[A_DEX], r_ptr->body.stats[A_CON], r_ptr->body.stats[A_CHR],
                r_ptr->body.life,
                r_ptr->body.skills.dis, r_ptr->body.extra_skills.dis,
                r_ptr->body.skills.dev, r_ptr->body.extra_skills.dev,
                r_ptr->body.skills.sav, r_ptr->body.extra_skills.sav,
                r_ptr->body.skills.stl,
                r_ptr->body.skills.srh,
                r_ptr->body.skills.fos,
                r_ptr->body.skills.thn, r_ptr->body.extra_skills.thn,
                r_ptr->body.skills.thb, r_ptr->body.extra_skills.thb
            );
        }
    }
}

/******************************************************************************
 * Spoilers: Skill Tables with Actual Numbers for Design Purposes
 ******************************************************************************/
static void _skills_race_table(FILE* fp)
{
    int i,j;
    fputs("种族,解除,装置,豁免,潜行,搜索,察觉,近战,箭术,属性,经验\n", fp);
    for (i = 0; i < MAX_RACES; i++)
    {
        int max_j = 1;
        if (i == RACE_DEMIGOD)
            max_j = DEMIGOD_MAX;
        else if (i == RACE_DRACONIAN)
            max_j = DRACONIAN_MAX;

        for (j = 0; j < max_j; j++)
        {
            race_t *race_ptr = get_race_aux(i, j);
            int     stats = 0, k;

            if (race_ptr->flags & RACE_IS_MONSTER) continue;

            for (k = 0; k < MAX_STATS; k++)
                stats += race_ptr->stats[k];

            if (race_ptr->subname && strlen(race_ptr->subname))
                fprintf(fp, "\"%s:%s\",", race_ptr->name, race_ptr->subname);
            else
                fprintf(fp, "\"%s\",", race_ptr->name);
            fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                race_ptr->skills.dis,
                race_ptr->skills.dev,
                race_ptr->skills.sav,
                race_ptr->skills.stl,
                race_ptr->skills.srh,
                race_ptr->skills.fos,
                race_ptr->skills.thn,
                race_ptr->skills.thb,
                stats,
                race_ptr->exp
            );
        }
    }
}

static void _skills_class_table(FILE* fp)
{
    int i,j;
    fputs("职业,解除,装置,豁免,潜行,搜索,察觉,近战,箭术,解除2,装置2,豁免2,近战2,箭术2,生命,基础血量,骑乘,双持\n", fp);
    for (i = 0; i < MAX_CLASS; i++)
    {
        int max_j = 1;

        if (i == CLASS_MONSTER || class_is_deprecated(i))
            continue;
        else if (i == CLASS_WEAPONMASTER)
            max_j = WEAPONMASTER_MAX;
        else if (i == CLASS_WARLOCK)
            max_j = WARLOCK_MAX;

        for (j = 0; j < max_j; j++)
        {
            class_t *class_ptr = get_class_aux(i, j);

            if (class_ptr->subname && strlen(class_ptr->subname))
                fprintf(fp, "\"%s:%s\",", class_ptr->name, class_ptr->subname);
            else
                fprintf(fp, "\"%s\",", class_ptr->name);
            fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d+%d,%d+%d,%d+%d,%d+%d,%d+%d,%d,%d,%d,%d\n",
                class_ptr->base_skills.dis + 5*class_ptr->extra_skills.dis,
                class_ptr->base_skills.dev + 5*class_ptr->extra_skills.dev,
                class_ptr->base_skills.sav + 5*class_ptr->extra_skills.sav,
                class_ptr->base_skills.stl + 5*class_ptr->extra_skills.stl,
                class_ptr->base_skills.srh,
                class_ptr->base_skills.fos,
                class_ptr->base_skills.thn + 5*class_ptr->extra_skills.thn,
                class_ptr->base_skills.thb + 5*class_ptr->extra_skills.thb,
                class_ptr->base_skills.dis, class_ptr->extra_skills.dis,
                class_ptr->base_skills.dev, class_ptr->extra_skills.dev,
                class_ptr->base_skills.sav, class_ptr->extra_skills.sav,
                class_ptr->base_skills.thn, class_ptr->extra_skills.thn,
                class_ptr->base_skills.thb, class_ptr->extra_skills.thb,
                class_ptr->life, class_ptr->base_hp,
                s_info[i].s_max[SKILL_RIDING],
                s_info[i].s_max[SKILL_DUAL_WIELDING]
            );
        }
    }
}

static void _spells_table(FILE* fp) /*m_info.txt*/
{
    int class_idx, realm_idx, spell_idx;
    fputs("职业,领域,索引,名称,等级,消耗,失败率\n", fp);
    for (class_idx = 0; class_idx < MAX_CLASS; class_idx++)
    {
        class_t      *class_ptr;
        player_magic *magic_ptr;
        if (class_is_deprecated(class_idx)) continue;
        class_ptr = get_class_aux(class_idx, 0);
        magic_ptr = &m_info[class_idx];
        for (realm_idx = REALM_LIFE; realm_idx <= MAX_MAGIC; realm_idx++)
        {
            for (spell_idx = 0; spell_idx < 32; spell_idx++)
            {
                magic_type *spell_ptr = &magic_ptr->info[realm_idx-1][spell_idx];
                if (0 < spell_ptr->slevel && spell_ptr->slevel <= PY_MAX_LEVEL)
                {
                    fprintf(fp, "\"%s\",\"%s\",%d,\"%s\",%d,%d,%d\n",
                        class_ptr->name,
                        realm_names[realm_idx],
                        spell_idx+1,
                        do_spell(realm_idx, spell_idx, SPELL_NAME),
                        spell_ptr->slevel,
                        spell_ptr->smana,
                        spell_ptr->sfail
                    );
                }
            }
        }
    }
}

/******************************************************************************
 * Auto-generate HTML and TEXT files from the help system
 ******************************************************************************/
typedef struct {
    string_ptr dir;
    string_ptr base;
    string_ptr ext;
} _file_parts_t, *_file_parts_ptr;
static _file_parts_ptr _file_parts_alloc(void)
{
    _file_parts_ptr result = malloc(sizeof(_file_parts_t));
    result->dir = string_alloc();
    result->base = string_alloc();
    result->ext = string_alloc();
    return result;
}
static void _file_parts_free(_file_parts_ptr fp)
{
    if (fp)
    {
        string_free(fp->dir);
        string_free(fp->base);
        string_free(fp->ext);
        free(fp);
    }
}
static string_ptr _file_parts_build_fullname(_file_parts_ptr fp)
{
    string_ptr result = string_copy(fp->dir);

    string_append_s(result, PATH_SEP);
    string_append(result, fp->base);
    if (string_length(fp->ext))
    {
        string_append_c(result, '.');
        string_append(result, fp->ext);
    }
    return result;
}
static void _file_parts_change_name(_file_parts_ptr fp, cptr filename)
{
    string_ptr s = string_copy_s(filename);
    int        pos = string_last_chr(s, '.');

    string_clear(fp->base);
    string_clear(fp->ext);

    if (pos >= 0)
    {
        string_append_sn(fp->base, string_buffer(s), pos);
        string_append_s(fp->ext, string_buffer(s) + pos + 1);
    }
    else
    {
        string_append(fp->base, s);
    }
    string_free(s);
}
static void _file_parts_change_extension(_file_parts_ptr fp, cptr ext)
{
    string_clear(fp->ext);
    string_append_s(fp->ext, ext);
}
static void _file_parts_extend_path(_file_parts_ptr fp, cptr dirname)
{
    char buf[1024];
    path_build(buf, sizeof(buf), string_buffer(fp->dir), dirname);
    string_clear(fp->dir);
    string_append_s(fp->dir, buf);
}

static void _generate_html_help_aux(cptr name, str_map_ptr prev, int format)
{
    if (!str_map_contains(prev, name))
    {
        int              i;
        doc_ptr          doc;
        FILE            *fff;
        _file_parts_ptr  dfp;
        string_ptr       dest_path;
        char             src_path[1024];
        vec_ptr          links;

        /* Read Source Document*/
        path_build(src_path, sizeof(src_path), ANGBAND_DIR_HELP, name);
        str_map_add(prev, name, 0); /* optimism */

        fff = my_fopen(src_path, "r");
        if (!fff)
            return;

        doc = doc_alloc(80);
        doc_read_file(doc, fff);
        my_fclose(fff);

        /* Output Dest Document */
        dfp = _file_parts_alloc();
        string_append_s(dfp->dir, ANGBAND_DIR_HELP);
        if (format == DOC_FORMAT_HTML)
        {
            _file_parts_extend_path(dfp, "html");
            _file_parts_change_name(dfp, name);
            _file_parts_change_extension(dfp, "html");
        }
        else
        {
            _file_parts_extend_path(dfp, "text");
            _file_parts_change_name(dfp, name);
            _file_parts_change_extension(dfp, "txt");
        }
        dest_path = _file_parts_build_fullname(dfp);

        fff = my_fopen(string_buffer(dest_path), "w");
        if (fff)
        {
            doc_write_file(doc, fff, format);
            my_fclose(fff);
        }
        _file_parts_free(dfp);
        string_free(dest_path);

        /* Recurse On Links */
        links = doc_get_links(doc);
        for (i = 0; i < vec_length(links); i++)
        {
            doc_link_ptr link = vec_get(links, i);
            _generate_html_help_aux(string_buffer(link->file), prev, format);
        }
        vec_free(links);

        doc_free(doc);
    }
}

static void _generate_html_help(void)
{
    str_map_ptr prev = str_map_alloc(NULL);
    _generate_html_help_aux("start.txt", prev, DOC_FORMAT_HTML);
    str_map_free(prev);
}

static void _generate_text_help(void)
{
    str_map_ptr prev = str_map_alloc(NULL);
    _generate_html_help_aux("start.txt", prev, DOC_FORMAT_TEXT);
    str_map_free(prev);
}

void generate_spoilers(void)
{
    int taso = p_ptr->lev;
    p_ptr->lev = 35; /* stats may vary based on the player's level, we use 35 to at least be consistent */
    spoiler_hack = TRUE;

    _help_file("Races.txt", _races_help);
    _help_file("Demigods.txt", _demigods_help);
    _help_file("Draconians.txt", _draconians_help);

    _help_file("Classes.txt", _classes_help);
    _help_file("Weaponmasters.txt", _weaponmasters_help);
    _help_file("Warlocks.txt", _warlocks_help);
    _help_file("Disciples.txt", _disciples_help);

    _help_file("Personalities.txt", _personalities_help);

    _help_file("MonsterRaces.txt", _monster_races_help);
    _help_file("Demons.txt", _demons_help);
    _help_file("Dragons.txt", _dragons_help);
    _help_file("DragonRealms.txt", _dragon_realms_help);
    _help_file("Orcs.txt", _orcs_help);

    _csv_file("PossessorStats.csv", _possessor_stats_table);
    _csv_file("Skills-Racial.csv", _skills_race_table);
    _csv_file("Skills-Class.csv", _skills_class_table);
    /*_csv_file("Skills-Monster.csv", _skills_mon_table);*/
    _csv_file("Spells.csv", _spells_table);

    _generate_html_help();
    _generate_text_help();
    spoiler_hack = FALSE;
    p_ptr->lev = taso;
}

#endif
