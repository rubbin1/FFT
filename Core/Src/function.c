//
// Created by 35156 on 26-5-15.
//此文件用于封装功能
//

#include "function.h"
#include "fft.h"
#include "key_press.h"
#include "private_typedef.h"
#include "show_data.h"
#include "zero_crossing_and_dft.h"
#include "error_type_dispaly.h"

void key0_pressed(void)
{
    if (key0.short_pressed_flag)
    {
        key0.short_pressed_flag = 0;

        if (display_state.mode == SINGLE_WAVE_Input)
        {
            //进入非正弦输入时，每次都要回到基波界面
            display_state.harmonic_pages = 0;
            display_state.mode = MULTI_WAVE_Input;
            display_state.imageMod = IMAGE_MODE_OFF;
        }
        else display_state.mode = SINGLE_WAVE_Input;
    }
}

void key1_pressed(void)
{
    if (key1.short_pressed_flag)
    {
        key1.short_pressed_flag = 0;
        if (display_state.mode == MULTI_WAVE_Input && display_state.imageMod == IMAGE_MODE_OFF)
        {
            display_state.harmonic_pages = (display_state.harmonic_pages + 1) % 5;
        }
    }
}

void key2_pressed(void)
{
    if (key2.short_pressed_flag)
    {
        key2.short_pressed_flag = 0;
        if (display_state.mode == MULTI_WAVE_Input)
        {
            if (display_state.imageMod == IMAGE_MODE_OFF)   display_state.imageMod = IMAGE_MODE_ON;
            else display_state.imageMod = IMAGE_MODE_OFF;
        }
    }
}

void adc_DataProcessing(void)
{
    // 数据处理
    float sum = 0;
    for (int i = 0; i < ADC_BUFFER_SIZE; i++)
    {
        float v = adcbuf_flag.snapshot[i] * system_config.adc_vref / system_config.adc_resolution;
        Data_buffer[ADC_BUFFER_SIZE + i] = v;
        sum += v;
    }
    float avg = sum / system_config.adc_buffer_size;

    //去直流
    for (int i = 0; i < system_config.adc_buffer_size; i++)
    {
        Data_buffer[system_config.adc_buffer_size + i] -= avg;
    }
}

void single_wave_input_function(void)
{
    Data_buffer_sin(Data_buffer);
    float probably_freq = zero_crossing_raw(Data_buffer, system_config.adc_buffer_size);
    if (probably_freq > 0)
    {
        precise_measure(probably_freq);
        OLED_Show_sin_input();
    }
}

void multi_wave_input_function(void)
{
    Data_buffer_nosin(Data_buffer);
    fft_process_harmonics();
    if (display_state.imageMod == IMAGE_MODE_ON)
    {
        if (harmonicsResult.fundamental.frequency <= 0)
        {
            //此时的错误为：非正弦输入时，测得的基波频率<=0
            display_state.error_type = 1;
            Error_Type();
        }
        OLED_Show_Image(adcbuf_flag.snapshot, harmonicsResult.fundamental.frequency);
    }
    else
    {
        float freqs[5] =
        {
            harmonicsResult.fundamental.frequency,
            harmonicsResult.harmonics[0].frequency,
            harmonicsResult.harmonics[1].frequency,
            harmonicsResult.harmonics[2].frequency,
            harmonicsResult.harmonics[3].frequency
          };
        float ampls[5] =
        {
            harmonicsResult.fundamental.amplitude,
            harmonicsResult.harmonics[0].amplitude,
            harmonicsResult.harmonics[1].amplitude,
            harmonicsResult.harmonics[2].amplitude,
            harmonicsResult.harmonics[3].amplitude
          };
        OLED_Show_mul_input(freqs, ampls, display_state.harmonic_pages);
    }
}