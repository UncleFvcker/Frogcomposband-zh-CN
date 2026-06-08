#include "angband.h"

void satisfy_hunger_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "满足饥饿");
        break;
    case SPELL_DESC:
        var_set_string(res, "用纯粹的美味填饱你的肚子。");
        break;
    case SPELL_CAST:
        set_food(PY_FOOD_MAX - 1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_satisfy_hunger(void) { return cast_spell(satisfy_hunger_spell); }

void scare_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恐吓怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图恐吓附近的一只怪物。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        stop_mouth();
        /*
        msg_print("You make a horrible scream!";
        msg_print("You emit an eldritch howl!");
        */
        fear_monster(dir, p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void scare_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "惊骇术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图恐吓一只或多只怪物。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            fear_monster(dir, p_ptr->lev);
        }
        else
        {
            turn_monsters(p_ptr->lev);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void self_knowledge_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "自我认知");
        break;
    case SPELL_DESC:
        var_set_string(res, "提供关于你当前的抗性、武器力量以及属性上限的有用信息。");
        break;
    case SPELL_CAST:
        self_knowledge();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sense_surroundings_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "感知周围");
        break;
    default:
        magic_mapping_spell(cmd, res);
        break;
    }
}

void shadow_shifting_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "阴影位移");
        break;
    case SPELL_DESC:
        var_set_string(res, "在短暂延迟后重置当前地下城层。");
        break;
    case SPELL_CAST:
        msg_print("你开始四处走动。");
        alter_reality();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void shoot_arrow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "射箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "射出一支箭。");
        break;
    case SPELL_INFO:
    {
        int slot = equip_find_first(object_is_melee_weapon);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            var_set_string(res, info_damage(o_ptr->dd, o_ptr->ds, o_ptr->to_d));
        }
        else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        {
            /* XXX Remove this spell for Possessors.
            monster_race *r_ptr = &r_info[p_ptr->current_r_idx];
            int i;
            for (i = 0; i < 4; i++)
            {
                if (r_ptr->blows[i].method == RBM_SHOOT)
                {
                    var_set_string(res, info_damage(r_ptr->blows[i].effects[0].dd, r_ptr->blows[i].effects[0].ds, 0));
                    return;
                }
            }
            */
        }
        else
            var_set_string(res, info_damage(0, 0, 1));
        break;
    }
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = 1;
        int slot;
        
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你射出了一支箭。");

        slot = equip_find_first(object_is_melee_weapon);
        if (slot)
        {
            object_type *o_ptr = equip_obj(slot);
            dam = damroll(o_ptr->dd, o_ptr->ds)+ o_ptr->to_d;
            if (dam < 1) dam = 1;
        }
        else if (p_ptr->prace == RACE_MON_POSSESSOR || p_ptr->prace == RACE_MON_MIMIC)
        {
            /* XXX Remove me! */
        }

        fire_bolt(GF_ARROW, dir, spell_power(dam));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void shriek_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "尖啸");
        break;
    case SPELL_DESC:
        var_set_string(res, "以你为中心生成一个巨大的声波球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的声带变得强壮多了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的声带变得虚弱多了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以发出一声可怕的尖啸。");
        break;
    case SPELL_CAST:
        stop_mouth();
        fire_ball(GF_SOUND, 0, spell_power(2 * p_ptr->lev), 8);
        aggravate_monsters(0);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_shriek(void) { return cast_spell(shriek_spell); }

void sleeping_dust_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "睡眠粉");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        msg_print("你撒出一些魔法粉尘……");
        if (p_ptr->lev < 25) sleep_monsters_touch();
        else sleep_monsters(p_ptr->lev);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sleep_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "睡眠术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图使一只或多只怪物陷入沉睡。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            sleep_monster(dir, p_ptr->lev*2);
        }
        else
            sleep_monsters(p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void slow_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "减速术");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图使一只或多只怪物减速。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_power(p_ptr->lev * 2));
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (p_ptr->lev < 30)
        {
            int dir = 0;
            if (!get_fire_dir(&dir)) return;
            slow_monster(dir, p_ptr->lev * 2);
        }
        else
            slow_monsters(p_ptr->lev * 2);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void smell_metal_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "嗅金属");
        break;
    case SPELL_DESC:
        var_set_string(res, "闻出附近金属的气味。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你闻到了金属的气味。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能闻到金属的气味了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以闻出附近的贵重金属。");
        break;
    case SPELL_CAST:
        stop_mouth();
        detect_treasure(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void smell_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "嗅怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "侦测附近的怪物。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你闻到了肮脏怪物的气味。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能闻到肮脏怪物的气味了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以闻出附近的怪物。");
        break;
    case SPELL_CAST:
        stop_mouth();
        detect_monsters_normal(DETECT_RAD_DEFAULT);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sp_to_hp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "法力转生命");
        break;
    case SPELL_DESC:
        var_set_string(res, "将法力值转化为生命值");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你时不时地体验到一种魔法般的治愈感。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再体验到那种魔法般的治愈感了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "有时你的气血会涌入肌肉。");
        break;
    case SPELL_PROCESS:
        if (one_in_(2000))
        {
            int wounds = p_ptr->mhp - p_ptr->chp;

            if (wounds > 0)
            {
                int healing = p_ptr->csp;

                if (healing > wounds)
                    healing = wounds;

                hp_player(healing);
                p_ptr->csp -= healing;

                p_ptr->redraw |= (PR_MANA);
            }
        }
        break;
    case SPELL_CAST:
        if (p_ptr->csp >= p_ptr->lev / 5)
        {
            p_ptr->csp -= p_ptr->lev / 5;
            p_ptr->redraw |= PR_MANA;
            hp_player(p_ptr->lev);
        }
        else
            msg_print("你转换失败了。");

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void spit_acid_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "喷吐酸液");
        break;
    case SPELL_DESC:
        if (p_ptr->lev < 25)
            var_set_string(res, "喷吐出一发酸液箭。");
        else
            var_set_string(res, "喷吐出一个酸液球。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了喷吐酸液的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了喷吐酸液的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以喷吐酸液（伤害为 等级*2）。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev/5);
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            stop_mouth();
            msg_print("你喷吐出酸液……");
            if (p_ptr->lev < 25) fire_bolt(GF_ACID, dir, spell_power(p_ptr->lev * 2));
            else fire_ball(GF_ACID, dir, spell_power(p_ptr->lev * 2), 2);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_spit_acid(void) { return cast_spell(spit_acid_spell); }

static int _starburst_I_dam(void)
{
    if (p_ptr->pclass == CLASS_WILD_TALENT) /* Wild-Talents gain both I and II versions ... */
        return 100 + py_prorata_level_aux(100, 1, 1, 0);
    return 100 + py_prorata_level_aux(200, 1, 1, 2);
}

void starburst_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "星辰爆裂");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个由强光组成的巨大法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_starburst_I_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你唤起了星辰爆裂。");
        fire_ball(GF_LITE, dir, spell_power(_starburst_I_dam() + p_ptr->to_d_spell), spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _starburst_II_dam(void)
{
    return py_prorata_level_aux(450, 1, 0, 2);
}

void starburst_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "星辰爆裂");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个由强光组成的巨大法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(_starburst_II_dam() + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你唤起了星辰爆裂。");
        fire_ball(GF_LITE, dir, 
            spell_power(_starburst_II_dam() + p_ptr->to_d_spell),
            spell_power(4));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void sterility_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "绝育术");
        break;
    case SPELL_DESC:
        var_set_string(res, "阻止可繁殖的怪物去……嗯……做那种羞羞的事。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你能让周围的一切都感到头疼。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你听到了一阵如释重负的集体叹息。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以导致大规模的阳痿。");
        break;
    case SPELL_CAST:
        msg_print("你突然感到头疼！");
        take_hit(DAMAGE_LOSELIFE, randint1(17) + 17, "强行禁食的负担");

        /* Fake a population explosion. */
        num_repro += MAX_REPRO;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_sterility(void) { return cast_spell(sterility_spell); }

void stinking_cloud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "臭云术");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个毒素球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(10 + p_ptr->lev / 2 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_POIS, dir, spell_power(10 + p_ptr->lev / 2 + p_ptr->to_d_spell), spell_power(2));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void stone_skin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "石肤术");
        break;
    case SPELL_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CAST:
        set_shield(randint1(30) + 20, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_stone_skin(void) { return cast_spell(stone_skin_spell); }

void stone_to_mud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "化石为泥");
        break;
    case SPELL_DESC:
        var_set_string(res, "将一格岩石转化为泥土（破坏墙壁）。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        wall_to_mud(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_stone_to_mud(void) { return cast_spell(stone_to_mud_spell); }

void stop_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "世界");
        break;
    case SPELL_DESC:
        var_set_string(res, "消耗你所有的法力值来停止时间。你会获得若干次免费行动机会，次数取决于所消耗的法力值。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("%d 次行动。", MIN((p_ptr->csp + 100-p_ptr->energy_need - 50)/100, 5)));
        break;
    case SPELL_CAST:
    {
        var_set_bool(res, FALSE);
        if (world_player)
        {
            msg_print("时间已经停止了。");
            return;
        }

        world_player = TRUE;
        msg_print("你大喊着“时间停止！”");
        msg_print(NULL);

        /* Note: We pay the casting cost up front these days. So, add back the 150
           to figure the starting sp, and then bash sp down to 0. We can't use the 
           SPELL_COST_EXTRA mechanism here ... */
        p_ptr->energy_need -= 1000 + (100 + (p_ptr->csp + 150) - 50)*TURNS_PER_TICK/10;
        p_ptr->energy_need = MAX(-1550, p_ptr->energy_need);

        p_ptr->csp = 0;
        p_ptr->csp_frac = 0;

        p_ptr->redraw |= (PR_MAP | PR_STATUS);
        p_ptr->update |= (PU_MONSTERS);
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
        handle_stuff();

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_amberites_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤安珀人");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤安珀人来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        msg_print("你召唤了一位安珀领主！");
        if (!summon_specific(-1, py, px, l, SUMMON_AMBERITE, PM_FORCE_PET | PM_ALLOW_UNIQUE))
            msg_print("没有安珀人响应召唤。");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_angel_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤天使");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一位天使来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_ANGEL, PM_FORCE_PET);
        if (!ct)
            msg_print("没有天使响应召唤。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_ants_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤蚂蚁");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤蚂蚁群来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_ANT, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("没有蚂蚁响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_cyberdemon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤赛博恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一只赛博恶魔来协助你。");
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_CYBER, PM_FORCE_PET);
        if (!ct)
            msg_print("没有赛博恶魔响应召唤。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一只恶魔来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(spell_power(p_ptr->lev*2/3), spell_power(p_ptr->lev/2)));
        break;
    case SPELL_CAST:
    {
        bool pet = !one_in_(3);
        u32b mode = 0L;

        if (pet) mode |= PM_FORCE_PET;
        else mode |= PM_NO_PET;
        if (!(pet && (p_ptr->lev < 50))) mode |= PM_ALLOW_GROUP;

        if (summon_specific(SUMMON_WHO_PLAYER, py, px, spell_power(p_ptr->lev*2/3+randint1(p_ptr->lev/2)), SUMMON_DEMON, mode))
        {
            msg_print("这个区域充满了硫磺的恶臭。");
            if (pet)
                msg_print("“你的吩咐是什么……主人？”");
            else
                msg_print("“绝不屈服！可怜虫！我要生吞你的凡人灵魂！”");
        }
        else
            msg_print("没有恶魔响应召唤。");
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_demon_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一只恶魔来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        ct += summon_specific(-1, py, px, l, SUMMON_DEMON, PM_FORCE_PET);
        if (ct)
            msg_print("这个区域充满了硫磺的恶臭。");
        else
            msg_print("没有恶魔响应召唤。");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤巨龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一条巨龙来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
    {
        int ct = 0;
        int l = p_ptr->lev + randint1(p_ptr->lev * 2 / 3);

        ct += summon_specific(-1, py, px, l, SUMMON_DRAGON, PM_FORCE_PET);
        if (!ct)
            msg_print("没有巨龙响应召唤。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_greater_demon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤高等恶魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤高等恶魔。你需要献祭一具人类尸体（符号为'p'、'h'或't'），尸体越强大，你召唤出的恶魔就越强大。");
        break;
    case SPELL_INFO:
        var_set_string(res, format("等级 %d+献祭物", p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
        var_set_bool(res, cast_summon_greater_demon());
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hi_dragon_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤远古巨龙");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤一只或多只远古巨龙来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev * 2 / 3));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;

        if (p_ptr->dragon_realm == DRAGON_REALM_DOMINATION)
            num = 2 + randint1(3);

        for (i = 0; i < num; i++)
        {
            int l = p_ptr->lev + randint1(p_ptr->lev * 2 / 3);
            ct += summon_specific(-1, py, px, l, SUMMON_HI_DRAGON, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("没有巨龙响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hi_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤高级死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤高级死灵来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HI_UNDEAD, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("没有死灵响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hounds_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤猎犬");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤猎犬来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HOUND, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("没有猎犬响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_hydras_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤多头蛇");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤多头蛇来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_HYDRA, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("没有多头蛇响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_kin_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤同族");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤与你相关的怪物来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, 0));
        break;
    case SPELL_CAST:
        if (!summon_kin_player(p_ptr->lev, py, px, PM_FORCE_PET | PM_ALLOW_GROUP))
            msg_print("没有任何援助到来。");
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_manes_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤劣魔");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤一些恶魔朋友。");
        break;
    case SPELL_CAST:
        if (!summon_specific(-1, py, px, (p_ptr->lev * 3) / 2, SUMMON_MANES, (PM_ALLOW_GROUP | PM_FORCE_PET)))
            msg_print("没有劣魔响应召唤。");

        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_monster_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤一只怪物来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        if (!summon_specific(-1, py, px, l, 0, PM_FORCE_PET | PM_ALLOW_GROUP))
            msg_print("没有怪物响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_monsters_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤怪物群");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤怪物群来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, 0, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("没有怪物响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_spiders_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤蜘蛛");
        break;
    case SPELL_DESC:
        var_set_string(res, "召唤蜘蛛来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_SPIDER, PM_FORCE_PET | PM_ALLOW_GROUP);
        }
        if (!ct)
            msg_print("没有蜘蛛响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_tree_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= 45)
            var_set_string(res, "召唤树人");
        else
            var_set_string(res, "召唤树人");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "召唤树人");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= 45)
            var_set_string(res, "试图召唤许多树人。");
        else
            var_set_string(res, "试图召唤一只树人。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "试图召唤一只树人。在 45 级时，试图在玩家周围召唤一圈树人。");
        break;
    case SPELL_CAST:
        if (p_ptr->lev >= 45)
        {
            tree_creation();
            var_set_bool(res, TRUE);
        }    
        else
        {
            int attempts = 0;
            int x, y, dir;

            var_set_bool(res, TRUE);
            for (;;)
            {
                if (attempts > 4)
                {
                    msg_print("没有树人响应召唤。");
                    break;
                }

                dir = randint0(9);
                if (dir == 5) continue;

                attempts++;
                y = py + ddy[dir];
                x = px + ddx[dir];

                if (!in_bounds(y, x)) continue;
                if (!cave_naked_bold(y, x)) continue;
                if (player_bold(y, x)) continue;

                cave_set_feat(y, x, feat_tree);
                break;
            }
        }
        break;
    case SPELL_COST_EXTRA:
    {
        int n = 0;
        if (p_ptr->lev >= 45)
            n += 30;

        var_set_int(res, n);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_summon_tree(void) { return cast_spell(summon_tree_spell); }

void summon_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤死灵来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int num = randint1(p_ptr->lev/10);
        int ct = 0, i;
        int l = p_ptr->lev + randint1(p_ptr->lev);

        for (i = 0; i < num; i++)
        {
            ct += summon_specific(-1, py, px, l, SUMMON_UNDEAD, PM_FORCE_PET);
        }
        if (!ct)
            msg_print("没有死灵响应召唤。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void summon_uniques_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "召唤独特怪物");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图召唤独特怪物来协助你。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_level(p_ptr->lev, p_ptr->lev));
        break;
    case SPELL_CAST:
    {
        int l = p_ptr->lev + randint1(p_ptr->lev);

        msg_print("你召唤了一个特殊的对手！");
        if (!summon_specific(-1, py, px, l, SUMMON_UNIQUE, PM_FORCE_PET | PM_ALLOW_UNIQUE))
            msg_print("没有人响应召唤。");

        var_set_bool(res, TRUE);
        break;
    } 
    default:
        default_spell(cmd, res);
        break;
    }
}

void super_stealth_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "藏身黑暗");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予忍者的潜行能力！你可以在阴影中隐藏自己，并能在黑暗中视物。你的照明半径减少 3 点。");
        break;
    case SPELL_CAST:
        if (p_ptr->tim_superstealth)
        {
            msg_print("你已经潜行在阴影之中了。");
            var_set_bool(res, FALSE);
        }
        else
        {
            set_tim_superstealth(spell_power(randint1(p_ptr->lev/2) + p_ptr->lev/2), FALSE);
            var_set_bool(res, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void swap_pos_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "交换位置");
        break;
    case SPELL_DESC:
        var_set_string(res, "与指定的怪物交换位置。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你觉得很想体验一下别人的位置。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你觉得还是待在自己的位置比较好。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以与另一个生物交换位置。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);

        project_length = -1;
        if (get_fire_dir(&dir))
        {
            teleport_swap(dir);
            var_set_bool(res, TRUE);
        }
        project_length = 0;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_swap_pos(void) { return cast_spell(swap_pos_spell); }

void sword_dance_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "剑舞");
        break;
    case SPELL_DESC:
        var_set_string(res, "随机攻击相邻的怪物。");
        break;
    case SPELL_CAST:
    {
        int y = 0, x = 0, i, dir = 0;
        cave_type *c_ptr;

        for (i = 0; i < 6; i++)
        {
            dir = randint0(8);
            y = py + ddy_ddd[dir];
            x = px + ddx_ddd[dir];
            c_ptr = &cave[y][x];

            if (c_ptr->m_idx)
                py_attack(y, x, 0);
            else
                msg_print("你攻击了空气。");
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void telekinesis_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "心灵遥控");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图隔空取回远处的物品。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了用心灵遥控移动物品的能力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了用心灵遥控移动物品的能力。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你拥有了心灵遥控能力。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_aim_dir(&dir))
        {
            fetch(dir, p_ptr->lev * 10, TRUE);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_telekinesis(void) { return cast_spell(telekinesis_spell); }

void telepathy_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "心灵感应");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内赋予心灵感应。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 30));
        break;
    case SPELL_CAST:
        set_tim_esp(randint1(25) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void teleport_other_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "传送他者");
        break;
    case SPELL_DESC:
        var_set_string(res, "将路径上的所有怪物传送走，除非被抵抗。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_dist(spell_power(p_ptr->lev*2)));
        break;
    case SPELL_CAST:
    {
        int dir;
        int power = spell_power(p_ptr->lev*2);

        var_set_bool(res, FALSE);

        if (!get_fire_dir(&dir)) return;
        fire_beam(GF_AWAY_ALL, dir, power);
            
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void teleport_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "传送术");
        break;
    case SPELL_DESC:
        var_set_string(res, "逃往远处的一个位置。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了随心所欲传送的力量。");
        mut_lose(MUT_TELEPORT_RND);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了随心所欲传送的力量。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以随心所欲地传送。");
        break;
    case SPELL_CAST:
        teleport_player(10 + 4 * p_ptr->lev, 0);
        var_set_bool(res, TRUE);
        break;
    case SPELL_ENERGY:
        if (mut_present(MUT_ASTRAL_GUIDE))
        {
            var_set_int(res, 30);
            break;
        }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_teleport(void) { return cast_spell(teleport_spell); }

void teleport_level_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "传送楼层");
        break;
    case SPELL_DESC:
        var_set_string(res, "逃往另一个楼层。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if (!py_teleport_level("Are you sure? (Teleport Level) ")) return;
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_teleport_level(void) { return cast_spell(teleport_level_spell); }

void teleport_to_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "拉近传送");
        break;
    case SPELL_DESC:
        var_set_string(res, "将一个可见的怪物传送到你身边。");
        break;
    case SPELL_CAST:
    {
        monster_type *m_ptr;
        char m_name[80];

        if (!target_set(TARGET_KILL)) break;
        if (!cave[target_row][target_col].m_idx) break;
        if (!player_has_los_bold(target_row, target_col)) break;
        if (!projectable(py, px, target_row, target_col)) break;
        if (cave[target_row][target_col].m_idx == p_ptr->riding) break;

        var_set_bool(res, TRUE);

        m_ptr = &m_list[cave[target_row][target_col].m_idx];
        monster_desc(m_name, m_ptr, 0);
        if (mon_save_tele_to(m_ptr, m_name, TRUE)) break;
        msg_format("你命令 %s 返回。", m_name);
        teleport_monster_to(cave[target_row][target_col].m_idx, py, px, 100, TELEPORT_PASSIVE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

static int _boulder_dam(void)
{
    return py_prorata_level_aux(250, 2, 1, 2);
}
void throw_boulder_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "投掷巨石");
        break;
    case SPELL_DESC:
        var_set_string(res, "向选定目标投掷一块巨大的石头。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, _boulder_dam()));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你投出了一块巨大的石头。");
        fire_bolt(GF_ROCK, dir, _boulder_dam());
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        var_set_int(res, (_boulder_dam() + 6)/7);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void touch_of_confusion_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "困惑之触");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图让你击中的下一个怪物陷入困惑。");
        break;
    case SPELL_CAST:
        if (!(p_ptr->special_attack & ATTACK_CONFUSE))
        {
            msg_print("你的双手开始发光。");
            p_ptr->special_attack |= ATTACK_CONFUSE;
            p_ptr->redraw |= (PR_STATUS);
        }
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void turn_undead_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "驱散死灵");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图恐吓视线内的死灵怪物。");
        break;
    case SPELL_CAST:
        if (project_hack(GF_TURN_UNDEAD, spell_power(p_ptr->lev)))
            virtue_add(VIRTUE_UNLIFE, -1);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vampirism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸血");
        break;
    case SPELL_DESC:
        var_set_string(res, "吸取相邻怪物的血液，在此过程中恢复生命值。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得像吸血鬼一样。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再像吸血鬼了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以像吸血鬼一样从敌人身上吸取生命。");
        break;
    case SPELL_CAST:
        var_set_bool(res, FALSE);
        if ((d_info[dungeon_type].flags1 & DF1_NO_MELEE) || (no_melee_challenge))
        {
            msg_print("某种东西阻止了你进行攻击。");
            return;
        }
        else
        {
            int x, y, dummy;
            cave_type *c_ptr;
            int dir = 0;

            /* Only works on adjacent monsters */
            if (!get_rep_dir2(&dir)) break;

            var_set_bool(res, TRUE);

            y = py + ddy[dir];
            x = px + ddx[dir];
            c_ptr = &cave[y][x];

            stop_mouth();

            if (!(c_ptr->m_idx))
            {
                msg_print("你咬到了空气！");
                break;
            }

            msg_print("你咧开嘴，露出了尖牙……");
            dummy = spell_power(p_ptr->lev * 2);

            if (drain_life(dir, dummy))
            {
                /* No heal if we are "full" */
                if (p_ptr->food < PY_FOOD_FULL)
                    vamp_player(dummy);
                else
                    msg_print("你不饿。");

                /* Gain nutritional sustenance: 150/hp drained
                 * A Food ration gives 5000 food points (by contrast)
                 * Don't ever get more than "Full" this way
                 * But if we ARE Gorged,  it won't cure us 
                 */
                dummy = p_ptr->food + MIN(5000, 100 * dummy);
                if (p_ptr->food < PY_FOOD_MAX)   /* Not gorged already */
                    set_food(dummy >= PY_FOOD_MAX ? PY_FOOD_MAX-1 : dummy);
            }
            else
                msg_print("呸。味道真恶心。");
        }
        break;
    case SPELL_COST_EXTRA:
        var_set_int(res, p_ptr->lev / 3);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_vampirism(void) { return cast_spell(vampirism_spell); }

void water_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水球术");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个水球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev*4 + 50 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你做出了一个流畅的施法手势。");
        fire_ball(GF_WATER, dir, spell_power(50 + p_ptr->lev*4 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void water_bolt_spell(int cmd, variant *res)
{
    int dd = 7 + p_ptr->lev / 4;
    int ds = 15;

    if (elemental_is_(ELEMENTAL_WATER)) dd = (dd - 3 + water_flow_rate() / 10);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "水流箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发水流箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        int dam = spell_power(damroll(dd, ds) + p_ptr->to_d_spell);
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(GF_WATER, dir, dam);
        water_mana_action(2, dam / 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void weigh_magic_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "衡量魔法");
        break;
    case SPELL_DESC:
        var_set_string(res, "确定影响你的魔法的强度。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你觉得你可以更好地理解周围的魔法了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能感知魔法了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你能感觉到影响你的魔法的强度。");
        break;
    case SPELL_CAST:
        report_magics();
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_weigh_magic(void) { return cast_spell(weigh_magic_spell); }

void wonder_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "万象无常");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射某种具有随机效果的东西。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        cast_wonder(dir);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void wraithform_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幽灵形态");
        break;
    case SPELL_DESC:
        var_set_string(res, "离开活人的世界，在冥界的阴影中穿行。你获得穿墙能力和极高的伤害抗性。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(spell_power(p_ptr->lev/2), spell_power(p_ptr->lev/2)));
        break;
    case SPELL_CAST:
    {
        int base = spell_power(p_ptr->lev / 2);
        set_wraith_form(randint1(base) + base, FALSE);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
