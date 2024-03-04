#include "./Ali.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
// NTPClient timeClient(ntpUDP, "pool.ntp.org", 8 * 3600, 60000);  // 要防着是否出现问题的

String currentTime;
unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient client(espClient);
WiFiMulti wifiMulti;

unsigned int pwm_r = 0, pwm_g = 0, pwm_b = 0;
int openFlag = 0;
int time_setting = 4;
uint16 ali_cnt = 0;
bool turn_flag = true;

int get_hour() {
  int currentHour = timeClient.getHours();
  Serial.print("Hour:");
  Serial.println(currentHour);
  return currentHour;
}

int get_day() {
  int currentDay = timeClient.getDay();
  Serial.print("Day:");
  Serial.println(currentDay);
  return currentDay;
}

void work() {
  if (openFlag == 1) {
    Serial.println("[info] this is the truly answer!!!");
  }
}

// 检查设备与MQTT服务器连接情况
void mqttCheckConnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT Server ...");
    if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.print("MQTT Connect err:");
      Serial.println(client.state());
      delay(5000);
    }
  }
  if (client.connected()) {
    Serial.println("[info] keeping alive");
  }
}


// 收到信息后的回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[info]Message arrived,the topic is [");
  Serial.print(topic);
  Serial.println("] ");
  payload[length] = '\0';

  const char* json = (char*)payload;
  Serial.println("receiving json:");
  Serial.println(json);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonObject root = doc.as<JsonObject>();

  //云端下发的数据只有一个数据点，因此要判断是哪一个数据点下发了数据
  if (root["params"].containsKey("WaterOutletSwitch"))  //containsKey方法为判断json对象是否包含指定字段
  {
    openFlag = root["params"]["WaterOutletSwitch"];
    Serial.println("***WaterOutletSwitch***");
    Serial.println(openFlag);
  }
  if (root["params"].containsKey("time_setting"))  //containsKey方法为判断json对象是否包含指定字段
  {
    time_setting = root["params"]["time_setting"];
    Serial.println("***time_setting***");
    Serial.println(time_setting);
  }
}



// 连接wifi
void wifiInit() {
  // wifiMulti.addAP("18pro", "43426mqj"); // 将需要连接的一系列WiFi ID和密码输入这里
  wifiMulti.addAP("trace", "43426mqj");
  wifiMulti.addAP("CU_玉波", "12345678");
  wifiMulti.addAP("TP-LINK_84FE", "kong$123&K");
  wifiMulti.addAP("Xiaomi_365F", "15137839141");
  wifiMulti.addAP("18pro", "43426mqj");  // ESP8266-NodeMCU再启动后会扫描当前网络
  // wifiMulti.addAP("LAPTOP9#337", "xy1229033519"); // 环境查找是否有这里列出的WiFi ID。如果有
  Serial.println("[info] Connecting ...");  // 则尝试使用此处存储的密码进行连接。

  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) {  // 此处的wifiMulti.run()是重点。通过wifiMulti.run()，NodeMCU将会在当前
    delay(1000);                             // 环境中搜索addAP函数所存储的WiFi。如果搜到多个存储的WiFi那么NodeMCU
    Serial.print(i++);
    Serial.print(' ');  // 将会连接信号最强的那一个WiFi信号。
  }                     // 一旦连接WiFI成功，wifiMulti.run()将会返回“WL_CONNECTED”。这也是
                        // 此处while循环判断是否跳出循环的条件。

  // WiFi连接成功后将通过串口监视器输出连接成功信息
  Serial.print("[info] Connected to ");
  Serial.println(WiFi.SSID());  // 通过串口监视器输出连接的WiFi名称
  Serial.print("[info] IP address:\t");
  Serial.println(WiFi.localIP());  // 通过串口监视器输出ESP8266-NodeMCU的IP
}


/* 上报消息时间 */
bool mqttIntervalPost() {
  char param[128];
  char jsonBuf[128];

  // 将current转换为字符数组
  const char* temp = currentTime.c_str();

  //上传的数据在这里编辑，该例程将上报的数据为当前时间
  // sprintf(param, "{\"openTime\":\"%s\"}",temp);
  sprintf(param, "{\"test_type:FanRunTime\":\"%s\"}", temp);
  // sprintf(param, "{\"temperature\":\"%f\"}",30.5);

  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);  //发送格式化输出到 str 所指向的字符串。
  Serial.println("[info] uploading json:");
  Serial.println(jsonBuf);

  // 上传数据
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  if (d == 1) {
    Serial.println("[info] send success!!");
    return true;
  } else {
    Serial.println("[info] send failed!!");
    return false;
  }
}

/* 上报消息switch状态 */
bool mqttIntervalPost_swithch(bool turn_flag) {
  char param[128];
  char jsonBuf[128];

  // 将current转换为字符数组
  // const char * temp = currentTime.c_str();

  //上传的数据在这里编辑，该例程将上报的数据为当前时间
  // sprintf(param, "{\"openTime\":\"%s\"}",temp);
  if (turn_flag) {
    sprintf(param, "{\"WorkSwitch\":%d,\"temperature\":%f}", 1, 37.5);
  } else {
    sprintf(param, "{\"WorkSwitch\":%d,\"temperature\":%f}", 0, 36.5);
  }
  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);  //发送格式化输出到 str 所指向的字符串。
  Serial.println("[info] uploading json:");
  Serial.println(jsonBuf);

  // 上传数据
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  if (d == 1) {
    Serial.println("[info] send success!!");
    return true;
  } else {
    Serial.println("[info] send failed!!");
    return false;
  }
}

/* 上报消息switch状态 */
bool mqttIntervalPost_swithch(bool turn_flag, char const* device_id_name, char const* module_id_name) {
  char param[128];
  char jsonBuf[128];

  // 将current转换为字符数组
  // const char * temp = currentTime.c_str();

  //上传的数据在这里编辑，该例程将上报的数据为当前时间
  // sprintf(param, "{\"openTime\":\"%s\"}",temp);
  if (device_id_name != "") {
    if (turn_flag) {
      sprintf(param, "{\"%s:%s\":%d}", device_id_name, module_id_name, 1);
    } else {
      sprintf(param, "{\"%s:%s\":%d}", device_id_name, module_id_name, 0);
    }
  } else {
    if (turn_flag) {
      sprintf(param, "{\"%s\":%d}", module_id_name, 1);
    } else {
      sprintf(param, "{\"%s\":%d}", module_id_name, 0);
    }
  }

  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);  //发送格式化输出到 str 所指向的字符串。
  Serial.println("[info] uploading json:");
  Serial.println(jsonBuf);

  // 上传数据
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  if (d == 1) {
    Serial.println("[info] send success!!");
    return true;
  } else {
    Serial.println("[info] send failed!!");
    return false;
  }
}

/* 上报消息 
数据格式：https://help.aliyun.com/document_detail/89301.htm?spm=a2c4g.11186623.0.0.77b871e9cKDac8#concept-mvc-4tw-y2b
开发者社区：https://developer.aliyun.com/article/740235
*/
bool mqttIntervalPost_sleep_num(uint16 cnt) {
  char param[128];
  char jsonBuf[128];


  // 将current转换为字符数组
  const char* temp = currentTime.c_str();

  //上传的数据在这里编辑，苏醒次数
  sprintf(param, "{\"Irrigation_pump:Integratedcnt\":%d}", cnt);  // 注意如果不是字符串类型的话就需要将“”去掉

  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);  //发送格式化输出到 str 所指向的字符串。
  Serial.println("[info] uploading json:");
  Serial.println(jsonBuf);

  // 上传数据
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  if (d == 1) {
    Serial.println("[info] send success!!");
    return true;
  } else {
    Serial.println("[info] send failed!!");
    return false;
  }
}

// bool mqttIntervalPost_int(uint16 cnt, String device_id_name, String module_id_name)
bool mqttIntervalPost_int(uint16 cnt, char const* device_id_name, char const* module_id_name)

{
  char param[128];
  char jsonBuf[128];

  // 将current转换为字符数组
  const char* temp = currentTime.c_str();

  //上传的数据在这里编辑，苏醒次数
  // sprintf(param, "{\"Irrigation_pump:Integratedcnt\":%d}",cnt); // 注意如果不是字符串类型的话就需要将“”去掉
  if (device_id_name == "") {
    sprintf(param, "{\"%s\":%d}", module_id_name, cnt);  // 注意如果不是字符串类型的话就需要将“”去掉

  } else
    sprintf(param, "{\"%s:%s\":%d}", device_id_name, module_id_name, cnt);  // 注意如果不是字符串类型的话就需要将“”去掉
  sprintf(jsonBuf, ALINK_BODY_FORMAT, param);                               //发送格式化输出到 str 所指向的字符串。
  Serial.println("[info] uploading json:");
  Serial.println(jsonBuf);

  // 上传数据
  boolean d = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
  if (d == 1) {
    Serial.println("[info] send success!!");
    return true;
  } else {
    Serial.println("[info] send failed!!");
    return false;
  }
}


bool ali_ini_send_cnt(uint32 cnt) {
  /*初始化WiFi，并将cnt发送给阿里云平台*/
  Serial.println("ini ali and sent:");
  Serial.println(cnt);
  ali_setup();
  // if (millis() - lastMs >= 10000)
  // {
  // 获取当前时间
  lastMs = millis();

  // 检查连接状态
  mqttCheckConnect();

  timeClient.update();
  // 获取当前时间
  currentTime = timeClient.getFormattedTime();
  Serial.print("[info] now time is :");
  Serial.println(currentTime);

  // // 上报消息
  mqttIntervalPost();

  // // 根据下发的数据进行反馈
  // work();
  delay(4);
  Serial.println("mqttIntervalPost_sleep_num!");
  bool s_flag = mqttIntervalPost_sleep_num(cnt);
  delay(4);
  Serial.println("mqttIntervalPost_swithch!");
  turn_flag = !turn_flag;
  mqttIntervalPost_swithch(turn_flag);
  // }
  client.loop();
  return s_flag;
}

bool ali_send_cnt(uint32 cnt) {
  /*初始化WiFi，并将cnt发送给阿里云平台*/
  Serial.println("ali_send_cnt:");
  Serial.println(cnt);
  // 获取当前时间
  lastMs = millis();

  // 检查连接状态
  mqttCheckConnect();

  timeClient.update();
  // 获取当前时间
  currentTime = timeClient.getFormattedTime();
  Serial.print("[info] now time is :");
  Serial.println(currentTime);


  delay(4);
  Serial.println("mqttIntervalPost_sleep_num!");
  // bool s_flag = mqttIntervalPost_sleep_num(cnt);
  bool s_flag = mqttIntervalPost_int(cnt, "", "RunTimes");
  client.loop();
  return s_flag;
}

bool ali_send_irri_time(int cnt) {
  /*初始化WiFi，并将cnt发送给阿里云平台*/
  // 检查连接状态
  mqttCheckConnect();

  timeClient.update();
  // 获取当前时间
  currentTime = timeClient.getFormattedTime();
  Serial.print("[info] now time is :");
  Serial.println(currentTime);

  delay(4);
  Serial.println("irrigation time!");
  bool s_flag = mqttIntervalPost_int(cnt, "", "time_setting");
  client.loop();
  return s_flag;
}

bool ali_send_pump_state(bool pump_flag) {
  /*初始化WiFi，并将cnt发送给阿里云平台*/
  Serial.println("ali_send_pump_state:");
  Serial.println(pump_flag);
  // 获取当前时间
  lastMs = millis();
  // 检查连接状态
  mqttCheckConnect();
  timeClient.update();
  // 获取当前时间
  currentTime = timeClient.getFormattedTime();
  Serial.print("[info] now time is :");
  Serial.println(currentTime);
  // // 上报消息
  // mqttIntervalPost();

  // // 根据下发的数据进行反馈
  // work();
  delay(4);
  Serial.println("mqttIntervalPost_swithch!");
  // bool s_flag = mqttIntervalPost_swithch(pump_flag);
  bool s_flag = mqttIntervalPost_swithch(pump_flag, "", "WaterOutletSwitch");
  client.loop();
  return s_flag;
}
// 初始化Mqtt服务
void mqttServeInit() {
  // 新增2022年11月16日
  // client.setBufferSize(1024);
  // client.setKeepAlive(60);
  // 设置MQTT服务器和端口号
  client.setServer(MQTT_SERVER, MQTT_PORT);
  // 设置MQTT订阅回调函数
  client.setCallback(callback);
}

void ali_setup() {
  Serial.println("[info] Ali demo Start");
  Serial.print("[info] CLIENT_ID:");
  Serial.println(CLIENT_ID);
  Serial.print("[info] MQTT_USRNAME:");
  Serial.println(MQTT_USRNAME);
  Serial.print("[info] MQTT_PASSWD:");
  Serial.println(MQTT_PASSWD);
  // 连接WIFI
  wifiInit();

  // 初始化Mqtt服务
  mqttServeInit();

  // 初始化NTP时间服务
  timeClient.begin();
}


void ali_loop() {
  //millis()是系统启动到目前的总时间，以下为5s上传一次数据
  if (millis() - lastMs >= 10000) {
    ali_cnt++;
    // 获取当前时间
    lastMs = millis();

    // 检查连接状态
    mqttCheckConnect();

    timeClient.update();
    // 获取当前时间
    currentTime = timeClient.getFormattedTime();
    Serial.print("[info] now time is :");
    Serial.println(currentTime);

    // 上报消息
    mqttIntervalPost();

    // 根据下发的数据进行反馈
    work();
    delay(4);
    Serial.println("mqttIntervalPost_sleep_num!");
    mqttIntervalPost_sleep_num(ali_cnt);
    delay(4);
    Serial.println("mqttIntervalPost_swithch!");
    turn_flag = !turn_flag;
    mqttIntervalPost_swithch(turn_flag);
  }
  client.loop();
}
