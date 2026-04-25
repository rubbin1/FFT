//
// 此文件内函数用于串口发送数据，以及在OLED屏幕上显示相应的东西
//

#include "show_data.h"
#include <stdint.h>
#include <stdio.h>

void show_data(float *buffer, uint16_t size)
{
    for (int i = 0; i < size; i++)  printf("%.4f\n", buffer[i]);
}
