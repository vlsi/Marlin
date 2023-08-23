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

#include "draw_ui.h"
#include <lv_conf.h>

#include "../../../inc/MarlinConfig.h"
#include <string.h>

enum {
  ID_CN = 1,
  ID_T_CN,
  ID_EN,
  ID_RU,
  ID_ES,
  ID_FR,
  ID_IT,
  ID_L_RETURN,
  ID_L_NULL,
};

#define SELECTED    1
#define UNSELECTED  0

// static void disp_language(uint8_t language, uint8_t state);

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *buttonCN, *buttonT_CN, *buttonEN, *buttonRU;
static lv_obj_t *buttonES, *buttonFR, *buttonIT;
static lv_obj_t *imgset;
static lv_obj_t *labelCN, *labelT_CN, *labelEN, *labelRU, *label_ES, *labelFR, *labelIT; 

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW); 

  switch (obj->mks_obj_id) {
    case ID_CN:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonCN, "F:/bmp_simplified_cn_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonCN);
      gCfgItems.language = LANG_SIMPLE_CHINESE;
      disp_language_init();
      update_spi_flash();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_T_CN:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonT_CN, "F:/bmp_traditional_cn_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonT_CN);
      gCfgItems.language = LANG_COMPLEX_CHINESE;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_EN:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonEN, "F:/bmp_english_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonEN);
      gCfgItems.language = LANG_ENGLISH;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_RU:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonRU, "F:/bmp_russian_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonRU);
      gCfgItems.language = LANG_RUSSIAN;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_ES:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonES, "F:/bmp_spanish_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonES);
      gCfgItems.language = LANG_SPANISH;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_FR:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonFR, "F:/bmp_french_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonFR);
      gCfgItems.language = LANG_FRENCH;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_IT:
      disp_language(gCfgItems.language, UNSELECTED);
      // lv_imgbtn_set_src_both(buttonIT, "F:/bmp_italy_sel.bin");
      // lv_obj_refresh_ext_draw_pad(buttonIT);
      gCfgItems.language = LANG_ITALY;
      update_spi_flash();
      disp_language_init();
      // lv_clear_language();
      // lv_draw_language();
      break;
    case ID_L_RETURN:
      buttonCN   = nullptr;
      buttonT_CN = nullptr;
      buttonEN   = nullptr;
      buttonRU   = nullptr;
      buttonES   = nullptr;
      buttonFR   = nullptr;
      buttonFR   = nullptr;
      buttonIT   = nullptr;
      // lv_clear_language();
      // lv_draw_set();
      clear_cur_ui();
      draw_return_ui();
      break;
    case ID_L_NULL:
      break;
    default: 
      break;
  }
}

void disp_language(uint8_t language, uint8_t state) {
  uint16_t id;
  lv_obj_t *obj;

  public_buf_l[0] = '\0';

  switch (language) {
    case LANG_SIMPLE_CHINESE:
      id = ID_CN;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_simplified_cn"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_r.bin");
      lv_obj_align(imgset,buttonCN, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonCN;
      break;
    case LANG_COMPLEX_CHINESE:
      id = ID_T_CN;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_traditional_cn"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_b.bin");
      lv_obj_align(imgset,buttonT_CN, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonT_CN;
      break;
    case LANG_ENGLISH:
      id = ID_EN;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_english"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_r.bin");
      lv_obj_align(imgset,buttonEN, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonEN;
      break;
    case LANG_RUSSIAN:
      id = ID_RU;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_russian"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_b.bin");
      lv_obj_align(imgset,buttonRU, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonRU;
      break;
    case LANG_SPANISH:
      id = ID_ES;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_spanish"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_h.bin");
      lv_obj_align(imgset,buttonES, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonES;
      break;
    case LANG_FRENCH:
      id = ID_FR;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_french"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_g.bin");
      lv_obj_align(imgset,buttonFR, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonFR;
      break;
    case LANG_ITALY:
      id = ID_IT;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_italy"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_h.bin");
      lv_obj_align(imgset,buttonIT, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonIT;
      break;
    default:
      id = ID_CN;
      // strcpy_P(public_buf_l, PSTR("F:/bmp_simplified_cn"));
      // lv_obj_del(imgset);
      // imgset = lv_img_create(scr, nullptr);
      lv_img_set_src(imgset, "F:/bmp_language_set_r.bin");
      lv_obj_align(imgset,buttonCN, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
      obj = buttonCN;
      break;
  }

  lv_refr_now(lv_refr_get_disp_refreshing());
}

void lv_draw_language() {
  scr = lv_screen_create(LANGUAGE_UI);

  static lv_style_t btn_red, btn_blue, btn_yellow, btn_green;

  lv_style_copy(&btn_red, &tft_style_preHeat_label);
  btn_red.body.main_color   = LV_COLOR_MAKE(0xd9, 0x4d, 0x40);//LV_COLOR_RED
  btn_red.body.grad_color   = LV_COLOR_MAKE(0xd9, 0x4d, 0x40);
  btn_red.text.color = LV_COLOR_WHITE;
  btn_red.text.font = &gb2312_puhui32;

  lv_style_copy(&btn_blue, &tft_style_preHeat_label);
  btn_blue.body.main_color   = LV_COLOR_MAKE(0x40, 0x84, 0xf0);//LV_COLOR_BLUE
  btn_blue.body.grad_color   = LV_COLOR_MAKE(0x40, 0x84, 0xf0);
  btn_blue.text.color = LV_COLOR_WHITE;
  btn_blue.text.font = &gb2312_puhui32;

  lv_style_copy(&btn_yellow, &tft_style_preHeat_label);
  btn_yellow.body.main_color   = LV_COLOR_MAKE(0xf8, 0xa0, 0x10);//LV_COLOR_ORANGE
  btn_yellow.body.grad_color   = LV_COLOR_MAKE(0xf8, 0xa0, 0x10);
  btn_yellow.text.color = LV_COLOR_WHITE;
  btn_yellow.text.font = &gb2312_puhui32;

  lv_style_copy(&btn_green, &tft_style_preHeat_label);
  btn_green.body.main_color   = LV_COLOR_MAKE(0x18, 0xa0, 0x60);//LV_COLOR_GREEN
  btn_green.body.grad_color   = LV_COLOR_MAKE(0x18, 0xa0, 0x60);
  btn_green.text.color = LV_COLOR_WHITE;
  btn_green.text.font = &gb2312_puhui32;

  // Create image buttons
  buttonCN = lv_label_btn_create(scr,9 , 53, 114, 90, event_handler, ID_CN);
  lv_btn_set_style(buttonCN, LV_BTN_STYLE_REL, &btn_red);
  lv_btn_set_style(buttonCN, LV_BTN_STYLE_PR,  &btn_red);
  labelCN = lv_label_create_empty(scr);
  lv_obj_set_style(labelCN, &btn_red);
  lv_label_set_text(labelCN, language_menu.chinese_s);
    lv_label_set_style(labelCN,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelCN, buttonCN, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonT_CN = lv_label_btn_create(scr, 125, 53, 114, 90, event_handler, ID_T_CN);
  lv_btn_set_style(buttonT_CN, LV_BTN_STYLE_REL, &btn_blue);
  lv_btn_set_style(buttonT_CN, LV_BTN_STYLE_PR,  &btn_blue);
  labelT_CN = lv_label_create_empty(scr);
  lv_obj_set_style(labelT_CN, &btn_blue);
  lv_label_set_text(labelT_CN, language_menu.chinese_t);
    lv_label_set_style(labelT_CN,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelT_CN, buttonT_CN, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonEN = lv_label_btn_create(scr, 241, 53, 114, 90, event_handler, ID_EN);
  lv_btn_set_style(buttonEN, LV_BTN_STYLE_REL, &btn_red);
  lv_btn_set_style(buttonEN, LV_BTN_STYLE_PR,  &btn_red);
  labelEN = lv_label_create_empty(scr);
  lv_obj_set_style(labelEN, &btn_red);
  lv_label_set_text(labelEN, language_menu.english);
    lv_label_set_style(labelEN,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelEN, buttonEN, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonRU = lv_label_btn_create(scr, 357, 53, 114, 90, event_handler, ID_RU);
  lv_btn_set_style(buttonRU, LV_BTN_STYLE_REL, &btn_blue);
  lv_btn_set_style(buttonRU, LV_BTN_STYLE_PR,  &btn_blue);
  labelRU = lv_label_create_empty(scr);
  lv_obj_set_style(labelRU, &btn_blue);
  lv_label_set_text(labelRU, language_menu.russian);
    lv_label_set_style(labelRU,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelRU, buttonRU, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonES = lv_label_btn_create(scr, 9, 145, 114, 90, event_handler, ID_ES);
  lv_btn_set_style(buttonES, LV_BTN_STYLE_REL, &btn_yellow);
  lv_btn_set_style(buttonES, LV_BTN_STYLE_PR,  &btn_yellow);
  label_ES = lv_label_create_empty(scr);
  lv_obj_set_style(label_ES, &btn_yellow);
  lv_label_set_text(label_ES, language_menu.spanish);
    lv_label_set_style(label_ES,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(label_ES, buttonES, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonFR = lv_label_btn_create(scr, 125, 145, 114, 90, event_handler, ID_FR);
  lv_btn_set_style(buttonFR, LV_BTN_STYLE_REL, &btn_green);
  lv_btn_set_style(buttonFR, LV_BTN_STYLE_PR,  &btn_green);
  labelFR = lv_label_create_empty(scr);
  lv_obj_set_style(labelFR, &btn_green);
  lv_label_set_text(labelFR, language_menu.french);
    lv_label_set_style(labelFR,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelFR, buttonFR, LV_ALIGN_IN_TOP_MID, 0, 0);

  buttonIT = lv_label_btn_create(scr, 241, 145, 114, 90, event_handler, ID_IT);
  lv_btn_set_style(buttonIT, LV_BTN_STYLE_REL, &btn_yellow);
  lv_btn_set_style(buttonIT, LV_BTN_STYLE_PR,  &btn_yellow);
  labelIT = lv_label_create_empty(scr);
  lv_obj_set_style(buttonIT, &btn_yellow);
  lv_label_set_text(labelIT, language_menu.italy);
    lv_label_set_style(labelIT,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelIT, buttonIT, LV_ALIGN_IN_TOP_MID, 0, 0);

  lv_obj_t *buttonbreak = lv_label_btn_create(scr, 357, 145, 114, 90, event_handler, ID_L_NULL);
  lv_btn_set_style(buttonbreak, LV_BTN_STYLE_REL, &btn_green);
  lv_btn_set_style(buttonbreak, LV_BTN_STYLE_PR,  &btn_green);
  // lv_obj_set_pos(buttonbreak, 357, 145);
  lv_refr_now(lv_refr_get_disp_refreshing());

  lv_obj_t *buttonUp = lv_imgbtn_create(scr, "F:/bmp_pageUp.bin", 5, 250, event_handler, ID_L_NULL);
  lv_obj_t *buttonDown = lv_imgbtn_create(scr, "F:/bmp_pageDown.bin", 176, 250, event_handler, ID_L_NULL);
  lv_obj_t *buttonBack = lv_imgbtn_create(scr, "F:/bmp_wifi_return.bin", 345, 250, event_handler, ID_L_RETURN);
  lv_refr_now(lv_refr_get_disp_refreshing());
  
  imgset = lv_img_create(scr, nullptr);
  lv_img_set_src(imgset, "F:/bmp_language_set_r.bin");
  lv_obj_align(imgset,buttonCN, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  disp_language(gCfgItems.language, SELECTED);

  lv_top_name(scr, "Language");//set_menu.language "Language"
}

void lv_clear_language() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
