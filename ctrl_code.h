#ifndef CTRL_CODE_H_

#define CTRL_CODE_H_
#include "Arduino.h"
#include "Ticker.h"
#include <EEPROM.h>
#include "./c_types.h"

// #define latchPin 1
// #define clockPin 2
// #define dataPin 3

#define oxygen 4  // LED灯控制引脚
#define pump 5  // 表示大USB上面
#define flower1 6  // 表示大USB下面  pump_supply
// #define flower1 D8  // 表示大USB下面  pump_supply

const uint8 leisure_time_interval = 10; // 空闲时间
const uint8 work_time_interval = 1; // 工作时间
const uint16_t night_start = 22*60+30; // 供氧停止工作时间--开始22:30
const uint16_t night_end = 8*60+30; // 供氧停止工作时间--开始08:30
const uint16_t day_end = 24*60; // 24:00的时候min_cnt需要归0的
const uint8_t led1_pin_num = 6;  // 这里指的是第几位
const uint8_t led2_pin_num = 5;
const uint8_t led3_pin_num = 4;
const uint8_t led4_pin_num = 3;
const uint8_t relay_unknown_pin_num = 2;
const uint8_t relay_flower_pin_num = 1;
const uint8_t relay_pump_pin_num = 7;
const uint8_t oxy_gen_pin_num = 0;

// typedef struct struct_a
// { int x;}a;
typedef struct ctrl_code_flag
{
    bool pump_flag = false;
    bool oxgen_flag = false;
    bool led_flag = false;

} ctrl_code_flag;

typedef struct data_32
{
    uint8 first = 0;
    uint8 second = 0;
    uint8 three = 0;
    uint8 four = 0;
} data_32;

struct Witharray{
	int a[5];
};
// ctrl_code_flag *ccf = (ctrl_code_flag *)malloc(sizeof(ctrl_code_flag));

void setup_pin_mode(void);
// void oxygen_supply(bool open_flag);
// void pump_supply(bool open_flag);
// void led_supply(bool open_flag);
void setup_ticker(void);
void cnt_minute(void);
ctrl_code_flag ctrl_strategy(uint16_t current_time);
void ctrl_strategy_ntc(uint16_t current_time);
bool ctrl_led_indicator(uint16_t current_time);
// void out_put_595(uint8_t & data_flag);
void change_supply(bool open_flag, uint8_t & data_flag, const uint8_t pin_num);
void setBit(uint8_t & input,const unsigned char n);
void clearBit(uint8_t & input,const unsigned char n);
void print_bin_data(uint8_t data, uint8_t num_bit);
void change_supply(bool open_flag, const uint8_t pin_number);
char* get_cmd(char* cmd_pre, char* buff);
char* get_cmd(String cmd_pre, char* buff, char sep);
int e2prom_write_str(char* buff, int start_add);
char * read_value(int start_addr, int size);
bool save_setting(char* buff);
void change_supply(bool open_flag, const uint8_t pin_number, const bool esp32_flag);

// char * read_setting(char * buff); //有问题找不出来！
// char * read_setting2(char * buff);
void read_setting(void);
uint32 read_eeprom_32(int start_address);
bool write_eeprom_32(uint32 cnt, int start_address);
data_32 transfer_32_8(uint32 cnt);
void clear_eeprom();
void shine_num(uint8 cnt);
void shine_num(uint8 cnt, uint8 interval);
uint8_t read_eeprom_8(int start_address);
bool write_eeprom_8(uint8_t cnt, int start_address);

#endif

