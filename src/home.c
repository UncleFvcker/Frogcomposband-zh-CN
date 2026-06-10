#include "angband.h"
#include "shop.h"

#include <assert.h>

static inv_ptr _home = NULL;
static inv_ptr _museum = NULL;

#define MUSEUM_SHARED_FILE "museum.txt"

/* Function to convert a hex string representation back to an object
   This is useful for debugging or object deserialization */
bool hex_to_obj(cptr hex_str, obj_ptr obj)
{
    byte *p = (byte *)obj;
    size_t i, len;
    
    /* Check for NULL pointers */
    if (!hex_str || !obj) return FALSE;
    
    /* Get the string length */
    len = strlen(hex_str);
    
    /* Ensure the string is valid (even length, contains only hex chars) */
    if (len % 2 != 0) return FALSE;
    
    /* Clear the object first */
    object_wipe(obj);
    
    /* Convert hex pairs back to bytes */
    for (i = 0; i < len / 2 && i < sizeof(obj_t); i++) 
    {
        byte hi, lo;
        
        /* Extract hex digits */
        if (hex_str[i*2] >= '0' && hex_str[i*2] <= '9')
            hi = hex_str[i*2] - '0';
        else if (hex_str[i*2] >= 'A' && hex_str[i*2] <= 'F')
            hi = hex_str[i*2] - 'A' + 10;
        else if (hex_str[i*2] >= 'a' && hex_str[i*2] <= 'f')
            hi = hex_str[i*2] - 'a' + 10;
        else
            return FALSE;

        if (hex_str[i*2+1] >= '0' && hex_str[i*2+1] <= '9')
            lo = hex_str[i*2+1] - '0';
        else if (hex_str[i*2+1] >= 'A' && hex_str[i*2+1] <= 'F')
            lo = hex_str[i*2+1] - 'A' + 10;
        else if (hex_str[i*2+1] >= 'a' && hex_str[i*2+1] <= 'f')
            lo = hex_str[i*2+1] - 'a' + 10;
        else
            return FALSE;
            
        /* Combine the nibbles */
        p[i] = (hi << 4) | lo;
    }
    
    /* Success */
    return TRUE;
}

/* Function to dump an object to a hex string representation
   This is useful for debugging or object serialization */
void obj_dump_hex(obj_ptr obj, char *buf, size_t max)
{
    byte *p = (byte *)obj;
    size_t i, size = sizeof(obj_t);
    char *q = buf;
    
    /* Ensure we have at least some space */
    if (max < 3) {
        if (max > 0) *buf = '\0';
        return;
    }
    
    /* Cap at maximum available buffer space, allowing for null termination */
    if (size * 2 >= max) size = (max - 1) / 2;
    
    /* Convert each byte to a 2-digit hex representation */
    for (i = 0; i < size; i++)
    {
        byte b = *p++;
        *q++ = hexsym[(b & 0xF0) >> 4];
        *q++ = hexsym[b & 0x0F];
    }
    
    /* Null terminate the string */
    *q = '\0';
}

void home_init(void)
{
    inv_free(_home);
    inv_free(_museum);

    _home = inv_alloc("家", INV_HOME, 0);
    _museum = inv_alloc("博物馆", INV_MUSEUM, 0);
}

inv_ptr home_filter(obj_p p)
{
    return inv_filter(_home, p);
}

obj_ptr home_obj(slot_t slot)
{
    return inv_obj(_home, slot);
}

int home_max(void)
{
    return inv_max(_home);
}

void home_for_each(obj_f f)
{
    inv_for_each(_home, f);
}

void home_optimize(void)
{
    inv_optimize(_home);
}

void home_carry(obj_ptr obj)
{
    if (obj->number)
        inv_combine_ex(_home, obj);
    if (obj->number)
        inv_add(_home, obj);
}

static void museum_carry(obj_ptr obj)
{

    obj->inscription = 0; // Clear inscription
    if (obj->number)
        inv_combine_ex(_museum, obj);
    if (obj->number)
        inv_add(_museum, obj);
}

static void _museum_reset(void)
{
    inv_free(_museum);
    _museum = inv_alloc("博物馆", INV_MUSEUM, 0);
}

static void _museum_path(char *buf, size_t max)
{
    path_build(buf, max, ANGBAND_DIR_USER, MUSEUM_SHARED_FILE);
}

static void _strip_newline(char *buf)
{
    char *pos = strchr(buf, '\n');
    if (pos) *pos = '\0';
    pos = strchr(buf, '\r');
    if (pos) *pos = '\0';
}

static bool _save_museum_data(void)
{
    char path[1024];
    FILE *fp;
    slot_t slot;
    slot_t max;
    bool success = TRUE;

    _museum_path(path, sizeof(path));

    safe_setuid_grab();
    fp = my_fopen(path, "w");
    safe_setuid_drop();

    if (!fp) return FALSE;

    fprintf(fp, "# RoguelikeFansBand local shared museum v1\n");

    max = inv_last(_museum, obj_exists);
    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(_museum, slot);
        char hex_buf[2048];
        cptr art_name = "";

        if (!obj) continue;

        obj_dump_hex(obj, hex_buf, sizeof(hex_buf));
        if (obj->art_name) art_name = quark_str(obj->art_name);
        if (fprintf(fp, "%s|%d|%s\n", hex_buf, obj->number, art_name ? art_name : "") < 0)
        {
            success = FALSE;
            break;
        }
    }

    if (my_fclose(fp)) success = FALSE;
    return success;
}

bool _fetch_museum_data(void)
{
    char path[1024];
    char line[4096];
    FILE *fp;

    _museum_reset();
    _museum_path(path, sizeof(path));

    safe_setuid_grab();
    fp = my_fopen(path, "r");
    safe_setuid_drop();

    if (!fp) return TRUE;

    while (fgets(line, sizeof(line), fp))
    {
        char *hex_buf;
        char *number_buf;
        char *art_name_buf;
        int number;
        obj_ptr obj;

        _strip_newline(line);
        if (!line[0] || line[0] == '#') continue;

        hex_buf = line;
        number_buf = strchr(hex_buf, '|');
        if (!number_buf) continue;
        *number_buf++ = '\0';

        art_name_buf = strchr(number_buf, '|');
        if (art_name_buf) *art_name_buf++ = '\0';

        number = atoi(number_buf);
        if (number <= 0) continue;

        obj = obj_alloc();
        if (hex_to_obj(hex_buf, obj))
        {
            if (obj->art_name)
                obj->art_name = (art_name_buf && art_name_buf[0]) ? quark_add(art_name_buf) : 0;

            obj->number = number;
            museum_carry(obj);
            obj_free(obj);
        }
        else
            obj_free(obj);
    }

    my_fclose(fp);
    inv_sort(_museum);
    return TRUE;
}

/************************************************************************
 * Character Sheet (py_display)
 ***********************************************************************/

void home_display(doc_ptr doc, obj_p p, int flags)
{
    inv_ptr inv = inv_filter(_home, obj_exists);
    char    name[MAX_NLEN];
    slot_t  slot;
    slot_t  max = inv_count_slots(inv, obj_exists);

    inv_sort(inv);

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(inv, slot);
        if (!obj) continue; /* bug */
        object_desc(name, obj, OD_COLOR_CODED);
        if (!obj->scratch) obj->scratch = obj_value(obj);
        doc_printf(doc, "<color:R>%6d</color> <indent><style:indent>%s</style></indent>\n", obj->scratch, name);
    }
    inv_free(inv);
}

int home_count(obj_p p)
{
    return inv_count(_home, p);
}

int museum_count(obj_p p)
{
    return inv_count(_museum, p);
}

void museum_display(doc_ptr doc, obj_p p, int flags)
{
    slot_t slot;
    slot_t max = inv_last(_museum, obj_exists);
    char   name[MAX_NLEN];

    for (slot = 1; slot <= max; slot++)
    {
        obj_ptr obj = inv_obj(_museum, slot);
        if (!obj) continue; /* bug */
        object_desc(name, obj, OD_COLOR_CODED);
        doc_printf(doc, "<indent><style:indent>%s</style></indent>\n", name);
    }

}


/************************************************************************
 * Savefiles
 ***********************************************************************/
void home_load(savefile_ptr file)
{
    inv_load(_home, file);
    inv_load(_museum, file);
}

void home_save(savefile_ptr file)
{
    inv_save(_home, file);
    inv_save(_museum, file);
}

/************************************************************************
 * User Interface
 ***********************************************************************/
struct _ui_context_s
{
    inv_ptr inv;
    slot_t  top;
    int     page_size;
    doc_ptr doc;
};
typedef struct _ui_context_s _ui_context_t, *_ui_context_ptr;

static void _display(_ui_context_ptr context);
static void _drop(_ui_context_ptr context);
static void _remove(_ui_context_ptr context);
static void _examine(_ui_context_ptr context);
static void _get(_ui_context_ptr context);
static void _ui(_ui_context_ptr context);
static void _drop_aux(obj_ptr obj, _ui_context_ptr context);



void home_ui(void)
{
    _ui_context_t context = {0};

    context.inv = _home;
    context.top = 1;

    _ui(&context);
}

void museum_ui(void)
{
    _ui_context_t context = {0};
    int success = _fetch_museum_data();
    if (!success){
        msg_format("<color:R>获取博物馆数据失败。</color>");
        return;
    }

    context.inv = _museum;
    context.top = 1;
    _ui(&context);
}


static void _ui(_ui_context_ptr context)
{
    forget_lite(); /* resizing the term would redraw the map ... sigh */
    forget_view();
    character_icky = TRUE;

    msg_line_clear();
    msg_line_init(ui_shop_msg_rect());

    Term_clear();
    context->doc = doc_alloc(MIN(80, ui_shop_rect().cx));
    for (;;)
    {
        int    max = inv_last(context->inv, obj_exists);
        rect_t r = ui_shop_rect(); /* recalculate in case resize */
        int    cmd, ct;

        context->page_size = MIN(26, r.cy - 3 - 4);
        if ((context->top - 1) % context->page_size != 0) /* resize?? */
            context->top = 1;

        _display(context);

        cmd = inkey_special(TRUE);
        msg_line_clear();
        msg_boundary(); /* turn_count is unchanging while in home/museum */
        if (cmd == ESCAPE || cmd == 'q' || cmd == 'Q') break;
        pack_lock();
        if (!shop_common_cmd_handler(cmd))
        {
            switch (cmd)
            {
            case 'g': case 'b': case 'p':  _get(context); break;
            case 'd': case 's':  _drop(context); break;
            case 'x': _examine(context); break;
            case 'r': _remove(context); break;
            case '?':
                doc_display_help("context_home.txt", inv_loc(context->inv) == INV_MUSEUM ? "博物馆" : NULL);
                Term_clear_rect(ui_shop_msg_rect());
                break;
            case SKEY_PGDOWN: case '3': case ' ':
                if (context->top + context->page_size - 1 < max)
                    context->top += context->page_size;
                break;
            case SKEY_PGUP: case '9': case '-':
                if (context->top > context->page_size)
                    context->top -= context->page_size;
                break;
            case SKEY_BOTTOM: case '1':
                 while (context->top + context->page_size - 1 < max)
                 {
                    context->top += context->page_size;
                 }
                 break;
            case SKEY_TOP: case '7':
                 context->top = 1;
                 break;
            default:
                if (cmd < 256 && isprint(cmd))
                {
                    msg_format("未识别的命令：<color:R>%c</color>。按 <color:keypress>?</color> 获取帮助。", cmd);
                }
                else if (KTRL('A') <= cmd && cmd <= KTRL('Z'))
                {
                    cmd |= 0x40;
                    msg_format("未识别的命令：<color:R>^%c</color>。按 <color:keypress>?</color> 获取帮助。", cmd);
                }
            }
            ct = inv_count_slots(context->inv, obj_exists);
            if (ct)
            {
                max = inv_last(context->inv, obj_exists);
                while (context->top > max)
                    context->top -= context->page_size;
                if (context->top < 1) context->top = 1;
            }
        }
        pack_unlock();
        notice_stuff(); /* PW_INVEN and PW_PACK ... */
        handle_stuff(); /* Plus 'C' to view character sheet */
        if ((shop_exit_hack) || (pack_overflow_count() > ((pack_is_full()) ? 0 : 1)))
        {
            if (shop_exit_hack) msg_print("你该离开了！");
            else msg_print("<color:v>你的背包满了！</color>你该离开了！");
            msg_print(NULL);
            shop_exit_hack = FALSE;
            break;
        }
    }
    character_icky = FALSE;
    energy_use = 100;
    msg_line_clear();
    msg_line_init(ui_msg_rect());

    Term_clear();
    do_cmd_redraw();

    doc_free(context->doc);
}

static void _display(_ui_context_ptr context)
{
    rect_t  r = ui_shop_rect();
    doc_ptr doc = context->doc;

    doc_clear(doc);
    doc_insert(doc, "<style:table>");
    doc_printf(doc, "%*s<color:G>%s</color>\n\n",
        (doc_width(doc) - 10)/2, "", inv_name(context->inv));

    shop_display_inv(doc, context->inv, context->top, context->page_size);
    
    {
        slot_t max = inv_last(context->inv, obj_exists);
        slot_t bottom = context->top + context->page_size - 1;

        if (context->top > 1 || bottom < max)
        {
            int page_count = (max - 1) / context->page_size + 1;
            int page_current = (context->top - 1) / context->page_size + 1;

            doc_printf(doc, "<color:B>(第 %d 页，共 %d 页)</color>\n", page_current, page_count);
        }
        else
            doc_newline(doc);
    }
    if (inv_loc(context->inv) == INV_HOME)
    {
        doc_insert(doc,
            "<color:keypress>g</color> to get an item. "
            "<color:keypress>d</color> to drop an item. ");

        doc_insert(doc,
            "<color:keypress>x</color> to begin examining items.\n"
            "<color:keypress>r</color> to begin removing (destroying) items.\n"
            "<color:keypress>Esc</color> to exit. "
            "<color:keypress>?</color> for help.");
        }
    else {
        doc_insert(doc, "按 <color:keypress>d</color> 捐赠物品。"
                        "按 <color:keypress>g</color> 取出物品。");
        doc_insert(doc,
            "<color:keypress>x</color> to begin examining items.\n"
            "<color:keypress>Esc</color> to exit. "
            "<color:keypress>?</color> for help.");
    }

    doc_insert(doc, "</style>");

    Term_clear_rect(r);
    doc_sync_term(doc,
        doc_range_top_lines(context->doc, r.cy),
        doc_pos_create(r.x, r.y));
}

static void _get_aux(obj_ptr obj)
{
    /*char name[MAX_NLEN];
    object_desc(name, obj, OD_COLOR_CODED);
    msg_format("You get %s.", name);*/
    pack_carry(obj);
}

static void _get(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        int     amt = 1;

        if (!msg_command("<color:y>Get which item <color:w>(<color:keypress>Esc</color> "
                         "to cancel)</color>?</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        if (obj->number > 1)
        {
            if (!msg_input_num("数量", &amt, 1, obj->number)) continue;
        }

        if (amt < obj->number)
        {
            obj_t copy = *obj;
            copy.number = amt;
            obj->number -= amt;
            if (obj->insured)
            {
                int vahennys = MIN(amt, (obj->insured % 100));
                copy.insured = (obj->insured / 100) * 100 + vahennys;
                obj_dec_insured(obj, vahennys);
            }
            _get_aux(&copy);
        }
        else
        {
            _get_aux(obj);
            if (!obj->number)
            {
                inv_remove(context->inv, slot);
                inv_sort(context->inv);
            }
        }
        if (inv_loc(context->inv) == INV_MUSEUM && !_save_museum_data())
            msg_print("<color:R>保存本地博物馆数据失败。</color>");
        break;
    }
}

static void _drop_aux(obj_ptr obj, _ui_context_ptr context)
{
    char name[MAX_NLEN];
    if (object_is_(obj, TV_POTION, SV_POTION_BLOOD))
    {
        msg_print("药水变酸了。");
        obj->sval = SV_POTION_SALT_WATER;
        obj->k_idx = lookup_kind(TV_POTION, SV_POTION_SALT_WATER);
        object_origins(obj, ORIGIN_BLOOD);
        obj->mitze_type = 0;
    }
    object_desc(name, obj, OD_COLOR_CODED);
    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        msg_format("你捐赠了%s。", name);

        museum_carry(obj);
        inv_sort(_museum);
        virtue_add(VIRTUE_SACRIFICE, 1); /* TODO: should depend on obj_value() */
    }
    else
    {
        msg_format("你丢下了%s。", name);
        home_carry(obj);
        inv_sort(_home);
    }
}

static void _drop(_ui_context_ptr context)
{
    obj_prompt_t prompt = {0};
    int          amt = 1;
    bool         new_id = FALSE;

    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        prompt.prompt = "捐赠哪件物品？";
        prompt.error = "你没有可捐赠的物品。";
    }
    else
    {
        prompt.prompt = "丢下哪件物品？";
        prompt.error = "你没有可丢下的物品。";
    }
    prompt.where[0] = INV_PACK;
    prompt.where[1] = INV_EQUIP;
    prompt.where[2] = INV_QUIVER;
    obj_prompt_add_special_packs(&prompt);

    obj_prompt(&prompt);
    if (!prompt.obj) return;

    if (obj_is_true_art(prompt.obj) && inv_loc(context->inv) == INV_MUSEUM)
    {
        msg_print("你不能将真正的神器捐赠给博物馆。");
        return;
    }

    if (prompt.obj->loc.where == INV_EQUIP)
    {
        if (prompt.obj->tval == TV_QUIVER && quiver_count(NULL))
        {
            msg_print("你的箭袋里还有弹药。请先取出箭袋里的所有弹药。");
            return;
        }
        if (!equip_can_takeoff(prompt.obj)) return;
    }

    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        char       name[MAX_NLEN];
        string_ptr s = string_copy_s("<color:v>Warning:</color> All donations are final! ");
        char       c;

        object_desc(name, prompt.obj, OD_COLOR_CODED);
        string_printf(s, "确定要将 %s 捐献给博物馆吗？<color:y>[y/n]</color>", name);
        c = msg_prompt(string_buffer(s), "ny", PROMPT_YES_NO);
        string_free(s);
        if (c == 'n') return;
    }
    else
        amt = prompt.obj->number;

    if (prompt.obj->number > 1)
    {
        if (!msg_input_num("数量", &amt, 1, prompt.obj->number)) return;
    }

    if (prompt.obj->loc.where == INV_EQUIP)
    {
        char name[MAX_NLEN];
        object_desc(name, prompt.obj, OD_COLOR_CODED);
        msg_format("你不再穿戴%s。", name);
        p_ptr->update |= PU_BONUS | PU_TORCH | PU_MANA;
        p_ptr->redraw |= PR_EQUIPPY;
        p_ptr->window |= PW_EQUIP;        
    }


    if (amt < prompt.obj->number)
    {
        obj_t copy = *prompt.obj;
        copy.number = amt;
        prompt.obj->number -= amt;
        if (prompt.obj->insured)
        {
            copy.insured = 0;
            if ((prompt.obj->insured % 100) > prompt.obj->number)
            {
                int vahennys = (prompt.obj->insured % 100) - prompt.obj->number;
                copy.insured = prompt.obj->insured / 100 * 100 + vahennys;
                obj_dec_insured(prompt.obj, vahennys);
            }
        }
        _drop_aux(&copy, context);
        if (new_id) autopick_alter_obj(prompt.obj, ((destroy_identify) && (obj_value(prompt.obj) < 1)));
    }
    else
        _drop_aux(prompt.obj, context);

    if (inv_loc(context->inv) == INV_MUSEUM && !_save_museum_data())
        msg_print("<color:R>保存本地博物馆数据失败。</color>");

    obj_release(prompt.obj, OBJ_RELEASE_QUIET);
}

static void _examine(_ui_context_ptr context)
{
    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;

        if (!msg_command("<color:y>调查哪件物品 <color:w>(<color:keypress>Esc</color> 结束)</color>？</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        obj_display(obj);
    }
}

static void _remove(_ui_context_ptr context)
{
    if (inv_loc(context->inv) == INV_MUSEUM)
    {
        msg_print("<color:R>你不能从博物馆中取出物品。</color>");
        return;
    }

    for (;;)
    {
        char    cmd;
        slot_t  slot;
        obj_ptr obj;
        char    name[MAX_NLEN];

        if (!msg_command("<color:y>移除哪件物品 <color:w>(<color:keypress>Esc</color> 结束)</color>？</color>", &cmd)) break;
        if (cmd < 'a' || cmd > 'z') continue;
        slot = label_slot(cmd);
        slot = slot + context->top - 1;
        obj = inv_obj(context->inv, slot);
        if (!obj) continue;

        object_desc(name, obj, OD_COLOR_CODED);
        cmd = msg_prompt(format("<color:y>Really remove %s?</color> <color:v>It will "
            "be permanently destroyed!</color> <color:y>[Y,n]</color>", name), "ny", PROMPT_YES_NO);
        if (cmd == 'n') continue;
        if (!can_player_destroy_object(obj))
        {
            object_desc(name, obj, OD_COLOR_CODED);
            msg_format("你不能破坏%s。", name);
            continue;
        }
        inv_remove(context->inv, slot);
        inv_sort(context->inv);
        _display(context);
    }
}
