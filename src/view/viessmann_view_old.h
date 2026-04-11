#ifndef VIESSMANN_VIEW_H
#define VIESSMANN_VIEW_H

#include <lvgl.h>
#include "../viewmodel/viessmann_viewmodel.h"

/**
 * ViessmannView - UI Layer (MVVM Pattern)
 *
 * Responsabilità:
 * - Creare widget LVGL
 * - Gestire event callbacks (ma forwarda a ViewModel)
 * - Aggiornare display quando ViewModel notifica
 *
 * NON contiene logica di business.
 * NON contiene trasformazione dati.
 * È puramente grafica.
 */

#define COLOR_BG lv_color_hex(0x1a1a2e)
#define COLOR_CARD lv_color_hex(0x16213e)
#define COLOR_ACCENT lv_color_hex(0xe94560)
#define COLOR_COOL lv_color_hex(0x0f3460)
#define COLOR_HEAT lv_color_hex(0xff4444)
#define COLOR_INACTIVE lv_color_hex(0x444444)
#define COLOR_TEXT lv_color_hex(0xeeeeee)
#define COLOR_DIM lv_color_hex(0xaaaaaa)

class ViessmannView {
private:
  ViessmannViewModel &viewModel;

  // Widget pointers
  lv_obj_t *label_temp = nullptr;
  lv_obj_t *label_status = nullptr;
  lv_obj_t *label_fan = nullptr;
  lv_obj_t *label_power = nullptr;

  lv_obj_t *btn_temp_up = nullptr;
  lv_obj_t *btn_temp_down = nullptr;
  lv_obj_t *btn_power_on = nullptr;
  lv_obj_t *btn_power_off = nullptr;
  lv_obj_t *btn_heat = nullptr;
  lv_obj_t *btn_cool = nullptr;
  lv_obj_t *btn_fan[4] = {nullptr, nullptr, nullptr, nullptr};

  // Stili
  lv_style_t style_card;
  lv_style_t style_btn;
  lv_style_t style_btn_active;
  lv_style_t style_btn_heat;
  lv_style_t style_btn_cool;

public:
  ViessmannView(ViessmannViewModel &vm) : viewModel(vm) {
    // Registra callback per aggiornamenti
    viewModel.onDisplayUpdate = [this]() {
      refreshDisplay();
    };
  }

  /**
   * Crea tutta l'interfaccia LVGL
   */
  void create() {
    // Inizializza stili
    initStyles();

    // Crea schermata principale
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);

    // Temperatura grande (al centro)
    label_temp = lv_label_create(scr);
    lv_label_set_text(label_temp, viewModel.getDisplayTemp());
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_temp, COLOR_TEXT, 0);
    lv_obj_align(label_temp, LV_ALIGN_TOP_MID, 0, 50);

    // Status (FREDDO/CALDO)
    label_status = lv_label_create(scr);
    lv_label_set_text(label_status, viewModel.getDisplayStatus());
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_26, 0);
    lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 120);

    // ===== CONTROLLI TEMPERATURA =====
    lv_obj_t *container_temp = lv_obj_create(scr);
    lv_obj_set_size(container_temp, 400, 80);
    lv_obj_align(container_temp, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_add_style(container_temp, &style_card, 0);

    btn_temp_down = lv_btn_create(container_temp);
    lv_obj_set_size(btn_temp_down, 80, 60);
    lv_obj_align(btn_temp_down, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_style(btn_temp_down, &style_btn, 0);
    lv_obj_set_user_data(btn_temp_down, this);
    lv_obj_add_event_cb(btn_temp_down, btn_temp_down_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_down = lv_label_create(btn_temp_down);
    lv_label_set_text(label_down, "-");
    lv_obj_center(label_down);

    btn_temp_up = lv_btn_create(container_temp);
    lv_obj_set_size(btn_temp_up, 80, 60);
    lv_obj_align(btn_temp_up, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_style(btn_temp_up, &style_btn, 0);
    lv_obj_set_user_data(btn_temp_up, this);
    lv_obj_add_event_cb(btn_temp_up, btn_temp_up_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_up = lv_label_create(btn_temp_up);
    lv_label_set_text(label_up, "+");
    lv_obj_center(label_up);

    // ===== CONTROLLI POTENZA =====
    lv_obj_t *container_power = lv_obj_create(scr);
    lv_obj_set_size(container_power, 400, 80);
    lv_obj_align(container_power, LV_ALIGN_TOP_MID, 0, 260);
    lv_obj_add_style(container_power, &style_card, 0);

    btn_power_on = lv_btn_create(container_power);
    lv_obj_set_size(btn_power_on, 150, 60);
    lv_obj_align(btn_power_on, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_style(btn_power_on, &style_btn, 0);
    lv_obj_set_user_data(btn_power_on, this);
    lv_obj_add_event_cb(btn_power_on, btn_power_on_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_on = lv_label_create(btn_power_on);
    lv_label_set_text(label_on, "🔌 ON");
    lv_obj_center(label_on);

    btn_power_off = lv_btn_create(container_power);
    lv_obj_set_size(btn_power_off, 150, 60);
    lv_obj_align(btn_power_off, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_style(btn_power_off, &style_btn, 0);
    lv_obj_set_user_data(btn_power_off, this);
    lv_obj_add_event_cb(btn_power_off, btn_power_off_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_off = lv_label_create(btn_power_off);
    lv_label_set_text(label_off, "⏻ OFF");
    lv_obj_center(label_off);

    // ===== CONTROLLI MODALITÀ =====
    lv_obj_t *container_mode = lv_obj_create(scr);
    lv_obj_set_size(container_mode, 400, 80);
    lv_obj_align(container_mode, LV_ALIGN_TOP_MID, 0, 360);
    lv_obj_add_style(container_mode, &style_card, 0);

    btn_cool = lv_btn_create(container_mode);
    lv_obj_set_size(btn_cool, 150, 60);
    lv_obj_align(btn_cool, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_style(btn_cool, &style_btn_cool, 0);
    lv_obj_set_user_data(btn_cool, this);
    lv_obj_add_event_cb(btn_cool, btn_cool_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_cool = lv_label_create(btn_cool);
    lv_label_set_text(label_cool, "❄️ COOL");
    lv_obj_center(label_cool);

    btn_heat = lv_btn_create(container_mode);
    lv_obj_set_size(btn_heat, 150, 60);
    lv_obj_align(btn_heat, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_style(btn_heat, &style_btn_heat, 0);
    lv_obj_set_user_data(btn_heat, this);
    lv_obj_add_event_cb(btn_heat, btn_heat_callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_heat = lv_label_create(btn_heat);
    lv_label_set_text(label_heat, "🔥 HEAT");
    lv_obj_center(label_heat);

    // Aggiorna display iniziale
    refreshDisplay();

    Serial.println("[VIEW] Interfaccia LVGL creata");
  }

  /**
   * Aggiorna i widget con i dati dal ViewModel
   * Chiamato quando ViewModel notifica onDisplayUpdate
   */
  void refreshDisplay() {
    if (!label_temp) return;  // UI non ancora creata

    // Aggiorna temperatura
    lv_label_set_text(label_temp, viewModel.getDisplayTemp());

    // Aggiorna status
    lv_label_set_text(label_status, viewModel.getDisplayStatus());

    // Aggiorna colore bottoni a seconda dello stato
    if (viewModel.isPowerOn()) {
      lv_obj_add_state(btn_power_on, LV_STATE_DISABLED);
      lv_obj_clear_state(btn_power_off, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_power_on, LV_STATE_DISABLED);
      lv_obj_add_state(btn_power_off, LV_STATE_DISABLED);
    }

    if (viewModel.isHeating()) {
      lv_obj_add_state(btn_heat, LV_STATE_DISABLED);
      lv_obj_clear_state(btn_cool, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_state(btn_heat, LV_STATE_DISABLED);
      lv_obj_add_state(btn_cool, LV_STATE_DISABLED);
    }

    Serial.printf("[VIEW] Display aggiornato: %s\n", viewModel.getDisplayTemp());
  }

private:
  void initStyles() {
    // Card background
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_pad_all(&style_card, 15);
    lv_style_set_border_width(&style_card, 0);

    // Button default
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COLOR_INACTIVE);
    lv_style_set_radius(&style_btn, 8);
    lv_style_set_text_color(&style_btn, lv_color_white());
    lv_style_set_border_width(&style_btn, 0);
    lv_style_set_pad_ver(&style_btn, 14);

    // Button active
    lv_style_init(&style_btn_active);
    lv_style_set_bg_color(&style_btn_active, COLOR_ACCENT);

    // Button heat
    lv_style_init(&style_btn_heat);
    lv_style_set_bg_color(&style_btn_heat, COLOR_HEAT);

    // Button cool
    lv_style_init(&style_btn_cool);
    lv_style_set_bg_color(&style_btn_cool, COLOR_COOL);
  }

  // ========== LVGL CALLBACKS (solo forwarding a ViewModel) ==========

  static void btn_temp_up_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onTemperatureUp();
  }

  static void btn_temp_down_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onTemperatureDown();
  }

  static void btn_power_on_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onPowerOn();
  }

  static void btn_power_off_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onPowerOff();
  }

  static void btn_heat_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onHeatingToggle();
  }

  static void btn_cool_callback(lv_event_t *e) {
    ViessmannView *view = (ViessmannView *)lv_obj_get_user_data(e->target);
    view->viewModel.onCoolingToggle();
  }
};

#endif
