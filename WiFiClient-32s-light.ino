/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.


当程序开始执行时，将调用setup()函数，使用此函数
  ①用来初始化变量
  ②管脚模式
  ③调用库函数

  注意：setup()函数将只运行一次。
loop()
  功能：连续执行函数内的语句
  实际上Arduino编程环境封装了上层调用，使得在进行Arduino开发时，没有像传统C/C++程序需要在Arduino中显式使用入口函数main。但实际上main函数存在于Arduino核心库中，且仍然是程序的入口。
  在Arduino核心库中可见main.cpp文件，其内容可见main_origin.cpp文件中：
  
  通过以上程序可见，Arduino程序中编写的setup和loop函数，都在main函数中调用了。loop的循环执行，是通过for循环实现的，且每次loop结束后，都会进行串口事件判断。
  return 只会跳出当前的loop，并重新运行loop中的内容。
  delay(500);1000表示1s

注意：
1. 本程序需要有WiFi参与的；（可以在以下代码中添加WiFi热点名称）
  wifiMulti.addAP("trace", "43426mqj"); 
  wifiMulti.addAP("18pro", "43426mqj"); // ESP8266-NodeMCU再启动后会扫描当前网络
https://blog.csdn.net/dpjcn1990/article/details/92831686

*/

// #include <ESP8266WiFi.h>
// #include "/head/ctrl_code.h"   // 这样是不对的
// #include "head/ctrl_code.h"
#include "Ali.h"
#include "button.h"
#include "ctrl_code.h"
#include "c_types.h"
#include "deep_sleep.h"
#include <OneButton.h>
#include <Ticker.h>
// #include "Ds1302.h"
// http://www.360doc.com/content/20/0128/21/59057945_888387060.shtml
// https://www.bbsmax.com/A/RnJW3e8BJq/
// https://www.xpstem.com/article/10115
# define pump_gpio 25 // 这个部分是水泵的打开
# define flower1 26  // 在没有网的时候用来开启那个4G网络；
// 而在有网的情况下，可以打开摄像头传输摄像数据

unsigned long inter_cnt = 24;  //3天 irrigation
unsigned char irri_time_stop = 23; // 灌溉20s
unsigned char irri_time_start = 18; // 灌溉20s
unsigned char irri_time_pump = 20;  //灌溉20s
uint32 cnt=0;
Ticker ticker;

bool skip_flag = false;
bool light_flag = false;

int current_day_back=25;
void setup(){
  delay(1000);
  Serial.begin(115200);
  // button_setup();
  btn.attachClick(single_skip);
  // 添加双击事件函数
  btn.attachDoubleClick(double_clear);
  // 添加长按事件结束函数
  btn.attachLongPressStop(long_irri_off);
  // // 添加按下事件函数
  btn.attachLongPressStart(long_irri_on);
  pinMode(pump_gpio, OUTPUT);
  pinMode(flower1, OUTPUT);
//   change_supply(true, flower1); // 开机前启动那个WiFi
  change_supply(false, pump_gpio);
  pinMode(LED_BUILTIN, OUTPUT);
//   change_supply(false, LED_BUILTIN);

  /*是否需要清除ee2prom*/
  unsigned long int start_time = millis();

  Serial.println("please press the butuon!");
  Serial.println("single click: skip!");
  Serial.println("double click: clear!");

  while (true)
  {
    // Serial.println("test!!");
    btn.tick();
    // Serial.println("tick");
    delay(1);
    /* code */
    // if (millis()-start_time > 1e7){
    if (millis()-start_time > 30e3){
      /*超时并不擦除*/
      shine_num(4);
      Serial.println("over time!");

      break;
    }
    if (skip_flag){
      Serial.println("skip!");
      break;
    }
    // Serial.println("tick2");

  }
  Serial.println("finsih setting!");
  Serial.println("Start Ali plateform setting!");
  ali_setup();
  shine_num(6, 100);
  delay(2e3);
  shine_num(6, 100);
  // ali_loop();
  // ticker.attach(1, update_setting);

}

/*添加 阿里云平台代码*/

void loop() {
  // char * cnt;
  // uint32 value;
  cnt = read_eeprom_32(0);
  // cnt = cnt + 1<<24;
  char  param[20];
  sprintf(param, "cnt is %d", cnt);
  Serial.println(param);
  Serial.println(cnt);

  /*将当前cnt存储到e2prom中 */
  cnt ++;
  Serial.println("cnt is:");
  Serial.println(cnt);
  if(write_eeprom_32(cnt, 0)){
    Serial.println("store cnt-CS");
  }else{
    Serial.println("store cnt fail");
  }
  /*发送 cnt 信息*/
  // ali_setup();
  int currentHour = get_hour();
  int currentDay = get_day();

  //  ESP.deepSleep(3e6); //作者：夏祭鱿鱼 https://www.bilibili.com/read/cv5988390  睡眠30s  最大值4,294,967,295， 大约71分钟。
  // if (cnt%inter_cnt==0){
  if (currentHour >= irri_time_start and currentHour <= irri_time_stop)
    {
        // change_supply(true, pump);
        // change_supply(true, LED_BUILTIN);
       Serial.println("on");
       ali_send_pump_state(true);

        // delay(irri_time_pump * 1000);
        // change_supply(false, pump); // 当休眠的时候，输出也会为低电平的
        // change_supply(false, LED_BUILTIN);
        // Serial.println("off");
        // ali_send_pump_state(false);

        // current_day_back = currentDay;
        if (!light_flag)
        {
          change_supply(true, pump);  // 亮灯
          change_supply(true, LED_BUILTIN);
        }
       light_flag = true;
      
    }
    else
    {
       if (light_flag)
        {
          change_supply(false, pump);
          change_supply(false, LED_BUILTIN);
        }
      light_flag = false;
      ali_send_pump_state(false);
    }

  ali_send_cnt(cnt);
  // 设置irri_time
  // if(irri_time_pump != time_setting){
  //   irri_time_pump = time_setting;
  //   ali_send_irri_time(irri_time_pump);
    
  // }
  // Serial.println("time setting");
  // Serial.println(time_setting);
  delay(1e3);
  // delay(3600e3);  //休眠一个小时
  // delay(3e3);  //休眠一个小时
  // ESP.deepSleep(3e6); //睡眠1s  最大值4,294,967,295， 大约71分钟 60s*60min*1h 360
  // ESP.deepSleep(3600e6); //睡眠1s  最大值4,294,967,295， 大约71分钟 60s*60min*1h 3600
  deep_sleep_setup(60*60); // 每一个小时深度休眠一次
  // deep_sleep_setup(5); // 每隔5s休眠一次
  
}

void single_skip(){
  skip_flag = true;
  Serial.println("single click!");

  shine_num(1);
}


void long_irri_on(){
    change_supply(true, pump_gpio); 
    change_supply(true, LED_BUILTIN);
    Serial.println("on");
}

void long_irri_off(){
    change_supply(false, pump_gpio); 
    change_supply(false, LED_BUILTIN);
    Serial.println("off");
}

void double_clear(){
  skip_flag = true;
  clear_eeprom();
  Serial.println("double click!");
  shine_num(2);

}

void update_setting()
{
  // 每隔固定时间更新一下设置
  if (time_setting!=irri_time_pump){
    irri_time_pump = time_setting;
  }
}


// 使用按键控制：更新信息
// 1. 自动更新时间；
// 2. 更新配置信息；





