//
// Created by 35156 on 26-4-25.
//

#ifndef FFT_H
#define FFT_H

void FFT_Init();
void Data_buffer_nosin(float *buf);
void fft_process_harmonics();

extern float Data_buffer[];
// Wave data now read from Data_buffer's real parts (Data_buffer[2*i])

#endif //FFT_H
