#include "angband.h"
#include "equip.h"

void absconding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "潜逃");
        break;
    default:
        teleport_spell(cmd, res);
        break;
    }
}

static void _ancient_knowledge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "远古知识");
        break;
    default:
        identify_spell(cmd, res);
        break;
    }
}

static void _bind_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "束缚怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试冻结一只怪物。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stasis_monster(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void bunshin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "分身术");
        break;
    case SPELL_DESC:
        var_set_string(res, "创造你的影子分身，让你有三分之一的几率完全避开任何攻击。");
        break;
    case SPELL_CAST:
        set_multishadow(6+randint1(6), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _chain_hook_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "锁镰");
        break;
    case SPELL_DESC:
        var_set_string(res, "将一只怪物传送到你相邻的位置。");
        break;
    case SPELL_CAST:
    {
        monster_type *m_ptr;
        int m_idx;
        char m_name[80];
        int i;
        int path_n;
        u16b path_g[512];
        int ty,tx;

        var_set_bool(res, FALSE);
        if (!target_set(TARGET_KILL)) return;
        m_idx = cave[target_row][target_col].m_idx;
        if (!m_idx) return;
        if (m_idx == p_ptr->riding) return;
        if (!player_has_los_bold(target_row, target_col)) return;
        if (!projectable(py, px, target_row, target_col)) return;
        m_ptr = &m_list[m_idx];
        monster_desc(m_name, m_ptr, 0);
        msg_format("你把%s拉了过来。", m_name);

        path_n = project_path(path_g, MAX_RANGE, target_row, target_col, py, px, 0);
        ty = target_row, tx = target_col;
        for (i = 1; i < path_n; i++)
        {
            int ny = GRID_Y(path_g[i]);
            int nx = GRID_X(path_g[i]);
            cave_type *c_ptr = &cave[ny][nx];

            if (in_bounds(ny, nx) && cave_empty_bold(ny, nx) &&
                !(c_ptr->info & CAVE_OBJECT) &&
                !pattern_tile(ny, nx))
            {
                ty = ny;
                tx = nx;
            }
        }
        cave[target_row][target_col].m_idx = 0;
        cave[ty][tx].m_idx = m_idx;
        m_ptr->fy = ty;
        m_ptr->fx = tx;
        (void)set_monster_csleep(m_idx, 0);
        update_mon(m_idx, TRUE);
        lite_spot(target_row, target_col);
        lite_spot(ty, tx);

        if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
            p_ptr->update |= (PU_MON_LITE);

        if (m_ptr->ml)
        {
            if (!p_ptr->image) mon_track(m_ptr);
            health_track(m_idx);
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _detect_near_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "近距侦测");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "侦测附近的怪物、陷阱、门、楼梯和物品。映射整个楼层。");
        else if (p_ptr->lev >= 15)
            var_set_string(res, "侦测附近的怪物、陷阱、门、楼梯和物品。");
        else if (p_ptr->lev >= 5)
            var_set_string(res, "侦测附近的怪物、陷阱、门和楼梯。");
        else 
            var_set_string(res, "侦测附近的怪物。");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 45)
        {
            wiz_lite(TRUE);
        }
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        if (p_ptr->lev >= 5)
        {
            detect_traps(DETECT_RAD_DEFAULT, TRUE);
            detect_doors(DETECT_RAD_DEFAULT);
            detect_stairs(DETECT_RAD_DEFAULT);
        }
        if (p_ptr->lev >= 15)
        {
            detect_objects_normal(DETECT_RAD_DEFAULT);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void floating_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "浮空术");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时提供飘浮能力。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(20, 20));
        break;
    case SPELL_CAST:
        set_tim_levitation(randint1(20) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _glyph_of_explosion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆裂符文");
        break;
    default:
        explosive_rune_spell(cmd, res);
        break;
    }
}

void hide_in_flame_spell(int cmd, variant *res)
{
    int dam = 50 + p_ptr->lev;
    int rad = 2 + p_ptr->lev/10;
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火遁");
        break;
    case SPELL_DESC:
        var_set_string(res, "生成一个火球并同时进行传送。暂时提供火抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam/2));
        break;
    case SPELL_CAST:
        fire_ball(GF_FIRE, 0, dam, rad);
        teleport_player(30, 0);
        set_oppose_fire(p_ptr->lev, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hide_in_leaves_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "木遁");
        break;
    default:
        phase_door_spell(cmd, res);
        break;
    }
}

void hide_in_mist_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "雾遁");
        break;
    case SPELL_DESC:
        var_set_string(res, "生成巨大的毒素、生命吸取和混乱球，然后传送 30 格。");
        break;
    case SPELL_CAST:
        fire_ball(GF_POIS, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        fire_ball(GF_OLD_DRAIN, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        fire_ball(GF_CONFUSION, 0, 75+p_ptr->lev*2/3, p_ptr->lev/5+2);
        teleport_player(30, 0L);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hit_and_away_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "一击脱离");
        break;
    default:
        panic_hit_spell(cmd, res);
        break;
    }
}

void kawarimi_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "替身术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在受到攻击时传送。在更高等级时，可能可以在受到伤害之前传送。");
        break;
    case SPELL_CAST:
        if (!(p_ptr->special_defense & NINJA_KAWARIMI))
        {
            msg_print("你现在已经准备好避开任何攻击了。");
            p_ptr->special_defense |= NINJA_KAWARIMI;
            p_ptr->redraw |= PR_STATUS;
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nyusin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "瞬身术");
        break;
    case SPELL_DESC:
        var_set_string(res, "瞬间靠近怪物并同时发动攻击。");
        break;
    case SPELL_CAST:
        var_set_bool(res, rush_attack(5, NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void quick_walk_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "缩地");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        if (p_ptr->action == ACTION_QUICK_WALK) set_action(ACTION_NONE);
        else set_action(ACTION_QUICK_WALK);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _rengoku_kaen_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "炼狱火炎");
        break;
    case SPELL_DESC:
        var_set_string(res, "向随机方向发射若干道火、地狱或等离子射线。");
        break;
    case SPELL_CAST:
    {
        int k, x = 0, y = 0;
        int num = damroll(3, 9);

        for (k = 0; k < num; k++)
        {
            int typ = one_in_(2) ? GF_FIRE : one_in_(3) ? GF_NETHER : GF_PLASMA;
            int attempts = 1000;

            while (attempts--)
            {
                scatter(&y, &x, py, px, 4, 0);

                if (!player_bold(y, x)) break;
            }
            project(0, 0, y, x, damroll(6 + p_ptr->lev / 8, 10), typ,
                (PROJECT_BEAM | PROJECT_THRU | PROJECT_GRID | PROJECT_KILL));
        }

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static void _smoke_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "烟雾弹");
        break;
    case SPELL_DESC:
        var_set_string(res, "释放一个不造成任何伤害的混乱球。");
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_OLD_CONF, dir, p_ptr->lev*3, 3);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static bool _obj_is_shuriken(obj_ptr obj) { return obj->tval == TV_SPIKE; }

void syuriken_spreading_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "手里剑散射");
        break;
    case SPELL_DESC:
        var_set_string(res, "向 8 个随机方向发射 8 枚铁蒺藜。");
        break;
    case SPELL_CAST:
    {
        int i;
        for (i = 0; i < 8; i++)
        {
            int        slot = pack_find_first(_obj_is_shuriken);
            py_throw_t context = {0}; /* better reset for each shot! */
            if (!slot)
            {
                if (!i) msg_print("你没有铁蒺藜。");
                else msg_print("你没有多余的铁蒺藜了。");
                break;
            }
            context.dir = DIR_RANDOM;
            context.obj = pack_obj(slot);
            py_throw(&context);
        }
        var_set_bool(res, TRUE);
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
static spell_info _get_spells[] =
{
    /*lvl cst fail spell */
    { 1,   1,  20, create_darkness_spell},
    { 2,   2,  25, _detect_near_spell},
    { 3,   3,  25, hide_in_leaves_spell},
    { 5,   3,  30, kawarimi_spell},
    { 7,   8,  35, absconding_spell},
    { 8,  10,  35, hit_and_away_spell},
    {10,  10,  40, _bind_monster_spell},
    {12,  12,  70, _ancient_knowledge_spell},
    {15,  10,  50, floating_spell},
    {17,  12,  45, hide_in_flame_spell},
    {18,  20,  40, nyusin_spell},
    {20,   5,  50, syuriken_spreading_spell},
    {22,   5,  55, _chain_hook_spell},
    {25,  32,  60, _smoke_ball_spell},
    {28,  32,  60, swap_pos_spell},
    {30,  30,  70, _glyph_of_explosion_spell},
    {32,  40,  40, hide_in_mud_spell},
    {34,  35,  50, hide_in_mist_spell},
    {38,  40,  60, _rengoku_kaen_spell},
    {41,  50,  55, bunshin_spell},
    { -1, -1,  -1, NULL}
};

static power_info _get_powers[] =
{
    { A_NONE, { 20, 0,  0, quick_walk_spell}}, 
    { -1, {-1, -1, -1, NULL}}
};

static void _calc_bonuses(void)
{
    if (heavy_armor())
    {
        p_ptr->pspeed -= p_ptr->lev/10;
        p_ptr->skills.stl -= p_ptr->lev/10;
    }
    else if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->pspeed += 3;
        p_ptr->pspeed += p_ptr->lev/10;
        p_ptr->skills.stl += p_ptr->lev/10;
        if (p_ptr->lev >= 25)
            p_ptr->free_act++;
        /* Ninjas are not archers, and have relatively poor thb skills.
         * However, they excel at throwing (tht)! */
        p_ptr->skill_tht += 30 + p_ptr->lev;
    }
    if (!equip_find_obj(TV_SHIELD, SV_ANY))
    {
        p_ptr->to_a += p_ptr->lev/2 + 5;
        p_ptr->dis_to_a += p_ptr->lev/2 + 5;
    }
    p_ptr->slow_digest = TRUE;
    res_add(RES_FEAR);
    if (p_ptr->lev >= 20) res_add(RES_POIS);
    if (p_ptr->lev >= 25) p_ptr->sustain_dex = TRUE;
    if (p_ptr->lev >= 30) p_ptr->see_inv++;
    if (p_ptr->lev >= 45) res_add(RES_POIS);
    p_ptr->see_nocto = TRUE;
}

static void _get_flags(u32b flgs[OF_ARRAY_SIZE])
{
    if (heavy_armor())
        add_flag(flgs, OF_SPEED);
    else
    {
        if (!equip_find_obj(TV_SHIELD, SV_ANY))
        {
            add_flag(flgs, OF_SPEED);
        }
        if (p_ptr->lev >= 25)
            add_flag(flgs, OF_FREE_ACT);
    }
    add_flag(flgs, OF_SLOW_DIGEST);
    add_flag(flgs, OF_RES_FEAR);
    add_flag(flgs, OF_NIGHT_VISION);
    if (p_ptr->lev >= 20) add_flag(flgs, OF_RES_POIS);
    if (p_ptr->lev >= 25) add_flag(flgs, OF_SUST_DEX);
    if (p_ptr->lev >= 30) add_flag(flgs, OF_SEE_INVIS);
}

static void _calc_weapon_bonuses(object_type *o_ptr, weapon_info_t *info_ptr)
{
    if ( skills_weapon_is_icky(o_ptr->tval, o_ptr->sval) 
      || equip_find_obj(TV_SHIELD, SV_ANY) )
    {
        info_ptr->to_h -= 40;
        info_ptr->dis_to_h -= 40;
        info_ptr->icky_wield = TRUE;
        info_ptr->base_blow /= 2;
        info_ptr->xtra_blow /= 2;
        if (info_ptr->base_blow < 100) info_ptr->base_blow = 100;
    }
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "ninjutsu";
        me.options = CASTER_USE_HP;
        me.which_stat = A_DEX;
        init = TRUE;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_POTION, SV_POTION_SPEED, 1);
    py_birth_obj_aux(TV_SPIKE, 0, rand_range(15, 20));
}

class_t *ninja_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    /* static info never changes */
    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  24,  36,   8,  48,  32,  70,  35 };
    skills_t xs = { 15,  10,  10,   0,   0,   0,  25,  11 };

        me.name = "忍者";
        me.desc = "忍者是潜伏在黑暗中可怕的刺客。他们能在没有光源的情况下有效导航，对敌人进行出其不意的袭击，并能一击必杀。忍者可以使用忍术，擅长寻找隐藏的陷阱和门、解除陷阱以及撬锁。由于重甲、重型武器或盾牌会极大地限制他们的行动，他们更喜欢轻便的衣服，并在升级时变得更快、更隐蔽。忍者无所畏惧，在高级别时几乎对毒素免疫，并能看到隐形的事物。敏捷决定了忍者使用忍术的能力。\n\n忍者可以使用忍术进行潜伏和奇袭。随着等级的提升，他们会获得更多的忍术技巧。他们拥有一项职业能力——“缩地(Quick Walk)”——可以让他们行走的速度极快。";

        me.stats[A_STR] =  0;
        me.stats[A_INT] = -1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  2;
        me.stats[A_CHR] =  2;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 4;
        me.exp = 120;
        me.pets = 40;
        me.flags = CLASS_SENSE1_MED | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_STRONG;

        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.get_flags = _get_flags;
        me.calc_weapon_bonuses = _calc_weapon_bonuses;
        me.caster_info = _caster_info;
        me.get_spells = _get_spells;
        me.get_powers = _get_powers;
        me.character_dump = py_dump_spells;
        me.known_icky_object = skills_obj_is_icky_weapon;
        init = TRUE;
    }

    return &me;
}
