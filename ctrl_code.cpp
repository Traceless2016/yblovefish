/*
USB电源控制程序：
控制逻辑：
node mcu控制三个USB口：
首先，夜里需要停止水泵和供养（23:00~7:00），在7:00~23:00每隔leisure_time_interval分钟
进行供氧work_time_interval分钟，另外在该时间段一直进行水体清洁；此外，如果红外人体检测模块的
输出引脚为高电平，则打开照明灯。

*/

#include "ctrl_code.h"
// 工作模式
char *model_str = (char *)"Model";
char const *auto_model = "Auto"; // 0
char const *manu_model = "Hand"; // 1
char const *interval = "Inter";
char const *irri_time = "Time";
char const *humi_thr = "Humid";
/*
如果对象不是针对，它们没有区别
int const x = 3;
const int x = 3;
如果对象是指针，它们有区别
int* const p = &array: 指针p不能够指向其他地址
const int* p = &array: 指针p只读&array，不能够对其进行修改
https://www.cnblogs.com/yaos/p/12099521.html
*/
int const model_addr = 0;      // "Model:0"
int const inter_addr = 7;      // 间隔时间 "Model:0"+"Inter:20" 7+7
int const work_time_addr = 14; // 湿度阈值 "Model:0"+"Inter:20"+"Time:20" 7+7+6
int const humi_thr_addr = 20;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+6+7

int const model_size = 7;
int const inter_size = 7;
int const work_time_size = 6; // 湿度阈值 "Model:0"+"Inter:20"+"Time:20" 7+7+6
int const humi_thr_size = 7;

uint16_t min_cnt = 0; // 初始值为0
Ticker scheduledTicker;
uint8_t back_data_595 = 0b00000000;
#define EEPROM_SIZE 64

// 控制引脚定义

// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t LED_ctrl = D0;  //控制LED照明
// static const uint8_t oxygen_ctrl = D1;  //控制供氧引脚
// static const uint8_t motor_ctrl = D2; //控制电机
void shine_num(uint8 cnt)
{

    for (size_t i = 0; i < cnt; i++)
    {
        /* code */
        change_supply(true, LED_BUILTIN, true);
        delay(1e3);
        change_supply(false, LED_BUILTIN, true);
        delay(1e3);
    }
}

void shine_num(uint8 cnt, uint8 interval)
{
    for (size_t i = 0; i < cnt; i++)
    {
        /* code */
        change_supply(true, LED_BUILTIN, true);
        delay(interval);
        change_supply(false, LED_BUILTIN, true);
        delay(interval);
    }
}

void setup_pin_mode(void)
{
    uint8_t data_595 = 0b00000000;
    pinMode(pump, OUTPUT);
    pinMode(oxygen, OUTPUT);
    pinMode(flower1, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(IN2, OUTPUT);
    change_supply(false, pump);
    change_supply(false, oxygen);
    change_supply(false, flower1);
    change_supply(false, LED_BUILTIN);
}

/******************2021年12月2日************************/
// void out_put_595(uint8_t &data_flag)
// {
//     digitalWrite(latchPin, LOW);
//     shiftOut(dataPin, clockPin, MSBFIRST, data_flag); // 7 6 5 4 3 2 1 15(小数点)---数值指的是引脚哦
//     digitalWrite(latchPin, HIGH);
// }

void change_supply(bool open_flag, uint8_t &data_flag, const uint8_t pin_num)
{
    // if(open_flag) *data_flag = * data_flag | 0b00000000;  //对应位置1
    // else
    // relay_oxygen 为QA--15引脚
    // shiftOut(dataPin, clockPin, MSBFIRST, 0b11111110); //7 6 5 4 3 2 1 15(小数点)---数值指的是引脚哦

    if (open_flag)
        setBit(data_flag, pin_num);
    else
        clearBit(data_flag, pin_num);
}

void change_supply(bool open_flag, uint8_t &data_flag, const uint8_t pin_num, const bool esp32_flag)
{
    // if(open_flag) *data_flag = * data_flag | 0b00000000;  //对应位置1
    // else
    // relay_oxygen 为QA--15引脚
    // shiftOut(dataPin, clockPin, MSBFIRST, 0b11111110); //7 6 5 4 3 2 1 15(小数点)---数值指的是引脚哦
    if (esp32_flag)
    {
        if (open_flag)
            digitalWrite(pin_num, HIGH);
        else
            digitalWrite(pin_num, LOW);
    }
    else
    {
        if (open_flag)
            setBit(data_flag, pin_num);
        else
            clearBit(data_flag, pin_num);
    }
}

void setBit(uint8_t &input, const unsigned char n)
{
    input |= (uint8_t)(1 << n);
}
void clearBit(uint8_t &input, const unsigned char n)
{
    input &= ~(uint8_t)(1 << n);
}
/******************2021年12月2日************************/

void setup_ticker(void)
{
    // 设置定时器
    scheduledTicker.attach(60, cnt_minute); // 每隔1分钟调用1次cnt_minute函数
}

void cnt_minute(void)
{
    // 每隔1分钟min_cnt会加1
    min_cnt += 1;

    if (min_cnt >= day_end)
        min_cnt = 0;
    // Serial.println("cnt_minute");
}

ctrl_code_flag ctrl_strategy(uint16_t current_time)
{
    /*

        1. 水泵会一直运行(每隔10分钟，会供水一分钟)；
        2. 供氧设备只在白天（8:30~22:30）；
        3. 根据按键来开启灯光；
        未做的事情：
        1. 加入液晶显示；
        2. 加入人体感应模块；

    */

    ctrl_code_flag flag_struct;
    bool pump_flag = false;
    bool oxgen_flag = false;
    bool led_flag = false;
    uint8_t data_595 = 0b00000000;
    // current_time = 0;
    ctrl_led_indicator(current_time);
    int8_t ctrl_time_interval = 4;
    if (current_time % ctrl_time_interval == 0)
    {
        // 水泵设置
        pump_flag = true; // 打开水泵
        // 供氧设备
    }
    else if (current_time % ctrl_time_interval == 1)
    {
        // 水泵设置
        pump_flag = false; // 关闭水泵
    }
    else if (current_time % ctrl_time_interval == 2)
    {
        // Serial.println("current_time==2");
        Serial.println("night_end is");
        Serial.println(night_end);
        Serial.println("night_start");
        Serial.println(night_start);

        if (night_end <= current_time and current_time <= night_start)
        {
            Serial.println("not at night!!");
            oxgen_flag = true; // 不是在夜间
        }
        else
        {
            Serial.println("at night!!");
            oxgen_flag = false;
        }
    }
    else if (current_time % ctrl_time_interval == 3)
    {
        // 供氧设备
        oxgen_flag = false; // 关闭供氧
    }
    led_flag = ctrl_led_indicator(current_time);
    // 测试时使用--start
    Serial.println("pump_flag:");
    Serial.println(pump_flag);
    Serial.println("oxgen_flag");
    Serial.println(oxgen_flag);
    Serial.println("led_flag");
    Serial.println(led_flag);
    // 测试时使用--end

    // pump_supply(pump_flag);

    // pump_flag = true;
    // oxgen_flag = true;
    // led_flag = true;

    // change_supply(oxgen_flag, data_595, oxy_gen_pin_num);
    // change_supply(pump_flag, data_595, relay_pump_pin_num);
    // change_supply(led_flag, data_595, led1_pin_num);
    // change_supply(led_flag, data_595, led2_pin_num);
    // change_supply(led_flag, data_595, led3_pin_num);
    // change_supply(led_flag, data_595, led4_pin_num);

    change_supply(pump_flag, pump);
    change_supply(oxgen_flag, oxygen);
    change_supply(led_flag, flower1);
    change_supply(led_flag, LED_BUILTIN);

    flag_struct.pump_flag = pump_flag;
    flag_struct.oxgen_flag = oxgen_flag;
    flag_struct.led_flag = led_flag;
    return flag_struct;
}

bool ctrl_led_indicator(uint16_t current_time)
{
    bool led_flag = false;

    if (current_time % 3 == 0)
    {
        // digitalWrite(LED_BUILTIN, LOW);
        // Serial.println("current_time is ");
        // Serial.println(current_time);
        // Serial.println("turn on");
        led_flag = false;
    }
    else
    {
        // Serial.println("current_time is ");
        // Serial.println(current_time);
        // Serial.println("turn off");

        //    digitalWrite(LED_BUILTIN, HIGH);
        led_flag = true;
    }

    return led_flag;
}

void print_bin_data(uint8_t data, uint8_t num_bit)
{
    //     int   nb = 10;
    //   int   i = 31;
    uint8_t n = data, i = 0;
    double D = 0;
    do
    {
        if (n % 2 == 1)
            D += pow(10, i); // D = (n % 2) * pow(10,i)
        n /= 2;              // n = n / 2
        i++;                 // i = i + 1
    } while (n != 0);        // 非 0，则重复“除 2 取余”转换
                             //  printf("二进制数序列是：%.0lf\n",D);
                             //  system("pause");
    Serial.println(D);
}

/***************2022年2月20日***********************/

void change_supply(bool open_flag, const uint8_t pin_number)
{
    // 供氧机
    if (pin_number == LED_BUILTIN)
    {
        /*LED_BUILTIN 好像低电平点亮*/
        if (open_flag)
            digitalWrite(pin_number, HIGH);
        else
            digitalWrite(pin_number, LOW);
    }
    else
    {
        if (open_flag)
            digitalWrite(pin_number, LOW);
        else
            digitalWrite(pin_number, HIGH);
    }
}

void change_supply(bool open_flag, const uint8_t pin_number, const bool esp32_flag)
{
    // 供氧机
    if (pin_number == LED_BUILTIN)
    {
        /*LED_BUILTIN 好像低电平点亮*/
        if (esp32_flag)
        {
            if (open_flag)
                digitalWrite(pin_number, HIGH);
            else
                digitalWrite(pin_number, LOW);
        }
        else
        {
            if (open_flag)
                digitalWrite(pin_number, LOW);
            else
                digitalWrite(pin_number, HIGH);
        }
    }
    else
    {
        if (open_flag)
            digitalWrite(pin_number, HIGH);
        else
            digitalWrite(pin_number, LOW);
    }
}

int checkSensor(uint8_t num, const uint8_t threshold)
{
    //   if (analogRead(0) > 400) {
    if (num > threshold)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*************** 2022年11月9日 ***********************/
bool test()
{
    unsigned long timeout = millis();
    if (millis() - timeout > 5000)
    { // 5秒
        Serial.println(">>> No data are received!!");
    }
    return true;
}

/*
根据服务器发过来的信息来做出相应的动作：
while(client.available())
{
  buff[nm++]=client.read();
  if(nm>=511)break;
}
1. Auto：自动(浇水)模式；---
*/

// void auto_irrigation(void){
//     unsigned long time

// }
char *get_cmd(char *cmd_pre, char *buff)
{
    // char* buff
    // if buff
    Serial.println("cmd_pre");
    Serial.println(cmd_pre);
    uint8 num = 0;
    char *results;
    while (1)
    {
        while (cmd_pre[num] != 0x00)
        {
            if (buff[num] == cmd_pre[num])
            {
                Serial.println(buff[num]);
                Serial.println(cmd_pre[num]);
                num++;
            }
            else
            {
                // 在到达0x00之前，存在(buff[num] != cmd_pre[num])的情况
                results = (char *)"error";
                Serial.println("e is :");
                Serial.println(num);
                Serial.println(buff[num]);
                Serial.println(cmd_pre[num]);
                break;
            }
        }
        if (buff[num] == ':') // 分隔符
        {
            num++;
            uint8 temp_num = 0;
            while (1)
            {
                if (buff[temp_num] != 0x00)
                {
                    delay(1);
                    results[temp_num] = buff[num];
                    temp_num++;
                    num++;
                    Serial.println(buff[num]);
                }
                else
                {
                    // 到达buff中的结尾字符
                    results[temp_num] = 0x00;
                    break;
                }
            }
        }
        else
        {
            results = (char *)"error2";
            break;
            // Serial.println(buff[num]);
        }
        break;
    }
    return results; // 返回最终的结果值
}

char *get_cmd(String cmd_pre, char *buff, char sep)
{
    // char* buff
    // if buff
    // sep 指cmd_pre与内容之间的间隔
    uint8 num = 0;
    char *results;
    while (1)
    {
        while (cmd_pre[num] != 0x00)
        {
            if (buff[num] == cmd_pre[num])
            {
                // Serial.println(buff[num]);
                // Serial.println(cmd_pre[num]);
                num++;
            }
            else
            {
                // 在到达0x00之前，存在(buff[num] != cmd_pre[num])的情况
                results = (char *)"error: received buff: ";
                Serial.println("error num is :");
                Serial.println(num);
                break;
            }
        }
        if (buff[num] == sep) // 分隔符
        {
            num++;
            uint8 temp_num = 0;
            while (1)
            {
                if (buff[temp_num] != 0x00)
                {
                    // Serial.println(buff[num]);
                    delay(1);
                    results[temp_num] = buff[num];
                    temp_num++;
                    num++;
                }
                else
                {
                    // 到达buff中的结尾字符
                    results[temp_num] = 0x00;
                    break;
                }
            }
        }
        else
        {
            results = (char *)"error: received buff: ";
            // Serial.println(buff[num]);
        }
        break;
    }
    return results; // 返回最终的结果值
}

// https://blog.csdn.net/amin_hui/article/details/115359358 字符串操作

void ctrl_irrigation(char *buff)
{
    char *rec_cmd = get_cmd((char *)"CMD", buff);
    // Serial.println("received cmd:");
    // Serial.println(rec_cmd);
    // 根据收到的命令来确定工作模式
    char *model_str = (char *)"Model";

    uint8 temp_num = 0;
    int temp_addr = 0;

    if (strncmp(auto_model, rec_cmd, 4) == 0)
    { // CMD:Auto->Auto
        // 写入控制模式为Auto
        // Serial.println("write Auto!");
        temp_num = 0;
        temp_addr = e2prom_write_str(model_str, model_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        EEPROM.write(temp_addr, 0);
    }
    else if (strncmp(manu_model, rec_cmd, 4) == 0) // CMD:Manu->Manu
    {
        // Serial.println("write manu!");
        temp_num = 0;
        temp_addr = e2prom_write_str((char *)"Model", model_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        EEPROM.write(temp_addr, 1);
    }
    else if (strncmp(interval, rec_cmd, 5) == 0)
    {
        /* 取出对应的数据，并写入到EEPROM中对应位置 */
        // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
        // Serial.println("write interval!");
        temp_num = 0;
        temp_addr = e2prom_write_str((char *)interval, inter_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    else if (strncmp(irri_time, rec_cmd, 4) == 0)
    {
        // Serial.println("write irrigation!");
        temp_num = 0;
        temp_addr = e2prom_write_str((char *)irri_time, work_time_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    else if (strncmp(humi_thr, rec_cmd, 5) == 0)
    {
        // Serial.println("write humi!");

        temp_num = 0;
        temp_addr = e2prom_write_str((char *)irri_time, humi_thr_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    if (EEPROM.commit())
    {
        Serial.println("cS!");
    }
    else
    {
        Serial.println("ce!");
    }
}

bool save_setting(char *buff)
{
    EEPROM.begin(EEPROM_SIZE);
    // Serial.println("save_setting cmd:");
    // Serial.println(buff);
    char *rec_cmd = get_cmd((char *)"CMD", buff);
    // Serial.println("received cmd:");
    // Serial.println(rec_cmd);
    // 根据收到的命令来确定工作模式
    char *model_str = (char *)"Model";
    uint8 temp_num = 0;
    int temp_addr = 0;
    // Serial.println("test strncmp");
    // int temp =  strncmp(auto_model, rec_cmd, 4);
    // Serial.println(temp, DEC);

    if (strncmp(auto_model, rec_cmd, 4) == 0)
    { // CMD:Auto->Auto
        // 写入控制模式为Auto
        // Serial.println("write Auto!");
        temp_num = 0;
        temp_addr = e2prom_write_str(model_str, model_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        EEPROM.write(temp_addr, '0');
    }
    else if (strncmp(manu_model, rec_cmd, 4) == 0) // CMD:Manu->Manu
    {
        // Serial.println("write manu!");
        temp_num = 0;
        temp_addr = e2prom_write_str((char *)"Model", model_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        EEPROM.write(temp_addr, '1');
    }
    else if (strncmp(interval, rec_cmd, 5) == 0)
    {
        /* 取出对应的数据，并写入到EEPROM中对应位置 */
        // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
        // Serial.println("write interval!");

        temp_num = 0;
        temp_addr = e2prom_write_str((char *)interval, inter_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    else if (strncmp(irri_time, rec_cmd, 4) == 0)
    {
        // Serial.println("write irrigation!");

        temp_num = 0;
        temp_addr = e2prom_write_str((char *)irri_time, work_time_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    else if (strncmp(humi_thr, rec_cmd, 5) == 0)
    {
        // Serial.println("write humi!");
        temp_num = 0;
        temp_addr = e2prom_write_str((char *)irri_time, humi_thr_addr);
        EEPROM.write(temp_addr, ':');
        temp_addr++;
        char *rec_value = get_cmd(interval, rec_cmd, '-');
        EEPROM.write(temp_addr, rec_value[0]);
    }
    else
    {
        Serial.println("nw!");
        Serial.println(buff);
        return false;
    }
    if (EEPROM.commit())
    {
        Serial.println("cS");
    }
    else
    {
        Serial.println("EC");
    }
    return true;
}

/*读取e2prom中的配置*/
char *read_value(int start_addr, int size)
{
    EEPROM.begin(EEPROM_SIZE);
    // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
    char *res = (char *)"Q";
    int temp_num = 0;
    int address = 0;
    byte value;
    char char_value = 'Q';
    for (int i = 0; i < size; i++)
    {
        /* 读取e2prom中的配置 */
        address = start_addr + i;
        // Serial.println(address);
        value = EEPROM.read(address);
        // Serial.print("\t");
        char_value = char(value);
        // Serial.println(char_value);
        // Serial.println(); // 默认输出缓冲器中的值
    }
    res[size] = 0x00;
    // Serial.println(res);
    return res;
}

void read_setting(void)
{
    // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
    // int temp_num = 0;
    char *model_res = read_value(model_addr, model_size);
    // Serial.println(model_res);
    char *inter_res = read_value(inter_addr, inter_size);
    // Serial.println(inter_res);

    char *work_time_res = read_value(work_time_addr, work_time_size);
    // Serial.println(work_time_res);

    char *humi_res = read_value(humi_thr_addr, humi_thr_size);
    // Serial.println(humi_res);
}

char *read_setting(char *buff)
{
    // 有问题
    // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
    char *cmd_pre = (char *)"R";
    Serial.println(cmd_pre); // 这一步输出的不是cmd_pre的值

    char *rec_cmd = get_cmd(cmd_pre, buff);
    Serial.println("rec_cmd:");
    Serial.println(rec_cmd);
    // 根据收到的命令来确定工作模式

    uint8 temp_num = 0;
    int temp_addr = 0;
    char *res;
    if (strncmp(model_str, rec_cmd, 4) == 0)
    { // CMD:Auto->Auto
        // 写入控制模式为Auto
        // Serial.println("read model!");
        temp_num = 0;
        res = read_value(model_addr, model_size);
    }
    else if (strncmp(interval, rec_cmd, 5) == 0)
    {
        /* 取出对应的数据，并写入到EEPROM中对应位置 */
        // int const humi_thr_addr = 21;  // 湿度阈值 "Model:0"+"Inter:20"+"Time:20"+"Humid:20" 7+7+7+7
        // Serial.println("read interval!");
        temp_num = 0;
        res = read_value(inter_addr, inter_size);
    }
    else if (strncmp(irri_time, rec_cmd, 4) == 0)
    {
        // Serial.println("read irrigation!");

        temp_num = 0;
        res = read_value(work_time_addr, work_time_size);
    }
    else if (strncmp(humi_thr, rec_cmd, 5) == 0)
    {
        // Serial.println("read humidation!");

        temp_num = 0;
        res = read_value(humi_thr_addr, humi_thr_size);
    }
    else
    {
        // 说明并没有要读取的项目
        Serial.println("Nr!");
        Serial.println(buff);
        return (char *)"false";
    }
    Serial.println();
    Serial.println(res);
    return res;
}

int e2prom_write_str(char *buff, int start_add)
{
    // 将buff里面的内容写入到 start_add的位置，并返回最后的位置
    // 注意最后结尾必须是0x00
    // EEPROM.begin(EEPROM_SIZE);
    int temp_num = 0;
    int temp_addr = start_add;
    Serial.println("S w");

    while (buff[temp_num] != 0x00)
    {
        delay(1);
        Serial.println(buff[temp_num]);

        EEPROM.write(temp_addr, buff[temp_num]); // 写数据，参数分别为地址&数据,写数据后需要通过,EEPROM.commit()或EEPROM.end()将数据保存到EEPROM；
        temp_addr++;
        temp_num++;
    }
    if (EEPROM.commit())
    {
        Serial.println("CS");
    }
    else
    {
        Serial.println("EC");
    }
    // EEPROM.commit(); //保存更改的数据
    // EEPROM.end(); //同EEPROM.commit();
    Serial.println("E w");
    return temp_addr;
}

data_32 transfer_32_8(uint32 cnt)
{
    // https://blog.csdn.net/MQ0522/article/details/110136850#:~:text=32%E4%BD%8D%E8%BD%AC8%E4%BD%8D%20%E5%B0%86%E4%B8%80%E4%B8%AA32%E4%BD%8D%E6%95%B0%E6%8D%AEdata_u32%E6%8B%86%E5%88%86%E6%88%904%E4%B8%AA8%E4%BD%8D%E6%95%B0%E6%8D%AEdata_u8%20%EF%BC%9A%20data_u8%20%3D%20%28data_u32%20%3E%3E%2024%29,0xff%3B%20data_u8%20%3D%20data_u32%20%26%200xff%3B%20%2F%2F%E9%AB%98%E4%BD%8D%E5%9C%A8%E5%89%8D%EF%BC%8C%E4%BD%8E%E4%BD%8D%E5%9C%A8%E5%90%8E%201
    data_32 cnt_struct;
    cnt_struct.first = (cnt >> 24) & 0xff;
    cnt_struct.second = (cnt >> 16) & 0xff;
    cnt_struct.three = (cnt >> 8) & 0xff;
    cnt_struct.four = cnt & 0xff;
    return cnt_struct;
}

bool write_eeprom_32(uint32 cnt, int start_address)
{
    EEPROM.begin(EEPROM_SIZE);
    data_32 cnt_struct = transfer_32_8(cnt);
    EEPROM.write(start_address, cnt_struct.first);
    EEPROM.write(start_address + 1, cnt_struct.second);
    EEPROM.write(start_address + 2, cnt_struct.three);
    EEPROM.write(start_address + 3, cnt_struct.four);
    if (EEPROM.commit())
    {
        // EEPROM.end();

        return true;
    }
    else
    {
        // EEPROM.end();

        return false;
    }
}

uint32 read_eeprom_32(int start_address)
{
    // char * param="0";
    // char param[20];  // 使用sprintf函数时需要定义param的字节数

    EEPROM.begin(EEPROM_SIZE);
    data_32 cnt_struct;

    cnt_struct.first = EEPROM.read(start_address);
    start_address = start_address + 1;

    cnt_struct.second = EEPROM.read(start_address);
    start_address = start_address + 1;

    cnt_struct.three = EEPROM.read(start_address);
    start_address = start_address + 1;

    cnt_struct.four = EEPROM.read(start_address);
    // Serial.println("read_eeprom_32");
    // Serial.println(cnt_struct.first);
    uint32 data_uint32 = (cnt_struct.first << 24) | (cnt_struct.second << 16) | (cnt_struct.three << 8) | cnt_struct.four;

    // Serial.println(data_uint32+1);
    // Serial.println("uinttest1");
    // sprintf(param, "uint32:%d", data_uint32);
    // Serial.println("uinttest2");
    // Serial.println(param);
    EEPROM.end();
    return data_uint32;
}

bool write_eeprom_8(uint8_t cnt, int start_address)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(start_address, cnt);
    if (EEPROM.commit())
    {
        // EEPROM.end();

        return true;
    }
    else
    {
        // EEPROM.end();

        return false;
    }
}

uint8_t read_eeprom_8(int start_address)
{
    // char * param="0";
    // char param[20];  // 使用sprintf函数时需要定义param的字节数

    EEPROM.begin(EEPROM_SIZE);
    uint8_t tmp_cnt;
    tmp_cnt = EEPROM.read(start_address);
    // Serial.println("read_eeprom_8");
    // Serial.println(tmp_cnt);
    EEPROM.end();
    return tmp_cnt;
}

void clear_eeprom()
{
    EEPROM.begin(EEPROM_SIZE);
    // write a 0 to all 512 bytes of the EEPROM
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.end();
    // turn the LED on when we're done
    //   pinMode(13, OUTPUT);
    //   digitalWrite(13, HIGH);
}
