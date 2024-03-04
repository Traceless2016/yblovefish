#ifndef Ali_H_
#define Ali_H_
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "c_types.h"

// WiFiMulti wifiMulti;

// /* 设备证书信息*/
// #define PRODUCT_KEY       "hwky0lAlnzo"
// #define DEVICE_NAME       "nodeMCU"
// #define DEVICE_SECRET     "a0a2197c4c5c23a5993448bdad5fb15c"
// #define REGION_ID         "cn-shanghai"

/* 设备证书信息*/
#define PRODUCT_KEY       "i6j7XynAQ6M"
#define DEVICE_NAME       "solar_light_esp32s"
#define DEVICE_SECRET     "de2b814d32f25c244895536b31b2c125"
#define REGION_ID         "cn-shanghai"


// 注：产品密钥（ProductSecret）、设备密钥（DeviceSecret）为两种不同密钥。如您要获取的是设备密钥 （设备证书），请前往对应的设备详情页面获取。
/* Aliyun线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME "&" PRODUCT_KEY

#define CLIENT_ID         "i6j7XynAQ6M.solar_light_esp32s|securemode=2,signmethod=hmacsha256,timestamp=1708417975980|" // 注意hmacsha1这个是加密方式，还有hmacsha256--
// mqtt connect err:-2 更改securemode=3应该就好了的
// MQTT连接报文参数,请参见MQTT-TCP连接通信文档，文档地址：https://help.aliyun.com/document_detail/73742.html
// https://iot.console.aliyun.com/product  官方文档：
/*
mqttclientId=12345|securemode=3,signmethod=hmacsha1,timestamp=789|
mqttUsername=device&pk
mqttPassword=hmacsha1("secret","clientId12345deviceNamedeviceproductKeypktimestamp789").toHexString();
*/
// 时间戳：https://developer.aliyun.com/skills/timestamp.html
// 加密明文是参数和对应的值（clientIdesp8266deviceName${deviceName}productKey${productKey}timestamp1234567890）按字典顺序拼接
// clientIdesp8266deviceNamenodeMCUproductKeyhwky0lAlnzotimestamp1668152369
// clientIdesp8266deviceNamepumpproductKeyi6j7XynAQ6Mtimestamp1676004410
// 密钥是设备的DeviceSecret **********************
//要使用加密工具，输入以上证书信息加密（时间戳可以省略）
#define MQTT_PASSWD       "c03d24ad527527df50e73352694bf39c4a3dcac7b0de4af8a1c7603153cdbf63"

// 发送报文的json格式
#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
// 上报报文主题
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

// 自己设置的模块儿

extern int time_setting;

void ali_setup();
void ali_loop();
bool ali_ini_send_cnt(uint32 cnt);
bool ali_send_cnt(uint32 cnt);
bool ali_send_pump_state(bool pump_flag);
int get_hour();
int get_day();
bool ali_send_irri_time(int cnt);
// bool mqttIntervalPost_int(uint16 cnt, String device_id_name, String module_id_name);
bool mqttIntervalPost_int(uint16 cnt, char const* device_id_name, char const* module_id_name);
// bool mqttIntervalPost_swithch(bool turn_flag, String device_id_name, String module_id_name);
bool mqttIntervalPost_swithch(bool turn_flag, const char* device_id_name, const char* module_id_name);

#endif



