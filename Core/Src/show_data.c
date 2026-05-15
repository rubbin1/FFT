//
// 此文件内函数用于串口发送数据，以及在OLED屏幕上显示相应的东西
//

#include "show_data.h"
#include <stdint.h>
#include <stdio.h>
#include "oled.h"
#include "main.h"
#include "private_typedef.h"

extern const SystemConfig system_config;

char display_char_1[24] = {0};
char display_char_2[24] = {0};
char display_char_3[24] = {0};

//纯正弦输入时的屏幕显示
void OLED_Show_sin_input()
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "纯正弦输入", &font12x12, OLED_COLOR_NORMAL);
    int freq_int = (int)(waveParam.frequency * 10000);
    int ampl_int = (int)(waveParam.amplitude * 10000);
    sprintf(display_char_1, "freq=%d.%04d", freq_int/10000, freq_int%10000);
    sprintf(display_char_2, "ampl=%d.%04d", ampl_int/10000, ampl_int%10000);
    OLED_PrintASCIIString(0, 15, display_char_1, &afont12x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(0, 30, display_char_2, &afont12x6, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
}

//非正弦输入时的屏幕显示
void OLED_Show_mul_input(float *freqs, float *ampls, int pages)
{
    OLED_NewFrame();
    OLED_PrintString(0, 0, "非正弦输入", &font12x12, OLED_COLOR_NORMAL);
    //因为要是启用浮点数的话，会造成大量的内存占用，所以采取此方法规避浮点数的打印
    int ampl_base_int = (int)(ampls[0] * 10000);
    int freq_int = (int)(freqs[pages] * 10000);
    int ampl_int = (int)(ampls[pages] * 10000);
    if (pages > 0 && ampl_base_int > 0)
    {
        int H_i = ampl_int * 100 * 10 / ampl_base_int;
        sprintf(display_char_3, "Hi(%d)=%d.%01d%%", pages + 1, H_i/10, H_i%10);
    }
    //显示波的频率和幅值
    sprintf(display_char_1, "freq=%d.%04d", freq_int/10000, freq_int%10000);
    sprintf(display_char_2, "ampl=%d.%04d", ampl_int/10000, ampl_int%10000);
    OLED_PrintASCIIString(0, 15, display_char_1, &afont12x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(0, 30, display_char_2, &afont12x6, OLED_COLOR_NORMAL);

    //显示波的类型和占有率
    if (pages == 0)
    {
        OLED_PrintString(80, 0, "基波", &font12x12, OLED_COLOR_NORMAL);
        //显示页码
        OLED_PrintASCIIString(116, 52, "1", &afont12x6, OLED_COLOR_NORMAL);
    }
    else
    {
        //先显示占有率
        OLED_PrintASCIIString(0, 45, display_char_3, &afont12x6, OLED_COLOR_NORMAL);
        //再显示右上角的种类
        const char *wave_kind[] = {"二次谐波", "三次谐波", "四次谐波", "五次谐波"};
        OLED_PrintString(80, 0, wave_kind[pages - 1], &font12x12, OLED_COLOR_NORMAL);
        //最后显示右下角页码
        const char *number[] = {"2", "3", "4", "5"};
        OLED_PrintASCIIString(116, 52, number[pages - 1], &afont12x6, OLED_COLOR_NORMAL);
    }
    OLED_ShowFrame();
}

//在非正弦输入模式下，对信号波形图进行绘制
void OLED_Show_Image(uint16_t *adc_data, float f0)
{
    const int SCREEN_W = 128;
    const int SCREEN_H = 64;
    static float wave[1024];
    float sum = 0;

    for (int i = 0; i < 1024; i++)
    {
        float v = adc_data[i] * 3.3f / 4096.0f;
        wave[i] = v;
        sum += v;
    }
    float avg = sum / 1024.0f;
    for (int i = 0; i < 1024; i++)
    {
        wave[i] -= avg;
    }

    // 查找第一个上升过零点（从负到正）
    int start_idx = 0;
    for (int i = 1; i < 1024; i++) {
        if (wave[i-1] < 0 && wave[i] >= 0) {
            start_idx = i - 1;   // 从过零点前一个点开始，也可用插值提高精度
            break;
        }
    }

    int y_center = SCREEN_H / 2;
    float y_scale = 28.0f;

    // 一个周期的采样点数
    float period_samples = system_config.adc_sample_rate / f0;

    OLED_NewFrame();
    int prev_x = 0, prev_y = y_center;
    for (int col = 0; col < SCREEN_W; col++)
    {
        // 映射到从 start_idx 开始的一个周期内
        float idx_f = start_idx + (float) 1.5 * col / SCREEN_W * period_samples;
        int idx = (int)idx_f;
        float frac = idx_f - idx;

        float val;
        if (idx + 1 < 1024)
            val = wave[idx] * (1.0f - frac) + wave[idx + 1] * frac;
        else
            val = wave[idx];

        int y = y_center - (int)(val * y_scale);
        int x = col;
        if (col > 0) OLED_DrawLine(prev_x, prev_y, x, y, OLED_COLOR_NORMAL);
        prev_x = x; prev_y = y;
    }
    OLED_ShowFrame();
}

void Open_OLED_Show()
{
    OLED_PrintASCIIString(0, 0, "System Begin", &afont12x6, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    HAL_Delay(3000);
    OLED_NewFrame();
    OLED_ShowFrame();
}