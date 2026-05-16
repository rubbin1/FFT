//
//本文件中函数用于进行fft算法
//
#include "fft.h"
#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "window_table.h"
#include "private_typedef.h"

#define FFT_LEN 1024

extern const SystemConfig system_config;
extern HarmonicsResult harmonicsResult;

float Data_buffer[FFT_LEN * 2] = {0};
/*若为纯正弦输入时，Data_buffer为数据采样序列
 *若为非正弦输入，则为FFT数据数组
 */

float FFT_mag_max = 0;
uint32_t FFT_mag_max_index = 0;
float FFT_freq = 0;
float FFT_ampl = 0;

arm_cfft_instance_f32 scfft;

void FFT_Init()
{
    arm_cfft_init_f32(&scfft, FFT_LEN);
}

void Data_buffer_fft(float *buf)
{
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] = buf[FFT_LEN + i];
        Data_buffer[2 * i + 1] = 0;
    }
}

void fft_process_harmonics()
{
    // 加汉宁窗
    for (int i = 0; i < FFT_LEN; i++) {
        Data_buffer[2 * i] *= Hanning_Window[i];
    }
    arm_cfft_f32(&scfft, Data_buffer, 0, 1);
    arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);
    float calib = 4.0f / FFT_LEN;
    for (int i = 0; i < FFT_LEN / 2; i++) {
        Data_buffer[i] *= calib;
    }

    // 基波频率和幅值（三点插值）
    float max_val;
    uint32_t max_pos;
    arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_val, &max_pos);
    max_pos += 1;
    if (max_pos < 1) max_pos = 1;
    if (max_pos > FFT_LEN/2 - 2) max_pos = FFT_LEN/2 - 2;
    float y1 = Data_buffer[max_pos - 1];
    float y2 = Data_buffer[max_pos];
    float y3 = Data_buffer[max_pos + 1];
    float denom = 2.0f * (2.0f * y2 - y1 - y3);
    float delta = (fabsf(denom) > 1e-12f) ? (y3 - y1) / denom : 0.0f;
    float frequency = (max_pos + delta) * system_config.adc_sample_rate / FFT_LEN;
    // 基波幅值（用插值后的峰值）
    float fundamental_amplitude = y2 - 0.25f * (y1 - y3) * delta; // 汉宁窗三点插值幅值公式
    harmonicsResult.fundamental.frequency = frequency;
    harmonicsResult.fundamental.amplitude = fundamental_amplitude;

    float df = system_config.adc_sample_rate / FFT_LEN;

    // 谐波幅值（三点插值）
    for (int k = 2; k <= 5; k++) {
        float harmonic_freq = k * frequency;
        float idx_float = harmonic_freq / df;
        int center_idx = (int)roundf(idx_float);
        if (center_idx < 1) center_idx = 1;
        if (center_idx > FFT_LEN/2 - 2) center_idx = FFT_LEN/2 - 2;
        float y1_h = Data_buffer[center_idx - 1];
        float y2_h = Data_buffer[center_idx];
        float y3_h = Data_buffer[center_idx + 1];
        float denom_h = 2.0f * (2.0f * y2_h - y1_h - y3_h);
        float delta_h = (fabsf(denom_h) > 1e-12f) ? (y3_h - y1_h) / denom_h : 0.0f;
        float harmonic_amplitude = y2_h - 0.25f * (y1_h - y3_h) * delta_h;
        harmonicsResult.harmonics[k-2].frequency = harmonic_freq;
        harmonicsResult.harmonics[k-2].amplitude = harmonic_amplitude;
    }
}

void fft_process_sin_first(void)
{
    float amplitude_flat;
    // ===== 第一次 FFT：平顶窗，用于幅值 =====
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] *= FlatTop_Window[i];
    }
    arm_cfft_f32(&scfft, Data_buffer, 0, 1);
    arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);
    // 幅值校准（平顶窗相干增益 0.26526）
    for (int i = 0; i < FFT_LEN / 2; i++)
    {
        Data_buffer[i] *= 2.0f / (FFT_LEN * 0.26526f);
    }
    // 寻找最大谱线（幅值）
    float max_ampl;
    uint32_t max_pos_ampl;
    arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_ampl, &max_pos_ampl);
    max_pos_ampl += 1;
    amplitude_flat = max_ampl;   // 平顶窗幅值

    waveParam.amplitude = amplitude_flat;
}

void fft_process_sin_second(void)
{
    float frequency_hann;
    // 加汉宁窗
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] *= Hanning_Window[i];
    }
    arm_cfft_f32(&scfft, Data_buffer, 0, 1);
    arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);
    // 幅值校正（汉宁窗）
    float calib = 4.0f / FFT_LEN;
    for (int i = 0; i < FFT_LEN / 2; i++)
    {
        Data_buffer[i] *= calib;
    }
    // 寻找最大谱线（用于插值）
    float max_val;
    uint32_t max_pos_freq;
    arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_val, &max_pos_freq);
    max_pos_freq += 1;
    // 三点抛物线插值
    if (max_pos_freq < 1) max_pos_freq = 1;
    if (max_pos_freq > FFT_LEN/2 - 2) max_pos_freq = FFT_LEN/2 - 2;
    float y1 = Data_buffer[max_pos_freq - 1];
    float y2 = Data_buffer[max_pos_freq];
    float y3 = Data_buffer[max_pos_freq + 1];
    float denom = 2.0f * (2.0f * y2 - y1 - y3);
    float delta = (fabsf(denom) > 1e-12f) ? (y3 - y1) / denom : 0.0f;
    frequency_hann = (max_pos_freq + delta) * system_config.adc_sample_rate / FFT_LEN;

    // 组合最佳结果
    waveParam.frequency = frequency_hann;
}