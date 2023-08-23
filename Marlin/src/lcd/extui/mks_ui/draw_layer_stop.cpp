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
#include "../../../gcode/queue.h"
#include "../../../gcode/gcode.h"

#include "../../../module/temperature.h"
#include "../../../module/motion.h"
#include "../../../sd/cardreader.h"
#include "../../../inc/MarlinConfig.h"

extern lv_group_t *g;
extern uint8_t layer_stop_flag;
extern lv_font_t lv_font_roboto_28;

static lv_obj_t *scr;
static lv_obj_t *btn_zoffset, *btn_fan;
static lv_obj_t *label_zoffset, *label_fan;
static lv_obj_t *btn_levelstop, *btn_op_filament;
static lv_obj_t *label_levelstop, *label_op_filament;
static lv_obj_t *imgbtn_break,*imgbtn_l_up,*imgbtn_l_next;
static lv_obj_t *btn_save,*btn_l_num;
static lv_obj_t *label_save,*label_l_num,*label_l_name;

lv_style_t save_btn_style;
lv_style_t num_btn_style;
lv_style_t label_btn_style;
lv_style_t label_big_black_style;


int32_t disp_layer_stop_num = 0;
int32_t save_disp_layer_stop_num = 0;
int32_t save_layer_stop_num = 0;

enum {
  ID_L_SAVE = 1,
  ID_L_UP,
  ID_L_NEXT,
  ID_L_NUM,
  ID_L_RETURN,
};

void disp_label_layer_stop_num()
{
    char buf[30];
    ZERO(buf);
    sprintf(buf,"%d",disp_layer_stop_num);
    lv_label_set_text(label_l_num,buf);
    lv_obj_align(label_l_num, nullptr, LV_ALIGN_CENTER, 0, 0);
}


static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW); 
    switch (obj->mks_obj_id) {
      case ID_L_UP:
            disp_layer_stop_num++; 
            disp_label_layer_stop_num();
            break;

      case ID_L_NEXT:
          disp_layer_stop_num--;
          if(disp_layer_stop_num < 0)
          {
            disp_layer_stop_num = 0;
          }
            disp_label_layer_stop_num();
      break;

      case ID_L_NUM:
            temp_value = layer_stop;
            keyboard_value = GTempsetting;
            lv_clear_layer_stop();
            lv_draw_numkeyboard();
            break;
            
      case ID_L_SAVE:
        layer_stop_flag = LAYER_STOP_NO_TRIGGERED;
        if((Layout_stop_num.data > 0)&&(Layout_stop_num.bottom_data > 0))
        {
          if((disp_layer_stop_num >= 1))
          {
            save_disp_layer_stop_num = disp_layer_stop_num;
            save_layer_stop_num = (save_disp_layer_stop_num - 1) * Layout_stop_num.data * 100;
            save_layer_stop_num += Layout_stop_num.bottom_data * 100;

            gCfgItems.save_layer_stop_num  = save_layer_stop_num;
            gCfgItems.save_disp_layer_stop_num = save_disp_layer_stop_num;
            update_spi_flash();
            // char buff[50];
            // dtostrf(save_layer_stop_num, 1, 1, buff);
            // // sprintf(buff,"%s ll0\r\n",);
            // SERIAL_ECHOPGM(buff);
            // SERIAL_ECHOPGM("\r\n");
            lv_clear_layer_stop();
            lv_draw_operation();
          }
          else
          {
            save_disp_layer_stop_num = 0;
            save_layer_stop_num = 0;
          }
        }
        break;
        case ID_L_RETURN:
        lv_clear_layer_stop();
        lv_draw_operation();
        break;
    }
}

void lv_draw_layer_stop() 
{
  scr = lv_screen_create(LAYER_STOP_UI);
  lv_obj_t *imgtop = lv_obj_create(scr, nullptr);
  lv_obj_set_style(imgtop, &tft_style_preHeat_BLUE);
  lv_obj_set_size(imgtop, 480, 42);
  lv_obj_set_pos(imgtop, 0, 0);


  lv_style_copy(&save_btn_style, &lv_style_scr);
  save_btn_style.body.main_color = LV_COLOR_MAKE(0x41,0x85,0xf7);//LV_COLOR_MAKE(0x41,0x85,0xf7);
  save_btn_style.body.grad_color = LV_COLOR_MAKE(0x41,0x85,0xf7);//LV_COLOR_MAKE(0x41,0x85,0xf7);
  save_btn_style.body.radius = 10;

  lv_style_copy(&num_btn_style, &lv_style_scr);
  num_btn_style.body.main_color = LV_COLOR_MAKE(0xF9,0xF9,0xF9);//LV_COLOR_MAKE(0x41,0x85,0xf7);
  num_btn_style.body.grad_color = LV_COLOR_MAKE(0xF9,0xF9,0xF9);//LV_COLOR_MAKE(0x41,0x85,0xf7);
  num_btn_style.body.border.width  = 1;
  num_btn_style.body.border.part   = LV_BORDER_FULL;
  num_btn_style.body.border.color  = LV_COLOR_MAKE(0xC9,0xC9,0xC9);//LV_COLOR_MAKE(0x41,0x85,0xf7);
  num_btn_style.body.border.opa    = 170;

  lv_style_copy(&label_btn_style, &tft_style_preHeat_label_BLACK);
  label_btn_style.text.color = LV_COLOR_BLACK;
  label_btn_style.text.font = &lv_font_roboto_28;


  lv_style_copy(&label_big_black_style, &tft_style_preHeat_label);
  label_big_black_style.text.font = &gb2312_puhui32;

#if 1

  label_l_name = lv_label_create_empty(scr);
  lv_label_set_style(label_l_name, LV_LABEL_STYLE_MAIN, &label_big_black_style);
  lv_label_set_text(label_l_name, printing_MENU.layer_num);
  lv_obj_align(label_l_name, nullptr, LV_ALIGN_CENTER, 0, -70);

  btn_save = lv_button_btn_create(scr, 44, 227, 135, 78, event_handler, ID_L_SAVE);
  lv_btn_set_style_both(btn_save,&save_btn_style);

  label_save = lv_label_create_empty(btn_save);
  lv_label_set_style(label_save, LV_LABEL_STYLE_MAIN, &label_dialog_white);
  lv_label_set_text(label_save, printing_MENU.save);
  lv_obj_align(label_save, nullptr, LV_ALIGN_CENTER, 0, 0);

  imgbtn_break = lv_imgbtn_create(scr, "F:/layer_return.bin", event_handler, ID_L_RETURN);
  lv_obj_set_pos(imgbtn_break, 360, 230);

  btn_l_num = lv_button_btn_create(scr, 129, 123, 223, 74, event_handler, ID_L_NUM);
  lv_btn_set_style_both(btn_l_num,&num_btn_style);


  char buf[30];
  ZERO(buf);
  sprintf(buf,"%ld",disp_layer_stop_num);

  label_l_num = lv_label_create_empty(btn_l_num);
  lv_label_set_style(label_l_num, LV_LABEL_STYLE_MAIN, &label_btn_style);
  lv_label_set_text(label_l_num, buf);
  lv_obj_align(label_l_num, nullptr, LV_ALIGN_CENTER, 0, 0);

  imgbtn_l_up = lv_imgbtn_create(scr, "F:/layer_up.bin", event_handler, ID_L_NEXT);
  lv_obj_set_pos(imgbtn_l_up, 48, 123);

  imgbtn_l_next = lv_imgbtn_create(scr, "F:/layer_next.bin", event_handler, ID_L_UP);
  lv_obj_set_pos(imgbtn_l_next, 352, 123);

  lv_refr_now(lv_refr_get_disp_refreshing());
  // lv_imgbtn_set_src_both(buttonReturn, "F:/bmp_preHeat_return.bin");
  

  lv_obj_t *labelname = lv_label_create_empty(scr);
  lv_label_set_text(labelname, printing_MENU.layer_to_stop);
  lv_obj_set_style(labelname, &label_dialog_white);
  lv_obj_set_pos(labelname, 18, 10);

  #endif
}


void lv_clear_layer_stop() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}


#endif // HAS_TFT_LVGL_UI
