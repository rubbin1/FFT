//
// Created by 35156 on 26-5-15.
//

#include "../Inc/error_type_dispaly.h"

#include "oled.h"
#include "private_typedef.h"

void Error_Type(void)
{
    switch (display_state.error_type)
    {
        case 0:
            break;
        case 1:
            //此时错误为：非正弦输入时，测得的基波频率<=0
            OLED_NewFrame();
            OLED_PrintASCIIString(0, 0, "fundamental frequency <= 0", &afont12x6, OLED_COLOR_NORMAL);
            OLED_ShowFrame();
            break;
    }
}
