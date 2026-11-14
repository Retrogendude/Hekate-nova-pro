
/* Hands-on logic adapted from the supplied HWTest-RCM GUI. GPL-2.0. */
#include "gui.h"
#include "gui_nova.h"
#include "gui_choices.h"
#include "gui_interactive.h"
#include "gui_stick.h"
#include "../diagnostics/component_summary.h"
#include "../diagnostics/diag_format.h"
#include "../fonts/font_manager.h"
#include <string.h>
#include <stdlib.h>
#define GUI_W 1280
#define GUI_H 720
#define GUI_CONTENT_W 1212
#define GUI_CONTENT_H 480
#define GUI_MARGIN 34
#define GUI_RIGHT (GUI_W-GUI_MARGIN)
#define GUI_TOP 16
#define GUI_COL_BG_DARK nyx_nova_color(NOVA_COLOR_BACKGROUND)
#define GUI_COL_BG_LIGHT LV_COLOR_HEX(0x334050)
#define GUI_COL_LINE nyx_nova_color(NOVA_COLOR_BORDER)
#define GUI_COL_TEXT nyx_nova_color(NOVA_COLOR_TEXT)
#define GUI_COL_ACCENT nyx_nova_color(NOVA_COLOR_ICONS)
#define GUI_COL_OK LV_COLOR_HEX(0x8AE6B6)
#define GUI_COL_WARN LV_COLOR_HEX(0xF4B85F)
#define GUI_COL_FAIL LV_COLOR_HEX(0xFF6B6B)
#define GUI_HEX_ACCENT "55C9F1"
#define GUI_HEX_OK "8AE6B6"
#define GUI_HEX_WARN "F4B85F"
#define GUI_HEX_FAIL "FF6B6B"
#define ICON_OK "+"
#define ICON_WARN "!"
#define ICON_FAIL "!"
#define ICON_GAMEPAD ""
#define ICON_TOUCH ""
#define ICON_CHECK ""
#define gui_font_20 (*nyx_font_small())
#define gui_font_30 (*nyx_font_ui(30))
static lv_obj_t *interactive_window,*interactive_overlay;
static lv_task_t *interactive_power;
static bool interactive_jc,interactive_power_held;
static jc_gamepad_rpt_t interactive_pad;
static bool (*interactive_sink)(const touch_event_t *);
static void (*interactive_leave)(void);
static lv_signal_func_t interactive_base_signal;
static u32 interactive_mire,interactive_started,mire_visited,mire_observed,mire_anomalies;
static lv_obj_t *mire_controls,*mire_previous,*mire_next;
static bool mire_active;
static u32 mire_buttons;
static lv_res_t mire_change(lv_obj_t *button);
void gui_touch_test(void);
void gui_joycon_test(void);
static void _tt_result(void);
static bool s_tt_over;
static void gui_back(void);
static void gui_on_leave(void (*leave)(void)){interactive_leave=leave;}
static void gui_jc_nav(bool enabled){interactive_jc=!enabled;}
static jc_gamepad_rpt_t *gui_jc_state(void){return &interactive_pad;}
bool nyx_interactive_joycon_active(void){return interactive_jc;}
bool nyx_interactive_joycon(const jc_gamepad_rpt_t *pad){if(!interactive_jc)return false;if(pad)interactive_pad=*pad;return true;}
bool nyx_interactive_touch(const touch_event_t *event){return interactive_sink?interactive_sink(event):false;}
static void gui_touch_sink(bool (*sink)(const touch_event_t *)){interactive_sink=sink;}
static void gui_statusbar_hide(bool hidden){(void)hidden;}
static bool gui_touch_ok(void){return gui_diagnostics_inputs_ready();}
static void gui_uart_puts(const char *text){(void)text;}
static lv_theme_t *gui_th(void){return lv_theme_get_current();}
static lv_obj_t *gui_panel(lv_obj_t *p,int x,int y,int w,int h){return nyx_nova_panel(p,x,y,w,h);}
static lv_obj_t *gui_label(lv_obj_t *p,const lv_font_t *font,lv_color_t color,const char *text){
 (void)color;lv_obj_t *label=nyx_nova_text(p,text,0,0,0,font==nyx_font_small()?NOVA_SMALL:NOVA_BODY);lv_label_set_recolor(label,true);lv_label_set_long_mode(label,LV_LABEL_LONG_EXPAND);return label;
}
static lv_obj_t *gui_label_wrap(lv_obj_t *p,const lv_font_t *font,lv_color_t color,int w,const char *text){lv_obj_t *label=gui_label(p,font,color,text);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,w);return label;}
static lv_obj_t *gui_hint(lv_obj_t *p,int width,const char *text){return width?gui_label_wrap(p,nyx_font_small(),GUI_COL_TEXT,width,text):gui_label(p,nyx_font_small(),GUI_COL_TEXT,text);}
static lv_obj_t *gui_button(lv_obj_t *p,const char *text,lv_action_t action){lv_obj_t *b=lv_btn_create(p,NULL);lv_btn_set_fit(b,true,true);lv_btn_set_action(b,LV_BTN_ACTION_CLICK,action);lv_obj_t *l=lv_label_create(b,NULL);lv_label_set_text(l,text);return b;}
static void gui_show(void (*view)(void)){gui_back();view();}
static void gui_mbox(const char *text,const char **map,lv_btnm_action_t action){lv_obj_t *box=lv_mbox_create(lv_layer_top(),NULL);lv_obj_set_width(box,1000);lv_mbox_set_text(box,text);lv_mbox_add_btns(box,map,action);lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);lv_obj_set_free_num(box,0x1747);}
static lv_res_t interactive_window_signal(lv_obj_t *obj,lv_signal_t signal,void *parameter){
 if(signal==LV_SIGNAL_CLEANUP&&obj==interactive_window){
  interactive_sink=NULL;interactive_jc=false;mire_active=false;if(interactive_leave)interactive_leave();interactive_leave=NULL;
  if(interactive_power)lv_task_del(interactive_power);
  interactive_power=NULL;
  if(interactive_overlay)lv_obj_del(interactive_overlay);
  interactive_overlay=NULL;interactive_window=NULL;
 }
 return interactive_base_signal(obj,signal,parameter);
}
static lv_res_t interactive_close(lv_obj_t *button){return nyx_win_close_action(button);}
static void interactive_power_tick(void *context){
 (void)context;u32 buttons=btn_read();bool held=(buttons&BTN_POWER)!=0;
 if(mire_active){
  if((buttons&BTN_VOL_UP)&&!(mire_buttons&BTN_VOL_UP))mire_change(mire_next);
  if((buttons&BTN_VOL_DOWN)&&!(mire_buttons&BTN_VOL_DOWN))mire_change(mire_previous);
  mire_buttons=buttons;
 }
 if(held&&!interactive_power_held){if(interactive_sink&&!s_tt_over)_tt_result();else gui_back();}
 interactive_power_held=held;
}
static lv_obj_t *gui_win_new(const char *title){
 interactive_window=nyx_create_standard_window(title,interactive_close);
 nyx_nova_fixed_page(((lv_win_ext_t *)lv_obj_get_ext_attr(interactive_window))->page);
 interactive_base_signal=lv_obj_get_signal_func(interactive_window);lv_obj_set_signal_func(interactive_window,interactive_window_signal);
 interactive_started=get_tmr_ms();
 interactive_power_held=(btn_read()&BTN_POWER)!=0;
 interactive_power=lv_task_create(interactive_power_tick,40,LV_TASK_PRIO_MID,NULL);return interactive_window;
}
static lv_obj_t *gui_blank_new(void){
 gui_win_new("Grille tactile");interactive_overlay=lv_cont_create(lv_layer_top(),NULL);lv_cont_set_fit(interactive_overlay,false,false);lv_cont_set_layout(interactive_overlay,LV_LAYOUT_OFF);
 lv_obj_set_size(interactive_overlay,1280,720);lv_obj_set_pos(interactive_overlay,0,0);return interactive_overlay;
}
static void gui_back(void){
 /* Remove only this test's result box; never unrelated application dialogs. */
 for(lv_obj_t *child=lv_obj_get_child(lv_layer_top(),NULL);child;){lv_obj_t *next=lv_obj_get_child(lv_layer_top(),child);if(lv_obj_get_free_num(child)==0x1747)lv_obj_del(child);child=next;}
 if(interactive_window&&close_btn)nyx_win_close_action(close_btn);
}
/*
 * hwtest GUI - the hands-on tests: Joy-Con and touchscreen.
 *
 * Both read the hardware directly, many times a second, instead of going
 * through LVGL's input: every Joy-Con button is under test in the first,
 * every touch in the second. So the way out is always something the test
 * does not use - POWER (the window's focused Close button, the touch test's
 * Finish), or the touch test's message box once it is over.
 */



static lv_style_t *_box_style(lv_style_t *st, lv_color_t bg, lv_color_t border,
                              lv_coord_t radius, lv_coord_t border_w)
{
    lv_style_copy(st, &lv_style_plain);
    st->body.main_color = bg;
    st->body.grad_color = bg;
    st->body.radius = radius;
    st->body.border.color = border;
    st->body.border.width = border_w;
    st->body.border.opa = LV_OPA_COVER;
    return st;
}

static lv_obj_t *_box(lv_obj_t *parent, lv_style_t *st, lv_coord_t x,
                      lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *o = lv_obj_create(parent, NULL);
    lv_obj_set_style(o, st);
    lv_obj_set_click(o, false);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    return o;
}

/* ======================================================================= */
/* Joy-Con                                                                 */

#define STICK_RING  170
#define STICK_DOT   22
#define STICK_DEAD  56

/* The panel: two side by side across the content, 24 px in from their
 * frame for everything inside. */
#define JC_PANEL_W  ((GUI_CONTENT_W - 36) / 2)
#define JC_IN       24
#define JC_TEXT_X   (JC_IN + STICK_RING + 24)

typedef struct {
    const char *name;
    u8 bit;
} jc_btn_t;

/* Bit numbers of jc_gamepad_rpt_t.buttons (bdk input/joycon.h). Not SL
 * and SR: they sit inside the rail once a Joy-Con is on it, and the test
 * only reads Joy-Cons on the rails. */
static const jc_btn_t s_btn_l[] = {
    { "Up", 17 }, { "Down", 16 }, { "Left", 19 }, { "Right", 18 },
    { "L", 22 }, { "ZL", 23 }, { "-", 8 }, { "Capture", 13 },
    { "Stick", 11 },
};
static const jc_btn_t s_btn_r[] = {
    { "A", 3 }, { "B", 2 }, { "X", 1 }, { "Y", 0 },
    { "R", 6 }, { "ZR", 7 }, { "+", 9 }, { "Home", 12 },
    { "Stick", 10 },
};
#define JC_BTNS 9

typedef struct {
    lv_obj_t *status, *dot, *centre, *range, *count;
    lv_obj_t *btn[JC_BTNS];
    u32 lit;                 /* held right now                            */
    u32 done;                /* pressed at some point                     */
    bool seen;               /* connected at the last tick                */
    bool ever;               /* connected at some point                   */
    u32 drops;               /* times the connection went since then      */
    u8 batt;
    gui_stick_t stick;
} jc_side_t;

static jc_side_t s_jc[2];
static lv_task_t *s_jc_task;
static lv_style_t st_ring, st_dead, st_dot, st_pill, st_pill_on, st_pill_done;

/* How many of the buttons have been pressed so far, on the title line,
 * against the panel's right inset. */
static void _jc_count(jc_side_t *s)
{
    u32 n = 0;
    for (u32 b = s->done; b; b >>= 1)
        n += b & 1;
    char buf[48];
    nyx_diag_snprintf(buf,sizeof(buf),"Boutons : %d/%d",n,JC_BTNS);
    lv_label_set_text(s->count, buf);
    lv_obj_align(s->count, NULL, LV_ALIGN_IN_TOP_RIGHT, -JC_IN,
                 26 + (gui_font_30.h_px - gui_font_20.h_px) / 2);
}

static void _jc_side_build(lv_obj_t *win, int side, lv_coord_t x0)
{
    jc_side_t *s = &s_jc[side];
    memset(s, 0, sizeof(*s));
    lv_obj_t *panel = gui_panel(win, x0, GUI_TOP, JC_PANEL_W, GUI_CONTENT_H);

    lv_obj_t *t = gui_label(panel, &gui_font_30, GUI_COL_TEXT,
        side ? "Joy-Con droit" : "Joy-Con gauche");
    lv_obj_set_pos(t, JC_IN, 26);
    s->count = gui_hint(panel, 0, "");
    _jc_count(s);
    s->status = gui_hint(panel, 0, "Non connecté · placer sur le rail");
    lv_label_set_long_mode(s->status,LV_LABEL_LONG_BREAK);lv_obj_set_width(s->status,JC_PANEL_W-2*JC_IN);nyx_nova_fit_text(s->status,32);
    lv_obj_set_pos(s->status, JC_IN, 68);

    lv_obj_t *ring = _box(panel, &st_ring, JC_IN, 108, STICK_RING, STICK_RING);
    _box(ring, &st_dead, (STICK_RING - STICK_DEAD) / 2, (STICK_RING - STICK_DEAD) / 2,
         STICK_DEAD, STICK_DEAD);
    s->dot = _box(ring, &st_dot, (STICK_RING - STICK_DOT) / 2,
                  (STICK_RING - STICK_DOT) / 2, STICK_DOT, STICK_DOT);

    lv_coord_t tw = JC_PANEL_W - JC_TEXT_X - JC_IN;
    lv_obj_t *sh = gui_label(panel, &gui_font_30, GUI_COL_TEXT, "Stick");
    lv_obj_set_pos(sh, JC_TEXT_X, 112);
    s->centre = gui_hint(panel, tw, "Relâcher le stick une seconde, puis parcourir les quatre directions.");
    lv_obj_set_pos(s->centre, JC_TEXT_X, 146);
    s->range = gui_hint(panel, tw, "");
    lv_obj_set_pos(s->range, JC_TEXT_X, 214);

    /* The buttons, drawn as theme buttons in four even columns: one held
     * on the Joy-Con looks pressed here, and once let go it keeps the
     * theme's toggled look, so the plain ones are what is left to press. */
    const jc_btn_t *btns = side ? s_btn_r : s_btn_l;
    lv_coord_t pw = (JC_PANEL_W - 2 * JC_IN - 3 * 12) / 4;
    for (u32 i = 0; i < JC_BTNS; i++) {
        lv_obj_t *p = lv_cont_create(panel, NULL);
        lv_cont_set_style(p, &st_pill);
        lv_obj_set_click(p, false);
        lv_obj_set_size(p, pw, 42);
        lv_obj_set_pos(p, JC_IN + (i % 4) * (pw + 12), 292 + (i / 4) * 50);
        lv_obj_t *l = lv_label_create(p, NULL);
        lv_label_set_text(l, btns[i].name);
        lv_obj_align(l, NULL, LV_ALIGN_CENTER, 0, 0);
        s->btn[i] = p;
    }
}

static void _jc_status(jc_side_t *s)
{
    static const char *batt_name[] = { "empty", "very low", "low", "half", "full" };
    u8 lvl = s->batt >> 1;
    char buf[128];
    if(s->seen)nyx_diag_snprintf(buf,sizeof(buf),"Connecté · batterie %s",nyx_tr(lvl<5?batt_name[lvl]:"unknown"));
    else nyx_diag_snprintf(buf,sizeof(buf),"%s",nyx_tr("Connexion perdue · vérifier le rail"));
    if(s->ever)nyx_diag_snprintf(buf+strlen(buf),sizeof(buf)-strlen(buf)," · pertes %d / retours %d",s->drops+!s->seen,s->drops);
    lv_label_set_text(s->status, buf);
    nyx_nova_fit_text(s->status,32);
}

/* The travel: how many directions it has been round, then whether each
 * reaches as far as the stick's own best one. */
static void _jc_range(jc_side_t *s, int side)
{
    (void)side;u32 reach[4],pct[4];u32 covered=gui_stick_range(&s->stick,reach,pct);
    char text[192];nyx_diag_snprintf(text,sizeof(text),"Amplitude G %d / D %d\nH %d / B %d · %d/4",reach[0],reach[1],reach[2],reach[3],covered);
    lv_label_set_text(s->range,text);nyx_nova_fit_text(s->range,72);
}

static void _jc_judge(jc_side_t *s)
{
    u32 reach[4],pct[4],covered=gui_stick_range(&s->stick,reach,pct);char text[192];
    if(covered<4)nyx_diag_snprintf(text,sizeof(text),"Repos %d, %d\nDérive non évaluée : parcourir les 4 directions.",s->stick.rest_x,s->stick.rest_y);
    else nyx_diag_snprintf(text,sizeof(text),"Repos %d, %d\nDécalage mesuré : %d%% · %s",s->stick.rest_x,s->stick.rest_y,gui_stick_drift(&s->stick),nyx_tr(gui_stick_drift(&s->stick)>8?"À contrôler":"Centré"));
    lv_label_set_text(s->centre,text);nyx_nova_fit_text(s->centre,64);
}

static void _jc_side_update(int side, bool conn, u16 sx, u16 sy, u32 buttons,
                            u8 batt, bool inv_y)
{
    jc_side_t *s = &s_jc[side];

    if (conn != s->seen) {
        s->seen = conn;
        if (conn) {
            if (s->ever)
                s->drops++;
            s->ever = true;
            s->batt = batt;
            _jc_status(s);
        } else {
            /* bdk's driver drops a Joy-Con it got no report from for 1.8 s
             * or whose rail says it is gone, and brings it up again; what
             * was learnt about the stick is kept across that. */
            lv_label_set_text(s->status, s->ever ?
                "Connexion perdue · vérifier le rail" :
                "Non connecté · placer sur le rail");
            _jc_status(s);
        }
    }
    if (!conn)
        return;
    if (inv_y)
        sy = 0xFFF - sy;

    u32 ev = gui_stick_feed(&s->stick, sx, sy, get_tmr_ms());
    s32 half = (STICK_RING - STICK_DOT) / 2, px, py;
    gui_stick_dot(&s->stick, sx, sy, half, &px, &py);
    lv_obj_set_pos(s->dot, half + px, half + py);
    if (ev & STICK_RESTED)
        _jc_judge(s);
    if ((ev & STICK_GREW) && s->stick.rest_known) {
        _jc_range(s, side);
        /* The verdict, once the travel is complete and it has rested. */
        u32 reach[4], pct[4];
        if (gui_stick_range(&s->stick, reach, pct) == 4 && !s->stick.still_since)
            _jc_judge(s);
    }

    const jc_btn_t *btns = side ? s_btn_r : s_btn_l;
    for (u32 i = 0; i < JC_BTNS; i++) {
        bool on = (buttons >> btns[i].bit) & 1;
        bool was = (s->lit >> i) & 1;
        if (on == was)
            continue;
        s->lit ^= BIT(i);
        lv_cont_set_style(s->btn[i], on ? &st_pill_on : &st_pill_done);
        if (on && !(s->done & BIT(i))) {
            s->done |= BIT(i);
            _jc_count(s);
        }
    }
}

static void _jc_tick(void *param)
{
    jc_gamepad_rpt_t *jc = gui_jc_state();
    if (!jc)
        return;
    if (jc->sio_mode) {
        /* Lite: one controller, both halves always present. */
        _jc_side_update(0, jc->conn_l, jc->lstick_x, jc->lstick_y,
                        jc->buttons, jc->batt_info_l, true);
        _jc_side_update(1, jc->conn_l, jc->rstick_x, jc->rstick_y,
                        jc->buttons, jc->batt_info_l, true);
        return;
    }
    _jc_side_update(0, jc->conn_l, jc->lstick_x, jc->lstick_y, jc->buttons,
                    jc->batt_info_l, false);
    _jc_side_update(1, jc->conn_r, jc->rstick_x, jc->rstick_y, jc->buttons,
                    jc->batt_info_r, false);
}

static void _jc_leave(void)
{
    char record[1000],side[2][450];
    for(u32 i=0;i<2;++i){
        jc_side_t *s=&s_jc[i];u32 buttons=0,reach[4],percent[4];
        for(u32 bits=s->done;bits;bits>>=1)buttons+=bits&1;
        u32 directions=gui_stick_range(&s->stick,reach,percent);
        nyx_diag_snprintf(side[i],sizeof(side[i]),"%s : connecté une fois=%d, connecté à la fin=%d, boutons=%d/9, pertes=%d, reconnexions=%d. Repos connu=%d (%d, %d), directions=%d/4, amplitudes brutes G/D/H/B=%d/%d/%d/%d.",
            nyx_tr(i?"Droite":"Gauche"),s->ever,s->seen,buttons,s->drops+(s->ever&&!s->seen),s->drops,s->stick.rest_known,s->stick.rest_x,s->stick.rest_y,directions,reach[0],reach[1],reach[2],reach[3]);
        if(directions==4&&s->stick.rest_known){u32 used=strlen(side[i]);nyx_diag_snprintf(side[i]+used,sizeof(side[i])-used," Décalage mesuré=%d%%.",gui_stick_drift(&s->stick));}
    }
    nyx_diag_snprintf(record,sizeof(record),"Joy-Con/Lite (%d ms, Lite=%d).\n%s\n%s\nBoutons non pressés et directions non parcourues : NON TESTÉS. Aucun verdict de panne automatique.",get_tmr_ms()-interactive_started,interactive_pad.sio_mode,side[0],side[1]);
    nyx_component_observation(NYX_MANUAL_COMMANDS,record,false);
    if (s_jc_task)
        lv_task_del(s_jc_task);
    s_jc_task = NULL;
}

void gui_joycon_test(void)
{
    lv_obj_t *win = gui_win_new("Test Joy-Con");
    gui_jc_nav(false);
    gui_on_leave(_jc_leave);

    lv_theme_t *th = gui_th();
    _box_style(&st_ring, GUI_COL_BG_DARK, GUI_COL_LINE, LV_RADIUS_CIRCLE, 3);
    _box_style(&st_dead, GUI_COL_BG_LIGHT, GUI_COL_LINE, LV_RADIUS_CIRCLE, 2);
    _box_style(&st_dot, GUI_COL_ACCENT, GUI_COL_ACCENT, LV_RADIUS_CIRCLE, 0);
    lv_style_copy(&st_pill, th->btn.rel);
    st_pill.text.font = &gui_font_20;
    lv_style_copy(&st_pill_on, th->btn.pr);
    st_pill_on.text.font = &gui_font_20;
    lv_style_copy(&st_pill_done, th->btn.tgl_rel);
    st_pill_done.text.font = &gui_font_20;
    st_pill_done.text.color = th->btn.pr->text.color;

    _jc_side_build(win, 0, GUI_MARGIN);
    _jc_side_build(win, 1, GUI_RIGHT - JC_PANEL_W);

    if (s_jc_task)
        lv_task_del(s_jc_task);
    s_jc_task = lv_task_create(_jc_tick, 20, LV_TASK_PRIO_MID, NULL);
    _jc_tick(NULL);
}

/* ======================================================================= */
/* Touchscreen                                                             */

#define TT_CELL 80
#define TT_COLS (GUI_W / TT_CELL)
#define TT_ROWS (GUI_H / TT_CELL)

/* The panel in the middle: half the screen wide, 32 px in from its frame. */
#define TT_PANEL_W  (GUI_W / 2)
#define TT_PANEL_H  300
#define TT_IN       32

static lv_obj_t *s_cell[TT_ROWS][TT_COLS];
static bool s_hit[TT_ROWS][TT_COLS];
static u32 s_hits;
static bool s_tt_full;
static bool s_tt_pass;           /* this stroke is LVGL's: it began on Finish */
static int s_last_x = -1, s_last_y;
static u32 s_tt_up_at;           /* when the finger last lifted               */
static lv_task_t *s_tt_task;
static lv_obj_t *s_tt_info, *s_tt_count, *s_tt_finger, *s_tt_fin;
static lv_style_t st_cell, st_cell_hit, st_cell_miss, st_finger;

/* The panel comes back this long after the finger lifts. */
#define TT_PANEL_BACK_MS 1000

static void _tt_mark(int x, int y)
{
    int c = x / TT_CELL, r = y / TT_CELL;
    if (c < 0 || r < 0 || c >= TT_COLS || r >= TT_ROWS || s_hit[r][c])
        return;
    s_hit[r][c] = true;
    s_hits++;
    lv_obj_set_style(s_cell[r][c], &st_cell_hit);
}

static void _tt_count(void)
{
    char buf[64];
    nyx_diag_snprintf(buf,sizeof(buf),"Zones : %d/%d", s_hits, TT_COLS * TT_ROWS);
    lv_label_set_text(s_tt_count, buf);
}

static void _tt_result(void);

/* Whether a touch lands on the Finish button, while there is one to see. */
static bool _tt_on_finish(int x, int y)
{
    if (lv_obj_get_hidden(s_tt_info))
        return false;
    lv_area_t a;
    lv_obj_get_coords(s_tt_fin, &a);
    return x >= a.x1 && x <= a.x2 && y >= a.y1 && y <= a.y2;
}

static bool _tt_sink(const touch_event_t *ev)
{
    if (s_tt_over)
        return false;            /* finished: taps go to the message box */
    if (!ev->touch) {
        bool pass = s_tt_pass;
        s_tt_pass = false;
        if (s_last_x >= 0) {
            s_last_x = -1;
            s_tt_up_at = get_tmr_ms();
            lv_obj_set_hidden(s_tt_finger, true);
        }
        return !pass;            /* LVGL must see the end of its tap too */
    }
    if (s_tt_pass)
        return false;
    int x = ev->x, y = ev->y;
    /* Samples arrive at the input rate, so a quick stroke leaves gaps:
     * fill in along the line from the last one. */
    if (s_last_x >= 0) {
        int dx = x - s_last_x, dy = y - s_last_y;
        int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
        int steps = (adx > ady ? adx : ady) / (TT_CELL / 4) + 1;
        for (int i = 1; i <= steps; i++)
            _tt_mark(s_last_x + dx * i / steps, s_last_y + dy * i / steps);
    } else {
        /* A new stroke. One that begins on Finish is a tap, LVGL's to
         * handle; any other takes the panel away while the finger is
         * down, so the squares under it can be seen turning. */
        if (_tt_on_finish(x, y)) {
            s_tt_pass = true;
            return false;
        }
        lv_obj_set_hidden(s_tt_info, true);
        _tt_mark(x, y);
    }
    s_last_x = x;
    s_last_y = y;
    lv_obj_set_hidden(s_tt_finger, false);
    lv_obj_set_pos(s_tt_finger, x - 30, y - 30);
    _tt_count();
    /* Not _tt_result() here: this runs inside LVGL's input read, and the
     * verdict deletes and creates objects. _tt_tick picks it up. */
    if (s_hits == TT_COLS * TT_ROWS)
        s_tt_full = true;
    return true;
}

static void _tt_tick(void *param)
{
    if (s_tt_full) {
        _tt_result();
        return;
    }
    /* The panel comes back a moment after the finger lifts. */
    if (!s_tt_over && s_last_x < 0 && lv_obj_get_hidden(s_tt_info)
        && get_tmr_ms() - s_tt_up_at >= TT_PANEL_BACK_MS) {
        lv_obj_set_hidden(s_tt_info, false);
    }
}

static void touch_record(void)
{
    char record[512];nyx_diag_snprintf(record,sizeof(record),"Grille tactile : %d/%d zones parcourues, contrôleur initialisé=%d, fin demandée=%d, durée=%d ms. Les zones non parcourues sont NON TESTÉES. Ce contrôle manuel ne remplace pas le test matériel ITO.",s_hits,TT_COLS*TT_ROWS,gui_touch_ok(),s_tt_over,get_tmr_ms()-interactive_started);
    nyx_component_observation(NYX_MANUAL_TOUCH,record,false);
}
static void _tt_leave(void)
{
    if(s_tt_task)touch_record();
    if (s_tt_task)
        lv_task_del(s_tt_task);
    s_tt_task = NULL;
}

static lv_res_t _act_tt_finish(lv_obj_t *b)
{
    _tt_result();
    return LV_RES_OK;
}

static lv_res_t _tt_action(lv_obj_t *btns, const char *txt)
{
    if (lv_btnm_get_pressed(btns)==0)
        gui_show(gui_touch_test);
    else
        gui_back();
    return LV_RES_INV;
}

/* The verdict: what was never touched turns red, and a message box says
 * how much of the screen answered. */
static void _tt_result(void)
{
    if (s_tt_over)
        return;
    s_tt_over = true;
    touch_record();
    u32 total = TT_COLS * TT_ROWS;
    for (int r = 0; r < TT_ROWS; r++)
        for (int c = 0; c < TT_COLS; c++)
            if (!s_hit[r][c])
                lv_obj_set_style(s_cell[r][c], &st_cell_miss);
    lv_obj_set_hidden(s_tt_finger, true);
    lv_obj_set_hidden(s_tt_info, true);

    static char text[512];
    nyx_diag_snprintf(text,sizeof(text),"%d / %d zones parcourues.\n\nLes zones restantes sont NON TESTÉES. Ce contrôle manuel ne remplace pas le test matériel du contrôleur tactile.",s_hits,total);
    static const char *map[] = { "\221Again", "\221Done", "" };
    gui_mbox(text, map, _tt_action);
}

void gui_touch_test(void)
{
    lv_obj_t *root = gui_blank_new();
    gui_statusbar_hide(true);
    gui_touch_sink(_tt_sink);
    gui_on_leave(_tt_leave);

    _box_style(&st_cell, GUI_COL_BG_DARK, GUI_COL_BG_LIGHT, 0, 1);
    _box_style(&st_cell_hit, LV_COLOR_HEX(0x00775C), GUI_COL_ACCENT, 0, 1);
    _box_style(&st_cell_miss, LV_COLOR_HEX(0x493D26), GUI_COL_WARN, 0, 1);
    _box_style(&st_finger, GUI_COL_ACCENT, LV_COLOR_WHITE, LV_RADIUS_CIRCLE, 4);
    st_finger.body.opa = LV_OPA_60;

    /* The grid covers the whole panel: dead spots are often along an
     * edge. */
    memset(s_hit, 0, sizeof(s_hit));
    s_hits = 0;
    s_tt_over = false;
    s_tt_full = false;
    s_tt_pass = false;
    s_last_x = -1;
    _tt_leave();
    s_tt_task = lv_task_create(_tt_tick, 50, LV_TASK_PRIO_MID, NULL);
    for (int r = 0; r < TT_ROWS; r++)
        for (int c = 0; c < TT_COLS; c++)
            s_cell[r][c] = _box(root, &st_cell, c * TT_CELL, r * TT_CELL,
                                TT_CELL, TT_CELL);

    /* One line taller when the touchscreen failed, so its warning gets a
     * line of its own instead of crowding the bottom inset. */
    bool touch_ok = gui_touch_ok();
    lv_coord_t ph = TT_PANEL_H + (touch_ok ? 0 : 36);
    s_tt_info = gui_panel(root, (GUI_W - TT_PANEL_W) / 2, (GUI_H - ph) / 2,
                          TT_PANEL_W, ph);
    lv_obj_t *t = gui_label(s_tt_info, &gui_font_30, GUI_COL_TEXT,
                            "Test tactile");
    lv_obj_set_pos(t, TT_IN, 26);
    lv_obj_t *d = gui_hint(s_tt_info, TT_PANEL_W - 2 * TT_IN,
        "Parcourir chaque case avec un doigt. Une case sombre n’a pas été testée ; cela ne prouve pas un défaut.");
    lv_obj_set_pos(d, TT_IN, 80);
    lv_obj_t *fin = gui_button(s_tt_info, "Terminer", _act_tt_finish);
    lv_obj_align(fin, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -TT_IN, -TT_IN);
    s_tt_fin = fin;

    /* The short lines on the left sit 36 px apart, the last of them on the
     * Finish button's centre line. */
    lv_coord_t y = lv_obj_get_y(fin) + lv_obj_get_height(fin) / 2
                   - gui_font_20.h_px / 2 - (touch_ok ? 1 : 2) * 36;
    s_tt_count = gui_label(s_tt_info, &gui_font_20, GUI_COL_TEXT, "");
    lv_obj_set_pos(s_tt_count, TT_IN, y);
    _tt_count();
    lv_obj_t *hint = gui_hint(s_tt_info, 0, "POWER permet aussi de terminer.");
    lv_obj_set_pos(hint, TT_IN, y + 36);
    if (!touch_ok) {
        lv_obj_t *w = gui_label(s_tt_info, &gui_font_20, GUI_COL_WARN,
                                "Le contrôleur tactile n’a pas démarré.");
        lv_obj_set_pos(w, TT_IN, y + 72);
    }

    s_tt_finger = _box(root, &st_finger, 0, 0, 60, 60);
    lv_obj_set_hidden(s_tt_finger, true);
}

lv_res_t nyx_joycon_test(lv_obj_t *button){(void)button;if(!interactive_window)gui_joycon_test();return LV_RES_OK;}
lv_res_t nyx_touch_test(lv_obj_t *button){(void)button;if(!interactive_window)gui_touch_test();return LV_RES_OK;}
static const u32 mire_colors[]={0xFF0000,0x00FF00,0x0000FF,0xFFFFFF,0x000000};
static lv_style_t mire_style;
static lv_obj_t *mire_area,*mire_observation;
static void mire_record(void)
{
    char record[512];nyx_diag_snprintf(record,sizeof(record),"Mires R/V/B/blanc/noir : masques affichés=0x%02X, observés=0x%02X, anomalies signalées=0x%02X, durée=%d ms. Les masques sont dans cet ordre, bit 0=rouge. Déclaration visuelle de l’utilisateur, aucune détection automatique des pixels.",mire_visited,mire_observed,mire_anomalies,get_tmr_ms()-interactive_started);
    nyx_component_observation(NYX_MANUAL_DISPLAY,record,mire_anomalies!=0);
}
static lv_res_t mire_change(lv_obj_t *button){
 interactive_mire=(interactive_mire+(lv_obj_get_free_num(button)?1:4))%5;
 mire_visited|=BIT(interactive_mire);mire_record();
 mire_style.body.main_color=mire_style.body.grad_color=LV_COLOR_HEX(mire_colors[interactive_mire]);lv_btn_set_style(mire_area,LV_BTN_STYLE_REL,&mire_style);lv_btn_set_style(mire_area,LV_BTN_STYLE_PR,&mire_style);return LV_RES_OK;
}
static lv_res_t mire_observe(lv_obj_t *button){
 mire_observed|=BIT(interactive_mire);
 if(lv_obj_get_free_num(button))mire_anomalies|=BIT(interactive_mire);else mire_anomalies&=~BIT(interactive_mire);
 mire_record();
 lv_label_set_text(mire_observation,lv_obj_get_free_num(button)?"Observation manuelle : anomalie visuelle signalée.":"Observation manuelle : aucune anomalie visuelle signalée. Les pixels ne sont pas vérifiés automatiquement.");return LV_RES_OK;
}
static lv_res_t mire_toggle_controls(lv_obj_t *button)
{
    (void)button;lv_obj_set_hidden(mire_controls,!lv_obj_get_hidden(mire_controls));return LV_RES_OK;
}
lv_res_t nyx_screen_patterns(lv_obj_t *button)
{
    (void)button;if(interactive_window)return LV_RES_OK;
    gui_win_new("Mires de l'écran");interactive_mire=0;mire_active=true;mire_buttons=btn_read();
    mire_visited=1;mire_observed=mire_anomalies=0;gui_on_leave(mire_record);mire_record();
    interactive_overlay=lv_cont_create(lv_layer_top(),NULL);lv_cont_set_fit(interactive_overlay,false,false);lv_cont_set_layout(interactive_overlay,LV_LAYOUT_OFF);
    lv_obj_set_size(interactive_overlay,1280,720);lv_obj_set_pos(interactive_overlay,0,0);
    mire_area=lv_btn_create(interactive_overlay,NULL);lv_btn_set_fit(mire_area,false,false);lv_obj_set_size(mire_area,1280,720);lv_obj_set_pos(mire_area,0,0);
    lv_style_copy(&mire_style,&lv_style_plain);mire_style.body.border.width=0;mire_style.body.radius=0;
    mire_style.body.main_color=mire_style.body.grad_color=LV_COLOR_HEX(0xFF0000);lv_btn_set_style(mire_area,LV_BTN_STYLE_REL,&mire_style);lv_btn_set_style(mire_area,LV_BTN_STYLE_PR,&mire_style);
    lv_btn_set_action(mire_area,LV_BTN_ACTION_CLICK,mire_toggle_controls);
    mire_controls=nyx_nova_panel(interactive_overlay,34,526,1212,172);lv_obj_set_click(mire_controls,true);
    mire_previous=gui_button(mire_controls,"Précédent",mire_change);lv_obj_set_free_num(mire_previous,0);lv_obj_set_pos(mire_previous,20,14);
    mire_next=gui_button(mire_controls,"Suivant",mire_change);lv_obj_set_free_num(mire_next,1);lv_obj_set_pos(mire_next,240,14);
    lv_obj_t *ok=gui_button(mire_controls,"Aucune anomalie observée",mire_observe);lv_obj_set_pos(ok,460,14);lv_obj_set_free_num(ok,0);
    lv_obj_t *fault=gui_button(mire_controls,"Anomalie observée",mire_observe);lv_obj_set_pos(fault,850,14);lv_obj_set_free_num(fault,1);
    lv_obj_t *commands[]={mire_previous,mire_next,ok,fault};
    const int widths[]={200,200,370,360},positions[]={20,234,448,832};
    for(u32 i=0;i<4;++i){
        lv_btn_set_fit(commands[i],false,false);lv_obj_set_size(commands[i],widths[i],60);lv_obj_set_pos(commands[i],positions[i],14);
        lv_obj_t *label=lv_obj_get_child(commands[i],NULL);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,widths[i]-24);lv_label_set_style(label,(lv_style_t *)nyx_nova_text_style(NOVA_SMALL));
    }
    mire_observation=nyx_nova_text(mire_controls,"Toucher la mire masque les commandes. VOL+/VOL- : changer. POWER : quitter. Observation visuelle uniquement, sans détection automatique des pixels.",20,84,1172,NOVA_SMALL);nyx_nova_fit_text(mire_observation,76);
    return LV_RES_OK;
}
