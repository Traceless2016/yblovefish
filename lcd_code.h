#ifndef LCD_CODE_H_

#define LCD_CODE_H_

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "c_types.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
// https://blog.csdn.net/weixin_45488643/article/details/106029569?utm_term=nodemcuspi&utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~sobaiduweb~default-2-106029569&spm=3001.4430
#define OLED_MOSI   MOSI // OLED 的 D1 脚，在 SPI 和 IIC 通信中为数据管脚
#define OLED_CLK   SCK // D0引脚，在 SPI 和 IIC 通信中为时钟管脚
#define OLED_DC    SS // OLED 的 D/C#E 脚，数据和命令控制管脚

// #define OLED_CS    D6 // OLED 的 CS#脚，也就是片选管脚
#define OLED_CS    A6 // OLED 的 CS#脚，也就是片选管脚

// #define OLED_RESET D7 // OLED 的 RES#脚，用来复位（低电平复位）
#define OLED_RESET A7 // OLED 的 RES#脚，用来复位（低电平复位）


void set_up_oled(void);
// void draw_string(const String text_string, int8 cursor[2], int8 text_size);
void draw_string(const String &text_string, int8 cursor[2], int8 text_size);

// void com_str(String base_str, uint16_t num);
String com_str(String  base_str, uint16_t num);
String com_str(const String base_str, bool flag_bool);
// void draw_ctrl_interface(uint16_t min_cnt );
void draw_ctrl_interface(uint16_t min_cnt, const String real_time_str);
void draw_real_time(const String real_time_str);
void testdrawline();
void draw_num_test(void);
void draw_bool_status(const String &status_name, bool bool_flag, int8 cursor[2], int8 base_text_size);
extern Adafruit_SSD1306 display;

# endif