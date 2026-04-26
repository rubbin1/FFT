//
// Created by 35156 on 26-4-25.
//

#ifndef SHOW_DATA_H
#define SHOW_DATA_H
#include <stdint.h>

void OLED_Show_sin_input(float freq, float ampl);
void OLED_Show_mul_input(float *freqs, float *ampls, int pages);

#endif //SHOW_DATA_H
