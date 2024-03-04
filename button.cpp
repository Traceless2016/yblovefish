#include "button.h"
// OneButton btn = OneButton(D3, false, false);
OneButton btn = OneButton(0, true, true);  //Esp8266实测可用
// OneButton btn = OneButton(D3, true, true);  //Esp8266实测可用

// 记录按键按下时间
uint32_t clicktime = 0;

/**
 * 处理单击
 */
void singleClick() {
  Serial.println("按键单击");
}

/**
 * 处理双击
 */
void doubleClick() {
  Serial.println("按键双击");
}

/**
 * 按键长按开始做的事情
 */
void longClickStart() {
  Serial.println("按键长按开始");
  clicktime = millis();
}

/**
 * 处理按键长按
 */
void longClick() {
  Serial.println("按键长按结束");
  Serial.println("按键按下时间：");
  // 这里为啥加1000？ 因为按键长按开始时间时按下一秒后开始计算的，所以就要加上我们原本已经按下的一秒种
  Serial.print(millis()-clicktime+1000);
  // 重置按下时间
  clicktime = 0;
}

void button_setup() {
  // 添加单击事件函数
  btn.attachClick(singleClick);
  // 添加双击事件函数
  btn.attachDoubleClick(doubleClick);
  // 添加长按事件函数
  btn.attachLongPressStop(longClick);
  // 添加按下事件函数
  btn.attachLongPressStart(longClickStart);
}

void button_loop() {
  btn.tick();
}





