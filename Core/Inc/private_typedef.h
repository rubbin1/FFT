//
// Created by 35156 on 26-5-15.
//

#ifndef PRIVATE_TYPEDEF_H
#define PRIVATE_TYPEDEF_H
#include <stdint.h>

//1. 系统模式
//输入模式
typedef enum
{
    SINGLE_WAVE_Input,
    MULTI_WAVE_Input,
  }Input_Mode;

//是否进行图像显示
typedef enum
{
    IMAGE_MODE_ON,
    IMAGE_MODE_OFF,
  }IMAGE_MOD;

typedef struct
{
    Input_Mode mode;
    IMAGE_MOD imageMod;
    int harmonic_pages;     //用于控制非正弦输入时，OLED屏幕页数的变量
    int error_type;         //错误码
}Display_State;
extern Display_State display_state;

//2. 系统常用参数结构体
typedef struct
{
    float adc_sample_rate; // 10kHz
    int fft_len; // 1024
    int adc_buffer_size; // 1024
    float adc_vref;     //3.3V
    uint16_t adc_resolution;    // ADC分辨率（满量程码值，通常为4096）
}SystemConfig;
extern const SystemConfig system_config;

//3. 测量结果结构体
//纯正弦情况
typedef struct
{
    float frequency;
    float amplitude;
}WaveParam;
extern WaveParam waveParam;
//非正弦情况
typedef struct
{
    WaveParam fundamental;
    WaveParam harmonics[4];   //2~5次谐波
} HarmonicsResult;
extern HarmonicsResult harmonicsResult;

//4. ADC数据缓冲区及其状态结构体
#define ADC_BUFFER_SIZE 1024
typedef struct
{
    uint16_t raw[ADC_BUFFER_SIZE];      //adc输入的数组
    uint16_t snapshot[ADC_BUFFER_SIZE]; //adc快照数组，TIM3每次到达一个周期，把raw中的数同步至snapshot中，防止adc数据丢失
    volatile uint8_t data_ready;        //raw到snapshot传递成功标志
} AdcBuffers_flag;
extern AdcBuffers_flag adcbuf_flag;


#endif //PRIVATE_TYPEDEF_H
