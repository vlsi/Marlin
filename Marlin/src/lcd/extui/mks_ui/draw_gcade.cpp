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
#include "draw_endstop.h"

#include "../../../gcode/queue.h"
#include "../../../inc/MarlinConfig.h"

extern const char G28_STR[];

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *labelXEndstop, *labelYEndstop, *labelZEndstop, *labelMaterialDetection, *labelTrigger, *labelOpen;
// static lv_obj_t *labelname;
static lv_obj_t *btnXTrigger, *btnYTrigger, *btnZTrigger, *btnMDTrigger, *btnXOpen, *btnYOpen, *btnZOpen, *btnMDOpen;
// extern bool XState = true, YState = true, ZState = true, MDState = true;
static char gcadeBuf[30] = {};
static char absplanumbuf[30] = {};
lv_obj_t *labelBuf;

enum {
  ID_GCADE_M = 1,
  ID_GCADE_G,
  ID_GCADE_S,
  ID_GCADE_P,
  ID_GCADE_E,
  ID_GCADE_1,
  ID_GCADE_2,
  ID_GCADE_3,
  ID_GCADE_4,
  ID_GCADE_5,
  ID_GCADE_6,
  ID_GCADE_7,
  ID_GCADE_8,
  ID_GCADE_9,
  ID_GCADE_0,
  ID_GCADE_X,
  ID_GCADE_Y,
  ID_GCADE_Z,
  ID_GCADE_POT,
  ID_GCADE_LINK,
  ID_GCADE_SPACE,
  ID_GCADE_RETURN,
};

static const char * kb_map_lc[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                   "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                   "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                   LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};
static const lv_btnm_ctrl_t kb_ctrl_lc_map[] = {
  5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7,
  6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 6, 2, 2};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  switch (obj->mks_obj_id) {
    case ID_GCADE_M:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "M");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_G:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "G");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_S:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "S");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_P:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "P");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_E:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "E");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_1:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "1");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_2:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "2");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_3:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "3");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_4:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "4");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_5:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "5");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_6:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "6");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_7:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "7");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_8:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "8");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_9:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "9");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_0:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "0");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_X:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "Z");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_Y:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "Y");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_Z:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "Z");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_POT:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, ".");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_LINK:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, "-");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_SPACE:
      if(strlen(gcadeBuf) < 30)
      {
        strcat(gcadeBuf, " ");
        lv_label_set_text(labelBuf, gcadeBuf);
      }
      break;
    case ID_GCADE_RETURN:
      memset(gcadeBuf, 0 , sizeof(gcadeBuf));
      lv_clear_gcade();
      lv_draw_tool();
      break;
    default:
      break;
  }
}


static void lv_kb_event_cb(lv_obj_t *kb, lv_event_t event) {
  if (event != LV_EVENT_VALUE_CHANGED) return;

  lv_kb_ext_t *ext = (lv_kb_ext_t*)lv_obj_get_ext_attr(kb);
  const uint16_t btn_id = lv_btnm_get_active_btn(kb);
  if (btn_id == LV_BTNM_BTN_NONE) return;
  if (lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_HIDDEN | LV_BTNM_CTRL_INACTIVE)) return;
  if (lv_btnm_get_btn_ctrl(kb, btn_id, LV_BTNM_CTRL_NO_REPEAT) && event == LV_EVENT_LONG_PRESSED_REPEAT) return;

  const char * txt = lv_btnm_get_active_btn_text(kb);
  if (!txt) return;

  // Do the corresponding action according to the text of the button
  // if (strcmp(txt, "abc") == 0) {
  //   lv_btnm_set_map(kb, kb_map_lc);
  //   lv_btnm_set_ctrl_map(kb, kb_ctrl_lc_map);
  //   return;
  // }
  // else if (strcmp(txt, "ABC") == 0) {
  //   lv_btnm_set_map(kb, kb_map_uc);
  //   lv_btnm_set_ctrl_map(kb, kb_ctrl_uc_map);
  //   return;
  // }
  // else if (strcmp(txt, "1#") == 0) {
  //   lv_btnm_set_map(kb, kb_map_spec);
  //   lv_btnm_set_ctrl_map(kb, kb_ctrl_spec_map);
  //   return;
  // }
  // else 
  if (strcmp(txt, LV_SYMBOL_CLOSE) == 0) {
    if (kb->event_cb != lv_kb_def_event_cb) {
      lv_clear_keyboard();
      draw_return_ui();
    }
    else {
      lv_kb_set_ta(kb, nullptr); // De-assign the text area to hide its cursor if needed
      lv_obj_del(kb);
      return;
    }
    return;
  }
  else if (strcmp(txt, LV_SYMBOL_OK) == 0) {
    if (kb->event_cb != lv_kb_def_event_cb) {
      const char * ret_ta_txt = lv_ta_get_text(ext->ta);
      switch (keyboard_value) {
        #if ENABLED(MKS_WIFI_MODULE)
          case wifiName:
            memcpy(uiCfg.wifi_name, ret_ta_txt, sizeof(uiCfg.wifi_name));
            lv_clear_keyboard();
            draw_return_ui();
            break;
          case wifiPassWord:
            memcpy(uiCfg.wifi_key, ret_ta_txt, sizeof(uiCfg.wifi_name));
            lv_clear_keyboard();
            draw_return_ui();
            break;
          case wifiConfig:
            ZERO(uiCfg.wifi_name);
            memcpy((void *)uiCfg.wifi_name, wifi_list.wifiName[wifi_list.nameIndex], 32);

            ZERO(uiCfg.wifi_key);
            memcpy((void *)uiCfg.wifi_key, ret_ta_txt, sizeof(uiCfg.wifi_key));

            gCfgItems.wifi_mode_sel = STA_MODEL;

            package_to_wifi(WIFI_PARA_SET, nullptr, 0);

            public_buf_l[0] = 0xA5;
            public_buf_l[1] = 0x09;
            public_buf_l[2] = 0x01;
            public_buf_l[3] = 0x00;
            public_buf_l[4] = 0x01;
            public_buf_l[5] = 0xFC;
            public_buf_l[6] = 0x00;
            raw_send_to_wifi((uint8_t*)public_buf_l, 6);

            last_disp_state = KEYBOARD_UI;
            lv_clear_keyboard();
            wifi_tips_type = TIPS_TYPE_JOINING;
            lv_draw_wifi_tips();
            break;
        #endif // MKS_WIFI_MODULE
        case autoLevelGcodeCommand:
          uint8_t buf[100];
          strncpy((char *)buf, ret_ta_txt, sizeof(buf));
          update_gcode_command(AUTO_LEVELING_COMMAND_ADDR, buf);
          lv_clear_keyboard();
          draw_return_ui();
          break;
        case GCodeCommand:
          if (!queue.ring_buffer.full(3)) {
            // Hook anything that goes to the serial port
            MYSERIAL1.setHook(lv_serial_capt_hook, lv_eom_hook, 0);
            queue.enqueue_one_now(ret_ta_txt);
          }
          lv_clear_keyboard();
          // draw_return_ui is called in the end of message hook
          break;
        default: break;
      }
    }
    else
      lv_kb_set_ta(kb, nullptr); // De-assign the text area to hide it cursor if needed
    return;
  }

  // Add the characters to the text area if set
  if (!ext->ta) return;

  if (strcmp(txt, "Enter") == 0 || strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
    lv_ta_add_char(ext->ta, '\n');
  else if (strcmp(txt, LV_SYMBOL_LEFT) == 0)
    lv_ta_cursor_left(ext->ta);
  else if (strcmp(txt, LV_SYMBOL_RIGHT) == 0)
    lv_ta_cursor_right(ext->ta);
  else if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
    lv_ta_del_char(ext->ta);
  else if (strcmp(txt, "+/-") == 0) {
    uint16_t cur = lv_ta_get_cursor_pos(ext->ta);
    const char * ta_txt = lv_ta_get_text(ext->ta);
    if (ta_txt[0] == '-') {
      lv_ta_set_cursor_pos(ext->ta, 1);
      lv_ta_del_char(ext->ta);
      lv_ta_add_char(ext->ta, '+');
      lv_ta_set_cursor_pos(ext->ta, cur);
    }
    else if (ta_txt[0] == '+') {
      lv_ta_set_cursor_pos(ext->ta, 1);
      lv_ta_del_char(ext->ta);
      lv_ta_add_char(ext->ta, '-');
      lv_ta_set_cursor_pos(ext->ta, cur);
    }
    else {
      lv_ta_set_cursor_pos(ext->ta, 0);
      lv_ta_add_char(ext->ta, '-');
      lv_ta_set_cursor_pos(ext->ta, cur + 1);
    }
  }
  else {
    lv_ta_add_text(ext->ta, txt);
  }
}

void lv_draw_gcade() {
  scr = lv_screen_create(GCADE_UI , "");

  //create top img
  lv_obj_t *imgtop = lv_img_create(scr, NULL);
  lv_img_set_src(imgtop, "F:/bmp_preHeat_top.bin");
  lv_obj_set_pos(imgtop, 0, 0);
  lv_refr_now(lv_refr_get_disp_refreshing());

  lv_obj_t *buttonReturn = lv_imgbtn_create(scr, "F:/bmp_preHeat_return.bin", event_handler, ID_GCADE_RETURN);
  lv_obj_set_pos(buttonReturn, 6, 3);

  lv_obj_t *labelnum = lv_label_create_empty(scr);
  // itoa(100, absplanumbuf, 0);
  lv_label_set_text(labelnum, "100");
  lv_label_set_style(labelnum, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label);
  // lv_obj_set_pos(labelnum, 63, 13);
  lv_obj_align(labelnum, imgtop, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  // gcade_btn_label_create(scr, 64, 66, "M",event_handler, ID_GCADE_M);
  // gcade_btn_label_create(scr, 131, 66, "G",event_handler, ID_GCADE_G);
  // gcade_btn_label_create(scr, 197, 66, "S",event_handler, ID_GCADE_S);
  // gcade_btn_label_create(scr, 261, 66, "P",event_handler, ID_GCADE_P);
  // gcade_btn_label_create(scr, 326, 66, "E",event_handler, ID_GCADE_E);
  lv_obj_t *ta = lv_ta_create(scr, nullptr);
  lv_obj_set_size(ta,238, 31);
  lv_obj_set_style(ta , &tft_style_preHeat_label);
  lv_ta_set_text(ta, "100");
  // lv_obj_set_pos(ta, 126, 6);
  lv_obj_align(ta, labelnum, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  // gcade_btn_label_create(scr, 6, 94, " 1 ",event_handler, ID_GCADE_1);
  // gcade_btn_label_create(scr, 124, 94, " 2 ",event_handler, ID_GCADE_2);
  // gcade_btn_label_create(scr, 242, 94, " 3 ",event_handler, ID_GCADE_3);
  // gcade_btn_label_create(scr, 360, 94, " 0 ",event_handler, ID_GCADE_0);
  // gcade_btn_label_create(scr, 6, 244, " 7 ",event_handler, ID_GCADE_7);
  // gcade_btn_label_create(scr, 124, 244, " 8 ",event_handler, ID_GCADE_8);
  // gcade_btn_label_create(scr, 242, 244, " 9 ",event_handler, ID_GCADE_9);
  // gcade_btn_label_create(scr, 360, 244, " <- ",event_handler, ID_GCADE_Z);
  // gcade_btn_label_create(scr, 6, 169, " 4 ",event_handler, ID_GCADE_4);
  // gcade_btn_label_create(scr, 124, 169, " 5 ",event_handler, ID_GCADE_5);
  // gcade_btn_label_create(scr, 242, 169, " 6 ",event_handler, ID_GCADE_6);
  // gcade_btn_label_create(scr, 360, 169, " . ",event_handler, ID_GCADE_POT);

  // gcade_btn_label_create(scr, 324, 107, "X",event_handler, ID_GCADE_X);
  
  // gcade_btn_label_create(scr, 264, 148, "-",event_handler, ID_GCADE_LINK);
  // gcade_btn_label_create(scr, 324, 148, "Y",event_handler, ID_GCADE_Y);
  // gcade_btn_label_create(scr, 411, 148, "_",event_handler, ID_GCADE_SPACE);
  
  // gcade_btn_label_create(scr, 324, 209, "Z",event_handler, ID_GCADE_Z);

  lv_img_set_src(imgtop, "F:/bmp_preHeat_top.bin");
  lv_imgbtn_set_src_both(buttonReturn,"F:/bmp_preHeat_return.bin");
  // lv_label_set_text(labelname, "Gcode");
  // lv_obj_set_pos(labelname, 69, 13);
}

void lv_clear_gcade() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

void lv_gcade_temp_refr()
{
  if(READ(PA15)){
    lv_img_set_src(btnXTrigger, "F:/bmp_endstop_off.bin");
    lv_img_set_src(btnXOpen, "F:/bmp_endstop_on.bin");
  }
  else{
    lv_img_set_src(btnXTrigger, "F:/bmp_endstop_on.bin");
    lv_img_set_src(btnXOpen, "F:/bmp_endstop_off.bin");
  }

  if(READ(PD2)){
    lv_img_set_src(btnYTrigger, "F:/bmp_endstop_off.bin");
    lv_img_set_src(btnYOpen, "F:/bmp_endstop_on.bin");
  }
  else{
    lv_img_set_src(btnYTrigger, "F:/bmp_endstop_on.bin");
    lv_img_set_src(btnYOpen, "F:/bmp_endstop_off.bin");
  }
  
  if(READ(PC8)){
    lv_img_set_src(btnZTrigger, "F:/bmp_endstop_off.bin");
    lv_img_set_src(btnZOpen, "F:/bmp_endstop_on.bin");
  }
  else{
    lv_img_set_src(btnZTrigger, "F:/bmp_endstop_on.bin");
    lv_img_set_src(btnZOpen, "F:/bmp_endstop_off.bin");
  }
  
  if(READ(PC4)){
    lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_off.bin");
    lv_img_set_src(btnMDOpen, "F:/bmp_endstop_on.bin");
  }
  else{
    lv_img_set_src(btnMDTrigger, "F:/bmp_endstop_on.bin");
    lv_img_set_src(btnMDOpen, "F:/bmp_endstop_off.bin");
  }
}


lv_obj_t *gcade_btn_label_create(lv_obj_t *par, lv_coord_t x, lv_coord_t y, const char *text,lv_event_cb_t cb, const int id/*=0*/)
{
  lv_obj_t *btn = lv_button_btn_create(scr, x, y, 114, 71, cb, id);
  lv_btn_set_style(btn, LV_BTN_STYLE_REL, &tft_style_preHeat_scr);
  lv_btn_set_style(btn, LV_BTN_STYLE_PR,  &tft_style_preHeat_scr);
  lv_obj_t *label = lv_label_create_empty(btn);
  lv_label_set_text(label , text);
  lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &tft_style_preHeat_label);
  lv_obj_align(label, btn , LV_ALIGN_CENTER, 0, 0);
  lv_refr_now(lv_refr_get_disp_refreshing());

  return btn;
}
#endif // HAS_TFT_LVGL_UI
