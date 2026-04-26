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
#define WAVE_DISP_MAX 150

//模拟信号频率
float moni_freq = 200.f;
float sample_rate = 10000.f;        //采样率
float Data_buffer[FFT_LEN * 2] = {0};
/*若为纯正弦输入时，Data_buffer为数据采样序列
 *若为非正弦输入，则为FFT数据数组
 */

float FFT_mag_max = 0;
uint32_t FFT_mag_max_index = 0;
float FFT_freq = 0;
float FFT_ampl = 0;

//图像绘制前，把1024个实数采样点存入如下数组中
float Wave_Buffer[FFT_LEN] = {0};
//图像绘制时所用的采样点数组
float Wave_Disp[WAVE_DISP_MAX];
int wave_len = 0;

arm_cfft_instance_f32 scfft;

void FFT_Init()
{
    arm_cfft_init_f32(&scfft, FFT_LEN);
}

//模拟出一个方波，并且传入Data_buffer数组
void generate_square_wave()
{
    for (int i = 0; i < FFT_LEN; i++)
    {
        float phase = 2.0f * M_PI * moni_freq * i / sample_rate;
        float val = (sinf(phase) >= 0) ? 1.0f : -1.0f;
        Wave_Buffer[i] = val;
        Data_buffer[ 2 * i] = val;
        Data_buffer[ 2 * i + 1] = 0;
    }
}

//利用过零检测，保存一个周期波形
void capture_waveform()
{
    // 1. 先从 Data_buffer 的实部拷贝原始采样数据（2048 个 float）
    //    过零检测需要这段数据
    float prob_freq = zero_crossing_raw(Wave_Buffer, FFT_LEN, sample_rate);
    if (prob_freq < 50.0f) prob_freq = 50.0f;   // 限制范围
    if (prob_freq > 200.0f) prob_freq = 200.0f;

    int N_cycle = (int)(sample_rate / prob_freq + 0.5f);
    if (N_cycle > WAVE_DISP_MAX) N_cycle = WAVE_DISP_MAX;
    if (N_cycle > FFT_LEN) N_cycle = FFT_LEN;

    // 2. 拷贝一个周期的数据到 Wave_Disp（只取实部，因 Data_buffer[0], [2], [4]... 是实部）
    for (int i = 0; i < N_cycle; i++)
    {
        Wave_Disp[i] = Data_buffer[2 * i];   // 实部
    }
    wave_len = N_cycle;
}

void fft_process_harmonics(float *freqs, float *ampls)
{
    // 1.给Data_buffer数组加窗，并且进行FFT和幅度计算
    for (int i = 0; i < FFT_LEN; i++)
    {
        Data_buffer[2 * i] = Data_buffer[2 * i] * FlatTop_Window[i];
    }
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
        freqs[k-1] = k * f0;   // 用基频的整数倍作为谐波频率

        int center_idx = (int)((k * f0) / df + 0.5f);
        if (center_idx < 1) center_idx = 1;
        if (center_idx > FFT_LEN/2 - 2) center_idx = FFT_LEN/2 - 2;

        float harm_max = Data_buffer[center_idx];
        for (int i = center_idx - 1; i <= center_idx + 1; i++)
        {
            if (Data_buffer[i] > harm_max)
                harm_max = Data_buffer[i];
        }
        ampls[k-1] = harm_max;
    }
}