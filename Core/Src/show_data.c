//
// 此文件内函数用于串口发送数据，以及在OLED屏幕上显示相应的东西
//

#include "show_data.h"
#include <stdint.h>
#include <stdio.h>
#include "oled.h"

char display_char_1[16] = {0};
char display_char_2[16] = {0};
void OLED_Show_sin_input(float freq, float ampl)
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "纯正弦输入", &font12x12, OLED_COLOR_NORMAL);
    int freq_int = (int)(freq * 10000);
    int ampl_int = (int)(ampl * 10000);
    sprintf(display_char_1, "freq=%d.%04d", freq_int/10000, freq_int%10000);
    sprintf(display_char_2, "ampl=%d.%04d", ampl_int/10000, ampl_int%10000);
    OLED_PrintASCIIString(0, 20, display_char_1, &afont12x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(0, 40, display_char_2, &afont12x6, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

void OLED_Show_mul_input(float *freqs, float *ampls, int pages)
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "非正弦输入", &font12x12, OLED_COLOR_NORMAL);
    int freq_int = (int)(freqs[pages] * 10000);
    int ampl_int = (int)(ampls[pages] * 10000);
    sprintf(display_char_1, "freq=%d.%04d", freq_int/10000, freq_int%10000);
    sprintf(display_char_2, "ampl=%d.%04d", ampl_int/10000, ampl_int%10000);
    OLED_PrintASCIIString(0, 20, display_char_1, &afont12x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(0, 40, display_char_2, &afont12x6, OLED_COLOR_NORMAL);
    switch (pages)
    {
        case 0:
        OLED_PrintString(80, 0, "基波", &font12x12, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(116, 52, "1", &afont12x6, OLED_COLOR_NORMAL);
        break;
        case 1:
        OLED_PrintString(80, 0, "二次谐波", &font12x12, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(116, 52, "2", &afont12x6, OLED_COLOR_NORMAL);
        break;
        case 2:
        OLED_PrintString(80, 0, "三次谐波", &font12x12, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(116, 52, "3", &afont12x6, OLED_COLOR_NORMAL);
        break;
        case 3:
        OLED_PrintString(80, 0, "四次谐波", &font12x12, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(116, 52, "4", &afont12x6, OLED_COLOR_NORMAL);
        break;
        case 4:
        OLED_PrintString(80, 0, "五次谐波", &font12x12, OLED_COLOR_NORMAL);
        OLED_PrintASCIIString(116, 52, "5", &afont12x6, OLED_COLOR_NORMAL);
        break;
    }
    OLED_ShowFrame();
}