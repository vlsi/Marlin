/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include <lv_conf.h>
#include "tft_lvgl_configuration.h"

#if ENABLED(MKS_WIFI_MODULE)

#include "draw_ui.h"

extern lv_group_t *g;
static lv_obj_t *scr, *wifi_name_text, *wifi_key_text, *wifi_state_text, *wifi_ip_text, *wifi_tips_text;
static lv_obj_t *imgtop, *labelname;

enum {
  ID_W_RETURN = 1,
  ID_W_CLOUD,
  ID_W_RECONNECT
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  clear_cur_ui();
  switch (obj->mks_obj_id) {
    case ID_W_RETURN:
      // lv_draw_set();
      lv_draw_tool();
      break;
    case ID_W_CLOUD:
      lv_draw_cloud_bind();
      break;
    #if ENABLED(MKS_WIFI_MODULE)
      case ID_W_RECONNECT: {
        uint8_t cmd_wifi_list[] = { 0xA5, 0x07, 0x00, 0x00, 0xFC };
        raw_send_to_wifi(cmd_wifi_list, COUNT(cmd_wifi_list));
        lv_draw_wifi_list();
      } break;
    #endif
  }
}

void lv_draw_wifi() {
  scr = lv_screen_create(WIFI_UI);

  lv_obj_t *buttonReconnect = nullptr, *label_Reconnect = nullptr;
  lv_obj_t *buttonCloud = nullptr, *label_Cloud = nullptr;

  const bool enc_ena = TERN0(HAS_ROTARY_ENCODER, gCfgItems.encoder_enable);

  if (gCfgItems.wifi_mode_sel == STA_MODEL) {
    imgtop = lv_obj_create(scr, nullptr);
    lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
    lv_obj_set_size(imgtop, 480, 50);
    lv_obj_set_pos(imgtop, 0, 0);
    labelname = lv_label_create_empty(imgtop);
    lv_label_set_text(labelname,"WIFI");
    lv_label_set_style(labelname, LV_LABEL_STYLE_MAIN, &label_dialog_white);
    lv_obj_align(labelname, imgtop, LV_ALIGN_IN_LEFT_MID, 10, 0);

    if (gCfgItems.cloud_enable)
      buttonCloud = lv_imgbtn_create(scr, "F:/bmp_cloud.bin", BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_W_CLOUD);

    buttonReconnect = lv_button_btn_create(scr, 153, 250, 115, 57, event_handler, ID_W_RECONNECT);//40
    lv_btn_set_style(buttonReconnect, LV_BTN_STYLE_REL, &tft_style_button_BLUE);
    lv_btn_set_style(buttonReconnect, LV_BTN_STYLE_PR,  &tft_style_button_BLUE);

    #if HAS_ROTARY_ENCODER
      if (gCfgItems.cloud_enable) lv_group_add_obj(g, buttonCloud);
      // if (enc_ena) lv_group_add_obj(g, buttonReconnect);
    #endif

    label_Reconnect = lv_label_create_empty(buttonReconnect);
    if (gCfgItems.cloud_enable) label_Cloud = lv_label_create_empty(buttonCloud);
  }

  // Create an Image button
  lv_obj_t *buttonBack = lv_imgbtn_create(scr, "F:/bmp_wifi_return.bin", 335, 249, event_handler, ID_W_RETURN);
  lv_obj_align(buttonReconnect, buttonBack, LV_ALIGN_OUT_LEFT_TOP, -20, 3);

  if (gCfgItems.multiple_language) {
    if (gCfgItems.wifi_mode_sel == STA_MODEL) {
      if (gCfgItems.cloud_enable) {
        lv_label_set_text(label_Cloud, wifi_menu.cloud);
        lv_obj_align(label_Cloud, buttonCloud, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
      }
      lv_label_set_text(label_Reconnect, wifi_menu.reconnect);
      lv_obj_set_style(label_Reconnect, &label_dialog_white);
      lv_obj_align(label_Reconnect, buttonReconnect, LV_ALIGN_CENTER, 0, 0);
    }
  }

  wifi_tips_text = lv_label_create_empty(scr);
  lv_label_set_text(wifi_tips_text, wifi_menu.wifi_connect);
  lv_obj_set_style(wifi_tips_text, &tft_style_preHeat_label_BLACK);
  lv_obj_set_pos(wifi_tips_text, 9, 52);
  
  wifi_ip_text = lv_label_create_empty(scr);
  lv_obj_set_style(wifi_ip_text, &tft_style_preHeat_label_BLACK);
  lv_obj_set_pos(wifi_ip_text, 9, 120);
  wifi_name_text = lv_label_create_empty(scr);
  lv_obj_set_style(wifi_name_text, &tft_style_preHeat_label_BLACK);
  lv_obj_set_pos(wifi_name_text, 9, 157);
  wifi_key_text = lv_label_create_empty(scr);
  lv_obj_set_style(wifi_key_text, &tft_style_preHeat_label_BLACK);
  wifi_state_text = lv_label_create_empty(scr);
  lv_obj_set_style(wifi_state_text, &tft_style_preHeat_label_BLACK);

  disp_wifi_state();
}

void disp_wifi_state() {
  strcpy(public_buf_m, wifi_menu.ip);
  strcat(public_buf_m, ipPara.ip_addr);
  lv_label_set_text(wifi_ip_text, public_buf_m);

  strcpy(public_buf_m, wifi_menu.wifi);
  strcat(public_buf_m, wifiPara.ap_name);
  lv_label_set_text(wifi_name_text, public_buf_m);

  if (wifiPara.mode == AP_MODEL) {
    strcpy(public_buf_m, wifi_menu.key);
    strcat(public_buf_m, wifiPara.keyCode);
    lv_label_set_text(wifi_key_text, public_buf_m);
    lv_obj_set_pos(wifi_key_text, 9, 190);

    strcpy(public_buf_m, wifi_menu.state_ap);
    if (wifi_link_state == WIFI_CONNECTED)
      strcat(public_buf_m, wifi_menu.connected);
    else if (wifi_link_state == WIFI_NOT_CONFIG)
      strcat(public_buf_m, wifi_menu.disconnected);
    else
      strcat(public_buf_m, wifi_menu.exception);
    lv_label_set_text(wifi_state_text, public_buf_m);
    lv_obj_set_pos(wifi_state_text, 9, 222);
  }
  else {
    strcpy(public_buf_m, wifi_menu.state_sta);
    if (wifi_link_state == WIFI_CONNECTED)
      strcat(public_buf_m, wifi_menu.connected);
    else if (wifi_link_state == WIFI_NOT_CONFIG)
      strcat(public_buf_m, wifi_menu.disconnected);
    else
      strcat(public_buf_m, wifi_menu.exception);
    lv_label_set_text(wifi_state_text, public_buf_m);
    lv_obj_set_pos(wifi_state_text, 9, 190);

    lv_label_set_text(wifi_key_text, "");
    lv_obj_set_pos(wifi_key_text, 9, 222);
  }
}

void lv_clear_wifi() {
  if (TERN0(HAS_ROTARY_ENCODER, gCfgItems.encoder_enable))
    lv_group_remove_all_objs(g);
  lv_obj_del(scr);
}

#endif // MKS_WIFI_MODULE
#endif // HAS_TFT_LVGL_UI
