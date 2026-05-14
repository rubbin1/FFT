//
// Created by 35156 on 26-4-9.
//

#include "key_press.h"
#include "main.h"

//定义四个按键key0，key1，key2,key3
Key_TypeDef key0 = {0};
Key_TypeDef key1 = {0};
Key_TypeDef key2 = {0};
Key_TypeDef key3 = {0};

// 按键映射表
const KeyMap all_keys[] = {
    {&key0, KEY0_Pin, KEY0_GPIO_Port},
    {&key1, KEY1_Pin, KEY1_GPIO_Port},
    {&key2, KEY2_Pin, KEY2_GPIO_Port},
};
const uint8_t KEY_COUNT = sizeof(all_keys) / sizeof(all_keys[0]);

void Key_Press(Key_TypeDef *key, uint16_t GPIO_Pin, GPIO_TypeDef *GPIO_Port)
{
    int leval = HAL_GPIO_ReadPin(GPIO_Port, GPIO_Pin);
    uint32_t now_time = HAL_GetTick();

    if (leval != key->last_state)
    {
        //此时按键电平变化，记录此时间和电平高低
        key->last_state = leval;
        key->last_time = now_time;
    }

    //消抖20ms
    //短按判断
    if ((now_time - key->last_time) > 20)
    {
        if (leval == GPIO_PIN_RESET && key->current_state == 0)
        {
            //此时按键按下，现在电平为高，确定为短按
            key->current_state = 1;
            key->short_pressed_flag = 1;
            //对长按检测进行排除,长按开始时间、长按增加的初始时间都是now_time
            key->long_press_start_time = now_time;
            key->last_added_time = now_time;
            key->long_pressed_flag = 0;
        }
        //按键释放
        else if (leval == GPIO_PIN_SET && key->current_state == 1)
        {
            key->current_state = 0;
            key->long_pressed_flag = 0;
        }
    }

    //长按判断
    if (key->current_state == 1)
    {
        if (key->long_pressed_flag == 0)
        {
            if ((now_time - key->long_press_start_time) > 100)
            {
                key->long_pressed_flag = 1;
                key->last_added_time = now_time;
            }
        }
        else
        {
            if ((now_time - key->last_added_time) > 200)
            {
                key->last_added_time = now_time;
                key->short_pressed_flag = 1;
            }
        }
    }
}
