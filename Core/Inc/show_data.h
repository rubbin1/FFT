//
// Created by 35156 on 26-4-25.
//

#ifndef SHOW_DATA_H
#define SHOW_DATA_H
#include <stdint.h>

void show_data(float *buffer, uint16_t size);
void OLED_Show_sin_input(float freq, float ampl);
void OLED_Show_mul_input();

#endif //SHOW_DATA_H
