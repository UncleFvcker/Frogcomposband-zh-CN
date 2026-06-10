/* 文件: cmd4.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Interface commands */

#include "angband.h"
#include "equip.h"
#include "int-map.h"
#include "z-doc.h"

#include <assert.h>

static cptr _stat_label_zh(int stat)
{
    switch (stat)
    {
    case A_STR: return "力量";
    case A_INT: return "智力";
    case A_WIS: return "感知";
    case A_DEX: return "敏捷";
    case A_CON: return "体质";
    case A_CHR: return "魅力";
    }
    return "";
}

static void browser_cursor(char ch, int *column, int *grp_cur, int grp_cnt, int *list_cur, int list_cnt);

/*
 * A set of functions to maintain automatic dumps of various kinds.
 * -Mogami-
 *
 * remove_auto_dump(orig_file, mark)
 *     Remove the old automatic dump of type "mark".
 * auto_dump_printf(fmt, ...)
 *     Dump a formatted string using fprintf().
 * open_auto_dump(buf, mark)
 *     Open a file, remove old dump, and add new header.
 * close_auto_dump(void)
 *     Add a footer, and close the file.
 *
 *    The dump commands of original Angband simply add new lines to
 * existing files; these files will become bigger and bigger unless
 * an user deletes some or all of these files by hand at some
 * point.
 *
 *     These three functions automatically delete old dumped lines
 * before adding new ones. Since there are various kinds of automatic
 * dumps in a single file, we add a header and a footer with a type
 * name for every automatic dump, and kill old lines only when the
 * lines have the correct type of header and footer.
 *
 *     We need to be quite paranoid about correctness; the user might
 * (mistakenly) edit the file by hand, and see all their work come
 * to nothing on the next auto dump otherwise. The current code only
 * detects changes by noting inconsistencies between the actual number
 * of lines and the number written in the footer. Note that this will
 * not catch single-line edits.
 */

/*
 *  Mark strings for auto dump
 */
static char auto_dump_header[] = "# vvvvvvv== %s ==vvvvvvv";
static char auto_dump_footer[] = "# ^^^^^^^== %s ==^^^^^^^";

/*
 * Variables for auto dump
 */
static FILE *auto_dump_stream;
static cptr auto_dump_mark;
static int auto_dump_line_num;

/*
 * Remove old lines automatically generated before.
 */
static void remove_auto_dump(cptr orig_file)
{
    FILE *tmp_fff, *orig_fff;

    char tmp_file[1024];
    char buf[1024];
    bool between_mark = FALSE;
    bool changed = FALSE;
    int line_num = 0;
    long header_location = 0;
    char header_mark_str[80];
    char footer_mark_str[80];
    size_t mark_len;

    /* Prepare a header/footer mark string */
    sprintf(header_mark_str, auto_dump_header, auto_dump_mark);
    sprintf(footer_mark_str, auto_dump_footer, auto_dump_mark);

    mark_len = strlen(footer_mark_str);

    /* Open an old dump file in read-only mode */
    orig_fff = my_fopen(orig_file, "r");

    /* If original file does not exist, nothing to do */
    if (!orig_fff) return;

    /* Open a new (temporary) file */
    tmp_fff = my_fopen_temp(tmp_file, 1024);

    if (!tmp_fff)
    {
        msg_format("无法创建临时文件 %s。", tmp_file);
        msg_print(NULL);
        return;
    }

    /* Loop for every line */
    while (TRUE)
    {
        /* Read a line */
        if (my_fgets(orig_fff, buf, sizeof(buf)))
        {
            /* Read error: Assume End of File */

            /*
             * Was looking for the footer, but not found.
             *
             * Since automatic dump might be edited by hand,
             * it's dangerous to kill these lines.
             * Seek back to the next line of the (pseudo) header,
             * and read again.
             */
            if (between_mark)
            {
                fseek(orig_fff, header_location, SEEK_SET);
                between_mark = FALSE;
                continue;
            }

            /* Success -- End the loop */
            else
            {
                break;
            }
        }

        /* We are looking for the header mark of automatic dump */
        if (!between_mark)
        {
            /* Is this line a header? */
            if (!strcmp(buf, header_mark_str))
            {
                /* Memorise seek point of this line */
                header_location = ftell(orig_fff);

                /* Initialize counter for number of lines */
                line_num = 0;

                /* Look for the footer from now */
                between_mark = TRUE;

                /* There are some changes */
                changed = TRUE;
            }

            /* Not a header */
            else
            {
                /* Copy orginally lines */
                fprintf(tmp_fff, "%s\n", buf);
            }
        }

        /* We are looking for the footer mark of automatic dump */
        else
        {
            /* Is this line a footer? */
            if (!strncmp(buf, footer_mark_str, mark_len))
            {
                int tmp;

                /*
                 * Compare the number of lines
                 *
                 * If there is an inconsistency between
                 * actual number of lines and the
                 * number here, the automatic dump
                 * might be edited by hand. So it's
                 * dangerous to kill these lines.
                 * Seek back to the next line of the
                 * (pseudo) header, and read again.
                 */
                if (!sscanf(buf + mark_len, " (%d)", &tmp)
                    || tmp != line_num)
                {
                    fseek(orig_fff, header_location, SEEK_SET);
                }

                /* Look for another header */
                between_mark = FALSE;
            }

            /* Not a footer */
            else
            {
                /* Ignore old line, and count number of lines */
                line_num++;
            }
        }
    }

    /* Close files */
    my_fclose(orig_fff);
    my_fclose(tmp_fff);

    /* If there are some changes, overwrite the original file with new one */
    if (changed)
    {
        /* Copy contents of temporary file */

        tmp_fff = my_fopen(tmp_file, "r");
        orig_fff = my_fopen(orig_file, "w");

        while (!my_fgets(tmp_fff, buf, sizeof(buf)))
            fprintf(orig_fff, "%s\n", buf);

        my_fclose(orig_fff);
        my_fclose(tmp_fff);
    }

    /* Kill the temporary file */
    fd_kill(tmp_file);

    return;
}


/*
 * Dump a formatted line, using "vstrnfmt()".
 */
static void auto_dump_printf(cptr fmt, ...)
{
    cptr p;
    va_list vp;

    char buf[1024];

    /* Begin the Varargs Stuff */
    va_start(vp, fmt);

    /* Format the args, save the length */
    (void)vstrnfmt(buf, sizeof(buf), fmt, vp);

    /* End the Varargs Stuff */
    va_end(vp);

    /* Count number of lines */
    for (p = buf; *p; p++)
    {
        if (*p == '\n') auto_dump_line_num++;
    }

    /* Dump it */
    fprintf(auto_dump_stream, "%s", buf);
}


/*
 *  Open file to append auto dump.
 */
static bool open_auto_dump(cptr buf, cptr mark)
{

    char header_mark_str[80];

    /* Save the mark string */
    auto_dump_mark = mark;

    /* Prepare a header mark string */
    sprintf(header_mark_str, auto_dump_header, auto_dump_mark);

    /* Remove old macro dumps */
    remove_auto_dump(buf);

    /* Append to the file */
    auto_dump_stream = my_fopen(buf, "a");

    /* Failure */
    if (!auto_dump_stream) {
        msg_format("无法打开 %s。", buf);
        msg_print(NULL);

        /* Failed */
        return FALSE;
    }

    /* Start dumping */
    fprintf(auto_dump_stream, "%s\n", header_mark_str);

    /* Initialize counter */
    auto_dump_line_num = 0;

    auto_dump_printf("# *警告!* 下面的行是自动转储的。\n");
    auto_dump_printf("# 请勿编辑这些内容；手动修改会被自动删除和替换。\n");

    /* Success */
    return TRUE;
}

/*
 *  Append foot part and close auto dump.
 */
static void close_auto_dump(void)
{
    char footer_mark_str[80];

    /* Prepare a footer mark string */
    sprintf(footer_mark_str, auto_dump_footer, auto_dump_mark);

    auto_dump_printf("# *警告!* 上面的行是自动转储的。\n");
    auto_dump_printf("# 请勿编辑这些内容；手动修改会被自动删除和替换。\n");

    /* End of dump */
    fprintf(auto_dump_stream, "%s (%d)\n", footer_mark_str, auto_dump_line_num);

    /* Close */
    my_fclose(auto_dump_stream);

    return;
}


/*
 * Return suffix of ordinal number
 */
cptr get_ordinal_number_suffix(int num)
{
    num = ABS(num) % 100;
    switch (num % 10)
    {
    case 1:
        return (num == 11) ? "th" : "st";
    case 2:
        return (num == 12) ? "th" : "nd";
    case 3:
        return (num == 13) ? "th" : "rd";
    default:
        return "th";
    }
}

/*
 * Toggle easy_mimics
 */
void toggle_easy_mimics(bool kayta)
{
    int i;
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        if (!r_ptr->name) continue;
        if (r_ptr->flags7 & RF7_NASTY_GLYPH)
        {
            if ((kayta) && (r_ptr->x_char == r_ptr->d_char)) r_ptr->x_char = 'x';
            else if ((!kayta) && (r_ptr->x_char == 'x')) r_ptr->x_char = r_ptr->d_char;
            if (r_ptr->d_attr == color_char_to_attr('d'))
            {
                if (kayta) r_ptr->x_attr = color_char_to_attr('D');
                else r_ptr->x_attr = color_char_to_attr('d');
            } 
        }
    }
}

/*
 * Calculate delay length
 */
int delay_time(void)
{
    if (square_delays) return (delay_factor * delay_factor * 2);
    else return (delay_factor * delay_factor * delay_factor);
}


/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 */
bool redraw_hack;
void do_cmd_redraw(void)
{
    int j;

    term *old = Term;


    /* Hack -- react to changes */
    Term_xtra(TERM_XTRA_REACT, 0);


    /* Combine and Reorder the pack (later) */
    p_ptr->notice |= (PN_OPTIMIZE_PACK | PN_OPTIMIZE_QUIVER);


    /* Update torch */
    p_ptr->update |= (PU_TORCH);

    /* Update stuff */
    p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

    /* Forget lite/view */
    p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

    /* Update lite/view */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_MON_LITE);

    /* Update monsters */
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw everything */
    p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY | PR_MSG_LINE);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL);

    /* Window stuff */
    p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_DUNGEON |
        PW_MONSTER | PW_MONSTER_LIST | PW_OBJECT_LIST | PW_OBJECT);

    update_playtime();

    /* Prevent spamming ^R to circumvent fuzzy detection */
    redraw_hack = TRUE;
    handle_stuff();
    redraw_hack = FALSE;

    if (p_ptr->prace == RACE_ANDROID) android_calc_exp();


    /* Redraw every window */
    for (j = 0; j < 8; j++)
    {
        /* Dead window */
        if (!angband_term[j]) continue;

        /* Activate */
        Term_activate(angband_term[j]);

        /* Redraw */
        Term_redraw();

        /* Refresh */
        Term_fresh();

        /* Restore */
        Term_activate(old);
    }
}

/*
 * Show previous messages to the user    -BEN-
 *
 */
void do_cmd_messages(int old_now_turn)
{
    int     i;
    doc_ptr doc;
    int     current_turn = 0;
    int     current_row = 0;

    doc = doc_alloc(80);
    for (i = msg_count() - 1; i >= 0; i--)
    {
        msg_ptr m = msg_get(i);

        if (m->turn != current_turn)
        {
            if (doc_cursor(doc).y > current_row + 1)
                doc_newline(doc);
            current_turn = m->turn;
            current_row = doc_cursor(doc).y;
        }

        doc_insert_text(doc, m->color, string_buffer(m->msg));
        if (m->count > 1)
        {
            char buf[10];
            sprintf(buf, " (x%d)", m->count);
            doc_insert_text(doc, m->color, buf);
        }
        doc_newline(doc);
    }
    screen_save();
    doc_display(doc, "历史消息", doc_cursor(doc).y);
    screen_load();
    doc_free(doc);
}

#ifdef ALLOW_WIZARD

/*
 * Number of cheating options
 */
#define CHEAT_MAX 6

/*
 * Cheating options
 */
static option_type cheat_info[CHEAT_MAX] =
{
    { &cheat_peek,        FALSE,    255,    0x01, 0x00,
    "cheat_peek",        "查看物品生成"
    },

    { &cheat_hear,        FALSE,    255,    0x02, 0x00,
    "cheat_hear",        "查看怪物生成"
    },

    { &cheat_room,        FALSE,    255,    0x04, 0x00,
    "cheat_room",        "查看地城生成"
    },

    { &cheat_xtra,        FALSE,    255,    0x08, 0x00,
    "cheat_xtra",        "查看其他调试信息"
    },

    { &cheat_live,        FALSE,    255,    0x20, 0x00,
    "cheat_live",        "允许玩家避免死亡"
    },

    { &cheat_save,        FALSE,    255,    0x40, 0x00,
    "cheat_save",        "死亡时询问是否保存"
    }
};

/*
 * Interact with some options for cheating
 */
static void do_cmd_options_cheat(cptr info)
{
    int    ch;

    int        i, k = 0, n = CHEAT_MAX;

    char    buf[1024];


    /* Clear screen */
    Term_clear();

    /* Interact with the player */
    while (TRUE)
    {
        int dir;

        /* Prompt XXX XXX XXX */
        sprintf(buf, "%s (回车:下一项, y/n:设置, ESC:确认) ", info);

        prt(buf, 0, 0);

        /* Display the options */
        for (i = 0; i < n; i++)
        {
            byte a = TERM_WHITE;

            /* Color current option */
            if (i == k) a = TERM_L_BLUE;

            /* Display the option text */
            sprintf(buf, "%-48s: %s (%s)",
                cheat_info[i].o_desc,
                (*cheat_info[i].o_var ? "是" : "no "),

                cheat_info[i].o_text);
            c_prt(a, buf, i + 2, 0);
        }

        /* Hilite current option */
        move_cursor(k + 2, 50);

        autopick_inkey_hack = 1;

        /* Get a key */
        ch = inkey_special(TRUE);

        /*
         * HACK - Try to translate the key into a direction
         * to allow using the roguelike keys for navigation.
         */
        if (ch < 256)
        {
            dir = get_keymap_dir(ch, FALSE);
            if ((dir == 2) || (dir == 4) || (dir == 6) || (dir == 8) || (dir == 9) || (dir == 1))
                ch = I2D(dir);
        }

        /* Analyze */
        switch (ch)
        {
            case ESCAPE:
            {
                return;
            }

            case '-':
            case '8':
            case SKEY_UP:
            {
                k = (n + k - 1) % n;
                break;
            }

            case ' ':
            case '\n':
            case '\r':
            case '2':
            case SKEY_DOWN:
            {
                k = (k + 1) % n;
                break;
            }

            case SKEY_TOP:
            {
                k = MAX(0, k - 10);
                break;
            }

            case SKEY_BOTTOM:
            {
                k = MIN(n - 1, k + 10);
                break;
            }

            case 'y':
            case 'Y':
            case '6':
            case SKEY_RIGHT:
            {
                p_ptr->noscore |= (cheat_info[k].o_set * 256 + cheat_info[k].o_bit);
                (*cheat_info[k].o_var) = TRUE;
                k = (k + 1) % n;
                break;
            }

            case 'n':
            case 'N':
            case '4':
            case SKEY_LEFT:
            {
                (*cheat_info[k].o_var) = FALSE;
                k = (k + 1) % n;
                break;
            }

            case '?':
            {
                doc_display_help("option.txt", cheat_info[k].o_text);
                Term_clear();
                break;
            }

            default:
            {
                bell();
                break;
            }
        }
    }
}
#endif

static option_type autosave_info[2] =
{
    { &autosave_l,      TRUE, 255, 0x01, 0x00,
        "autosave_l",    "进入新楼层时自动保存" },


    { &autosave_t,      FALSE, 255, 0x02, 0x00,
        "autosave_t",   "定时自动保存" },

};


static s16b toggle_frequency(s16b current)
{
    switch (current)
    {
    case 0: return 50;
    case 50: return 100;
    case 100: return 250;
    case 250: return 500;
    case 500: return 1000;
    case 1000: return 2500;
    case 2500: return 5000;
    case 5000: return 10000;
    case 10000: return 25000;
    default: return 0;
    }
}


/*
 * Interact with some options for autosaving
 */
static void do_cmd_options_autosave(cptr info)
{
    char    ch;

    int     i, k = 0, n = 2;

    char    buf[80];


    /* Clear screen */
    Term_clear();

    /* Interact with the player */
    while (TRUE)
    {
        /* Prompt XXX XXX XXX */
        sprintf(buf, "%s (回车:下一项, y/n:设置, F:频率, ESC:确认) ", info);

        prt(buf, 0, 0);

        /* Display the options */
        for (i = 0; i < n; i++)
        {
            byte a = TERM_WHITE;

            /* Color current option */
            if (i == k) a = TERM_L_BLUE;

            /* Display the option text */
            sprintf(buf, "%-48s: %s (%s)",
                autosave_info[i].o_desc,
                (*autosave_info[i].o_var ? "是" : "no "),

                autosave_info[i].o_text);
            c_prt(a, buf, i + 2, 0);
        }

        prt(format("定时自动保存频率: 每 %d 回合",  autosave_freq), 5, 0);



        /* Hilite current option */
        move_cursor(k + 2, 50);

        /* Get a key */
        ch = inkey();

        /* Analyze */
        switch (ch)
        {
            case ESCAPE:
            {
                return;
            }

            case '-':
            case '8':
            {
                k = (n + k - 1) % n;
                break;
            }

            case ' ':
            case '\n':
            case '\r':
            case '2':
            {
                k = (k + 1) % n;
                break;
            }

            case 'y':
            case 'Y':
            case '6':
            {

                (*autosave_info[k].o_var) = TRUE;
                k = (k + 1) % n;
                break;
            }

            case 'n':
            case 'N':
            case '4':
            {
                (*autosave_info[k].o_var) = FALSE;
                k = (k + 1) % n;
                break;
            }

            case 'f':
            case 'F':
            {
                autosave_freq = toggle_frequency(autosave_freq);
                prt(format("定时自动保存频率: 每 %d 回合",
                       autosave_freq), 5, 0);
                break;
            }

            case '?':
            {
                doc_display_help("option.txt", "自动保存");

                Term_clear();
                break;
            }

            default:
            {
                bell();
                break;
            }
        }
    }
}


/*
 * Interact with some options
 */
void do_cmd_options_aux(int page, cptr info)
{
    enum { MAX_OPTIONS_PER_PAGE = 128 };
    int     ch;
    int     i, k = 0, n = 0, l;
    int     opt[MAX_OPTIONS_PER_PAGE];
    char    buf[1024];
    bool    browse_only = (page == OPT_PAGE_BIRTH) && character_generated &&
                          (!p_ptr->wizard || !allow_debug_opts);
    bool    scroll_mode;
    byte    option_offset = 0;
    byte    bottom_opt = Term->hgt - ((page == OPT_PAGE_AUTODESTROY) ? 5 : 2);

/*    browse_only = FALSE; */

    /* Lookup the options */
    for (i = 0; i < MAX_OPTIONS_PER_PAGE; i++) opt[i] = 0;

    /* Scan the options */
    for (i = 0; option_info[i].o_desc; i++)
    {
        /* Notice options on this "page" */
        if (option_info[i].o_page == page)
        {
            if (n >= MAX_OPTIONS_PER_PAGE)
            {
                quit_fmt("Too many options on option page %d", page);
            }
            opt[n++] = i;
        }
    }

    scroll_mode = (n > bottom_opt);

    /* Clear screen */
    Term_clear();

    /* Interact with the player */
    while (TRUE)
    {
        int dir;

        /* Prompt XXX XXX XXX */
        sprintf(buf, "%s (回车:下一项, %s, ?:帮助) ", info, browse_only ? "ESC:退出" : "y/n:切换, ESC:确认");

        prt(buf, 0, 0);


        /* HACK -- description for easy-auto-destroy options */
        if (page == OPT_PAGE_AUTODESTROY) c_prt(TERM_YELLOW, "以下选项会保护物品不被简易自动销毁器销毁。", 11, 3);

        /* Display the options */
        for (i = option_offset; i < n; i++)
        {
            int rivi;
            byte a = TERM_WHITE;

            /* Color current option */
            if (i == k) a = TERM_L_BLUE;

            /* Display the option text */
            if (option_info[opt[i]].o_var == &random_artifacts)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                if (random_artifacts)
                    strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%d%% ", random_artifact_pct);
                else
                    strcat(buf, "no  ");
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "(%.19s)", option_info[opt[i]].o_text);
            }
            else if (option_info[opt[i]].o_var == &ironman_empty_levels)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s", empty_lv_description[generate_empty]);
            }
            else if (option_info[opt[i]].o_var == &reduce_uniques)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                if (reduce_uniques)
                    strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%d%% ", reduce_uniques_pct);
                else
                    strcat(buf, "no  ");
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "(%.19s)", option_info[opt[i]].o_text);
            }
            else if (option_info[opt[i]].o_var == &obj_list_width)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%-3d ", object_list_width);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "(%.19s)", option_info[opt[i]].o_text);
            }
            else if (option_info[opt[i]].o_var == &mon_list_width)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%-3d ", monster_list_width);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "(%.19s)", option_info[opt[i]].o_text);
            }
            else if (option_info[opt[i]].o_var == &single_pantheon)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%d / %d", pantheon_count, PANTHEON_MAX - 1);
            }
            else if (option_info[opt[i]].o_var == &guaranteed_pantheon)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                if (pantheon_count == PANTHEON_MAX - 1)
                {
                    strcat(buf, "全部");
                }
                else if ((game_pantheon) && (game_pantheon < PANTHEON_MAX))
                {
                    strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%.3s ", pant_list[game_pantheon].short_name);
                }
                else
                    strcat(buf, "无");
            }
            else if (option_info[opt[i]].o_var == &always_small_levels)
            {
                strnfmt(buf, sizeof(buf), "%-48s: ", option_info[opt[i]].o_desc);
                strnfmt(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s ", lv_size_options[small_level_type]);
            }
            else
            {
                strnfmt(buf, sizeof(buf), "%-48s: %s (%.19s)",
                    option_info[opt[i]].o_desc,
                    (*option_info[opt[i]].o_var ? "是" : "no "),
                    option_info[opt[i]].o_text);
            }
            if ((page == OPT_PAGE_AUTODESTROY) && i > 7) rivi = i + 5 - option_offset;
            else rivi = i + 2 - option_offset;
            if ((scroll_mode) && (rivi == Term->hgt - 1) && (i < n - 1)) c_prt(TERM_YELLOW, " (向下滚动查看更多选项)", rivi, 0);
            else if ((scroll_mode) && (rivi == 2) && (i > 0)) c_prt(TERM_YELLOW, " (向上滚动查看更多选项)", rivi, 0);
            else if (((rivi >= 2) && (rivi < Term->hgt - 1)) || ((rivi == Term->hgt - 1) && ((i == n - 1) || (!scroll_mode)))) c_prt(a, buf, rivi, 0);
        }

        if ((page == OPT_PAGE_AUTODESTROY) && (k > 7)) l = 3;
        else l = 0;

        /* Hilite current option */
        move_cursor(k + 2 + l - option_offset, 50);

        autopick_inkey_hack = 1;

        /* Get a key */
        ch = inkey_special(TRUE);

        /*
         * HACK - Try to translate the key into a direction
         * to allow using the roguelike keys for navigation.
         */
        if (ch < 256)
        {
            dir = get_keymap_dir(ch, FALSE);
            if ((dir == 2) || (dir == 4) || (dir == 6) || (dir == 8) || (dir == 9) || (dir == 1))
                ch = I2D(dir);
        }

        /* Analyze */
        switch (ch)
        {
            case ESCAPE:
            {
                return;
            }

            case '-':
            case '8':
            case SKEY_UP:
            {
                k = (n + k - 1) % n;
                if (scroll_mode)
                {
                    if (k > bottom_opt - 1 + option_offset) option_offset = k - bottom_opt + 1; /* ((k == n - 1) ? 1 : 2); */
                    else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                }
                break;
            }

            case ' ':
            case '\n':
            case '\r':
            case '2':
            case SKEY_DOWN:
            {
                k = (k + 1) % n;
                if (scroll_mode)
                {
                    if (k > bottom_opt - 2 + option_offset) option_offset = k - bottom_opt + ((k == n - 1) ? 1 : 2);
                    else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                }
                break;
            }

            case '7':
            case '9':
            case SKEY_PGUP:
            case SKEY_TOP:
            {
                k = MAX(0, k - 10);
                if (scroll_mode)
                {
                    if (k > bottom_opt - 1 + option_offset) option_offset = k - bottom_opt + 1; /* ((k == n - 1) ? 1 : 2); */
                    else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                }
                break;
            }

            case '1':
            case '3':
            case SKEY_PGDOWN:
            case SKEY_BOTTOM:
            {
                k = MIN(n - 1, k + 10);
                if (scroll_mode)
                {
                    if (k > bottom_opt - 2 + option_offset) option_offset = k - bottom_opt + ((k == n - 1) ? 1 : 2);
                    else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                }
                break;
            }

            case 'y':
            case 'Y':
            case '6':
            case SKEY_RIGHT:
            {
                if (browse_only) break;
                if (option_info[opt[k]].o_var == &random_artifacts)
                {
                    if (!random_artifacts)
                    {
                        random_artifacts = TRUE;
                        random_artifact_pct = 10;
                    }
                    else
                    {
                        random_artifact_pct += 10;
                        if (random_artifact_pct > 100) random_artifacts = FALSE;
                    }
                }
                else if (option_info[opt[k]].o_var == &obj_list_width)
                {
                    int maksi = MAX(50, Term->wid - 15);
                    maksi &= ~(0x01);
                    object_list_width += 2;
                    if (object_list_width > maksi) object_list_width = maksi;
                }
                else if (option_info[opt[k]].o_var == &mon_list_width)
                {
                    int maksi = MAX(50, Term->wid - 15);
                    maksi &= ~(0x01);
                    monster_list_width += 2;
                    if (monster_list_width > maksi) monster_list_width = maksi;
                }
                else if (option_info[opt[k]].o_var == &reduce_uniques)
                {
                    if (!reduce_uniques)
                    {
                        reduce_uniques = TRUE;
                        reduce_uniques_pct = 10;
                    }
                    else
                    {
                        reduce_uniques_pct += 10;
                        if (reduce_uniques_pct >= 100) reduce_uniques = FALSE;
                    }
                }
                else if (option_info[opt[k]].o_var == &ironman_empty_levels)
                {
                    generate_empty++;
                    if (generate_empty == EMPTY_MAX) generate_empty = 0;
                    ironman_empty_levels = (generate_empty == EMPTY_ALWAYS);
                }
                else if (option_info[opt[k]].o_var == &single_pantheon)
                {
                    pantheon_count++;
                    if (pantheon_count >= PANTHEON_MAX) pantheon_count = 1;
                }
                else if (option_info[opt[k]].o_var == &guaranteed_pantheon)
                {
                    game_pantheon++;
                    if (game_pantheon >= PANTHEON_MAX) game_pantheon = 0;
                }
                else if (option_info[opt[k]].o_var == &always_small_levels)
                {
                    if (!always_small_levels)
                    {
                        always_small_levels = TRUE;
                        small_level_type = 1;
                    }
                    else
                    {
                        small_level_type++;
                        if (small_level_type > SMALL_LVL_MAX)
                        {
                            always_small_levels = FALSE;
                            small_level_type = 0;
                        }
                    }
                }
                else
                {
                    (*option_info[opt[k]].o_var) = TRUE;
                    k = (k + 1) % n;
                    if (scroll_mode)
                    {
                        if (k > bottom_opt - 2 + option_offset) option_offset = k - bottom_opt + ((k == n - 1) ? 1 : 2);
                        else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                    }
                }
                break;
            }

            case 'n':
            case 'N':
            case '4':
            case SKEY_LEFT:
            {
                if (browse_only) break;
                if (option_info[opt[k]].o_var == &random_artifacts)
                {
                    if (!random_artifacts)
                    {
                        random_artifacts = TRUE;
                        random_artifact_pct = 100;
                    }
                    else
                    {
                        random_artifact_pct -= 10;
                        if (random_artifact_pct <= 0) random_artifacts = FALSE;
                    }
                }
                else if (option_info[opt[k]].o_var == &reduce_uniques)
                {
                    if (!reduce_uniques)
                    {
                        reduce_uniques = TRUE;
                        reduce_uniques_pct = 90;
                    }
                    else
                    {
                        reduce_uniques_pct -= 10;
                        if (reduce_uniques_pct <= 0)
                        {
                            reduce_uniques = FALSE;
                            reduce_uniques_pct = 100;
                        }
                    }
                }
                else if (option_info[opt[k]].o_var == &obj_list_width)
                {
                    object_list_width -= 2;
                    if (object_list_width < 24) object_list_width = 24;
                }
                else if (option_info[opt[k]].o_var == &mon_list_width)
                {
                    monster_list_width -= 2;
                    if (monster_list_width < 24) monster_list_width = 24;
                }
                else if (option_info[opt[k]].o_var == &ironman_empty_levels)
                {
                    if (generate_empty == 0) generate_empty = EMPTY_MAX - 1;
                    else generate_empty--;
                    ironman_empty_levels = (generate_empty == EMPTY_ALWAYS);
                }
                else if (option_info[opt[k]].o_var == &single_pantheon)
                {
                    pantheon_count--;
                    if (pantheon_count < 1) pantheon_count = PANTHEON_MAX - 1;
                }
                else if (option_info[opt[k]].o_var == &guaranteed_pantheon)
                {
                    if (game_pantheon) game_pantheon--;
                    else game_pantheon = PANTHEON_MAX - 1;
                }
                else if (option_info[opt[k]].o_var == &always_small_levels)
                {
                    if (!always_small_levels)
                    {
                        always_small_levels = TRUE;
                        small_level_type = SMALL_LVL_MAX;
                    }
                    else
                    {
                        small_level_type--;
                        if (small_level_type == 0) always_small_levels = FALSE;
                    }
                }
                else
                {
                    (*option_info[opt[k]].o_var) = FALSE;
                    k = (k + 1) % n;
                    if (scroll_mode)
                    {
                        if (k > bottom_opt - 2 + option_offset) option_offset = k - bottom_opt + ((k == n - 1) ? 1 : 2);
                        else if ((k < option_offset) || ((k > 0) && (k == option_offset))) option_offset = MAX(0, k - 3);
                    }
                }
                break;
            }

            case 't':
            case 'T':
            {
                if (!browse_only) (*option_info[opt[k]].o_var) = !(*option_info[opt[k]].o_var);
                break;
            }

            case '?':
            {
                doc_display_help("option.txt", option_info[opt[k]].o_text);
                Term_clear();
                break;
            }

            default:
            {
                bell();
                break;
            }
        }
    }
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
    int i, j, d;

    int y = 0;
    int x = 0;

    char ch;

    bool go = TRUE;

    u32b old_flag[8];


    /* Memorize old flags */
    for (j = 0; j < 8; j++)
    {
        /* Acquire current flags */
        old_flag[j] = window_flag[j];
    }


    /* Clear screen */
    Term_clear();

    /* Interact */
    while (go)
    {
        /* Prompt XXX XXX XXX */
        prt("窗口标记(<dir>, t, y, n, ESC) ", 0, 0);


        /* Display the windows */
        for (j = 0; j < 8; j++)
        {
            byte a = TERM_WHITE;

            cptr s = angband_term_name[j];

            /* Use color */
            if (j == x) a = TERM_L_BLUE;

            /* Window name, staggered, centered */
            Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
        }

        /* Display the options */
        for (i = 0; i < 16; i++)
        {
            byte a = TERM_WHITE;

            cptr str = window_flag_desc[i];

            /* Use color */
            if (i == y) a = TERM_L_BLUE;

            /* Unused option */
            if (!str) str = "(未使用选项)";


            /* Flag name */
            Term_putstr(0, i + 5, -1, a, str);

            /* Display the windows */
            for (j = 0; j < 8; j++)
            {
                byte a = TERM_WHITE;

                char c = '.';

                /* Use color */
                if ((i == y) && (j == x)) a = TERM_L_BLUE;

                /* Active flag */
                if (window_flag[j] & (1L << i)) c = 'X';

                /* Flag value */
                Term_putch(35 + j * 5, i + 5, a, c);
            }
        }

        /* Place Cursor */
        Term_gotoxy(35 + x * 5, y + 5);

        /* Get key */
        ch = inkey();

        /* Analyze */
        switch (ch)
        {
            case ESCAPE:
            {
                go = FALSE;
                break;
            }

            case 'T':
            case 't':
            {
                /* Clear windows */
                for (j = 0; j < 8; j++)
                {
                    window_flag[j] &= ~(1L << y);
                }

                /* Clear flags */
                for (i = 0; i < 16; i++)
                {
                    window_flag[x] &= ~(1L << i);
                }
            }   /* Fall through */
            case 'y':
            case 'Y':
            {
                /* Ignore screen */
                if (x == 0) break;

                /* Set flag */
                window_flag[x] |= (1L << y);
                break;
            }

            case 'n':
            case 'N':
            {
                /* Clear flag */
                window_flag[x] &= ~(1L << y);
                break;
            }

            case '?':
            {
                doc_display_help("option.txt", "窗口");
                Term_clear();
                break;
            }

            default:
            {
                d = get_keymap_dir(ch, FALSE);

                x = (x + ddx[d] + 8) % 8;
                y = (y + ddy[d] + 16) % 16;

                if (!d) bell();
            }
        }
    }

    /* Notice changes */
    for (j = 0; j < 8; j++)
    {
        term *old = Term;

        /* Dead window */
        if (!angband_term[j]) continue;

        /* Ignore non-changes */
        if (window_flag[j] == old_flag[j]) continue;

        /* Activate */
        Term_activate(angband_term[j]);

        /* Erase */
        Term_clear();

        /* Refresh */
        Term_fresh();

        /* Restore */
        Term_activate(old);
    }
}



#define OPT_NUM 15

static struct opts
{
    char key;
    cptr name;
    int row;
}
option_fields[OPT_NUM] =
{
    { '1', "输入选项", 3 },
    { '2', "地图显示选项", 4 },
    { '3', "文本显示选项", 5 },
    { '4', "游戏玩法选项", 6 },
    { '5', "打扰提示选项", 7 },
    { '6', "自动销毁选项", 8 },
    { '7', "列表显示选项", 9 },

    { 'p', "墨家明器偏好", 11 },
    { 'd', "基础延迟系数", 12 },
    { 'h', "生命警告阈值", 13 },
    { 'm', "法力颜色阈值", 14 },
    { 'a', "自动保存选项", 15 },
    { 'w', "窗口标记", 16 },

    { 'b', "出生选项(仅浏览)", 18 },
    { 'c', "作弊选项", 19 },
};


/*
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void)
{
    char k;
    int i, d, skey;
    int y = 0;
    bool old_easy_mimics = easy_mimics;

    /* Save the screen */
    screen_save();

    /* Interact */
    while (1)
    {
        int n = OPT_NUM;

        /* Does not list cheat option when cheat option is off */
        if (!p_ptr->noscore && !allow_debug_opts) n--;

        /* Clear screen */
        Term_clear();

        /* Why are we here */
        prt("RoguelikeFansBand 选项", 1, 0);

        while(1)
        {
            /* Give some choices */
            for (i = 0; i < n; i++)
            {
                byte a = TERM_WHITE;
                if (i == y) a = TERM_L_BLUE;
#ifndef ALLOW_WIZARD
                if (option_fields[i].key == 'c') continue;
#endif
                Term_putstr(5, option_fields[i].row, -1, a,
                    format("(%c) %s", toupper(option_fields[i].key), option_fields[i].name));
            }

            prt("移动:<dir> 选择:回车 取消:ESC  ?:帮助: ", 21, 0);

            /* Get command */
            skey = inkey_special(TRUE);
            if (!(skey & SKEY_MASK)) k = (char)skey;
            else k = 0;

            /* Exit */
            if (IS_ESCAPE(k)) break;

            if (my_strchr("\n\r ", k))
            {
                k = option_fields[y].key;
                break;
            }

            for (i = 0; i < n; i++)
            {
                if (tolower(k) == option_fields[i].key) break;
            }

            /* Command is found */
            if (i < n) break;

            /* Hack -- browse help */
            if (k == '?') break;

            /* Move cursor */
            d = 0;
            if (skey == SKEY_UP) d = 8;
            if (skey == SKEY_DOWN) d = 2;
            y = (y + ddy[d] + n) % n;
            if (!d) bell();
        }

        /* Exit */
        if (IS_ESCAPE(k)) break;

        /* Analyze */
        switch (k)
        {
            case '1':
            {
                /* Process the general options */
                do_cmd_options_aux(OPT_PAGE_INPUT, "输入选项");
                break;
            }

            case '2':
            {
                /* Process the general options */
                do_cmd_options_aux(OPT_PAGE_MAPSCREEN, "地图显示选项");
                break;
            }

            case '3':
            {
                /* Spawn */
                do_cmd_options_aux(OPT_PAGE_TEXT, "文本显示选项");
                break;
            }

            case '4':
            {
                /* Spawn */
                do_cmd_options_aux(OPT_PAGE_GAMEPLAY, "游戏玩法选项");
                break;
            }

            case '5':
            {
                /* Spawn */
                do_cmd_options_aux(OPT_PAGE_DISTURBANCE, "打扰提示选项");
                break;
            }

            case '6':
            {
                /* Spawn */
                do_cmd_options_aux(OPT_PAGE_AUTODESTROY, "自动销毁选项");
                break;
            }

            case '7':
            {
                /* Spawn */
                do_cmd_options_aux(OPT_PAGE_LIST, "列表显示选项");
                break;
            }

            /* Birth Options */
            case 'B':
            case 'b':
            {
                do_cmd_options_aux(OPT_PAGE_BIRTH, (!p_ptr->wizard || !allow_debug_opts) ? "出生选项(仅浏览)" : "出生选项((*)影响分数)");
                break;
            }

            /* Cheating Options */
            case 'C':
            {
#ifdef ALLOW_WIZARD
                if (!p_ptr->noscore && !allow_debug_opts)
                {
                    /* Cheat options are not permitted */
                    bell();
                    break;
                }

                /* Spawn */
                do_cmd_options_cheat("作弊者不会胜利");
#else
                bell();
#endif
                break;
            }

            case 'a':
            case 'A':
            {
                do_cmd_options_autosave("自动保存");
                break;
            }

            /* Window flags */
            case 'W':
            case 'w':
            {
                /* Spawn */
                do_cmd_options_win();
                p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL |
                          PW_MONSTER_LIST | PW_OBJECT_LIST | PW_MESSAGE | PW_OVERHEAD |
                          PW_MONSTER | PW_OBJECT | PW_SNAPSHOT |
                          PW_BORG_1 | PW_BORG_2 | PW_DUNGEON);
                break;
            }

            /* Auto-picker/destroyer editor */
            case 'P':
            case 'p':
            {
                do_cmd_edit_autopick();
                break;
            }

            /* Hack -- Delay Speed */
            case 'D':
            case 'd':
            {
                /* Prompt */
                clear_from(18);
                prt("命令: 基础延迟系数", 19, 0);

                /* Get a new value */
                while (1)
                {
                    int msec = delay_time();
                    prt(format("当前基础延迟: %d (%d毫秒)",
                           delay_factor, msec), 22, 0);

                    prt("延迟系数(0-9, ESC确认): ", 20, 0);

                    k = inkey();
                    if (k == ESCAPE) break;
                    else if (k == '?')
                    {
                        doc_display_help("option.txt", "基础延迟");
                        Term_clear();
                    }
                    else if (isdigit(k)) delay_factor = D2I(k);
                    else bell();
                }

                break;
            }

            /* Hack -- hitpoint warning factor */
            case 'H':
            case 'h':
            {
                /* Prompt */
                clear_from(18);
                prt("命令: 生命警告阈值", 19, 0);

                /* Get a new value */
                while (1)
                {
                    prt(format("当前生命警告: %d0%%",
                           hitpoint_warn), 22, 0);

                    prt("生命警告阈值(0-9, ESC确认): ", 20, 0);

                    k = inkey();
                    if (k == ESCAPE) break;
                    else if (k == '?')
                    {
                        doc_display_help("option.txt", "生命值");
                        Term_clear();
                    }
                    else if (isdigit(k)) hitpoint_warn = D2I(k);
                    else bell();
                }

                break;
            }

            /* Hack -- mana color factor */
            case 'M':
            case 'm':
            {
                /* Prompt */
                clear_from(18);
                prt("命令: 法力颜色阈值", 19, 0);

                /* Get a new value */
                while (1)
                {
                    prt(format("当前法力颜色阈值: %d0%%",
                           mana_warn), 22, 0);

                    prt("法力颜色阈值(0-9, ESC确认): ", 20, 0);

                    k = inkey();
                    if (k == ESCAPE) break;
                    else if (k == '?')
                    {
                        doc_display_help("option.txt", "法力值");
                        Term_clear();
                    }
                    else if (isdigit(k)) mana_warn = D2I(k);
                    else bell();
                }

                break;
            }

            case '?':
                doc_display_help("option.txt", NULL);
                Term_clear();
                break;

            /* Unknown option */
            default:
            {
                /* Oops */
                bell();
                break;
            }
        }

        /* Flush messages */
        msg_print(NULL);
    }

    /* Big fat hack */
    if (easy_mimics || old_easy_mimics) toggle_easy_mimics(easy_mimics);

    /* Restore the screen */
    screen_load();

    /* Hack - Redraw equippy chars */
    p_ptr->redraw |= (PR_EQUIPPY);
}



/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 */
void do_cmd_pref(void)
{
    char buf[80];

    /* Default */
    strcpy(buf, "");

    /* Ask for a "user pref command" */
    if (!get_string("首选项:", buf, 80)) return;


    /* Process that pref command */
    (void)process_pref_file_command(buf);
}

void do_cmd_reload_autopick(void)
{
    if (!get_check("重新加载自动拾取首选项(pref)文件？")) return;

    /* Load the file with messages */
    autopick_load_pref(ALP_DISP_MES);
}

#ifdef ALLOW_MACROS

/*
 * Hack -- append all current macros to the given file
 */
static errr macro_dump(cptr fname)
{
    static cptr mark = "宏转储(Macro Dump)";

    int i;

    char buf[1024];

    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);

    /* Append to the file */
    if (!open_auto_dump(buf, mark)) return (-1);

    /* Start dumping */
    auto_dump_printf("\n# 自动宏转储\n\n");

    /* Dump them */
    for (i = 0; i < macro__num; i++)
    {
        /* Extract the action */
        ascii_to_text(buf, macro__act[i]);

        /* Dump the macro */
        auto_dump_printf("A:%s\n", buf);

        /* Extract the action */
        ascii_to_text(buf, macro__pat[i]);

        /* Dump normal macros */
        auto_dump_printf("P:%s\n", buf);

        /* End the macro */
        auto_dump_printf("\n");
    }

    /* Close */
    close_auto_dump();

    /* Success */
    return (0);
}


/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "util.c".
 *
 * Note that both "flush()" calls are extremely important.
 */
static void do_cmd_macro_aux(char *buf)
{
    int i, n = 0;

    char tmp[1024];


    /* Flush */
    flush();

    /* Do not process macros */
    inkey_base = TRUE;

    /* First key */
    i = inkey();

    /* Read the pattern */
    while (i)
    {
        /* Save the key */
        buf[n++] = i;

        /* Do not process macros */
        inkey_base = TRUE;

        /* Do not wait for keys */
        inkey_scan = TRUE;

        /* Attempt to read a key */
        i = inkey();
    }

    /* Terminate */
    buf[n] = '\0';

    /* Flush */
    flush();


    /* Convert the trigger */
    ascii_to_text(tmp, buf);

    /* Hack -- display the trigger */
    Term_addstr(-1, TERM_WHITE, tmp);
}

#endif


/*
 * Hack -- ask for a keymap "trigger" (see below)
 *
 * Note that both "flush()" calls are extremely important. This may
 * no longer be true, since "util.c" is much simpler now. XXX XXX XXX
 */
static void do_cmd_macro_aux_keymap(char *buf)
{
    char tmp[1024];


    /* Flush */
    flush();


    /* Get a key */
    buf[0] = inkey();
    buf[1] = '\0';


    /* Convert to ascii */
    ascii_to_text(tmp, buf);

    /* Hack -- display the trigger */
    Term_addstr(-1, TERM_WHITE, tmp);


    /* Flush */
    flush();
}


/*
 * Hack -- append all keymaps to the given file
 */
static errr keymap_dump(cptr fname)
{
    static cptr mark = "键位映射转储(Keymap Dump)";
    int i;

    char key[1024];
    char buf[1024];

    int mode;

    /* Roguelike */
    if (rogue_like_commands)
    {
        mode = KEYMAP_MODE_ROGUE;
    }

    /* Original */
    else
    {
        mode = KEYMAP_MODE_ORIG;
    }


    /* Build the filename */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);

    /* Append to the file */
    if (!open_auto_dump(buf, mark)) return -1;

    /* Start dumping */
    auto_dump_printf("\n# 自动键位映射转储\n\n");

    /* Dump them */
    for (i = 0; i < 256; i++)
    {
        cptr act;

        /* Loop up the keymap */
        act = keymap_act[mode][i];

        /* Skip empty keymaps */
        if (!act) continue;

        /* Encode the key */
        buf[0] = i;
        buf[1] = '\0';
        ascii_to_text(key, buf);

        /* Encode the action */
        ascii_to_text(buf, act);

        /* Dump the macro */
        auto_dump_printf("A:%s\n", buf);
        auto_dump_printf("C:%d:%s\n", mode, key);
    }

    /* Close */
    close_auto_dump();

    /* Success */
    return (0);
}



/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * Could use some helpful instructions on this page. XXX XXX XXX
 */
void do_cmd_macros(void)
{
    int i;

    char tmp[1024];

    char buf[1024];

    int mode;


    /* Roguelike */
    if (rogue_like_commands)
    {
        mode = KEYMAP_MODE_ROGUE;
    }

    /* Original */
    else
    {
        mode = KEYMAP_MODE_ORIG;
    }

    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);


    /* Save screen */
    screen_save();


    /* Process requests until done */
    while (1)
    {
        /* Clear screen */
        Term_clear();

        /* Describe */
        prt("宏设置", 2, 0);



        /* Describe that action */
        prt("当前动作如下:", 20, 0);


        /* Analyze the current action */
        ascii_to_text(buf, macro__buf);

        /* Display the current action */
        prt(buf, 22, 0);


        /* Selections */
        prt("(1) 读取用户 pref 文件", 4, 5);

#ifdef ALLOW_MACROS
        prt("(2) 追加宏和键位映射到文件", 5, 5);
        prt("(3) 查询宏", 6, 5);
        prt("(4) 创建宏", 7, 5);
        prt("(5) 移除宏", 8, 5);
        prt("(7) 查询键位映射", 10, 5);
        prt("(8) 创建键位映射", 11, 5);
        prt("(9) 移除键位映射", 12, 5);
        prt("(0) 输入新动作", 13, 5);

#endif /* ALLOW_MACROS */

        /* Prompt */
        prt("命令: ", 16, 0);


        /* Get a command */
        i = inkey();

        /* Leave */
        if (i == ESCAPE) break;

        /* Load a 'macro' file */
        else if (i == '1')
        {
            errr err;

            /* Prompt */
            prt("命令: 读取用户 pref 文件", 16, 0);


            /* Prompt */
            prt("文件: ", 18, 0);


            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Ask for a file */
            if (!askfor(tmp, 80)) continue;

            /* Process the given filename */
            err = process_pref_file(tmp);
            if (-2 == err)
            {
                msg_format("已读取默认 '%s'。", tmp);
            }
            else if (err)
            {
                /* Prompt */
                msg_format("读取 '%s' 失败！", tmp);
            }
            else
            {
                msg_format("已读取 '%s'。", tmp);
            }
        }

#ifdef ALLOW_MACROS

        /* Save macros */
        else if (i == '2')
        {
            /* Prompt */
            prt("命令: 追加宏和键位映射到文件", 16, 0);


            /* Prompt */
            prt("文件: ", 18, 0);


            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Ask for a file */
            if (!askfor(tmp, 80)) continue;

            /* Dump the macros */
            (void)macro_dump(tmp);
            (void)keymap_dump(tmp);

            /* Prompt */
            msg_print("已追加宏和键位映射。");

        }

        /* Query a macro */
        else if (i == '3')
        {
            int k;

            /* Prompt */
            prt("命令: 查询宏", 16, 0);


            /* Prompt */
            prt("触发键: ", 18, 0);


            /* Get a macro trigger */
            do_cmd_macro_aux(buf);

            /* Acquire action */
            k = macro_find_exact(buf);

            /* Nothing found */
            if (k < 0)
            {
                /* Prompt */
                msg_print("未找到宏。");

            }

            /* Found one */
            else
            {
                /* Obtain the action */
                strcpy(macro__buf, macro__act[k]);

                /* Analyze the current action */
                ascii_to_text(buf, macro__buf);

                /* Display the current action */
                prt(buf, 22, 0);

                /* Prompt */
                msg_print("已找到宏。");

            }
        }

        /* Create a macro */
        else if (i == '4')
        {
            /* Prompt */
            prt("命令: 创建宏", 16, 0);


            /* Prompt */
            prt("触发键: ", 18, 0);


            /* Get a macro trigger */
            do_cmd_macro_aux(buf);

            /* Clear */
            clear_from(20);

            /* Help message */
            c_prt(TERM_L_RED, "用左右方向键移动光标，退格/Delete 删除字符。", 22, 0);

            /* Prompt */
            prt("动作: ", 20, 0);


            /* Convert to text */
            ascii_to_text(tmp, macro__buf);

            /* Get an encoded action */
            if (askfor(tmp, 80))
            {
                /* Convert to ascii */
                text_to_ascii(macro__buf, tmp);

                /* Link the macro */
                macro_add(buf, macro__buf);

                /* Prompt */
                msg_print("已添加宏。");

            }
        }

        /* Remove a macro */
        else if (i == '5')
        {
            /* Prompt */
            prt("命令: 移除宏", 16, 0);


            /* Prompt */
            prt("触发键: ", 18, 0);


            /* Get a macro trigger */
            do_cmd_macro_aux(buf);

            /* Link the macro */
            macro_add(buf, buf);

            /* Prompt */
            msg_print("已移除宏。");

        }

        /* Query a keymap */
        else if (i == '7')
        {
            cptr act;

            /* Prompt */
            prt("命令: 查询键位映射", 16, 0);


            /* Prompt */
            prt("按键: ", 18, 0);


            /* Get a keymap trigger */
            do_cmd_macro_aux_keymap(buf);

            /* Look up the keymap */
            act = keymap_act[mode][(byte)(buf[0])];

            /* Nothing found */
            if (!act)
            {
                /* Prompt */
                msg_print("未找到键位映射。");

            }

            /* Found one */
            else
            {
                /* Obtain the action */
                strcpy(macro__buf, act);

                /* Analyze the current action */
                ascii_to_text(buf, macro__buf);

                /* Display the current action */
                prt(buf, 22, 0);

                /* Prompt */
                msg_print("已找到键位映射。");

            }
        }

        /* Create a keymap */
        else if (i == '8')
        {
            /* Prompt */
            prt("命令: 创建键位映射", 16, 0);


            /* Prompt */
            prt("按键: ", 18, 0);


            /* Get a keymap trigger */
            do_cmd_macro_aux_keymap(buf);

            /* Clear */
            clear_from(20);

            /* Help message */
            c_prt(TERM_L_RED, "用左右方向键移动光标，退格/Delete 删除字符。", 22, 0);

            /* Prompt */
            prt("动作: ", 20, 0);


            /* Convert to text */
            ascii_to_text(tmp, macro__buf);

            /* Get an encoded action */
            if (askfor(tmp, 80))
            {
                /* Convert to ascii */
                text_to_ascii(macro__buf, tmp);

                /* Free old keymap */
                z_string_free(keymap_act[mode][(byte)(buf[0])]);

                /* Make new keymap */
                keymap_act[mode][(byte)(buf[0])] = z_string_make(macro__buf);

                /* Prompt */
                msg_print("已添加键位映射。");

            }
        }

        /* Remove a keymap */
        else if (i == '9')
        {
            /* Prompt */
            prt("命令: 移除键位映射", 16, 0);


            /* Prompt */
            prt("按键: ", 18, 0);


            /* Get a keymap trigger */
            do_cmd_macro_aux_keymap(buf);

            /* Free old keymap */
            z_string_free(keymap_act[mode][(byte)(buf[0])]);

            /* Make new keymap */
            keymap_act[mode][(byte)(buf[0])] = NULL;

            /* Prompt */
            msg_print("已移除键位映射。");

        }

        /* Enter a new action */
        else if (i == '0')
        {
            /* Prompt */
            prt("命令: 输入新动作", 16, 0);

            /* Clear */
            clear_from(20);

            /* Help message */
            c_prt(TERM_L_RED, "用左右方向键移动光标，退格/Delete 删除字符。", 22, 0);

            /* Prompt */
            prt("动作: ", 20, 0);

            /* Hack -- limit the value */
            tmp[80] = '\0';

            /* Get an encoded action */
            if (!askfor(buf, 80)) continue;

            /* Extract an action */
            text_to_ascii(macro__buf, buf);
        }

#endif /* ALLOW_MACROS */

        /* Oops */
        else
        {
            /* Oops */
            bell();
        }

        /* Flush messages */
        msg_print(NULL);
    }

    /* Load screen */
    screen_load();
}


static cptr lighting_level_str[F_LIT_MAX] =
{
    "标准",
    "明亮",
    "昏暗",
};

static void _sync_feature_lighting_chars(feature_type *f_ptr, byte old_char, byte new_char)
{
    int i;
    bool force = have_flag(f_ptr->flags, FF_FLOOR);

    if (!force && (old_char == new_char)) return;

    for (i = F_LIT_NS_BEGIN; i < F_LIT_MAX; i++)
    {
        if (force || (f_ptr->x_char[i] == old_char))
            f_ptr->x_char[i] = new_char;
    }
}


static bool cmd_visuals_aux(int i, int *num, int max)
{
    if (iscntrl(i))
    {
        char str[10] = "";
        int tmp;

        sprintf(str, "%d", *num);

        if (!get_string(format("输入新数字(0-%d): ", max-1), str, 5))
            return FALSE;

        tmp = strtol(str, NULL, 0);
        if (tmp >= 0 && tmp < max)
            *num = tmp;
    }
    else if (isupper(i))
        *num = (*num + max - 1) % max;
    else
        *num = (*num + 1) % max;

    return TRUE;
}

static void _graph_visuals_color_swatch(int col, int row, u32b rgb)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        Term_queue_char(col + i, row, TERM_WHITE, ' ', 0, 0);
        Term_queue_rgb(col + i, row, 0x00FFFFFF, rgb & 0x00FFFFFF);
    }
}

static void _graph_visuals_reset_fill_colors(void)
{
    graph_wall_rgb = GRAPH_DEFAULT_WALL_RGB;
    graph_permawall_rgb = GRAPH_DEFAULT_PERMAWALL_RGB;
    graph_magma_wall_rgb = GRAPH_DEFAULT_MAGMA_WALL_RGB;
    graph_quartz_wall_rgb = GRAPH_DEFAULT_QUARTZ_WALL_RGB;
    graph_treasure_wall_rgb = GRAPH_DEFAULT_TREASURE_WALL_RGB;
    graph_floor_rgb = GRAPH_DEFAULT_FLOOR_RGB;
}

#define GRAPH_VISUALS_FILL_COUNT 6
#define GRAPH_VISUALS_FILL_ROW 14
#define GRAPH_VISUALS_SLIDER_ROW 20

static cptr _graph_visuals_fill_label(int item)
{
    switch (item)
    {
    case 0: return "普通墙";
    case 1: return "永久墙";
    case 2: return "岩浆矿脉";
    case 3: return "石英矿脉";
    case 4: return "宝藏墙";
    case 5: return "地面";
    }
    return "";
}

static u32b *_graph_visuals_fill_rgb(int item)
{
    switch (item)
    {
    case 0: return &graph_wall_rgb;
    case 1: return &graph_permawall_rgb;
    case 2: return &graph_magma_wall_rgb;
    case 3: return &graph_quartz_wall_rgb;
    case 4: return &graph_treasure_wall_rgb;
    case 5: return &graph_floor_rgb;
    }
    return &graph_wall_rgb;
}

static int _graph_visuals_get_channel(u32b rgb, int channel)
{
    switch (channel)
    {
    case 0: return (int)((rgb >> 16) & 0xFF);
    case 1: return (int)((rgb >> 8) & 0xFF);
    case 2: return (int)(rgb & 0xFF);
    }
    return 0;
}

static void _graph_visuals_set_channel(u32b *rgb, int channel, int value)
{
    int r = _graph_visuals_get_channel(*rgb, 0);
    int g = _graph_visuals_get_channel(*rgb, 1);
    int b = _graph_visuals_get_channel(*rgb, 2);

    if (value < 0) value = 0;
    if (value > 255) value = 255;

    switch (channel)
    {
    case 0: r = value; break;
    case 1: g = value; break;
    case 2: b = value; break;
    }

    *rgb = ((u32b)r << 16) | ((u32b)g << 8) | (u32b)b;
}

static void _graph_visuals_step_channel(u32b *rgb, int channel, int delta)
{
    _graph_visuals_set_channel(rgb, channel, _graph_visuals_get_channel(*rgb, channel) + delta);
}

static void _graph_visuals_color_bar(int col, int row, int value, u32b rgb)
{
    int i;
    int width = 24;
    int filled = (value * width + 127) / 255;

    for (i = 0; i < width; i++)
    {
        u32b bg = (i < filled) ? rgb : 0x00111111;
        Term_queue_char(col + i, row, TERM_WHITE, ' ', 0, 0);
        Term_queue_rgb(col + i, row, 0x00FFFFFF, bg);
    }
}

static void _graph_visuals_print_slider(int row, cptr label, int value, u32b rgb, bool selected)
{
    Term_putstr(5, row, -1, TERM_WHITE, format("%c %s %3d", selected ? '>' : ' ', label, value));
    _graph_visuals_color_bar(14, row, value, rgb);
}

static void _graph_visuals_print_fill_editor(int item, int channel)
{
    int i;
    u32b rgb = *_graph_visuals_fill_rgb(item);

    for (i = 0; i < GRAPH_VISUALS_FILL_COUNT; i++)
    {
        Term_putstr(5, GRAPH_VISUALS_FILL_ROW + i, -1, TERM_WHITE,
            format("%c %-10s", (i == item) ? '>' : ' ', _graph_visuals_fill_label(i)));
        _graph_visuals_color_swatch(18, GRAPH_VISUALS_FILL_ROW + i, *_graph_visuals_fill_rgb(i));
    }

    _graph_visuals_print_slider(GRAPH_VISUALS_SLIDER_ROW, "红", _graph_visuals_get_channel(rgb, 0), 0x00CC3333, channel == 0);
    _graph_visuals_print_slider(GRAPH_VISUALS_SLIDER_ROW + 1, "绿", _graph_visuals_get_channel(rgb, 1), 0x0033AA33, channel == 1);
    _graph_visuals_print_slider(GRAPH_VISUALS_SLIDER_ROW + 2, "蓝", _graph_visuals_get_channel(rgb, 2), 0x003366CC, channel == 2);
}

static void print_visuals_menu(cptr choice_msg);

static void _graph_visuals_edit_fill_colors(bool *need_redraw)
{
    int item = 0;
    int channel = 0;

    while (1)
    {
        int i;
        u32b *rgb = _graph_visuals_fill_rgb(item);

        Term_clear();
        print_visuals_menu("修改图形色块颜色");

        _graph_visuals_print_fill_editor(item, channel);
        prt("↑↓选色块 w/p/m/q/t/f跳转 r/g/b选通道 ←→微调 d默认 ESC确认", 23, 0);

        i = inkey_special(TRUE);
        if (i == ESCAPE) break;

        switch (i)
        {
        case SKEY_UP:
        case 'k':
        case 'K':
            item = (item + GRAPH_VISUALS_FILL_COUNT - 1) % GRAPH_VISUALS_FILL_COUNT;
            break;
        case SKEY_DOWN:
        case 'j':
        case 'J':
            item = (item + 1) % GRAPH_VISUALS_FILL_COUNT;
            break;
        case 'w':
        case 'W':
            item = 0;
            break;
        case 'p':
        case 'P':
            item = 1;
            break;
        case 'm':
        case 'M':
            item = 2;
            break;
        case 'q':
        case 'Q':
            item = 3;
            break;
        case 't':
        case 'T':
            item = 4;
            break;
        case 'f':
        case 'F':
            item = 5;
            break;
        case 'r':
        case 'R':
            channel = 0;
            break;
        case 'g':
        case 'G':
            channel = 1;
            break;
        case 'b':
        case 'B':
            channel = 2;
            break;
        case SKEY_LEFT:
        case 'h':
        case 'H':
            _graph_visuals_step_channel(rgb, channel, -1);
            *need_redraw = TRUE;
            break;
        case SKEY_RIGHT:
        case 'l':
        case 'L':
            _graph_visuals_step_channel(rgb, channel, 1);
            *need_redraw = TRUE;
            break;
        case ',':
        case '<':
            _graph_visuals_step_channel(rgb, channel, -16);
            *need_redraw = TRUE;
            break;
        case '.':
        case '>':
            _graph_visuals_step_channel(rgb, channel, 16);
            *need_redraw = TRUE;
            break;
        case 'd':
        case 'D':
            _graph_visuals_reset_fill_colors();
            *need_redraw = TRUE;
            break;
        default:
            bell();
            break;
        }

    }
}

static void print_visuals_menu(cptr choice_msg)
{
    prt("外观设置", 1, 0);

    /* Give some choices */
    prt("(0) 读取用户 pref 文件", 3, 5);

#ifdef ALLOW_VISUALS
    prt("(1) 导出怪物颜色/字符", 4, 5);
    prt("(2) 导出物品颜色/字符", 5, 5);
    prt("(3) 导出地形颜色/字符", 6, 5);
    prt("(4) 修改怪物颜色/字符(数字)", 7, 5);
    prt("(5) 修改物品颜色/字符(数字)", 8, 5);
    prt("(6) 修改地形颜色/字符(数字)", 9, 5);
    prt("(7) 修改怪物颜色/字符(可视)", 10, 5);
    prt("(8) 修改物品颜色/字符(可视)", 11, 5);
    prt("(9) 修改地形颜色/字符(可视)", 12, 5);
    prt("(G) 修改图形色块颜色", 13, 5);

#endif /* ALLOW_VISUALS */

    prt("(R) 重置外观", 14, 5);

    /* Prompt */
    prt(format("命令: %s", choice_msg ? choice_msg : ""), 16, 0);
}

static void do_cmd_knowledge_monsters(bool *need_redraw, bool visual_only, int direct_r_idx);
static void do_cmd_knowledge_objects(bool *need_redraw, bool visual_only, int direct_k_idx);
static void do_cmd_knowledge_features(bool *need_redraw, bool visual_only, int direct_f_idx, int *lighting_level);

/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
    int i;
    char tmp[160];
    char buf[1024];
    bool need_redraw = FALSE;
    const char *empty_symbol = "<< ? >>";

    if (use_bigtile) empty_symbol = "<< ?? >>";

    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);

    /* Save the screen */
    screen_save();

    /* Interact until done */
    while (1)
    {
        /* Clear screen */
        Term_clear();

        /* Ask for a choice */
        print_visuals_menu(NULL);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        switch (i)
        {
        /* Load a 'pref' file */
        case '0':
            /* Prompt */
            prt("命令: 读取用户 pref 文件", 15, 0);

            /* Prompt */
            prt("文件: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Query */
            if (!askfor(tmp, 70)) continue;

            /* Process the given filename */
            (void)process_pref_file(tmp);

            need_redraw = TRUE;
            break;

#ifdef ALLOW_VISUALS

        /* Dump monster attr/chars */
        case '1':
        {
            static cptr mark = "怪物颜色/字符";

            /* Prompt */
            prt("命令: 转储怪物外观属性/字符(attr/chars)", 15, 0);

            /* Prompt */
            prt("文件: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Get a filename */
            if (!askfor(tmp, 70)) continue;

            /* Build the filename */
            path_build(buf, sizeof(buf), ANGBAND_DIR_USER, tmp);

            /* Append to the file */
            if (!open_auto_dump(buf, mark)) continue;

            /* Start dumping */
            auto_dump_printf("\n# 怪物颜色/字符定义\n\n");

            /* Dump monsters */
            for (i = 0; i < max_r_idx; i++)
            {
                monster_race *r_ptr = &r_info[i];

                /* Skip non-entries */
                if (!r_ptr->name) continue;

                /* Dump a comment */
                auto_dump_printf("# %s\n", (r_name + r_ptr->name));

                /* Dump the monster attr/char info */
                auto_dump_printf("R:%d:0x%02X/0x%02X\n\n", i,
                    (byte)(r_ptr->x_attr), (byte)(r_ptr->x_char));
            }

            /* Close */
            close_auto_dump();

            /* Message */
            msg_print("已导出怪物颜色/字符。");

            break;
        }

        /* Dump object attr/chars */
        case '2':
        {
            static cptr mark = "物品颜色/字符";

            /* Prompt */
            prt("命令: 转储物品外观属性/字符(attr/chars)", 15, 0);

            /* Prompt */
            prt("文件: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Get a filename */
            if (!askfor(tmp, 70)) continue;

            /* Build the filename */
            path_build(buf, sizeof(buf), ANGBAND_DIR_USER, tmp);

            /* Append to the file */
            if (!open_auto_dump(buf, mark)) continue;

            /* Start dumping */
            auto_dump_printf("\n# 物品颜色/字符定义\n\n");

            /* Dump objects */
            for (i = 0; i < max_k_idx; i++)
            {
                char o_name[80];
                object_kind *k_ptr = &k_info[i];

                /* Skip non-entries */
                if (!k_ptr->name) continue;

                if (!k_ptr->flavor)
                {
                    /* Tidy name */
                    strip_name(o_name, i);
                }
                else
                {
                    object_type forge;

                    /* Prepare dummy object */
                    object_prep(&forge, i);

                    /* Get un-shuffled flavor name */
                    object_desc(o_name, &forge, OD_FORCE_FLAVOR);
                }

                /* Dump a comment */
                auto_dump_printf("# %s\n", o_name);

                /* Dump the object attr/char info */
                auto_dump_printf("K:%d:%d:0x%02X/0x%02X\n\n",
                    k_ptr->tval, k_ptr->sval,
                    (byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
            }

            /* Close */
            close_auto_dump();

            /* Message */
            msg_print("已导出物品颜色/字符。");

            break;
        }

        /* Dump feature attr/chars */
        case '3':
        {
            static cptr mark = "地形特征颜色/字符";

            /* Prompt */
            prt("命令: 转储地形外观属性/字符(attr/chars)", 15, 0);

            /* Prompt */
            prt("文件: ", 17, 0);

            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Get a filename */
            if (!askfor(tmp, 70)) continue;

            /* Build the filename */
            path_build(buf, sizeof(buf), ANGBAND_DIR_USER, tmp);

            /* Append to the file */
            if (!open_auto_dump(buf, mark)) continue;

            /* Start dumping */
            auto_dump_printf("\n# 地形特征颜色/字符定义\n\n");

            /* Dump features */
            for (i = 0; i < max_f_idx; i++)
            {
                feature_type *f_ptr = &f_info[i];

                /* Skip non-entries */
                if (!f_ptr->name) continue;

                /* Skip mimiccing features */
                if (f_ptr->mimic != i) continue;

                /* Dump a comment */
                auto_dump_printf("# %s\n", (f_name + f_ptr->name));

                /* Dump the feature attr/char info */
                auto_dump_printf("F:%d:0x%02X/0x%02X:0x%02X/0x%02X:0x%02X/0x%02X\n\n", i,
                    (byte)(f_ptr->x_attr[F_LIT_STANDARD]), (byte)(f_ptr->x_char[F_LIT_STANDARD]),
                    (byte)(f_ptr->x_attr[F_LIT_LITE]), (byte)(f_ptr->x_char[F_LIT_LITE]),
                    (byte)(f_ptr->x_attr[F_LIT_DARK]), (byte)(f_ptr->x_char[F_LIT_DARK]));
            }

            auto_dump_printf("# 图形 ASCII 色块颜色\n");
            auto_dump_printf("G:WALL:0x%06lX\n", (unsigned long)(graph_wall_rgb & 0x00FFFFFF));
            auto_dump_printf("G:PERMAWALL:0x%06lX\n", (unsigned long)(graph_permawall_rgb & 0x00FFFFFF));
            auto_dump_printf("G:MAGMA:0x%06lX\n", (unsigned long)(graph_magma_wall_rgb & 0x00FFFFFF));
            auto_dump_printf("G:QUARTZ:0x%06lX\n", (unsigned long)(graph_quartz_wall_rgb & 0x00FFFFFF));
            auto_dump_printf("G:TREASURE:0x%06lX\n", (unsigned long)(graph_treasure_wall_rgb & 0x00FFFFFF));
            auto_dump_printf("G:FLOOR:0x%06lX\n\n", (unsigned long)(graph_floor_rgb & 0x00FFFFFF));

            /* Close */
            close_auto_dump();

            /* Message */
            msg_print("已导出地形颜色/字符。");

            break;
        }

        /* Modify monster attr/chars (numeric operation) */
        case '4':
        {
            static cptr choice_msg = "更改怪物颜色/字符";
            static int r = 0;

            prt(format("命令: %s", choice_msg), 15, 0);

            /* Hack -- query until done */
            while (1)
            {
                monster_race *r_ptr = &r_info[r];
                char c;
                int t;

                byte da = r_ptr->d_attr;
                byte dc = r_ptr->d_char;
                byte ca = r_ptr->x_attr;
                byte cc = r_ptr->x_char;

                /* Label the object */
                Term_putstr(5, 17, -1, TERM_WHITE,
                        format("怪物 = %d, 名称 = %-40.40s",
                           r, (r_name + r_ptr->name)));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                        format("默认颜色/字符 = %3u / %3u", da, dc));

                Term_putstr(40, 19, -1, TERM_WHITE, empty_symbol);
                Term_queue_bigchar(43, 19, da, dc, 0, 0);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                        format("当前颜色/字符 = %3u / %3u", ca, cc));

                Term_putstr(40, 20, -1, TERM_WHITE, empty_symbol);
                Term_queue_bigchar(43, 20, ca, cc, 0, 0);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                        "Command (n/N/^N/a/A/^A/c/C/^C/v/V/^V): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                if (iscntrl(i)) c = 'a' + i - KTRL('A');
                else if (isupper(i)) c = 'a' + i - 'A';
                else c = i;

                switch (c)
                {
                case 'n':
                    {
                        int prev_r = r;
                        do
                        {
                            if (!cmd_visuals_aux(i, &r, max_r_idx))
                            {
                                r = prev_r;
                                break;
                            }
                        }
                        while (!r_info[r].name);
                    }
                    break;
                case 'a':
                    t = (int)r_ptr->x_attr;
                    (void)cmd_visuals_aux(i, &t, 256);
                    r_ptr->x_attr = (byte)t;
                    need_redraw = TRUE;
                    break;
                case 'c':
                    t = (int)r_ptr->x_char;
                    (void)cmd_visuals_aux(i, &t, 256);
                    r_ptr->x_char = (byte)t;
                    need_redraw = TRUE;
                    break;
                case 'v':
                    do_cmd_knowledge_monsters(&need_redraw, TRUE, r);

                    /* Clear screen */
                    Term_clear();
                    print_visuals_menu(choice_msg);
                    break;
                }
            }

            break;
        }

        /* Modify object attr/chars (numeric operation) */
        case '5':
        {
            static cptr choice_msg = "更改物品颜色/字符";
            static int k = 0;

            prt(format("命令: %s", choice_msg), 15, 0);

            /* Hack -- query until done */
            while (1)
            {
                object_kind *k_ptr = &k_info[k];
                char c;
                int t;

                byte da = k_ptr->d_attr;
                byte dc = k_ptr->d_char;
                byte ca = k_ptr->x_attr;
                byte cc = k_ptr->x_char;

                /* Label the object */
                Term_putstr(5, 17, -1, TERM_WHITE,
                        format("物品 = %d, 名称 = %-40.40s",
                           k, k_name + (!k_ptr->flavor ? k_ptr->name : k_ptr->flavor_name)));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                        format("默认颜色/字符 = %3d / %3d", da, dc));

                Term_putstr(40, 19, -1, TERM_WHITE, empty_symbol);
                Term_queue_bigchar(43, 19, da, dc, 0, 0);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                        format("当前颜色/字符 = %3d / %3d", ca, cc));

                Term_putstr(40, 20, -1, TERM_WHITE, empty_symbol);
                Term_queue_bigchar(43, 20, ca, cc, 0, 0);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                        "Command (n/N/^N/a/A/^A/c/C/^C/v/V/^V): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                if (iscntrl(i)) c = 'a' + i - KTRL('A');
                else if (isupper(i)) c = 'a' + i - 'A';
                else c = i;

                switch (c)
                {
                case 'n':
                    {
                        int prev_k = k;
                        do
                        {
                            if (!cmd_visuals_aux(i, &k, max_k_idx))
                            {
                                k = prev_k;
                                break;
                            }
                        }
                        while (!k_info[k].name);
                    }
                    break;
                case 'a':
                    t = (int)k_ptr->x_attr;
                    (void)cmd_visuals_aux(i, &t, 256);
                    k_ptr->x_attr = (byte)t;
                    need_redraw = TRUE;
                    break;
                case 'c':
                    t = (int)k_ptr->x_char;
                    (void)cmd_visuals_aux(i, &t, 256);
                    k_ptr->x_char = (byte)t;
                    need_redraw = TRUE;
                    break;
                case 'v':
                    do_cmd_knowledge_objects(&need_redraw, TRUE, k);

                    /* Clear screen */
                    Term_clear();
                    print_visuals_menu(choice_msg);
                    break;
                }
            }

            break;
        }

        /* Modify feature attr/chars (numeric operation) */
        case '6':
        {
            static cptr choice_msg = "更改地形特征颜色/字符";
            static int f = 0;
            static int lighting_level = F_LIT_STANDARD;

            prt(format("命令: %s", choice_msg), 15, 0);

            /* Hack -- query until done */
            while (1)
            {
                feature_type *f_ptr = &f_info[f];
                char c;
                int t;

                byte da = f_ptr->d_attr[lighting_level];
                byte dc = f_ptr->d_char[lighting_level];
                byte ca = f_ptr->x_attr[lighting_level];
                byte cc = f_ptr->x_char[lighting_level];

                /* Label the object */
                prt("", 17, 5);
                Term_putstr(5, 17, -1, TERM_WHITE,
                        format("地形 = %d, 名称 = %s, 光照 = %s",
                           f, (f_name + f_ptr->name), lighting_level_str[lighting_level]));

                /* Label the Default values */
                Term_putstr(10, 19, -1, TERM_WHITE,
                        format("默认颜色/字符 = %3d / %3d", da, dc));

                Term_putstr(40, 19, -1, TERM_WHITE, empty_symbol);

                Term_queue_bigchar(43, 19, da, dc, 0, 0);

                /* Label the Current values */
                Term_putstr(10, 20, -1, TERM_WHITE,
                        format("当前颜色/字符 = %3d / %3d", ca, cc));

                Term_putstr(40, 20, -1, TERM_WHITE, empty_symbol);
                Term_queue_bigchar(43, 20, ca, cc, 0, 0);

                /* Prompt */
                Term_putstr(0, 22, -1, TERM_WHITE,
                        "Command (n/N/^N/a/A/^A/c/C/^C/l/L/^L/d/D/^D/v/V/^V): ");

                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                if (iscntrl(i)) c = 'a' + i - KTRL('A');
                else if (isupper(i)) c = 'a' + i - 'A';
                else c = i;

                switch (c)
                {
                case 'n':
                    {
                        int prev_f = f;
                        do
                        {
                            if (!cmd_visuals_aux(i, &f, max_f_idx))
                            {
                                f = prev_f;
                                break;
                            }
                        }
                        while (!f_info[f].name || (f_info[f].mimic != f));
                    }
                    break;
                case 'a':
                    t = (int)f_ptr->x_attr[lighting_level];
                    (void)cmd_visuals_aux(i, &t, 256);
                    f_ptr->x_attr[lighting_level] = (byte)t;
                    need_redraw = TRUE;
                    break;
                case 'c':
                    {
                        byte old_char = f_ptr->x_char[lighting_level];

                        t = (int)f_ptr->x_char[lighting_level];
                        (void)cmd_visuals_aux(i, &t, 256);
                        f_ptr->x_char[lighting_level] = (byte)t;
                        if (lighting_level == F_LIT_STANDARD)
                            _sync_feature_lighting_chars(f_ptr, old_char, f_ptr->x_char[lighting_level]);
                        need_redraw = TRUE;
                    }
                    break;
                case 'l':
                    (void)cmd_visuals_aux(i, &lighting_level, F_LIT_MAX);
                    break;
                case 'd':
                    apply_default_feat_lighting(f_ptr->x_attr, f_ptr->x_char);
                    need_redraw = TRUE;
                    break;
                case 'v':
                    do_cmd_knowledge_features(&need_redraw, TRUE, f, &lighting_level);

                    /* Clear screen */
                    Term_clear();
                    print_visuals_menu(choice_msg);
                    break;
                }
            }

            break;
        }

        /* Modify monster attr/chars (visual mode) */
        case '7':
            do_cmd_knowledge_monsters(&need_redraw, TRUE, -1);
            break;

        /* Modify object attr/chars (visual mode) */
        case '8':
            do_cmd_knowledge_objects(&need_redraw, TRUE, -1);
            break;

        /* Modify feature attr/chars (visual mode) */
        case '9':
        {
            int lighting_level = F_LIT_STANDARD;
            do_cmd_knowledge_features(&need_redraw, TRUE, -1, &lighting_level);
            break;
        }

        /* Modify graph ASCII fill colors */
        case 'G':
        case 'g':
            _graph_visuals_edit_fill_colors(&need_redraw);
            break;

#endif /* ALLOW_VISUALS */

        /* Reset visuals */
        case 'R':
        case 'r':
            /* Reset */
            reset_visuals();

            /* Message */
            msg_print("外观颜色/字符表已重置。");

            need_redraw = TRUE;
            break;

        /* Unknown option */
        default:
            bell();
            break;
        }

        /* Flush messages */
        msg_print(NULL);
    }

    /* Restore the screen */
    screen_load();

    if (need_redraw) do_cmd_redraw();
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
    int i;

    char tmp[160];

    char buf[1024];


    /* File type is "TEXT" */
    FILE_TYPE(FILE_TYPE_TEXT);


    /* Save the screen */
    screen_save();


    /* Interact until done */
    while (1)
    {
        /* Clear screen */
        Term_clear();

        /* Ask for a choice */
        prt("颜色设置", 2, 0);


        /* Give some choices */
        prt("(1) 读取用户 pref 文件", 4, 5);

#ifdef ALLOW_COLORS
        prt("(2) 导出颜色", 5, 5);
        prt("(3) 修改颜色", 6, 5);
        prt("(4) 读取简易配色", 7, 5);
        prt("(5) 读取 Windows 配色", 8, 5);

#endif

        /* Prompt */
        prt("命令: ", 10, 0);


        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;

        /* Load a 'pref' file */
        if (i == '1')
        {
            /* Prompt */
            prt("命令: 读取用户 pref 文件", 10, 0);


            /* Prompt */
            prt("文件: ", 12, 0);


            /* Default file */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Query */
            if (!askfor(tmp, 70)) continue;

            /* Process the given filename */
            (void)process_pref_file(tmp);

            /* Mega-Hack -- react to changes */
            Term_xtra(TERM_XTRA_REACT, 0);

            /* Mega-Hack -- redraw */
            Term_redraw();
        }

#ifdef ALLOW_COLORS

        /* Dump colors */
        else if (i == '2')
        {
            static cptr mark = "颜色";

            /* Prompt */
            prt("命令: 转储颜色(Dump colors)", 10, 0);


            /* Prompt */
            prt("文件: ", 12, 0);


            /* Default filename */
            sprintf(tmp, "%s.prf", pref_save_base);

            /* Get a filename */
            if (!askfor(tmp, 70)) continue;

            /* Build the filename */
            path_build(buf, sizeof(buf), ANGBAND_DIR_USER, tmp);

            /* Append to the file */
            if (!open_auto_dump(buf, mark)) continue;

            /* Start dumping */
            auto_dump_printf("\n# 颜色重新定义\n\n");

            /* Dump colors */
            for (i = 0; i < 256; i++)
            {
                int kv = angband_color_table[i][0];
                int rv = angband_color_table[i][1];
                int gv = angband_color_table[i][2];
                int bv = angband_color_table[i][3];

                cptr name = "未知";


                /* Skip non-entries */
                if (!kv && !rv && !gv && !bv) continue;

                /* Extract the color name */
                if (i < MAX_COLOR) name = color_names[i];

                /* Dump a comment */
                auto_dump_printf("# 颜色 '%s'\n", name);

                /* Dump the monster attr/char info */
                auto_dump_printf("V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
                    i, kv, rv, gv, bv);
            }

            /* Close */
            close_auto_dump();

            /* Message */
            msg_print("已导出颜色重定义。");

        }

        /* Edit colors */
        else if (i == '3')
        {
            static byte a = 0;

            /* Prompt */
            prt("命令: 修改颜色(Modify colors)", 10, 0);


            /* Hack -- query until done */
            while (1)
            {
                cptr name;
                byte j;

                /* Clear */
                clear_from(10);

                /* Exhibit the normal colors */
                for (j = 0; j < 16; j++)
                {
                    /* Exhibit this color */
                    Term_putstr(j*4, 19, -1, a, "###");

                    /* Exhibit all colors */
                    Term_putstr(j*4, 20, -1, j, format("%3d", j));
                }
                if (MAX_COLOR > 16)
                {
                    for (j = 0; j < MAX_COLOR - 16; j++)
                    {
                        /* Exhibit this color */
                        Term_putstr(j*4, 21, -1, a, "###");

                        /* Exhibit all colors */
                        Term_putstr(j*4, 22, -1, j + 16, format("%3d", j + 16));
                    }
                }

                /* Describe the color */
                name = ((a < MAX_COLOR) ? color_names[a] : "未定义");


                /* Describe the color */
                Term_putstr(5, 12, -1, TERM_WHITE,
                        format("颜色 = %d, 名称 = %s", a, name));


                /* Label the Current values */
                Term_putstr(5, 14, -1, TERM_WHITE,
                        format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
                           angband_color_table[a][0],
                           angband_color_table[a][1],
                           angband_color_table[a][2],
                           angband_color_table[a][3]));

                /* Prompt */
                Term_putstr(0, 16, -1, TERM_WHITE,
                        "Command (n/N/k/K/r/R/g/G/b/B): ");


                /* Get a command */
                i = inkey();

                /* All done */
                if (i == ESCAPE) break;

                /* Analyze */
                if (i == 'n') a = (byte)(a + 1);
                if (i == 'N') a = (byte)(a - 1);
                if (i == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
                if (i == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
                if (i == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
                if (i == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
                if (i == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
                if (i == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
                if (i == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
                if (i == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

                /* Hack -- react to changes */
                Term_xtra(TERM_XTRA_REACT, 0);

                /* Hack -- redraw */
                Term_redraw();
            }
        }

        else if (i == '4')
        {
            if (process_pref_file("user-lim.prf")) msg_print("完成。");
            Term_xtra(TERM_XTRA_REACT, 0);
            Term_redraw();
        }

        else if (i == '5')
        {
            if (process_pref_file("user-win.prf")) msg_print("完成。");
            Term_xtra(TERM_XTRA_REACT, 0);
            Term_redraw();
        }

#endif

        /* Unknown option */
        else
        {
            bell();
        }

        /* Flush messages */
        msg_print(NULL);
    }


    /* Restore the screen */
    screen_load();
}

void msg_add_tiny_screenshot(int cx, int cy)
{
    if (!statistics_hack)
    {
        string_ptr s = get_tiny_screenshot(cx, cy);
        msg_add(string_buffer(s));
        string_free(s);
    }
}

string_ptr get_tiny_screenshot(int cx, int cy)
{
    string_ptr s = string_alloc_size(cx * cy);
    bool       old_use_graphics = use_graphics;
    int        y1, y2, x1, x2, y, x;

    y1 = py - cy/2;
    y2 = py + cy/2;
    if (y1 < 0) y1 = 0;
    if (y2 > cur_hgt) y2 = cur_hgt;

    x1 = px - cx/2;
    x2 = px + cx/2;
    if (x1 < 0) x1 = 0;
    if (x2 > cur_wid) x2 = cur_wid;

    if (old_use_graphics)
    {
        use_graphics = FALSE;
        reset_visuals();
    }

    for (y = y1; y < y2; y++)
    {
        int  current_a = -1;
        for (x = x1; x < x2; x++)
        {
            byte a, ta;
            char c, tc;

            assert(in_bounds2(y, x));
            map_info(y, x, &a, &c, &ta, &tc);

            if (c == 127) /* Hack for special wall characters on Windows. See font-win.prf and main-win.c */
                c = '#';

            if (a != current_a)
            {
                if (current_a >= 0 && current_a != TERM_WHITE)
                {
                    string_append_s(s, "</color>");
                }
                if (a != TERM_WHITE)
                {
                    string_printf(s, "<color:%c>", attr_to_attr_char(a));
                }
                current_a = a;
            }
            string_append_c(s, c);
        }
        if (current_a >= 0 && current_a != TERM_WHITE)
            string_append_s(s, "</color>");
        string_append_c(s, '\n');
    }
    if (old_use_graphics)
    {
        use_graphics = TRUE;
        reset_visuals();
    }
    return s;
}

/* Note: This will not work if the screen is "icky" */
string_ptr get_screenshot(void)
{
    string_ptr s = string_alloc_size(80 * 27);
    bool       old_use_graphics = use_graphics;
    int        wid, hgt, x, y;

    Term_get_size(&wid, &hgt);

    if (old_use_graphics)
    {
        use_graphics = FALSE;
        reset_visuals();

        p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY | PR_MSG_LINE);
        redraw_stuff();
    }

    for (y = 0; y < hgt; y++)
    {
        int  current_a = -1;
        for (x = 0; x < wid; x++)
        {
            byte a;
            char c;

            Term_what(x, y, &a, &c);

            if (c == 127) /* Hack for special wall characters on Windows. See font-win.prf and main-win.c */
                c = '#';

            if (a != current_a)
            {
                if (current_a >= 0 && current_a != TERM_WHITE)
                {
                    string_append_s(s, "</color>");
                }
                if (a != TERM_WHITE)
                {
                    string_printf(s, "<color:%c>", attr_to_attr_char(a));
                }
                current_a = a;
            }
            string_append_c(s, c);
        }
        if (current_a >= 0 && current_a != TERM_WHITE)
            string_append_s(s, "</color>");
        string_append_c(s, '\n');
    }
    if (old_use_graphics)
    {
        use_graphics = TRUE;
        reset_visuals();

        p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY | PR_MSG_LINE);
        redraw_stuff();
    }
    return s;
}

/*
 * Note something in the message recall
 */
void do_cmd_note(void)
{
    char buf[80];
    string_ptr s = 0;

    /* Default */
    strcpy(buf, "");

    /* Input */
    if (!get_string("笔记:", buf, 60)) return;

    /* Ignore empty notes */
    if (!buf[0] || (buf[0] == ' ')) return;

    /* Add the note to the message recall */
    msg_format("<color:R>笔记:</color> %s\n", buf);

    s = get_tiny_screenshot(50, 24);
    msg_add(string_buffer(s));
    string_free(s);
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
    cptr xtra = "";
    if (VER_MINOR == 0)
    {
/*        if (VER_PATCH == 0) xtra = " (Alpha)"; */
        if (VER_MAJOR != 7) xtra = " (Beta)";
    }
    msg_format("你正在游玩 <color:B>FrogNet</color> <color:r>%s%s</color>。",
        VERSION_STRING, xtra);
    if (1)
    {
        rect_t r = ui_map_rect();
        msg_format("地图显示尺寸为 %dx%d。", r.cx, r.cy);
    }
}



/*
 * Array of feeling strings
 */
struct _feeling_info_s
{
    byte color;
    cptr msg;
};
typedef struct _feeling_info_s _feeling_info_t;
static _feeling_info_t _level_feelings[11] =
{
    {TERM_SLATE, "看起来和其它楼层没什么两样。"},
    {TERM_L_BLUE, "你觉得这层有一些特别之处。"},
    {TERM_VIOLET, "死亡的可怕景象充斥着你的大脑，你差点晕倒！"},
    {TERM_RED, "这层看起来非常危险。"},
    {TERM_L_RED, "你有一种非常不祥的预感……"},
    {TERM_ORANGE, "你有一种不祥的预感……"},
    {TERM_YELLOW, "你感到紧张。"},
    {TERM_L_UMBER, "你感觉你的运气正在好转……"},
    {TERM_L_WHITE, "你不喜欢这个地方的样子。"},
    {TERM_WHITE, "这层看起来相当安全。"},
    {TERM_WHITE, "真是个无聊的地方……"},
};

static _feeling_info_t _level_feelings_lucky[11] =
{
    {TERM_SLATE, "看起来和其它楼层没什么两样。"},
    {TERM_L_BLUE, "你觉得这层有一些特别之处。"},
    {TERM_VIOLET, "你对这层有一种绝佳的感觉。"},
    {TERM_RED, "你有一种极好的感觉……"},
    {TERM_L_RED, "你有一种非常好的感觉……"},
    {TERM_ORANGE, "你有一种不错的感觉……"},
    {TERM_YELLOW, "你感到出奇的幸运……"},
    {TERM_L_UMBER, "你感觉你的运气正在好转……"},
    {TERM_L_WHITE, "你喜欢这个地方的样子……"},
    {TERM_WHITE, "这层总不至于太糟……"},
    {TERM_WHITE, "真是个无聊的地方……"},
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
    /* No useful feeling in quests */
    if (!quests_allow_feeling())
    {
        msg_print("看起来像个典型的任务层。");
    }

    /* No useful feeling in town */
    else if (p_ptr->town_num && !dun_level)
    {
        if (p_ptr->town_num == TOWN_RANDOM)
        {
            msg_print("看起来像是一片奇怪的荒野。");
        }
        else
        {
            msg_print("看起来像个典型的城镇。");
        }
    }

    /* No useful feeling in the wilderness */
    else if (!dun_level)
    {
        msg_print("看起来像个典型的荒野。");
    }

    /* Display the feeling */
    else
    {
        _feeling_info_t feeling;
        assert(/*0 <= p_ptr->feeling &&*/ p_ptr->feeling < 11);
        if (p_ptr->good_luck || p_ptr->pclass == CLASS_ARCHAEOLOGIST)
            feeling = _level_feelings_lucky[p_ptr->feeling];
        else
            feeling = _level_feelings[p_ptr->feeling];
        cmsg_print(feeling.color, feeling.msg);
    }
}



/*
 * Description of each monster group.
 */
static cptr monster_group_text[] =
{
    "尸体",
    "唯一怪物",
    "可骑乘怪物",
    "通缉怪物",
    "地下城守卫",
    "安珀皇族",
    "神祗",
    "蚂蚁",
    "蝙蝠",
    "蜈蚣",
    "龙",
    "漂浮眼",
    "猫科/狐狸",
    "魔像",
    "霍比特人/精灵/矮人",
    "恶心之物",
    "果冻怪",
    "狗头人",
    "水生怪物",
    "霉菌",
    "娜迦",
    "兽人",
    "人类",
    "四足动物",
    "啮齿动物",
    "骷髅",
    "恶魔",
    "漩涡",
    "蠕虫/蠕虫群",
    /* "unused", */
    "伊克",
    "僵尸/木乃伊",
    "天使",
    "鸟类",
    "犬科",
    /* "Ancient Dragon/Wyrm", */
    "元素",
    "龙蝇",
    "幽灵",
    "混血生物",
    "昆虫",
    "蛇",
    "杀人甲虫",
    "巫妖",
    "多头爬行动物",
    "神秘活物",
    "食人魔",
    "巨大人形生物",
    "脉动肉块",
    "爬行/两栖动物",
    "蜘蛛/蝎子/蜱虫",
    "巨魔",
    /* "Major Demon", */
    "吸血鬼",
    "尸妖/幽灵等",
    "索恩/萨伦等",
    "雪人",
    "微风猎犬",
    "宝箱怪",
    "墙壁/植物/气体",
    "蘑菇丛",
    "球体",
    "玩家",
    NULL
};


/*
 * Symbols of monsters in each group. Note the "Uniques" group
 * is handled differently.
 */
static cptr monster_group_char[] =
{
    (char *) -1L,
    (char *) -2L,
    (char *) -3L,
    (char *) -4L,
    (char *) -5L,
    (char *) -6L,
    (char *) -7L,
    "a",
    "b",
    "c",
    "dD",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "pt",
    "q",
    "r",
    "s",
    "uU",
    "v",
    "w",
    /* "x", */
    "y",
    "z",
    "A",
    "B",
    "C",
    /* "D", */
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    /* "U", */
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "!$&()+./=>?[\\]`{|~x",
    "#%",
    ",",
    "*",
    "@",
    NULL
};


/*
 * hook function to sort monsters by level
 */
static bool ang_sort_comp_monster_level(vptr u, vptr v, int a, int b)
{
    u16b *who = (u16b*)(u);

    int w1 = who[a];
    int w2 = who[b];

    monster_race *r_ptr1 = &r_info[w1];
    monster_race *r_ptr2 = &r_info[w2];

    /* Unused */
    (void)v;

    if (r_ptr2->level > r_ptr1->level) return FALSE;
    if (r_ptr1->level > r_ptr2->level) return TRUE;

    if ((r_ptr2->flags1 & RF1_UNIQUE) && !(r_ptr1->flags1 & RF1_UNIQUE)) return TRUE;
    if ((r_ptr1->flags1 & RF1_UNIQUE) && !(r_ptr2->flags1 & RF1_UNIQUE)) return FALSE;
    return w1 <= w2;
}

/*
 * Build a list of monster indexes in the given group. Return the number
 * of monsters in the group.
 *
 * mode & 0x01 : check for non-empty group
 * mode & 0x02 : visual operation only
 */
static int collect_monsters(int grp_cur, s16b mon_idx[], byte mode)
{
    int i, mon_cnt = 0;
    int dummy_why;

    /* Get a list of x_char in this group */
    cptr group_char = monster_group_char[grp_cur];

    /* XXX Hack -- Check for special groups */
    bool        grp_corpses = (monster_group_char[grp_cur] == (char *) -1L);
    bool        grp_unique = (monster_group_char[grp_cur] == (char *) -2L);
    bool        grp_riding = (monster_group_char[grp_cur] == (char *) -3L);
    bool        grp_wanted = (monster_group_char[grp_cur] == (char *) -4L);
    bool        grp_guardian = (monster_group_char[grp_cur] == (char *) -5L);
    bool        grp_amberite = (monster_group_char[grp_cur] == (char *) -6L);
    bool        grp_god = (monster_group_char[grp_cur] == (char *) -7L);
    int_map_ptr available_corpses = NULL;

    if (grp_corpses)
    {
        available_corpses = int_map_alloc(NULL);

        /* In Pack */
        for (i = 1; i <= pack_max(); i++)
        {
            object_type *o_ptr = pack_obj(i);
            if (!o_ptr) continue;
            if (!object_is_(o_ptr, TV_CORPSE, SV_CORPSE)) continue;
            int_map_add(available_corpses, o_ptr->pval, NULL);
        }

        /* At Home */
        for (i = 1; i <= home_max(); i++)
        {
            object_type *o_ptr = home_obj(i);
            if (!o_ptr) continue;
            if (!object_is_(o_ptr, TV_CORPSE, SV_CORPSE)) continue;
            int_map_add(available_corpses, o_ptr->pval, NULL);
        }

        /* Underfoot */
        if (in_bounds2(py, px))
        {
            cave_type  *c_ptr = &cave[py][px];
            s16b        o_idx = c_ptr->o_idx;

            while (o_idx)
            {
                object_type *o_ptr = &o_list[o_idx];

                if (object_is_(o_ptr, TV_CORPSE, SV_CORPSE))
                    int_map_add(available_corpses, o_ptr->pval, NULL);

                o_idx = o_ptr->next_o_idx;
            }
        }

        /* Current Form for Easier Comparisons */
        if (p_ptr->prace == RACE_MON_POSSESSOR && p_ptr->current_r_idx != MON_POSSESSOR_SOUL)
            int_map_add(available_corpses, p_ptr->current_r_idx, NULL);

    }


    /* Check every race */
    for (i = 1; i < max_r_idx; i++)
    {
        /* Access the race */
        monster_race *r_ptr = &r_info[i];

        /* Skip empty race */
        if (!r_ptr->name) continue;
        if (!p_ptr->wizard && (r_ptr->flagsx & RFX_SUPPRESS)) continue;

        /* Require known monsters */
        if (!(mode & 0x02) && !easy_lore && !r_ptr->r_sights) continue;

        if (grp_corpses)
        {
            if (!int_map_contains(available_corpses, i))
                continue;
        }

        else if (grp_unique)
        {
            if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
        }

        else if (grp_riding)
        {
            if (!(r_ptr->flags7 & RF7_RIDING)) continue;
        }

        else if (grp_wanted)
        {
            bool wanted = FALSE;
            int j;
            for (j = 0; j < MAX_KUBI; j++)
            {
                if (kubi_r_idx[j] == i || kubi_r_idx[j] - 10000 == i ||
                    (p_ptr->today_mon && p_ptr->today_mon == i))
                {
                    wanted = TRUE;
                    break;
                }
            }
            if (!wanted) continue;
        }

        else if (grp_amberite)
        {
            if (!(r_ptr->flags3 & RF3_AMBERITE)) continue;
        }

        else if (grp_god)
        {
            if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
            if (!monster_pantheon(r_ptr)) continue;
        }

        else if (grp_guardian)
        {
            if (!(r_ptr->flags7 & RF7_GUARDIAN)) continue;
            if ((d_info[DUNGEON_MYSTERY].final_guardian == i) &&
                (!(d_info[DUNGEON_MYSTERY].flags1 & DF1_SUPPRESSED)) &&
                (d_info[DUNGEON_MYSTERY].maxdepth > max_dlv[DUNGEON_MYSTERY])) continue;
        }

        else
        {
            /* Check for race in the group */
            if (!my_strchr(group_char, r_ptr->d_char)) continue;
        }

        /* Add the race */
        mon_idx[mon_cnt++] = i;

        /* XXX Hack -- Just checking for non-empty group */
        if (mode & 0x01) break;
    }

    /* Terminate the list */
    mon_idx[mon_cnt] = -1;

    /* Select the sort method */
    ang_sort_comp = ang_sort_comp_monster_level;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort by monster level */
    ang_sort(mon_idx, &dummy_why, mon_cnt);

    if (grp_corpses)
        int_map_free(available_corpses);

    /* Return the number of races */
    return mon_cnt;
}


/*
 * Description of each object group.
 */
static cptr object_group_text[] =
{
    "食物",
    "药水",
/*  "Flasks", */
    "卷轴",
/*  "Rings",
    "Amulets", */
/*  "Whistle",
    "Lanterns", */
/*  "Wands",
    "Staves",
    "Rods", */
/*  "Cards",
    "Capture Balls",
    "Parchments",
    "Spikes",
    "Boxs",
    "Figurines",
    "Statues",
    "Junks",
    "Bottles",
    "Skeletons",
    "Corpses", */
    "剑类",
    "钝器",
    "长柄武器",
    "挖掘工具",
    "弓类",
    "弹丸",
    "箭矢",
    "弩箭",
    "软甲",
    "硬甲",
    "龙鳞甲",
    "盾牌",
    "披风",
    "手套",
    "头盔",
    "皇冠",
    "鞋子",
    "法术书",
/*  "Treasure", */
    "某物",
    NULL
};


/*
 * TVALs of items in each group
 */
static byte object_group_tval[] =
{
    TV_FOOD,
    TV_POTION,
/*  TV_FLASK, */
    TV_SCROLL,
/*  TV_RING,
    TV_AMULET, */
/*  TV_WHISTLE,
    TV_LITE, */
/*  TV_WAND,
    TV_STAFF,
    TV_ROD,  */
/*  TV_CARD,
    TV_CAPTURE,
    TV_PARCHMENT,
    TV_SPIKE,
    TV_CHEST,
    TV_FIGURINE,
    TV_STATUE,
    TV_JUNK,
    TV_BOTTLE,
    TV_SKELETON,
    TV_CORPSE, */
    TV_SWORD,
    TV_HAFTED,
    TV_POLEARM,
    TV_DIGGING,
    TV_BOW,
    TV_SHOT,
    TV_ARROW,
    TV_BOLT,
    TV_SOFT_ARMOR,
    TV_HARD_ARMOR,
    TV_DRAG_ARMOR,
    TV_SHIELD,
    TV_CLOAK,
    TV_GLOVES,
    TV_HELM,
    TV_CROWN,
    TV_BOOTS,
    TV_LIFE_BOOK, /* Hack -- all spellbooks */
/*  TV_GOLD, */
    0,
    0,
};

static bool _compare_k_level(vptr u, vptr v, int a, int b)
{
    int *indices = (int*)u;
    int left = indices[a];
    int right = indices[b];
    return k_info[left].level <= k_info[right].level;
}

static void _swap_int(vptr u, vptr v, int a, int b)
{
    int *indices = (int*)u;
    int tmp = indices[a];
    indices[a] = indices[b];
    indices[b] = tmp;
}

/*
 * Build a list of object indexes in the given group. Return the number
 * of objects in the group.
 *
 * mode & 0x01 : check for non-empty group
 * mode & 0x02 : visual operation only
 */
static int collect_objects(int grp_cur, int object_idx[], byte mode)
{
    int i, j, k, object_cnt = 0;

    /* Get a list of x_char in this group */
    byte group_tval = object_group_tval[grp_cur];

    /* Check every object */
    for (i = 0; i < max_k_idx; i++)
    {
        /* Access the object */
        object_kind *k_ptr = &k_info[i];

        /* Skip empty objects */
        if (!k_ptr->name) continue;

        if (mode & 0x02)
        {
            /* Any objects will be displayed */
        }
        else
        {
            if (!k_ptr->flavor)
            {
                if (!k_ptr->counts.found && !k_ptr->counts.bought) continue;
            }

            /* Require objects ever seen */
            if (!k_ptr->aware) continue;

            /* Skip items with no distribution (special artifacts) */
            for (j = 0, k = 0; j < 4; j++) k += k_ptr->chance[j];
            if (!k) continue;
        }

        /* Check for objects in the group */
        if (TV_LIFE_BOOK == group_tval)
        {
            /* Hack -- All spell books */
            if (TV_BOOK_BEGIN <= k_ptr->tval && k_ptr->tval <= TV_BOOK_END)
            {
                /* Add the object */
                object_idx[object_cnt++] = i;
            }
            else continue;
        }
        else if (k_ptr->tval == group_tval)
        {
            /* Add the object */
            object_idx[object_cnt++] = i;
        }
        else continue;

        /* XXX Hack -- Just checking for non-empty group */
        if (mode & 0x01) break;
    }

    /* Sort Results */
    ang_sort_comp = _compare_k_level;
    ang_sort_swap = _swap_int;
    ang_sort(object_idx, NULL, object_cnt);

    /* Terminate the list */
    object_idx[object_cnt] = -1;

    /* Return the number of objects */
    return object_cnt;
}


/*
 * Description of each feature group.
 */
static cptr feature_group_text[] =
{
    "地形",
    NULL
};


/*
 * Build a list of feature indexes in the given group. Return the number
 * of features in the group.
 *
 * mode & 0x01 : check for non-empty group
 */
static int collect_features(int grp_cur, int *feat_idx, byte mode)
{
    int i, feat_cnt = 0;

    /* Unused;  There is a single group. */
    (void)grp_cur;

    /* Check every feature */
    for (i = 0; i < max_f_idx; i++)
    {
        /* Access the index */
        feature_type *f_ptr = &f_info[i];

        /* Skip empty index */
        if (!f_ptr->name) continue;

        /* Skip mimiccing features */
        if (f_ptr->mimic != i) continue;

        /* Add the index */
        feat_idx[feat_cnt++] = i;

        /* XXX Hack -- Just checking for non-empty group */
        if (mode & 0x01) break;
    }

    /* Terminate the list */
    feat_idx[feat_cnt] = -1;

    /* Return the number of races */
    return feat_cnt;
}

void do_cmd_save_screen_doc(void)
{
    string_ptr s = get_screenshot();
    char       buf[1024];
    FILE      *fff;

    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "screen.doc");
    FILE_TYPE(FILE_TYPE_TEXT);
    fff = my_fopen(buf, "w");
    if (fff)
    {
        string_write_file(s, fff);
        my_fclose(fff);
    }
    string_free(s);
}

void save_screen_aux(cptr file, int format)
{
    string_ptr s = get_screenshot();
    doc_ptr    doc = doc_alloc(Term->wid);
    FILE      *fff;

    doc_insert(doc, "<style:screenshot>");
    doc_insert(doc, string_buffer(s));
    doc_insert(doc, "</style>");

    FILE_TYPE(FILE_TYPE_TEXT);
    fff = my_fopen(file, "w");
    if (fff)
    {
        doc_write_file(doc, fff, format);
        my_fclose(fff);
    }
    string_free(s);
    doc_free(doc);
}

static void _save_screen_aux(int format)
{
    char buf[1024];

    if (format == DOC_FORMAT_HTML)
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "screen.html");
    else
        path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "screen.txt");

    save_screen_aux(buf, format);
}

void do_cmd_save_screen_txt(void)
{
    _save_screen_aux(DOC_FORMAT_TEXT);
}

void do_cmd_save_screen_html(void)
{
    _save_screen_aux(DOC_FORMAT_HTML);
}

void do_cmd_save_screen(void)
{
    string_ptr s = get_screenshot();
    doc_ptr    doc = doc_alloc(Term->wid);

    doc_insert(doc, "<style:screenshot>");
    doc_insert(doc, string_buffer(s));
    doc_insert(doc, "</style>");
    screen_save();
    doc_display(doc, "当前截图", 0);
    screen_load();

    string_free(s);
    doc_free(doc);
}

/************************************************************************
 * Artifact Lore (Standard Arts Only)
 * Note: Check out the Wizard Spoiler Commands for an alternative approach.
 *       ^a"a and ^a"O
 ************************************************************************/
typedef struct {
    object_p filter;
    cptr     name;
} _art_type_t;

static _art_type_t _art_types[] = {
    { object_is_melee_weapon, "武器" },
    { object_is_shield, "盾牌" },
    { object_is_bow, "弓类" },
    { object_is_ring, "戒指" },
    { object_is_amulet, "护身符" },
    { object_is_lite, "光源" },
    { object_is_body_armour, "躯干护甲" },
    { object_is_cloak, "披风" },
    { object_is_helmet, "头盔" },
    { object_is_gloves, "手套" },
    { object_is_boots, "鞋子" },
    { object_is_ammo, "弹药" },
    { NULL, NULL },
};

static bool _compare_a_level(vptr u, vptr v, int a, int b)
{
    int *indices = (int*)u;
    int left = indices[a];
    int right = indices[b];
    return a_info[left].level <= a_info[right].level;
}

static int _collect_arts(int grp_cur, int art_idx[], bool show_all)
{
    int i, cnt = 0;

    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];
        object_type    forge;

        if (!a_ptr->name) continue;
        if (!a_ptr->found)
        {
            if (!show_all) continue;
            /*if (!a_ptr->generated) continue;*/
            if (!art_has_lore(a_ptr)) continue;
        }
        if (!create_named_art_aux_aux(i, &forge)) continue;
        if (!_art_types[grp_cur].filter(&forge)) continue;

        art_idx[cnt++] = i;
    }

    /* Sort Results */
    ang_sort_comp = _compare_a_level;
    ang_sort_swap = _swap_int;
    ang_sort(art_idx, NULL, cnt);

    /* Terminate the list */
    art_idx[cnt] = -1;

    return cnt;
}


static void do_cmd_knowledge_artifacts(void)
{
    static bool show_all = TRUE;

    int i, len, max;
    int grp_cur, grp_top, old_grp_cur;
    int art_cur, art_top;
    int grp_cnt, grp_idx[100];
    int art_cnt;
    int *art_idx;

    int column = 0;
    bool flag;
    bool redraw;
    bool rebuild;

    int browser_rows;
    int wid, hgt;

    if (random_artifacts)
    {
        /* FIXED_ART ... 
        if (random_artifact_pct >= 100)
        {
            cmsg_print(TERM_L_RED, "You won't find any fixed artifacts this game.");
            return;
        }
        */
    }
    else if (no_artifacts)
    {
        cmsg_print(TERM_L_RED, "你在这场游戏中找不到任何神器。");
        return;
    }

    /* Get size */
    Term_get_size(&wid, &hgt);

    browser_rows = hgt - 8;

    C_MAKE(art_idx, max_a_idx, int);

    max = 0;
    grp_cnt = 0;
    for (i = 0; _art_types[i].filter; i++)
    {
        len = strlen(_art_types[i].name);
        if (len > max)
            max = len;

        if (_collect_arts(i, art_idx, TRUE))
            grp_idx[grp_cnt++] = i;
    }
    grp_idx[grp_cnt] = -1;

    if (!grp_cnt)
    {
        prt("你还没有找到任何神器。按任意键继续。", 0, 0);
        inkey();
        prt("", 0, 0);
        C_KILL(art_idx, max_a_idx, int);
        return;
    }

    art_cnt = 0;

    old_grp_cur = -1;
    grp_cur = grp_top = 0;
    art_cur = art_top = 0;

    flag = FALSE;
    redraw = TRUE;
    rebuild = TRUE;

    while (!flag)
    {
        char ch;
        if (redraw)
        {
            clear_from(0);

            prt(format("%s - 神器", "知识"), 2, 0);
            prt("分组", 4, 0);
            prt("名称", 4, max + 3);

            for (i = 0; i < 72; i++)
            {
                Term_putch(i, 5, TERM_WHITE, '=');
            }

            for (i = 0; i < browser_rows; i++)
            {
                Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
            }

            redraw = FALSE;
        }

        /* Scroll group list */
        if (grp_cur < grp_top) grp_top = grp_cur;
        if (grp_cur >= grp_top + browser_rows) grp_top = grp_cur - browser_rows + 1;

        /* Display a list of object groups */
        for (i = 0; i < browser_rows && grp_idx[i] >= 0; i++)
        {
            int  grp = grp_idx[grp_top + i];
            byte attr = (grp_top + i == grp_cur) ? TERM_L_BLUE : TERM_WHITE;

            Term_erase(0, 6 + i, max);
            c_put_str(attr, _art_types[grp].name, 6 + i, 0);
        }

        if (rebuild || old_grp_cur != grp_cur)
        {
            old_grp_cur = grp_cur;

            /* Get a list of objects in the current group */
            art_cnt = _collect_arts(grp_idx[grp_cur], art_idx, show_all);
            rebuild = FALSE;
        }

        /* Scroll object list */
        while (art_cur < art_top)
            art_top = MAX(0, art_top - browser_rows/2);
        while (art_cur >= art_top + browser_rows)
            art_top = MIN(art_cnt - browser_rows, art_top + browser_rows/2);

        /* Display a list of objects in the current group */
        /* Display lines until done */
        for (i = 0; i < browser_rows && art_top + i < art_cnt && art_idx[art_top + i] >= 0; i++)
        {
            char        name[MAX_NLEN];
            int         idx = art_idx[art_top + i];
            object_type forge;
            byte        attr = TERM_WHITE;

            create_named_art_aux_aux(idx, &forge);
            forge.ident = IDENT_KNOWN;
            object_desc(name, &forge, OD_OMIT_INSCRIPTION);

            if (i + art_top == art_cur)
                attr = TERM_L_BLUE;
            else if ((p_ptr->wizard) &&(!a_info[idx].generated))
                attr = TERM_L_DARK;
            else if (!a_info[idx].found)
                attr = (p_ptr->wizard) ? TERM_GREEN : TERM_L_DARK;
            else
                attr = tval_to_attr[forge.tval % 128];

            c_prt(attr, name, 6 + i, max + 3);
        }

        /* Clear remaining lines */
        for (; i < browser_rows; i++)
        {
            Term_erase(max + 3, 6 + i, 255);
        }

        if (show_all)
            prt("<dir>, 'r' 召回, 't' 隐藏未发现, ESC", hgt - 1, 0);
        else
            prt("<dir>, 'r' 召回, 't' 显示全部, ESC", hgt - 1, 0);

        if (!column)
        {
            Term_gotoxy(0, 6 + (grp_cur - grp_top));
        }
        else
        {
            Term_gotoxy(max + 3, 6 + (art_cur - art_top));
        }

        ch = inkey();

        switch (ch)
        {
        case ESCAPE:
            flag = TRUE;
            break;

        case 'T': case 't':
            show_all = !show_all;
            art_cur = 0;
            rebuild = TRUE;
            break;

        case 'R': case 'r':
        case 'I': case 'i':
            if (grp_cnt > 0 && art_idx[art_cur] >= 0)
            {
                int idx = art_idx[art_cur];
                object_type forge;
                create_named_art_aux_aux(idx, &forge);
                forge.ident = IDENT_KNOWN;
                obj_display(&forge);
                redraw = TRUE;
            }
            break;

        default:
            browser_cursor(ch, &column, &grp_cur, grp_cnt, &art_cur, art_cnt);
        }
    }

    C_KILL(art_idx, max_a_idx, int);
}


/*
 * Display known uniques
 * With "XTRA HACK UNIQHIST" (Originally from XAngband)
 */
static void do_cmd_knowledge_uniques(void)
{
    int i, k, n = 0;
    u16b why = 2;
    s16b *who;

    FILE *fff;

    char file_name[1024];

    int n_alive[10];
    int n_alive_surface = 0;
    int n_alive_over100 = 0;
    int n_alive_total = 0;
    int max_lev = -1;

    for (i = 0; i < 10; i++) n_alive[i] = 0;

    /* Open a new file */
    fff = my_fopen_temp(file_name, 1024);

    if (!fff)
    {
        msg_format("无法创建临时文件 %s。", file_name);
        msg_print(NULL);
        return;
    }

    /* Allocate the "who" array */
    C_MAKE(who, max_r_idx, s16b);

    /* Scan the monsters */
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];
        int          lev;

        if (!r_ptr->name) continue;

        /* Require unique monsters */
        if (!(r_ptr->flags1 & RF1_UNIQUE)) continue;
        if (r_ptr->flagsx & RFX_SUPPRESS) continue;

        /* Only display "known" uniques */
		if (!easy_lore && !r_ptr->r_sights) continue;

        /* Only print rarity <= 100 uniques */
        if (!r_ptr->rarity || ((r_ptr->rarity > 100) && !(r_ptr->flagsx & RFX_QUESTOR))) continue;

        /* Only "alive" uniques */
        if (r_ptr->max_num == 0) continue;

        if (r_ptr->level)
        {
            lev = (r_ptr->level - 1) / 10;
            if (lev < 10)
            {
                n_alive[lev]++;
                if (max_lev < lev) max_lev = lev;
            }
            else n_alive_over100++;
        }
        else n_alive_surface++;

        /* Collect "appropriate" monsters */
        who[n++] = i;
    }

    /* Select the sort method */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort the array by dungeon depth of monsters */
    ang_sort(who, &why, n);

    if (n_alive_surface)
    {
        fprintf(fff, "      Surface  alive: %3d\n", n_alive_surface);
        n_alive_total += n_alive_surface;
    }
    for (i = 0; i <= max_lev; i++)
    {
        fprintf(fff, "Level %3d-%3d  alive: %3d\n", 1 + i * 10, 10 + i * 10, n_alive[i]);
        n_alive_total += n_alive[i];
    }
    if (n_alive_over100)
    {
        fprintf(fff, "Level 101-     alive: %3d\n", n_alive_over100);
        n_alive_total += n_alive_over100;
    }

    if (n_alive_total)
    {
        fputs("-------------  ----------\n", fff);
        fprintf(fff, "        Total  alive: %3d\n\n", n_alive_total);
    }
    else
    {
        fputs("没有已知的唯一怪物存活。\n", fff);
    }

    /* Scan the monster races */
    for (k = 0; k < n; k++)
    {
        monster_race *r_ptr = &r_info[who[k]];

        /* Print a message */
        fprintf(fff, "     %s (level %d)\n", r_name + r_ptr->name, r_ptr->level);
    }

    /* Free the "who" array */
    C_KILL(who, max_r_idx, s16b);

    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(TRUE, file_name, "存活的唯一怪物", 0, 0);


    /* Remove the file */
    fd_kill(file_name);
}

void do_cmd_knowledge_shooter(void)
{
    doc_ptr doc = doc_alloc(80);

    display_shooter_info(doc);
    if (doc_line_count(doc))
    {
        screen_save();
        doc_display(doc, "射击", 0);
        screen_load();
    }
    else
        msg_print("你没有装备弓。");

    doc_free(doc);
}

void do_cmd_knowledge_weapon(void)
{
    int i;
    doc_ptr doc = doc_alloc(80);

    for (i = 0; i < MAX_HANDS; i++)
    {
        if (p_ptr->weapon_info[i].wield_how == WIELD_NONE) continue;

        if (p_ptr->weapon_info[i].bare_hands)
            monk_display_attack_info(doc, i);
        else
            display_weapon_info(doc, i);
    }

    for (i = 0; i < p_ptr->innate_attack_ct; i++)
    {
        display_innate_attack_info(doc, i);
    }

    if (doc_line_count(doc))
    {
        screen_save();
        doc_display(doc, "近战", 0);
        screen_load();
    }
    else
        msg_print("你没有近战攻击能力。");

    doc_free(doc);
}

void display_weapon_info_aux(int mode)
{
    bool screen_hack = screen_is_saved();
    if (screen_hack) screen_load();

    display_weapon_mode = mode;
    do_cmd_knowledge_weapon();
    display_weapon_mode = 0;

    if (screen_hack) screen_save();
}

static void do_cmd_knowledge_extra(void)
{
    doc_ptr  doc = doc_alloc(80);
    class_t *class_ptr = get_class();
    race_t  *race_ptr = get_race();

    doc_insert(doc, "<style:wide>");

    if (race_ptr->character_dump)
        race_ptr->character_dump(doc);

    if (class_ptr->character_dump)
        class_ptr->character_dump(doc);

    doc_insert(doc, "</style>");

    doc_display(doc, "种族/职业 额外信息", 0);
    doc_free(doc);
}

/*
 * Display weapon-exp.
 */
static int _compare_k_lvl(object_kind *left, object_kind *right)
{
    if (left->level < right->level) return -1;
    if (left->level > right->level) return 1;
    return 0;
}

static vec_ptr _prof_weapon_alloc(int tval)
{
    int i;
    vec_ptr v = vec_alloc(NULL);
    for (i = 0; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];
        if (k_ptr->tval != tval) continue;
        if ((tval == TV_POLEARM) && (k_ptr->sval == (prace_is_(RACE_MON_SWORD) ? SV_DEATH_SCYTHE : SV_DEATH_SCYTHE_HACK))) continue;
        if (tval == TV_BOW && k_ptr->sval == SV_HARP) continue;
        if (tval == TV_BOW && k_ptr->sval == SV_FLUTE) continue;
        if (tval == TV_BOW && k_ptr->sval == SV_CRIMSON) continue;
        if (tval == TV_BOW && k_ptr->sval == SV_RAILGUN) continue;
        vec_add(v, k_ptr);
    }
    vec_sort(v, (vec_cmp_f)_compare_k_lvl);
    return v;
}
 
static cptr _prof_exp_str[5]   = {"[无]", "[初]", "[熟]", "[专]", "[师]"};
static char _prof_exp_color[5] = {'w',    'G',    'y',    'r',    'v'};
static cptr _prof_weapon_heading(int tval)
{
    switch (tval)
    {
    case TV_SWORD: return "剑类";
    case TV_POLEARM: return "长柄武器";
    case TV_HAFTED: return "钝器/有柄武器";
    case TV_DIGGING: return "挖掘工具";
    case TV_BOW: return "弓类";
    }
    return "";
}

static void _prof_weapon_doc(doc_ptr doc, int tval, int mode)
{
    vec_ptr v = _prof_weapon_alloc(tval);
    int     i;

    doc_insert_text(doc, TERM_RED, _prof_weapon_heading(tval));
    doc_newline(doc);

    for (i = 0; i < vec_length(v); i++)
    {
        object_kind *k_ptr = vec_get(v, i);
        int          exp = skills_weapon_current(k_ptr->tval, k_ptr->sval);
        int          max = skills_weapon_max(k_ptr->tval, k_ptr->sval);
        int          max_lvl = weapon_exp_level(max);
        int          exp_lvl = weapon_exp_level(exp);
        char         name[MAX_NLEN];

        strip_name(name, k_ptr->idx);
        doc_printf(doc, "<color:%c>%-19s</color> ", equip_find_obj(k_ptr->tval, k_ptr->sval) ? 'B' : 'w', name);
        switch (mode)
        {
            case 1:
                doc_printf(doc, " <color:%c>%-4s</color>", _prof_exp_color[max_lvl], _prof_exp_str[max_lvl]);
                break;
            case 2:
                {
                    s32b pct = 0;
                    int pct_lvl;
                    if (max > 0) pct = ((s32b)exp * 100L) / (s32b)max;
                    pct_lvl = weapon_exp_level((WEAPON_EXP_MASTER / 100) * pct);
                    doc_printf(doc, " <color:%c>%3d%%</color>", _prof_exp_color[pct_lvl], pct);
                    break;
                }
            case 3:
                {
                    s32b pct = ((s32b)exp * 100L) / WEAPON_EXP_MASTER;
                    int pct_lvl = weapon_exp_level((WEAPON_EXP_MASTER / 100) * pct);
                    doc_printf(doc, " <color:%c>%3d%%</color>", _prof_exp_color[pct_lvl], pct);
                    break;
                }
            default:
                doc_printf(doc, "%c<color:%c>%-4s</color>", exp >= max ? '!' : ' ', _prof_exp_color[exp_lvl], _prof_exp_str[exp_lvl]);
                break;
        }
        doc_newline(doc);
    }
    doc_newline(doc);
    vec_free(v);
}

static void _prof_skill_aux(doc_ptr doc, int skill, int mode)
{
    int  exp, max, exp_lvl, max_lvl, pct_lvl;
    cptr name;
    char color = 'w';

    switch (skill)
    {
    case SKILL_MARTIAL_ARTS:
        name = "武术";
        exp = skills_martial_arts_current();
        max = skills_martial_arts_max();
        max_lvl = weapon_exp_level(max);
        exp_lvl = weapon_exp_level(exp);
        break;
    case SKILL_DUAL_WIELDING:
        name = "双持";
        exp = skills_dual_wielding_current();
        max = skills_dual_wielding_max();
        max_lvl = weapon_exp_level(max);
        exp_lvl = weapon_exp_level(exp);
        break;
    case SKILL_RIDING:
    default: /* gcc warnings ... */
        name = "骑乘";
        exp = skills_riding_current();
        max = skills_riding_max();
        max_lvl = riding_exp_level(max);
        exp_lvl = riding_exp_level(exp);
        break;
    }
    doc_printf(doc, "<color:%c>%-19s</color> ", color, name);
    switch (mode)
    {
        case 1:
            doc_printf(doc, " <color:%c>%-4s</color>", _prof_exp_color[max_lvl], _prof_exp_str[max_lvl]);
            break;
        case 2:
            {
                s32b pct = 0;
                if (max > 0) pct = ((s32b)exp * 100L) / (s32b)max;
                if (skill == SKILL_RIDING) pct_lvl = riding_exp_level(RIDING_EXP_MASTER / 100 * pct);
                else pct_lvl = weapon_exp_level((WEAPON_EXP_MASTER / 100) * pct);
                doc_printf(doc, " <color:%c>%3d%%</color>", _prof_exp_color[pct_lvl], pct);
                break;
            }
        case 3:
            {
                s32b pct = ((s32b)exp * 100L) / WEAPON_EXP_MASTER;
                if (skill == SKILL_RIDING) pct_lvl = riding_exp_level(RIDING_EXP_MASTER / 100 * pct);
                else pct_lvl = weapon_exp_level((WEAPON_EXP_MASTER / 100) * pct);
                doc_printf(doc, " <color:%c>%3d%%</color>", _prof_exp_color[pct_lvl], pct);
                break;
            }
        default:
            doc_printf(doc, "%c<color:%c>%-4s</color>", exp >= max ? '!' : ' ', _prof_exp_color[exp_lvl], _prof_exp_str[exp_lvl]);
            break;
    }
    doc_newline(doc);
}

static void _prof_skill_doc(doc_ptr doc, int mode)
{
    doc_insert_text(doc, TERM_RED, "杂项");
    doc_newline(doc);
    _prof_skill_aux(doc, SKILL_MARTIAL_ARTS, mode);
    _prof_skill_aux(doc, SKILL_DUAL_WIELDING, mode);
    _prof_skill_aux(doc, SKILL_RIDING, mode);
    doc_newline(doc);
}

static int _do_cmd_knowledge_weapon_exp_aux(int mode, int *huippu)
{
    doc_ptr doc = doc_alloc(80);
    doc_ptr cols[3] = {0};
    int     i, tulos;

    for (i = 0; i < 3; i++)
        cols[i] = doc_alloc(26);

    _prof_weapon_doc(cols[0], TV_SWORD, mode);
    _prof_weapon_doc(cols[1], TV_POLEARM, mode);
    _prof_weapon_doc(cols[1], TV_BOW, mode);
    _prof_weapon_doc(cols[2], TV_HAFTED, mode);
    _prof_weapon_doc(cols[2], TV_DIGGING, mode);
    _prof_skill_doc(cols[2], mode);

    doc_insert_cols(doc, cols, 3, 1);
    switch (mode)
    {   
        case 1:
        {
            class_t *class_ptr = get_class();
            char buf[64];
            strcpy(buf, class_ptr->name);
            strcat(buf, "熟练度上限");
            tulos = weapon_exp_display(doc, buf, huippu); break;
        }
        case 2: tulos = weapon_exp_display(doc, "当前熟练度占上限的百分比", huippu); break;
        case 3: tulos = weapon_exp_display(doc, "当前熟练度占完全精通的百分比", huippu); break;
        default: tulos = weapon_exp_display(doc, "当前熟练度", huippu); break;
    }

    doc_free(doc);
    for (i = 0; i < 3; i++)
        doc_free(cols[i]);
    return tulos;
}

static void do_cmd_knowledge_weapon_exp(void)
{
    int mode = 0;
    bool lopeta = FALSE;
    int huippu = 0;

    while (!lopeta)
    {
        if (_do_cmd_knowledge_weapon_exp_aux(mode, &huippu)) mode = ((mode + 1) % 4);
        else lopeta = TRUE;
    } 
}

/*
 * Display spell-exp
 */
static void do_cmd_knowledge_spell_exp(void)
{
    doc_ptr doc = doc_alloc(80);

    doc_insert(doc, "<style:wide>");
    spellbook_character_dump(doc);
    doc_insert(doc, "</style>");
    doc_display(doc, "法术熟练度", 0);
    doc_free(doc);
}

/*
 * Pluralize a monster name
 */
static bool _plural_imp(char *name, const char *suffix, const char *replacement)
{
    bool result = FALSE;
    int l1 = strlen(name);
    int l2 = strlen(suffix);

    if (l1 >= l2)
    {
        char *tmp = name + (l1 - l2);
        if (streq(tmp, suffix))
        {
            strcpy(tmp, replacement);
            result = TRUE;
        }
    }
    return result;
}

void plural_aux(char *Name)
{
    int NameLen = strlen(Name);

    if (my_strstr(Name, "脱体之手"))
    {
        strcpy(Name, "勒死人的脱体之手");
    }
    else if (my_strstr(Name, "星之彩"))
    {
        strcpy(Name, "星之彩");
    }
    else if (my_strstr(Name, "stairway to hell"))
    {
        strcpy(Name, "stairways to hell");
    }
    else if (my_strstr(Name, "门之主"))
    {
        strcpy(Name, "门之主");
    }
    else if (my_strstr(Name, " of "))
    {
        cptr aider = my_strstr(Name, " of ");
        char dummy[80];
        int i = 0;
        cptr ctr = Name;

        while (ctr < aider)
        {
            dummy[i] = *ctr;
            ctr++; i++;
        }

        if (dummy[i-1] == 's')
        {
            strcpy(&(dummy[i]), "es");
            i++;
        }
        else
        {
            strcpy(&(dummy[i]), "s");
        }

        strcpy(&(dummy[i+1]), aider);
        strcpy(Name, dummy);
    }
    else if (my_strstr(Name, "coins"))
    {
        char dummy[80];
        strcpy(dummy, "piles of ");
        strcat(dummy, Name);
        strcpy(Name, dummy);
        return;
    }
    else if (my_strstr(Name, "亡魂"))
    {
        return;
    }
    else if (_plural_imp(Name, "ey", "eys"))
    {
    }
    else if (_plural_imp(Name, "y", "ies"))
    {
    }
    else if (_plural_imp(Name, "ouse", "ice"))
    {
    }
    else if (_plural_imp(Name, "us", "i"))
    {
    }
    else if (_plural_imp(Name, "kelman", "kelmen"))
    {
    }
    else if (_plural_imp(Name, "wordsman", "wordsmen"))
    {
    }
    else if (_plural_imp(Name, "oodsman", "oodsmen"))
    {
    }
    else if (_plural_imp(Name, "eastman", "eastmen"))
    {
    }
    else if (_plural_imp(Name, "izardman", "izardmen"))
    {
    }
    else if (_plural_imp(Name, "geist", "geister"))
    {
    }
    else if (_plural_imp(Name, "ex", "ices"))
    {
    }
    else if (_plural_imp(Name, "lf", "lves"))
    {
    }
    else if (suffix(Name, "ch") ||
         suffix(Name, "sh") ||
             suffix(Name, "nx") ||
             suffix(Name, "s") ||
             suffix(Name, "o"))
    {
        strcpy(&(Name[NameLen]), "es");
    }
    else
    {
        strcpy(&(Name[NameLen]), "s");
    }
}

/*
 * Display current pets
 */
static void do_cmd_knowledge_pets(void)
{
    int             i;
    FILE            *fff;
    monster_type    *m_ptr;
    monster_race    *r_ptr;
    char            pet_name[80];
    int             t_friends = 0;
    int             show_upkeep = 0;
    char            file_name[1024];


    /* Open a new file */
    fff = my_fopen_temp(file_name, 1024);
    if (!fff) {
        msg_format("无法创建临时文件 %s。", file_name);
        msg_print(NULL);
        return;
    }

    /* Process the monsters (backwards) */
    for (i = m_max - 1; i >= 1; i--)
    {
        /* Access the monster */
        m_ptr = &m_list[i];
        r_ptr = &r_info[m_ptr->r_idx];

        /* Ignore "dead" monsters */
        if (!m_ptr->r_idx) continue;

        /* Calculate "upkeep" for pets */
        if (is_pet(m_ptr))
        {
            t_friends++;
            monster_desc(pet_name, m_ptr, MD_ASSUME_VISIBLE | MD_INDEF_VISIBLE);
            fprintf(fff, "%s (", pet_name);
            if (r_ptr->r_tkills)
                fprintf(fff, "L%d, ", r_ptr->level);
            fprintf(fff, "%s)\n", mon_health_desc(m_ptr));
        }
    }

    show_upkeep = calculate_upkeep();

    fprintf(fff, "----------------------------------------------\n");
    fprintf(fff, "   Total: %d pet%s.\n",
        t_friends, (t_friends == 1 ? "" : "s"));
    fprintf(fff, "   Upkeep: %d%% mana.\n", show_upkeep);



    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(TRUE, file_name, "当前宠物", 0, 0);


    /* Remove the file */
    fd_kill(file_name);
}


/*
 * Total kill count
 *
 * Note that the player ghosts are ignored. XXX XXX XXX
 */
static void do_cmd_knowledge_kill_count(void)
{
    int i, k, n = 0;
    u16b why = 2;
    s16b *who;

    FILE *fff;

    char file_name[1024];

    s32b Total = 0;


    /* Open a new file */
    fff = my_fopen_temp(file_name, 1024);

    if (!fff) {
        msg_format("无法创建临时文件 %s。", file_name);
        msg_print(NULL);
        return;
    }

    /* Allocate the "who" array */
    C_MAKE(who, max_r_idx, s16b);

    {
        /* Monsters slain */
        int kk;

        for (kk = 1; kk < max_r_idx; kk++)
        {
            monster_race *r_ptr = &r_info[kk];

            if (r_ptr->flags1 & (RF1_UNIQUE))
            {
                bool dead = (r_ptr->max_num == 0);

                if (dead)
                {
                    Total++;
                }
            }
            else
            {
                s16b This = r_ptr->r_pkills;

                if (This > 0)
                {
                    Total += This;
                }
            }
        }

        if (Total < 1)
            fprintf(fff,"你还没有击败任何敌人。\n\n");
        else
            fprintf(fff,"你已经击败了 %d %s。\n\n", Total, (Total == 1) ? "个敌人" : "个敌人");
    }

    Total = 0;

    /* Scan the monsters */
    for (i = 1; i < max_r_idx; i++)
    {
        monster_race *r_ptr = &r_info[i];

        /* Use that monster */
        if (r_ptr->name) who[n++] = i;
    }

    /* Select the sort method */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort the array by dungeon depth of monsters */
    ang_sort(who, &why, n);

    /* Scan the monster races */
    for (k = 0; k < n; k++)
    {
        monster_race *r_ptr = &r_info[who[k]];

        if (r_ptr->flags1 & (RF1_UNIQUE))
        {
            bool dead = (r_ptr->max_num == 0);

            if (dead)
            {
                /* Print a message */
                fprintf(fff, "     %s\n",
                    (r_name + r_ptr->name));
                Total++;
            }
        }
        else
        {
            s16b This = r_ptr->r_pkills;

            if (This > 0)
            {
                if (This < 2)
                {
                    if (my_strstr(r_name + r_ptr->name, "coins"))
                    {
                        fprintf(fff, "     1 pile of %s\n", (r_name + r_ptr->name));
                    }
                    else
                    {
                        fprintf(fff, "     1 %s\n", (r_name + r_ptr->name));
                    }
                }
                else
                {
                    char ToPlural[80];
                    strcpy(ToPlural, (r_name + r_ptr->name));
                    plural_aux(ToPlural);
                    fprintf(fff, "     %d %s\n", This, ToPlural);
                }


                Total += This;
            }
        }
    }

    fprintf(fff,"----------------------------------------------\n");
    fprintf(fff,"   Total: %d creature%s killed.\n",
        Total, (Total == 1 ? "" : "s"));


    /* Free the "who" array */
    C_KILL(who, max_r_idx, s16b);

    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(TRUE, file_name, "击杀统计", 0, 0);


    /* Remove the file */
    fd_kill(file_name);
}


/*
 * Display the object groups.
 */
static void display_group_list(int col, int row, int wid, int per_page,
    int grp_idx[], cptr group_text[], int grp_cur, int grp_top)
{
    int i;

    /* Display lines until done */
    for (i = 0; i < per_page && (grp_idx[i] >= 0); i++)
    {
        /* Get the group index */
        int grp = grp_idx[grp_top + i];

        /* Choose a color */
        byte attr = (grp_top + i == grp_cur) ? TERM_L_BLUE : TERM_WHITE;

        /* Erase the entire line */
        Term_erase(col, row + i, wid);

        /* Display the group label */
        c_put_str(attr, group_text[grp], row + i, col);
    }
}


/*
 * Move the cursor in a browser window
 */
static void browser_cursor(char ch, int *column, int *grp_cur, int grp_cnt,
                           int *list_cur, int list_cnt)
{
    int d;
    int col = *column;
    int grp = *grp_cur;
    int list = *list_cur;

    /* Extract direction */
    if (ch == ' ')
    {
        /* Hack -- scroll up full screen */
        d = 3;
    }
    else if (ch == '-')
    {
        /* Hack -- scroll down full screen */
        d = 9;
    }
    else
    {
        d = get_keymap_dir(ch, FALSE);
    }

    if (!d) return;

    /* Diagonals - hack */
    if ((ddx[d] > 0) && ddy[d])
    {
        int browser_rows;
        int wid, hgt;

        /* Get size */
        Term_get_size(&wid, &hgt);

        browser_rows = hgt - 8;

        /* Browse group list */
        if (!col)
        {
            int old_grp = grp;

            /* Move up or down */
            grp += ddy[d] * (browser_rows - 1);

            /* Verify */
            if (grp >= grp_cnt)    grp = grp_cnt - 1;
            if (grp < 0) grp = 0;
            if (grp != old_grp)    list = 0;
        }

        /* Browse sub-list list */
        else
        {
            /* Move up or down */
            list += ddy[d] * browser_rows;

            /* Verify */
            if (list >= list_cnt) list = list_cnt - 1;
            if (list < 0) list = 0;
        }

        (*grp_cur) = grp;
        (*list_cur) = list;

        return;
    }

    if (ddx[d])
    {
        col += ddx[d];
        if (col < 0) col = 0;
        if (col > 1) col = 1;

        (*column) = col;

        return;
    }

    /* Browse group list */
    if (!col)
    {
        int old_grp = grp;

        /* Move up or down */
        grp += ddy[d];

        /* Verify */
        if (grp >= grp_cnt)    grp = grp_cnt - 1;
        if (grp < 0) grp = 0;
        if (grp != old_grp)    list = 0;
    }

    /* Browse sub-list list */
    else
    {
        /* Move up or down */
        list += ddy[d];

        /* Verify */
        if (list >= list_cnt) list = list_cnt - 1;
        if (list < 0) list = 0;
    }

    (*grp_cur) = grp;
    (*list_cur) = list;
}


/*
 * Display visuals.
 */
static void display_visual_list(int col, int row, int height, int width, byte attr_top, byte char_left)
{
    int i, j;

    /* Clear the display lines */
    for (i = 0; i < height; i++)
    {
        Term_erase(col, row + i, width);
    }

    /* Bigtile mode uses double width */
    if (use_bigtile) width /= 2;

    /* Display lines until done */
    for (i = 0; i < height; i++)
    {
        /* Display columns until done */
        for (j = 0; j < width; j++)
        {
            byte a;
            char c;
            int x = col + j;
            int y = row + i;
            int ia, ic;

            /* Bigtile mode uses double width */
            if (use_bigtile) x += j;

            ia = attr_top + i;
            ic = char_left + j;

            /* Ignore illegal characters */
            if (ia > 0x7f || ic > 0xff || ic < ' ' ||
                (!use_graphics && ic > 0x7f))
                continue;

            a = (byte)ia;
            c = (char)ic;

            /* Force correct code for both ASCII character and tile */
            if (c & 0x80) a |= 0x80;

            /* Display symbol */
            Term_queue_bigchar(x, y, a, c, 0, 0);
        }
    }
}


/*
 * Place the cursor at the collect position for visual mode
 */
static void place_visual_list_cursor(int col, int row, byte a, byte c, byte attr_top, byte char_left)
{
    int i = (a & 0x7f) - attr_top;
    int j = c - char_left;

    int x = col + j;
    int y = row + i;

    /* Bigtile mode uses double width */
    if (use_bigtile) x += j;

    /* Place the cursor */
    Term_gotoxy(x, y);
}


/*
 *  Clipboard variables for copy&paste in visual mode
 */
static byte attr_idx = 0;
static byte char_idx = 0;

/* Hack -- for feature lighting */
static byte attr_idx_feat[F_LIT_MAX];
static byte char_idx_feat[F_LIT_MAX];

/*
 *  Do visual mode command -- Change symbols
 */
static bool visual_mode_command(char ch, bool *visual_list_ptr,
                int height, int width,
                byte *attr_top_ptr, byte *char_left_ptr,
                byte *cur_attr_ptr, byte *cur_char_ptr, bool *need_redraw)
{
    static byte attr_old = 0, char_old = 0;

    switch (ch)
    {
    case ESCAPE:
        if (*visual_list_ptr)
        {
            /* Cancel change */
            *cur_attr_ptr = attr_old;
            *cur_char_ptr = char_old;
            *visual_list_ptr = FALSE;

            return TRUE;
        }
        break;

    case '\n':
    case '\r':
        if (*visual_list_ptr)
        {
            /* Accept change */
            *visual_list_ptr = FALSE;
            *need_redraw = TRUE;

            return TRUE;
        }
        break;

    case 'V':
    case 'v':
        if (!*visual_list_ptr)
        {
            *visual_list_ptr = TRUE;

            *attr_top_ptr = MAX(0, (*cur_attr_ptr & 0x7f) - 5);
            *char_left_ptr = MAX(0, *cur_char_ptr - 10);

            attr_old = *cur_attr_ptr;
            char_old = *cur_char_ptr;

            return TRUE;
        }
        break;

    case 'C':
    case 'c':
        {
            int i;

            /* Set the visual */
            attr_idx = *cur_attr_ptr;
            char_idx = *cur_char_ptr;

            /* Hack -- for feature lighting */
            for (i = 0; i < F_LIT_MAX; i++)
            {
                attr_idx_feat[i] = 0;
                char_idx_feat[i] = 0;
            }
        }
        return TRUE;

    case 'P':
    case 'p':
        if (attr_idx || (!(char_idx & 0x80) && char_idx)) /* Allow TERM_DARK text */
        {
            /* Set the char */
            *cur_attr_ptr = attr_idx;
            *attr_top_ptr = MAX(0, (*cur_attr_ptr & 0x7f) - 5);
            if (!*visual_list_ptr) *need_redraw = TRUE;
        }

        if (char_idx)
        {
            /* Set the char */
            *cur_char_ptr = char_idx;
            *char_left_ptr = MAX(0, *cur_char_ptr - 10);
            if (!*visual_list_ptr) *need_redraw = TRUE;
        }

        return TRUE;

    default:
        if (*visual_list_ptr)
        {
            int eff_width;
            int d = get_keymap_dir(ch, FALSE);
            byte a = (*cur_attr_ptr & 0x7f);
            byte c = *cur_char_ptr;

            if (use_bigtile) eff_width = width / 2;
            else eff_width = width;

            /* Restrict direction */
            if ((a == 0) && (ddy[d] < 0)) d = 0;
            if ((c == 0) && (ddx[d] < 0)) d = 0;
            if ((a == 0x7f) && (ddy[d] > 0)) d = 0;
            if ((c == 0xff) && (ddx[d] > 0)) d = 0;

            a += ddy[d];
            c += ddx[d];

            /* Force correct code for both ASCII character and tile */
            if (c & 0x80) a |= 0x80;

            /* Set the visual */
            *cur_attr_ptr = a;
            *cur_char_ptr = c;


            /* Move the frame */
            if ((ddx[d] < 0) && *char_left_ptr > MAX(0, (int)c - 10)) (*char_left_ptr)--;
            if ((ddx[d] > 0) && *char_left_ptr + eff_width < MIN(0xff, (int)c + 10)) (*char_left_ptr)++;
            if ((ddy[d] < 0) && *attr_top_ptr > MAX(0, (int)(a & 0x7f) - 4)) (*attr_top_ptr)--;
            if ((ddy[d] > 0) && *attr_top_ptr + height < MIN(0x7f, (a & 0x7f) + 4)) (*attr_top_ptr)++;
            return TRUE;
        }
        break;
    }

    /* Visual mode command is not used */
    return FALSE;
}

enum monster_mode_e
{
    MONSTER_MODE_STATS,
    MONSTER_MODE_SKILLS,
    MONSTER_MODE_EXTRA,
    MONSTER_MODE_MAX
};
static int monster_mode = MONSTER_MODE_STATS;

static void _prt_equippy(int col, int row, int tval, int sval)
{
    int k_idx = lookup_kind(tval, sval);
    object_kind *k_ptr = &k_info[k_idx];
    Term_putch(col, row, k_ptr->x_attr, k_ptr->x_char);
}

/*
 * Display the monsters in a group.
 */
static void display_monster_list(int col, int row, int per_page, s16b mon_idx[],
    int mon_cur, int mon_top, bool visual_only)
{
    int i;

    /* Display lines until done */
    for (i = 0; i < per_page && (mon_idx[mon_top + i] >= 0); i++)
    {
        byte attr;

        /* Get the race index */
        int r_idx = mon_idx[mon_top + i] ;

        /* Access the race */
        monster_race *r_ptr = &r_info[r_idx];

        /* Choose a color */
        attr = ((i + mon_top == mon_cur) ? TERM_L_BLUE : TERM_WHITE);
        if (attr == TERM_WHITE && (r_ptr->flagsx & RFX_SUPPRESS))
            attr = TERM_L_DARK;

        /* Display the name */
        c_prt(attr, monster_race_display_name(r_idx), row + i, col);

        /* Hack -- visual_list mode */
        if (per_page == 1)
        {
            c_prt(attr, format("%02x/%02x", r_ptr->x_attr, r_ptr->x_char), row + i, (p_ptr->wizard || visual_only) ? 56 : 61);
        }
        if (p_ptr->wizard || visual_only || ethereal_mimic_is_())
        {
            c_prt(attr, format("%d", r_idx), row + i, 62);
        }

        /* Erase chars before overwritten by the race letter */
        Term_erase(69, row + i, 255);

        /* Display symbol */
        Term_queue_bigchar(use_bigtile ? 69 : 70, row + i, r_ptr->x_attr, r_ptr->x_char, 0, 0);

        if (!visual_only)
        {
            /* Display kills */
            if (!(r_ptr->flags1 & RF1_UNIQUE)) put_str(format("%5d", r_ptr->r_pkills), row + i, 73);
            else c_put_str((r_ptr->max_num == 0 ? TERM_L_DARK : TERM_WHITE), (r_ptr->max_num == 0 ? "死亡" : "存活"), row + i, 73);

            if (ethereal_mimic_is_() && !visual_only)
            {
                if (ethereal_mimic_can_mimic(r_idx))
                {
                    int req = ethereal_mimic_kill_requirement(r_idx);
                    if (ethereal_mimic_is_learned(r_idx))
                        c_put_str(TERM_L_GREEN, "已学会", row + i, 80);
                    else
                        c_put_str(TERM_YELLOW, format("%d/%d", r_ptr->r_pkills, req), row + i, 80);
                }
                else
                    c_put_str(TERM_L_DARK, "-", row + i, 80);
            }

            /* Only Possessors get the extra body info display */
            if (p_ptr->wizard || p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
            {
                /* And then, they must learn about the body first. (Or be a cheating wizard :) */
                if ((p_ptr->wizard || (r_ptr->r_xtra1 & MR1_POSSESSOR)) && r_ptr->body.life)
                {
                    char buf[255];
                    equip_template_ptr body = &b_info[r_ptr->body.body_idx];
                    if (monster_mode == MONSTER_MODE_STATS)
                    {
                        int j;
                        for (j = 0; j < 6; j++)
                        {
                            sprintf(buf, "%+3d", r_ptr->body.stats[j]);
                            c_put_str(j == r_ptr->body.spell_stat ? TERM_L_GREEN : TERM_WHITE,
                                      buf, row + i, 80 + j * 5);
                        }
                        sprintf(buf, "%+3d%%", r_ptr->body.life);
                        c_put_str(TERM_WHITE, buf, row + i, 110);

                        for (j = 1; j <= body->max; j++)
                        {
                            int c = 115 + j;
                            int r = row + i;
                            switch (body->slots[j].type)
                            {
                            case EQUIP_SLOT_GLOVES:
                                _prt_equippy(c, r, TV_GLOVES, SV_SET_OF_GAUNTLETS);
                                break;
                            case EQUIP_SLOT_WEAPON_SHIELD:
                                if (body->slots[j].hand % 2)
                                    _prt_equippy(c, r, TV_SHIELD, SV_LARGE_METAL_SHIELD);
                                else
                                    _prt_equippy(c, r, TV_SWORD, SV_LONG_SWORD);
                                break;
                            case EQUIP_SLOT_WEAPON:
                                _prt_equippy(c, r, TV_SWORD, SV_LONG_SWORD);
                                break;
                            case EQUIP_SLOT_RING:
                                _prt_equippy(c, r, TV_RING, 0);
                                break;
                            case EQUIP_SLOT_BOW:
                                _prt_equippy(c, r, TV_BOW, SV_LONG_BOW);
                                break;
                            case EQUIP_SLOT_AMULET:
                                _prt_equippy(c, r, TV_AMULET, 0);
                                break;
                            case EQUIP_SLOT_LITE:
                                _prt_equippy(c, r, TV_LITE, SV_LITE_FEANOR);
                                break;
                            case EQUIP_SLOT_BODY_ARMOR:
                                _prt_equippy(c, r, TV_HARD_ARMOR, SV_CHAIN_MAIL);
                                break;
                            case EQUIP_SLOT_CLOAK:
                                _prt_equippy(c, r, TV_CLOAK, SV_CLOAK);
                                break;
                            case EQUIP_SLOT_BOOTS:
                                _prt_equippy(c, r, TV_BOOTS, SV_PAIR_OF_HARD_LEATHER_BOOTS);
                                break;
                            case EQUIP_SLOT_HELMET:
                                _prt_equippy(c, r, TV_HELM, SV_IRON_HELM);
                                break;
                            case EQUIP_SLOT_ANY:
                                Term_putch(c, r, TERM_WHITE, '*');
                                break;
                            case EQUIP_SLOT_CAPTURE_BALL:
                                _prt_equippy(c, r, TV_CAPTURE, 0);
                                break;
                            }
                        }
                    }
                    else if (monster_mode == MONSTER_MODE_SKILLS)
                    {
                        sprintf(buf, "%2d+%-2d  %2d+%-2d  %2d+%-2d  %4d  %4d  %4d  %2d+%-2d  %2d+%-2d\n",
                            r_ptr->body.skills.dis, r_ptr->body.extra_skills.dis,
                            r_ptr->body.skills.dev, r_ptr->body.extra_skills.dev,
                            r_ptr->body.skills.sav, r_ptr->body.extra_skills.sav,
                            r_ptr->body.skills.stl,
                            r_ptr->body.skills.srh,
                            r_ptr->body.skills.fos,
                            r_ptr->body.skills.thn, r_ptr->body.extra_skills.thn,
                            r_ptr->body.skills.thb, r_ptr->body.extra_skills.thb
                        );
                        c_put_str(TERM_WHITE, buf, row + i, 80);
                    }
                    else if (monster_mode == MONSTER_MODE_EXTRA)
                    {
                        int speed = possessor_r_speed(r_idx);
                        int ac = possessor_r_ac(r_idx);

                        sprintf(buf, "%3d  %3d  %+5d  %+4d  %s",
                            r_ptr->level, possessor_max_plr_lvl(r_idx), speed, ac,
                            get_class_aux(r_ptr->body.class_idx, 0)->name
                        );
                        c_put_str(TERM_WHITE, buf, row + i, 80);
                    }
                }
            }
        }
    }

    /* Clear remaining lines */
    for (; i < per_page; i++)
    {
        Term_erase(col, row + i, 255);
    }
}


/*
 * Display known monsters.
 */
static void do_cmd_knowledge_monsters(bool *need_redraw, bool visual_only, int direct_r_idx)
{
    int i, len, max;
    int grp_cur, grp_top, old_grp_cur;
    int mon_cur, mon_top;
    int grp_cnt, grp_idx[100];
    int mon_cnt;
    s16b *mon_idx;

    int column = 0;
    bool flag;
    bool redraw;

    bool visual_list = FALSE;
    byte attr_top = 0, char_left = 0;

    int browser_rows;
    int wid, hgt;

    byte mode;

    /* Get size */
    Term_get_size(&wid, &hgt);

    browser_rows = hgt - 8;

    /* Allocate the "mon_idx" array */
    C_MAKE(mon_idx, max_r_idx, s16b);

    max = 0;
    grp_cnt = 0;

    if (direct_r_idx < 0)
    {
        mode = visual_only ? 0x03 : 0x01;

        /* Check every group */
        for (i = 0; monster_group_text[i] != NULL; i++)
        {
            if (monster_group_char[i] == ((char *) -1L) && p_ptr->prace != RACE_MON_POSSESSOR)
                continue;

            /* Measure the label */
            len = strlen(monster_group_text[i]);

            /* Save the maximum length */
            if (len > max) max = len;

            /* See if any monsters are known */
            if ((monster_group_char[i] == ((char *) -2L)) || collect_monsters(i, mon_idx, mode))
            {
                /* Build a list of groups with known monsters */
                grp_idx[grp_cnt++] = i;
            }
        }

        mon_cnt = 0;
    }
    else
    {
        mon_idx[0] = direct_r_idx;
        mon_cnt = 1;

        /* Terminate the list */
        mon_idx[1] = -1;

        (void)visual_mode_command('v', &visual_list, browser_rows - 1, wid - (max + 3),
            &attr_top, &char_left, &r_info[direct_r_idx].x_attr, &r_info[direct_r_idx].x_char, need_redraw);
    }

    /* Terminate the list */
    grp_idx[grp_cnt] = -1;

    old_grp_cur = -1;
    grp_cur = grp_top = 0;
    mon_cur = mon_top = 0;

    flag = FALSE;
    redraw = TRUE;

    mode = visual_only ? 0x02 : 0x00;

    while (!flag)
    {
        char ch;
        monster_race *r_ptr;

        if (redraw)
        {
            clear_from(0);

            prt(format("%s - 怪物", !visual_only ? "知识" : "视觉(Visuals)"), 2, 0);
            if (direct_r_idx < 0) prt("分组", 4, 0);
            prt("名称", 4, max + 3);
            if (p_ptr->wizard || visual_only || ethereal_mimic_is_()) prt("序号(Idx)", 4, 62);
            prt("字符(Sym)", 4, 68);
            if (!visual_only) prt("击杀", 4, 73);
            if (!visual_only && ethereal_mimic_is_()) prt("模仿", 4, 80);

            if (p_ptr->wizard || p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
            {
                char buf[255];
                if (monster_mode == MONSTER_MODE_STATS)
                {
                    sprintf(buf, "力 智 感 敏 体 魅 命 躯");
                    c_put_str(TERM_WHITE, buf, 4, 80);
                    for (i = 78; i < 130; i++)
                        Term_putch(i, 5, TERM_WHITE, '=');
                }
                else if (monster_mode == MONSTER_MODE_SKILLS)
                {
                    sprintf(buf, "Dsrm   Dvce   Save   Stlh  Srch  Prcp  Melee  Bows");
                    c_put_str(TERM_WHITE, buf, 4, 80);
                    for (i = 78; i < 130; i++)
                        Term_putch(i, 5, TERM_WHITE, '=');
                }
                else if (monster_mode == MONSTER_MODE_EXTRA)
                {
                    sprintf(buf, "等级 最大 速度 护甲 伪职业");
                    c_put_str(TERM_WHITE, buf, 4, 80);
                    for (i = 78; i < 130; i++)
                        Term_putch(i, 5, TERM_WHITE, '=');
                }
            }

            for (i = 0; i < 78; i++)
            {
                Term_putch(i, 5, TERM_WHITE, '=');
            }

            if (direct_r_idx < 0)
            {
                for (i = 0; i < browser_rows; i++)
                {
                    Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
                }
            }

            redraw = FALSE;
        }

        if (direct_r_idx < 0)
        {
            /* Scroll group list */
            if (grp_cur < grp_top) grp_top = grp_cur;
            if (grp_cur >= grp_top + browser_rows) grp_top = grp_cur - browser_rows + 1;

            /* Display a list of monster groups */
            display_group_list(0, 6, max, browser_rows, grp_idx, monster_group_text, grp_cur, grp_top);

            if (old_grp_cur != grp_cur)
            {
                old_grp_cur = grp_cur;

                /* Get a list of monsters in the current group */
                mon_cnt = collect_monsters(grp_idx[grp_cur], mon_idx, mode);
            }

            /* Scroll monster list */
            while (mon_cur < mon_top)
                mon_top = MAX(0, mon_top - browser_rows/2);
            while (mon_cur >= mon_top + browser_rows)
                mon_top = MIN(mon_cnt - browser_rows, mon_top + browser_rows/2);
        }

        if (!visual_list)
        {
            /* Display a list of monsters in the current group */
            display_monster_list(max + 3, 6, browser_rows, mon_idx, mon_cur, mon_top, visual_only);
        }
        else
        {
            mon_top = mon_cur;

            /* Display a monster name */
            display_monster_list(max + 3, 6, 1, mon_idx, mon_cur, mon_top, visual_only);

            /* Display visual list below first monster */
            display_visual_list(max + 3, 7, browser_rows-1, wid - (max + 3), attr_top, char_left);
        }

        /* Prompt */
        if (p_ptr->wizard || p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        {
            prt(format("<dir>%s%s%s%s, ESC",
                (!visual_list && !visual_only) ? "，'?' 回忆" : "",
                visual_list ? "，回车 接受" : "，'v' 视觉效果",
                (attr_idx || char_idx) ? "，'c', 'p' 粘贴" : "，'c' 复制",
                "，'=' 更多信息"),
                hgt - 1, 0);
        }
        else
        {
            prt(format("<dir>%s%s%s, ESC",
                (!visual_list && !visual_only) ? "，'?' 回忆" : "",
                visual_list ? "，回车 接受" : "，'v' 视觉效果",
                (attr_idx || char_idx) ? "，'c', 'p' 粘贴" : "，'c' 复制"),
                hgt - 1, 0);
        }

        /* Get the current monster */
        r_ptr = &r_info[mon_idx[mon_cur]];

        if (!visual_only)
        {
            /* Mega Hack -- track this monster race */
            if (mon_cnt) monster_race_track(mon_idx[mon_cur]);

            /* Hack -- handle stuff */
            handle_stuff();
        }

        if (visual_list)
        {
            place_visual_list_cursor(max + 3, 7, r_ptr->x_attr, r_ptr->x_char, attr_top, char_left);
        }
        else if (!column)
        {
            Term_gotoxy(0, 6 + (grp_cur - grp_top));
        }
        else
        {
            Term_gotoxy(max + 3, 6 + (mon_cur - mon_top));
        }

        ch = inkey();

        /* Do visual mode command if needed */
        if (visual_mode_command(ch, &visual_list, browser_rows-1, wid - (max + 3), &attr_top, &char_left, &r_ptr->x_attr, &r_ptr->x_char, need_redraw))
        {
            if (direct_r_idx >= 0)
            {
                switch (ch)
                {
                case '\n':
                case '\r':
                case ESCAPE:
                    flag = TRUE;
                    break;
                }
            }
            continue;
        }

        switch (ch)
        {
            case ESCAPE:
            {
                flag = TRUE;
                break;
            }

            case 'R':
            case 'r':
            case '?':
            {
                /* Recall on screen */
                if (!visual_list && !visual_only && (mon_idx[mon_cur] > 0))
                {
                    int r_idx = mon_idx[mon_cur];
                    mon_display(&r_info[r_idx]);
                    redraw = TRUE;
                }
                break;
            }

            case 'm':
            case 'n':
            case 'h':
            case '=':
                monster_mode++;
                if (monster_mode == MONSTER_MODE_MAX)
                    monster_mode = MONSTER_MODE_STATS;
                redraw = TRUE;
                break;

            default:
            {
                /* Move the cursor */
                browser_cursor(ch, &column, &grp_cur, grp_cnt, &mon_cur, mon_cnt);

                break;
            }
        }
    }

    /* Free the "mon_idx" array */
    C_KILL(mon_idx, max_r_idx, s16b);
}


/*
 * Display the objects in a group.
 */
static void display_object_list(int col, int row, int per_page, int object_idx[],
    int object_cur, int object_top, int object_count, bool visual_only)
{
    int i;

    /* Display lines until done */
    for (i = 0; i < per_page && object_top + i < object_count && object_idx[object_top + i] >= 0; i++)
    {
        char o_name[80];
        char buf[255];
        byte a, c;
        object_kind *flavor_k_ptr;

        /* Get the object index */
        int k_idx = object_idx[object_top + i];

        /* Access the object */
        object_kind *k_ptr = &k_info[k_idx];

        /* Choose a color */
        byte attr = ((k_ptr->aware || visual_only) ? TERM_WHITE : TERM_SLATE);
        byte cursor = ((k_ptr->aware || visual_only) ? TERM_L_BLUE : TERM_BLUE);


        if (!visual_only && k_ptr->flavor)
        {
            /* Appearance of this object is shuffled */
            flavor_k_ptr = &k_info[k_ptr->flavor];
        }
        else
        {
            /* Appearance of this object is very normal */
            flavor_k_ptr = k_ptr;
        }



        attr = ((i + object_top == object_cur) ? cursor : attr);

        if (!k_ptr->flavor || (!visual_only && k_ptr->aware))
        {
            /* Tidy name */
            strip_name(o_name, k_idx);
        }
        else
        {
            /* Flavor name */
            strcpy(o_name, k_name + flavor_k_ptr->flavor_name);
        }

        /* Display the name */
        sprintf(buf, "%-35.35s %5d %6d %4d %4d", o_name, k_ptr->counts.found, k_ptr->counts.bought, k_ptr->counts.used, k_ptr->counts.destroyed);
        c_prt(attr, buf, row + i, col);

        /* Hack -- visual_list mode */
        if (per_page == 1)
        {
            c_prt(attr, format("%02x/%02x", flavor_k_ptr->x_attr, flavor_k_ptr->x_char), row + i, (p_ptr->wizard || visual_only) ? 64 : 68);
        }
        if (visual_only)
        {
            c_prt(attr, format("%d", k_idx), row + i, 70);
        }

        a = flavor_k_ptr->x_attr;
        c = flavor_k_ptr->x_char;

        /* Display symbol */
        Term_queue_bigchar(use_bigtile ? 76 : 77, row + i, a, c, 0, 0);
    }

    /* Total Line? */
    if (!visual_only && i < per_page && object_idx[object_top + i] < 0)
    {
        char     buf[255];
        counts_t totals = {0};
        int      j;

        for (j = 0; object_idx[j] >= 0; j++)
        {
            object_kind   *k_ptr = &k_info[object_idx[j]];

            totals.found += k_ptr->counts.found;
            totals.bought += k_ptr->counts.bought;
            totals.used += k_ptr->counts.used;
            totals.destroyed += k_ptr->counts.destroyed;
        }

        sprintf(buf, "%-35.35s %5d %6d %4d %4d",
            "总计",
            totals.found, totals.bought, totals.used, totals.destroyed
        );
        c_prt(TERM_YELLOW, buf, row + i, col);
        i++;
    }

    /* Clear remaining lines */
    for (; i < per_page; i++)
    {
        Term_erase(col, row + i, 255);
    }
}

/*
 * Describe fake object
 */
static void desc_obj_fake(int k_idx)
{
    object_type *o_ptr;
    object_type object_type_body;

    /* Get local object */
    o_ptr = &object_type_body;

    /* Wipe the object */
    object_wipe(o_ptr);

    /* Create the artifact */
    object_prep(o_ptr, k_idx);

    /* It's fully know */
    o_ptr->ident |= IDENT_KNOWN;

    /* Track the object */
    /* object_actual_track(o_ptr); */

    /* Hack - mark as fake */
    /* term_obj_real = FALSE; */

    /* Hack -- Handle stuff */
    handle_stuff();

    obj_display(o_ptr);
}

static void desc_ego_fake(int e_idx)
{
    ego_type *e_ptr = &e_info[e_idx];
    ego_display(e_ptr);
}


typedef struct {
    u32b id;
    cptr name;
} _ego_type_t;

static _ego_type_t _ego_types[] = {
    { EGO_TYPE_WEAPON, "武器" },
    { EGO_TYPE_DIGGER, "挖掘工具" },

    { EGO_TYPE_SHIELD, "盾牌" },
    { EGO_TYPE_BODY_ARMOR, "躯干护甲" },
    { EGO_TYPE_ROBE, "长袍" },
    { EGO_TYPE_DRAGON_ARMOR, "龙鳞甲" },
    { EGO_TYPE_CLOAK, "披风" },
    { EGO_TYPE_HELMET, "头盔" },
    { EGO_TYPE_CROWN, "皇冠" },
    { EGO_TYPE_GLOVES, "手套" },
    { EGO_TYPE_BOOTS, "鞋子" },

    { EGO_TYPE_BOW, "弓类" },
    { EGO_TYPE_AMMO, "弹药" },
    { EGO_TYPE_HARP, "竖琴" },

    { EGO_TYPE_RING, "戒指" },
    { EGO_TYPE_AMULET, "护身符" },
    { EGO_TYPE_LITE, "光源" },
    { EGO_TYPE_DEVICE, "魔法装置" },

    { EGO_TYPE_NONE, NULL },
};

static bool _compare_e_level(vptr u, vptr v, int a, int b)
{
    int *indices = (int*)u;
    int left = indices[a];
    int right = indices[b];
    return e_info[left].level <= e_info[right].level;
}

static int _collect_egos(int grp_cur, int ego_idx[])
{
    int i, cnt = 0;
    int type = _ego_types[grp_cur].id;

    for (i = 0; i < max_e_idx; i++)
    {
        ego_type *e_ptr = &e_info[i];

        if (!e_ptr->name) continue;
        /*if (!e_ptr->aware) continue;*/
        if (!ego_has_lore(e_ptr) && !e_ptr->counts.found && !e_ptr->counts.bought) continue;
        if (!(e_ptr->type & type)) continue;

        ego_idx[cnt++] = i;
    }

    /* Sort Results */
    ang_sort_comp = _compare_e_level;
    ang_sort_swap = _swap_int;
    ang_sort(ego_idx, NULL, cnt);

    /* Terminate the list */
    ego_idx[cnt] = -1;

    return cnt;
}

static void do_cmd_knowledge_egos(void)
{
    int i, len, max;
    int grp_cur, grp_top, old_grp_cur;
    int ego_cur, ego_top;
    int grp_cnt, grp_idx[100];
    int ego_cnt;
    int *ego_idx;

    int column = 0;
    bool flag;
    bool redraw;

    int browser_rows;
    int wid, hgt;

    /* Get size */
    Term_get_size(&wid, &hgt);

    browser_rows = hgt - 8;

    C_MAKE(ego_idx, max_e_idx, int);

    max = 0;
    grp_cnt = 0;
    for (i = 0; _ego_types[i].id != EGO_TYPE_NONE; i++)
    {
        len = strlen(_ego_types[i].name);
        if (len > max)
            max = len;

        if (_collect_egos(i, ego_idx))
            grp_idx[grp_cnt++] = i;
    }
    grp_idx[grp_cnt] = -1;

    if (!grp_cnt)
    {
        prt("你还没有发现任何 Ego 物品。按任意键继续。", 0, 0);
        inkey();
        prt("", 0, 0);
        C_KILL(ego_idx, max_e_idx, int);
        return;
    }

    ego_cnt = 0;

    old_grp_cur = -1;
    grp_cur = grp_top = 0;
    ego_cur = ego_top = 0;

    flag = FALSE;
    redraw = TRUE;

    while (!flag)
    {
        char ch;
        if (redraw)
        {
            clear_from(0);

            prt(format("%s - Ego物品", "知识"), 2, 0);
            prt("分组", 4, 0);
            prt("名称", 4, max + 3);
            prt("发现 购买 破坏", 4, max + 3 + 36);

            for (i = 0; i < 72; i++)
            {
                Term_putch(i, 5, TERM_WHITE, '=');
            }

            for (i = 0; i < browser_rows; i++)
            {
                Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
            }

            redraw = FALSE;
        }

        /* Scroll group list */
        if (grp_cur < grp_top) grp_top = grp_cur;
        if (grp_cur >= grp_top + browser_rows) grp_top = grp_cur - browser_rows + 1;

        /* Display a list of object groups */
        for (i = 0; i < browser_rows && grp_idx[i] >= 0; i++)
        {
            int  grp = grp_idx[grp_top + i];
            byte attr = (grp_top + i == grp_cur) ? TERM_L_BLUE : TERM_WHITE;

            Term_erase(0, 6 + i, max);
            c_put_str(attr, _ego_types[grp].name, 6 + i, 0);
        }

        if (old_grp_cur != grp_cur)
        {
            old_grp_cur = grp_cur;

            /* Get a list of objects in the current group */
            ego_cnt = _collect_egos(grp_idx[grp_cur], ego_idx) + 1;
        }

        /* Scroll object list */
        while (ego_cur < ego_top)
            ego_top = MAX(0, ego_top - browser_rows/2);
        while (ego_cur >= ego_top + browser_rows)
            ego_top = MIN(ego_cnt - browser_rows, ego_top + browser_rows/2);

        /* Display a list of objects in the current group */
        /* Display lines until done */
        for (i = 0; i < browser_rows && ego_top + i < ego_cnt && ego_idx[ego_top + i] >= 0; i++)
        {
            char           buf[255];
            char           name[255];
            int            idx = ego_idx[ego_top + i];
            ego_type      *e_ptr = &e_info[idx];
            byte           attr = TERM_WHITE;

            if (i + ego_top == ego_cur)
                attr = TERM_L_BLUE;

            strip_name_aux(name, e_name + e_ptr->name);
            if (e_ptr->type & (~_ego_types[grp_idx[grp_cur]].id))
                strcat(name, " [Shared]");

            sprintf(buf, "%-35.35s %5d %6d %4d",
                name,
                e_ptr->counts.found, e_ptr->counts.bought, e_ptr->counts.destroyed
            );
            c_prt(attr, buf, 6 + i, max + 3);
        }
        /* Total Line? */
        if (i < browser_rows && ego_idx[ego_top + i] < 0)
        {
            char     buf[255];
            counts_t totals = {0};
            int j;
            for (j = 0; ego_idx[j] >= 0; j++)
            {
                ego_type *e_ptr = &e_info[ego_idx[j]];
                totals.found += e_ptr->counts.found;
                totals.bought += e_ptr->counts.bought;
                totals.destroyed += e_ptr->counts.destroyed;
            }

            sprintf(buf, "%35.35s %5d %6d %4d",
                "总计",
                totals.found, totals.bought, totals.destroyed
            );
            c_prt(TERM_YELLOW, buf, 6 + i, max + 3);
            i++;
        }


        /* Clear remaining lines */
        for (; i < browser_rows; i++)
        {
            Term_erase(max + 3, 6 + i, 255);
        }

        prt("<dir>, 'r' 召回, ESC", hgt - 1, 0);

        if (!column)
        {
            Term_gotoxy(0, 6 + (grp_cur - grp_top));
        }
        else
        {
            Term_gotoxy(max + 3, 6 + (ego_cur - ego_top));
        }

        ch = inkey();

        switch (ch)
        {
        case ESCAPE:
            flag = TRUE;
            break;

        case 'R':
        case 'r':
        case 'I':
        case 'i':
            if (grp_cnt > 0 && ego_idx[ego_cur] >= 0)
            {
                desc_ego_fake(ego_idx[ego_cur]);
                redraw = TRUE;
            }
            break;

        default:
            browser_cursor(ch, &column, &grp_cur, grp_cnt, &ego_cur, ego_cnt);
        }
    }

    C_KILL(ego_idx, max_e_idx, int);
}


/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(bool *need_redraw, bool visual_only, int direct_k_idx)
{
    int i, len, max;
    int grp_cur, grp_top, old_grp_cur;
    int object_old, object_cur, object_top;
    int grp_cnt, grp_idx[100];
    int object_cnt;
    int *object_idx;

    int column = 0;
    bool flag;
    bool redraw;

    bool visual_list = FALSE;
    byte attr_top = 0, char_left = 0;

    int browser_rows;
    int wid, hgt;

    byte mode;

    /* Get size */
    Term_get_size(&wid, &hgt);

    browser_rows = hgt - 8;

    /* Allocate the "object_idx" array */
    C_MAKE(object_idx, max_k_idx, int);

    max = 0;
    grp_cnt = 0;

    if (direct_k_idx < 0)
    {
        mode = visual_only ? 0x03 : 0x01;

        /* Check every group */
        for (i = 0; object_group_text[i] != NULL; i++)
        {
            /* Measure the label */
            len = strlen(object_group_text[i]);

            /* Save the maximum length */
            if (len > max) max = len;

            /* See if any monsters are known */
            if (collect_objects(i, object_idx, mode))
            {
                /* Build a list of groups with known monsters */
                grp_idx[grp_cnt++] = i;
            }
        }

        object_old = -1;
        object_cnt = 0;
    }
    else
    {
        object_kind *k_ptr = &k_info[direct_k_idx];
        object_kind *flavor_k_ptr;

        if (!visual_only && k_ptr->flavor)
        {
            /* Appearance of this object is shuffled */
            flavor_k_ptr = &k_info[k_ptr->flavor];
        }
        else
        {
            /* Appearance of this object is very normal */
            flavor_k_ptr = k_ptr;
        }

        object_idx[0] = direct_k_idx;
        object_old = direct_k_idx;
        object_cnt = 1;

        /* Terminate the list */
        object_idx[1] = -1;

        (void)visual_mode_command('v', &visual_list, browser_rows - 1, wid - (max + 3),
            &attr_top, &char_left, &flavor_k_ptr->x_attr, &flavor_k_ptr->x_char, need_redraw);
    }

    /* Terminate the list */
    grp_idx[grp_cnt] = -1;

    old_grp_cur = -1;
    grp_cur = grp_top = 0;
    object_cur = object_top = 0;

    flag = FALSE;
    redraw = TRUE;

    mode = visual_only ? 0x02 : 0x00;

    while (!flag)
    {
        char ch;
        object_kind *k_ptr = NULL, *flavor_k_ptr = NULL;

        if (redraw)
        {
            clear_from(0);

            prt(format("%s - 物品", !visual_only ? "知识" : "视觉(Visuals)"), 2, 0);
            if (direct_k_idx < 0) prt("分组", 4, 0);
            prt("名称", 4, max + 3);
            if (visual_only) prt("序号(Idx)", 4, 70);
            prt("发现 购买 使用 破坏 字符", 4, 52);

            for (i = 0; i < 78; i++)
            {
                Term_putch(i, 5, TERM_WHITE, '=');
            }

            if (direct_k_idx < 0)
            {
                for (i = 0; i < browser_rows; i++)
                {
                    Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
                }
            }

            redraw = FALSE;
        }

        if (direct_k_idx < 0)
        {
            /* Scroll group list */
            if (grp_cur < grp_top) grp_top = grp_cur;
            if (grp_cur >= grp_top + browser_rows) grp_top = grp_cur - browser_rows + 1;

            /* Display a list of object groups */
            display_group_list(0, 6, max, browser_rows, grp_idx, object_group_text, grp_cur, grp_top);

            if (old_grp_cur != grp_cur)
            {
                old_grp_cur = grp_cur;

                /* Get a list of objects in the current group */
                object_cnt = collect_objects(grp_idx[grp_cur], object_idx, mode) + 1;
            }

            /* Scroll object list */
            while (object_cur < object_top)
                object_top = MAX(0, object_top - browser_rows/2);
            while (object_cur >= object_top + browser_rows)
                object_top = MIN(object_cnt - browser_rows, object_top + browser_rows/2);
        }

        if (!visual_list)
        {
            /* Display a list of objects in the current group */
            display_object_list(max + 3, 6, browser_rows, object_idx, object_cur, object_top, object_cnt, visual_only);
        }
        else
        {
            object_top = object_cur;

            /* Display a list of objects in the current group */
            display_object_list(max + 3, 6, 1, object_idx, object_cur, object_top, object_cnt, visual_only);

            /* Display visual list below first object */
            display_visual_list(max + 3, 7, browser_rows-1, wid - (max + 3), attr_top, char_left);
        }

        /* Get the current object */
        if (object_idx[object_cur] >= 0)
        {
            k_ptr = &k_info[object_idx[object_cur]];

            if (!visual_only && k_ptr->flavor)
            {
                /* Appearance of this object is shuffled */
                flavor_k_ptr = &k_info[k_ptr->flavor];
            }
            else
            {
                /* Appearance of this object is very normal */
                flavor_k_ptr = k_ptr;
            }
        }
        else
        {
            k_ptr = NULL;
            flavor_k_ptr = NULL;
        }

        /* Prompt */
        prt(format("<dir>%s%s%s, ESC",
            (!visual_list && !visual_only) ? "，'r' 回忆" : "",
            visual_list ? "，回车 接受" : "，'v' 视觉效果",
            (attr_idx || char_idx) ? "，'c', 'p' 粘贴" : "，'c' 复制"),
            hgt - 1, 0);

        if (!visual_only && object_idx[object_cur] >= 0)
        {
            /* Mega Hack -- track this object */
            if (object_cnt)
                object_kind_track(object_idx[object_cur]);

            /* The "current" object changed */
            if (object_old != object_idx[object_cur])
            {
                /* Hack -- handle stuff */
                handle_stuff();

                /* Remember the "current" object */
                object_old = object_idx[object_cur];
            }
        }

        if (visual_list && flavor_k_ptr)
        {
            place_visual_list_cursor(max + 3, 7, flavor_k_ptr->x_attr, flavor_k_ptr->x_char, attr_top, char_left);
        }
        else if (!column)
        {
            Term_gotoxy(0, 6 + (grp_cur - grp_top));
        }
        else
        {
            Term_gotoxy(max + 3, 6 + (object_cur - object_top));
        }

        ch = inkey();

        /* Do visual mode command if needed */
        if (flavor_k_ptr && visual_mode_command(ch, &visual_list, browser_rows-1, wid - (max + 3), &attr_top, &char_left, &flavor_k_ptr->x_attr, &flavor_k_ptr->x_char, need_redraw))
        {
            if (direct_k_idx >= 0)
            {
                switch (ch)
                {
                case '\n':
                case '\r':
                case ESCAPE:
                    flag = TRUE;
                    break;
                }
            }
            continue;
        }

        switch (ch)
        {
            case ESCAPE:
            {
                flag = TRUE;
                break;
            }

            case 'R':
            case 'r':
            {
                /* Recall on screen */
                if (!visual_list && !visual_only && (grp_cnt > 0) && object_idx[object_cur] >= 0)
                {
                    desc_obj_fake(object_idx[object_cur]);
                    redraw = TRUE;
                }
                break;
            }

            default:
            {
                /* Move the cursor */
                browser_cursor(ch, &column, &grp_cur, grp_cnt, &object_cur, object_cnt);
                break;
            }
        }
    }

    /* Free the "object_idx" array */
    C_KILL(object_idx, max_k_idx, int);
}


/*
 * Display the features in a group.
 */
static void display_feature_list(int col, int row, int per_page, int *feat_idx,
    int feat_cur, int feat_top, bool visual_only, int lighting_level)
{
    int lit_col[F_LIT_MAX], i, j;
    int f_idx_col = use_bigtile ? 62 : 64;

    /* Correct columns 1 and 4 */
    lit_col[F_LIT_STANDARD] = use_bigtile ? (71 - F_LIT_MAX) : 71;
    for (i = F_LIT_NS_BEGIN; i < F_LIT_MAX; i++)
        lit_col[i] = lit_col[F_LIT_STANDARD] + 2 + (i - F_LIT_NS_BEGIN) * 2 + (use_bigtile ? i : 0);

    /* Display lines until done */
    for (i = 0; i < per_page && (feat_idx[feat_top + i] >= 0); i++)
    {
        byte attr;

        /* Get the index */
        int f_idx = feat_idx[feat_top + i];

        /* Access the index */
        feature_type *f_ptr = &f_info[f_idx];

        int row_i = row + i;

        /* Choose a color */
        attr = ((i + feat_top == feat_cur) ? TERM_L_BLUE : TERM_WHITE);

        /* Display the name */
        c_prt(attr, f_name + f_ptr->name, row_i, col);

        /* Hack -- visual_list mode */
        if (per_page == 1)
        {
            /* Display lighting level */
            c_prt(attr, format("(%s)", lighting_level_str[lighting_level]), row_i, col + 1 + strlen(f_name + f_ptr->name));

            c_prt(attr, format("%02x/%02x", f_ptr->x_attr[lighting_level], f_ptr->x_char[lighting_level]), row_i, f_idx_col - ((p_ptr->wizard || visual_only) ? 6 : 2));
        }
        if (p_ptr->wizard || visual_only)
        {
            c_prt(attr, format("%d", f_idx), row_i, f_idx_col);
        }

        /* Display symbol */
        Term_queue_bigchar(lit_col[F_LIT_STANDARD], row_i, f_ptr->x_attr[F_LIT_STANDARD], f_ptr->x_char[F_LIT_STANDARD], 0, 0);

        Term_putch(lit_col[F_LIT_NS_BEGIN], row_i, TERM_SLATE, '(');
        for (j = F_LIT_NS_BEGIN + 1; j < F_LIT_MAX; j++)
        {
            Term_putch(lit_col[j], row_i, TERM_SLATE, '/');
        }
        Term_putch(lit_col[F_LIT_MAX - 1] + (use_bigtile ? 3 : 2), row_i, TERM_SLATE, ')');

        /* Mega-hack -- Use non-标准 colour */
        for (j = F_LIT_NS_BEGIN; j < F_LIT_MAX; j++)
        {
            Term_queue_bigchar(lit_col[j] + 1, row_i, f_ptr->x_attr[j], f_ptr->x_char[j], 0, 0);
        }
    }

    /* Clear remaining lines */
    for (; i < per_page; i++)
    {
        Term_erase(col, row + i, 255);
    }
}


/*
 * Interact with feature visuals.
 */
static void do_cmd_knowledge_features(bool *need_redraw, bool visual_only, int direct_f_idx, int *lighting_level)
{
    int i, len, max;
    int grp_cur, grp_top, old_grp_cur;
    int feat_cur, feat_top;
    int grp_cnt, grp_idx[100];
    int feat_cnt;
    int *feat_idx;

    int column = 0;
    bool flag;
    bool redraw;

    bool visual_list = FALSE;
    byte attr_top = 0, char_left = 0;

    int browser_rows;
    int wid, hgt;

    byte attr_old[F_LIT_MAX];
    byte char_old[F_LIT_MAX];
    byte *cur_attr_ptr, *cur_char_ptr;

    C_WIPE(attr_old, F_LIT_MAX, byte);
    C_WIPE(char_old, F_LIT_MAX, byte);

    /* Get size */
    Term_get_size(&wid, &hgt);

    browser_rows = hgt - 8;

    /* Allocate the "feat_idx" array */
    C_MAKE(feat_idx, max_f_idx, int);

    max = 0;
    grp_cnt = 0;

    if (direct_f_idx < 0)
    {
        /* Check every group */
        for (i = 0; feature_group_text[i] != NULL; i++)
        {
            /* Measure the label */
            len = strlen(feature_group_text[i]);

            /* Save the maximum length */
            if (len > max) max = len;

            /* See if any features are known */
            if (collect_features(i, feat_idx, 0x01))
            {
                /* Build a list of groups with known features */
                grp_idx[grp_cnt++] = i;
            }
        }

        feat_cnt = 0;
    }
    else
    {
        feature_type *f_ptr = &f_info[direct_f_idx];

        feat_idx[0] = direct_f_idx;
        feat_cnt = 1;

        /* Terminate the list */
        feat_idx[1] = -1;

        (void)visual_mode_command('v', &visual_list, browser_rows - 1, wid - (max + 3),
            &attr_top, &char_left, &f_ptr->x_attr[*lighting_level], &f_ptr->x_char[*lighting_level], need_redraw);

        for (i = 0; i < F_LIT_MAX; i++)
        {
            attr_old[i] = f_ptr->x_attr[i];
            char_old[i] = f_ptr->x_char[i];
        }
    }

    /* Terminate the list */
    grp_idx[grp_cnt] = -1;

    old_grp_cur = -1;
    grp_cur = grp_top = 0;
    feat_cur = feat_top = 0;

    flag = FALSE;
    redraw = TRUE;

    while (!flag)
    {
        char ch;
        feature_type *f_ptr;

        if (redraw)
        {
            clear_from(0);

            prt("视觉(Visuals) - 地形", 2, 0);
            if (direct_f_idx < 0) prt("分组", 4, 0);
            prt("名称", 4, max + 3);
            if (use_bigtile)
            {
                if (p_ptr->wizard || visual_only) prt("序号(Idx)", 4, 62);
                prt("字符(亮/暗)", 4, 67);
            }
            else
            {
                if (p_ptr->wizard || visual_only) prt("序号(Idx)", 4, 64);
                prt("字符(亮/暗)", 4, 69);
            }

            for (i = 0; i < 78; i++)
            {
                Term_putch(i, 5, TERM_WHITE, '=');
            }

            if (direct_f_idx < 0)
            {
                for (i = 0; i < browser_rows; i++)
                {
                    Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
                }
            }

            redraw = FALSE;
        }

        if (direct_f_idx < 0)
        {
            /* Scroll group list */
            if (grp_cur < grp_top) grp_top = grp_cur;
            if (grp_cur >= grp_top + browser_rows) grp_top = grp_cur - browser_rows + 1;

            /* Display a list of feature groups */
            display_group_list(0, 6, max, browser_rows, grp_idx, feature_group_text, grp_cur, grp_top);

            if (old_grp_cur != grp_cur)
            {
                old_grp_cur = grp_cur;

                /* Get a list of features in the current group */
                feat_cnt = collect_features(grp_idx[grp_cur], feat_idx, 0x00);
            }

            /* Scroll feature list */
            while (feat_cur < feat_top)
                feat_top = MAX(0, feat_top - browser_rows/2);
            while (feat_cur >= feat_top + browser_rows)
                feat_top = MIN(feat_cnt - browser_rows, feat_top + browser_rows/2);
        }

        if (!visual_list)
        {
            /* Display a list of features in the current group */
            display_feature_list(max + 3, 6, browser_rows, feat_idx, feat_cur, feat_top, visual_only, F_LIT_STANDARD);
        }
        else
        {
            feat_top = feat_cur;

            /* Display a list of features in the current group */
            display_feature_list(max + 3, 6, 1, feat_idx, feat_cur, feat_top, visual_only, *lighting_level);

            /* Display visual list below first object */
            display_visual_list(max + 3, 7, browser_rows-1, wid - (max + 3), attr_top, char_left);
        }

        /* Prompt */
        prt(format("<dir>%s, 'd' 恢复默认光照%s, ESC",
            visual_list ? "，回车 接受，'a' 光照等级" : "，'v' 视觉效果",
            (attr_idx || char_idx) ? "，'c', 'p' 粘贴" : "，'c' 复制"),
            hgt - 1, 0);

        /* Get the current feature */
        f_ptr = &f_info[feat_idx[feat_cur]];
        cur_attr_ptr = &f_ptr->x_attr[*lighting_level];
        cur_char_ptr = &f_ptr->x_char[*lighting_level];

        if (visual_list)
        {
            place_visual_list_cursor(max + 3, 7, *cur_attr_ptr, *cur_char_ptr, attr_top, char_left);
        }
        else if (!column)
        {
            Term_gotoxy(0, 6 + (grp_cur - grp_top));
        }
        else
        {
            Term_gotoxy(max + 3, 6 + (feat_cur - feat_top));
        }

        ch = inkey();

        if (visual_list && ((ch == 'A') || (ch == 'a')))
        {
            int prev_lighting_level = *lighting_level;

            if (ch == 'A')
            {
                if (*lighting_level <= 0) *lighting_level = F_LIT_MAX - 1;
                else (*lighting_level)--;
            }
            else
            {
                if (*lighting_level >= F_LIT_MAX - 1) *lighting_level = 0;
                else (*lighting_level)++;
            }

            if (f_ptr->x_attr[prev_lighting_level] != f_ptr->x_attr[*lighting_level])
                attr_top = MAX(0, (f_ptr->x_attr[*lighting_level] & 0x7f) - 5);

            if (f_ptr->x_char[prev_lighting_level] != f_ptr->x_char[*lighting_level])
                char_left = MAX(0, f_ptr->x_char[*lighting_level] - 10);

            continue;
        }

        else if ((ch == 'D') || (ch == 'd'))
        {
            byte prev_x_attr = f_ptr->x_attr[*lighting_level];
            byte prev_x_char = f_ptr->x_char[*lighting_level];

            apply_default_feat_lighting(f_ptr->x_attr, f_ptr->x_char);

            if (visual_list)
            {
                if (prev_x_attr != f_ptr->x_attr[*lighting_level])
                     attr_top = MAX(0, (f_ptr->x_attr[*lighting_level] & 0x7f) - 5);

                if (prev_x_char != f_ptr->x_char[*lighting_level])
                    char_left = MAX(0, f_ptr->x_char[*lighting_level] - 10);
            }
            else *need_redraw = TRUE;

            continue;
        }

        /* Do visual mode command if needed */
        else
        {
            byte old_standard_char = f_ptr->x_char[F_LIT_STANDARD];

            if (visual_mode_command(ch, &visual_list, browser_rows-1, wid - (max + 3), &attr_top, &char_left, cur_attr_ptr, cur_char_ptr, need_redraw))
            {
                if ((*lighting_level == F_LIT_STANDARD) && (f_ptr->x_char[F_LIT_STANDARD] != old_standard_char))
                    _sync_feature_lighting_chars(f_ptr, old_standard_char, f_ptr->x_char[F_LIT_STANDARD]);

                switch (ch)
                {
                /* Restore previous visual settings */
                case ESCAPE:
                    for (i = 0; i < F_LIT_MAX; i++)
                    {
                        f_ptr->x_attr[i] = attr_old[i];
                        f_ptr->x_char[i] = char_old[i];
                    }

                    /* Fall through */

                case '\n':
                case '\r':
                    if (direct_f_idx >= 0) flag = TRUE;
                    else *lighting_level = F_LIT_STANDARD;
                    break;

                /* Preserve current visual settings */
                case 'V':
                case 'v':
                    for (i = 0; i < F_LIT_MAX; i++)
                    {
                        attr_old[i] = f_ptr->x_attr[i];
                        char_old[i] = f_ptr->x_char[i];
                    }
                    *lighting_level = F_LIT_STANDARD;
                    break;

                case 'C':
                case 'c':
                    if (!visual_list)
                    {
                        for (i = 0; i < F_LIT_MAX; i++)
                        {
                            attr_idx_feat[i] = f_ptr->x_attr[i];
                            char_idx_feat[i] = f_ptr->x_char[i];
                        }
                    }
                    break;

                case 'P':
                case 'p':
                    if (!visual_list)
                    {
                        /* Allow TERM_DARK text */
                        for (i = F_LIT_NS_BEGIN; i < F_LIT_MAX; i++)
                        {
                            if (attr_idx_feat[i] || (!(char_idx_feat[i] & 0x80) && char_idx_feat[i])) f_ptr->x_attr[i] = attr_idx_feat[i];
                            if (char_idx_feat[i]) f_ptr->x_char[i] = char_idx_feat[i];
                        }
                    }
                    break;
                }
                continue;
            }

            switch (ch)
            {
            case ESCAPE:
                {
                    flag = TRUE;
                    break;
                }

            default:
                {
                    /* Move the cursor */
                    browser_cursor(ch, &column, &grp_cur, grp_cnt, &feat_cur, feat_cnt);
                    break;
                }
            }
        }
    }

    /* Free the "feat_idx" array */
    C_KILL(feat_idx, max_f_idx, int);
}


/*
 * List wanted monsters
 */
static void do_cmd_knowledge_kubi(void)
{
    int i;
    FILE *fff;

    char file_name[1024];


    /* Open a new file */
    fff = my_fopen_temp(file_name, 1024);
    if (!fff) {
        msg_format("无法创建临时文件 %s。", file_name);
        msg_print(NULL);
        return;
    }

    if (fff)
    {
        bool listed = FALSE;

        fprintf(fff, "今日目标：%s\n", (p_ptr->today_mon ? monster_race_display_name(p_ptr->today_mon) : "未知"));
        fprintf(fff, "\n");
        fprintf(fff, "通缉怪物列表\n");
        fprintf(fff, "----------------------------------------------\n");

        for (i = 0; i < MAX_KUBI; i++)
        {
            int id = kubi_r_idx[i];
            if (0 < id && id < 10000)
            {
                fprintf(fff,"%s\n", monster_race_display_name(id));
                listed = TRUE;
            }
        }

        if (!listed)
        {
            fprintf(fff,"\n%s\n", "你已经上交了所有被悬赏的怪物。");
        }
    }

    /* Close the file */
    my_fclose(fff);

    /* Display the file contents */
    show_file(TRUE, file_name, "通缉怪物", 0, 0);


    /* Remove the file */
    fd_kill(file_name);
}

/*
 * List virtues & status
 */

static void do_cmd_knowledge_virtues(void)
{
    doc_ptr doc = doc_alloc(80);

    virtue_display(doc);
    doc_display(doc, "美德(Virtues)", 0);
    doc_free(doc);
}

/*
* Dungeon
*
*/
static void do_cmd_knowledge_dungeon(void)
{
    doc_ptr doc = doc_alloc(80);

    py_display_dungeons(doc);
    doc_display(doc, "地下城(Dungeons)", 0);
    doc_free(doc);
}

static void do_cmd_knowledge_stat(void)
{
    doc_ptr          doc = doc_alloc(80);
    race_t          *race_ptr = get_race();
    class_t         *class_ptr = get_class();
    personality_ptr  pers_ptr = get_personality();
    int              i;

    if (p_ptr->knowledge & KNOW_HPRATE)
        doc_printf(doc, "你当前的生命评级(Life Rating)是 %s。\n\n", life_rating_desc(TRUE));
    else
        doc_insert(doc, "你当前的生命评级(Life Rating)是 <color:y>???</color>。\n\n");

    doc_insert(doc, "<color:r>最大属性上限</color>\n");

    for (i = 0; i < MAX_STATS; i++)
    {
        if ((p_ptr->knowledge & KNOW_STAT) || p_ptr->stat_max[i] == p_ptr->stat_max_max[i])
        {
            if (decimal_stats)
                doc_printf(doc, "%s : <color:G>%d</color>\n", _stat_label_zh(i), (p_ptr->stat_max_max[i]-18)/10+18);
            else doc_printf(doc, "%s : <color:G>18/%d</color>\n", _stat_label_zh(i), p_ptr->stat_max_max[i]-18);
        }
        else
            doc_printf(doc, "%s : <color:y>\?\?\?</color>\n", _stat_label_zh(i));
    }
    doc_insert(doc, "\n\n");

    doc_printf(doc, "<color:r>种族:</color> <color:B>%s</color>\n", race_ptr->name);
    doc_insert(doc, race_ptr->desc);
    if (p_ptr->pclass == CLASS_MONSTER)
        doc_printf(doc, "更多信息，请参见 <link:MonsterRaces.txt#%s>。\n\n", race_ptr->name);
    else
        doc_printf(doc, "更多信息，请参见 <link:Races.txt#%s>。\n\n", race_ptr->name);

    if (race_ptr->subdesc && strlen(race_ptr->subdesc))
    {
        doc_printf(doc, "<color:r>亚种:</color> <color:B>%s</color>\n", race_ptr->subname);
        doc_insert(doc, race_ptr->subdesc);
        doc_insert(doc, "\n\n");
    }

    if (p_ptr->pclass != CLASS_MONSTER)
    {
        doc_printf(doc, "<color:r>职业:</color> <color:B>%s</color>\n", class_ptr->name);
        doc_insert(doc, class_ptr->desc);
        doc_printf(doc, "更多信息，请参见 <link:Classes.txt#%s>。\n\n", class_ptr->name);
    }

    doc_printf(doc, "<color:r>性格:</color> <color:B>%s</color>\n", pers_ptr->name);
    doc_insert(doc, pers_ptr->desc);
    doc_printf(doc, "更多信息，请参见 <link:Personalities.txt#%s>。\n\n", pers_ptr->name);

    if (p_ptr->realm1)
    {
        doc_printf(doc, "<color:r>法术领域:</color> <color:B>%s</color>\n", realm_names[p_ptr->realm1]);
        doc_insert(doc, realm_jouhou[technic2magic(p_ptr->realm1)-1]);
        doc_insert(doc, "\n\n");
    }

    if (p_ptr->realm2)
    {
        doc_printf(doc, "<color:r>法术领域:</color> <color:B>%s</color>\n", realm_names[p_ptr->realm2]);
        doc_insert(doc, realm_jouhou[technic2magic(p_ptr->realm2)-1]);
        doc_insert(doc, "\n\n");
    }

    doc_display(doc, "自我知识", 0);
    doc_free(doc);
}

/*
 * Check the status of "autopick"
 */
static void do_cmd_knowledge_autopick(void)
{
    int k;
    doc_ptr doc = doc_alloc(80);

    if (no_mogaminator)
    {
        doc_insert(doc, "你已禁用墨家明器。\n");
    }
    else if (!max_autopick)
    {
        doc_insert(doc, "你尚未启用墨家明器。\n");
    }
    else if (max_autopick == 1)
    {
        doc_insert(doc, "有 1 条用于自动物品管理的注册规则。\n");
    }
    else
    {
        doc_printf(doc, "有 %d 条用于自动物品管理的注册规则。\n", max_autopick);
    }
    doc_insert(doc, "墨家明器帮助见 <link:editor.txt>。\n\n");

    if (!no_mogaminator)
    {
        for (k = 0; k < max_autopick; k++)
        {
            cptr tmp;
            string_ptr line = 0;
            char color = 'w';
            byte act = autopick_list[k].action;
            if (act & DONT_AUTOPICK)
            {
                tmp = "保留";
                color = 'U';
            }
            else if (act & DO_AUTODESTROY)
            {
                tmp = "破坏";
                color = 'r';
            }
            else if (act & DO_AUTOPICK)
            {
                tmp = "拾取";
                color = 'B';
            }
            else /* if (act & DO_QUERY_AUTOPICK) */ /* Obvious */
            {
                tmp = "询问";
                color = 'y';
            }

            if (act & DO_DISPLAY)
                doc_printf(doc, "<color:%c>%-9.9s</color>", color, format("[%s]", tmp));
            else
                doc_printf(doc, "<color:%c>%-9.9s</color>", color, format("(%s)", tmp));

            line = autopick_line_from_entry(&autopick_list[k], AUTOPICK_COLOR_CODED);
            doc_printf(doc, " <indent><style:indent>%s</style></indent>\n", string_buffer(line));
            string_free(line);
        }
    }

    doc_display(doc, "墨家明器偏好", 0);
    doc_free(doc);
}


/*
 * Interact with "knowledge"
 */
void do_cmd_knowledge(void)
{
    int      i, row, col;
    bool     need_redraw = FALSE;
    class_t *class_ptr = get_class();
    race_t  *race_ptr = get_race();

    screen_save();

    while (1)
    {
        Term_clear();

        prt("显示当前知识", 2, 0);

        /* Give some choices */
        row = 4;
        col = 2;
        c_prt(TERM_RED, "物品知识", row++, col - 2);
        prt("(a) 神器", row++, col);
        prt("(o) 物品", row++, col);
        prt("(e) Ego物品", row++, col);
        prt("(_) 自动拾取/破坏", row++, col);
        row++;

        c_prt(TERM_RED, "怪物知识", row++, col - 2);
        prt("(m) 已知怪物", row++, col);
        prt("(w) 通缉怪物", row++, col);
        prt("(u) 剩余唯一怪物", row++, col);
        prt("(k) 击杀统计", row++, col);
        prt("(p) 宠物", row++, col);
        row++;

        row = 4;
        col = 30;

        c_prt(TERM_RED, "地下城知识", row++, col - 2);
        prt("(d) 地下城", row++, col);
        prt("(q) 任务", row++, col);
        prt("(t) 地形字符", row++, col);
        row++;

        c_prt(TERM_RED, "自我知识", row++, col - 2);
        prt("(@) 关于你自己", row++, col);
        if (p_ptr->prace != RACE_MON_RING)
            prt("(W) 武器伤害", row++, col);
        if (equip_find_obj(TV_BOW, SV_ANY) && !prace_is_(RACE_MON_JELLY) && p_ptr->shooter_info.tval_ammo != TV_NO_AMMO)
            prt("(S) 射击伤害", row++, col);
        if (mut_count(NULL))
            prt("(M) 突变", row++, col);
        if (enable_virtues)
            prt("(v) 美德", row++, col);
        if (class_ptr->character_dump || race_ptr->character_dump)
            prt("(x) 额外信息", row++, col);
        prt("(H) 高分榜", row++, col);
        row++;

        c_prt(TERM_RED, "技能", row++, col - 2);
        prt("(P) 武器熟练度", row++, col);
        if (p_ptr->pclass != CLASS_RAGE_MAGE) /* TODO */
            prt("(s) 法术熟练度", row++, col);
        row++;

        /* Prompt */
        prt("ESC) 退出菜单", 21, 1);
        prt("命令: ", 20, 0);

        /* Prompt */
        i = inkey();

        /* Done */
        if (i == ESCAPE) break;
        switch (i)
        {
        /* Object Knowledge */
        case 'a':
            do_cmd_knowledge_artifacts();
            break;
        case 'o':
            do_cmd_knowledge_objects(&need_redraw, FALSE, -1);
            break;
        case 'e':
            do_cmd_knowledge_egos();
            break;
        case '_':
            do_cmd_knowledge_autopick();
            break;

        /* Monster Knowledge */
        case 'm':
            do_cmd_knowledge_monsters(&need_redraw, FALSE, -1);
            break;
        case 'w':
            do_cmd_knowledge_kubi();
            break;
        case 'u':
            do_cmd_knowledge_uniques();
            break;
        case 'k':
            do_cmd_knowledge_kill_count();
            break;
        case 'p':
            do_cmd_knowledge_pets();
            break;

        /* Dungeon Knowledge */
        case 'd':
            do_cmd_knowledge_dungeon();
            break;
        case 'q':
            quests_display();
            break;
        case 't':
            {
                int lighting_level = F_LIT_STANDARD;
                do_cmd_knowledge_features(&need_redraw, FALSE, -1, &lighting_level);
            }
            break;

        /* Self Knowledge */
        case '@':
            do_cmd_knowledge_stat();
            break;
        case 'W':
            if (p_ptr->prace != RACE_MON_RING)
                do_cmd_knowledge_weapon();
            else
                bell();
            break;
        case 'S':
            if (equip_find_obj(TV_BOW, SV_ANY) && !prace_is_(RACE_MON_JELLY) && p_ptr->shooter_info.tval_ammo != TV_NO_AMMO)
                do_cmd_knowledge_shooter();
            else
                bell();
            break;
        case 'M':
            if (mut_count(NULL))
                mut_do_cmd_knowledge();
            else
                bell();
            break;
        case 'v':
            if (enable_virtues)
                do_cmd_knowledge_virtues();
            else
                bell();
            break;
        case 'x':
            if (class_ptr->character_dump || race_ptr->character_dump)
                do_cmd_knowledge_extra();
            else
                bell();
            break;
        case 'H': {
            vec_ptr scores;
            if (check_score())
                scores_update();
            scores = scores_load(NULL);
            scores_display(scores);
            vec_free(scores);
            break; }

        /* Skills */
        case 'P':
            do_cmd_knowledge_weapon_exp();
            break;
        case 's':
            if (p_ptr->pclass != CLASS_RAGE_MAGE)  /* TODO */
                do_cmd_knowledge_spell_exp();
            break;

        default:
            bell();
        }

        /* Flush messages */
        msg_print(NULL);
    }

    /* Restore the screen */
    screen_load();

    if (need_redraw) do_cmd_redraw();
}

/*
 * Display the time and date
 */
void do_cmd_time(void)
{
    int day, hour, min, full, start, end, num;
    char desc[1024];

    char buf[1024];
    char day_buf[10];

    FILE *fff;

    extract_day_hour_min(&day, &hour, &min);

    full = hour * 100 + min;

    start = 9999;
    end = -9999;

    num = 0;

    strcpy(desc, "这是一个奇怪的时间。");


    if (day < MAX_DAYS) sprintf(day_buf, "%d", day);
    else strcpy(day_buf, "*****");

    /* Message */
    msg_format("今天是第 %s 天。时间是 %d:%02d %s。",
           day_buf, (hour % 12 == 0) ? 12 : (hour % 12),
           min, (hour < 12) ? "上午" : "下午");


    /* Find the path */
    if (!randint0(10) || p_ptr->image)
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "timefun.txt");

    }
    else
    {
        path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, "timenorm.txt");

    }

    /* Open this file */
    fff = my_fopen(buf, "rt");

    /* Oops */
    if (!fff) return;

    /* Find this time */
    while (!my_fgets(fff, buf, sizeof(buf)))
    {
        /* Ignore comments */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Ignore invalid lines */
        if (buf[1] != ':') continue;

        /* Process 'Start' */
        if (buf[0] == 'S')
        {
            /* Extract the starting time */
            start = atoi(buf + 2);

            /* Assume valid for an hour */
            end = start + 59;

            /* Next... */
            continue;
        }

        /* Process 'End' */
        if (buf[0] == 'E')
        {
            /* Extract the ending time */
            end = atoi(buf + 2);

            /* Next... */
            continue;
        }

        /* Ignore incorrect range */
        if ((start > full) || (full > end)) continue;

        /* Process 'Description' */
        if (buf[0] == 'D')
        {
            num++;

            /* Apply the randomizer */
            if (!randint0(num)) strcpy(desc, buf + 2);

            /* Next... */
            continue;
        }
    }

    if (p_ptr->prace == RACE_WEREWOLF)
    {
        strcat(desc, werewolf_moon_message());
    }

    /* Message */
    msg_print(desc);

    /* Close the file */
    my_fclose(fff);
}
