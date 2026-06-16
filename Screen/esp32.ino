#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "TAMC_GT911.h"
#include <lvgl.h>

/*******************************************************
 * BACKLIGHT (LEDC PWM)
 ******************************************************/
#define BL_PIN   45
#define BL_FREQ  5000
#define BL_RES   8

void bl_init()
{
  ledcAttach(BL_PIN, BL_FREQ, BL_RES);
  ledcWrite(BL_PIN, 255);
}

void bl_set(uint8_t val)
{
  ledcWrite(BL_PIN, val);
}

/*******************************************************
 * TOUCH CONFIG
 ******************************************************/
#define TOUCH_SDA    8
#define TOUCH_SCL    9
#define TOUCH_INT    -1
#define TOUCH_RST    38
#define TOUCH_WIDTH  800
#define TOUCH_HEIGHT 480

TAMC_GT911 tp(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST,
              TOUCH_WIDTH, TOUCH_HEIGHT);

/*******************************************************
 * DISPLAY CONFIG
 ******************************************************/
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  5,3,46,7,
  1,2,42,41,40,
  39,0,45,48,47,21,
  14,38,18,17,10,
  0,40,48,88,
  0,13,3,32,
  1,
  16000000
);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  800,480,rgbpanel,0,true
);

/*******************************************************
 * MOODS
 ******************************************************/
struct MoodPreset { const char *name; uint8_t r,g,b; };

MoodPreset moods[] = {
  {"Cool White",255,255,220},
  {"Warm White",255,200,160},
  {"Focus Blue",80,160,255},
  {"Chill Purple",180,80,255},
  {"Sunset",255,120,40},
  {"Night Red",255,40,40},
  {"Mint",80,255,180}
};

const int moodCount = sizeof(moods)/sizeof(moods[0]);
int currentMood = 0;

/*******************************************************
 * POMODORO STATE
 ******************************************************/
#define POMO_WORK_SECS  (25 * 60)
#define POMO_BREAK_SECS (5  * 60)

enum PomoPhase { POMO_IDLE, POMO_WORK, POMO_BREAK };

static PomoPhase  pomoPhase     = POMO_IDLE;
static bool       pomoRunning   = false;
static int32_t    pomoSecsLeft  = POMO_WORK_SECS;
static lv_timer_t *pomoTimer    = NULL;

static lv_obj_t  *lbl_countdown = NULL;
static lv_obj_t  *lbl_phase     = NULL;
static lv_obj_t  *btn_startpause= NULL;
static lv_obj_t  *lbl_sp        = NULL;   // label inside start/pause btn

void pomo_update_display()
{
  if (!lbl_countdown) return;

  int32_t m = pomoSecsLeft / 60;
  int32_t s = pomoSecsLeft % 60;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02ld:%02ld", (long)m, (long)s);
  lv_label_set_text(lbl_countdown, buf);

  if (lbl_phase)
  {
    if      (pomoPhase == POMO_WORK)  lv_label_set_text(lbl_phase, "WORK SESSION");
    else if (pomoPhase == POMO_BREAK) lv_label_set_text(lbl_phase, "BREAK TIME");
    else                              lv_label_set_text(lbl_phase, "READY");
  }

  if (lbl_sp)
    lv_label_set_text(lbl_sp, pomoRunning ? "PAUSE" : "START");
}

void pomo_tick_cb(lv_timer_t *t)
{
  if (!pomoRunning) return;

  pomoSecsLeft--;

  if (pomoSecsLeft <= 0)
  {
    if (pomoPhase == POMO_WORK)
    {
      pomoPhase    = POMO_BREAK;
      pomoSecsLeft = POMO_BREAK_SECS;
      Serial.println("[POMO] Break started");
    }
    else
    {
      pomoPhase    = POMO_WORK;
      pomoSecsLeft = POMO_WORK_SECS;
      Serial.println("[POMO] Work session started");
    }
  }

  pomo_update_display();
}

/*******************************************************
 * LVGL
 ******************************************************/
static lv_display_t  *disp;
static lv_indev_t    *indev;
static lv_color_t    *buf1 = NULL;
static lv_draw_buf_t  draw_buf1;

void my_flush_cb(lv_display_t *d, const lv_area_t *area, uint8_t *px_map)
{
  lv_color_t *color_p = (lv_color_t *)px_map;
  int32_t w = (area->x2 - area->x1 + 1);
  int32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, w, h);
  lv_display_flush_ready(d);
}

void my_touch_read_cb(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
  tp.read();
  if (tp.isTouched && tp.touches > 0)
  {
    data->state   = LV_INDEV_STATE_PRESSED;
    data->point.x = TOUCH_WIDTH  - tp.points[0].x;
    data->point.y = TOUCH_HEIGHT - tp.points[0].y;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/*******************************************************
 * THEME
 ******************************************************/
static lv_style_t st_screen, st_card, st_btn, st_btn_pr, st_title, st_hint;
static lv_style_t st_splash_big, st_splash_sub;
static lv_color_t accent_color, screen_tint, btn_tint;

static lv_color_t clamp_dark_tint(uint8_t r,uint8_t g,uint8_t b,uint8_t div)
{ return lv_color_make(r/div,g/div,b/div); }

static lv_color_t clamp_mid_tint(uint8_t r,uint8_t g,uint8_t b,
                                  uint8_t div,uint8_t add)
{
  uint16_t rr=(uint16_t)(r/div)+add;
  uint16_t gg=(uint16_t)(g/div)+add;
  uint16_t bb=(uint16_t)(b/div)+add;
  if(rr>255)rr=255; if(gg>255)gg=255; if(bb>255)bb=255;
  return lv_color_make((uint8_t)rr,(uint8_t)gg,(uint8_t)bb);
}

void apply_theme_from_mood()
{
  uint8_t r=moods[currentMood].r;
  uint8_t g=moods[currentMood].g;
  uint8_t b=moods[currentMood].b;
  accent_color = lv_color_make(r,g,b);
  screen_tint  = clamp_dark_tint(r,g,b,8);
  btn_tint     = clamp_mid_tint(r,g,b,6,18);
  lv_style_set_bg_color(&st_screen, screen_tint);
  lv_style_set_border_color(&st_card, accent_color);
  lv_style_set_bg_color(&st_btn, btn_tint);
  lv_style_set_border_color(&st_btn, accent_color);
  lv_style_set_bg_color(&st_btn_pr, accent_color);
  lv_obj_report_style_change(&st_screen);
  lv_obj_report_style_change(&st_card);
  lv_obj_report_style_change(&st_btn);
  lv_obj_report_style_change(&st_btn_pr);
}

void theme_init()
{
  lv_style_init(&st_screen);
  lv_style_set_bg_color(&st_screen, lv_color_hex(0x070A10));
  lv_style_set_bg_opa(&st_screen, LV_OPA_COVER);

  lv_style_init(&st_card);
  lv_style_set_radius(&st_card, 20);
  lv_style_set_bg_color(&st_card, lv_color_hex(0x0F1624));
  lv_style_set_bg_opa(&st_card, LV_OPA_COVER);
  lv_style_set_border_width(&st_card, 1);
  lv_style_set_border_color(&st_card, lv_color_hex(0x23324A));
  lv_style_set_shadow_width(&st_card, 0);
  lv_style_set_shadow_opa(&st_card, LV_OPA_0);
  lv_style_set_pad_all(&st_card, 12);

  lv_style_init(&st_btn);
  lv_style_set_radius(&st_btn, 16);
  lv_style_set_bg_color(&st_btn, lv_color_hex(0x121B2A));
  lv_style_set_bg_opa(&st_btn, LV_OPA_COVER);

  lv_style_init(&st_btn_pr);
  lv_style_set_bg_opa(&st_btn_pr, LV_OPA_60);

  lv_style_init(&st_title);
  lv_style_set_text_color(&st_title, lv_color_hex(0xEAF2FF));

  lv_style_init(&st_hint);
  lv_style_set_text_color(&st_hint, lv_color_hex(0xB3C6E6));

  lv_style_init(&st_splash_big);
  lv_style_set_text_color(&st_splash_big, lv_color_hex(0xFFFFFF));
  lv_style_set_text_font(&st_splash_big, &lv_font_montserrat_24);
  lv_style_set_text_letter_space(&st_splash_big, 2);

  lv_style_init(&st_splash_sub);
  lv_style_set_text_color(&st_splash_sub, lv_color_hex(0xB3C6E6));
  lv_style_set_text_font(&st_splash_sub, &lv_font_montserrat_24);

  apply_theme_from_mood();
}

/*******************************************************
 * UI OBJECTS
 ******************************************************/
static lv_obj_t *scr_splash;
static lv_obj_t *scr_main;
static lv_obj_t *scr_settings;
static lv_obj_t *scr_timer;
static lv_obj_t *lbl_mood;
static lv_obj_t *mood_area;

/*******************************************************
 * UI HELPERS
 ******************************************************/
lv_obj_t* make_btn(lv_obj_t *parent, const char *txt,
                   lv_event_cb_t cb, const char *cmd,
                   int x, int y, int w=200, int h=80)
{
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_add_style(btn, &st_btn, 0);
  lv_obj_add_style(btn, &st_btn_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_pos(btn, x, y);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, (void*)cmd);
  lv_obj_t *lab = lv_label_create(btn);
  lv_label_set_text(lab, txt);
  lv_obj_center(lab);
  return btn;
}

void updateMoodLabel()
{
  if (!lbl_mood) return;
  char line[90];
  snprintf(line, sizeof(line), "Mood: %s", moods[currentMood].name);
  lv_label_set_text(lbl_mood, line);
}

/*******************************************************
 * EVENTS
 ******************************************************/
void go_settings(lv_event_t *e) { lv_screen_load(scr_settings); }
void go_main(lv_event_t *e)     { lv_screen_load(scr_main); }
void go_timer(lv_event_t *e)    { lv_screen_load(scr_timer); }

void splash_timer_cb(lv_timer_t *t)
{
  lv_screen_load(scr_main);
  lv_timer_delete(t);
}

void brightness_slider_cb(lv_event_t *e)
{
  lv_obj_t *slider = (lv_obj_t*)lv_event_get_target(e);
  bl_set((uint8_t)lv_slider_get_value(slider));
}

// Pomodoro button events
void pomo_startpause_cb(lv_event_t *e)
{
  if (pomoPhase == POMO_IDLE)
  {
    pomoPhase    = POMO_WORK;
    pomoSecsLeft = POMO_WORK_SECS;
  }
  pomoRunning = !pomoRunning;
  pomo_update_display();
}

void pomo_reset_cb(lv_event_t *e)
{
  pomoRunning  = false;
  pomoPhase    = POMO_IDLE;
  pomoSecsLeft = POMO_WORK_SECS;
  pomo_update_display();
}

/*******************************************************
 * MOOD DRAG
 ******************************************************/
static bool moodDragging = false;
static int  lastDragX    = 0;
static const int DRAG_STEP_PIXELS = 60;

void mood_drag_event(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_PRESSED)
  {
    moodDragging = true;
    lv_point_t p; lv_indev_get_point(lv_indev_get_act(), &p);
    lastDragX = p.x; return;
  }
  if (code == LV_EVENT_PRESSING && moodDragging)
  {
    lv_point_t p; lv_indev_get_point(lv_indev_get_act(), &p);
    int dx = p.x - lastDragX;
    if (abs(dx) >= DRAG_STEP_PIXELS)
    {
      if (dx>0){currentMood++; if(currentMood>=moodCount)currentMood=0;}
      else     {currentMood--; if(currentMood<0)currentMood=moodCount-1;}
      updateMoodLabel();
      lastDragX = p.x;
    }
    return;
  }
  if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
  {
    moodDragging = false;
    apply_theme_from_mood();
  }
}

/*******************************************************
 * BUILD SPLASH
 ******************************************************/
void build_splash_screen()
{
  scr_splash = lv_obj_create(NULL);
  lv_obj_add_style(scr_splash, &st_screen, 0);

  lv_obj_t *welcome = lv_label_create(scr_splash);
  lv_obj_add_style(welcome, &st_splash_big, 0);
  lv_label_set_text(welcome, "WELCOME\nABDULAZIZ");
  lv_obj_set_width(welcome, 760);
  lv_obj_set_style_text_align(welcome, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(welcome, LV_ALIGN_CENTER, 0, -35);

  lv_obj_t *sub = lv_label_create(scr_splash);
  lv_obj_add_style(sub, &st_splash_sub, 0);
  lv_label_set_text(sub, "NerdCave Control System");
  lv_obj_align(sub, LV_ALIGN_CENTER, 0, 80);
}

/*******************************************************
 * BUILD MAIN
 ******************************************************/
void build_main_screen()
{
  scr_main = lv_obj_create(NULL);
  lv_obj_add_style(scr_main, &st_screen, 0);

  lv_obj_t *hdr = lv_obj_create(scr_main);
  lv_obj_add_style(hdr, &st_card, 0);
  lv_obj_set_size(hdr, 780, 70);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_t *title = lv_label_create(hdr);
  lv_obj_add_style(title, &st_title, 0);
  lv_label_set_text(title, "Desk Control Panel");
  lv_obj_align(title, LV_ALIGN_LEFT_MID, 12, 0);

  lv_obj_t *card = lv_obj_create(scr_main);
  lv_obj_add_style(card, &st_card, 0);
  lv_obj_set_size(card, 780, 280);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 90);

  make_btn(card, "SIT",   nullptr,     "",  20,  20);
  make_btn(card, "STAND", nullptr,     "",  20, 110);
  make_btn(card, "STOP",  nullptr,     "",  20, 200);
  make_btn(card, "M1",    nullptr,     "", 260,  20);
  make_btn(card, "M2",    nullptr,     "", 260, 110);
  make_btn(card, "UP",    nullptr,     "", 520,  20);
  make_btn(card, "DOWN",  nullptr,     "", 520, 110);
  // Two nav buttons side by side
  make_btn(card, "MORE",  go_settings, "", 380, 200, 180, 60);
  make_btn(card, "TIMER", go_timer,    "", 570, 200, 180, 60);

  mood_area = lv_obj_create(scr_main);
  lv_obj_add_style(mood_area, &st_card, 0);
  lv_obj_set_size(mood_area, 610, 90);
  lv_obj_align(mood_area, LV_ALIGN_BOTTOM_LEFT, 10, -10);
  lv_obj_add_flag(mood_area, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(mood_area, mood_drag_event, LV_EVENT_PRESSED,    NULL);
  lv_obj_add_event_cb(mood_area, mood_drag_event, LV_EVENT_PRESSING,   NULL);
  lv_obj_add_event_cb(mood_area, mood_drag_event, LV_EVENT_RELEASED,   NULL);
  lv_obj_add_event_cb(mood_area, mood_drag_event, LV_EVENT_PRESS_LOST, NULL);

  lbl_mood = lv_label_create(mood_area);
  lv_obj_add_style(lbl_mood, &st_title, 0);
  lv_obj_align(lbl_mood, LV_ALIGN_TOP_LEFT, 12, 10);

  lv_obj_t *hint = lv_label_create(mood_area);
  lv_obj_add_style(hint, &st_hint, 0);
  lv_label_set_text(hint, "< drag left/right to change theme >");
  lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 12, -10);

  updateMoodLabel();
}

/*******************************************************
 * BUILD SETTINGS
 ******************************************************/
void build_settings_screen()
{
  scr_settings = lv_obj_create(NULL);
  lv_obj_add_style(scr_settings, &st_screen, 0);

  lv_obj_t *hdr = lv_obj_create(scr_settings);
  lv_obj_add_style(hdr, &st_card, 0);
  lv_obj_set_size(hdr, 780, 90);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_t *title = lv_label_create(hdr);
  lv_obj_add_style(title, &st_title, 0);
  lv_label_set_text(title, "Settings");
  lv_obj_align(title, LV_ALIGN_TOP_LEFT, 12, 10);
  lv_obj_t *sub = lv_label_create(hdr);
  lv_obj_add_style(sub, &st_hint, 0);
  lv_label_set_text(sub, "Display & controls");
  lv_obj_align(sub, LV_ALIGN_BOTTOM_LEFT, 12, -10);

  lv_obj_t *card = lv_obj_create(scr_settings);
  lv_obj_add_style(card, &st_card, 0);
  lv_obj_set_size(card, 780, 330);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 110);

  lv_obj_t *bl_lbl = lv_label_create(card);
  lv_obj_add_style(bl_lbl, &st_title, 0);
  lv_label_set_text(bl_lbl, "Brightness");
  lv_obj_set_pos(bl_lbl, 40, 30);

  lv_obj_t *slider = lv_slider_create(card);
  lv_obj_set_size(slider, 560, 50);
  lv_obj_set_pos(slider, 40, 80);
  lv_slider_set_range(slider, 10, 255);
  lv_slider_set_value(slider, 255, LV_ANIM_OFF);
  lv_obj_add_style(slider, &st_btn, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, accent_color, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, accent_color, LV_PART_KNOB);
  lv_obj_set_style_radius(slider, 8, LV_PART_KNOB);
  lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *lbl_lo = lv_label_create(card);
  lv_obj_add_style(lbl_lo, &st_hint, 0);
  lv_label_set_text(lbl_lo, "Low");
  lv_obj_set_pos(lbl_lo, 40, 145);

  lv_obj_t *lbl_hi = lv_label_create(card);
  lv_obj_add_style(lbl_hi, &st_hint, 0);
  lv_label_set_text(lbl_hi, "High");
  lv_obj_set_pos(lbl_hi, 560, 145);

  make_btn(card, "BACK", go_main, "", 260, 210, 220, 80);
}

/*******************************************************
 * BUILD TIMER SCREEN
 ******************************************************/
void build_timer_screen()
{
  scr_timer = lv_obj_create(NULL);
  lv_obj_add_style(scr_timer, &st_screen, 0);

  // Header
  lv_obj_t *hdr = lv_obj_create(scr_timer);
  lv_obj_add_style(hdr, &st_card, 0);
  lv_obj_set_size(hdr, 780, 70);
  lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_t *title = lv_label_create(hdr);
  lv_obj_add_style(title, &st_title, 0);
  lv_label_set_text(title, "Pomodoro Timer");
  lv_obj_align(title, LV_ALIGN_LEFT_MID, 12, 0);

  // Big card
  lv_obj_t *card = lv_obj_create(scr_timer);
  lv_obj_add_style(card, &st_card, 0);
  lv_obj_set_size(card, 780, 360);
  lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 90);

  // Phase label (WORK / BREAK / READY)
  lbl_phase = lv_label_create(card);
  lv_obj_add_style(lbl_phase, &st_hint, 0);
  lv_obj_set_style_text_font(lbl_phase, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_phase, "READY");
  lv_obj_align(lbl_phase, LV_ALIGN_TOP_MID, 0, 10);

  // Big countdown
  lbl_countdown = lv_label_create(card);
  lv_obj_add_style(lbl_countdown, &st_title, 0);
lv_obj_set_style_text_font(lbl_countdown, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_countdown, "25:00");
  lv_obj_align(lbl_countdown, LV_ALIGN_CENTER, 0, -20);

  // START/PAUSE button
  btn_startpause = lv_btn_create(card);
  lv_obj_add_style(btn_startpause, &st_btn, 0);
  lv_obj_add_style(btn_startpause, &st_btn_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn_startpause, 220, 75);
  lv_obj_align(btn_startpause, LV_ALIGN_BOTTOM_LEFT, 40, -20);
  lv_obj_add_event_cb(btn_startpause, pomo_startpause_cb, LV_EVENT_CLICKED, NULL);
  lbl_sp = lv_label_create(btn_startpause);
  lv_label_set_text(lbl_sp, "START");
  lv_obj_center(lbl_sp);

  // RESET button
  lv_obj_t *btn_reset = lv_btn_create(card);
  lv_obj_add_style(btn_reset, &st_btn, 0);
  lv_obj_add_style(btn_reset, &st_btn_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn_reset, 220, 75);
  lv_obj_align(btn_reset, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_add_event_cb(btn_reset, pomo_reset_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_r = lv_label_create(btn_reset);
  lv_label_set_text(lbl_r, "RESET");
  lv_obj_center(lbl_r);

  // BACK button
  lv_obj_t *btn_back = lv_btn_create(card);
  lv_obj_add_style(btn_back, &st_btn, 0);
  lv_obj_add_style(btn_back, &st_btn_pr, LV_STATE_PRESSED);
  lv_obj_set_size(btn_back, 220, 75);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_RIGHT, -40, -20);
  lv_obj_add_event_cb(btn_back, go_main, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_b = lv_label_create(btn_back);
  lv_label_set_text(lbl_b, "BACK");
  lv_obj_center(lbl_b);

  // LVGL timer — ticks every 1000ms
  pomoTimer = lv_timer_create(pomo_tick_cb, 1000, NULL);
}

/*******************************************************
 * SETUP
 ******************************************************/
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("=== BOOT START ===");

  bl_init();
  Serial.println("Backlight OK");

  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  Wire.setClock(400000);
  Serial.println("Wire OK");

  gfx->begin();
  gfx->fillScreen(0);
  Serial.println("GFX OK");

  tp.begin();
  tp.setRotation(ROTATION_NORMAL);
  Serial.println("Touch OK");

  lv_init();
  Serial.println("LVGL OK");

  buf1 = (lv_color_t*)ps_malloc(800 * 480 * sizeof(lv_color_t));
  if (!buf1) { Serial.println("PSRAM FAILED"); while(1) delay(1000); }
  Serial.println("PSRAM OK");

  lv_draw_buf_init(&draw_buf1, 800, 480, LV_COLOR_FORMAT_RGB565,
                   800*2, buf1, 800*480*sizeof(lv_color_t));

  disp = lv_display_create(800, 480);
  lv_display_set_flush_cb(disp, my_flush_cb);
  lv_display_set_draw_buffers(disp, &draw_buf1, NULL);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touch_read_cb);

  theme_init();
  Serial.println("Theme OK");

  build_splash_screen();
  build_main_screen();
  build_settings_screen();
  build_timer_screen();
  Serial.println("Screens OK");

  lv_screen_load(scr_splash);
  lv_timer_create(splash_timer_cb, 3000, NULL);

  updateMoodLabel();
  apply_theme_from_mood();

  Serial.println("=== READY ===");
}

/*******************************************************
 * LOOP
 ******************************************************/
void loop()
{
  static uint32_t last = millis();
  uint32_t now = millis();
  lv_tick_inc(now - last);
  last = now;
  lv_timer_handler();
  delay(3);
}