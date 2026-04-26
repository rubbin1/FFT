//
//本文件中函数用于进行fft算法
//
#include "fft.h"
#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "window_table.h"

#define FFT_LEN 2048

//模拟信号频率
float moni_freq = 200.f;
float sample_rate = 20000.f;        //采样率
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

//模拟出一个方波
void generate_square_wave()
{
    for (int i = 0; i < FFT_LEN; i++)
    {
        float phase = 2.0f * M_PI * moni_freq * i / sample_rate;
        float val = (sinf(phase) >= 0) ? 1.0f : -1.0f;   // 幅值 ±1
        Data_buffer[2 * i]     = val * FlatTop_Window[i];  // 加窗
        Data_buffer[2 * i + 1] = 0;
    }
}

void fft_process_harmonics(float *freqs, float *ampls)
{
    // 1.FFT和幅度计算
    arm_cfft_f32(&scfft, Data_buffer, 0, 1);
    arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);

    // 2.幅值校准
    for (int i = 0; i < FFT_LEN / 2; i++)
        Data_buffer[i] *= 2.0f / (FFT_LEN * 0.26526f);

    // 3.寻找基波
    float max_val;
    uint32_t max_pos;
    arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_val, &max_pos);
    max_pos += 1;

    // 4.基波精确频率和幅值
    float y1 = Data_buffer[max_pos - 1];
    float y2 = Data_buffer[max_pos];
    float y3 = Data_buffer[max_pos + 1];
    float delta = 0.5f * (y1 - y3) / (y1 - 2.0f * y2 + y3);
    freqs[0] = (max_pos + delta) * sample_rate / FFT_LEN;
    ampls[0] = max_val;

    float f0 = freqs[0];                    // 基波频率
    float df = sample_rate / FFT_LEN;       // 频率分辨率

    // 5.搜索 2~5 次谐波
    for (int k = 2; k <= 5; k++)
    {
        float target_freq = k * f0;
        int center_idx = (int)(target_freq / df + 0.5f);
        // 限制搜索范围
        if (center_idx < 1) center_idx = 1;
        if (center_idx > FFT_LEN/2 - 2) center_idx = FFT_LEN/2 - 2;

        float harm_max = Data_buffer[center_idx];
        int harm_pos = center_idx;
        for (int i = center_idx - 1; i <= center_idx + 1; i++)
        {
            if (Data_buffer[i] > harm_max)
            {
                harm_max = Data_buffer[i];
                harm_pos = i;
            }
        }
        // 抛物线插值 (与基波完全相同)
        float h_y1 = Data_buffer[harm_pos - 1];
        float h_y2 = Data_buffer[harm_pos];
        float h_y3 = Data_buffer[harm_pos + 1];
        float h_delta = 0.5f * (h_y1 - h_y3) / (h_y1 - 2.0f * h_y2 + h_y3);

        freqs[k-1] = (harm_pos + h_delta) * sample_rate / FFT_LEN;
        ampls[k-1] = harm_max;
    }
}
