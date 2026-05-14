//
//本文件中函数用于进行fft算法
//
#include "fft.h"
#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "window_table.h"
#include "zero_crossing_and_dft.h"

#define FFT_LEN 1024

float sample_rate = 10000.f;        //采样率
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

void Data_buffer_nosin(float *buf)
{
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] = buf[FFT_LEN + i];
        Data_buffer[2 * i + 1] = 0;
    }
}

void fft_process_harmonics(float *freqs, float *ampls)
{
    // 1. 加窗 + FFT + 幅度
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] = Data_buffer[2 * i] * FlatTop_Window[i];
    }
    arm_cfft_f32(&scfft, Data_buffer, 0, 1);
    arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);

    // 2. 幅值校准
    for (int i = 0; i < FFT_LEN / 2; i++)
        Data_buffer[i] *= 2.0f / (FFT_LEN * 0.26526f);  // 确认窗的相干增益

    // 3. 寻找基波最大谱线（跳过直流）
    float max_val;
    uint32_t max_pos;
    arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_val, &max_pos);
    max_pos += 1;

    // 4. 五点最小二乘抛物线插值，求基波精确频率
    if (max_pos < 2) max_pos = 2;
    if (max_pos > FFT_LEN/2 - 3) max_pos = FFT_LEN/2 - 3;

    float ym2 = Data_buffer[max_pos - 2];
    float ym1 = Data_buffer[max_pos - 1];
    float y0  = Data_buffer[max_pos];
    float yp1 = Data_buffer[max_pos + 1];
    float yp2 = Data_buffer[max_pos + 2];

    float num = -2.0f * ym2 - ym1 + yp1 + 2.0f * yp2;
    float den = 2.0f * (ym2 - ym1 - 2.0f * y0 - yp1 + yp2);
    float delta = (fabsf(den) > 1e-12f) ? (num / den) : 0.0f;

    freqs[0] = (max_pos + delta) * sample_rate / FFT_LEN;
    ampls[0] = max_val;   // 基波幅值（平顶窗下可直接用最大谱线）

    float f0 = freqs[0];
    float df = sample_rate / FFT_LEN;

    // 5. 搜索 2~5 次谐波
    for (int k = 2; k <= 5; k++)
    {
        freqs[k-1] = k * f0;                 // 谐波频率 = k * 基频

        // 计算谐波对应的谱线中心索引
        int center_idx = (int)((k * f0) / df + 0.5f);
        if (center_idx < 1) center_idx = 1;
        if (center_idx > FFT_LEN/2 - 2) center_idx = FFT_LEN/2 - 2;

        // 取 center_idx 和相邻两根谱线中的最大值作为谐波幅值
        float harm_max = Data_buffer[center_idx];
        for (int i = center_idx - 1; i <= center_idx + 1; i++)
        {
            if (Data_buffer[i] > harm_max)
                harm_max = Data_buffer[i];
        }
        ampls[k-1] = harm_max;
    }
}