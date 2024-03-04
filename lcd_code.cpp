/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using SPI to communicate
 4 or 5 pins are required to interface.

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 1. 希望可以多一些颜色，但是只有白黑两种颜色；
 2. 参考程序：D:\Code\nodemcu\lcd_spi_code\lcd-spi-1-1
 3. 显示一下内容：
    1）显示当前真实时间；
    2）显示水泵、供氧机等等的状态；


 **************************************************************************/

#include "lcd_code.h"
// #include <iostream>
// #include <sstream>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void set_up_oled(void){
    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
  }
}

void draw_string(const String &text_string, int8 cursor[2], int8 text_size=1) {
    /****************************************************************************/
//   display.clearDisplay();

  display.setTextSize(text_size);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(cursor[0], cursor[1]);             // Start at top-left corner
  display.println(text_string);
  display.display();
  delay(1);
}

void draw_num_test(void){
  display.clearDisplay();
  delay(1);
  int max_num = 10;
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
   
  for(int i=0;i<max_num;i++)
  {
    display.clearDisplay();
    display.setCursor(0,0); // 好像每次都需要设定显示坐标的诶
    display.println(i);
    display.display();
    delay(20);

  }
}


void draw_ctrl_interface(uint16_t min_cnt,const String real_time){
  /*******要显示的内容*********/
  display.clearDisplay();
  delay(1);
  
  // display.display();
  // str(min_cnt);
  // draw_string("real time")
  int8 real_time_cursor[2] = {0, 0};   //real time 坐标开始位置
  int8 base_text_size = 1; // 字体大小
  //  真实时间--real time
  // String real_time_str = com_str("real time", min_cnt);  // 显示真实的时间
  String real_time_str = "real time: " + real_time;
  // Serial.println("real_time_str:"+real_time_str);
  draw_string(real_time_str, real_time_cursor, base_text_size);
  // min_cnt 
  int8 min_cnt_time_cursor[2] = {0, 8};   //real time 坐标开始位置

  String min_cnt_str = com_str("min_cnt", min_cnt);  // 显示真实的时间
  // Serial.println("min_cnt_str: "+min_cnt_str);
  draw_string(min_cnt_str, min_cnt_time_cursor, base_text_size);

  display.display(); //更新当前的屏幕
  // delay(25000);
  delay(1);

}

void draw_bool_status(const String &status_name, bool bool_flag, int8 cursor[2], int8 base_text_size=1){
  /*
    显示各个组件的的状态
  */
  String status_flag_string = com_str(status_name, bool_flag);
  draw_string(status_flag_string, cursor, base_text_size);
  
}

void draw_real_time(String real_time){
    display.clearDisplay();
    // display.display();
  // draw_string("real time")
    int8 real_time_cursor[2] = {0, 0};   //real time 坐标开始位置
    int8 base_text_size = 1; // 字体大小
    //  真实时间--real time
    // String real_time_str = com_str("real time", min_cnt);  // 显示真实的时间
    String real_time_str = "real time: " + real_time;
    // Serial.println("real_time_str:"+real_time_str);

    draw_string(real_time_str, real_time_cursor, base_text_size);
    display.display(); //更新当前的屏幕
    delay(1);

}

String com_str(String base_str, uint16_t num){
    // 将base_str与num相结合：base_str：num
    // String num_str = String(num, 10);
    String num_str = base_str + ": " + String(num);
    // Serial.println("num_str: "+num_str);
    return num_str;
}

String com_str(const String base_str, bool flag_bool){
    // 将base_str与num相结合：base_str：num
    // String num_str = String(num, 10);
    String blag_str = base_str + ": " + String(flag_bool);
    // Serial.println("num_str: "+num_str);
    return blag_str;
}


// void printDateTime(DateTime dateTime) {
//   display.setTextSize(2);
//   display.setTextColor(WHITE);
//   display.setCursor(20,10);
//   display.println("- LEO -");
//   display.setCursor(20,35);
//   //传送小时
//   display.print(dateTime.hour(), DEC);
//   display.print(':');
//   //传送分钟
//   display.print(dateTime.minute(), DEC);
//   display.print(':');
//   //传送秒
//   display.print(dateTime.second(), DEC);
// }



void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}






