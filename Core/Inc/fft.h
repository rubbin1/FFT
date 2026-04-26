//
// Created by 35156 on 26-4-25.
//

#ifndef FFT_H
#define FFT_H

void FFT_Init();
void generate_square_wave();
void fft_process_harmonics(float *freqs, float *ampls);
void capture_waveform();

extern float Data_buffer[];

#endif //FFT_H
