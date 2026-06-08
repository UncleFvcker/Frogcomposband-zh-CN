#include "angband.h"

static bool _on_mirror = FALSE;

typedef void (*pos_fn)(int y, int x);
static void _for_each_mirror(pos_fn f)
{
    int x, y;
    for (x = 0; x < cur_wid; x++)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            if (is_mirror_grid(&cave[y][x]))
                f(y, x);
        }
    }
}

static int _mirrors_ct(void)
{
    int ct = 0;
    int x, y;
    for (x = 0; x < cur_wid; x++)
    {
        for (y = 0; y < cur_hgt; y++)
        {
            if (is_mirror_grid(&cave[y][x]))
                ct++;
        }
    }
    return ct;
}

static int _mirrors_max(void)
{
    return 4 + p_ptr->lev/10;
}

static bool _mirror_place(void)
{
    if (!cave_clean_bold(py, px))
    {
        msg_print("该物品抵抗了法术。");
        return FALSE;
    }

    cave[py][px].info |= CAVE_OBJECT;
    cave[py][px].mimic = feat_mirror;
    cave[py][px].info |= CAVE_GLOW;
    note_spot(py, px);
    lite_spot(py, px);
    update_local_illumination(py, px);

    return TRUE;
}

static void _banishing_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "放逐之镜");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "快速将附近的怪物传送走。");
        else
            var_set_string(res, "将附近的怪物传送走。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_AWAY_ALL, dir, spell_power(p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_ENERGY:
        if (_on_mirror)
        {
            var_set_int(res, 50);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _binding_field_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev*11 + 5 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "束缚力场");
        break;
    case SPELL_DESC:
        var_set_string(res, "产生一个魔法三角形，对该区域内的所有怪物造成伤害。三角形的顶点是你以及视线内的两面镜子。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
        if (!binding_field(dam))
            msg_print("你无法选择合适的镜子！");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _break_mirrors_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "打碎镜子");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁当前楼层所有的镜子。靠近镜子的怪物会受到伤害。");
        break;
    case SPELL_CAST:
        remove_all_mirrors(TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _drip_of_light_spell(int cmd, variant *res)
{
    int  dd = 3 + (p_ptr->lev-1)/5;
    int  ds = 4;
    bool beam = (p_ptr->lev >= 10 && _on_mirror) ? TRUE : FALSE;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光之滴");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 10)
            var_set_string(res, "发射一束光线或一支光箭，取决于你是否站在镜子上。");
        else
            var_set_string(res, "发射一支光箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        if (beam)
            fire_beam(GF_LITE, dir,spell_power(damroll(dd, ds) + p_ptr->to_d_spell));
        else
            fire_bolt(GF_LITE, dir,spell_power(damroll(dd, ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _illusion_light_spell(int cmd, variant *res)
{
    int mult = _on_mirror ? 4 : 3;
    int power = spell_power(p_ptr->lev * mult);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幻觉之光");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试减速、震慑、混乱、恐吓或冻结视线内的所有怪物。如果你站在镜子上，此效果会更强大。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(power));
        break;
    case SPELL_CAST:
        slow_monsters(power);
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(power);
        turn_monsters(power);
        stasis_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _make_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造镜子");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你脚下制造一面镜子。");
        break;
    case SPELL_CAST:
        if (_mirrors_ct() < _mirrors_max())
            _mirror_place();
        else
            msg_print("镜子太多了，无法控制！");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_clashing_spell(int cmd, variant *res)
{
    int dd = 8 + (p_ptr->lev - 5)/4;
    int ds = 8;
    int rad = p_ptr->lev > 20 ? spell_power((p_ptr->lev - 20)/8 + 1) : 0;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镜面碎击");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个由碎片组成的球体。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_SHARDS, dir, spell_power(damroll(dd, ds) + p_ptr->to_d_spell), rad);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_concentration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镜之专注");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (total_friends)
        {
            msg_print("你需要专注于你的宠物。");
            return;
        }
        if (_on_mirror)
        {
            msg_print("你觉得头脑清醒了一点。");

            p_ptr->csp += (5 + p_ptr->lev * p_ptr->lev / 100);
            if (p_ptr->csp >= p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }

            p_ptr->redraw |= (PR_MANA);
            var_set_bool(res, TRUE);
        }
        else
        {
            msg_print("你需要站在镜子上才能使用这个法术！");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "光之镜");
        break;
    case SPELL_DESC:
        var_set_string(res, "永久照亮附近区域和房间内部。");
        break;
    case SPELL_CAST:
        lite_area(damroll(2, p_ptr->lev/2), p_ptr->lev/10 + 1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_recall_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召回之镜");
        break;
    case SPELL_DESC:
        var_set_string(res, "将玩家从地下城召回城镇，或从城镇召回地下城最深的一层。");
        break;
    case SPELL_CAST:
        var_set_bool(res, word_of_recall(TRUE));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_ruffnor_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "拉夫诺之镜");
        break;
    case SPELL_DESC:
        var_set_string(res, "产生一个屏障，几乎完全保护你免受所有伤害。当屏障破裂或持续时间超过时，会消耗你几个回合的时间。");
        break;
    case SPELL_CAST:
        set_invuln(spell_power(randint1(4) + 4), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


static void _mirror_of_seeing_spell(int cmd, variant *res)
{
    int lvl = p_ptr->lev;

    if (_on_mirror)
        lvl += 4;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "洞察之镜");
        break;
    case SPELL_DESC:
        if (lvl >= 39)
            var_set_string(res, "探测你附近的怪物。提供临时心灵感应(ESP)。绘制附近区域的地图。");
        else if (lvl >= 29)
            var_set_string(res, "探测你附近的怪物。提供临时心灵感应(ESP)");
        else if (lvl >= 19)
            var_set_string(res, "探测你附近的怪物。");
        else if (lvl >= 5)
            var_set_string(res, "探测你附近可见的怪物。");
        else
            var_set_string(res, "因为你太弱了而毫无效果。试着站在一面镜子上。");
        break;
    case SPELL_CAST:
    {
        if (lvl < 5)
            msg_print("你需要一面镜子来集中精神！");

        if (lvl >= 5)
            detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (lvl >= 19)
            detect_monsters_invis(DETECT_RAD_DEFAULT);
        if (lvl >= 29)
            set_tim_esp(lvl + randint1(lvl),FALSE);
        if (lvl >= 39)
            map_area(DETECT_RAD_MAP);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_of_wandering_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "漂泊之镜");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "快速传送很长一段距离。");
        else
            var_set_string(res, "传送很长一段距离。");
        break;
    case SPELL_CAST:
        teleport_player(p_ptr->lev*5, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
            break;
        } 
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_shifting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镜面穿梭");
        break;
    case SPELL_DESC:
        var_set_string(res, "重新生成当前地下城楼层。只能在镜子上使用。");
        break;
    case SPELL_CAST:
        if (!_on_mirror)
            msg_print("你无法找到镜之世界！");
        else
            alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_sleeping_fn(int y, int x)
{
    project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev, GF_OLD_SLEEP, 
        PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
}
static void _mirror_sleeping_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "沉睡之镜");
        break;
    case SPELL_DESC:
        var_set_string(res, "在整个楼层的所有镜子上产生让怪物沉睡的球体。");
        break;
    case SPELL_CAST:
        _for_each_mirror(_mirror_sleeping_fn);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _mirror_tunnel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镜之隧道");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "快速传送到给定位置。");
        else
            var_set_string(res, "传送到给定位置。");
        break;
    case SPELL_CAST:
    {
        int x = 0, y = 0;
        var_set_bool(res, FALSE);

        msg_print("你穿过了镜之世界……");
        if (!tgt_pt(&x, &y, p_ptr->lev / 2 + 10)) return;
        if (!dimension_door_aux(x, y, p_ptr->lev / 2 + 10))
            msg_print("你未能正确进入镜像位面！");

        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
            break;
        } 
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _multi_shadow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "多重影身");
        break;
    case SPELL_DESC:
        var_set_string(res, "给予你 1/3 的几率完全闪避一次攻击。");
        break;
    case SPELL_CAST:
        set_multishadow(6 + randint1(6), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _robe_of_dust_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "尘埃之袍");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内提供镜之碎片光环，伤害任何在近战中攻击你的怪物。");
        break;
    case SPELL_CAST:
        set_dustrobe(20+randint1(20),FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _seal_of_mirror_fn(int y, int x)
{
    int dam = spell_power(p_ptr->lev*4 + 100);
    if (project_m(0, 0, y, x, dam, GF_GENOCIDE, PROJECT_GRID|PROJECT_ITEM|PROJECT_KILL|PROJECT_JUMP,TRUE))
    {
        if(!cave[y][x].m_idx)
            remove_mirror(y,x);
    }
}
static void _seal_of_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "镜之封印");
        break;
    case SPELL_DESC:
        var_set_string(res, "将一只位于镜子上的怪物从当前地下城楼层抹除。");
        break;
    case SPELL_CAST:
        _for_each_mirror(_seal_of_mirror_fn);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _seeker_ray_spell(int cmd, variant *res)
{
    int dd = 11 + (p_ptr->lev - 5)/4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "追踪射线");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一束法力射线。如果射线击中镜子，它会打破那面镜子，并向另一面镜子反射。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(dd), ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_SEEKER, dir, spell_power(damroll(dd,ds) + p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _shield_of_water_spell(int cmd, variant *res)
{
    int lvl = p_ptr->lev; /* Boost if _on_mirror? */

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水之盾");
        break;
    case SPELL_DESC:
        if (lvl >= 40)
            var_set_string(res, "提供护甲等级(AC)、反射和魔法抗性的加成。");
        else if (lvl >= 32)
            var_set_string(res, "提供护甲等级(AC)和反射的加成。");
        else
            var_set_string(res, "提供护甲等级(AC)加成。");
        break;
    case SPELL_CAST:
        set_shield(20 + randint1(20), FALSE);
        if (lvl >= 32) 
            set_tim_reflect(20 + randint1(20), FALSE);
        if (lvl >= 40) 
            set_resist_magic(20 + randint1(20),FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _super_ray_spell(int cmd, variant *res)
{
    int dd = 1;
    int ds = p_ptr->lev * 2;
    int b = 150 + p_ptr->to_d_spell;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "超级射线");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道强大的法力射线。如果射线击中镜子，它会打破那面镜子，并从该点向8个不同方向发射8道法力射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), b));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_SUPER_RAY, dir, spell_power(damroll(dd,ds) + b));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _warped_mirror_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "扭曲之镜");
        break;
    case SPELL_DESC:
        if (_on_mirror)
            var_set_string(res, "快速传送一小段距离。");
        else
            var_set_string(res, "传送一小段距离。");
        break;
    case SPELL_CAST:
        teleport_player(10, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, ((_on_mirror) ? 15 : 33));
            break;
        }
        else
        {
            var_set_int(res, ((_on_mirror) ? 50 : 100));
            break;
        } 
    default:
        default_spell(cmd, res);
        break;
    }
}

/****************************************************************
 * Spell Table and Exports
 ****************************************************************/
static spell_info _spells[] = 
{
    /*lvl cst fail spell */
    { 1,   1,  15, _mirror_of_seeing_spell},
    { 1,   2,  40, _make_mirror_spell},
    { 2,   2,  20, _drip_of_light_spell},
    { 3,   2,  20, _warped_mirror_spell},
    { 5,   3,  35, _mirror_of_light_spell},
    { 6,   5,  35, _mirror_of_wandering_spell},
    {10,   5,  30, _robe_of_dust_spell},
    {12,  12,  30, _banishing_mirror_spell},
    {15,  15,  30, _mirror_clashing_spell},
    {19,  13,  30, _mirror_sleeping_spell},
    {23,  18,  50, _seeker_ray_spell},
    {25,  20,  40, _seal_of_mirror_spell},
    {27,  30,  60, _shield_of_water_spell},
    {29,  30,  60, _super_ray_spell},
    {31,  35,  60, _illusion_light_spell},
    {33,  50,  80, _mirror_shifting_spell},
    {36,  30,  80, _mirror_tunnel_spell},
    {38,  40,  70, _mirror_of_recall_spell},
    {40,  50,  55, _multi_shadow_spell},
    {43,  55,  70, _binding_field_spell},
    {46,  70,  75, _mirror_of_ruffnor_spell},
    { -1, -1,  -1, NULL}
};

static power_info _powers[] =
{
    { A_NONE, { 1, 0,  0, _break_mirrors_spell}}, 
    { A_INT,  {30, 0, 50, _mirror_concentration_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};
static spell_info *_get_spells(void)
{
    _on_mirror = is_mirror_grid(&cave[py][px]);
    return _spells;
}

static power_info *_get_powers(void)
{    
    _on_mirror = is_mirror_grid(&cave[py][px]);
    return _powers;
}

static void _character_dump(doc_ptr doc)
{
    spellbook_character_dump(doc);
    doc_insert(doc, "<color:r>领域:</color> <color:B>镜之魔法</color>\n");
    py_dump_spells_aux(doc);
}

static void _calc_bonuses(void)
{
    if (p_ptr->lev >= 40) 
        p_ptr->reflect = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if(p_ptr->lev >= 40)
        add_flag(flgs, OF_REFLECT);
}

static void _on_fail(const spell_info *spell)
{
    if (randint1(100) < (spell->fail / 2))
    {
        int b = randint1(100);

        if (b <= 50)
        {
        }
        else if (b <= 80)
        {
            msg_print("奇怪的幻象似乎在你眼前舞动……");
            teleport_player(10, TELEPORT_PASSIVE);
        }
        else if (b <= 95)
        {
            msg_print("你的大脑混乱了！");
            set_image(p_ptr->image + 5 + randint1(10), FALSE);
        }
        else
        {
            msg_print("你的心智释放出不可控制的风暴力量！");

            project(PROJECT_WHO_UNCTRL_POWER, 2 + p_ptr->lev / 10, py, px, p_ptr->lev * 2,
                GF_MANA, PROJECT_JUMP | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM);
            p_ptr->csp = MAX(0, p_ptr->csp - p_ptr->lev * MAX(1, p_ptr->lev / 10));
        }
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "镜之魔法";
        me.which_stat = A_INT;
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 50;
        me.encumbrance.enc_wgt = 800;
        me.on_fail = _on_fail;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_ROBE, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, rand_range(2, 5));
}

class_t *mirror_master_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 30,  33,  40,   3,  14,  16,  34,  30 };
    skills_t xs = { 10,  11,  12,   0,   0,   0,   6,  10 };

        me.name = "镜术士";
        me.desc = "镜术士是施法者；像法师一样，他们必须依靠智慧生存。他们能创造魔法镜子，以此来辅助他们独特的镜之魔法(Mirror-Magic)法术。智力决定了镜术士的施法能力。\n \n镜术士随着经验的积累能获得更多的法术，他们的法术也会随着时间变得更加强大。他们的大部分魔法都依赖于镜子的精心摆放，这些镜子既可用于进攻也可用于防御。站在镜子上也会增强镜术士的能力；例如，他们可以从那里进行快速传送或快速恢复法力。然而，并非所有的镜之魔法都需要实体的镜子；许多效果仅仅依赖于光线、镜子碎片，或是依赖于不确定性、反射和幻觉。同时可控制的镜子最大数量取决于玩家的等级；有时，可能需要打碎不必要的镜子。";

        me.stats[A_STR] = -2;
        me.stats[A_INT] =  3;
        me.stats[A_WIS] =  1;
        me.stats[A_DEX] = -1;
        me.stats[A_CON] = -2;
        me.stats[A_CHR] = -2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 130;
        me.pets = 30;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_WEAK |
                   CLASS_SENSE2_FAST | CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.caster_info = _caster_info;
        me.get_spells_fn = _get_spells;
        me.get_powers_fn = _get_powers;
        me.character_dump = _character_dump;
        init = TRUE;
    }

    return &me;
}

bool is_mirror_grid(cave_type *c_ptr)
{
    if ((c_ptr->info & CAVE_OBJECT) && have_flag(f_info[c_ptr->mimic].flags, FF_MIRROR))
        return TRUE;
    else
        return FALSE;
}

void remove_mirror(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    c_ptr->info &= ~(CAVE_OBJECT);
    c_ptr->mimic = 0;

    if (d_info[dungeon_type].flags1 & DF1_DARKNESS)
    {
        c_ptr->info &= ~(CAVE_GLOW);
        if (!view_torch_grids) c_ptr->info &= ~(CAVE_MARK);
        if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);
        update_local_illumination(y, x);
    }

    note_spot(y, x);
    lite_spot(y, x);
}

static void _explode_fn(int y, int x)
{
    remove_mirror(y, x);
    project(PROJECT_WHO_MIRROR, 2, y, x, p_ptr->lev / 2 + 5, GF_SHARDS,
            PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP);
}
void remove_all_mirrors(bool explode)
{
    if (explode)
        _for_each_mirror(_explode_fn);
    else
        _for_each_mirror(remove_mirror);
}
