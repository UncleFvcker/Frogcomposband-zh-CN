/*
 * Mutations that cannot be activated as normal spells.
 * These mutations might be timed effects, or just things
 * like "Horns" that you simply have. They might be augmentations
 * like "Super Human He-man".
 *
 * We are still implementing all mutations as spells for
 * uniformity.
 *
 * Again, spells are (stateless) objects, implemented as functions.
 */

#include "angband.h"

/*
void foo_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "");
        break;
    case SPELL_GAIN_MUT:
        msg_print("");
        break;
    case SPELL_LOSE_MUT:
        msg_print("");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "");
        break;
    case SPELL_CALC_BONUS:
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
*/


void albino_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "白化病");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变成了白化病患者！你感到很虚弱...");
        mut_lose(MUT_RESILIENT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再患有白化病了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你患有白化病 (-4 体质)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void alcohol_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "酒精");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的身体开始分泌酒精！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的身体停止分泌酒精！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的身体会分泌酒精。");
        break;
    case SPELL_PROCESS:
        if (randint1(6400) == 321)
        {
            if (!res_save_default(RES_CONF) && !res_save_default(RES_CHAOS))
            {
                disturb(0, 0);
                p_ptr->redraw |= PR_EXTRA;
                msg_print("你gan觉YI阵hun沉xi来... *嗝*！");
            }

            if (!res_save_default(RES_CONF))
                set_confused(p_ptr->confused + randint0(20) + 15, FALSE);

            if (!res_save_default(RES_CHAOS))
            {
                if (one_in_(20))
                {
                    msg_print(NULL);
                    if (one_in_(3)) lose_all_info();
                    else wiz_dark();
                    teleport_player_aux(100, TELEPORT_NONMAGICAL | TELEPORT_PASSIVE);
                    wiz_dark();
                    msg_print("你在某个地方醒来，头痛欲裂...");
                    msg_print("你什么都记不起来了，也不知道自己是怎么到这儿的！");
                }
                else if (one_in_(3))
                {
                    msg_print("zhE sHii Hao DoNg xi!");
                    set_image(p_ptr->image + randint0(15) + 15, FALSE);
                }
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void ambidexterity_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "左右开弓");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你想要双持武器了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再想要双持武器了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你左右开弓。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供相当于15个百分点的双持技能加成。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void arcane_mastery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "奥术精通");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了奥术洞察力。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉你的奥术精通正在流失。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你精通奥术。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "降低 3% 的法术失败率（最大和最小失败率仍然适用）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void arthritis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "关节炎");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的关节突然开始疼痛。");
        mut_lose(MUT_LIMBER);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的关节不疼了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的关节持续酸痛 (-3 敏捷)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void astral_guide_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "星界向导");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你能够快速传送！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能够快速传送！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是一名星界向导（传送消耗更少的能量）。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "将玩家触发的传送所需时间减少 67% 到 70%。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_animal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸引动物");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始吸引动物。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止吸引动物。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会吸引动物。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(7000))
        {
            bool pet = one_in_(3);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px, dun_level, SUMMON_ANIMAL, mode))
            {
                msg_print("你吸引了一只动物！");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_demon_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸引恶魔");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始吸引恶魔。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止吸引恶魔。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会吸引恶魔。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && (randint1(6666) == 666))
        {
            bool pet = one_in_(6);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px,
                        dun_level, SUMMON_DEMON, mode))
            {
                msg_print("你吸引了一只恶魔！");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void attract_dragon_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吸引龙类");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始吸引龙类。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止吸引龙类。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会吸引龙类。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            bool pet = one_in_(5);
            u32b mode = PM_ALLOW_GROUP;

            if (pet) mode |= PM_FORCE_PET;
            else mode |= (PM_ALLOW_UNIQUE | PM_NO_PET);

            if (summon_specific((pet ? -1 : 0), py, px, dun_level, SUMMON_DRAGON, mode))
            {
                msg_print("你吸引了一只龙！");
                disturb(0, 0);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void black_marketeer_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑市商人");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你成为了黑市的代理人！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再是黑市的代理人了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是黑市的代理人。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "让你在黑市中享受优惠价格（所有物品半价）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void blank_face_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "面无表情");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的脸变得完全没有五官！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的五官恢复了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你没有五官 (-1 魅力)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void bad_luck_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "黑色光环");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉到与“魔棒大师丹尼斯”有一种奇怪的亲切感！");
        mut_lose(MUT_GOOD_LUCK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的黑色光环旋转着消散了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的周围环绕着黑色的光环。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void beak_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鸟喙");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的嘴变成了一个尖锐、有力的喙！");
        mut_lose(MUT_TRUNK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的嘴恢复了正常！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着鸟喙。");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 4;
        a.weight = 30;
        a.blows = 100;
        a.msg = "你啄了过去。";
        a.name = "鸟喙";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void berserk_rage_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂暴");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得容易陷入狂暴状态！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再容易陷入狂暴状态！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你容易陷入狂暴状态。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->shero && one_in_(3000))
        {
            disturb(0, 0);
            cast_berserk();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cerebral_pultitis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "大脑化脓");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的大脑感觉就像一锅粥...");
        mut_lose(MUT_HYPER_INT);
        mut_lose(MUT_MORONIC);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的大脑不再像一锅粥了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你患有大脑化脓 (-3 智力)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void chaos_deity_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "混沌神明");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你引起了混沌神明的注意！");
        /* In case it isn't obvious, every character has a chaos deity assigned at birth. */
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了混沌神明的关注。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "混沌神明会给予你礼物。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cowardice_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "懦弱");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得极其懦弱！");
        mut_lose(MUT_FEARLESS);
        mut_lose(MUT_NO_INHIBITIONS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再是个极端的懦夫！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你容易退缩。");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_FEAR) && (randint1(3000) == 13))
        {
            disturb(0, 0);
            msg_print("好黑...好可怕！");
            fear_add_p(FEAR_SCARED);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void cult_of_personality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "个人崇拜");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了对敌人召唤物的控制权！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了对敌人召唤物的控制权！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "被召唤出来的怪物有时是友好的。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "被召唤出的敌对怪物有最高50%的几率变为友好甚至成为宠物，这取决于它们的等级和玩家的魅力。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->cult_of_personality = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void demonic_grasp_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恶魔之握");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你紧紧抓住了你的魔法装置。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再能紧紧抓住你的魔法装置。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你抵抗充能流失。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "免疫充能流失。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_breath_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "致命吐息");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的吐息变得更加强劲。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你的吐息将变得更致命。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_kin_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_MUT_DESC:
        var_set_string(res, "你能够召唤同类。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你将能够从龙族那里召唤援助。");
        break;
    default:
        summon_kin_spell(cmd, res);
        break;
    }
}

void draconian_lore_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "远古知识");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "物品在你捡起时会自动鉴定。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "物品在你捡起时会自动鉴定。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->auto_id = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_magic_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法抗性");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你对魔法具有抗性。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你将获得对抗魔法攻击的额外豁免检定。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.sav += 15 + p_ptr->lev / 5;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_metamorphosis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "变形");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你变形成了一条龙！");
        break;
    case SPELL_HELP_DESC:
        if (p_ptr->pclass == CLASS_MONK || p_ptr->pclass == CLASS_FORCETRAINER)
            var_set_string(res, "你的身体将变形成一条龙（例如：6个戒指槽，无法装备武器）。警告：你将无法使用武术！");
        else
            var_set_string(res, "你的身体将变形成一条龙（例如：6个戒指槽，无法装备武器）");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_regen_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "再生");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的再生速度极快。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你的再生速度将会快得多。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen += 150;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "抗性增加");
        break;
    case SPELL_MUT_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            var_set_string(res, "你获得额外的火抗性。");
            break;
        case DRACONIAN_WHITE:
            var_set_string(res, "你获得额外的寒冷抗性。");
            break;
        case DRACONIAN_BLUE:
            var_set_string(res, "你获得额外的电抗性。");
            break;
        case DRACONIAN_BLACK:
            var_set_string(res, "你获得额外的酸抗性。");
            break;
        case DRACONIAN_GREEN:
            var_set_string(res, "你获得额外的毒抗性。");
            break;
        case DRACONIAN_BRONZE:
            var_set_string(res, "你获得额外的混乱抗性。");
            break;
        case DRACONIAN_GOLD:
            var_set_string(res, "你获得额外的声音抗性。");
            break;
        case DRACONIAN_CRYSTAL:
            var_set_string(res, "你获得额外的碎片抗性。");
            break;
        case DRACONIAN_SHADOW:
            var_set_string(res, "你获得额外的地狱抗性。");
            break;
        }
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你将获得相应类型的额外抗性（如火或寒冷）。");
        break;
    case SPELL_CALC_BONUS:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            res_add(RES_FIRE);
            break;
        case DRACONIAN_WHITE:
            res_add(RES_COLD);
            break;
        case DRACONIAN_BLUE:
            res_add(RES_ELEC);
            break;
        case DRACONIAN_BLACK:
            res_add(RES_ACID);
            break;
        case DRACONIAN_GREEN:
            res_add(RES_POIS);
            break;
        case DRACONIAN_BRONZE:
            res_add(RES_CONF);
            break;
        case DRACONIAN_GOLD:
            res_add(RES_SOUND);
            break;
        case DRACONIAN_CRYSTAL:
            res_add(RES_SHARDS);
            break;
        case DRACONIAN_SHADOW:
            res_add(RES_NETHER);
            break;
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_shield_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "龙皮");
        break;
    case SPELL_MUT_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            var_set_string(res, "龙皮：你获得 +15 AC 和火光环");
            break;
        case DRACONIAN_WHITE:
            var_set_string(res, "龙皮：你获得 +15 AC 和寒冷光环");
            break;
        case DRACONIAN_BLUE:
            var_set_string(res, "龙皮：你获得 +15 AC 和电光环");
            break;
        case DRACONIAN_CRYSTAL:
            var_set_string(res, "龙皮：你获得 +10 AC 和碎片光环");
            break;
        default:
            var_set_string(res, "龙皮：你获得 +25 AC");
        }
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你将获得额外的护甲等级，如果有条件，还会获得一个防御性光环。");
        break;
    case SPELL_CALC_BONUS:
    {
        int amt = 25;
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
            p_ptr->sh_fire++;
            amt = 15;
            break;
        case DRACONIAN_WHITE:
            p_ptr->sh_cold++;
            amt = 15;
            break;
        case DRACONIAN_BLUE:
            p_ptr->sh_elec++;
            amt = 15;
            break;
        case DRACONIAN_CRYSTAL:
            p_ptr->sh_shards++;
            amt = 10;
            break;
        }
        p_ptr->to_a += amt;
        p_ptr->dis_to_a += amt;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void draconian_strike_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "龙之打击");
        break;
    case SPELL_DESC:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED: var_set_string(res, "用带火焰效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_WHITE: var_set_string(res, "用带冰霜效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_BLUE: var_set_string(res, "用带电击效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_BLACK: var_set_string(res, "用带腐蚀效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_GREEN: var_set_string(res, "用带毒素效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_GOLD: var_set_string(res, "用带震慑效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_BRONZE: var_set_string(res, "用带混乱效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_CRYSTAL: var_set_string(res, "用带撕裂效果的攻击打击相邻的对手。"); break;
        case DRACONIAN_SHADOW: var_set_string(res, "用带吸血效果的攻击打击相邻的对手。"); break;
        }
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你拥有龙之打击的力量。");
        break;
    case SPELL_HELP_DESC:
        if (spoiler_hack)
            var_set_string(res, "你将能够用特殊攻击（火、寒冷等）打击相邻的对手。");
        else
        {
            switch (p_ptr->psubrace)
            {
            case DRACONIAN_RED: var_set_string(res, "你将能够用带火焰效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_WHITE: var_set_string(res, "你将能够用带冰霜效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_BLUE: var_set_string(res, "你将能够用带电击效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_BLACK: var_set_string(res, "你将能够用带腐蚀效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_GREEN: var_set_string(res, "你将能够用带毒素效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_GOLD: var_set_string(res, "你将能够用带震慑效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_BRONZE: var_set_string(res, "你将能够用带混乱效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_CRYSTAL: var_set_string(res, "你将能够用带撕裂效果的攻击打击相邻的对手。"); break;
            case DRACONIAN_SHADOW: var_set_string(res, "你将能够用带吸血效果的攻击打击相邻的对手。"); break;
            }
        }
        break;
    case SPELL_CAST:
    {
        int mode = 0;
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED: mode = DRACONIAN_STRIKE_FIRE; break;
        case DRACONIAN_WHITE: mode = DRACONIAN_STRIKE_COLD; break;
        case DRACONIAN_BLUE: mode = DRACONIAN_STRIKE_ELEC; break;
        case DRACONIAN_BLACK: mode = DRACONIAN_STRIKE_ACID; break;
        case DRACONIAN_GREEN: mode = DRACONIAN_STRIKE_POIS; break;
        case DRACONIAN_GOLD: mode = DRACONIAN_STRIKE_STUN; break;
        case DRACONIAN_BRONZE: mode = DRACONIAN_STRIKE_CONF; break;
        case DRACONIAN_CRYSTAL: mode = PY_ATTACK_VORPAL; break;
        case DRACONIAN_SHADOW: mode = PY_ATTACK_VAMP; break;
        }
        var_set_bool(res, do_blow(mode));
        break;
    }
    case SPELL_COST_EXTRA:
        switch (p_ptr->psubrace)
        {
        case DRACONIAN_RED:
        case DRACONIAN_WHITE:
        case DRACONIAN_BLUE:
        case DRACONIAN_BLACK:
        case DRACONIAN_GREEN:
            var_set_int(res, 15);
            break;
        case DRACONIAN_GOLD:
        case DRACONIAN_BRONZE:
            var_set_int(res, 20);
            break;
        case DRACONIAN_CRYSTAL:
            var_set_int(res, 12);
            break;
        case DRACONIAN_SHADOW:
            var_set_int(res, 7);
            break;
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void easy_tiring_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "易疲劳");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然感觉身体状态很差。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉身体吃不消了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "物理战斗会使你疲惫。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void easy_tiring_II_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "极易疲劳");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然感觉异常疲惫。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉异常疲惫。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "使用魔法或远程武器会使你疲惫。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void eat_light_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吞噬光芒");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉到与乌苟立安特(Ungoliant)有一种奇怪的亲切感。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉世界变得更明亮了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有时候会吸收周围的光线作为养分。");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
        {
            int slot = equip_find_obj(TV_LITE, SV_ANY);

            msg_print("一片阴影掠过你。");
            msg_print(NULL);

            if ((cave[py][px].info & (CAVE_GLOW | CAVE_MNDK)) == CAVE_GLOW)
            {
                hp_player(10);
            }

            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if (!object_is_fixed_artifact(o_ptr) && (o_ptr->xtra4 > 0))
                {
                    hp_player(o_ptr->xtra4 / 20);
                    o_ptr->xtra4 /= 2;

                    msg_print("你从你的光源中吸收了能量！");
                    notice_lite_change(o_ptr);
                }
            }
            unlite_area(50, 10);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void einstein_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "活体计算机");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的大脑进化成了活体计算机！");
        mut_lose(MUT_MORONIC);
        mut_lose(MUT_PULTITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的大脑恢复了正常。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的大脑是一台活体计算机 (+4 智力/感知)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void elec_aura_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "电击光环");
        break;
    case SPELL_GAIN_MUT:
        msg_print("电流开始流遍你的全身！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("电流停止在你的全身流动。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "电流在你的静脉中流淌。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->sh_elec++;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void evasion_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪避");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了闪避的力量。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了闪避的力量。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你能避免被地震压碎，并能躲避怪物的喷吐攻击。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "将所有敌人喷吐攻击、火箭和岩石的伤害降低 11% 到 20%，并有 50% 的几率避开地震。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_eyes_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "额外的眼睛");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你长出了一对额外的眼睛！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你额外的眼睛消失了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有一对额外的眼睛 (+15 搜索)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.fos += 15;
        p_ptr->skills.srh += 15;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_legs_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "额外的腿");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你长出了一对额外的腿！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你额外的腿消失了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有一对额外的腿 (+3 速度)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void extra_noise_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "额外噪音");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始发出奇怪的噪音！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止发出奇怪的噪音！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你发出很多奇怪的噪音 (-3 潜行)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.stl -= 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fantastic_frenzy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "奇妙狂乱");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉到一种奇妙的狂乱...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉奇妙狂乱。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你拥有奇妙狂乱的力量。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "保留在不到一回合内击杀怪物时剩余的能量，同时赋予你施展“旋风攻击(Whirlwind Attack)”的能力，允许你消耗 50 SP 或 HP 攻击所有相邻的怪物。");
        break;
    default:
        massacre_spell(cmd, res);
        break;
    }
}

void fast_learner_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "快速学习者");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你学东西变快了...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你恢复了正常迟钝的自我！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是个快速学习者。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你每击杀一只怪物将获得多 20% 的经验值。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fat_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "额外脂肪");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得异常肥胖，令人作呕！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你从奇迹般的节食中受益了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你极其肥胖 (+2 体质, -2 速度)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed -= 2;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fearless_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "无所畏惧");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得完全无所畏惧。");
        mut_lose(MUT_COWARDICE);
        mut_lose(MUT_NO_INHIBITIONS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你又开始感到恐惧了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你无所畏惧。");
        break;
    case SPELL_CALC_BONUS:
        res_add(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fell_sorcery_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恐怖巫术");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的法术变得更强大。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉你的魔法变得更强大了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉你的魔法恢复了正常。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你的法术将变得更强大 (+15% 法术强度)，代价是力量、敏捷和体质会受到轻微的惩罚。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->spell_power += 2;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fire_aura_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "火焰光环");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的身体被火焰包围了！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的身体不再被火焰包围。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的身体被火焰包围。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->sh_fire++;
        p_ptr->lite = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void flatulence_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "肠胃胀气");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始不受控制地肠胃胀气。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再不受控制地肠胃胀气了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会不受控制地肠胃胀气。");
        break;
    case SPELL_PROCESS:
        if (randint1(3000) == 13)
        {
            disturb(0, 0);
            /* Seriously, this the best mutation!  Ever!! :D */
            msg_print("噗——！糟糕。");
            msg_print(NULL);
            fire_ball(GF_POIS, 0, p_ptr->lev, 3);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fleet_of_foot_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "健步如飞");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉健步如飞！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉又回到了以前步履沉重的状态！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你健步如飞（移动消耗更少的能量）。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "行走消耗的能量减少 40%。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void fumbling_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "笨手笨脚");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的脚长到了以前的四倍大。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的脚缩小到了原来的大小。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你偶尔会绊倒并弄丢东西。");
        break;
    case SPELL_PROCESS:
        if (one_in_(10000))
        {
            int slot = equip_random_slot(object_is_melee_weapon);

            disturb(0, 0);
            msg_print("你被自己的脚绊倒了！");
            take_hit(DAMAGE_NOESCAPE, randint1(150 / 6), "绊倒");
            msg_print(NULL);

            if (slot)
            {
                object_type *o_ptr = equip_obj(slot);
                if (equip_can_takeoff(o_ptr))
                {
                    cmsg_print(TERM_VIOLET, "你掉落了武器！");
                    equip_drop(o_ptr);
                    msg_print("按“Y”键继续。");
                    flush();
                    for (;;)
                    {
                        char ch = inkey();
                        if (ch == 'Y') break;
                    }
                    msg_line_clear();
                }
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void good_luck_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "白色光环");
        break;
    case SPELL_GAIN_MUT:
        msg_print("有一股仁慈的白色光环包围着你...");
        mut_lose(MUT_BAD_LUCK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的白色光环闪烁着消散了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的周围环绕着白色的光环。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hallucination_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "幻觉");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你患上了产生幻觉的疯狂！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再患有产生幻觉的疯狂！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你患有产生幻觉的疯狂。");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_CHAOS) && randint1(6400) == 42 && !res_save_default(RES_CHAOS))
        {
            disturb(0, 0);
            p_ptr->redraw |= PR_EXTRA;
            set_image(p_ptr->image + randint0(50) + 20, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void he_man_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "肌肉猛男");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变成了超人般的肌肉猛男！");
        mut_lose(MUT_PUNY);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的肌肉恢复了正常。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你拥有超人般的力量 (+4 力量)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void horns_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "长角");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的额头上长出了角！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你额头上的角消失了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着角。");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 6;
        a.weight = 150;
        a.blows = 100;
        a.msg = "你刺穿了目标。";
        a.name = "长角";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_chr_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "过度自信");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得过度自信！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再过度自信。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "过度自信使你变得马虎。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你也许极具魅力；但过度沉醉于自己的炒作中会导致过度自信和马虎。由于缺乏足够的专注，你的近战和射击精度、以及使用装置和法术的失败率会受到惩罚，类似于“懒惰”性格带来的效果。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.dev -= 10;
        p_ptr->skills.thn -= 16;
        p_ptr->skills.thb -= 10;
        p_ptr->to_m_chance += 10;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_con_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "逼近极限");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的身体被逼到了极限。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的身体不再被逼到极限了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会频繁地感到身体不适。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你的体质和耐力非同凡响；但就像许多高强度运动员一样，你的免疫系统难以跟上。你会频繁地感到身体不适，这会暂时降低你的体质和敏捷。");
        break;
    case SPELL_PROCESS:
        if ((!p_ptr->unwell) && (one_in_(200)))
        {
            disturb(0, 0);
            set_unwell(50, TRUE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_dex_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "敏捷超乎肉体");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你对于这具身体来说太敏捷了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你对于你的身体不再过于敏捷。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你偶尔会在战斗中拉伤肌肉。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "敏捷是你最强的优势；但这具人类躯体能承受的极限是有限的。你在打击怪物、躲避敌人攻击或发射弹药时有时会拉伤肌肉；这会暂时使你的速度降低 10。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_int_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "想象力过于丰富");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的想象力变得过于丰富。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的想象力不再过于丰富了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的想象力过于丰富。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "作为人类，你的大脑高度发达——但也许有时好事过了头？你过于丰富的想象力使你容易感到恐惧，在所有与恐惧相关的玩家/怪物交互中，你的有效角色等级会降低 10 级。");
        break;
    case SPELL_CALC_BONUS:
        res_add_vuln(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_str_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "失衡打击");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你不再能控制自己的力量。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你又能控制自己的力量了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你最强力攻击的威力会使你失去平衡。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "力量是你最大的资本；但你现在打击的威力大到难以控制。你在近战中造成的每一次暴击都会使你失去平衡；在那个回合中你无法造成更多的暴击，而且因为你需要额外的时间来恢复平衡，这样的回合将消耗 1.2 回合而不是 1 回合。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void human_wis_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "与神同调");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你不再感觉与邪恶生物有任何联系。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再与邪恶的心智断开连接了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你感觉与邪恶生物没有任何联系。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你极其明智，并花费了大量时间冥想，以至于你的思想现在已完全与神性同调；但结果是，你感觉与邪恶怪物没有任何联系。你无法通过心灵感应探测到任何邪恶怪物，即使使用心灵感应法术或装备也不行。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hypochondria_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "急性疑病症");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你对自己的健康感到非常担忧。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再担心自己的健康了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你患有疑病症。");
        break;
    case SPELL_PROCESS:
        if (one_in_(1815))
        {
            if (one_in_(2))
            {
                disturb(0, 0);
                msg_print("你感到无比的担忧！");
                fear_add_p(FEAR_SCARED);
            }
            else
            {
                disturb(0, 0);
                set_unwell(50, TRUE);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void illusion_normal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "安心幻象");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始投射出令人安心的幻象。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止投射令人安心的幻象。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的外表被幻象所掩盖。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void impotence_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法无能");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的魔法装置突然感觉软绵绵的。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的魔法装置不再感觉软绵绵的了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你魔法无能。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void infernal_deal_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恶魔契约");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你与恶魔签订了契约！");
        mut_lose(MUT_ARTHRITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的恶魔契约被打破了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你已经签订了恶魔契约。");
        break;
    case SPELL_HELP_DESC:
        if ((p_ptr->pclass == CLASS_RUNE_KNIGHT) || (p_ptr->pclass == CLASS_SAMURAI) || (p_ptr->pclass == CLASS_MYSTIC) || ((p_ptr->lev > 18) && (p_ptr->msp == 0))) var_set_string(res, "每当附近的一只敌对怪物被击杀时，你将恢复HP，恢复量相当于怪物原生等级的三分之二。");
        else var_set_string(res, "每当附近的一只敌对怪物被击杀时，你将恢复HP和SP，HP恢复量相当于怪物原生等级的九分之四，SP恢复量为其一半。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void infravision_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "红外视力");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的红外视力增强了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的红外视力减弱了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你拥有非凡的红外视力 (+3)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->see_infra += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void inspired_smithing_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "灵感锻造");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了启发铁匠的能力！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你无法再启发铁匠了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的存在能启发铁匠。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "在物品重铸时，你将获得更好的结果（平均价值提高 10%）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void invulnerability_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "无敌");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你被赐予了偶尔无敌的祝福。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再被赐予偶尔无敌的祝福。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你偶尔感觉自己是无敌的。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(5000))
        {
            disturb(0, 0);
            msg_print("你感觉自己天下无敌！");

            msg_print(NULL);
            set_invuln(randint1(8) + 8, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void limber_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "柔韧");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的肌肉变得柔韧。");
        mut_lose(MUT_ARTHRITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的肌肉变得僵硬。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的身体非常柔韧 (+3 敏捷)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void limp_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "跛行");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始一瘸一拐地走。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再一瘸一拐地走了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你走路一瘸一拐的 (-10% 行走速度)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void loremaster_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "博学者");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉知识渊博。");
        identify_pack();
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再懂得那么多了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是一名博学者。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "在你捡起物品时自动鉴定。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->auto_id = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void magic_resistance_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法抗性");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得对魔法有抗性。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你再次变得容易受魔法影响。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你对魔法具有抗性。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.sav += (15 + (p_ptr->lev / 5));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void merchants_friend_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "商人之友");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感到有强烈的购物欲望！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再想去购物了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是商人之友。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "你将获得新的购物力量。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void moron_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "白痴");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的大脑萎缩了...");
        mut_lose(MUT_HYPER_INT);
        mut_lose(MUT_PULTITIS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的大脑恢复了正常。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是个白痴 (-4 智力/感知)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void motion_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "行动自如");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你走起路来更加自信。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你走起路来没那么自信了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的动作精准且有力 (自由行动; +1 潜行)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->free_act++;
        p_ptr->skills.stl += 1;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void nausea_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恶心");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的胃开始恶心地翻腾。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的胃停止了翻腾。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的胃非常不舒服。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->slow_digest && one_in_(9000))
        {
            disturb(0, 0);

            msg_print("你的胃一阵翻腾，把午饭都吐出来了！");
            msg_print(NULL);

            set_food(PY_FOOD_WEAK);

            stop_mouth();
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void no_inhibitions_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "毫无顾忌");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然觉得想做什么就做什么。");
        mut_lose(MUT_COWARDICE);
        mut_lose(MUT_FEARLESS);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉极其自由了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你毫无顾忌。");
        break;
    case SPELL_CALC_BONUS:
        res_add(RES_FEAR);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void normality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "正常化");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉出奇地正常。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉像平常一样奇怪。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你也许变异了，但你正在恢复。");
        break;
    case SPELL_PROCESS:
        if (one_in_(5000))
        {
            if (mut_lose_random(NULL))
                msg_print("你感觉古怪地正常。");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void one_with_magic_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魔法亲和");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉自己与魔法融为一体。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉与魔法融为一体。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有几率抵抗解除魔法和反魔法。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 77% 的几率抵抗反魔法和解除魔法（在变形或处于怨灵形态时几率较低）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void peerless_sniper_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "无双狙击手");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉远处的怪物放松了...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉远处的怪物恢复了平常暴躁的本性。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的远程投射物不再激怒怪物。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "用远程武器伤害怪物不再引发反击。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void peerless_tracker_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "无双追踪者");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉能追踪任何东西...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的追踪能力不再那么好了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是一名无双的追踪者。");
        break;
    case SPELL_DESC:
        var_set_string(res, "映射附近区域。探测所有怪物、陷阱、门和楼梯。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "赋予映射周围环境并探测附近怪物的能力。");
        break;
    case SPELL_CAST:
    {
        int rad1 = DETECT_RAD_MAP;
        int rad2 = DETECT_RAD_DEFAULT;
        map_area(rad1);
        detect_traps(rad2, TRUE);
        detect_doors(rad2);
        detect_stairs(rad2);
        detect_monsters_normal(rad2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void polymorph_wounds_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "伤口变形");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉混沌的力量进入了你的旧伤疤。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉混沌的力量离开了你的旧伤疤。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的生命值受混沌力量的影响。");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
            do_poly_wounds();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void potion_chugger_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "狂饮药水");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉能一口气干掉半打治疗药水。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再想要狂饮药水了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你喝药水的速度比平常快得多。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "让你喝药水的速度变为原来的两倍。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void produce_mana_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生成法力");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始不受控制地产生魔法能量。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止不受控制地产生魔法能量。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你不受控制地产生魔法能量。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(9000))
        {
            int dir = 0;
            disturb(0, 0);
            msg_print("魔法能量在你体内流淌！你必须释放它！");
            flush();
            msg_print(NULL);
            (void)get_hack_dir(&dir);
            fire_ball(GF_MANA, dir, p_ptr->lev * 2, 3);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void puny_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "孱弱");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的肌肉萎缩了...");
        mut_lose(MUT_HYPER_STR);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的肌肉恢复了正常。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你很孱弱 (-4 力量)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void purple_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "紫神眷顾");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你引起了一位紫神的注意！");
        /* In case it isn't obvious, every character has a chaos deity assigned at birth. */
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了紫神们的关注。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你是一位紫神的信徒。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_banish_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "随机放逐");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉到一股可怕的力量潜伏在你身后。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉有可怕的力量潜伏在你身后。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有时候会导致附近的生物消失。");
        break;
    case SPELL_PROCESS:
        if (one_in_(9000))
        {
            disturb(0, 0);
            msg_print("你突然感到有点孤单。");

            banish_monsters(100);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_teleport_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "随机传送");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的位置似乎很不确定...");
        mut_lose(MUT_TELEPORT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的位置似乎更确定了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会随机传送。");
        break;
    case SPELL_PROCESS:
        if (!res_save_default(RES_NEXUS) && !p_ptr->anti_tele && (randint1(5000) == 88))
        {
            disturb(0, 0);
            msg_print("你的位置突然变得很不确定...");
            msg_print(NULL);
            teleport_player(40, TELEPORT_PASSIVE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void raw_chaos_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "原始混沌");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉周围的宇宙变得不稳定了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉周围的宇宙变得更稳定了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你偶尔会被原始混沌所包围。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(8000))
        {
            disturb(0, 0);
            msg_print("你感觉世界在你周围扭曲！");
            msg_print(NULL);
            fire_ball(GF_CHAOS, 0, p_ptr->lev, 8);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void regeneration_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "再生");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始再生。");
        mut_lose(MUT_FLESH_ROT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你停止再生。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你正在再生。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen += 100;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void resilient_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "非凡韧性");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得异常有韧性。");
        mut_lose(MUT_ALBINO);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你恢复了寻常的韧性。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你非常有韧性 (+4 体质)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void rotting_flesh_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "腐烂之躯");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的肉体感染了腐烂病！");
        mut_lose(MUT_STEEL_SKIN);
        mut_lose(MUT_REGEN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的肉体不再受腐烂病折磨！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的肉体正在腐烂 (-2 体质, -1 魅力, 缓慢再生)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->regen -= 80;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void sacred_vitality_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神圣活力");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你获得了神圣活力的力量！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了神圣活力的力量！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你获得所有治疗效果的加成。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 20% 的所有治疗效果加成。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void scales_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鳞片");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的皮肤变成了黑色的鳞片！");
        mut_lose(MUT_STEEL_SKIN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的鳞片消失了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的皮肤变成了鳞片 (-1 魅力, +10 护甲)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void scorpion_tail_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "蝎尾");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你长出了一条蝎尾！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了蝎尾！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着蝎尾。");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 3;
        a.ds = 7;
        a.weight = 50;
        a.blows = 100;
        a.effect[0] = GF_POIS;
        a.msg = "你抽打了过去。";
        a.name = "尾巴";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void sensitive_eyes_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "敏感的双眼");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的眼睛突然感觉非常敏感。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的眼睛不再感觉敏感。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的眼睛非常敏感。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->see_infra += 4;
        res_add_vuln(RES_BLIND);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void shadow_walk_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "影中漫步");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉现实就像纸一样薄。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉自己被困在现实中了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你偶尔会跌入其他阴影中。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(12000) && !p_ptr->inside_arena)
            alter_reality();
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void short_legs_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "短腿");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的腿变成了短粗的树桩！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的腿伸长到了正常长度。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的腿是短粗的树桩 (-3 速度)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed -= 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void silly_voice_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "滑稽嗓音");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的声音变成了可笑的尖叫声！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的声音恢复了正常。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的声音是滑稽的尖叫声 (-4 魅力)。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}


void speed_flux_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "速度波动");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你变得时而狂躁时而抑郁。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再时而狂躁时而抑郁。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的移动速度会随机变快或变慢。");
        break;
    case SPELL_PROCESS:
        if (one_in_(6000))
        {
            disturb(0, 0);
            if (one_in_(2))
            {
                msg_print("你感觉没那么有活力了。");
                if (p_ptr->fast > 0)
                    set_fast(0, TRUE);
                else
                {
                    if ((!p_ptr->slow) && (one_in_(2))) set_slow(randint1(30) + 10, FALSE);
                    else (void)p_inc_minislow(10);
                }
            }
            else
            {
                msg_print("你感觉更有活力了。");
                if ((p_ptr->slow > 0) || (p_ptr->minislow > 0))
                {
                    if (p_ptr->slow > 0) set_slow(0, TRUE);
                    if (p_ptr->minislow > 0) (void)p_inc_minislow(-10);
                }
                else
                    set_fast(randint1(30) + 10, FALSE);
            }
            msg_print(NULL);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void speed_reader_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "快速阅读");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉能读完一本长篇小说。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你读书不再那么快了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你阅读卷轴的速度比平常快。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "让你阅读卷轴的速度变为原来的两倍。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void steel_skin_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "钢铁之肤");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的皮肤变成了钢铁！");
        mut_lose(MUT_SCALES);
        mut_lose(MUT_WARTS);
        mut_lose(MUT_FLESH_ROT);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的皮肤恢复成了血肉！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的皮肤是钢铁构成的 (-1 敏捷, +25 护甲)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 25;
        p_ptr->dis_to_a += 25;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void strong_mind_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "强韧心智");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉完全掌控了自己的心智！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉完全掌控了自己的心智。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你免疫法力吸取。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "使你免疫法力吸取。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void subtle_casting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "隐秘施法");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉远处的怪物放松了...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉远处的怪物恢复了平常暴躁的本性。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的远程攻击法术不再激怒怪物。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "用远程魔法伤害怪物不再引发反击。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void telepathy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "心灵感应");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你发展出了心灵感应能力！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你失去了心灵感应能力！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你具有心灵感应。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->telepathy = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void tentacles_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "触手");
        break;
    case SPELL_GAIN_MUT:
        msg_print("邪恶的触手从你的身体两侧长了出来。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你身体两侧的触手消失了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着邪恶的触手。");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 2;
        a.ds = 5;
        a.weight = 50;
        a.blows = 100;
        a.msg = "你用触手打中了目标。";
        a.name = "触手";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void tread_softly_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "悄步前行");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的潜行能力提升了。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉自己的潜行能力提升了。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉自己的潜行能力恢复了正常。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "使你的潜行能力提升 3。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->skills.stl += 3;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void trunk_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "象鼻");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的鼻子长成了大象一样的长鼻。");
        mut_lose(MUT_BEAK);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的鼻子恢复了正常长度。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着象鼻。");
        break;
    case SPELL_CALC_BONUS:
    {
        innate_attack_t    a = {0};
        a.dd = 1;
        a.ds = 4;
        a.weight = 200;
        a.blows = 100;
        a.msg = "你用象鼻打中了目标。";
        a.name = "象鼻";
        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void untouchable_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "不可触碰");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉自己是不可触碰的！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉自己是可以被触碰的！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你不可触碰并获得护甲加成。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +20 护甲。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 20;
        p_ptr->dis_to_a += 20;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void unyielding_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "不屈不挠");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你绝不屈服！！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("唉！还不如放弃算了...");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你不屈不挠并获得额外生命值。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供生命值加成（每角色等级 +1 HP）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_melee_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "致命旋风");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始带着致命的力量旋转！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再带着致命的力量旋转。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你带着致命的力量旋转。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供一个额外的伤害面骰以及 +25 命中/伤害加成。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_speed_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "龙卷风速");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始以龙卷风般的速度移动！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再以龙卷风般的速度移动。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你以龙卷风般的速度移动 (+8 速度)。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "提供 +8 速度。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->pspeed += 8;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vortex_control_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "吐息控制");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你学会了控制你的吐息。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再控制你的吐息。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你控制着你的吐息。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "增加吐息的消耗并降低某些元素的吐息威力，但允许你选择吐息的元素。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void vulnerability_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "脆弱");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉莫名地暴露在外。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉没那么暴露了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你很容易受到元素伤害。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void warning_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "预警");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然感到偏执多疑。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感到偏执多疑。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你会收到关于敌人的预警。");
        break;
    case SPELL_PROCESS:
        if (one_in_(1000))
        {
            int danger_amount = 0;
            int monster;

            for (monster = 0; monster < m_max; monster++)
            {
                monster_type    *m_ptr = &m_list[monster];
                monster_race    *r_ptr = &r_info[m_ptr->r_idx];

                /* Skip dead monsters */
                if (!m_ptr->r_idx) continue;

                if (r_ptr->level >= p_ptr->lev)
                {
                    danger_amount += r_ptr->level - p_ptr->lev + 1;
                }
            }

            if (danger_amount > 100)
                msg_print("你感到极度恐惧！");

            else if (danger_amount > 50)
                msg_print("你感到恐惧！");

            else if (danger_amount > 20)
                msg_print("你感到非常担忧！");

            else if (danger_amount > 10)
                msg_print("你感到偏执多疑！");

            else if (danger_amount > 5)
                msg_print("你感觉还算安全。");

            else
                msg_print("你感到孤单。");
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void warts_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "长疣");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你浑身长满了令人恶心的疣！");
        mut_lose(MUT_STEEL_SKIN);
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的疣消失了！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的皮肤上长满了疣 (-2 魅力, +5 护甲)。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->to_a += 5;
        p_ptr->dis_to_a += 5;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wasting_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "恐怖枯萎");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你突然染上了一种可怕的枯萎病。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你治愈了可怕的枯萎病！");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你患有可怕的枯萎病。");
        break;
    case SPELL_PROCESS:
        if (one_in_(3000))
        {
            int which_stat = randint0(6);
            int sustained = FALSE;

            switch (which_stat)
            {
            case A_STR:
                if (p_ptr->sustain_str) sustained = TRUE;
                break;
            case A_INT:
                if (p_ptr->sustain_int) sustained = TRUE;
                break;
            case A_WIS:
                if (p_ptr->sustain_wis) sustained = TRUE;
                break;
            case A_DEX:
                if (p_ptr->sustain_dex) sustained = TRUE;
                break;
            case A_CON:
                if (p_ptr->sustain_con) sustained = TRUE;
                break;
            case A_CHR:
                if (p_ptr->sustain_chr) sustained = TRUE;
                break;
            default:
                msg_print("选择了无效的属性！");
                sustained = TRUE;
                break;
            }

            if (!sustained)
            {
                disturb(0, 0);
                msg_print("你能感觉到自己正在日渐消瘦！");
                msg_print(NULL);
                dec_stat(which_stat, randint1(6) + 6, one_in_(6));
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void waybread_into_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "兰巴斯不耐受");
        break;
    case SPELL_GAIN_MUT:
        msg_print("兰巴斯的念头突然让你感到恶心！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你治愈了兰巴斯不耐受。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你有兰巴斯不耐受症。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->levitation = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void weapon_skills_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "武器多面手");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你感觉自己能精通任何武器...");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再感觉自己如此精通了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以精通任何武器。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "允许你对任何武器达到完全精通（但这不能消除不适合武器带来的惩罚）。");
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void random_telepathy_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "随机心灵感应");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的思绪突然飞向了奇怪的方向。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的思绪回到了无聊的正轨。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的心智会随机地扩展和收缩。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            if (p_ptr->tim_esp > 0)
            {
                msg_print("你感觉脑子里云雾缭绕！");
                set_tim_esp(0, TRUE);
            }
            else
            {
                msg_print("你的心智扩展了！");
                set_tim_esp(p_ptr->lev, FALSE);
            }
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void weird_mind_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "诡异心智");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你不再被不可名状的恐怖所困扰！");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你感觉自己又容易受到不可名状的恐怖的影响了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你诡异的心智不受不可名状的恐怖和幻觉的影响。");
        break;
    case SPELL_HELP_DESC:
        var_set_string(res, "保护你的心智免受幻觉和不可名状的恐怖的侵害。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->no_eldritch = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wings_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "翅膀");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你长出了一对翅膀。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的翅膀脱落了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你长着翅膀。");
        break;
    case SPELL_CALC_BONUS:
        p_ptr->levitation = TRUE;
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void wraith_mut(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "怨灵形态");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你开始在物质世界中忽隐忽现。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你稳固地存在于物质世界中了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你在物质现实中忽隐忽现。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(3000))
        {
            disturb(0, 0);
            msg_print("你感觉自己变得没有实体了！");
            msg_print(NULL);
            set_wraith_form(randint1(p_ptr->lev / 2) + (p_ptr->lev / 2), FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

