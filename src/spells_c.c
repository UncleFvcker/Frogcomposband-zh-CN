#include "angband.h"

void cause_wounds_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "造成轻伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图对单个敌人造成伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(3, spell_power(8), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_1, dir, spell_power(damroll(3, 8) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "造成中度伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图对单个敌人造成伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(8, spell_power(8), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_2, dir, spell_power(damroll(8, 8) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_III_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "造成重伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图对单个敌人造成伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(10, spell_power(15), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_3, dir, spell_power(damroll(10, 15) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void cause_wounds_IV_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "造成致命伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图对单个敌人造成伤害。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(15, spell_power(15), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CAUSE_4, dir, spell_power(damroll(15, 15) + p_ptr->to_d_spell), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void clairvoyance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "千里眼");
        break;
    case SPELL_DESC:
        var_set_string(res, "映射并照亮整个地下城层，并在一段时间内赋予心灵感应。");
        break;
    case SPELL_CAST:
        virtue_add(VIRTUE_KNOWLEDGE, 1);
        virtue_add(VIRTUE_ENLIGHTENMENT, 1);

        wiz_lite(p_ptr->tim_superstealth > 0);

        if (!p_ptr->telepathy)
            set_tim_esp(randint1(30) + 25, FALSE);

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void clear_mind_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "头脑清明");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "玩家回复 2+(L/30) 点法力值(SP)。如果玩家有任何宠物，此技能将无效。");
        break;
    case SPELL_CAST:
    {
        int amt;

        var_set_bool(res, FALSE);
        if (total_friends)
        {
            msg_print("你现在需要集中注意力在你的宠物上。");
            return;
        }
        if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_RAGE_MAGE))
        {
            msg_print("你的头脑依然昏沉。");
            return;
        }

        msg_print("你觉得头脑清醒了一点。");

        amt = 2 + p_ptr->lev/30;

        sp_player(amt);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_clear_mind(void) { return cast_spell(clear_mind_spell); }

void confuse_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "困惑术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图使一只或多只怪物陷入困惑。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 40)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            confuse_monster(dir, p_ptr->lev*2);
        }
        else
            confuse_monsters(p_ptr->lev*2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cold_touch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寒冷之触");
        break;
    case SPELL_DESC:
        var_set_string(res, "用你冰冷的手指冻结事物！");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(2 * p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的手变得非常冰冷。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的手变暖和了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以通过触碰来冻结事物。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int x, y;
        cave_type *c_ptr;

        if (!get_rep_dir2(&dir))
        {
            var_set_bool(res, FALSE);
            break;
        }
        var_set_bool(res, TRUE);
        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];

        if (!c_ptr->m_idx)
        {
            msg_print("你在空中挥动着双手。");
            break;
        }
        fire_bolt(GF_COLD, dir, spell_power(2 * p_ptr->lev));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_cold_touch(void) { return cast_spell(cold_touch_spell); }

void confusing_lights_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混乱之光");
        break;
    case SPELL_DESC:
        var_set_string(res, "散发出令人困惑的光芒，使附近的怪物减速、震慑、困惑、恐惧并冻结。");
        break;
    case SPELL_CAST:
        msg_print("你用令人眼花缭乱的混乱之光照射附近的怪物！");
        slow_monsters(p_ptr->lev * 3);
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(p_ptr->lev * 3);
        turn_monsters(p_ptr->lev * 3);
        stasis_monsters(p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void crafting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "工匠技艺");
        break;
    case SPELL_DESC:
        var_set_string(res, "使选定的武器、护甲或弹药成为高级(ego)物品。");
        break;
    case SPELL_CAST:
    {
        obj_prompt_t prompt = {0};
        bool         okay = FALSE;
        char         o_name[MAX_NLEN];

        var_set_bool(res, FALSE);

        prompt.prompt = "要附魔哪件物品？";
        prompt.error = "你没有东西可以附魔。";
        prompt.filter = object_is_weapon_armour_ammo;
        prompt.where[0] = INV_PACK;
        prompt.where[1] = INV_EQUIP;
        prompt.where[2] = INV_QUIVER;
        prompt.where[3] = INV_FLOOR;

        obj_prompt(&prompt);
        if (!prompt.obj || !prompt.obj->number) return;

        object_desc(o_name, prompt.obj, (OD_OMIT_PREFIX | OD_NAME_ONLY));

        if (!object_is_nameless(prompt.obj))
        {
            msg_print("你无法对该物品进行进一步附魔了。");
            return;
        }

        if ((!object_is_ammo(prompt.obj)) && (prompt.obj->number > 1))
        {
            msg_print("你不能同时对多件物品使用工匠技艺。");
            return;
        }

        if ((object_is_(prompt.obj, TV_SWORD, SV_POISON_NEEDLE)) ||
            (object_is_(prompt.obj, TV_SWORD, SV_RUNESWORD)) ||
            (object_is_(prompt.obj, TV_POLEARM, SV_DEATH_SCYTHE)))
        {
            msg_print("你无法附魔该物品。");
            return;
        }

        if (prompt.obj->number > 59)
        {
            msg_print("你不能同时对超过 59 个投射物使用工匠技艺。");
            return;
        }

        if (prompt.obj->number > 30)
        {
            int mahis = ((prompt.obj->number * 20) - 597) / 6;
            if (!get_check(format("附魔有 %s %d%% 的几率失败。无论如何都要继续吗？", ((mahis / 10) == 8) ? "``" : "a", mahis)))
            return;
        }
        
        if (object_is_nameless(prompt.obj))
        {
            if (object_is_ammo(prompt.obj) && randint1(30) > (prompt.obj->number - 30))
            {
                if (brand_weapon_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
            else if (object_is_weapon(prompt.obj) && prompt.obj->number == 1)
            {
                if (brand_weapon_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
            else if (object_is_armour(prompt.obj) && prompt.obj->number == 1)
            {
                if (brand_armour_aux(prompt.obj))
                {
                    prompt.obj->discount = 99;
                    okay = TRUE;
                }
            }
        }

        msg_format("%s 发出了明亮的光芒%s！", o_name,
                ((prompt.obj->number > 1) ? "" : "s"));

        if (!okay)
        {
            if (flush_failure) flush();
            msg_print("附魔失败了。");
            if (one_in_(3)) virtue_add(VIRTUE_ENCHANTMENT, -1);
        }
        else
        {
            virtue_add(VIRTUE_ENCHANTMENT, 1);
            object_origins(prompt.obj, ORIGIN_CRAFTING);
            prompt.obj->mitze_type = 0;
            obj_identify_fully(prompt.obj);
            if (!prompt.obj->mitze_type) object_mitze(prompt.obj, MITZE_ID);
            obj_display(prompt.obj);
            obj_release(prompt.obj, OBJ_RELEASE_ENCHANT);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_crafting(void) { return cast_spell(crafting_spell); }

void create_darkness_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造黑暗");
        break;
    case SPELL_DESC:
        var_set_string(res, "使附近的区域和房间内部变暗。");
        break;
    case SPELL_CAST:
        unlite_area(0, 3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_food_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造食物");
        break;
    case SPELL_DESC:
        if (p_ptr->prace == RACE_HOBBIT)
            var_set_string(res, "是时候吃第二顿早餐了！做一顿美味的饭菜。");
        else
            var_set_string(res, "制造一份美味的口粮。");
        break;
    case SPELL_CAST:
    {
        object_type forge;

        object_prep(&forge, lookup_kind(TV_FOOD, SV_FOOD_RATION));
        drop_near(&forge, -1, py, px);
        object_origins(&forge, ORIGIN_ACQUIRE);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_create_food(void) { return cast_spell(create_food_spell); }

void create_major_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造大型陷阱");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你的脚下设置一个陷阱。这个陷阱会对路过的怪物产生各种影响。");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_minor_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造小型陷阱");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你的脚下设置一个微弱的陷阱。这个陷阱会对路过的怪物产生各种微弱的影响。");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void create_ultimate_trap_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "制造终极陷阱");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你的脚下设置一个极其强大的陷阱。这个陷阱会对路过的怪物产生各种强力的影响。");
        break;
    case SPELL_CAST:
        set_trap(py, px, feat_rogue_trap3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void crusade_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "圣战");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图魅惑视线内的所有善良怪物，并恐吓所有未被魅惑的怪物，召唤大量骑士，并提供英雄气概、祝福、加速和防护邪恶。");
        break;
    case SPELL_CAST:
    {
        int base = 25;
        int sp_sides = 20 + p_ptr->lev;
        int sp_base = p_ptr->lev;
        int i;

        project_hack(GF_CRUSADE, p_ptr->lev*4);
        for (i = 0; i < 12; i++)
        {
            int attempt = 10;
            int my = 0, mx = 0;

            while (attempt--)
            {
                scatter(&my, &mx, py, px, 4, 0);
                if (cave_empty_bold2(my, mx)) break;
            }
            if (attempt < 0) continue;
            summon_specific(-1, my, mx, p_ptr->lev, SUMMON_KNIGHT, (PM_ALLOW_GROUP | PM_FORCE_PET | PM_HASTE));
        }
        set_hero(randint1(base) + base, FALSE);
        set_blessed(randint1(base) + base, FALSE);
        set_fast(randint1(sp_sides) + sp_base, FALSE);
        set_protevil(randint1(base) + base, FALSE);
        fear_clear_p();

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_crusade(void) { return cast_spell(crusade_spell); }

void cure_poison_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗毒素");
        break;
    case SPELL_DESC:
        var_set_string(res, "治愈中毒状态。");
        break;
    case SPELL_CAST:
        set_poisoned(p_ptr->poisoned - MAX(100, p_ptr->poisoned / 5), TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗轻伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "治愈割伤并恢复少量生命值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(2, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(2, 10)));
        set_cut(p_ptr->cut - 10, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗中度伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "治愈割伤并恢复更多生命值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(4, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(4, 10)));
        set_cut((p_ptr->cut / 2) - 20, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cure_wounds_III_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "治疗重伤");
        break;
    case SPELL_DESC:
        var_set_string(res, "治愈割伤、震慑并大量恢复生命值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(8, spell_power(10), 0));
        break;
    case SPELL_CAST:
        hp_player(spell_power(damroll(8, 10)));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void curing_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恢复术");
        break;
    case SPELL_DESC:
        var_set_string(res, "它能治疗你一些生命值，并治愈失明、中毒、困惑、震慑、割伤和幻觉状态。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(50)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(50));
        set_blind(0, TRUE);
        set_poisoned(p_ptr->poisoned - MAX(150, p_ptr->poisoned / 3), TRUE);
        set_confused(0, TRUE);
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        set_image(0, TRUE);
        set_shero(0,TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _darkness_storm_I_dam(void)
{
    if (p_ptr->pclass == CLASS_WILD_TALENT) /* Wild-Talents gain both I and II versions ... */
        return 100 + py_prorata_level_aux(100, 1, 1, 0);
    return 100 + py_prorata_level_aux(200, 1, 1, 2);
}

void darkness_storm_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑暗风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的黑暗法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_darkness_storm_I_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你唤起了一场黑暗风暴。");
        fire_ball(
            GF_DARK,
            dir,
            spell_power(_darkness_storm_I_dam() + p_ptr->to_d_spell),
            spell_power(4)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _darkness_storm_II_dam(void)
{
    return py_prorata_level_aux(450, 1, 0, 2);
}

void darkness_storm_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑暗风暴");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个威力无与伦比的巨大黑暗法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_darkness_storm_II_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你唤起了一场黑暗风暴。");
        fire_ball(GF_DARK, dir,
            spell_power(_darkness_storm_II_dam() + p_ptr->to_d_spell),
            spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void day_of_the_dove_spell(int cmd, variant *res)
{
    int power = spell_power(p_ptr->lev * 2);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "和平之日");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图魅惑视线内的所有怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(power));
        break;
    case SPELL_CAST:
        charm_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dazzle_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "目眩术");
        break;
    case SPELL_DESC:
        var_set_string(res, "散发出令人目眩的光芒，震慑、困惑并恐吓附近的怪物。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了散发目眩之光的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了散发目眩之光的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以散发出令人困惑的致盲辐射。");
        break;
    case SPELL_CAST:
        stun_monsters(5 + p_ptr->lev/5);
        confuse_monsters(p_ptr->lev * 4);
        turn_monsters(p_ptr->lev * 4);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_dazzle(void) { return cast_spell(dazzle_spell); }

void detect_life_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测生命");
        break;
    case SPELL_DESC:
        var_set_string(res, "定位附近活着的怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_living(DETECT_RAD_DEFAULT, "你感应到周围有生命存在。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_unlife_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测非生命");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测你附近所有的非生命怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_nonliving(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void demon_breath_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐火焰/幽冥");
        break;
    case SPELL_DESC:
        var_set_string(res, "向你的对手喷吐出强大的火焰或幽冥冲击。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 3)));
        break;
    case SPELL_CAST:
    {
        int type = (one_in_(2) ? GF_NETHER : GF_FIRE);
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;

        stop_mouth();

        msg_format("你喷吐出%s。", (type == GF_NETHER) ? "幽冥" : "火焰");

        fire_ball(type, dir, spell_power(p_ptr->lev * 3), -(p_ptr->lev / 15) - 1);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev/3);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void destruction_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毁灭真言");
        break;
    case SPELL_DESC:
        var_set_string(res, "摧毁你附近的一切……当然，除了你之外。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(spell_power(p_ptr->lev * 4)));
        break;
    case SPELL_CAST:
        destroy_area(py, px, 12 + randint1(4), spell_power(4 * p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_destruction(void) { return cast_spell(destruction_spell); }

static void _detect_curses(obj_ptr obj)
{
    if (object_is_cursed(obj))
        obj->feeling = FEEL_CURSED;
}

void detect_curses_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测诅咒");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测你物品栏中的受诅咒物品。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你能感觉到邪恶魔法。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能感觉到邪恶魔法了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你能感觉到邪恶魔法的危险。");
        break;
    case SPELL_CAST:
    {
        pack_for_each(_detect_curses);
        equip_for_each(_detect_curses);
        quiver_for_each(_detect_curses);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_curses(void) { return cast_spell(detect_curses_spell); }

void detect_doors_stairs_traps_spell(int cmd, variant *res)
{
    int rad = DETECT_RAD_DEFAULT;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测门与陷阱");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测你附近的门、楼梯和陷阱。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_radius(rad));
        break;
    case SPELL_CAST:
        detect_traps(rad, TRUE);
        detect_doors(rad);
        detect_stairs(rad);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_doors_stairs_traps(void) { return cast_spell(detect_doors_stairs_traps_spell); }

void detect_evil_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测邪恶");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的邪恶怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_evil(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_menace_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测凶意");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近具有威胁性的怪物。只能侦测到有智能的怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_mind(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void detect_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的怪物。");
        break;
    case SPELL_CAST:
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_monsters(void) { return cast_spell(detect_monsters_spell); }

void detect_objects_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测物品");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的物品。");
        break;
    case SPELL_CAST:
        detect_objects_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_objects(void) { return cast_spell(detect_objects_spell); }

void detect_traps_spell(int cmd, variant *res)
{
    int rad = DETECT_RAD_DEFAULT;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测陷阱");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测你附近的陷阱。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_radius(rad));
        break;
    case SPELL_CAST:
        detect_traps(rad, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_traps(void) { return cast_spell(detect_traps_spell); }

void detect_treasure_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测宝藏");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的宝藏。");
        break;
    case SPELL_CAST:
        detect_treasure(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_detect_treasure(void) { return cast_spell(detect_treasure_spell); }

void detection_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "侦测术");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测你附近所有的怪物、陷阱、门、楼梯、宝藏和物品。");
        break;
    case SPELL_CAST:
        detect_all(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dimension_door_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "次元门");
        break;
    case SPELL_DESC:
        var_set_string(res, "打开一扇通往另一个维度的传送门，并极其精确地传送到附近的位置。");
        break;
    case SPELL_CAST:
        var_set_bool(res, dimension_door(p_ptr->lev / 2 + 10));
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
            var_set_int(res, 30);
        else
            default_spell(cmd, res);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_dimension_door(void) { return cast_spell(dimension_door_spell); }

void disintegrate_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "分解术");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的分解法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev + 70 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dam = spell_power(p_ptr->lev + 70 + p_ptr->to_d_spell);
        int rad = 3 + p_ptr->lev / 40;
        int dir;

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_DISINTEGRATE, dir, dam, rad);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_evil_spell(int cmd, variant *res)
{
    int sides = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散邪恶");
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害视线内的所有邪恶怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        dispel_evil(randint1(sides) + spell_power(p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_life_spell(int cmd, variant *res)
{
    int ds = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散生命");
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害视线内的所有活物怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(1, ds, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        dispel_living(randint1(ds) + spell_power(p_ptr->to_d_spell));
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散魔法");
        break;
    case SPELL_DESC:
        var_set_string(res, "对单只怪物进行驱散，抵消其无敌护盾和暂时的加速效果。");
        break;
    case SPELL_CAST:
    {
        int m_idx;

        var_set_bool(res, FALSE);
        if (!target_set(TARGET_KILL)) return;
        m_idx = cave[target_row][target_col].m_idx;
        if (!m_idx) return;

        var_set_bool(res, TRUE);
        if (!player_has_los_bold(target_row, target_col)) return;
        if (!projectable(py, px, target_row, target_col)) return;
        dispel_monster_status(m_idx);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void dispel_undead_spell(int cmd, variant *res)
{
    int dice = 1;
    int sides = spell_power(p_ptr->lev * 5);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "伤害视线内的所有死灵怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dice, sides, spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        if(project_hack(GF_DISP_UNDEAD, damroll(dice, sides) + spell_power(p_ptr->to_d_spell)))
            virtue_add(VIRTUE_UNLIFE, -2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dominate_living_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "支配活物");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_CONTROL_LIVING, dir, p_ptr->lev, 0);
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+3)/4);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void dominate_living_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "群体支配活物");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        project_hack(GF_CONTROL_LIVING, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, (p_ptr->lev+20)/2);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void drain_mana_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸取法力");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图从选定的怪物身上吸取法力。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("%d+d%d", spell_power(p_ptr->lev), spell_power(p_ptr->lev*3)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball_hide(GF_DRAIN_MANA, dir, spell_power(randint1(p_ptr->lev*3)+p_ptr->lev), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void earthquake_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "地震术");
        break;
    case SPELL_DESC:
        var_set_string(res, "墙壁会颤抖，大地会震动。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了破坏地下城的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了破坏地下城的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以让你周围的地下城坍塌。");
        break;
    case SPELL_CAST:
        earthquake(py, px, 10);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_earthquake(void) { return cast_spell(earthquake_spell); }

void eat_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吞噬魔法");
        break;
    case SPELL_DESC:
        var_set_string(res, "消耗魔法装置以恢复法力值(SP)。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的魔法物品看起来很美味。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的魔法物品看起来不再美味了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以吞噬魔法能量供自己使用。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (eat_magic(20 + p_ptr->lev * 8 / 5)) /* skillmasters can do this on CL1 ... */
            var_set_bool(res, TRUE);
        break;
    case SPELL_FAIL_MIN:
        var_set_int(res, 11);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_eat_magic(void) { return cast_spell(eat_magic_spell); }

void eat_rock_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吞噬岩石");
        break;
    case SPELL_DESC:
        var_set_string(res, "吞噬附近的岩石。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("这些墙壁看起来很美味。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("这些墙壁看起来令人倒胃口。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以吞噬坚固的岩石。");
        break;
    case SPELL_CAST:
    {
        int x, y;
        cave_type *c_ptr;
        feature_type *f_ptr, *mimic_f_ptr;
        int dir = 0;

        var_set_bool(res, FALSE);

        if (!get_rep_dir2(&dir)) break;
        y = py + ddy[dir];
        x = px + ddx[dir];
        c_ptr = &cave[y][x];
        f_ptr = &f_info[c_ptr->feat];
        mimic_f_ptr = &f_info[get_feat_mimic(c_ptr)];

        stop_mouth();

        if (!have_flag(mimic_f_ptr->flags, FF_HURT_ROCK))
        {
            msg_print("你不能吃这个地形。");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_PERMANENT))
        {
            msg_format("哎哟！这%s比你的牙齿还硬！",
                f_name + mimic_f_ptr->name);

            break;
        }
        else if (c_ptr->m_idx)
        {
            monster_type *m_ptr = &m_list[c_ptr->m_idx];
            msg_print("有东西挡在路上！");
            if (!m_ptr->ml || !is_pet(m_ptr)) py_attack(y, x, 0);
            break;
        }
        else if (have_flag(f_ptr->flags, FF_TREE))
        {
            msg_print("你不喜欢这种木头的味道！");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_GLASS))
        {
            msg_print("你不喜欢这种玻璃的味道！");
            break;
        }
        else if (have_flag(f_ptr->flags, FF_DOOR) || have_flag(f_ptr->flags, FF_CAN_DIG))
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 500, PY_FOOD_MAX - 1));
            else
                set_food(p_ptr->food + 3000);
        }
        else if (have_flag(f_ptr->flags, FF_MAY_HAVE_GOLD) || have_flag(f_ptr->flags, FF_HAS_GOLD))
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 1000, PY_FOOD_MAX - 1));
            else
                set_food(p_ptr->food + 5000);
        }
        else
        {
            if (elemental_is_(ELEMENTAL_EARTH))
                set_food(MIN(p_ptr->food + 2000, PY_FOOD_MAX - 1));
            else
            {
                msg_format("这%s非常管饱！",
                    f_name + mimic_f_ptr->name);

                set_food(p_ptr->food + 10000);
            }
        }

        /* Destroy the wall */
        cave_alter_feat(y, x, FF_HURT_ROCK);

        /* Move the player */
        move_player_effect(y, x, MPE_DONT_PICKUP);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_eat_rock(void) { return cast_spell(eat_rock_spell); }

void evil_bless_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "邪恶祝福");
        break;
    default:
        bless_spell(cmd, res);
        break;
    }
}

void evocation_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 4 + p_ptr->to_d_spell);
    int power = spell_power(p_ptr->lev * 4);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "退魔敕令");
        break;
    case SPELL_DESC:
        var_set_string(res, "驱散、恐吓并放逐视线内的所有怪物。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
        dispel_monsters(dam);
        turn_monsters(power);
        banish_monsters(power);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void minor_enchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "次级附魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试附魔一件武器、弹药或护甲。");
        break;
    case SPELL_CAST:
        var_set_bool(res, craft_enchant(2 + p_ptr->lev/5, 1));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void enchantment_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "附魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试附魔一件武器、弹药或护甲。");
        break;
    case SPELL_CAST:
        var_set_bool(res, craft_enchant(15, 3));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_enchantment(void) { return cast_spell(enchantment_spell); }

void enslave_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "奴役死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "尝试奴役一只死灵怪物。");
        break;
    case SPELL_CAST:
    {
        int power, dir;
        if (p_ptr->pclass == CLASS_NECROMANCER)
            power = spell_power(p_ptr->lev*3);
        else
            power = spell_power(p_ptr->lev);

        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        control_one_undead(dir, power);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void explosive_rune_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "爆炸符文");
        break;
    case SPELL_DESC:
        var_set_string(res, "放置一个会在怪物经过时爆炸的符文。");
        break;
    case SPELL_CAST:
        msg_print("你小心翼翼地布置了一个爆炸符文……");
        explosive_rune();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void eye_for_an_eye_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "以眼还眼");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内赋予特殊光环。当你受到怪物攻击时，该怪物会受到与你承受的伤害相同的伤害。");
        break;
    case SPELL_CAST:
        set_tim_eyeeye(spell_power(randint1(10) + 10), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火球术");
        break;
    case SPELL_DESC:
        var_set_string(res, "在选定的目标处生成一个火球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 30 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(
            GF_FIRE,
            dir,
            spell_power(3*p_ptr->lev/2 + 30 + p_ptr->to_d_spell),
            2
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发火焰箭或火焰射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_FIRE,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void flow_of_lava_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "熔岩流");
        break;
    case SPELL_DESC:
        var_set_string(res, "以你为中心生成一个火球，将周围的地板转化为岩浆。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(55 + p_ptr->lev + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
        fire_ball(GF_FIRE, 0, spell_power(55 + p_ptr->lev + p_ptr->to_d_spell), 3);
        fire_ball_hide(GF_LAVA_FLOW, 0, 2 + randint1(2), 3);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void force_branding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "原力烙印");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时为你的武器赋予原力属性。");
        break;
    case SPELL_CAST:
    {
        int base = spell_power(p_ptr->lev / 4);
        set_tim_force(base + randint1(base), FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void frost_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "霜冻球");
        break;
    case SPELL_DESC:
        var_set_string(res, "在选定目标处生成一个霜冻球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 25 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_COLD, dir, spell_power(3*p_ptr->lev/2 + 25 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void frost_bolt_spell(int cmd, variant *res)
{
    int dd = 4 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寒霜箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发寒霜箭或寒霜射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt_or_beam(
            beam_chance(),
            GF_COLD,
            dir,
            spell_power(damroll(dd, ds) + p_ptr->to_d_spell)
        );
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void genocide_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "灭绝");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图将指定种类的所有怪物从当前层移除，此举会使你力竭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(spell_power(p_ptr->lev*3)));
        break;
    case SPELL_CAST:
    {
        int power = spell_power(p_ptr->lev*3);
        var_set_bool(res, symbol_genocide(power, TRUE));
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void glyph_of_warding_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "守护结界");
        break;
    case SPELL_DESC:
        var_set_string(res, "在你脚下的地板上设置一个结界。当你处于结界上时怪物无法攻击你，但它们可以尝试破坏结界。");
        break;
    case SPELL_CAST:
        warding_glyph();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void grow_mold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "培育霉菌");
        break;
    case SPELL_DESC:
        var_set_string(res, "用发霉的东西包围自己。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然对霉菌产生了一种亲和感。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你突然对霉菌产生了一种反感。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以使霉菌在你附近生长。");
        break;
    case SPELL_CAST:
    {
        int i;
        for (i = 0; i < 8; i++)
        {
            summon_specific(-1, py, px, p_ptr->lev, SUMMON_BIZARRE1, PM_FORCE_PET);
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_grow_mold(void) { return cast_spell(grow_mold_spell); }

