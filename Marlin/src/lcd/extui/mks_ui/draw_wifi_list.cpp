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

#define NAME_BTN_X 187//330
#define NAME_BTN_Y 21//48

#define MARK_BTN_X 0
#define MARK_BTN_Y 68

WIFI_LIST wifi_list;
list_menu_def list_menu;

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *buttonWifiN[NUMBER_OF_PAGE];
static lv_obj_t *labelWifiText[NUMBER_OF_PAGE];
static lv_obj_t *labelPageText;
static lv_obj_t *imgtop, *labelname;

#define ID_WL_RETURN 11
#define ID_WL_DOWN   12
#define ID_WL_UP     13

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  if (obj->mks_obj_id == ID_WL_RETURN) {
    clear_cur_ui();
    lv_draw_tool();
    // lv_draw_set();
  }
  else if (obj->mks_obj_id == ID_WL_DOWN) {
    if (wifi_list.getNameNum > 0) {
      if ((wifi_list.nameIndex + NUMBER_OF_PAGE) >= wifi_list.getNameNum) {
        wifi_list.nameIndex = 0;
        wifi_list.currentWifipage = 1;
      }
      else {
        wifi_list.nameIndex += NUMBER_OF_PAGE;
        wifi_list.currentWifipage++;
      }
      disp_wifi_list();
    }
  }
  else if (obj->mks_obj_id == ID_WL_UP) {
    if (wifi_list.getNameNum > 0) {
      if ((wifi_list.nameIndex - NUMBER_OF_PAGE) >= 0) {
        wifi_list.nameIndex = 0;
        wifi_list.currentWifipage = 1;
      }
      else {
        wifi_list.nameIndex -= NUMBER_OF_PAGE;
        wifi_list.currentWifipage--;
      }
      disp_wifi_list();
    }
  }
  else {
    for (uint8_t i = 0; i < NUMBER_OF_PAGE; i++) {
      if (obj->mks_obj_id == i + 1) {
        if (wifi_list.getNameNum != 0) {
          const bool do_wifi = wifi_link_state == WIFI_CONNECTED && strcmp((const char *)wifi_list.wifiConnectedName, (const char *)wifi_list.wifiName[wifi_list.nameIndex + i]) == 0;
          wifi_list.nameIndex += i;
          last_disp_state = WIFI_LIST_UI;
          lv_clear_wifi_list();
          if (do_wifi)
            lv_draw_wifi();
          else {
            keyboard_value = wifiConfig;
            lv_draw_keyboard();
          }
        }
      }
    }
  }
}

void lv_draw_wifi_list() {
  scr = lv_screen_create(WIFI_LIST_UI);
  
  imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 50);
  lv_obj_set_pos(imgtop, 0, 0);
  
  labelname = lv_label_create_empty(imgtop);
  lv_label_set_text(labelname,"WIFI");
  lv_obj_align(labelname, imgtop, LV_ALIGN_IN_LEFT_MID, 0, 0);

  lv_obj_t *buttonUp = lv_imgbtn_create(scr, "F:/bmp_pageUp.bin", 5, 251, event_handler, ID_WL_DOWN);
  // lv_imgbtn_set_src(buttonUp, LV_BTN_STATE_REL, "F:/bmp_pageUp.bin");
  // lv_imgbtn_set_src(buttonUp, LV_BTN_STATE_PR, "F:/bmp_pageDown.bin");
  lv_obj_t *buttonDown = lv_imgbtn_create(scr, "F:/bmp_pageDown.bin", 176, 251, event_handler, ID_WL_DOWN);
  lv_obj_t *buttonBack = lv_imgbtn_create(scr, "F:/bmp_wifi_return.bin", 345, 250, event_handler, ID_WL_RETURN);

  for (uint8_t i = 0 , n = 0; i < NUMBER_OF_PAGE; i++) {
    if(i%2 == 0){
      buttonWifiN[i] = lv_label_btn_create(scr, 19, 80 + 50 * n, NAME_BTN_X, NAME_BTN_Y, event_handler, i + 1);//lv_label_btn_create
      lv_btn_set_style(buttonWifiN[i], LV_BTN_STYLE_REL, &tft_style_preHeat_label);
      lv_btn_set_style(buttonWifiN[i], LV_BTN_STYLE_PR,  &tft_style_preHeat_label);
      // lv_obj_set_style(buttonWifiN[i], &tft_style_wifi_WHITE);
    }
    else{
      buttonWifiN[i] = lv_label_btn_create(scr, 275, 80 + 50 * n, NAME_BTN_X, NAME_BTN_Y, event_handler, i + 1);//NAME_BTN_Y * (i - 1) + 10 + titleHeight
      lv_btn_set_style(buttonWifiN[i], LV_BTN_STYLE_REL, &tft_style_preHeat_label);
      lv_btn_set_style(buttonWifiN[i], LV_BTN_STYLE_PR,  &tft_style_preHeat_label);
      // lv_obj_set_style(buttonWifiN[i], &tft_style_wifi_WHITE);
      n++;
    }
    labelWifiText[i] = lv_label_create_empty(buttonWifiN[i]);
    // lv_obj_set_style(buttonWifiN[i], &tft_style_preHeat_scr);
    // lv_obj_set_style(labelWifiText[i], &tft_style_preHeat_label);
    
    // #if HAS_ROTARY_ENCODER
    //   uint8_t j = 0;
    //   if (gCfgItems.encoder_enable) {
    //     j = wifi_list.nameIndex + i;
    //     if (j < wifi_list.getNameNum) lv_group_add_obj(g, buttonWifiN[i]);
    //   }
    // #endif
  }

  // labelPageText = lv_label_create_empty(scr);
  // lv_obj_set_style(labelPageText, &tft_style_label_rel);

  wifi_list.nameIndex = 0;
  wifi_list.currentWifipage = 1;

  if (wifi_link_state == WIFI_CONNECTED && wifiPara.mode == STA_MODEL) {
    ZERO(wifi_list.wifiConnectedName);
    memcpy(wifi_list.wifiConnectedName, wifiPara.ap_name, sizeof(wifi_list.wifiConnectedName));
  }
/*
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) {
      lv_group_add_obj(g, buttonUp);
      lv_group_add_obj(g, buttonDown);
      lv_group_add_obj(g, buttonBack);
    }
  #else
    UNUSED(buttonDown);
    UNUSED(buttonBack);
  #endif
*/
  disp_wifi_list();

  // lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_img_set_src(imgtop, "F:/bmp_preHeat_top.bin");
  // lv_label_set_text(labelname,"WIFI");
}

void disp_wifi_list() {
  int8_t tmpStr[WIFI_NAME_BUFFER_SIZE] = { 0 };
  uint8_t i, j;

  sprintf((char *)tmpStr, list_menu.file_pages, wifi_list.currentWifipage, wifi_list.getPage);
  // lv_label_set_text(labelPageText, (const char *)tmpStr);
  // lv_obj_align(labelPageText, nullptr, LV_ALIGN_CENTER, 0, 0);//50 -100

  for (i = 0; i < NUMBER_OF_PAGE; i++) {
    ZERO(tmpStr);

    j = wifi_list.nameIndex + i;
    if (j >= wifi_list.getNameNum) {
      lv_label_set_text(labelWifiText[i], (const char *)tmpStr);
      lv_obj_align(labelWifiText[i], buttonWifiN[i], LV_ALIGN_IN_LEFT_MID, 20, 0);
    }
    else {
      lv_label_set_text(labelWifiText[i], (char const *)wifi_list.wifiName[j]);
      lv_obj_align(labelWifiText[i], buttonWifiN[i], LV_ALIGN_IN_LEFT_MID, 20, 0);

      const bool btext = (wifi_link_state == WIFI_CONNECTED && strcmp((const char *)wifi_list.wifiConnectedName, (const char *)wifi_list.wifiName[j]) == 0);
      // lv_btn_set_style(buttonWifiN[i], LV_BTN_STYLE_REL, btext ? &style_sel_text : &tft_style_label_rel);
    }
  }
}

void wifi_scan_handle() {
  if (!DIALOG_IS(WIFI_ENABLE_TIPS) || !uiCfg.command_send) return;
  last_disp_state = DIALOG_UI;
  lv_clear_dialog();
  if (wifi_link_state == WIFI_CONNECTED && wifiPara.mode != AP_MODEL)
    lv_draw_wifi();
  else
    lv_draw_wifi_list();
}

void lv_clear_wifi_list() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // MKS_WIFI_MODULE
#endif // HAS_TFT_LVGL_UI
