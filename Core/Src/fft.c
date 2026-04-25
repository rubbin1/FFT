//
// Created by 35156 on 26-4-25.
//
#include "fft.h"
#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "window_table.h"

float moni_freq = 1000.f;            //模拟信号频率
float sample_rate = 10000.f;        //采样率
float FFT_buffer[FFT_LEN * 2] = {0};//FFT函数输入输出数组

float FFT_mag_max = 0;
uint32_t FFT_mag_max_index = 0;
float FFT_freq = 0;
float FFT_ampl = 0;

arm_cfft_instance_f32 scfft;

void FFT_Init()
{
    arm_cfft_init_f32(&scfft, FFT_LEN);
}

void generate_data()
{
    //1.对数据转成复数数组的同时，进行加汉宁窗
    for (int i = 0; i < FFT_LEN; i++)
    {
        float val = sinf(2.0 * M_PI * moni_freq * i / sample_rate);
        FFT_buffer[2 * i] = val * Hanning_Window[i];
        FFT_buffer[2 * i + 1] = 0;
    }
}

void fft_process()
{
     //1.FFT计算
     arm_cfft_f32(&scfft, FFT_buffer, 0, 1);
     //3.计算复数幅度
     arm_cmplx_mag_f32(FFT_buffer, FFT_buffer, FFT_LEN);
     //4.幅值校准
     for (int i = 0; i < FFT_LEN; i++)
     {
         FFT_buffer[i] = FFT_buffer[i] * 4.0f / FFT_LEN;
     }
     //5.找基波，求基波幅值、基波频率
     arm_max_f32(FFT_buffer, FFT_LEN / 2, &FFT_mag_max, &FFT_mag_max_index);
     //
     FFT_freq = (float)(FFT_mag_max_index) * sample_rate / (float)FFT_LEN;
     FFT_ampl = FFT_mag_max;
     printf("FFT_freq = %.4f\n", FFT_freq);
     printf("FFT_ampl = %.4f\n", FFT_ampl);
}