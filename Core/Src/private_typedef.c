//
// Created by 35156 on 26-5-15.
//

#include "private_typedef.h"

const SystemConfig system_config =
{
    .adc_sample_rate = 10000,   // 10 kHz 采样率
    .fft_len = 1024,            // FFT长度 1024
    .adc_buffer_size = 1024,    // DMA每次传输 1024个点
    .adc_vref = 3.1f,           // 参考电压 3.3V
    .adc_resolution = 4096,     // 12位ADC最大值 4096
    .square_root_2 = 1.41421356237,
};

WaveParam waveParam = {0.0f, 0.0f};
HarmonicsResult harmonicsResult = {
    {0.0f, 0.0f},
        {
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        {0.0f, 0.0f},
                        }
};

AdcBuffers_flag adcbuf_flag = {0};

Display_State display_state = {
    .mode = SINGLE_WAVE_Input,
    .imageMod = IMAGE_MODE_OFF,
    .single_wave_pages = 0,
    .harmonic_pages = 0,
    .error_type = 0,
};


float actual_vdda = 3.3f;   // 初始默认值，校准后会更新