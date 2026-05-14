/*
 *本文件用于在单纯输入正弦信号时，不用fft算法进行繁杂的计算
 *而是采用过零检测粗测频率，再对粗略频率附近的三点DFT准确求出频率和幅值
 */

#include "zero_crossing_and_dft.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#include "private_typedef.h"

#define FFT_LEN 1024

extern const SystemConfig system_config;

extern float Data_buffer[FFT_LEN * 2];

//将Data_buffer数组写为下面可以接受的样子
void Data_buffer_sin(float *buf)
{
    for(int i = 0; i < system_config.fft_len; i++)
    {
        Data_buffer[i] = buf[i + system_config.fft_len];
    }
}

//先进行过零检测，算出大概的频率
float zero_crossing_raw(const float *samples, int len)
{
    float fs = system_config.adc_sample_rate;
    int zero_cross_count = 0;
    float first_cross_time = 0.0f, last_cross_time = 0.0f;
    for (int i = 1; i < len; i++)
    {
        if (samples[i-1] < 0 && samples[i] >= 0)
        {
            float frac = -samples[i-1] / (samples[i] - samples[i-1]);
            float cross_time = (i-1 + frac) / fs;
            if (zero_cross_count == 0) first_cross_time = cross_time;
            last_cross_time = cross_time;
            zero_cross_count++;
        }
    }
    if (zero_cross_count < 2) return 0;
    return (zero_cross_count - 1) / (last_cross_time - first_cross_time);
}

//再进行三点DFT插值计算，
void precise_measure(float proboly_freq)
{
    int N = system_config.fft_len;
    float fs = system_config.adc_sample_rate;

    // 1. 粗频 → 浮点索引 → 取基准 k0
    float k_float = proboly_freq * N / fs;
    int k0 = (int) floorf(k_float);
    if (k0 < 1) k0 = 1;
    if (k0 > N/2 - 2) k0 = N/2 - 2;

    // 2. 计算三个相邻 DFT 点的模值 (k0, k0+1, k0+2)
    float mag[3];
    for (int idx = 0; idx < 3; idx++)
    {
        int k = k0 + idx;
        float re = 0, im = 0;
        for (int n = 0; n < N; n++)
        {
            float angle = 2.0f * M_PI * k * n / N;
            re += Data_buffer[n] * cosf(angle);
            im -= Data_buffer[n] * sinf(angle);
        }
        mag[idx] = sqrtf(re * re + im * im);
    }

    // 3. 找出主峰 m 和次峰方向
    int max_i = (mag[0] > mag[1]) ? 0 : 1;
    max_i = (mag[max_i] > mag[2]) ? max_i : 2;
    int m = k0 + max_i;

    float main_mag = mag[max_i];
    float side_mag;
    int side_sign;  // 1表示次峰在右，-1表示在左
    if (max_i == 0)
    {
        side_mag = mag[1];  // 没有左边，次峰就是右边
        side_sign = 1;
    }
    else if (max_i == 2)
    {
        side_mag = mag[1];  // 没有右边，次峰就是左边
        side_sign = -1;
    }
    else
    { // max_i == 1
        if (mag[0] > mag[2])
        {
            side_mag = mag[0];
            side_sign = -1;
        }
        else
        {
            side_mag = mag[2];
            side_sign = 1;
        }
    }

    // 4. 幅度比插值求偏移 δ
    float r = side_mag / main_mag;
    float delta = side_sign * r / (1.0f + r);

    waveParam.frequency = (m + delta) * fs / N;

    // 5. 幅值恢复（sinc 修正）
    float sinc_val;
    if (fabsf(delta) < 1e-6f)
        sinc_val = 1.0f;
    else
    {
        float x = M_PI * delta;
        sinc_val = sinf(x) / x;
    }
    waveParam.amplitude = (2.0f * main_mag) / (N * sinc_val);
}