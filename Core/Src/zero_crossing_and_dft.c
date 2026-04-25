/*
 *本文件用于在单纯输入正弦信号时，不用fft算法进行繁杂的计算
 *而是采用过零检测粗测频率，再对粗略频率附近的三点DFT准确求出频率和幅值
 */

#include "zero_crossing_and_dft.h"
#include "fft.h"

#define ZERO_CROSSING_LEN 1024          //定义过零检测采样数为1024

extern float sample_rate;        //采样率
