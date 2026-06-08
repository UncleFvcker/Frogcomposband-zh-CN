#include "angband.h"

/****************************************************************************
 * Burglary, the preferred realm for the Rogue
 ****************************************************************************/
static cptr _rogue_pick_pocket(int power)
{
    int           y, x, m_idx, dir;
    monster_type *m_ptr;
    monster_race *r_ptr;
    char          m_name[MAX_NLEN];
    char          o_name[MAX_NLEN];

    power += p_ptr->lev;
    power += adj_stat_save[p_ptr->stat_ind[A_DEX]];

    if (!get_rep_dir2(&dir)) return NULL;
    if (dir == 5) return NULL;

    y = py + ddy[dir];
    x = px + ddx[dir];

    if (!cave[y][x].m_idx)
    {
        msg_print("这里没有怪物。");
        return NULL;
    }

    m_idx = cave[y][x].m_idx;
    m_ptr = &m_list[m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    if (!m_ptr->ml || p_ptr->image) /* Can't see it, so can't steal! */
    {
        msg_print("这里没有怪物。");
        return NULL;
    }

    monster_desc(m_name, m_ptr, 0);

    if ( !mon_save_aux(m_ptr->r_idx, power)
      || (MON_CSLEEP(m_ptr) && !mon_save_aux(m_ptr->r_idx, power)))
    {
        object_type loot = {0};

        if (m_ptr->hold_o_idx && one_in_(2))
        {
            object_copy(&loot, &o_list[m_ptr->hold_o_idx]);
            delete_object_idx(m_ptr->hold_o_idx);
            loot.held_m_idx = 0;
            object_origins(&loot, ORIGIN_STOLEN);
            loot.origin_xtra = m_ptr->r_idx;
        }
        else if (m_ptr->drop_ct > m_ptr->stolen_ct)
        {
            if (get_monster_drop(m_idx, &loot))
            {
                m_ptr->stolen_ct++;
                if (r_ptr->flags1 & RF1_UNIQUE)
                    r_ptr->stolen_ct++;
                object_origins(&loot, ORIGIN_STOLEN);
                loot.origin_xtra = m_ptr->r_idx;
            }
        }

        if (!loot.k_idx)
        {
            msg_print("没有什么可以偷的！");
        }
        else
        {
            object_desc(o_name, &loot, 0);
            if (mon_save_aux(m_ptr->r_idx, power))
            {
                msg_format("哎呀！你掉了 %s。", o_name);
                drop_near(&loot, -1, y, x);
            }
            else if (loot.tval == TV_GOLD)
            {
                if ((p_ptr->prace == RACE_WEREWOLF) && (strpos("银", o_name)))
                {
                    msg_print("你偷了一些银币，但立刻又把它们掉了。");
                    take_hit(DAMAGE_NOESCAPE, randint1(10), "接触了银币");
                }
                else {
                    msg_format("你偷了价值 %d 金币的 %s。", (int)loot.pval, o_name);
                    sound(SOUND_SELL);
                    p_ptr->au += loot.pval;
                    stats_on_gold_find(loot.pval);
                    p_ptr->redraw |= (PR_GOLD);
                }
            }
            else
            {
                pack_carry(&loot);
                msg_format("你偷到了 %s。", o_name);
            }
        }

        if ((MON_CSLEEP(m_ptr)) && ((r_ptr->flags1 & RF1_UNIQUE) ||
             mon_save_aux(m_ptr->r_idx, power)))
        {
            set_monster_csleep(m_idx, 0);
            if ( allow_ticked_off(r_ptr)
              && ((r_ptr->flags1 & RF1_UNIQUE) || mon_save_aux(m_ptr->r_idx, power)) )
            {
                msg_format("%^s 醒了过来，看起来非常生气！", m_name);
                mon_anger(m_ptr);
            }
            else
                msg_format("%^s 醒了过来。", m_name);
        }

        if (loot.k_idx)
        {
            if (mon_save_aux(m_ptr->r_idx, power))
                msg_print("你没能逃脱！");
            else
            {
                if (p_ptr->lev < 35 || get_check("逃跑吗？"))
                    teleport_player(25 + p_ptr->lev/2, 0L);
            }
        }
    }
    else if (MON_CSLEEP(m_ptr))
    {
        set_monster_csleep(m_idx, 0);
        if (allow_ticked_off(r_ptr))
        {
            msg_format("失败了！%^s 醒了过来，看起来非常生气！", m_name);
            mon_anger(m_ptr);
        }
        else
            msg_format("失败了！%^s 醒了过来。", m_name);
    }
    else if (allow_ticked_off(r_ptr))
    {
        msg_format("失败了！%^s 看起来非常生气！", m_name);
        mon_anger(m_ptr);
    }
    else
    {
        msg_print("失败了！");
    }

    if (is_friendly(m_ptr) || is_pet(m_ptr))
    {
        msg_format("%^s 突然变得充满敌意！", m_name);
        set_hostile(m_ptr);
    }
    return "";
}

static cptr _rogue_negotiate(void)
{
    int           m_idx = 0;
    monster_type *m_ptr;
    monster_race *r_ptr;
    char          m_name[MAX_NLEN];

    if (target_set(TARGET_MARK))
    {
        if (target_who > 0)
            m_idx = target_who;
        else
            m_idx = cave[target_row][target_col].m_idx;
    }

    if (!m_idx)
    {
        msg_print("这里没有怪物。");
        return NULL;
    }

    m_ptr = &m_list[m_idx];
    r_ptr = &r_info[m_ptr->r_idx];

    if (!m_ptr->ml || p_ptr->image)
    {
        msg_print("这里没有怪物。");
        return NULL;
    }

    monster_desc(m_name, m_ptr, 0);

    if (is_pet(m_ptr) || is_friendly(m_ptr))
    {
        msg_format("%^s 已经在为你服务了。", m_name);
        return NULL;
    }

    set_monster_csleep(m_idx, 0);

    if (r_ptr->flags2 & RF2_THIEF)
        mon_lore_2(m_ptr, RF2_THIEF);

    if (!(r_ptr->flags2 & RF2_THIEF))
    {
        msg_format("%^s 不接受任何形式的交易！", m_name);
    }
    else if (!mon_save_p(m_ptr->r_idx, A_CHR))
    {
        int cost = 10 + r_ptr->level * 100;

        if (r_ptr->flags1 & RF1_UNIQUE)
            cost *= 10;

        if (p_ptr->au >= cost)
        {
            msg_format("%^s 说：“我的服务需要你支付 %d 枚金币。”", m_name, cost);

            if (get_check("你付款吗？"))
            {
                sound(SOUND_SELL);
                p_ptr->au -= cost;
                stats_on_gold_services(cost);
                p_ptr->redraw |= PR_GOLD;

                if (mon_save_p(m_ptr->r_idx, A_CHR))
                {
                    msg_format("%^s 说：“蠢货！永远别相信一个盗贼！”", m_name);
                    mon_anger(m_ptr);
                }
                else
                {
                    msg_format("%^s 说：“成交！”", m_name);
                    if (!(r_ptr->flags1 & RF1_UNIQUE) && !mon_save_p(m_ptr->r_idx, A_CHR))
                        set_pet(m_ptr);
                    else
                        set_friendly_ingame(m_ptr);
                }
            }
            else
            {
                msg_format("%^s 说：“无赖！”", m_name);
                mon_anger(m_ptr);
            }
        }
        else
        {
            msg_format("%^s 说：“哈！你可雇不起我帮忙！”", m_name);
        }
    }
    else
    {
        msg_format("你居然问出这种问题，%^s 觉得受到了侮辱！", m_name);
        mon_anger(m_ptr);
    }
    return "";
}


cptr do_burglary_spell(int spell, int mode)
{
    bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
    bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
    bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
    bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
    bool fail = (mode == SPELL_FAIL) ? TRUE : FALSE;

    int plev = p_ptr->lev;
    int rad = DETECT_RAD_DEFAULT;
    int dir;

    if (plev >= 45)
        rad = DETECT_RAD_ALL;
    else
        rad += plev;

    switch (spell)
    {
    /* Burglar's Handbook */
    case 0:
        if (name) return "探测陷阱";
        if (desc) return "探测附近的陷阱。";
        if (info) return info_radius(rad);
        if (cast)
            detect_traps(rad, TRUE);
        break;

    case 1:
        if (name) return "解除陷阱";
        if (desc) return "发射一道解除陷阱的射线。";

        if (cast)
        {
            if (!get_aim_dir(&dir)) return NULL;
            disarm_trap(dir);
        }
        break;

    case 2:
        if (name) return "探测财宝";
        if (desc) return "探测你附近的所有财宝。";
        if (info) return info_radius(rad);

        if (cast)
        {
            detect_treasure(rad);
            detect_objects_gold(rad);
        }
        break;

    case 3:
        if (name) return "探测物品";
        if (desc) return "探测你附近的所有物品。";
        if (info) return info_radius(rad);

        if (cast)
            detect_objects_normal(rad);
        break;

    case 4:
        if (name) return "黑暗视觉";
        if (desc) return "暂时提供红外视觉。";
        {
            int base = spell_power(100);

            if (info) return info_duration(base, base);

            if (cast)
                set_tim_infra(base + randint1(base), FALSE);
        }
        break;

    case 5:
        if (name) return "轻声行走";
        if (desc) return "短时间内大幅提升潜行能力。";
        {
            int base = spell_power(50);

            if (info) return info_duration(base, base);
            if (cast)
                set_tim_dark_stalker(base + randint1(base), FALSE);
        }
        break;

    case 6:
        if (name) return "小型逃遁";
        if (desc) return "进行中等距离的传送。";

        {
            int range = 30;

            if (info) return info_range(range);

            if (cast)
            {
                if (mut_present(MUT_ASTRAL_GUIDE))
                    energy_use = 30;
                teleport_player(range, 0L);
            }
        }
        break;

    case 7:
        if (name) return "设置小型陷阱";
        if (desc) return "在脚下设置一个微弱的陷阱。经过该陷阱的怪物会受到各种较弱的效果影响。";

        if (cast)
            set_trap(py, px, feat_rogue_trap1);
        break;

    /* Thieving Ways */
    case 8:
        if (name) return "标记逃跑路线";
        if (desc) return "映射附近区域。";
        if (info) return info_radius(rad);

        if (cast)
            map_area(rad);
        break;

    case 9:
        if (name) return "妙手空空";
        if (desc) return "尝试从相邻的怪物身上偷取物品或财宝。";

        if (cast)
            return _rogue_pick_pocket(0);
        break;

    case 10:
        if (name) return "谈判";
        if (desc) return "尝试花钱雇佣附近的窃贼为你服务。";

        if (cast)
            return _rogue_negotiate();
        break;

    case 11:
        if (name) return "取回物品";
        if (desc) return "将远处的物品拉到你身边。";

        {
            int weight = spell_power(plev * 15);
            if (info) return info_weight(weight);
            if (cast)
            {
                if (!get_aim_dir(&dir)) return NULL;
                fetch(dir, weight, FALSE);
            }
        }
        break;

    case 12:
        if (name) return "危险感知";
        if (desc) return "暂时提供心灵感应能力。";
        {
            int base = 25;
            int sides = 30;

            if (info) return info_duration(base, sides);

            if (cast)
                set_tim_esp(randint1(sides) + base, FALSE);
        }
        break;

    case 13:
        if (name) return "检查战利品";
        if (desc) return "鉴定一件物品。";

        if (cast)
        {
            if (!ident_spell(NULL))
                return NULL;
        }
        break;

    case 14:
        if (name) return "设置大型陷阱";
        if (desc) return "在脚下设置一个陷阱。经过该陷阱的怪物会受到各种效果影响。";

        if (cast)
            set_trap(py, px, feat_rogue_trap2);
        break;

    case 15:
        if (name) return "加速";
        if (desc) return "暂时为你提供加速效果。";

        {
            int base = spell_power(plev);
            int sides = spell_power(20 + plev);

            if (info) return info_duration(base, sides);

            if (cast)
                set_fast(randint1(sides) + base, FALSE);
        }
        break;

    /* Great Escapes */
    case 16:
        if (name) return "创造楼梯";
        if (desc) return "在你的正下方创造一阶楼梯。";

        if (cast)
            stair_creation(FALSE);
        break;

    case 17:
        if (name) return "恐慌打击";
        if (desc) return "攻击相邻的怪物并试图逃跑。";

        if (cast)
        {
            int dir = 0;
            int x, y;

            if (!get_rep_dir2(&dir)) return NULL;
            y = py + ddy[dir];
            x = px + ddx[dir];
            if (cave[y][x].m_idx)
            {
                py_attack(y, x, 0);
                if (randint0(p_ptr->skills.dis) < 7)
                    msg_print("你没能成功传送。");
                else
                    teleport_player(30, 0);
            }
            else
            {
                msg_print("在这个方向你没看到任何怪物");
                msg_print(NULL);
                return NULL;
            }
        }
        break;

    case 18:
        if (name) return "恐慌射击";
        if (desc) return "射击附近的怪物并试图逃跑。";

        if (cast)
        {
            if (!do_cmd_fire()) return NULL;
            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("你没能成功传送。");
            else
                teleport_player(30, 0);
        }
        break;

    case 19:
        if (name) return "恐慌召唤";
        if (desc) return "召唤援助并试图逃跑。";

        if (cast)
        {
            trump_summoning(damroll(2, 3), !fail, py, px, 0, SUMMON_THIEF, PM_ALLOW_GROUP);

            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("你没能成功传送。");
            else
                teleport_player(30, 0);
        }
        break;

    case 20:
        if (name) return "恐慌陷阱";
        if (desc) return "设置多个微弱的陷阱并试图逃跑。";

        if (cast)
        {
            int y = 0, x = 0;
            int dir;

            for (dir = 0; dir <= 8; dir++)
            {
                y = py + ddy_ddd[dir];
                x = px + ddx_ddd[dir];

                set_trap(y, x, feat_rogue_trap1);
            }

            if (randint0(p_ptr->skills.dis) < 7)
                msg_print("你没能成功传送。");
            else
                teleport_player(30, 0);
        }
        break;

    case 21:
        if (name) return "逃离楼层";
        if (desc) return "毫无延迟地逃离你当前的楼层。";

        if (cast)
        {
            if (!py_teleport_level("Are you sure? (Flee Level) ")) return NULL;
        }
        break;

    case 22:
        if (name) return "新的开始";
        if (desc) return "在短暂的延迟后重新生成当前的地下城楼层。";
        if (info) return info_delay(15, 20);

        if (cast)
            alter_reality();
        break;

    case 23:
        if (name) return "大型逃遁";
        if (desc) return "消耗极少的能量进行远距离传送。";

        {
            int range = plev * 5;

            if (info) return info_range(range);

            if (cast)
            {
                energy_use = 15;
                teleport_player(range, 0L);
            }
        }
        break;

    /* Book of Shadows */
    case 24:
        if (name) return "保护战利品";
        if (desc) return "在很长一段时间内，你物品栏中的物品将有几率抵抗损坏。";

        {
            int base = spell_power(plev*2);
            int sides = spell_power(plev*2);

            if (info) return info_duration(base, sides);

            if (cast)
                set_tim_inven_prot(randint1(sides) + base, FALSE);
        }
        break;

    case 25:
        if (name) return "传送到";
        if (desc) return "将一个可见怪物传送到你身边而不惊动它。";

        if (cast)
        {
            monster_type *m_ptr;
            char m_name[80];

            if (!target_set(TARGET_KILL)) return NULL;
            if (!cave[target_row][target_col].m_idx) return NULL;
            if (!player_has_los_bold(target_row, target_col)) return NULL;
            if (!projectable(py, px, target_row, target_col)) return NULL;

            /* This would cause bizarre behavior */
            if (cave[target_row][target_col].m_idx == p_ptr->riding) return NULL;

            m_ptr = &m_list[cave[target_row][target_col].m_idx];
            monster_desc(m_name, m_ptr, 0);
            if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
            msg_format("你命令 %s 返回。", m_name);
            teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        }
        break;

    case 26:
        if (name) return "大师级盗窃";
        if (desc) return "盗窃的终极境界。只需轻轻一触，你就能试图顺走怪物的财物。";

        if (cast)
            return _rogue_pick_pocket(100);
        break;

    case 27:
        if (name) return "暗影风暴";
        if (desc) return "发射一个巨大的黑暗球。";

        {
            int dam = spell_power(10 * (plev - 20) + p_ptr->to_d_spell);
            int rad = spell_power(4);

            if (info) return info_damage(0, 0, dam);

            if (cast)
            {
                if (!get_fire_dir(&dir)) return NULL;
                fire_ball(GF_DARK, dir, dam, rad);
            }
        }
        break;

    case 28:
        if (name) return "隐匿于阴影";
        if (desc) return "你被黑暗笼罩，你的火把光芒被魔法般地调暗了。";
        {
            int d = plev;
            if (info) return info_duration(spell_power(d), spell_power(d));
            if (cast)
            {
                if (p_ptr->tim_superstealth)
                {
                    msg_print("你已经隐匿在阴影中了。");
                    return NULL;
                }
                set_tim_superstealth(spell_power(randint1(d) + d), FALSE);
            }
        }
        break;

    case 29:
        if (name) return "盗贼巢穴";
        if (desc) return "作为盗贼领主，你可以随意召唤你的喽啰来提供援助。";
        if (cast)
        {
            int i;
            for (i = 0; i < 12; i++)
            {
                int attempt = 10;
                int my = 0, mx = 0;

                while (attempt--)
                {
                    scatter(&my, &mx, py, px, 4, 0);

                    /* Require empty grids */
                    if (cave_empty_bold2(my, mx)) break;
                }
                if (attempt < 0) continue;
                summon_specific(-1, my, mx, plev*3/2, SUMMON_THIEF, PM_FORCE_PET | PM_HASTE);
            }
        }
        break;

    case 30:
        if (name) return "设置终极陷阱";
        if (desc) return "在脚下设置一个极其强大的陷阱。经过该陷阱的怪物会受到各种强力效果影响。";

        if (cast)
            set_trap(py, px, feat_rogue_trap3);
        break;

    case 31:
        if (name) return "暗杀";
        if (desc) return "尝试秒杀一个正在沉睡的怪物。";

        if (cast)
        {
            int y, x, dir;
            if (!get_rep_dir2(&dir)) return NULL;
            if (dir == 5) return NULL;

            y = py + ddy[dir];
            x = px + ddx[dir];

            if (cave[y][x].m_idx)
            {
                monster_type *m_ptr = &m_list[cave[y][x].m_idx];
                if (MON_CSLEEP(m_ptr))
                    py_attack(y, x, ROGUE_ASSASSINATE);
                else
                {
                    msg_print("这只对睡眠中的怪物有效。");
                    return NULL;
                }
            }
            else
            {
                msg_print("这里没有怪物。");
                return NULL;
            }
        }
        break;

    }

    return "";
}

/****************************************************************************
 * Bonuses
 ****************************************************************************/
static void _calc_bonuses(void)
{
    /* rogues are decent shooters all around, but especially good with slings */
    slot_t slot = equip_find_obj(TV_BOW, SV_SLING); /* fyi, shooter_info not set yet ... */
    if (slot) p_ptr->skills.thb += 20 + p_ptr->lev;
}

static caster_info * _caster_info(void)
{
    static caster_info me = {0};
    static bool init = FALSE;
    if (!init)
    {
        me.magic_desc = "法术";
        me.encumbrance.max_wgt = 400;
        me.encumbrance.weapon_pct = 33;
        me.encumbrance.enc_wgt = 1000;
        me.options = CASTER_GLOVE_ENCUMBRANCE;
        init = TRUE;
    }
    if (p_ptr->realm1 == REALM_BURGLARY)
    {
        me.which_stat = A_DEX;
        me.min_level = 1;
        me.min_fail = 0;
    }
    else
    {
        me.which_stat = A_INT;
        me.min_level = 5;
        me.min_fail = 5;
    }
    return &me;
}

static void _birth(void)
{
    py_birth_obj_aux(TV_SWORD, SV_DAGGER, 1);
    py_birth_obj_aux(TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR, 1);
    py_birth_obj_aux(TV_SCROLL, SV_SCROLL_TELEPORT, randint1(3));
    py_birth_spellbooks();

    p_ptr->au += 200;
}

/****************************************************************************
 * Public
 ****************************************************************************/
class_t *rogue_get_class(void)
{
    static class_t me = {0};
    static bool init = FALSE;

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 45,  37,  36,   5,  32,  24,  60,  60};
    skills_t xs = { 15,  12,  10,   0,   0,   0,  21,  14};

        me.name = "盗贼";
        me.desc = "盗贼是喜欢依靠狡猾来生存的角色，但在必要时也有能力在困境中杀出一条血路。盗贼善于寻找密门和隐藏的陷阱，是解除陷阱和开锁的大师。他们非凡的潜行能力允许盗贼在沉睡的生物周围偷偷溜过而无需战斗；他们还可以背刺逃跑的怪物，或者用致命的第一击出其不意地袭击敌人。盗贼在远程战斗方面相当不错，并且在使用投石索射击时会获得特殊加成。\n \n盗贼可以选择一个魔法领域——咒术、死亡、王牌、奥秘、工匠、律法或行窃。除了最后一个领域外，盗贼在可以学习哪些法术上有一定的限制，并且他们学习新法术的速度不是很快。行窃(Burglary)领域是盗贼独有的，也是该职业的专长；它提供了用于设置陷阱、妙手空空、与其他盗贼谈判以及从困境中逃脱的法术。行窃盗贼是黑市的代理人，能从该商店获得优惠的价格。行窃盗贼使用敏捷作为他们的施法属性，并且可以从冒险一开始就使用这门特殊的艺术；然而，选择其他领域的盗贼依赖于智力，并且在达到5级之前无法学习魔法法术。";

        me.stats[A_STR] =  1;
        me.stats[A_INT] =  1;
        me.stats[A_WIS] = -1;
        me.stats[A_DEX] =  3;
        me.stats[A_CON] =  0;
        me.stats[A_CHR] =  1;
        me.base_skills = bs;
        me.extra_skills = xs;
        me.life = 100;
        me.base_hp = 12;
        me.exp = 125;
        me.pets = 40;
        me.flags = CLASS_SENSE1_FAST | CLASS_SENSE1_STRONG |
                   CLASS_SENSE2_MED | CLASS_SENSE2_STRONG;
        
        me.birth = _birth;
        me.calc_bonuses = _calc_bonuses;
        me.caster_info = _caster_info;
        /* TODO: This class uses spell books, so we are SOL
        me.get_spells = _get_spells;*/
        me.character_dump = spellbook_character_dump;
        init = TRUE;
    }

    return &me;
}
