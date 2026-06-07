#include "angband.h"

void heroism_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "英雄气概");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时提高战斗能力并赋予极大的勇气。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(25, 25));
        break;
    case SPELL_CAST:
        set_hero(randint1(25) + 25, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_heroism(void) { return cast_spell(heroism_spell); }

void hide_in_mud_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "藏身泥沼");
        break;
    case SPELL_DESC:
        var_set_string(res, "暂时获得穿墙能力，以及对酸的额外抗性。");
        break;
    case SPELL_CAST:
        set_kabenuke(randint1(p_ptr->lev/2) + p_ptr->lev/2, FALSE);
        set_oppose_acid(p_ptr->lev, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void ice_bolt_spell(int cmd, variant *res)
{
    int dd = 5 + p_ptr->lev / 4;
    int ds = 15;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "寒冰箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发寒冰箭。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(dd, spell_power(ds), spell_power(p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_bolt(
            GF_ICE,
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

void identify_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "鉴定术");
        break;
    case SPELL_DESC:
        var_set_string(res, "鉴定单件物品。");
        break;
    case SPELL_CAST:
        var_set_bool(res, ident_spell(NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_identify(void) { return cast_spell(identify_spell); }

void identify_fully_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "完全鉴定");
        break;
    case SPELL_DESC:
        var_set_string(res, "完全鉴定单件物品的全部属性。");
        break;
    case SPELL_CAST:
        var_set_bool(res, identify_fully(NULL));
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_identify_fully(void) { return cast_spell(identify_fully_spell); }

void hand_of_doom_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "末日之手");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图重创目标怪物，按比例大幅吸取其剩余生命值。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        msg_print("你唤起了末日之手！");
        fire_ball_hide(GF_HAND_DOOM, dir, spell_power(p_ptr->lev * 3), 0);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void haste_self_spell(int cmd, variant *res)
{
    int base = spell_power(p_ptr->lev);
    int sides = spell_power(20 + p_ptr->lev);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "自我加速");
        break;
    case SPELL_DESC:
        var_set_string(res, "在一段时间内使你加速。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(base, sides));
        break;
    case SPELL_CAST:
        set_fast(base + randint1(sides), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void healing_I_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "痊愈术");
        break;
    case SPELL_DESC:
        var_set_string(res, "强大的治疗魔法：恢复生命值，并治愈割伤和震慑。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(300)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(300));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void healing_II_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "痊愈术");
        break;
    case SPELL_DESC:
        var_set_string(res, "强大的治疗魔法：恢复生命值，并治愈割伤和震慑。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_heal(0, 0, spell_power(500)));
        break;
    case SPELL_CAST:
        hp_player(spell_power(500));
        set_stun(0, TRUE);
        set_cut(0, TRUE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void hellfire_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "地狱火");
        break;
    case SPELL_DESC:
        var_set_string(res, "直接从地狱深处发射一颗充满邪恶力量的强大法球。善良怪物对此格外脆弱。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(666 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_ball(GF_HELL_FIRE, dir, spell_power(666 + p_ptr->to_d_spell), 3);
            if (!demon_is_(DEMON_BALROG))
                take_hit(DAMAGE_USELIFE, 20 + randint1(30), "the strain of casting Hellfire");
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void hell_lance_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "地狱长枪");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道纯粹的地狱火射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_HELL_FIRE, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_hell_lance(void) { return cast_spell(hell_lance_spell); }

void holy_lance_spell(int cmd, variant *res)
{
    int dam = spell_power(p_ptr->lev * 3 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "神圣长枪");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道纯粹的神圣射线。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_HOLY_FIRE, dir, dam);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_holy_lance(void) { return cast_spell(holy_lance_spell); }

void hp_to_sp_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "生命转法力");
        break;
    case SPELL_DESC:
        var_set_string(res, "将生命值转化为法力值");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你时不时地体验到一种痛苦的清醒。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你不再体验到那种痛苦的清醒了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "有时你会气血上涌。");
        break;
    case SPELL_PROCESS:
        if (!p_ptr->anti_magic && one_in_(4000))
        {
            int wounds = p_ptr->msp - p_ptr->csp;

            if (wounds > 0 && p_ptr->pclass != CLASS_RUNE_KNIGHT)
            {
                int healing = p_ptr->chp;

                if (healing > wounds)
                    healing = wounds;

                p_ptr->csp += healing;

                p_ptr->redraw |= (PR_MANA);
                take_hit(DAMAGE_LOSELIFE, healing, "blood rushing to the head");
            }
        }
        break;

    case SPELL_CAST:
    {
        int gain_sp = take_hit(DAMAGE_USELIFE, p_ptr->lev, "thoughtless convertion from HP to SP") / 5;
        if (gain_sp && p_ptr->pclass != CLASS_RUNE_KNIGHT)
        {
            p_ptr->csp += gain_sp;
            if (p_ptr->csp > p_ptr->msp)
            {
                p_ptr->csp = p_ptr->msp;
                p_ptr->csp_frac = 0;
            }

            p_ptr->redraw |= PR_MANA;
        }
        else
            msg_print("你转换失败了。");

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void hypnotic_gaze_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "催眠凝视");
        break;
    case SPELL_DESC:
        var_set_string(res, "试图魅惑一只怪物。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的眼睛看起来迷人极了……");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的眼睛看起来平淡无奇。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的凝视具有催眠作用。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            int power = p_ptr->lev;
            
            if (prace_is_(RACE_MON_VAMPIRE))
                power *= 2;

            msg_print("你的眼睛看起来迷人极了……");
            charm_monster(dir, power);
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_hypnotic_gaze(void) { return cast_spell(hypnotic_gaze_spell); }

void imp_fire_spell(int cmd, variant *res)
{
    const int ball_lev = 30;
    switch (cmd)
    {
    case SPELL_NAME:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, "火球术");
        else
            var_set_string(res, "火焰箭");
        break;
    case SPELL_SPOIL_NAME:
        var_set_string(res, "火焰箭/火球术");
        break;
    case SPELL_DESC:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, "在选定目标处生成一个火球。");
        else
            var_set_string(res, "向选定目标投掷一枚火焰飞弹。");
        break;
    case SPELL_SPOIL_DESC:
        var_set_string(res, "造成 L 点伤害的火焰箭。在 30 级时，改为释放半径 2、造成 2L 伤害的火球。");
        break;
    case SPELL_INFO:
        if (p_ptr->lev >= ball_lev)
            var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev * 2)));
        else
            var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        if (p_ptr->lev >= ball_lev)
            fire_ball(GF_FIRE, dir, spell_power(p_ptr->lev * 2), 2);
        else
            fire_bolt(GF_FIRE, dir, spell_power(p_ptr->lev));
        var_set_bool(res, TRUE);
        break;
    }
    case SPELL_COST_EXTRA:
        if (p_ptr->lev >= ball_lev)
            var_set_int(res, 7);
        else
            var_set_int(res, 0);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void invoke_logrus_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "唤起罗格鲁斯");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个巨大的混沌法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(spell_power(10), 10, spell_power(p_ptr->lev*4 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_CHAOS, dir, spell_power(damroll(10, 10) + p_ptr->lev*4 + p_ptr->to_d_spell), 4);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void invulnerability_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "无敌术");
        break;
    case SPELL_DESC:
        var_set_string(res, "生成一个无敌法球，完全保护你免受几乎所有的攻击，当屏障破裂或持续时间结束时会消耗一回合。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_duration(4, 4));
        break;
    case SPELL_CAST:
        msg_print("你施放了无敌法球。");
        set_invuln(spell_power(randint1(4) + 4), FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void kiss_of_succubus_spell(int cmd, variant *res)
{
    int dam = spell_power(100 + p_ptr->lev * 2 + p_ptr->to_d_spell);
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "魅魔之吻");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个时空(Nexus)法球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, dam));
        break;
    case SPELL_CAST:
    {
        int dir;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) break;
        fire_ball(GF_NEXUS, dir, dam, 4);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void kutar_expand_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "横向膨胀");
        break;
    case SPELL_DESC:
        var_set_string(res, "像猫一样膨胀起来，暂时提供 +35 的防御等级(AC)，但豁免判定会变得非常差。");
        break;
    case SPELL_CAST:
        set_tsubureru(randint1(20) + 30, FALSE);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}

void laser_eye_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "激光眼");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一道激光束。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(p_ptr->lev*2)));
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的眼睛灼热了片刻。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的眼睛灼热了片刻，然后感到舒缓。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你的眼睛可以发射激光束。");
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (get_fire_dir(&dir))
        {
            fire_beam(GF_LITE, dir, spell_power(2 * p_ptr->lev));
            var_set_bool(res, TRUE);
        }
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_laser_eye(void) { return cast_spell(laser_eye_spell); }

void light_area_spell(int cmd, variant *res)
{
    int dice = 2;
    int sides = p_ptr->lev / 2;
    int rad = spell_power(p_ptr->lev / 10 + 1);

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "照亮区域");
        break;
    case SPELL_DESC:
        var_set_string(res, "永久照亮附近区域和房间内部。");
        break;
    case SPELL_GAIN_MUT:
        msg_print("你的存在可以照亮房间。");
        break;
    case SPELL_LOSE_MUT:
        msg_print("你的存在不再能照亮房间了。");
        break;
    case SPELL_MUT_DESC:
        var_set_string(res, "你可以散发出明亮的光芒。");
        break;
    case SPELL_CAST:
        lite_area(spell_power(damroll(dice, sides)), rad);
        var_set_bool(res, TRUE);
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
bool cast_light_area(void) { return cast_spell(light_area_spell); }

void lightning_ball_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪电球");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一个电光球。");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, spell_power(3*p_ptr->lev/2 + 20 + p_ptr->to_d_spell)));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_fire_dir(&dir)) return;
        fire_ball(GF_ELEC, dir, spell_power(3*p_ptr->lev/2 + 20 + p_ptr->to_d_spell), 2);
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

void lightning_bolt_spell(int cmd, variant *res)
{
    int dd = 3 + p_ptr->lev / 4;
    int ds = 8;

    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "闪电箭");
        break;
    case SPELL_DESC:
        var_set_string(res, "发射一发闪电箭或闪电射线。");
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
            GF_ELEC,
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

void living_trump_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "活体王牌");
        break;
    case SPELL_DESC:
        var_set_string(res, "赋予你随机传送的变异，或者让你能够随心所欲地传送。");
        break;
    case SPELL_CAST:
    {
        int mutation = one_in_(7) ? MUT_TELEPORT : MUT_TELEPORT_RND;

        if (mut_gain(mutation))
            msg_print("你变成了活体王牌。");
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}

