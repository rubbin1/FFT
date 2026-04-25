//
//本文件中函数用于进行fft算法
//
#include "fft.h"
#include <stdio.h>
#include "arm_math.h"
#include "math.h"
#include "show_data.h"
#include "window_table.h"

#define FFT_LEN 1024

//模拟信号频率
float moni_freq = 1000.f;
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

//模拟出一个方波
void generate_square_wave()
{
    for (int i = 0; i < FFT_LEN; i++)
    {
        float phase = 2.0f * M_PI * moni_freq * i / sample_rate;
        float val = (sinf(phase) >= 0) ? 1.0f : -1.0f;   // 幅值 ±1
        Data_buffer[2 * i]     = val * FlapTop_Window[i];  // 加窗（可选）
        Data_buffer[2 * i + 1] = 0;
    }
}

void fft_process(float *freq_out, float *ampl_out)
{
     //1.FFT计算
     arm_cfft_f32(&scfft, Data_buffer, 0, 1);
     //2.计算复数幅度
     arm_cmplx_mag_f32(Data_buffer, Data_buffer, FFT_LEN);
     //3.幅值校准
     for (int i = 0; i < FFT_LEN / 2; i++)
     {
         Data_buffer[i] = Data_buffer[i] * 2.0f / (FFT_LEN * 0.26526f);
     }
     //4.搜索峰值
     float max_val;
     uint32_t max_pos;
     arm_max_f32(&Data_buffer[1], FFT_LEN/2 - 1, &max_val, &max_pos);
     max_pos += 1;   // 恢复真正的索引
    //5.三点抛物线插值，消除栅栏效应
     float y1 = Data_buffer[max_pos - 1];
     float y2 = Data_buffer[max_pos];
     float y3 = Data_buffer[max_pos + 1];

     float delta = 0.5f * (y1 - y3) / (y1 - 2.0f * y2 + y3);

     *freq_out = (max_pos + delta) * sample_rate / (float)FFT_LEN;
     *ampl_out = max_val;
}
