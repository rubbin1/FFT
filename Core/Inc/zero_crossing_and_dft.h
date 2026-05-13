//
// Created by 35156 on 26-4-26.
//

#ifndef ZERO_CROSSING_AND_DFT_H
#define ZERO_CROSSING_AND_DFT_H

void Data_buffer_sin(float *buf);
float zero_crossing_raw(const float *samples, int len, float fs);
void precise_measure(float proboly_freq, float *exact_freq_out, float *exact_ampl_out);

#endif //ZERO_CROSSING_AND_DFT_H
