#ifndef VIESSMANN_VIEW_H
#define VIESSMANN_VIEW_H

#include <lvgl.h>
#include "../model/viessmann_model_v2.h"

/**
 * ViessmannView - BELLISSIMA UI funzionante
 *
 * Esattamente come la versione semplice che funziona:
 * - Layout 2 colonne (temperatura sx, controlli dx)
 * - Bottoni grandi e colorati
 * - Aggiornamento in tempo reale
 */

#define COLOR_BG       lv_color_hex(0x1a1a2e)
#define COLOR_CARD     lv_color_hex(0x16213e)
#define COLOR_ACCENT   lv_color_hex(0xe94560)
#define COLOR_COOL     lv_color_hex(0x0f3460)
#define COLOR_INACTIVE lv_color_hex(0x444444)
#define COLOR_TEXT     lv_color_hex(0xeeeeee)
#define COLOR_DIM      lv_color_hex(0xaaaaaa)

// Forward declaration
class ViessmannView;

// Struct per passare dati al fan callback
struct FanCallbackData {
  ViessmannView *view;
  int speed;
};

class ViessmannView {
private:
  ViessmannModel &model;

  // Widget pointers (uguali alla versione semplice che funziona)
  lv_obj_t *label_temp = nullptr;
  lv_obj_t *label_status = nullptr;
  lv_obj_t *label_wifi = nullptr;

  lv_obj_t *btn_on = nullptr;
  lv_obj_t *btn_off = nullptr;
  lv_obj_t *btn_heat = nullptr;
  lv_obj_t *btn_cool = nullptr;
  lv_obj_t *btn_fan[4] = {nullptr, nullptr, nullptr, nullptr};

  // Stili LVGL
  lv_style_t style_card;
  lv_style_t style_btn;
  lv_style_t style_btn_active;

public:
  ViessmannView(ViessmannModel &m) : model(m) {}

  /**
   * Crea l'intera UI LVGL — esattamente come la versione semplice
   */
  void create() {
    initStyles();

    lv_obj_set_style_bg_color(lv_scr_act(), COLOR_BG, 0);

    // Main container — 2 columns (ESATTAMENTE come funziona adesso)
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 780, 460);
    lv_obj_center(main_cont);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_style_pad_all(main_cont, 0, 0);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(main_cont, 15, 0);

    // === LEFT column: Temperature ===
    lv_obj_t *col_left = lv_obj_create(main_cont);
    lv_obj_set_size(col_left, 370, 450);
    lv_obj_add_style(col_left, &style_card, 0);
    lv_obj_set_flex_flow(col_left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col_left, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(col_left, 10, 0);

    // Title
    lv_obj_t *title = lv_label_create(col_left);
    lv_label_set_text(title, "VISLA Ventilconvettore");
    lv_obj_set_style_text_color(title, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

    // Temperature display (GRANDE!)
    label_temp = lv_label_create(col_left);
    lv_label_set_text(label_temp, "20.5°C");
    lv_obj_set_style_text_color(label_temp, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_48, 0);

    // Temp buttons row
    lv_obj_t *temp_row = lv_obj_create(col_left);
    lv_obj_set_size(temp_row, 300, 70);
    lv_obj_set_style_bg_opa(temp_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(temp_row, 0, 0);
    lv_obj_set_style_pad_all(temp_row, 0, 0);
    lv_obj_set_flex_flow(temp_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temp_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Minus button
    lv_obj_t *btn_minus = lv_btn_create(temp_row);
    lv_obj_set_size(btn_minus, 80, 60);
    lv_obj_set_style_radius(btn_minus, 30, 0);
    lv_obj_set_style_bg_color(btn_minus, COLOR_ACCENT, 0);
    lv_obj_add_event_cb(btn_minus, btn_temp_down_callback, LV_EVENT_CLICKED, (void*)this);
    lv_obj_t *lbl_m = lv_label_create(btn_minus);
    lv_label_set_text(lbl_m, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_font(lbl_m, &lv_font_montserrat_30, 0);
    lv_obj_center(lbl_m);

    // Plus button
    lv_obj_t *btn_plus = lv_btn_create(temp_row);
    lv_obj_set_size(btn_plus, 80, 60);
    lv_obj_set_style_radius(btn_plus, 30, 0);
    lv_obj_set_style_bg_color(btn_plus, COLOR_ACCENT, 0);
    lv_obj_add_event_cb(btn_plus, btn_temp_up_callback, LV_EVENT_CLICKED, (void*)this);
    lv_obj_t *lbl_p = lv_label_create(btn_plus);
    lv_label_set_text(lbl_p, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_font(lbl_p, &lv_font_montserrat_30, 0);
    lv_obj_center(lbl_p);

    // WiFi status
    label_wifi = lv_label_create(col_left);
    lv_label_set_text(label_wifi, "WiFi: ...");
    lv_obj_set_style_text_color(label_wifi, COLOR_DIM, 0);
    lv_obj_set_style_text_font(label_wifi, &lv_font_montserrat_14, 0);

    // Status
    label_status = lv_label_create(col_left);
    lv_label_set_text(label_status, "");
    lv_obj_set_style_text_color(label_status, COLOR_DIM, 0);
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_12, 0);

    // === RIGHT column: Controls ===
    lv_obj_t *col_right = lv_obj_create(main_cont);
    lv_obj_set_size(col_right, 370, 450);
    lv_obj_set_style_bg_opa(col_right, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col_right, 0, 0);
    lv_obj_set_style_pad_all(col_right, 0, 0);
    lv_obj_set_flex_flow(col_right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col_right, 10, 0);

    // --- Power card ---
    lv_obj_t *card_power = lv_obj_create(col_right);
    lv_obj_set_size(card_power, 370, LV_SIZE_CONTENT);
    lv_obj_add_style(card_power, &style_card, 0);
    lv_obj_set_flex_flow(card_power, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card_power, 8, 0);

    lv_obj_t *lbl_pwr = lv_label_create(card_power);
    lv_label_set_text(lbl_pwr, "Alimentazione");
    lv_obj_set_style_text_color(lbl_pwr, COLOR_DIM, 0);

    lv_obj_t *pwr_row = lv_obj_create(card_power);
    lv_obj_set_size(pwr_row, 330, 55);
    lv_obj_set_style_bg_opa(pwr_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pwr_row, 0, 0);
    lv_obj_set_style_pad_all(pwr_row, 0, 0);
    lv_obj_set_flex_flow(pwr_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(pwr_row, 8, 0);

    btn_on = makeBtn(pwr_row, "ACCESO", btn_power_on_callback);
    btn_off = makeBtn(pwr_row, "SPENTO", btn_power_off_callback);

    // --- Mode card ---
    lv_obj_t *card_mode = lv_obj_create(col_right);
    lv_obj_set_size(card_mode, 370, LV_SIZE_CONTENT);
    lv_obj_add_style(card_mode, &style_card, 0);
    lv_obj_set_flex_flow(card_mode, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card_mode, 8, 0);

    lv_obj_t *lbl_mode = lv_label_create(card_mode);
    lv_label_set_text(lbl_mode, "Stagione");
    lv_obj_set_style_text_color(lbl_mode, COLOR_DIM, 0);

    lv_obj_t *mode_row = lv_obj_create(card_mode);
    lv_obj_set_size(mode_row, 330, 55);
    lv_obj_set_style_bg_opa(mode_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(mode_row, 0, 0);
    lv_obj_set_style_pad_all(mode_row, 0, 0);
    lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(mode_row, 8, 0);

    btn_heat = makeBtn(mode_row, "CALDO", btn_heat_callback);
    btn_cool = makeBtn(mode_row, "FREDDO", btn_cool_callback);

    // --- Fan card ---
    lv_obj_t *card_fan = lv_obj_create(col_right);
    lv_obj_set_size(card_fan, 370, LV_SIZE_CONTENT);
    lv_obj_add_style(card_fan, &style_card, 0);
    lv_obj_set_flex_flow(card_fan, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card_fan, 8, 0);

    lv_obj_t *lbl_fan = lv_label_create(card_fan);
    lv_label_set_text(lbl_fan, "Ventola");
    lv_obj_set_style_text_color(lbl_fan, COLOR_DIM, 0);

    lv_obj_t *fan_row = lv_obj_create(card_fan);
    lv_obj_set_size(fan_row, 330, 55);
    lv_obj_set_style_bg_opa(fan_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(fan_row, 0, 0);
    lv_obj_set_style_pad_all(fan_row, 0, 0);
    lv_obj_set_flex_flow(fan_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(fan_row, 5, 0);

    const char* fan_labels[] = {"AUTO", "MIN", "NIGHT", "MAX"};
    for (int i = 0; i < 4; i++) {
      FanCallbackData *data = new FanCallbackData{this, i};
      btn_fan[i] = makeBtn(fan_row, fan_labels[i], btn_fan_callback_static, (void*)data);
    }

    refreshDisplay();
  }

  /**
   * Aggiorna il display (bottoni, temperatura, stato)
   * Chiamato sempre dentro un contesto LVGL, quindi NO lock
   */
  void refreshDisplay() {
    // Temperatura
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f°C", model.getTemperature());
    lv_label_set_text(label_temp, buf);

    // Power buttons
    lv_obj_set_style_bg_color(btn_on, model.isPowerOn() ? COLOR_ACCENT : COLOR_INACTIVE, 0);
    lv_obj_set_style_bg_color(btn_off, !model.isPowerOn() ? COLOR_ACCENT : COLOR_INACTIVE, 0);

    // Mode buttons
    lv_obj_set_style_bg_color(btn_heat, model.isHeating() ? COLOR_ACCENT : COLOR_INACTIVE, 0);
    lv_obj_set_style_bg_color(btn_cool, !model.isHeating() ? COLOR_COOL : COLOR_INACTIVE, 0);

    // Fan buttons
    int fs = (int)model.getFanSpeed();
    for (int i = 0; i < 4; i++) {
      lv_obj_set_style_bg_color(btn_fan[i], (i == fs) ? COLOR_ACCENT : COLOR_INACTIVE, 0);
    }

    // Status
    snprintf(buf, sizeof(buf), "R101=0x%04X", model.getRegConfig());
    lv_label_set_text(label_status, buf);
  }

private:
  void initStyles() {
    // Card style
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_border_width(&style_card, 0);
    lv_style_set_pad_all(&style_card, 15);
    lv_style_set_radius(&style_card, 12);

    // Button style
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COLOR_ACCENT);
    lv_style_set_text_color(&style_btn, COLOR_TEXT);
    lv_style_set_border_width(&style_btn, 0);
    lv_style_set_radius(&style_btn, 8);
    lv_style_set_pad_all(&style_btn, 10);
  }

  lv_obj_t* makeBtn(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data = nullptr) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_height(btn, 50);
    if (cb) {
      // Usa user_data personalizzato se fornito, altrimenti usa `this`
      void *cb_user_data = user_data ? user_data : (void*)this;
      lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, cb_user_data);
    }

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);
    return btn;
  }

  // === CALLBACKS (ESATTAMENTE come nella versione semplice che funziona) ===

  static void btn_fan_callback_static(lv_event_t *e) {
    FanCallbackData *data = (FanCallbackData*)lv_event_get_user_data(e);
    data->view->model.setFanSpeed((ViessmannModel::FanSpeed)data->speed);
    data->view->refreshDisplay();
  }

  static void btn_temp_up_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.increaseTemperature();
    view->refreshDisplay();
  }

  static void btn_temp_down_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.decreaseTemperature();
    view->refreshDisplay();
  }

  static void btn_power_on_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.setPower(true);
    view->refreshDisplay();
  }

  static void btn_power_off_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.setPower(false);
    view->refreshDisplay();
  }

  static void btn_heat_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.setHeating(true);
    view->refreshDisplay();
  }

  static void btn_cool_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView*)lv_event_get_user_data(e);
    view->model.setHeating(false);
    view->refreshDisplay();
  }
};

#endif
