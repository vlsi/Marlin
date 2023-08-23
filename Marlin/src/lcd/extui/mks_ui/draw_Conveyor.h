#pragma once

#ifdef __cplusplus
  extern "C" {
#endif


typedef struct
{
    bool open_flag;
    bool loop;
}Conveyor_T;
extern Conveyor_T convetor_t;


extern void lv_draw_Conveyor();
extern void lv_clear_Conveyor();
extern void lv_switch_refr();
void lv_loop_refr();

#ifdef __cplusplus
}
#endif
