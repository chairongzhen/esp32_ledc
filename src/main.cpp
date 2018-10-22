#include <SPIFFS.h>
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <cJSON.h>
#include <time.h>
#include "LED_ESP32.h"
#include <ESPmDNS.h>
#include <Update.h>
#include <PubSubClient.h>

#define RESET_BUTTON 16
#define VERSION_NUM "0.18"
#define ESP_HOST_NAME "esp004"
#define ESP_RTC_TICK 1539666903

String PWM_INFO_SHOWTYPE, PWM_INFO_TESTMODE, PWM_INFO_CONMODE, PWM_INFO_RTC, PWM_INFO_VERSION;
String P1_0, P1_1, P1_2, P1_3, P1_4, P1_5, P1_6, P1_7, P1_8, P1_9, P1_10, P1_11, P1_12, P1_13, P1_14, P1_15, P1_16, P1_17, P1_18, P1_19, P1_20, P1_21, P1_22, P1_23, P1_24;
String P2_0, P2_1, P2_2, P2_3, P2_4, P2_5, P2_6, P2_7, P2_8, P2_9, P2_10, P2_11, P2_12, P2_13, P2_14, P2_15, P2_16, P2_17, P2_18, P2_19, P2_20, P2_21, P2_22, P2_23, P2_24;
String P3_0, P3_1, P3_2, P3_3, P3_4, P3_5, P3_6, P3_7, P3_8, P3_9, P3_10, P3_11, P3_12, P3_13, P3_14, P3_15, P3_16, P3_17, P3_18, P3_19, P3_20, P3_21, P3_22, P3_23, P3_24;
String P4_0, P4_1, P4_2, P4_3, P4_4, P4_5, P4_6, P4_7, P4_8, P4_9, P4_10, P4_11, P4_12, P4_13, P4_14, P4_15, P4_16, P4_17, P4_18, P4_19, P4_20, P4_21, P4_22, P4_23, P4_24;
String P5_0, P5_1, P5_2, P5_3, P5_4, P5_5, P5_6, P5_7, P5_8, P5_9, P5_10, P5_11, P5_12, P5_13, P5_14, P5_15, P5_16, P5_17, P5_18, P5_19, P5_20, P5_21, P5_22, P5_23, P5_24;
String P6_0, P6_1, P6_2, P6_3, P6_4, P6_5, P6_6, P6_7, P6_8, P6_9, P6_10, P6_11, P6_12, P6_13, P6_14, P6_15, P6_16, P6_17, P6_18, P6_19, P6_20, P6_21, P6_22, P6_23, P6_24;
String P7_0, P7_1, P7_2, P7_3, P7_4, P7_5, P7_6, P7_7, P7_8, P7_9, P7_10, P7_11, P7_12, P7_13, P7_14, P7_15, P7_16, P7_17, P7_18, P7_19, P7_20, P7_21, P7_22, P7_23, P7_24;
String SSID, SSID_PWD;
bool IS_SMART = false;
bool RESET_FLAG = false;

const char *ssid = ESP_HOST_NAME;
const char *password = "11111111";
const char *host = ESP_HOST_NAME;
// const char *mqttServer = "m12.cloudmqtt.com";
// const int mqttPort = 16610;
// const char *mqttuser = "cqyjmitd";
// const char *mqttpwd = "SXLMuaorn881";
const char *mqttServer = "m12.cloudmqtt.com";
const int mqttPort = 16610;
const char *mqttuser = "cqyjmitd";
const char *mqttpwd = "SXLMuaorn881";

LED_ESP32 led1(4, 0, 100);
LED_ESP32 led2(12, 1, 100);
LED_ESP32 led3(13, 2, 100);
LED_ESP32 led4(15, 3, 100);
LED_ESP32 led5(22, 4, 100);
LED_ESP32 led6(23, 5, 100);
LED_ESP32 led7(25, 6, 100);
LED_ESP32 led8(2, 7, 100);

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

// file operation begin
String getFileString(fs::FS &fs, const char *path)
{
  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for geting");
    return "";
  }
  return file.readString();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  //Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    //Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
}

void appendLn(fs::FS &fs, const char *path)
{
  Serial.printf("Appending enter to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.println())
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("File deleted");
  }
  else
  {
    Serial.println("Delete failed");
  }
}

void initFileSystem()
{
  if (SPIFFS.exists("/wifi.ini"))
  {
    Serial.println("delete the wifi config file");
    deleteFile(SPIFFS, "/wifi.ini");
  }
  String pwminfocontent = "{\"showtype\":\"fix\",\"testmode\":\"test\",\"sysdate\":\"{unixtick}\",\"status\":\"stop\",\"conmode\": \"local\",\"version\":\"{version_num}\"}";
  pwminfocontent.replace("{version_num}", VERSION_NUM);
  pwminfocontent.replace("{unixtick}", String(ESP_RTC_TICK));
  writeFile(SPIFFS, "/pwminfo.ini", pwminfocontent.c_str());

  writeFile(SPIFFS, "/p.ini", "{\"p1\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0},\"p2\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0},\"p3\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0},\"p4\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0},\"p5\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0},\"p6\":{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}}");
  writeFile(SPIFFS, "/p1.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p2.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p3.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p4.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p5.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p6.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
  writeFile(SPIFFS, "/p7.ini", "{\"t0\":0,\"t1\":0,\"t2\":0,\"t3\":0,\"t4\":0,\"t5\":0,\"t6\":0,\"t7\":0,\"t8\":0,\"t9\":0,\"t10\":0,\"t11\":0,\"t12\":0,\"t13\":0,\"t14\":0,\"t15\":0,\"t16\":0,\"t17\":0,\"t18\":0,\"t19\":0,\"t20\":0,\"t21\":0,\"t22\":0,\"t23\":0,\"t24\":0}");
}

void onFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (index == 0)
  {
    Serial.printf("Update: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Update.printError(Serial);
    }
  }
  Update.write(data, len);
  if (final)
  {
    if (Update.end(true))
    {
      Serial.printf("Update Success: %u\nRebooting....\n", index + len);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}
// file operation end

// reset button handle event
void handleRestButtonChanged()
{
  Serial.println("");
  Serial.println("Rest begin.....");
  RESET_FLAG = true;
  Serial.println("rest end.....");
  detachInterrupt(RESET_BUTTON);
}

// 404 not found hook
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

// common function begin
int split(char dst[][80], char *str, const char *spl)
{
  int n = 0;
  char *result = NULL;
  result = strtok(str, spl);
  while (result != NULL)
  {
    strcpy(dst[n++], result);
    result = strtok(NULL, spl);
  }
  return n;
}
// common fucntion end

// mqtt callback function
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("the topic is: " + String(topic));

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  String topic_name_p = ESP_HOST_NAME;
  topic_name_p = topic_name_p + "/p";
  String topic_name_p1 = ESP_HOST_NAME;
  topic_name_p1 = topic_name_p1 + "/p1";
  String topic_name_p2 = ESP_HOST_NAME;
  topic_name_p2 = topic_name_p2 + "/p2";
  String topic_name_p3 = ESP_HOST_NAME;
  topic_name_p3 = topic_name_p3 + "/p3";
  String topic_name_p4 = ESP_HOST_NAME;
  topic_name_p4 = topic_name_p4 + "/p4";
  String topic_name_p5 = ESP_HOST_NAME;
  topic_name_p5 = topic_name_p5 + "/p5";
  String topic_name_p6 = ESP_HOST_NAME;
  topic_name_p6 = topic_name_p6 + "/p6";
  String topic_name_p7 = ESP_HOST_NAME;
  topic_name_p7 = topic_name_p7 + "/p7";
  String topic_name_pt = ESP_HOST_NAME;
  topic_name_pt = topic_name_pt + "/pt";

  String checkonine = ESP_HOST_NAME;
  checkonine = checkonine + "/check";

  String filecontent;
  for (int i = 0; i < length; i++)
  {
    filecontent = filecontent + (char)payload[i];
  }

  //Serial.println("the content is: " + filecontent);

  String tpl_content = "{\"t0\":{t0},\"t1\":{t1},\"t2\":{t2},\"t3\":{t3},\"t4\":{t4},\"t5\":{t5},\"t6\":{t6},\"t7\":{t7},\"t8\":{t8},\"t9\":{t9},\"t10\":{t10},\"t11\":{t11},\"t12\":{t12},\"t13\":{t13},\"t14\":{t14},\"t15\":{t15},\"t16\":{t16},\"t17\":{t17},\"t18\":{t18},\"t19\":{t19},\"t20\":{t20},\"t21\":{t21},\"t22\":{t22},\"t23\":{t23},\"t24\":{t24}}";
  char *p_content;
  char dst[30][80];
  int cnt;
  cJSON *root = NULL;
  cJSON *item = NULL;
  const char *jsonstr = filecontent.c_str();
  root = cJSON_Parse(jsonstr);
  String itemstr;
  String checktimetopic = "esp32/checktime";

  if (String(topic) != topic_name_p && String(topic) != checktimetopic)
  {
    p_content = new char[200];
    strcpy(p_content, filecontent.c_str());
    cnt = split(dst, p_content, ",");
  }

  if (String(topic) == topic_name_p)
  {
    item = cJSON_GetObjectItem(root, "showtype");
    itemstr = cJSON_Print(item);
    PWM_INFO_SHOWTYPE = itemstr;
    PWM_INFO_SHOWTYPE.replace("\"", "");
    item = cJSON_GetObjectItem(root, "testmode");
    itemstr = cJSON_Print(item);
    PWM_INFO_TESTMODE = itemstr;
    PWM_INFO_TESTMODE.replace("\"", "");
    item = cJSON_GetObjectItem(root, "sysdate");
    itemstr = cJSON_Print(item);
    PWM_INFO_RTC = itemstr;
    PWM_INFO_RTC.replace("\"", "");
    struct timeval stime;
    stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
    settimeofday(&stime, NULL);
    item = cJSON_GetObjectItem(root, "conmode");
    itemstr = cJSON_Print(item);
    PWM_INFO_CONMODE = itemstr;
    PWM_INFO_CONMODE.replace("\"", "");
    item = cJSON_GetObjectItem(root, "version");
    itemstr = cJSON_Print(item);
    PWM_INFO_VERSION = itemstr;
    PWM_INFO_VERSION.replace("\"", "");
    writeFile(SPIFFS, "/pwminfo.ini", filecontent.c_str());
  }
  else if (String(topic) == checktimetopic)
  {
    PWM_INFO_RTC = filecontent;
    String pwmifnocontent = getFileString(SPIFFS, "/pwminfo.ini");
    Serial.println(pwmifnocontent);
    p_content = new char[200];
    strcpy(p_content, pwmifnocontent.c_str());
    cnt = split(dst, p_content, ",");
    String tpl_sysdate;
    String change_sysdate = "\"sysdate\":\"";
    change_sysdate = change_sysdate + PWM_INFO_RTC;
    change_sysdate = change_sysdate + "\"";
    for (int i = 0; i < cnt; i++)
    {
      if (i == 2)
      {
        tpl_sysdate = dst[i];
        break;
      }
    }
    pwmifnocontent.replace(tpl_sysdate, change_sysdate);

    writeFile(SPIFFS, "/pwminfo.ini", pwmifnocontent.c_str());
    struct timeval stime;
    stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
    settimeofday(&stime, NULL);
  }
  else if(String(topic) == topic_name_pt) {
    for(int i=0;i<cnt;i++) {
      Serial.println(dst[i]);
      Serial.println(strlen(dst[i]));
      
    }
  }
  else if (String(topic) == topic_name_p1)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P1_0 = dst[i];
        break;
      case 1:
        P1_1 = dst[i];
        break;
      case 2:
        P1_2 = dst[i];
        break;
      case 3:
        P1_3 = dst[i];
        break;
      case 4:
        P1_4 = dst[i];
        break;
      case 5:
        P1_5 = dst[i];
        break;
      case 6:
        P1_6 = dst[i];
        break;
      case 7:
        P1_7 = dst[i];
        break;
      case 8:
        P1_8 = dst[i];
        break;
      case 9:
        P1_9 = dst[i];
        break;
      case 10:
        P1_10 = dst[i];
        break;
      case 11:
        P1_11 = dst[i];
        break;
      case 12:
        P1_12 = dst[i];
        break;
      case 13:
        P1_13 = dst[i];
        break;
      case 14:
        P1_14 = dst[i];
        break;
      case 15:
        P1_15 = dst[i];
        break;
      case 16:
        P1_16 = dst[i];
        break;
      case 17:
        P1_17 = dst[i];
        break;
      case 18:
        P1_18 = dst[i];
        break;
      case 19:
        P1_19 = dst[i];
        break;
      case 20:
        P1_20 = dst[i];
        break;
      case 21:
        P1_21 = dst[i];
        break;
      case 22:
        P1_22 = dst[i];
        break;
      case 23:
        P1_23 = dst[i];
        break;
      case 24:
        P1_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p1.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p2)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P2_0 = dst[i];
        break;
      case 1:
        P2_1 = dst[i];
        break;
      case 2:
        P2_2 = dst[i];
        break;
      case 3:
        P2_3 = dst[i];
        break;
      case 4:
        P2_4 = dst[i];
        break;
      case 5:
        P2_5 = dst[i];
        break;
      case 6:
        P2_6 = dst[i];
        break;
      case 7:
        P2_7 = dst[i];
        break;
      case 8:
        P2_8 = dst[i];
        break;
      case 9:
        P2_9 = dst[i];
        break;
      case 10:
        P2_10 = dst[i];
        break;
      case 11:
        P2_11 = dst[i];
        break;
      case 12:
        P2_12 = dst[i];
        break;
      case 13:
        P2_13 = dst[i];
        break;
      case 14:
        P2_14 = dst[i];
        break;
      case 15:
        P2_15 = dst[i];
        break;
      case 16:
        P2_16 = dst[i];
        break;
      case 17:
        P2_17 = dst[i];
        break;
      case 18:
        P2_18 = dst[i];
        break;
      case 19:
        P2_19 = dst[i];
        break;
      case 20:
        P2_20 = dst[i];
        break;
      case 21:
        P2_21 = dst[i];
        break;
      case 22:
        P2_22 = dst[i];
        break;
      case 23:
        P2_23 = dst[i];
        break;
      case 24:
        P2_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p2.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p3)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P3_0 = dst[i];
        break;
      case 1:
        P3_1 = dst[i];
        break;
      case 2:
        P3_2 = dst[i];
        break;
      case 3:
        P3_3 = dst[i];
        break;
      case 4:
        P3_4 = dst[i];
        break;
      case 5:
        P3_5 = dst[i];
        break;
      case 6:
        P3_6 = dst[i];
        break;
      case 7:
        P3_7 = dst[i];
        break;
      case 8:
        P3_8 = dst[i];
        break;
      case 9:
        P3_9 = dst[i];
        break;
      case 10:
        P3_10 = dst[i];
        break;
      case 11:
        P3_11 = dst[i];
        break;
      case 12:
        P3_12 = dst[i];
        break;
      case 13:
        P3_13 = dst[i];
        break;
      case 14:
        P3_14 = dst[i];
        break;
      case 15:
        P3_15 = dst[i];
        break;
      case 16:
        P3_16 = dst[i];
        break;
      case 17:
        P3_17 = dst[i];
        break;
      case 18:
        P3_18 = dst[i];
        break;
      case 19:
        P3_19 = dst[i];
        break;
      case 20:
        P3_20 = dst[i];
        break;
      case 21:
        P3_21 = dst[i];
        break;
      case 22:
        P3_22 = dst[i];
        break;
      case 23:
        P3_23 = dst[i];
        break;
      case 24:
        P3_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p3.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p4)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P4_0 = dst[i];
        break;
      case 1:
        P4_1 = dst[i];
        break;
      case 2:
        P4_2 = dst[i];
        break;
      case 3:
        P4_3 = dst[i];
        break;
      case 4:
        P4_4 = dst[i];
        break;
      case 5:
        P4_5 = dst[i];
        break;
      case 6:
        P4_6 = dst[i];
        break;
      case 7:
        P4_7 = dst[i];
        break;
      case 8:
        P4_8 = dst[i];
        break;
      case 9:
        P4_9 = dst[i];
        break;
      case 10:
        P4_10 = dst[i];
        break;
      case 11:
        P4_11 = dst[i];
        break;
      case 12:
        P4_12 = dst[i];
        break;
      case 13:
        P4_13 = dst[i];
        break;
      case 14:
        P4_14 = dst[i];
        break;
      case 15:
        P4_15 = dst[i];
        break;
      case 16:
        P4_16 = dst[i];
        break;
      case 17:
        P4_17 = dst[i];
        break;
      case 18:
        P4_18 = dst[i];
        break;
      case 19:
        P4_19 = dst[i];
        break;
      case 20:
        P4_20 = dst[i];
        break;
      case 21:
        P4_21 = dst[i];
        break;
      case 22:
        P4_22 = dst[i];
        break;
      case 23:
        P4_23 = dst[i];
        break;
      case 24:
        P4_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p4.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p5)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P5_0 = dst[i];
        break;
      case 1:
        P5_1 = dst[i];
        break;
      case 2:
        P5_2 = dst[i];
        break;
      case 3:
        P5_3 = dst[i];
        break;
      case 4:
        P5_4 = dst[i];
        break;
      case 5:
        P5_5 = dst[i];
        break;
      case 6:
        P5_6 = dst[i];
        break;
      case 7:
        P5_7 = dst[i];
        break;
      case 8:
        P5_8 = dst[i];
        break;
      case 9:
        P5_9 = dst[i];
        break;
      case 10:
        P5_10 = dst[i];
        break;
      case 11:
        P5_11 = dst[i];
        break;
      case 12:
        P5_12 = dst[i];
        break;
      case 13:
        P5_13 = dst[i];
        break;
      case 14:
        P5_14 = dst[i];
        break;
      case 15:
        P5_15 = dst[i];
        break;
      case 16:
        P5_16 = dst[i];
        break;
      case 17:
        P5_17 = dst[i];
        break;
      case 18:
        P5_18 = dst[i];
        break;
      case 19:
        P5_19 = dst[i];
        break;
      case 20:
        P5_20 = dst[i];
        break;
      case 21:
        P5_21 = dst[i];
        break;
      case 22:
        P5_22 = dst[i];
        break;
      case 23:
        P5_23 = dst[i];
        break;
      case 24:
        P5_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p5.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p6)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P6_0 = dst[i];
        break;
      case 1:
        P6_1 = dst[i];
        break;
      case 2:
        P6_2 = dst[i];
        break;
      case 3:
        P6_3 = dst[i];
        break;
      case 4:
        P6_4 = dst[i];
        break;
      case 5:
        P6_5 = dst[i];
        break;
      case 6:
        P6_6 = dst[i];
        break;
      case 7:
        P6_7 = dst[i];
        break;
      case 8:
        P6_8 = dst[i];
        break;
      case 9:
        P6_9 = dst[i];
        break;
      case 10:
        P6_10 = dst[i];
        break;
      case 11:
        P6_11 = dst[i];
        break;
      case 12:
        P6_12 = dst[i];
        break;
      case 13:
        P6_13 = dst[i];
        break;
      case 14:
        P6_14 = dst[i];
        break;
      case 15:
        P6_15 = dst[i];
        break;
      case 16:
        P6_16 = dst[i];
        break;
      case 17:
        P6_17 = dst[i];
        break;
      case 18:
        P6_18 = dst[i];
        break;
      case 19:
        P6_19 = dst[i];
        break;
      case 20:
        P6_20 = dst[i];
        break;
      case 21:
        P6_21 = dst[i];
        break;
      case 22:
        P6_22 = dst[i];
        break;
      case 23:
        P6_23 = dst[i];
        break;
      case 24:
        P6_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p6.ini", filecontent.c_str());
  }
  else if (String(topic) == topic_name_p7)
  {
    for (int i = 0; i < cnt; i++)
    {
      String tpl_t = "{t";
      tpl_t = tpl_t + i;
      tpl_t = tpl_t + "}";
      tpl_content.replace(tpl_t, dst[i]);
      switch (i)
      {
      case 0:
        P7_0 = dst[i];
        break;
      case 1:
        P7_1 = dst[i];
        break;
      case 2:
        P7_2 = dst[i];
        break;
      case 3:
        P7_3 = dst[i];
        break;
      case 4:
        P7_4 = dst[i];
        break;
      case 5:
        P7_5 = dst[i];
        break;
      case 6:
        P7_6 = dst[i];
        break;
      case 7:
        P7_7 = dst[i];
        break;
      case 8:
        P7_8 = dst[i];
        break;
      case 9:
        P7_9 = dst[i];
        break;
      case 10:
        P7_10 = dst[i];
        break;
      case 11:
        P7_11 = dst[i];
        break;
      case 12:
        P7_12 = dst[i];
        break;
      case 13:
        P7_13 = dst[i];
        break;
      case 14:
        P7_14 = dst[i];
        break;
      case 15:
        P7_15 = dst[i];
        break;
      case 16:
        P7_16 = dst[i];
        break;
      case 17:
        P7_17 = dst[i];
        break;
      case 18:
        P7_18 = dst[i];
        break;
      case 19:
        P7_19 = dst[i];
        break;
      case 20:
        P7_20 = dst[i];
        break;
      case 21:
        P7_21 = dst[i];
        break;
      case 22:
        P7_22 = dst[i];
        break;
      case 23:
        P7_23 = dst[i];
        break;
      case 24:
        P7_24 = dst[i];
        break;
      }
    }
    filecontent = tpl_content;
    writeFile(SPIFFS, "/p7.ini", filecontent.c_str());
  }
  else
  {
    Serial.println("mqtt action not found!");
  }

  SPIFFS.end();
}

// mqtt service
void mqttconn()
{
  while (!client.connected())
  {
    Serial.println("MQTT is connecting...");
    if (client.connect(ESP_HOST_NAME, mqttuser, mqttpwd))
    {
      led8.set(100);
      Serial.println("MQTT has connected...");
      // subscribe the checktime service
      client.subscribe("esp32/checktime");

      // subscribe the checkonine servcie
      String checkonine = ESP_HOST_NAME;
      checkonine = checkonine + "/check";
      client.subscribe(checkonine.c_str());

      // subscribe the light basic service
      String recv_topic_p = ESP_HOST_NAME;
      recv_topic_p = recv_topic_p + "/p";
      client.subscribe(recv_topic_p.c_str());

      // subscribe the all light service
      String recv_topic_pt = ESP_HOST_NAME;
      recv_topic_pt = recv_topic_pt + "/pt";      
      client.subscribe(recv_topic_pt.c_str());


      // subscibe the p1 light service
      String recv_topic_p1 = ESP_HOST_NAME;
      recv_topic_p1 = recv_topic_p1 + "/p1";
      client.subscribe(recv_topic_p1.c_str());
      // subscibe the p2 light service
      String recv_topic_p2 = ESP_HOST_NAME;
      recv_topic_p2 = recv_topic_p2 + "/p2";
      client.subscribe(recv_topic_p2.c_str());
      // subscibe the p3 light service
      String recv_topic_p3 = ESP_HOST_NAME;
      recv_topic_p3 = recv_topic_p3 + "/p3";
      client.subscribe(recv_topic_p3.c_str());
      // subscibe the p4 light service
      String recv_topic_p4 = ESP_HOST_NAME;
      recv_topic_p4 = recv_topic_p4 + "/p4";
      client.subscribe(recv_topic_p4.c_str());
      // subscibe the p5 light service
      String recv_topic_p5 = ESP_HOST_NAME;
      recv_topic_p5 = recv_topic_p5 + "/p5";
      client.subscribe(recv_topic_p5.c_str());
      // subscibe the p6 light service
      String recv_topic_p6 = ESP_HOST_NAME;
      recv_topic_p6 = recv_topic_p6 + "/p6";
      client.subscribe(recv_topic_p6.c_str());
      // subscibe the p7 light service
      String recv_topic_p7 = ESP_HOST_NAME;
      recv_topic_p7 = recv_topic_p7 + "/p7";
      client.subscribe(recv_topic_p7.c_str());

      // publish the oline notification
      String online_message = "{mid:";
      online_message = online_message + ESP_HOST_NAME;
      online_message = online_message + ",mac:";
      online_message = online_message + WiFi.macAddress();
      online_message = online_message + ",ip:";
      online_message = online_message + String(WiFi.localIP());
      online_message = online_message + "}";
      client.publish("esp32/online", online_message.c_str());
    }
    else
    {
      Serial.printf("MQTT connect failed, the rc=%d; try again in 2 second", client.state());
      delay(2000);
    }
  }
}

// light operation
void lightopr()
{
  cJSON *root = NULL;
  cJSON *item = NULL;

  String filestr = getFileString(SPIFFS, "/pwminfo.ini");
  const char *jsonstr = filestr.c_str();
  root = cJSON_Parse(jsonstr);
  String itemstr;
  item = cJSON_GetObjectItem(root, "showtype");
  itemstr = cJSON_Print(item);
  PWM_INFO_SHOWTYPE = itemstr;
  PWM_INFO_SHOWTYPE.replace("\"", "");

  item = cJSON_GetObjectItem(root, "testmode");
  itemstr = cJSON_Print(item);
  PWM_INFO_TESTMODE = itemstr;
  PWM_INFO_TESTMODE.replace("\"", "");

  item = cJSON_GetObjectItem(root, "sysdate");
  itemstr = cJSON_Print(item);
  PWM_INFO_RTC = itemstr;
  PWM_INFO_RTC.replace("\"", "");
  struct timeval stime;
  stime.tv_sec = PWM_INFO_RTC.toInt() + 30316;
  settimeofday(&stime, NULL);

  item = cJSON_GetObjectItem(root, "conmode");
  itemstr = cJSON_Print(item);
  PWM_INFO_CONMODE = itemstr;
  PWM_INFO_CONMODE.replace("\"", "");

  item = cJSON_GetObjectItem(root, "version");
  itemstr = cJSON_Print(item);
  PWM_INFO_VERSION = itemstr;
  PWM_INFO_VERSION.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p1.ini");
  const char *p1json = filestr.c_str();
  root = cJSON_Parse(p1json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P1_0 = itemstr;
  P1_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P1_1 = itemstr;
  P1_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P1_2 = itemstr;
  P1_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P1_3 = itemstr;
  P1_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P1_4 = itemstr;
  P1_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P1_5 = itemstr;
  P1_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P1_6 = itemstr;
  P1_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P1_7 = itemstr;
  P1_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P1_8 = itemstr;
  P1_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P1_9 = itemstr;
  P1_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P1_10 = itemstr;
  P1_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P1_11 = itemstr;
  P1_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P1_12 = itemstr;
  P1_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P1_13 = itemstr;
  P1_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P1_14 = itemstr;
  P1_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P1_15 = itemstr;
  P1_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P1_16 = itemstr;
  P1_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P1_17 = itemstr;
  P1_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P1_18 = itemstr;
  P1_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P1_19 = itemstr;
  P1_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P1_20 = itemstr;
  P1_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P1_21 = itemstr;
  P1_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P1_22 = itemstr;
  P1_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P1_23 = itemstr;
  P1_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P1_24 = itemstr;
  P1_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p2.ini");
  const char *p2json = filestr.c_str();
  root = cJSON_Parse(p2json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P2_0 = itemstr;
  P2_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P2_1 = itemstr;
  P2_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P2_2 = itemstr;
  P2_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P2_3 = itemstr;
  P2_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P2_4 = itemstr;
  P2_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P2_5 = itemstr;
  P2_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P2_6 = itemstr;
  P2_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P2_7 = itemstr;
  P2_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P2_8 = itemstr;
  P2_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P2_9 = itemstr;
  P2_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P2_10 = itemstr;
  P2_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P2_11 = itemstr;
  P2_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P2_12 = itemstr;
  P2_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P2_13 = itemstr;
  P2_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P2_14 = itemstr;
  P2_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P2_15 = itemstr;
  P2_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P2_16 = itemstr;
  P2_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P2_17 = itemstr;
  P2_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P2_18 = itemstr;
  P2_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P2_19 = itemstr;
  P2_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P2_20 = itemstr;
  P2_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P2_21 = itemstr;
  P2_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P2_22 = itemstr;
  P2_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P2_23 = itemstr;
  P2_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P2_24 = itemstr;
  P2_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p3.ini");
  const char *p3json = filestr.c_str();
  root = cJSON_Parse(p3json);
  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P3_0 = itemstr;
  P3_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P3_1 = itemstr;
  P3_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P3_2 = itemstr;
  P3_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P3_3 = itemstr;
  P3_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P3_4 = itemstr;
  P3_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P3_5 = itemstr;
  P3_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P3_6 = itemstr;
  P3_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P3_7 = itemstr;
  P3_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P3_8 = itemstr;
  P3_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P3_9 = itemstr;
  P3_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P3_10 = itemstr;
  P3_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P3_11 = itemstr;
  P3_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P3_12 = itemstr;
  P3_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P3_13 = itemstr;
  P3_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P3_14 = itemstr;
  P3_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P3_15 = itemstr;
  P3_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P3_16 = itemstr;
  P3_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P3_17 = itemstr;
  P3_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P3_18 = itemstr;
  P3_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P3_19 = itemstr;
  P3_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P3_20 = itemstr;
  P3_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P3_21 = itemstr;
  P3_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P3_22 = itemstr;
  P3_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P3_23 = itemstr;
  P3_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P3_24 = itemstr;
  P3_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p4.ini");
  const char *p4json = filestr.c_str();
  root = cJSON_Parse(p4json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P4_0 = itemstr;
  P4_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P4_1 = itemstr;
  P4_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P4_2 = itemstr;
  P4_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P4_3 = itemstr;
  P4_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P4_4 = itemstr;
  P4_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P4_5 = itemstr;
  P4_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P4_6 = itemstr;
  P4_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P4_7 = itemstr;
  P4_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P4_8 = itemstr;
  P4_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P4_9 = itemstr;
  P4_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P4_10 = itemstr;
  P4_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P4_11 = itemstr;
  P4_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P4_12 = itemstr;
  P4_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P4_13 = itemstr;
  P4_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P4_14 = itemstr;
  P4_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P4_15 = itemstr;
  P4_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P4_16 = itemstr;
  P4_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P4_17 = itemstr;
  P4_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P4_18 = itemstr;
  P4_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P4_19 = itemstr;
  P4_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P4_20 = itemstr;
  P4_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P4_21 = itemstr;
  P4_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P4_22 = itemstr;
  P4_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P4_23 = itemstr;
  P4_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P4_24 = itemstr;
  P4_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p5.ini");
  const char *p5json = filestr.c_str();
  root = cJSON_Parse(p5json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P5_0 = itemstr;
  P5_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P5_1 = itemstr;
  P5_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P5_2 = itemstr;
  P5_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P5_3 = itemstr;
  P5_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P5_4 = itemstr;
  P5_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P5_5 = itemstr;
  P5_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P5_6 = itemstr;
  P5_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P5_7 = itemstr;
  P5_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P5_8 = itemstr;
  P5_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P5_9 = itemstr;
  P5_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P5_10 = itemstr;
  P5_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P5_11 = itemstr;
  P5_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P5_12 = itemstr;
  P5_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P5_13 = itemstr;
  P5_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P5_14 = itemstr;
  P5_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P5_15 = itemstr;
  P5_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P5_16 = itemstr;
  P5_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P5_17 = itemstr;
  P5_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P5_18 = itemstr;
  P5_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P5_19 = itemstr;
  P5_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P5_20 = itemstr;
  P5_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P5_21 = itemstr;
  P5_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P5_22 = itemstr;
  P5_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P5_23 = itemstr;
  P5_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P5_24 = itemstr;
  P5_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p6.ini");
  const char *p6json = filestr.c_str();
  root = cJSON_Parse(p6json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P6_0 = itemstr;
  P6_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P6_1 = itemstr;
  P6_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P6_2 = itemstr;
  P6_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P6_3 = itemstr;
  P6_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P6_4 = itemstr;
  P6_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P6_5 = itemstr;
  P6_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P6_6 = itemstr;
  P6_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P6_7 = itemstr;
  P6_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P6_8 = itemstr;
  P6_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P6_9 = itemstr;
  P6_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P6_10 = itemstr;
  P6_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P6_11 = itemstr;
  P6_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P6_12 = itemstr;
  P6_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P6_13 = itemstr;
  P6_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P6_14 = itemstr;
  P6_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P6_15 = itemstr;
  P6_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P6_16 = itemstr;
  P6_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P6_17 = itemstr;
  P6_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P6_18 = itemstr;
  P6_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P6_19 = itemstr;
  P6_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P6_20 = itemstr;
  P6_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P6_21 = itemstr;
  P6_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P6_22 = itemstr;
  P6_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P6_23 = itemstr;
  P6_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P6_24 = itemstr;
  P6_24.replace("\"", "");

  filestr = getFileString(SPIFFS, "/p7.ini");
  const char *p7json = filestr.c_str();
  root = cJSON_Parse(p7json);

  item = cJSON_GetObjectItem(root, "t0");
  itemstr = cJSON_Print(item);
  P7_0 = itemstr;
  P7_0.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t1");
  itemstr = cJSON_Print(item);
  P7_1 = itemstr;
  P7_1.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t2");
  itemstr = cJSON_Print(item);
  P7_2 = itemstr;
  P7_2.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t3");
  itemstr = cJSON_Print(item);
  P7_3 = itemstr;
  P7_3.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t4");
  itemstr = cJSON_Print(item);
  P7_4 = itemstr;
  P7_4.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t5");
  itemstr = cJSON_Print(item);
  P7_5 = itemstr;
  P7_5.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t6");
  itemstr = cJSON_Print(item);
  P7_6 = itemstr;
  P7_6.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t7");
  itemstr = cJSON_Print(item);
  P7_7 = itemstr;
  P7_7.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t8");
  itemstr = cJSON_Print(item);
  P7_8 = itemstr;
  P7_8.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t9");
  itemstr = cJSON_Print(item);
  P7_9 = itemstr;
  P7_9.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t10");
  itemstr = cJSON_Print(item);
  P7_10 = itemstr;
  P7_10.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t11");
  itemstr = cJSON_Print(item);
  P7_11 = itemstr;
  P7_11.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t12");
  itemstr = cJSON_Print(item);
  P7_12 = itemstr;
  P7_12.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t13");
  itemstr = cJSON_Print(item);
  P7_13 = itemstr;
  P7_13.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t14");
  itemstr = cJSON_Print(item);
  P7_14 = itemstr;
  P7_14.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t15");
  itemstr = cJSON_Print(item);
  P7_15 = itemstr;
  P7_15.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t16");
  itemstr = cJSON_Print(item);
  P7_16 = itemstr;
  P7_16.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t17");
  itemstr = cJSON_Print(item);
  P7_17 = itemstr;
  P7_17.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t18");
  itemstr = cJSON_Print(item);
  P7_18 = itemstr;
  P7_18.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t19");
  itemstr = cJSON_Print(item);
  P7_19 = itemstr;
  P7_19.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t20");
  itemstr = cJSON_Print(item);
  P7_20 = itemstr;
  P7_20.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t21");
  itemstr = cJSON_Print(item);
  P7_21 = itemstr;
  P7_21.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t22");
  itemstr = cJSON_Print(item);
  P7_22 = itemstr;
  P7_22.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t23");
  itemstr = cJSON_Print(item);
  P7_23 = itemstr;
  P7_23.replace("\"", "");
  item = cJSON_GetObjectItem(root, "t24");
  itemstr = cJSON_Print(item);
  P7_24 = itemstr;
  P7_24.replace("\"", "");
}

// esp32 init config
void setup()
{
  Serial.begin(115200);
  RESET_FLAG = false;
  IS_SMART = false;
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON), handleRestButtonChanged, CHANGE);
  WiFi.mode(WIFI_AP_STA);

  // struct timeval stime;
  // stime.tv_sec = 1536725527 + 30316;
  // settimeofday(&stime, NULL);
  // time_t t = time(NULL);
  // struct tm *t_st;
  // t_st = localtime(&t);
  // char nowtime[24];
  // memset(nowtime, 0, sizeof(nowtime));
  // strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", t_st);
  // String strDate = nowtime;
  // Serial.println(strDate);
  Serial.println("the AP name is : " + String(ssid) + " password is: " + String(password) + "\n the mac address is: " + WiFi.macAddress());
  Serial.print("the current version is: ");
  Serial.println(VERSION_NUM);
  // /*use mdns for host name resolution*/
  if (!MDNS.begin(host))
  { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }

  led1.setup();
  led2.setup();
  led3.setup();
  led4.setup();
  led5.setup();
  led6.setup();
  led7.setup();
  led8.setup();

  led1.set(0);
  led2.set(0);
  led3.set(0);
  led4.set(0);
  led5.set(0);
  led6.set(0);
  led7.set(0);
  led8.set(0);

  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  else
  {
    // deleteFile(SPIFFS,"/wifi.ini");

    if (!SPIFFS.exists("/pwminfo.ini"))
    {
      Serial.println("begin to init system file");
      initFileSystem();
      Serial.println("init success...");
    }
    else
    {
      // String tempfile = getFileString(SPIFFS,"/wifi.ini");
      // Serial.println(tempfile);
      if (SPIFFS.exists("/wifi.ini"))
      {
        String filestr;
        filestr = getFileString(SPIFFS, "/wifi.ini");

        cJSON *root = NULL;
        cJSON *item = NULL;
        const char *jsonstr = filestr.c_str();
        root = cJSON_Parse(jsonstr);
        String itemstr;
        item = cJSON_GetObjectItem(root, "ssid");
        itemstr = cJSON_Print(item);
        String ssid = itemstr;
        if (strcmp(ssid.c_str(), "") == 0)
        {
          Serial.println("the ssid has missed");
          deleteFile(SPIFFS, "/wifi.ini");
          ESP.restart();
        }
        item = cJSON_GetObjectItem(root, "pwd");
        itemstr = cJSON_Print(item);
        String pwd = itemstr;

        ssid.replace("\"", "");
        pwd.replace("\"", "");
        SSID = ssid.c_str();
        SSID_PWD = pwd.c_str();
        WiFi.begin(ssid.c_str(), pwd.c_str());
        int i = 30;
        Serial.printf("WIFI is connecting");
        while ((WiFi.status() != WL_CONNECTED) && i > 0)
        {
          delay(1000);
          Serial.printf(".");
          i = i - 1;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          IS_SMART = true;
          Serial.println(WiFi.localIP());
          led8.set(100);
          client.setServer(mqttServer, mqttPort);
          client.setCallback(callback);
          mqttconn();
        }
      }
      else
      {
        int trytime = 50;
        WiFi.beginSmartConfig();
        Serial.printf("smart config begin");
        while (1)
        {
          Serial.print('.');
          led8.set(100);
          delay(500);
          led8.set(0);
          delay(500);
          if (WiFi.smartConfigDone())
          {
            IS_SMART = true;
            led8.set(100);
            Serial.println("SmartConfig Success");

            SSID = WiFi.SSID();
            SSID_PWD = WiFi.psk();
            String filecontent;
            filecontent = filecontent + "{\"ssid\":\"";
            filecontent = filecontent + WiFi.SSID().c_str();
            filecontent = filecontent + "\",\"pwd\":\"";
            filecontent = filecontent + WiFi.psk().c_str();
            filecontent = filecontent + "\"}";

            if (strcmp(WiFi.SSID().c_str(), "") == 0)
            {
              String tempfile = getFileString(SPIFFS, "/wifi.ini");
              Serial.println(tempfile);
              deleteFile(SPIFFS, "/wifi.ini");
              led8.set(0);
            }
            else
            {
              writeFile(SPIFFS, "/wifi.ini", filecontent.c_str());
              led8.set(100);
              delay(2000);
              ESP.restart();
            }
            break;
          }
          trytime = trytime - 1;
          if (trytime < 0)
          {
            Serial.println("");
            Serial.println("smartconfig cancel");
            WiFi.stopSmartConfig();
            WiFi.softAP(ssid, password);
            led8.set(0);
            break;
          }
        }
      }
      // online or offline light operation
      lightopr();
    }
  }

  SPIFFS.end();

  // index page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    String filestr;
    filestr = getFileString(SPIFFS, "/pwminfo.ini");
    cJSON *root = NULL;
    cJSON *item = NULL;
    const char *jsonstr = filestr.c_str();
    root = cJSON_Parse(jsonstr);

    String itemstr;
    item = cJSON_GetObjectItem(root, "showtype");
    itemstr = cJSON_Print(item);
    String showtype = itemstr;
    item = cJSON_GetObjectItem(root, "testmode");
    itemstr = cJSON_Print(item);
    String testmode = itemstr;
    item = cJSON_GetObjectItem(root, "sysdate");
    itemstr = cJSON_Print(item);
    String sysdate = itemstr;
    sysdate.replace("\"", "");
    PWM_INFO_RTC = sysdate;
    item = cJSON_GetObjectItem(root, "status");
    itemstr = cJSON_Print(item);
    String status = itemstr;
    item = cJSON_GetObjectItem(root, "conmode");
    itemstr = cJSON_Print(item);
    String conmode = itemstr;

    //todo
    String version = VERSION_NUM;
    Serial.println("the version is: " + version);
    String html = "";
    html = html + "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>NodeMCU Control Page</title><!--<script type=\"text/javascript\"src=\"jquery.js\"></script>--></head><body><div><h1 id=\"title\">基本信息</h1><table><tr><th style=\"text-align:left;\">版本号</th></tr><tr><td><span id=\"spversion\"></span></td></tr><tr><th style=\"text-align:left;\">系统时间</th></tr><tr><td><span id=\"spCurrent\"></span></td></tr><tr><th style=\"text-align: left;\">修改时间</th></tr><tr><td><input type=\"text\"value=\"2018-03-03 00:00:00\"id=\"txtsysdate\"/>*时间格式:2018-03-03 00:00:00</td></tr><tr><th style=\"text-align:left;\">循环模式</th></tr><tr><td><input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>循环模式<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>固定模式</td></tr><tr id=\"thtest\"><th style=\"text-align:left;\">是否测试</th></tr><tr id=\"tdtest\"><td><input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>否<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>是</td></tr><tr id=\"thonline\"><th style=\"text-align:left;\">云端控制</th></tr><tr id=\"tdonline\"><td><input value=\"local\"type=\"radio\"name=\"onlineMode\"id=\"rdlocal\"/>否<input value=\"online\"type=\"radio\"name=\"onlineMode\"id=\"rdonline\"/>是</td></tr><tr><td><input type=\"button\"value=\"联网设置\"id=\"btnwifi\"onclick=\"wifi();\"/><input type=\"submit\"value=\"保存\"id=\"submit\"onclick=\"submit();\"/><input type=\"button\"value=\"恢复出厂\"id=\"btnstop\"onclick=\"init();\"/><input type=\"button\"value=\"重启\"id=\"btnReset\"onclick=\"reset();\"/><input type=\"button\"value=\"更新固件\"id=\"btnupload\"onclick=\"upload();\"/></td></tr></table><hr/><h1 id=\"title\">灯光控制</h1><table><tr><th>第一排</th><th>第二排</th><th>第三排</th><th>第四排</th><th>第五排</th><th>第六排</th><th>第七排</th></tr><tr><td><a href=\"/p?mode=p1\">查看/修改</a></td><td><a href=\"/p?mode=p2\">查看/修改</a></td><td><a href=\"/p?mode=p3\">查看/修改</a></td><td><a href=\"/p?mode=p4\">查看/修改</a></td><td><a href=\"/p?mode=p5\">查看/修改</a></td><td><a href=\"/p?mode=p6\">查看/修改</a></td><td><a href=\"/p?mode=p7\">查看/修改</a></td></tr></table></div><script>function submit(){var selectshowtype=document.getElementsByName('showtype');var showtypevalue=\"\";for(var i=0;i<selectshowtype.length;i++){if(selectshowtype[i].checked){showtypevalue=selectshowtype[i].value;break}}var selecttestmode=document.getElementsByName('testMode');var testmodevalue=\"\";for(var i=0;i<selecttestmode.length;i++){if(selecttestmode[i].checked){testmodevalue=selecttestmode[i].value;break}}var str=document.getElementById('txtsysdate').value;str=str.replace(/-/g,\"/\");var date=new Date(str);var unixDate=date.getTime()/1000|0;console.log(unixDate);var selectedconnectionmode=document.getElementsByName(\"onlineMode\");var connectionmodevalue=\"\";for(var i=0;i<selectedconnectionmode.length;i++){if(selectedconnectionmode[i].checked){connectionmodevalue=selectedconnectionmode[i].value;break}}alert('保存成功');var url=\"pwmopr?showtype=\"+showtypevalue+\"&testmode=\"+testmodevalue+\"&sysdate=\"+unixDate+\"&conmode=\"+connectionmodevalue;window.location.href=url}function init(){alert('已恢复出厂设置!');var url=\"init\";window.location.href=url}function wifi(){var url=\"wifi\";window.location.href=url}function reset(){var url=\"reset\";alert(\"已重启,请关闭当前页面\");window.location.href=url}function upload(){var url=\"upload\";window.location.href=url}</script></body></html>";

    String tpl_currentdate = "<span id=\"spCurrent\"></span>";
    String change_currentdate = "<span id=\"spCurrent\">";
    time_t t = time(NULL);
    struct tm *t_st;
    t_st = localtime(&t);
    char nowtime[24];
    memset(nowtime, 0, sizeof(nowtime));
    strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", t_st);
    String strDate = nowtime;
    //Serial.println(strDate);

    change_currentdate = change_currentdate + strDate;
    change_currentdate = change_currentdate + "</span>";
    html.replace(tpl_currentdate, change_currentdate);

    String tpl_txtcurrentdate = "<input type=\"text\"value=\"2018-03-03 00:00:00\"id=\"txtsysdate\"/>";
    String change_txtcurrentdate = "<input type=\"text\"value=\"";
    change_txtcurrentdate = change_txtcurrentdate + strDate;
    change_txtcurrentdate = change_txtcurrentdate + "\"id=\"txtsysdate\"/>";
    html.replace(tpl_txtcurrentdate, change_txtcurrentdate);

    String tpl_cyclemode = "<input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>";
    String tpl_fixmode = "<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>";
    String change_runtype;
    if (showtype == "\"repeat\"")
    {
      change_runtype = "<input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\" checked/>";
      html.replace(tpl_cyclemode, change_runtype);
    }
    else if (showtype == "\"fix\"")
    {
      change_runtype = "<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\" checked/>";
      html.replace(tpl_fixmode, change_runtype);
    }

    String tpl_prodmode = "<input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>";
    String tpl_testmode = "<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>";
    String change_protype;
    if (testmode == "\"production\"")
    {
      change_protype = "<input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\" checked/>";
      html.replace(tpl_prodmode, change_protype);
    }
    else if (testmode == "\"test\"")
    {
      change_protype = "<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\" checked/>";
      html.replace(tpl_testmode, change_protype);
    }

    String tpl_oninemode = "<input value=\"online\"type=\"radio\"name=\"onlineMode\"id=\"rdonline\"/>";
    String tpl_localmode = "<input value=\"local\"type=\"radio\"name=\"onlineMode\"id=\"rdlocal\"/>";
    String Change_connectiontype;
    if (conmode == "\"online\"")
    {
      Change_connectiontype = "<input value=\"online\"type=\"radio\"name=\"onlineMode\"id=\"rdonline\" checked/>";
      html.replace(tpl_oninemode, Change_connectiontype);
    }
    else if (conmode == "\"local\"")
    {
      Change_connectiontype = "<input value=\"local\"type=\"radio\"name=\"onlineMode\"id=\"rdlocal\" checked/>";
      html.replace(tpl_localmode, Change_connectiontype);
    }

    // todo version
    String tpl_version = "<span id=\"spversion\"></span>";
    String change_version = "<span id=\"spversion\">";
    change_version = change_version + "[";
    change_version = change_version + ESP_HOST_NAME;
    change_version = change_version + "] v";
    change_version = change_version + version;
    change_version = change_version + "</span>";
    html.replace(tpl_version, change_version);

    request->send(200, "text/html", html);
  });

  // light page
  server.on("/p", HTTP_GET, [](AsyncWebServerRequest *request) {
    String lightSeq;
    String html = "";
    if (request->hasParam("mode"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      lightSeq = request->getParam("mode")->value();
      String filestr;
      //filestr = getFileString(SPIFFS,"/pwminfo.ini");

      if (lightSeq == "p1")
      {
        filestr = getFileString(SPIFFS, "/p1.ini");
      }
      else if (lightSeq == "p2")
      {
        filestr = getFileString(SPIFFS, "/p2.ini");
      }
      else if (lightSeq == "p3")
      {
        filestr = getFileString(SPIFFS, "/p3.ini");
      }
      else if (lightSeq == "p4")
      {
        filestr = getFileString(SPIFFS, "/p4.ini");
      }
      else if (lightSeq == "p5")
      {
        filestr = getFileString(SPIFFS, "/p5.ini");
      }
      else if (lightSeq == "p6")
      {
        filestr = getFileString(SPIFFS, "/p6.ini");
      }
      else if (lightSeq == "p7")
      {
        filestr = getFileString(SPIFFS, "/p7.ini");
      }

      cJSON *root = NULL;
      cJSON *item = NULL;
      const char *jsonstr = filestr.c_str();
      root = cJSON_Parse(jsonstr);

      if (!root)
      {
        Serial.println("Error occured");
      }
      else
      {
        //String lid;
        String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>操作</title></head><body><h1><span id=\"sptitle\">亮度操作</span></h1><Table><tr><th>0点</th><th>1点</th><th>2点</th><th>3点</th><th>4点</th><th>5点</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl0\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl1\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl2\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl3\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl4\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl5\"/></td></tr><tr><th>6点</th><th>7点</th><th>8点</th><th>9点</th><th>10点</th><th>11点</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl6\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl7\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl8\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl9\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl10\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl11\"/></td></tr><tr><th>12点</th><th>13点</th><th>14点</th><th>15点</th><th>16点</th><th>17点</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl12\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl13\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl14\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl15\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl16\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl17\"/></td></tr><tr><th>18点</th><th>19点</th><th>20点</th><th>21点</th><th>22点</th><th>23点</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl18\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl19\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl20\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl21\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl22\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl23\"/></td></tr><tr><th>固定</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl24\"/></td></tr><tr><td><input onclick=\"submit();\"type=\"submit\"id=\"submit\"value=\"保存\"/><input onclick=\"back();\"type=\"button\"id=\"btnback\"value=\"返回\"/></td></tr><tr><td sylte=\"color:red\">取值范围0~100</td></tr></Table></body><script language=\"javascript\">function submit(){var t0=document.getElementById(\"txtl0\").value;var t1=document.getElementById(\"txtl1\").value;var t2=document.getElementById(\"txtl2\").value;var t3=document.getElementById(\"txtl3\").value;var t4=document.getElementById(\"txtl4\").value;var t5=document.getElementById(\"txtl5\").value;var t6=document.getElementById(\"txtl6\").value;var t7=document.getElementById(\"txtl7\").value;var t8=document.getElementById(\"txtl8\").value;var t9=document.getElementById(\"txtl9\").value;var t10=document.getElementById(\"txtl10\").value;var t11=document.getElementById(\"txtl11\").value;var t12=document.getElementById(\"txtl12\").value;var t13=document.getElementById(\"txtl13\").value;var t14=document.getElementById(\"txtl14\").value;var t15=document.getElementById(\"txtl15\").value;var t16=document.getElementById(\"txtl16\").value;var t17=document.getElementById(\"txtl17\").value;var t18=document.getElementById(\"txtl18\").value;var t19=document.getElementById(\"txtl19\").value;var t20=document.getElementById(\"txtl20\").value;var t21=document.getElementById(\"txtl21\").value;var t22=document.getElementById(\"txtl22\").value;var t23=document.getElementById(\"txtl23\").value;var t24=document.getElementById(\"txtl24\").value;var mode=document.getElementById(\"sptitle\").innerHTML;var url=\"setp?mode=\"+mode+\"&t0=\"+t0+\"&t1=\"+t1+\"&t2=\"+t2+\"&t3=\"+t3+\"&t4=\"+t4+\"&t5=\"+t5+\"&t6=\"+t6+\"&t7=\"+t7+\"&t8=\"+t8+\"&t9=\"+t9+\"&t10=\"+t10+\"&t11=\"+t11+\"&t12=\"+t12+\"&t13=\"+t13+\"&t14=\"+t14+\"&t15=\"+t15+\"&t16=\"+t16+\"&t17=\"+t17+\"&t18=\"+t18+\"&t19=\"+t19+\"&t20=\"+t20+\"&t21=\"+t21+\"&t22=\"+t22+\"&t23=\"+t23+\"&t24=\"+t24;alert('保存成功');window.location.href=url}function back(){var url=\"/\";window.location.href=url}</script></html>";
        String tpl_data = "<input type=\"text\"width=\"50\"id=\"txtl{lid}\"/>";
        String itemstr;
        String changeinput;

        String tpl_title = "<span id=\"sptitle\">亮度操作</span>";
        String chang_title = "<span id=\"sptitle\">" + lightSeq + "</span>";
        rawhtml.replace(tpl_title, chang_title);

        item = cJSON_GetObjectItem(root, "t0");
        itemstr = cJSON_Print(item);
        String t0txt = tpl_data;
        t0txt.replace("{lid}", "0");
        changeinput = t0txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t0txt, changeinput);

        item = cJSON_GetObjectItem(root, "t1");
        itemstr = cJSON_Print(item);
        String t1txt = tpl_data;
        t1txt.replace("{lid}", "1");
        changeinput = t1txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t1txt, changeinput);

        item = cJSON_GetObjectItem(root, "t2");
        itemstr = cJSON_Print(item);
        String t2txt = tpl_data;
        t2txt.replace("{lid}", "2");
        changeinput = t2txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t2txt, changeinput);

        item = cJSON_GetObjectItem(root, "t3");
        itemstr = cJSON_Print(item);
        String t3txt = tpl_data;
        t3txt.replace("{lid}", "3");
        changeinput = t3txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t3txt, changeinput);

        item = cJSON_GetObjectItem(root, "t4");
        itemstr = cJSON_Print(item);
        String t4txt = tpl_data;
        t4txt.replace("{lid}", "4");
        changeinput = t4txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t4txt, changeinput);

        item = cJSON_GetObjectItem(root, "t5");
        itemstr = cJSON_Print(item);
        String t5txt = tpl_data;
        t5txt.replace("{lid}", "5");
        changeinput = t5txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t5txt, changeinput);

        item = cJSON_GetObjectItem(root, "t6");
        itemstr = cJSON_Print(item);
        String t6txt = tpl_data;
        t6txt.replace("{lid}", "6");
        changeinput = t6txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t6txt, changeinput);

        item = cJSON_GetObjectItem(root, "t7");
        itemstr = cJSON_Print(item);
        String t7txt = tpl_data;
        t7txt.replace("{lid}", "7");
        changeinput = t7txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t7txt, changeinput);

        item = cJSON_GetObjectItem(root, "t8");
        itemstr = cJSON_Print(item);
        String t8txt = tpl_data;
        t8txt.replace("{lid}", "8");
        changeinput = t8txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t8txt, changeinput);

        item = cJSON_GetObjectItem(root, "t9");
        itemstr = cJSON_Print(item);
        String t9txt = tpl_data;
        t9txt.replace("{lid}", "9");
        changeinput = t9txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t9txt, changeinput);

        item = cJSON_GetObjectItem(root, "t10");
        itemstr = cJSON_Print(item);
        String t10txt = tpl_data;
        t10txt.replace("{lid}", "10");
        changeinput = t10txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t10txt, changeinput);

        item = cJSON_GetObjectItem(root, "t11");
        itemstr = cJSON_Print(item);
        String t11txt = tpl_data;
        t11txt.replace("{lid}", "11");
        changeinput = t11txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t11txt, changeinput);

        item = cJSON_GetObjectItem(root, "t12");
        itemstr = cJSON_Print(item);
        String t12txt = tpl_data;
        t12txt.replace("{lid}", "12");
        changeinput = t12txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t12txt, changeinput);

        item = cJSON_GetObjectItem(root, "t13");
        itemstr = cJSON_Print(item);
        String t13txt = tpl_data;
        t13txt.replace("{lid}", "13");
        changeinput = t13txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t13txt, changeinput);

        item = cJSON_GetObjectItem(root, "t14");
        itemstr = cJSON_Print(item);
        String t14txt = tpl_data;
        t14txt.replace("{lid}", "14");
        changeinput = t14txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t14txt, changeinput);

        item = cJSON_GetObjectItem(root, "t15");
        itemstr = cJSON_Print(item);
        String t15txt = tpl_data;
        t15txt.replace("{lid}", "15");
        changeinput = t15txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t15txt, changeinput);

        item = cJSON_GetObjectItem(root, "t16");
        itemstr = cJSON_Print(item);
        String t16txt = tpl_data;
        t16txt.replace("{lid}", "16");
        changeinput = t16txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t16txt, changeinput);

        item = cJSON_GetObjectItem(root, "t17");
        itemstr = cJSON_Print(item);
        String t17txt = tpl_data;
        t17txt.replace("{lid}", "17");
        changeinput = t17txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t17txt, changeinput);

        item = cJSON_GetObjectItem(root, "t18");
        itemstr = cJSON_Print(item);
        String t18txt = tpl_data;
        t18txt.replace("{lid}", "18");
        changeinput = t18txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t18txt, changeinput);

        item = cJSON_GetObjectItem(root, "t19");
        itemstr = cJSON_Print(item);
        String t19txt = tpl_data;
        t19txt.replace("{lid}", "19");
        changeinput = t19txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t19txt, changeinput);

        item = cJSON_GetObjectItem(root, "t20");
        itemstr = cJSON_Print(item);
        String t20txt = tpl_data;
        t20txt.replace("{lid}", "20");
        changeinput = t20txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t20txt, changeinput);

        item = cJSON_GetObjectItem(root, "t21");
        itemstr = cJSON_Print(item);
        String t21txt = tpl_data;
        t21txt.replace("{lid}", "21");
        changeinput = t21txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t21txt, changeinput);

        item = cJSON_GetObjectItem(root, "t22");
        itemstr = cJSON_Print(item);
        String t22txt = tpl_data;
        t22txt.replace("{lid}", "22");
        changeinput = t22txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t22txt, changeinput);

        item = cJSON_GetObjectItem(root, "t23");
        itemstr = cJSON_Print(item);
        String t23txt = tpl_data;
        t23txt.replace("{lid}", "23");
        changeinput = t23txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t23txt, changeinput);

        item = cJSON_GetObjectItem(root, "t24");
        itemstr = cJSON_Print(item);
        String t24txt = tpl_data;
        t24txt.replace("{lid}", "24");
        changeinput = t24txt;
        changeinput.replace("/>", "value='" + itemstr + "'/>");
        rawhtml.replace(t24txt, changeinput);
        html = html + rawhtml;
        SPIFFS.end();
      }
    }
    else
    {
      html = html + "<p>参数错误</p>";
    }
    request->send(200, "text/html", html);
  });

  // light setting
  server.on("/setp", HTTP_GET, [](AsyncWebServerRequest *request) {
    String lightSeq;
    String t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19, t20, t21, t22, t23, t24;
    String resfilecontent = "";
    String filename = "";
    String html;

    if (request->hasParam("mode"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      lightSeq = request->getParam("mode")->value();
      filename = "/" + lightSeq + ".ini";
      t0 = request->getParam("t0")->value();
      t1 = request->getParam("t1")->value();
      t2 = request->getParam("t2")->value();
      t3 = request->getParam("t3")->value();
      t4 = request->getParam("t4")->value();
      t5 = request->getParam("t5")->value();
      t6 = request->getParam("t6")->value();
      t7 = request->getParam("t7")->value();
      t8 = request->getParam("t8")->value();
      t9 = request->getParam("t9")->value();
      t10 = request->getParam("t10")->value();
      t11 = request->getParam("t11")->value();
      t12 = request->getParam("t12")->value();
      t13 = request->getParam("t13")->value();
      t14 = request->getParam("t14")->value();
      t15 = request->getParam("t15")->value();
      t16 = request->getParam("t16")->value();
      t17 = request->getParam("t17")->value();
      t18 = request->getParam("t18")->value();
      t19 = request->getParam("t19")->value();
      t20 = request->getParam("t20")->value();
      t21 = request->getParam("t21")->value();
      t22 = request->getParam("t22")->value();
      t23 = request->getParam("t23")->value();
      t24 = request->getParam("t24")->value();

      if (lightSeq == "p1")
      {
        P1_0 = t0;
        P1_1 = t1;
        P1_2 = t2;
        P1_3 = t3;
        P1_4 = t4;
        P1_5 = t5;
        P1_6 = t6;
        P1_7 = t7;
        P1_8 = t8;
        P1_9 = t9;
        P1_10 = t10;
        P1_11 = t11;
        P1_12 = t12;
        P1_13 = t13;
        P1_14 = t14;
        P1_15 = t15;
        P1_16 = t16;
        P1_17 = t17;
        P1_18 = t18;
        P1_19 = t19;
        P1_20 = t20;
        P1_21 = t21;
        P1_22 = t22;
        P1_23 = t23;
        P1_24 = t24;
      }
      else if (lightSeq == "p2")
      {
        P2_0 = t0;
        P2_1 = t1;
        P2_2 = t2;
        P2_3 = t3;
        P2_4 = t4;
        P2_5 = t5;
        P2_6 = t6;
        P2_7 = t7;
        P2_8 = t8;
        P2_9 = t9;
        P2_10 = t10;
        P2_11 = t11;
        P2_12 = t12;
        P2_13 = t13;
        P2_14 = t14;
        P2_15 = t15;
        P2_16 = t16;
        P2_17 = t17;
        P2_18 = t18;
        P2_19 = t19;
        P2_20 = t20;
        P2_21 = t21;
        P2_22 = t22;
        P2_23 = t23;
        P2_24 = t24;
      }
      else if (lightSeq == "p3")
      {
        P3_0 = t0;
        P3_1 = t1;
        P3_2 = t2;
        P3_3 = t3;
        P3_4 = t4;
        P3_5 = t5;
        P3_6 = t6;
        P3_7 = t7;
        P3_8 = t8;
        P3_9 = t9;
        P3_10 = t10;
        P3_11 = t11;
        P3_12 = t12;
        P3_13 = t13;
        P3_14 = t14;
        P3_15 = t15;
        P3_16 = t16;
        P3_17 = t17;
        P3_18 = t18;
        P3_19 = t19;
        P3_20 = t20;
        P3_21 = t21;
        P3_22 = t22;
        P3_23 = t23;
        P3_24 = t24;
      }
      else if (lightSeq == "p4")
      {
        P4_0 = t0;
        P4_1 = t1;
        P4_2 = t2;
        P4_3 = t3;
        P4_4 = t4;
        P4_5 = t5;
        P4_6 = t6;
        P4_7 = t7;
        P4_8 = t8;
        P4_9 = t9;
        P4_10 = t10;
        P4_11 = t11;
        P4_12 = t12;
        P4_13 = t13;
        P4_14 = t14;
        P4_15 = t15;
        P4_16 = t16;
        P4_17 = t17;
        P4_18 = t18;
        P4_19 = t19;
        P4_20 = t20;
        P4_21 = t21;
        P4_22 = t22;
        P4_23 = t23;
        P4_24 = t24;
      }
      else if (lightSeq == "p5")
      {
        P5_0 = t0;
        P5_1 = t1;
        P5_2 = t2;
        P5_3 = t3;
        P5_4 = t4;
        P5_5 = t5;
        P5_6 = t6;
        P5_7 = t7;
        P5_8 = t8;
        P5_9 = t9;
        P5_10 = t10;
        P5_11 = t11;
        P5_12 = t12;
        P5_13 = t13;
        P5_14 = t14;
        P5_15 = t15;
        P5_16 = t16;
        P5_17 = t17;
        P5_18 = t18;
        P5_19 = t19;
        P5_20 = t20;
        P5_21 = t21;
        P5_22 = t22;
        P5_23 = t23;
        P5_24 = t24;
      }
      else if (lightSeq == "p6")
      {
        P6_0 = t0;
        P6_1 = t1;
        P6_2 = t2;
        P6_3 = t3;
        P6_4 = t4;
        P6_5 = t5;
        P6_6 = t6;
        P6_7 = t7;
        P6_8 = t8;
        P6_9 = t9;
        P6_10 = t10;
        P6_11 = t11;
        P6_12 = t12;
        P6_13 = t13;
        P6_14 = t14;
        P6_15 = t15;
        P6_16 = t16;
        P6_17 = t17;
        P6_18 = t18;
        P6_19 = t19;
        P6_20 = t20;
        P6_21 = t21;
        P6_22 = t22;
        P6_23 = t23;
        P6_24 = t24;
      }
      else if (lightSeq == "p7")
      {
        P7_0 = t0;
        P7_1 = t1;
        P7_2 = t2;
        P7_3 = t3;
        P7_4 = t4;
        P7_5 = t5;
        P7_6 = t6;
        P7_7 = t7;
        P7_8 = t8;
        P7_9 = t9;
        P7_10 = t10;
        P7_11 = t11;
        P7_12 = t12;
        P7_13 = t13;
        P7_14 = t14;
        P7_15 = t15;
        P7_16 = t16;
        P7_17 = t17;
        P7_18 = t18;
        P7_19 = t19;
        P7_20 = t20;
        P7_21 = t21;
        P7_22 = t22;
        P7_23 = t23;
        P7_24 = t24;
      }

      resfilecontent = resfilecontent + "{\"t0\":";
      resfilecontent = resfilecontent + t0;
      resfilecontent = resfilecontent + ",\"t1\":";
      resfilecontent = resfilecontent + t1;
      resfilecontent = resfilecontent + ",\"t2\":";
      resfilecontent = resfilecontent + t2;
      resfilecontent = resfilecontent + ",\"t3\":";
      resfilecontent = resfilecontent + t3;
      resfilecontent = resfilecontent + ",\"t4\":";
      resfilecontent = resfilecontent + t4;
      resfilecontent = resfilecontent + ",\"t5\":";
      resfilecontent = resfilecontent + t5;
      resfilecontent = resfilecontent + ",\"t6\":";
      resfilecontent = resfilecontent + t6;
      resfilecontent = resfilecontent + ",\"t7\":";
      resfilecontent = resfilecontent + t7;
      resfilecontent = resfilecontent + ",\"t8\":";
      resfilecontent = resfilecontent + t8;
      resfilecontent = resfilecontent + ",\"t9\":";
      resfilecontent = resfilecontent + t9;
      resfilecontent = resfilecontent + ",\"t10\":";
      resfilecontent = resfilecontent + t10;
      resfilecontent = resfilecontent + ",\"t11\":";
      resfilecontent = resfilecontent + t11;
      resfilecontent = resfilecontent + ",\"t12\":";
      resfilecontent = resfilecontent + t12;
      resfilecontent = resfilecontent + ",\"t13\":";
      resfilecontent = resfilecontent + t13;
      resfilecontent = resfilecontent + ",\"t14\":";
      resfilecontent = resfilecontent + t14;
      resfilecontent = resfilecontent + ",\"t15\":";
      resfilecontent = resfilecontent + t15;
      resfilecontent = resfilecontent + ",\"t16\":";
      resfilecontent = resfilecontent + t16;
      resfilecontent = resfilecontent + ",\"t17\":";
      resfilecontent = resfilecontent + t17;
      resfilecontent = resfilecontent + ",\"t18\":";
      resfilecontent = resfilecontent + t18;
      resfilecontent = resfilecontent + ",\"t19\":";
      resfilecontent = resfilecontent + t19;
      resfilecontent = resfilecontent + ",\"t20\":";
      resfilecontent = resfilecontent + t20;
      resfilecontent = resfilecontent + ",\"t21\":";
      resfilecontent = resfilecontent + t21;
      resfilecontent = resfilecontent + ",\"t22\":";
      resfilecontent = resfilecontent + t22;
      resfilecontent = resfilecontent + ",\"t23\":";
      resfilecontent = resfilecontent + t23;
      resfilecontent = resfilecontent + ",\"t24\":";
      resfilecontent = resfilecontent + t24 + "}";
      writeFile(SPIFFS, filename.c_str(), resfilecontent.c_str());
      SPIFFS.end();
    }
    else
    {
      html = "params not found";
    }
    request->redirect("/p?mode=" + lightSeq);
  });

  // basic page
  server.on("/pwmopr", HTTP_GET, [](AsyncWebServerRequest *request) {
    String showtype;
    String testmode;
    String sysdate;
    String conmode;
    if (request->hasParam("showtype"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      showtype = request->getParam("showtype")->value();
      testmode = request->getParam("testmode")->value();
      sysdate = request->getParam("sysdate")->value();
      conmode = request->getParam("conmode")->value();

      PWM_INFO_SHOWTYPE = showtype;
      PWM_INFO_TESTMODE = testmode;
      PWM_INFO_CONMODE = conmode;

      String filecontent;
      filecontent = "{\"showtype\":\"";
      filecontent = filecontent + showtype;
      filecontent = filecontent + "\",\"testmode\":\"";
      filecontent = filecontent + testmode;
      filecontent = filecontent + "\",\"sysdate\":\"";
      filecontent = filecontent + sysdate;
      filecontent = filecontent + "\",\"conmode\":\"";
      filecontent = filecontent + conmode;
      filecontent = filecontent + "\"}";
      PWM_INFO_RTC = sysdate;

      struct timeval stime;
      stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
      settimeofday(&stime, NULL);
      //Serial.println("the new time is: " + PWM_INFO_RTC);
      writeFile(SPIFFS, "/pwminfo.ini", filecontent.c_str());
      SPIFFS.end();
    }
    request->redirect("/");
  });

  // init system
  server.on("/init", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    Serial.println("init start.........................");
    initFileSystem();
    Serial.println("init success.........................");
    SPIFFS.end();
    request->redirect("/");
  });

  // wifi page
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>WiFi Setup Page</title></head><body><div><h1 id=\"title\">联网设置</h1><table><tr><th style=\"text-align:left;\">WIFI名称</th></tr><tr><td><input type=\"text\"id=\"txtssid\"/></td></tr><tr><th style=\"text-align:left;\">WIFI密码</th></tr><tr><td><input type=\"text\"id=\"txtpwd\"/></td></tr><tr><td><input type=\"submit\"value=\"连接\"id=\"btnConnect\"onclick=\"submit()\"/><input type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/></td></tr><tr><td><span id=\"spResult\"></span></td></tr></table><hr/><h1 id=\"title\">WIFI热点扫描</h1><table id=\"wifilist\"></table></div><script>function submit(){var ssid=document.getElementById('txtssid').value;var pwd=document.getElementById('txtpwd').value;alert(\"正在连接热点,请关闭此页面并切换选择的热点，通过http://esp32访问\");var url=\"savewifi?ssid=\"+ssid+\"&pwd=\"+pwd;window.location.href=url}function back(){var url=\"/\";window.location.href=url}</script></body></html>";
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    if (SPIFFS.exists("/wifi.ini"))
    {
      String filestr;
      filestr = getFileString(SPIFFS, "/wifi.ini");
      cJSON *root = NULL;
      cJSON *item = NULL;
      const char *jsonstr = filestr.c_str();
      root = cJSON_Parse(jsonstr);
      String itemstr;
      item = cJSON_GetObjectItem(root, "ssid");
      itemstr = cJSON_Print(item);
      String ssid = itemstr;
      item = cJSON_GetObjectItem(root, "pwd");
      itemstr = cJSON_Print(item);
      String pwd = itemstr;

      String tpl_ssid = "<input type=\"text\"id=\"txtssid\"/>";
      String tpl_pwd = "<input type=\"text\"id=\"txtpwd\"/>";
      String change_ssid = "<input type=\"text\"id=\"txtssid\" value=";
      change_ssid = change_ssid + ssid;
      change_ssid = change_ssid + "/>";
      String change_pwd = "<input type=\"text\"id=\"txtpwd\" value=";
      change_pwd = change_pwd + pwd;
      change_pwd = change_pwd + "/>";
      rawhtml.replace(tpl_ssid, change_ssid);
      rawhtml.replace(tpl_pwd, change_pwd);
    }
    SPIFFS.end();

    String tpl_scanresult = "<table id=\"wifilist\"></table>";
    String change_scanresult = "";
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
      change_scanresult = "<table id=\"wifilist\"><tr><td>wifi not found</td></tr></table>";
      rawhtml.replace(tpl_scanresult, change_scanresult);
    }
    else
    {
      String change_scanresult = "<table id=\"wifilist\">";
      for (int i = 0; i < n; ++i)
      {
        change_scanresult = change_scanresult + "<tr><td>";
        change_scanresult = change_scanresult + WiFi.SSID(i);
        change_scanresult = change_scanresult + "</td></tr>";
      }
      change_scanresult = change_scanresult + "</table>";
      rawhtml.replace(tpl_scanresult, change_scanresult);
    }

    request->send(200, "text/html", rawhtml);
  });

  // wifi setting
  server.on("/savewifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      String ssid = request->getParam("ssid")->value();
      String pwd = request->getParam("pwd")->value();
      String filecontent;
      filecontent = filecontent + "{\"ssid\":\"";
      filecontent = filecontent + ssid;
      filecontent = filecontent + "\",\"pwd\":\"";
      filecontent = filecontent + pwd;
      filecontent = filecontent + "\"}";
      writeFile(SPIFFS, "/wifi.ini", filecontent.c_str());
      SPIFFS.end();
      Serial.println("Begin to reboot!");
      ESP.restart();
    }
    else
    {
      Serial.println("param error");
    }
    request->redirect("/");
  });

  // reset operation
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Start to reset....");
    ESP.restart();
  });

  // bin upload page
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Update System</title><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script></head><body><div><h1 id=\"title\">更新固件</h1><hr/><form method='POST'action='#'enctype='multipart/form-data'id='upload_form'><input type='file'name='update'/><input type='submit'value='更新'/></form><div id='prg'>progress:0%</div></div><script>$('form').submit(function(e){e.preventDefault();var form=$('#upload_form')[0];var data=new FormData(form);$.ajax({url:'/update',type:'POST',data:data,contentType:false,processData:false,xhr:function(){var xhr=new window.XMLHttpRequest();xhr.upload.addEventListener('progress',function(evt){if(evt.lengthComputable){var per=evt.loaded/evt.total;$('#prg').html('progress: '+Math.round(per*100)+'%')}},false);return xhr},success:function(d,s){console.log('success!')},error:function(a,b,c){}})});</script></body></html>";
    request->send(200, "text/html", html);
  });

  // update operation
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    String pwminfocontent = "{\"showtype\":\"fix\",\"testmode\":\"test\",\"sysdate\":\"{unixtick}\",\"status\":\"stop\",\"conmode\": \"local\",\"version\":\"{version_num}\"}";
    pwminfocontent.replace("{version_num}", VERSION_NUM);
    pwminfocontent.replace("{unixtick}", String(ESP_RTC_TICK));
    writeFile(SPIFFS, "/pwminfo.ini", pwminfocontent.c_str());
    SPIFFS.end();
    ESP.restart();
  },
            onFileUpload);

  server.onNotFound(notFound);
  server.begin();
}

// esp32 loop operation
void loop()
{
  client.loop();
  if (RESET_FLAG)
  {
    Serial.println("Start to initialize the system configuration.");
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    initFileSystem();
    SPIFFS.end();
    RESET_FLAG = false;
    ESP.restart();
  }

  time_t t = time(NULL);
  struct tm *t_st;
  t_st = localtime(&t);
  int currenthour = t_st->tm_hour;
  int currentmin = t_st->tm_min;
  int currentsec = t_st->tm_sec;

  if (currentmin == 0 || currentmin == 5 || currentmin == 10 || currentmin == 15 || currentmin == 20 || currentmin == 25 || currentmin == 30 || currentmin == 35 || currentmin == 40 || currentmin == 45 || currentmin == 50 || currentmin == 55)
  {
    if (SSID != "" && WiFi.status() != WL_CONNECTED)
    {
      led8.set(0);
      Serial.printf(" %s try to reconnecting ..\n", ESP_HOST_NAME);
      WiFi.begin(SSID.c_str(), SSID_PWD.c_str());
      int i = 30;
      while ((WiFi.status() != WL_CONNECTED) && i > 0)
      {
        led8.set(100);
        delay(500);
        led8.set(0);
        Serial.printf(".");
        i = i - 1;
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println(WiFi.localIP());
        led8.set(100);
        client.setServer(mqttServer, mqttPort);
        client.setCallback(callback);
        mqttconn();
      }
    }
  }

  if (PWM_INFO_CONMODE == "online")
  {
    Serial.println("connection to the cloud....");
    led1.set(0);
    led2.set(0);
    led3.set(0);
    led4.set(0);
    led5.set(0);
    led6.set(0);
    led7.set(0);
  }
  else if (PWM_INFO_CONMODE == "local")
  {
    if (PWM_INFO_SHOWTYPE == "repeat")
    {

      if (PWM_INFO_TESTMODE == "production")
      {
        log_printf("product mode .... current hour is: %d \n", currenthour);
        if (currenthour == 0)
        {
          led1.set(P1_0.toInt());
          led2.set(P2_0.toInt());
          led3.set(P3_0.toInt());
          led4.set(P4_0.toInt());
          led5.set(P5_0.toInt());
          led6.set(P6_0.toInt());
          led7.set(P7_0.toInt());
        }
        else if (currenthour == 1)
        {
          led1.set(P1_1.toInt());
          led2.set(P2_1.toInt());
          led3.set(P3_1.toInt());
          led4.set(P4_1.toInt());
          led5.set(P5_1.toInt());
          led6.set(P6_1.toInt());
          led7.set(P7_1.toInt());
        }
        else if (currenthour == 2)
        {
          led1.set(P1_2.toInt());
          led2.set(P2_2.toInt());
          led3.set(P3_2.toInt());
          led4.set(P4_2.toInt());
          led5.set(P5_2.toInt());
          led6.set(P6_2.toInt());
          led7.set(P7_2.toInt());
        }
        else if (currenthour == 3)
        {
          led1.set(P1_3.toInt());
          led2.set(P2_3.toInt());
          led3.set(P3_3.toInt());
          led4.set(P4_3.toInt());
          led5.set(P5_3.toInt());
          led6.set(P6_3.toInt());
          led7.set(P7_3.toInt());
        }
        else if (currenthour == 4)
        {
          led1.set(P1_4.toInt());
          led2.set(P2_4.toInt());
          led3.set(P3_4.toInt());
          led4.set(P4_4.toInt());
          led5.set(P5_4.toInt());
          led6.set(P6_4.toInt());
          led7.set(P7_4.toInt());
        }
        else if (currenthour == 5)
        {
          led1.set(P1_5.toInt());
          led2.set(P2_5.toInt());
          led3.set(P3_5.toInt());
          led4.set(P4_5.toInt());
          led5.set(P5_5.toInt());
          led6.set(P6_5.toInt());
          led7.set(P7_5.toInt());
        }
        else if (currenthour == 6)
        {
          led1.set(P1_6.toInt());
          led2.set(P2_6.toInt());
          led3.set(P3_6.toInt());
          led4.set(P4_6.toInt());
          led5.set(P5_6.toInt());
          led6.set(P6_6.toInt());
          led7.set(P7_6.toInt());
        }
        else if (currenthour == 7)
        {
          led1.set(P1_7.toInt());
          led2.set(P2_7.toInt());
          led3.set(P3_7.toInt());
          led4.set(P4_7.toInt());
          led5.set(P5_7.toInt());
          led6.set(P6_7.toInt());
          led7.set(P7_7.toInt());
        }
        else if (currenthour == 8)
        {
          led1.set(P1_8.toInt());
          led2.set(P2_8.toInt());
          led3.set(P3_8.toInt());
          led4.set(P4_8.toInt());
          led5.set(P5_8.toInt());
          led6.set(P6_8.toInt());
          led7.set(P7_8.toInt());
        }
        else if (currenthour == 9)
        {
          led1.set(P1_9.toInt());
          led2.set(P2_9.toInt());
          led3.set(P3_9.toInt());
          led4.set(P4_9.toInt());
          led5.set(P5_9.toInt());
          led6.set(P6_9.toInt());
          led7.set(P7_9.toInt());
        }
        else if (currenthour == 10)
        {
          led1.set(P1_10.toInt());
          led2.set(P2_10.toInt());
          led3.set(P3_10.toInt());
          led4.set(P4_10.toInt());
          led5.set(P5_10.toInt());
          led6.set(P6_10.toInt());
          led7.set(P7_10.toInt());
        }
        else if (currenthour == 11)
        {
          led1.set(P1_11.toInt());
          led2.set(P2_11.toInt());
          led3.set(P3_11.toInt());
          led4.set(P4_11.toInt());
          led5.set(P5_11.toInt());
          led6.set(P6_11.toInt());
          led7.set(P7_11.toInt());
        }
        else if (currenthour == 12)
        {
          led1.set(P1_12.toInt());
          led2.set(P2_12.toInt());
          led3.set(P3_12.toInt());
          led4.set(P4_12.toInt());
          led5.set(P5_12.toInt());
          led6.set(P6_12.toInt());
          led7.set(P7_12.toInt());
        }
        else if (currenthour == 13)
        {
          led1.set(P1_13.toInt());
          led2.set(P2_13.toInt());
          led3.set(P3_13.toInt());
          led4.set(P4_13.toInt());
          led5.set(P5_13.toInt());
          led6.set(P6_13.toInt());
          led7.set(P7_13.toInt());
        }
        else if (currenthour == 14)
        {
          led1.set(P1_14.toInt());
          led2.set(P2_14.toInt());
          led3.set(P3_14.toInt());
          led4.set(P4_14.toInt());
          led5.set(P5_14.toInt());
          led6.set(P6_14.toInt());
          led7.set(P7_14.toInt());
        }
        else if (currenthour == 15)
        {
          led1.set(P1_15.toInt());
          led2.set(P2_15.toInt());
          led3.set(P3_15.toInt());
          led4.set(P4_15.toInt());
          led5.set(P5_15.toInt());
          led6.set(P6_15.toInt());
          led7.set(P7_15.toInt());
        }
        else if (currenthour == 16)
        {
          led1.set(P1_16.toInt());
          led2.set(P2_16.toInt());
          led3.set(P3_16.toInt());
          led4.set(P4_16.toInt());
          led5.set(P5_16.toInt());
          led6.set(P6_16.toInt());
          led7.set(P7_16.toInt());
        }
        else if (currenthour == 17)
        {
          led1.set(P1_17.toInt());
          led2.set(P2_17.toInt());
          led3.set(P3_17.toInt());
          led4.set(P4_17.toInt());
          led5.set(P5_17.toInt());
          led6.set(P6_17.toInt());
          led7.set(P7_17.toInt());
        }
        else if (currenthour == 18)
        {
          led1.set(P1_18.toInt());
          led2.set(P2_18.toInt());
          led3.set(P3_18.toInt());
          led4.set(P4_18.toInt());
          led5.set(P5_18.toInt());
          led6.set(P6_18.toInt());
          led7.set(P7_18.toInt());
        }
        else if (currenthour == 19)
        {
          led1.set(P1_19.toInt());
          led2.set(P2_19.toInt());
          led3.set(P3_19.toInt());
          led4.set(P4_19.toInt());
          led5.set(P5_19.toInt());
          led6.set(P6_19.toInt());
          led7.set(P7_19.toInt());
        }
        else if (currenthour == 20)
        {
          led1.set(P1_20.toInt());
          led2.set(P2_20.toInt());
          led3.set(P3_20.toInt());
          led4.set(P4_20.toInt());
          led5.set(P5_20.toInt());
          led6.set(P6_20.toInt());
          led7.set(P7_20.toInt());
        }
        else if (currenthour == 21)
        {
          led1.set(P1_21.toInt());
          led2.set(P2_21.toInt());
          led3.set(P3_21.toInt());
          led4.set(P4_21.toInt());
          led5.set(P5_21.toInt());
          led6.set(P6_21.toInt());
          led7.set(P7_21.toInt());
        }
        else if (currenthour == 22)
        {
          led1.set(P1_22.toInt());
          led2.set(P2_22.toInt());
          led3.set(P3_22.toInt());
          led4.set(P4_22.toInt());
          led5.set(P5_22.toInt());
          led6.set(P6_22.toInt());
          led7.set(P7_22.toInt());
        }
        else if (currenthour == 23)
        {
          led1.set(P1_23.toInt());
          led2.set(P2_23.toInt());
          led3.set(P3_23.toInt());
          led4.set(P4_23.toInt());
          led5.set(P5_23.toInt());
          led6.set(P6_23.toInt());
          led7.set(P7_23.toInt());
        }
      }
      else if (PWM_INFO_TESTMODE == "test")
      {

        log_printf("test mode .... current sec is: %d \n", currentsec);

        if (currentsec == 0 || currentsec == 30)
        {
          led1.set(P1_0.toInt());
          led2.set(P2_0.toInt());
          led3.set(P3_0.toInt());
          led4.set(P4_0.toInt());
          led5.set(P5_0.toInt());
          led6.set(P6_0.toInt());
          led7.set(P7_0.toInt());
        }
        else if (currentsec == 1 || currentsec == 31)
        {
          led1.set(P1_1.toInt());
          led2.set(P2_1.toInt());
          led3.set(P3_1.toInt());
          led4.set(P4_1.toInt());
          led5.set(P5_1.toInt());
          led6.set(P6_1.toInt());
          led7.set(P7_1.toInt());
        }
        else if (currentsec == 2 || currentsec == 32)
        {
          led1.set(P1_2.toInt());
          led2.set(P2_2.toInt());
          led3.set(P3_2.toInt());
          led4.set(P4_2.toInt());
          led5.set(P5_2.toInt());
          led6.set(P6_2.toInt());
          led7.set(P7_2.toInt());
        }
        else if (currentsec == 3 || currentsec == 33)
        {
          led1.set(P1_3.toInt());
          led2.set(P2_3.toInt());
          led3.set(P3_3.toInt());
          led4.set(P4_3.toInt());
          led5.set(P5_3.toInt());
          led6.set(P6_3.toInt());
          led7.set(P7_3.toInt());
        }
        else if (currentsec == 4 || currentsec == 34)
        {
          led1.set(P1_4.toInt());
          led2.set(P2_4.toInt());
          led3.set(P3_4.toInt());
          led4.set(P4_4.toInt());
          led5.set(P5_4.toInt());
          led6.set(P6_4.toInt());
          led7.set(P7_4.toInt());
        }
        else if (currentsec == 5 || currentsec == 35)
        {
          led1.set(P1_5.toInt());
          led2.set(P2_5.toInt());
          led3.set(P3_5.toInt());
          led4.set(P4_5.toInt());
          led5.set(P5_5.toInt());
          led6.set(P6_5.toInt());
          led7.set(P7_5.toInt());
        }
        else if (currentsec == 6 || currentsec == 36)
        {
          led1.set(P1_6.toInt());
          led2.set(P2_6.toInt());
          led3.set(P3_6.toInt());
          led4.set(P4_6.toInt());
          led5.set(P5_6.toInt());
          led6.set(P6_6.toInt());
          led7.set(P7_6.toInt());
        }
        else if (currentsec == 7 || currentsec == 37)
        {
          led1.set(P1_7.toInt());
          led2.set(P2_7.toInt());
          led3.set(P3_7.toInt());
          led4.set(P4_7.toInt());
          led5.set(P5_7.toInt());
          led6.set(P6_7.toInt());
          led7.set(P7_7.toInt());
        }
        else if (currentsec == 8 || currentsec == 38)
        {
          led1.set(P1_8.toInt());
          led2.set(P2_8.toInt());
          led3.set(P3_8.toInt());
          led4.set(P4_8.toInt());
          led5.set(P5_8.toInt());
          led6.set(P6_8.toInt());
          led7.set(P7_8.toInt());
        }
        else if (currentsec == 9 || currentsec == 39)
        {
          led1.set(P1_9.toInt());
          led2.set(P2_9.toInt());
          led3.set(P3_9.toInt());
          led4.set(P4_9.toInt());
          led5.set(P5_9.toInt());
          led6.set(P6_9.toInt());
          led7.set(P7_9.toInt());
        }
        else if (currentsec == 10 || currentsec == 40)
        {
          led1.set(P1_10.toInt());
          led2.set(P2_10.toInt());
          led3.set(P3_10.toInt());
          led4.set(P4_10.toInt());
          led5.set(P5_10.toInt());
          led6.set(P6_10.toInt());
          led7.set(P7_10.toInt());
        }
        else if (currentsec == 11 || currentsec == 41)
        {
          led1.set(P1_11.toInt());
          led2.set(P2_11.toInt());
          led3.set(P3_11.toInt());
          led4.set(P4_11.toInt());
          led5.set(P5_11.toInt());
          led6.set(P6_11.toInt());
          led7.set(P7_11.toInt());
        }
        else if (currentsec == 12 || currentsec == 42)
        {
          led1.set(P1_12.toInt());
          led2.set(P2_12.toInt());
          led3.set(P3_12.toInt());
          led4.set(P4_12.toInt());
          led5.set(P5_12.toInt());
          led6.set(P6_12.toInt());
          led7.set(P7_12.toInt());
        }
        else if (currentsec == 13 || currentsec == 43)
        {
          led1.set(P1_13.toInt());
          led2.set(P2_13.toInt());
          led3.set(P3_13.toInt());
          led4.set(P4_13.toInt());
          led5.set(P5_13.toInt());
          led6.set(P6_13.toInt());
          led7.set(P7_13.toInt());
        }
        else if (currentsec == 14 || currentsec == 44)
        {
          led1.set(P1_14.toInt());
          led2.set(P2_14.toInt());
          led3.set(P3_14.toInt());
          led4.set(P4_14.toInt());
          led5.set(P5_14.toInt());
          led6.set(P6_14.toInt());
          led7.set(P7_14.toInt());
        }
        else if (currentsec == 15 || currentsec == 45)
        {
          led1.set(P1_15.toInt());
          led2.set(P2_15.toInt());
          led3.set(P3_15.toInt());
          led4.set(P4_15.toInt());
          led5.set(P5_15.toInt());
          led6.set(P6_15.toInt());
          led7.set(P7_15.toInt());
        }
        else if (currentsec == 16 || currentsec == 46)
        {
          led1.set(P1_16.toInt());
          led2.set(P2_16.toInt());
          led3.set(P3_16.toInt());
          led4.set(P4_16.toInt());
          led5.set(P5_16.toInt());
          led6.set(P6_16.toInt());
          led7.set(P7_16.toInt());
        }
        else if (currentsec == 17 || currentsec == 47)
        {
          led1.set(P1_17.toInt());
          led2.set(P2_17.toInt());
          led3.set(P3_17.toInt());
          led4.set(P4_17.toInt());
          led5.set(P5_17.toInt());
          led6.set(P6_17.toInt());
          led7.set(P7_17.toInt());
        }
        else if (currentsec == 18 || currentsec == 48)
        {
          led1.set(P1_18.toInt());
          led2.set(P2_18.toInt());
          led3.set(P3_18.toInt());
          led4.set(P4_18.toInt());
          led5.set(P5_18.toInt());
          led6.set(P6_18.toInt());
          led7.set(P7_18.toInt());
        }
        else if (currentsec == 19 || currentsec == 49)
        {
          led1.set(P1_19.toInt());
          led2.set(P2_19.toInt());
          led3.set(P3_19.toInt());
          led4.set(P4_19.toInt());
          led5.set(P5_19.toInt());
          led6.set(P6_19.toInt());
          led7.set(P7_19.toInt());
        }
        else if (currentsec == 20 || currentsec == 50)
        {
          led1.set(P1_20.toInt());
          led2.set(P2_20.toInt());
          led3.set(P3_20.toInt());
          led4.set(P4_20.toInt());
          led5.set(P5_20.toInt());
          led6.set(P6_20.toInt());
          led7.set(P7_20.toInt());
        }
        else if (currentsec == 21 || currentsec == 51)
        {
          led1.set(P1_21.toInt());
          led2.set(P2_21.toInt());
          led3.set(P3_21.toInt());
          led4.set(P4_21.toInt());
          led5.set(P5_21.toInt());
          led6.set(P6_21.toInt());
          led7.set(P7_21.toInt());
        }
        else if (currentsec == 22 || currentsec == 52)
        {
          led1.set(P1_22.toInt());
          led2.set(P2_22.toInt());
          led3.set(P3_22.toInt());
          led4.set(P4_22.toInt());
          led5.set(P5_22.toInt());
          led6.set(P6_22.toInt());
          led7.set(P7_22.toInt());
        }
        else if (currentsec == 23 || currentsec == 53)
        {
          led1.set(P1_23.toInt());
          led2.set(P2_23.toInt());
          led3.set(P3_23.toInt());
          led4.set(P4_23.toInt());
          led5.set(P5_23.toInt());
          led6.set(P6_23.toInt());
          led7.set(P7_23.toInt());
        }
      }

      if (IS_SMART)
      {

        if ((WiFi.status() == WL_CONNECTED || WiFi.smartConfigDone()) && !client.connected())
        {
          led8.set(5);
          Serial.println("the mqtt service is disconnected");
          mqttconn();
          client.publish("esp32/disnotify", ESP_HOST_NAME);
        }
      }
      if (client.connected())
      {
        client.publish("esp32/heart", ESP_HOST_NAME);
      }
    }
    else if (PWM_INFO_SHOWTYPE == "fix")
    {
      //Serial.println(" this is the fix mode");
      led1.set(P1_24.toInt());
      led2.set(P2_24.toInt());
      led3.set(P3_24.toInt());
      led4.set(P4_24.toInt());
      led5.set(P5_24.toInt());
      led6.set(P6_24.toInt());
      led7.set(P7_24.toInt());
      //String topiccontent = ESP_HOST_NAME;
      //Serial.printf("the connect status is: %d, the smart status is: %d, the mqtt is: %d, the ssid is: %s \n", WiFi.status(), WiFi.smartConfigDone(), client.connected(), SSID.c_str());
      if (IS_SMART)
      {

        if ((WiFi.status() == WL_CONNECTED || WiFi.smartConfigDone()) && !client.connected())
        {
          led8.set(5);
          Serial.println("the mqtt service is disconnected");
          mqttconn();
          client.publish("esp32/disnotify", ESP_HOST_NAME);
        }
      }
      if (client.connected())
      {
        client.publish("esp32/heart", ESP_HOST_NAME);
      }
    }
  }
  delay(1000);
}