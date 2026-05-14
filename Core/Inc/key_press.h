//
// Created by 35156 on 26-4-9.
//

#ifndef KEY_PRESS_H
#define KEY_PRESS_H

#include "main.h"

//定义按键状态结构体，每一个按键一个结构体
typedef struct {
    uint8_t  current_state;             // 当前稳定电平：0=松开，1=按下
    uint8_t  last_state;                // 上一次读取的原始电平
    uint32_t last_time;                 // 上次电平变化的时间戳
    uint8_t  short_pressed_flag;        // 短按触发标志
    uint8_t  long_pressed_flag;         // 长按触发标志
    uint32_t  long_press_start_time;     // 长按开始时间
    uint32_t  last_added_time;           // 上次连续增加开始的时间
} Key_TypeDef;
extern  Key_TypeDef key0;
extern  Key_TypeDef key1;
extern  Key_TypeDef key2;
extern  Key_TypeDef key3;

//定义一个按键表
typedef struct {
    Key_TypeDef *key;
    uint16_t pin;
    GPIO_TypeDef *port;
} KeyMap;
extern const KeyMap all_keys[];
extern const uint8_t KEY_COUNT;

void Key_Press(Key_TypeDef *key, uint16_t GPIO_Pin, GPIO_TypeDef *GPIO_Port);

#endif //KEY_PRESS_H
