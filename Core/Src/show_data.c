//
// 此文件内函数用于串口发送数据，以及在OLED屏幕上显示相应的东西
//

#include "show_data.h"
#include <stdint.h>
#include <stdio.h>
#include "oled.h"

void show_data(float *buffer, uint16_t size)
{
    for (int i = 0; i < size; i++)  printf("%.4f\n", buffer[i]);
}

char display_char_1[16] = {0};
char display_char_2[16] = {0};
void OLED_Show_sin_input(float freq, float ampl)
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "纯正弦输入:", &font16x16, OLED_COLOR_NORMAL);
    int freq_int = (int)(freq * 10000);
    int ampl_int = (int)(ampl * 10000);
    sprintf(display_char_1, "频率:%d.%04d", freq_int/10000, freq_int%10000);
    sprintf(display_char_2, "幅值:%d.%04d", ampl_int/10000, ampl_int%10000);
    OLED_PrintString(0, 20, display_char_1, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 40, display_char_2, &font16x16, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

void OLED_Show_mul_input()
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "非正弦输入:", &font16x16, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}