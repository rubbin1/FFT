//
// Created by 35156 on 26-4-25.
//
#include "fft.h"

#include <stdio.h>

#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "main.h"

#define FFT_LEN 1024

float moni_freq = 1000.f;            //模拟信号频率
float sample_rate = 10000.f;        //采样率
float input_data[FFT_LEN] = {0};    //数据数组
float FFT_buffer[FFT_LEN * 2] = {0};//FFT函数输入输出数组
float FFT_mag[FFT_LEN] = {0};       //幅度数组

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
    for (int i = 0; i < FFT_LEN; i++)
    {
        input_data[i] = sinf(2.0 * M_PI * moni_freq * i / sample_rate);
    }
}

void fft_process()
{
    //1.转换成复数数组
    for (int i = 0; i < FFT_LEN; i++)
    {
        FFT_buffer[2 * i] = input_data[i];
        FFT_buffer[2 * i + 1] = 0;
    }
    //2.FFT计算
    arm_cfft_f32(&scfft, FFT_buffer, 0, 1);
    //3.计算复数幅度
    arm_cmplx_mag_f32(FFT_buffer, FFT_mag, FFT_LEN);
    //4.找基波，求基波频率和基波幅值
    arm_max_f32(FFT_mag, FFT_LEN / 2, &FFT_mag_max, &FFT_mag_max_index);
    FFT_freq = (float)(FFT_mag_max_index) * sample_rate / (float)FFT_LEN;
    FFT_ampl = FFT_mag_max * 2.0f / (float)FFT_LEN;
    printf("FFT_freq = %.4f\n", FFT_freq);
    printf("FFT_ampl = %.4f\n", FFT_ampl);
}