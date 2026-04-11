#ifndef SNIFFER_VIEW_H
#define SNIFFER_VIEW_H

#include <lvgl.h>
#include <cstdio>
#include <cstring>
#include <cstdint>

/**
 * SnifferView - LVGL Dashboard per Modbus Sniffer
 *
 * Mostra in tempo reale:
 * - Frame ricevuti dal master
 * - REG, Valore hex, Delay
 * - Statistiche timing
 */

// Copia della struttura SniffedFrame (da main_complete.cpp)
struct SniffedFrame {
  uint32_t timestamp;
  uint8_t addr;
  uint8_t func;
  uint16_t reg;
  uint16_t val;
  uint8_t lrc;
  bool lrc_ok;
  char rawHex[64];
};

class SnifferView {
private:
  lv_obj_t *scr_sniffer = nullptr;
  lv_obj_t *label_title = nullptr;
  lv_obj_t *label_stats = nullptr;
  lv_obj_t *table_frames = nullptr;
  lv_obj_t *label_info = nullptr;
  lv_obj_t *label_ip = nullptr;

  static const int MAX_TABLE_ROWS = 10;
  int displayedRows = 0;

public:
  SnifferView() {}

  void create() {
    // Crea schermata sniffer
    scr_sniffer = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr_sniffer, lv_color_hex(0x1a1a2e), 0);

    // Titolo
    label_title = lv_label_create(scr_sniffer);
    lv_label_set_text(label_title, "📡 RS485 Sniffer");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x00ff00), 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 10, 10);

    // Statistiche (1 riga)
    label_stats = lv_label_create(scr_sniffer);
    lv_label_set_text(label_stats, "Frames: 0 | Min: - | Max: - | Avg: -");
    lv_obj_set_style_text_font(label_stats, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_stats, lv_color_hex(0xffff00), 0);
    lv_obj_align(label_stats, LV_ALIGN_TOP_LEFT, 10, 40);

    // Tabella frame
    table_frames = lv_table_create(scr_sniffer);
    lv_table_set_col_cnt(table_frames, 5);
    lv_table_set_row_cnt(table_frames, MAX_TABLE_ROWS + 1);
    lv_obj_set_size(table_frames, 780, 360);
    lv_obj_align(table_frames, LV_ALIGN_TOP_LEFT, 10, 70);

    // Header
    lv_table_set_cell_value(table_frames, 0, 0, "#");
    lv_table_set_cell_value(table_frames, 0, 1, "REG");
    lv_table_set_cell_value(table_frames, 0, 2, "Hex Value");
    lv_table_set_cell_value(table_frames, 0, 3, "Decimal");
    lv_table_set_cell_value(table_frames, 0, 4, "Delay(ms)");

    for (int i = 0; i <= MAX_TABLE_ROWS; i++) {
      lv_obj_set_style_text_font(table_frames, &lv_font_montserrat_14, LV_PART_ITEMS | (i == 0 ? LV_STATE_DEFAULT : 0));
    }

    // Info footer
    label_info = lv_label_create(scr_sniffer);
    lv_label_set_text(label_info, "Registri: 101=Config | 102=Temp | 103=Mode");
    lv_obj_set_style_text_font(label_info, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_info, lv_color_hex(0x888888), 0);
    lv_obj_align(label_info, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    // IP address (top right)
    label_ip = lv_label_create(scr_sniffer);
    lv_label_set_text(label_ip, "IP: Connecting...");
    lv_obj_set_style_text_font(label_ip, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_ip, lv_color_hex(0x00ff00), 0);
    lv_obj_align(label_ip, LV_ALIGN_TOP_RIGHT, -10, 10);
  }

  void show() {
    if (scr_sniffer) {
      lv_scr_load(scr_sniffer);
    }
  }

  /**
   * Update tabella con frame (call questo ogni loop)
   * @param frames Array di frame
   * @param count Numero frame da mostrare
   * @param stats Statistiche (frame_count, min_delay, max_delay, avg_delay)
   */
  void updateFrames(const SniffedFrame *frames, int count, int totalFrames,
                    uint32_t minDelay, uint32_t maxDelay, uint32_t avgDelay) {
    if (!scr_sniffer) return;

    // Aggiorna statistiche
    static char statBuf[128];
    if (minDelay == 0xFFFFFFFF) {
      snprintf(statBuf, sizeof(statBuf), "Frames: %d | Min: - | Max: - | Avg: -", totalFrames);
    } else {
      snprintf(statBuf, sizeof(statBuf), "Frames: %d | Min: %lums | Max: %lums | Avg: %lums",
               totalFrames, (unsigned long)minDelay, (unsigned long)maxDelay, (unsigned long)avgDelay);
    }
    lv_label_set_text(label_stats, statBuf);

    // Pulisci tabella (meno header)
    for (int i = 1; i <= MAX_TABLE_ROWS; i++) {
      lv_table_set_cell_value(table_frames, i, 0, "");
      lv_table_set_cell_value(table_frames, i, 1, "");
      lv_table_set_cell_value(table_frames, i, 2, "");
      lv_table_set_cell_value(table_frames, i, 3, "");
      lv_table_set_cell_value(table_frames, i, 4, "");
    }

    // Popola tabella (ultimi frame, in ordine)
    int startIdx = (count > MAX_TABLE_ROWS) ? (count - MAX_TABLE_ROWS) : 0;
    int rowNum = 1;

    for (int i = startIdx; i < count && rowNum <= MAX_TABLE_ROWS; i++, rowNum++) {
      const SniffedFrame &f = frames[i];
      int regNum = f.reg - 0x0064;

      static char buf[64];

      // #
      snprintf(buf, sizeof(buf), "%d", totalFrames - count + i + 1);
      lv_table_set_cell_value(table_frames, rowNum, 0, buf);

      // REG
      snprintf(buf, sizeof(buf), "%d", regNum);
      lv_table_set_cell_value(table_frames, rowNum, 1, buf);

      // Hex
      snprintf(buf, sizeof(buf), "0x%04X", f.val);
      lv_table_set_cell_value(table_frames, rowNum, 2, buf);

      // Decimal
      snprintf(buf, sizeof(buf), "%u", f.val);
      lv_table_set_cell_value(table_frames, rowNum, 3, buf);

      // Delay
      if (i == 0) {
        snprintf(buf, sizeof(buf), "-");
      } else {
        uint32_t delay = f.timestamp - frames[i-1].timestamp;
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)delay);
      }
      lv_table_set_cell_value(table_frames, rowNum, 4, buf);
    }

    displayedRows = rowNum - 1;
  }

  void setIP(const char* ip_addr) {
    if (label_ip) {
      static char buf[64];
      snprintf(buf, sizeof(buf), "IP: %s", ip_addr);
      lv_label_set_text(label_ip, buf);
    }
  }

  lv_obj_t* getScreen() { return scr_sniffer; }
};

#endif
