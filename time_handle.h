#ifndef TIME_HANDLE_H_

#define TIME_HANDLE_H_
#include <WString.h>
#include <Arduino.h>
#include <time.h>

#define PTM(w)              \
  Serial.print(" " #w "="); \
  Serial.print(tm->tm_##w);

// struct timeval {
//   time_t      tv_sec;
//   suseconds_t tv_usec;
// };

class time_handle
{
private:
  /* data */
  int unix_time_stamp;

public:
  time_handle(void);
  ~time_handle();
  int parse_time();
  void printTm(const char *what, const tm *tm);
  void show_run_time_ms();
  uint16_t conv_time_to_int(String time_str);
  String getValue(String data, char separator, int index);
};

class temp
{
public:
  int amount;

  //构造函数
  temp(int amount)
  {
    this->amount = amount;
  }

  //普通成员函数，在类内声明时前面可以不加inline
  inline void print_amount(int amount);
  void print_string(String amount);
  uint16_t conv_time_to_int(String time_str);
  String getValue(String data, char separator, int index);
};

#endif