//
// Created by 35156 on 26-4-25.
//

#ifndef FFT_H
#define FFT_H

void FFT_Init();
void Data_buffer_fft(float *buf);

void fft_process_harmonics_first();
void fft_process_harmonics_second();

void fft_process_sin_first(void);
void fft_process_sin_second(void);

extern float Data_buffer[];

#endif //FFT_H
