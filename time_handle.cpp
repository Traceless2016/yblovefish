
#include <time.h>
// #include "../head/time_handle.h"
#include "time_handle.h"

time_handle::time_handle(/* args */)
{

    unix_time_stamp = 1000;
}

time_handle::~time_handle()
{
}

void time_handle::printTm(const char *what, const tm *tm)
{
    Serial.print(what);
    PTM(isdst);
    PTM(yday);
    PTM(wday);
    PTM(year);
    PTM(mon);
    PTM(mday);
    PTM(hour);
    PTM(min);
    PTM(sec);
}

inline void time_handle::show_run_time_ms()
{
    time_t now_stp;
    now_stp = time(nullptr);
    // now_ms
}

int time_handle::parse_time(void)
{
    return 123;
}

// 在类外定义函数体，必须在前面加上inline关键字
void temp::print_amount(int amount)
{
    Serial.println(amount);
}

inline void temp::print_string(String amount)
{
    Serial.println(amount);
}

uint16_t time_handle::conv_time_to_int(String time_str)
{
    //String time_str="23:57:23"
    String part01 = getValue(time_str, ':', 0);
    String part02 = getValue(time_str, ':', 1);
    // tm1637.display(0, (part01.toInt() / 10));
    Serial.println("conv_time_to_int");  // 2 是小时第一位
    Serial.println((part01.toInt() / 10));  // 2 是小时第一位
    
    // tm1637.display(1, (part01.toInt() % 10));
    Serial.println((part01.toInt() % 10));  // 3 是小时第二位
    // tm1637.display(2, (part02.toInt() / 10));
    Serial.println((part02.toInt() / 10));  // 5 是分钟第一位
    // tm1637.display(3, (part02.toInt() % 10));
    Serial.println((part02.toInt() % 10)); // 5 是分钟第二位
    // dian = !dian;
    // tm1637.point(dian);
    uint16_t hour_int = part01.toInt();
    uint16_t min_int = part02.toInt();
    uint16_t res_int = hour_int*60 + min_int;
    return res_int;
}

// 切割字符串
String time_handle::getValue(String data, char separator, int index)
{

    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
