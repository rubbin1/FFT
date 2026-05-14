//
// Created by 35156 on 26-4-26.
//

#ifndef ZERO_CROSSING_AND_DFT_H
#define ZERO_CROSSING_AND_DFT_H

void Data_buffer_sin(float *buf);
float zero_crossing_raw(const float *samples, int len);
void precise_measure(float proboly_freq);

#endif //ZERO_CROSSING_AND_DFT_H
