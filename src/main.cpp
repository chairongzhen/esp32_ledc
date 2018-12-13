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
#define VERSION_NUM "0.20"
#define ESP_HOST_NAME "esp1005"
#define ESP_RTC_TICK 1542012457

String PWM_INFO_SHOWTYPE, PWM_INFO_TESTMODE, PWM_INFO_CONMODE, PWM_INFO_RTC, PWM_INFO_VERSION;
String P1_0, P1_1, P1_2, P1_3, P1_4, P1_5, P1_6, P1_7, P1_8, P1_9, P1_10, P1_11, P1_12, P1_13, P1_14, P1_15, P1_16, P1_17, P1_18, P1_19, P1_20, P1_21, P1_22, P1_23, P1_24;
String P2_0, P2_1, P2_2, P2_3, P2_4, P2_5, P2_6, P2_7, P2_8, P2_9, P2_10, P2_11, P2_12, P2_13, P2_14, P2_15, P2_16, P2_17, P2_18, P2_19, P2_20, P2_21, P2_22, P2_23, P2_24;
String P3_0, P3_1, P3_2, P3_3, P3_4, P3_5, P3_6, P3_7, P3_8, P3_9, P3_10, P3_11, P3_12, P3_13, P3_14, P3_15, P3_16, P3_17, P3_18, P3_19, P3_20, P3_21, P3_22, P3_23, P3_24;
String P4_0, P4_1, P4_2, P4_3, P4_4, P4_5, P4_6, P4_7, P4_8, P4_9, P4_10, P4_11, P4_12, P4_13, P4_14, P4_15, P4_16, P4_17, P4_18, P4_19, P4_20, P4_21, P4_22, P4_23, P4_24;
String P5_0, P5_1, P5_2, P5_3, P5_4, P5_5, P5_6, P5_7, P5_8, P5_9, P5_10, P5_11, P5_12, P5_13, P5_14, P5_15, P5_16, P5_17, P5_18, P5_19, P5_20, P5_21, P5_22, P5_23, P5_24;
String P6_0, P6_1, P6_2, P6_3, P6_4, P6_5, P6_6, P6_7, P6_8, P6_9, P6_10, P6_11, P6_12, P6_13, P6_14, P6_15, P6_16, P6_17, P6_18, P6_19, P6_20, P6_21, P6_22, P6_23, P6_24;
String P7_0, P7_1, P7_2, P7_3, P7_4, P7_5, P7_6, P7_7, P7_8, P7_9, P7_10, P7_11, P7_12, P7_13, P7_14, P7_15, P7_16, P7_17, P7_18, P7_19, P7_20, P7_21, P7_22, P7_23, P7_24;

int P1[145] = {};
int P2[145] = {};
int P3[145] = {};
int P4[145] = {};
int P5[145] = {};
int P6[145] = {};
int P7[145] = {};

int TESTMODE_COUNT = 0;

String SSID, SSID_PWD;
//char LIGHTDEF[] = "000,001,002,003,004,005,010,011,012,013,014,015,020,021,022,023,024,025,030,031,032,033,034,035,040,041,042,043,044,045,050,051,052,053,054,055,060,061,062,063,064,065,070,071,072,073,074,075,080,081,082,083,084,085,090,091,092,093,094,095,100,101,102,103,104,105,110,111,112,113,114,115,120,121,122,123,124,125,130,131,132,133,134,135,140,141,142,143,144,145,150,151,152,153,154,155,160,161,162,163,164,165,170,171,172,173,174,175,180,181,182,183,184,185,190,191,192,193,194,195,200,201,202,203,204,205,210,211,212,213,214,215,220,221,222,223,224,225,230,231,232,233,234,235,fix";

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
LED_ESP32 led5(21, 4, 100);
LED_ESP32 led6(22, 5, 100);
LED_ESP32 led7(27, 6, 100);
LED_ESP32 led8(25, 7, 100);

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

  writeFile(SPIFFS, "/p.ini", "{\"t000\":\"00000000000000\",\"t001\":\"00000000000000\",\"t002\":\"00000000000000\",\"t003\":\"00000000000000\",\"t004\":\"00000000000000\",\"t005\":\"00000000000000\",\"t010\":\"00000000000000\",\"t011\":\"00000000000000\",\"t012\":\"00000000000000\",\"t013\":\"00000000000000\",\"t014\":\"00000000000000\",\"t015\":\"00000000000000\",\"t020\":\"00000000000000\",\"t021\":\"00000000000000\",\"t022\":\"00000000000000\",\"t023\":\"00000000000000\",\"t024\":\"00000000000000\",\"t025\":\"00000000000000\",\"t030\":\"00000000000000\",\"t031\":\"00000000000000\",\"t032\":\"00000000000000\",\"t033\":\"00000000000000\",\"t034\":\"00000000000000\",\"t035\":\"00000000000000\",\"t040\":\"00000000000000\",\"t041\":\"00000000000000\",\"t042\":\"00000000000000\",\"t043\":\"00000000000000\",\"t044\":\"00000000000000\",\"t045\":\"00000000000000\",\"t050\":\"00000000000000\",\"t051\":\"00000000000000\",\"t052\":\"00000000000000\",\"t053\":\"00000000000000\",\"t054\":\"00000000000000\",\"t055\":\"00000000000000\",\"t060\":\"00000000000000\",\"t061\":\"00000000000000\",\"t062\":\"00000000000000\",\"t063\":\"00000000000000\",\"t064\":\"00000000000000\",\"t065\":\"00000000000000\",\"t070\":\"00000000000000\",\"t071\":\"00000000000000\",\"t072\":\"00000000000000\",\"t073\":\"00000000000000\",\"t074\":\"00000000000000\",\"t075\":\"00000000000000\",\"t080\":\"00000000000000\",\"t081\":\"00000000000000\",\"t082\":\"00000000000000\",\"t083\":\"00000000000000\",\"t084\":\"00000000000000\",\"t085\":\"00000000000000\",\"t090\":\"00000000000000\",\"t091\":\"00000000000000\",\"t092\":\"00000000000000\",\"t093\":\"00000000000000\",\"t094\":\"00000000000000\",\"t095\":\"00000000000000\",\"t100\":\"00000000000000\",\"t101\":\"00000000000000\",\"t102\":\"00000000000000\",\"t103\":\"00000000000000\",\"t104\":\"00000000000000\",\"t105\":\"00000000000000\",\"t110\":\"00000000000000\",\"t111\":\"00000000000000\",\"t112\":\"00000000000000\",\"t113\":\"00000000000000\",\"t114\":\"00000000000000\",\"t115\":\"00000000000000\",\"t120\":\"00000000000000\",\"t121\":\"00000000000000\",\"t122\":\"00000000000000\",\"t123\":\"00000000000000\",\"t124\":\"00000000000000\",\"t125\":\"00000000000000\",\"t130\":\"00000000000000\",\"t131\":\"00000000000000\",\"t132\":\"00000000000000\",\"t133\":\"00000000000000\",\"t134\":\"00000000000000\",\"t135\":\"00000000000000\",\"t140\":\"00000000000000\",\"t141\":\"00000000000000\",\"t142\":\"00000000000000\",\"t143\":\"00000000000000\",\"t144\":\"00000000000000\",\"t145\":\"00000000000000\",\"t150\":\"00000000000000\",\"t151\":\"00000000000000\",\"t152\":\"00000000000000\",\"t153\":\"00000000000000\",\"t154\":\"00000000000000\",\"t155\":\"00000000000000\",\"t160\":\"00000000000000\",\"t161\":\"00000000000000\",\"t162\":\"00000000000000\",\"t163\":\"00000000000000\",\"t164\":\"00000000000000\",\"t165\":\"00000000000000\",\"t170\":\"00000000000000\",\"t171\":\"00000000000000\",\"t172\":\"00000000000000\",\"t173\":\"00000000000000\",\"t174\":\"00000000000000\",\"t175\":\"00000000000000\",\"t180\":\"00000000000000\",\"t181\":\"00000000000000\",\"t182\":\"00000000000000\",\"t183\":\"00000000000000\",\"t184\":\"00000000000000\",\"t185\":\"00000000000000\",\"t190\":\"00000000000000\",\"t191\":\"00000000000000\",\"t192\":\"00000000000000\",\"t193\":\"00000000000000\",\"t194\":\"00000000000000\",\"t195\":\"00000000000000\",\"t200\":\"00000000000000\",\"t201\":\"00000000000000\",\"t202\":\"00000000000000\",\"t203\":\"00000000000000\",\"t204\":\"00000000000000\",\"t205\":\"00000000000000\",\"t210\":\"00000000000000\",\"t211\":\"00000000000000\",\"t212\":\"00000000000000\",\"t213\":\"00000000000000\",\"t214\":\"00000000000000\",\"t215\":\"00000000000000\",\"t220\":\"00000000000000\",\"t221\":\"00000000000000\",\"t222\":\"00000000000000\",\"t223\":\"00000000000000\",\"t224\":\"00000000000000\",\"t225\":\"00000000000000\",\"t230\":\"00000000000000\",\"t231\":\"00000000000000\",\"t232\":\"00000000000000\",\"t233\":\"00000000000000\",\"t234\":\"00000000000000\",\"t235\":\"00000000000000\",\"tfix\":\"00000000000000\"}");
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
  else if (String(topic) == topic_name_pt)
  {
    for (int i = 0; i < cnt; i++)
    {
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

// void printJson(cJSON *root)
// {
//   for (int i = 0; i < cJSON_GetArraySize(root); i++)
//   {
//     cJSON *item = cJSON_GetArrayItem(root, i);
//     if (cJSON_Object == item->type)
//       printJson(item);
//     else
//     {
//       Serial.printf("%s->", item->string);
//       printf("%s\n", cJSON_Print(item));
//     }
//   }
// }

// light operation
void lightopr(String content)
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


  String sval;
  root = cJSON_Parse(content.c_str());
  for (int i = 0; i < cJSON_GetArraySize(root); i++)
  {
    String content = cJSON_GetArrayItem(root, i)->valuestring;

    int intval;
    sval = content.substring(0, 2);
    sscanf(sval.c_str(), "%x", &intval);
    P1[i] = intval;
    sval = content.substring(2, 4);
    sscanf(sval.c_str(), "%x", &intval);
    P2[i] = intval;
    sval = content.substring(4, 6);
    sscanf(sval.c_str(), "%x", &intval);
    P3[i] = intval;
    sval = content.substring(6, 8);
    sscanf(sval.c_str(), "%x", &intval);
    P4[i] = intval;
    sval = content.substring(8, 10);
    sscanf(sval.c_str(), "%x", &intval);
    P5[i] = intval;
    sval = content.substring(10, 12);
    sscanf(sval.c_str(), "%x", &intval);
    P6[i] = intval;
    sval = content.substring(12, 14);
    sscanf(sval.c_str(), "%x", &intval);
    P7[i] = intval;
  }

  cJSON_Delete(root);
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

        cJSON_Delete(root);

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
        int trytime = 5;
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
      String strlvs = getFileString(SPIFFS, "/p.ini");
      lightopr(strlvs);
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

    cJSON_Delete(root);

    //todo
    String version = VERSION_NUM;
    Serial.println("the version is: " + version);
    String html = "";
    html = html + "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>NodeMCU Control Page</title><!--<script type=\"text/javascript\"src=\"jquery.js\"></script>--></head><body><div><h1 id=\"title\">基本信息</h1><table><tr><th style=\"text-align:left;\">版本号</th></tr><tr><td><span id=\"spversion\"></span></td></tr><tr><th style=\"text-align:left;\">系统时间</th></tr><tr><td><span id=\"spCurrent\"></span></td></tr><tr><th style=\"text-align: left;\">修改时间</th></tr><tr><td><input type=\"text\"value=\"2018-03-03 00:00:00\"id=\"txtsysdate\"/>*时间格式:2018-03-03 00:00:00</td></tr><tr><th style=\"text-align:left;\">循环模式</th></tr><tr><td><input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>循环模式<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>固定模式</td></tr><tr id=\"thtest\"><th style=\"text-align:left;\">是否测试</th></tr><tr id=\"tdtest\"><td><input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>否<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>是</td></tr><tr id=\"thonline\"><th style=\"text-align:left;\">云端控制</th></tr><tr id=\"tdonline\"><td><input value=\"local\"type=\"radio\"name=\"onlineMode\"id=\"rdlocal\"/>否<input value=\"online\"type=\"radio\"name=\"onlineMode\"id=\"rdonline\"/>是</td></tr><tr><td><input type=\"button\"value=\"联网设置\"id=\"btnwifi\"onclick=\"wifi();\"/><input type=\"submit\"value=\"保存\"id=\"submit\"onclick=\"submit();\"/><input type=\"button\"value=\"恢复出厂\"id=\"btnstop\"onclick=\"init();\"/><input type=\"button\"value=\"重启\"id=\"btnReset\"onclick=\"reset();\"/><input type=\"button\"value=\"更新固件\"id=\"btnupload\"onclick=\"upload();\"/></td></tr></table><hr/><h1 id=\"title\">灯光控制</h1><table><tr><td><a href=\"/p\">进入设置页面</a></td></tr></table></div><script>function submit(){var selectshowtype=document.getElementsByName('showtype');var showtypevalue=\"\";for(var i=0;i<selectshowtype.length;i++){if(selectshowtype[i].checked){showtypevalue=selectshowtype[i].value;break}}var selecttestmode=document.getElementsByName('testMode');var testmodevalue=\"\";for(var i=0;i<selecttestmode.length;i++){if(selecttestmode[i].checked){testmodevalue=selecttestmode[i].value;break}}var str=document.getElementById('txtsysdate').value;str=str.replace(/-/g,\"/\");var date=new Date(str);var unixDate=date.getTime()/1000|0;console.log(unixDate);var selectedconnectionmode=document.getElementsByName(\"onlineMode\");var connectionmodevalue=\"\";for(var i=0;i<selectedconnectionmode.length;i++){if(selectedconnectionmode[i].checked){connectionmodevalue=selectedconnectionmode[i].value;break}}alert('保存成功');var url=\"pwmopr?showtype=\"+showtypevalue+\"&testmode=\"+testmodevalue+\"&sysdate=\"+unixDate+\"&conmode=\"+connectionmodevalue;window.location.href=url}function init(){alert('已恢复出厂设置!');var url=\"init\";window.location.href=url}function wifi(){var url=\"wifi\";window.location.href=url}function reset(){var url=\"reset\";alert(\"已重启,请关闭当前页面\");window.location.href=url}function upload(){var url=\"upload\";window.location.href=url}</script></body></html>";

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
    if (true)
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      String filestr;
      filestr = getFileString(SPIFFS, "/p.ini");

      if (false)
      {
        Serial.println("Error occured");
      }
      else
      {
        String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>操作</title></head><body><h1><span id=\"sptitle\">亮度控制</span><span id=\"lightvalues\"style=\"display:none;\"></span></h1><Table><tr><td colspan=\"6\"><input type=\"radio\"name=\"line\"value=\"1\"checked onchange=\"linechange(1)\"/>第一排<input type=\"radio\"name=\"line\"value=\"2\"onchange=\"linechange(2)\"/>第二排<input type=\"radio\"name=\"line\"value=\"3\"onchange=\"linechange(3)\"/>第三排<input type=\"radio\"name=\"line\"value=\"4\"onchange=\"linechange(4)\"/>第四排<input type=\"radio\"name=\"line\"value=\"5\"onchange=\"linechange(5)\"/>第五排<input type=\"radio\"name=\"line\"value=\"6\"onchange=\"linechange(6)\"/>第六排<input type=\"radio\"name=\"line\"value=\"7\"onchange=\"linechange(7)\"/>第七排<input type=\"hidden\"id=\"hidline\"value=\"1\"/></td></tr><tr><th>0：00</th><th>0:10</th><th>0:20</th><th>0:30</th><th>0:40</th><th>0:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl000\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl001\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl002\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl003\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl004\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl005\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>1：00</th><th>1:10</th><th>1:20</th><th>1:30</th><th>1:40</th><th>1:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl010\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl011\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl012\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl013\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl014\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl015\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>2：00</th><th>2:10</th><th>2:20</th><th>2:30</th><th>2:40</th><th>2:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl020\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl021\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl022\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl023\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl024\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl025\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>3：00</th><th>3:10</th><th>3:20</th><th>3:30</th><th>3:40</th><th>3:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl030\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl031\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl032\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl033\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl034\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl035\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>4：00</th><th>4:10</th><th>4:20</th><th>4:30</th><th>4:40</th><th>4:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl040\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl041\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl042\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl043\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl044\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl045\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>5：00</th><th>5:10</th><th>5:20</th><th>5:30</th><th>5:40</th><th>5:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl050\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl051\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl052\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl053\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl054\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl055\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>6：00</th><th>6:10</th><th>6:20</th><th>6:30</th><th>6:40</th><th>6:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl060\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl061\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl062\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl063\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl064\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl065\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>7：00</th><th>7:10</th><th>7:20</th><th>7:30</th><th>7:40</th><th>7:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl070\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl071\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl072\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl073\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl074\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl075\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>8：00</th><th>8:10</th><th>8:20</th><th>8:30</th><th>8:40</th><th>8:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl080\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl081\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl082\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl083\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl084\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl085\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>9：00</th><th>9:10</th><th>9:20</th><th>9:30</th><th>9:40</th><th>9:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl090\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl091\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl092\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl093\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl094\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl095\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>10：00</th><th>10:10</th><th>10:20</th><th>10:30</th><th>10:40</th><th>10:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl100\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl101\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl102\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl103\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl104\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl105\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>11：00</th><th>11:10</th><th>11:20</th><th>11:30</th><th>11:40</th><th>11:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl110\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl111\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl112\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl113\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl114\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl115\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>12：00</th><th>12:10</th><th>12:20</th><th>12:30</th><th>12:40</th><th>12:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl120\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl121\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl122\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl123\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl124\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl125\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>13：00</th><th>13:10</th><th>13:20</th><th>13:30</th><th>13:40</th><th>13:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl130\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl131\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl132\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl133\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl134\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl135\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>14：00</th><th>14:10</th><th>14:20</th><th>14:30</th><th>14:40</th><th>14:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl140\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl141\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl142\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl143\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl144\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl145\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>15：00</th><th>15:10</th><th>15:20</th><th>15:30</th><th>15:40</th><th>15:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl150\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl151\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl152\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl153\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl154\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl155\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>16：00</th><th>16:10</th><th>16:20</th><th>16:30</th><th>16:40</th><th>16:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl160\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl161\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl162\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl163\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl164\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl165\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>17：00</th><th>17:10</th><th>17:20</th><th>17:30</th><th>17:40</th><th>17:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl170\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl171\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl172\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl173\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl174\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl175\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>18：00</th><th>18:10</th><th>18:20</th><th>18:30</th><th>18:40</th><th>18:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl180\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl181\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl182\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl183\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl184\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl185\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>19：00</th><th>19:10</th><th>19:20</th><th>19:30</th><th>19:40</th><th>19:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl190\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl191\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl192\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl193\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl194\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl195\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>20：00</th><th>20:10</th><th>20:20</th><th>20:30</th><th>20:40</th><th>20:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl200\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl201\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl202\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl203\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl204\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl205\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>21：00</th><th>21:10</th><th>21:20</th><th>21:30</th><th>21:40</th><th>21:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl210\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl211\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl212\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl213\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl214\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl215\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>22：00</th><th>22:10</th><th>22:20</th><th>22:30</th><th>22:40</th><th>22:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl220\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl221\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl222\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl223\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl224\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl225\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>23：00</th><th>23:10</th><th>23:20</th><th>23:30</th><th>23:40</th><th>23:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl230\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl231\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl232\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl233\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl234\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl235\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>固定</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtlfix\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td><input onclick=\"submit();\"type=\"submit\"id=\"submit\"value=\"保存\"/><input onclick=\"back();\"type=\"button\"id=\"btnback\"value=\"返回\"/></td></tr><tr><td sylte=\"color:red\">取值范围0~255</td></tr></Table></body><script language=\"javascript\">function submit(){var savedata=document.getElementById(\"lightvalues\").innerHTML;var url=\"setp?t=\"+savedata;console.log(url);alert('保存成功');window.location.href=url}function tohex(str){if(str==\"0\"){return\"00\"}else{var intval=parseInt(str);if(intval){var hexval=intval.toString(16);if(hexval.length==1){var res=\"0\";return res+hexval}else{return hexval}}else{return\"00\"}}}function back(){var url=\"/\";window.location.href=url}function setlinevalue(pnum,lvs){var startIndex;switch(pnum){case 1:startIndex=0;break;case 2:startIndex=2;break;case 3:startIndex=4;break;case 4:startIndex=6;break;case 5:startIndex=8;break;case 6:startIndex=10;break;case 7:startIndex=12;break;default:break}for(var key in lvs){var resval=lvs[key];resval=resval.substr(startIndex,2);var res=parseInt(resval,16);var cid=key.replace(\"t\",\"\");cid=\"txtl\"+cid;document.getElementById(cid).value=res}}function linechange(val){document.getElementById(\"hidline\").value=val;var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(val,lvs)}function checkinput(input){var res=parseInt(input.value);if(res){if(res<0||res>255){document.getElementById(input.id).value=\"0\";alert('输入有误')}}else{document.getElementById(input.id).value=\"0\";alert('输入有误')}}function changevalue(input){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);var cid=input.id;var contentarr=tohex(input.value).split('');var tid=cid.replace(\"txtl\",\"t\");var line=parseInt(document.getElementById(\"hidline\").value);var arrcontent=lvs[tid].split('');var startIndex,endIndex;switch(line){case 1:startIndex=0;endIndex=1;break;case 2:startIndex=2;endIndex=3;break;case 3:startIndex=4;endIndex=5;break;case 4:startIndex=6;endIndex=7;break;case 5:startIndex=8;endIndex=9;break;case 6:startIndex=10;endIndex=11;break;case 7:startIndex=12;endIndex=13;break;default:break}arrcontent[startIndex]=contentarr[0];arrcontent[endIndex]=contentarr[1];lvs[tid]=arrcontent.join('');var updstr=JSON.stringify(lvs);document.getElementById('lightvalues').innerHTML=updstr}window.onload=function(){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(1,lvs)}</script></html>";
        String tpl_data = "<input type=\"text\"width=\"50\"id=\"txtl{lid}\"value=\"0\"/>";
        String itemstr;
        String changeinput;
        String lightvalue;

        String tpl_hidval = "<span id=\"lightvalues\"style=\"display:none;\"></span>";
        String change_hidval = "<span id=\"lightvalues\"style=\"display:none;\">";
        change_hidval += filestr;
        change_hidval += "</span>";
        rawhtml.replace(tpl_hidval, change_hidval);

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
    if (request->hasParam("t"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      String tvalues = request->getParam("t")->value();
      lightopr(tvalues);
      writeFile(SPIFFS, "/p.ini", tvalues.c_str());
      SPIFFS.end();
      request->redirect("/p");
    }
    else
    {
      request->send(200, "text/html", "<p>param not found</p>");
    }
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
        Serial.printf("product mode .... current hour is: %d \n",currenthour);
        if (currenthour == 0)
        {
          if(currentmin == 0) {
            led1.set(P1[0]);
            led2.set(P2[0]);
            led3.set(P3[0]);
            led4.set(P4[0]);
            led5.set(P5[0]);
            led6.set(P6[0]);
            led7.set(P7[0]);
          }
          else if(currentmin == 10) {
            led1.set(P1[1]);
            led2.set(P2[1]);
            led3.set(P3[1]);
            led4.set(P4[1]);
            led5.set(P5[1]);
            led6.set(P6[1]);
            led7.set(P7[1]);
          } else if(currentmin == 20) {
            led1.set(P1[2]);
            led2.set(P2[2]);
            led3.set(P3[2]);
            led4.set(P4[2]);
            led5.set(P5[2]);
            led6.set(P6[2]);
            led7.set(P7[2]);
          } else if(currentmin == 30) {
            led1.set(P1[3]);
            led2.set(P2[3]);
            led3.set(P3[3]);
            led4.set(P4[3]);
            led5.set(P5[3]);
            led6.set(P6[3]);
            led7.set(P7[3]);
          } else if(currentmin == 40) {
            led1.set(P1[4]);
            led2.set(P2[4]);
            led3.set(P3[4]);
            led4.set(P4[4]);
            led5.set(P5[4]);
            led6.set(P6[4]);
            led7.set(P7[4]);
          } else if(currentmin == 50) {
            led1.set(P1[5]);
            led2.set(P2[5]);
            led3.set(P3[5]);
            led4.set(P4[5]);
            led5.set(P5[5]);
            led6.set(P6[5]);
            led7.set(P7[5]);
          }
        }
        else if (currenthour == 1)
        {
          if(currentmin == 0) {
            led1.set(P1[6]);
            led2.set(P2[6]);
            led3.set(P3[6]);
            led4.set(P4[6]);
            led5.set(P5[6]);
            led6.set(P6[6]);
            led7.set(P7[6]);
          }
          else if(currentmin == 10) {
            led1.set(P1[7]);
            led2.set(P2[7]);
            led3.set(P3[7]);
            led4.set(P4[7]);
            led5.set(P5[7]);
            led6.set(P6[7]);
            led7.set(P7[7]);
          } else if(currentmin == 20) {
            led1.set(P1[8]);
            led2.set(P2[8]);
            led3.set(P3[8]);
            led4.set(P4[8]);
            led5.set(P5[8]);
            led6.set(P6[8]);
            led7.set(P7[8]);
          } else if(currentmin == 30) {
            led1.set(P1[9]);
            led2.set(P2[9]);
            led3.set(P3[9]);
            led4.set(P4[9]);
            led5.set(P5[9]);
            led6.set(P6[9]);
            led7.set(P7[9]);
          } else if(currentmin == 40) {
            led1.set(P1[10]);
            led2.set(P2[10]);
            led3.set(P3[10]);
            led4.set(P4[10]);
            led5.set(P5[10]);
            led6.set(P6[10]);
            led7.set(P7[10]);
          } else if(currentmin == 50) {
            led1.set(P1[11]);
            led2.set(P2[11]);
            led3.set(P3[11]);
            led4.set(P4[11]);
            led5.set(P5[11]);
            led6.set(P6[11]);
            led7.set(P7[11]);
          }
        }
        else if (currenthour == 2)
        {
          if(currentmin == 0) {
            led1.set(P1[12]);
            led2.set(P2[12]);
            led3.set(P3[12]);
            led4.set(P4[12]);
            led5.set(P5[12]);
            led6.set(P6[12]);
            led7.set(P7[12]);
          }
          else if(currentmin == 10) {
            led1.set(P1[13]);
            led2.set(P2[13]);
            led3.set(P3[13]);
            led4.set(P4[13]);
            led5.set(P5[13]);
            led6.set(P6[13]);
            led7.set(P7[13]);
          } else if(currentmin == 20) {
            led1.set(P1[14]);
            led2.set(P2[14]);
            led3.set(P3[14]);
            led4.set(P4[14]);
            led5.set(P5[14]);
            led6.set(P6[14]);
            led7.set(P7[14]);
          } else if(currentmin == 30) {
            led1.set(P1[15]);
            led2.set(P2[15]);
            led3.set(P3[15]);
            led4.set(P4[15]);
            led5.set(P5[15]);
            led6.set(P6[15]);
            led7.set(P7[15]);
          } else if(currentmin == 40) {
            led1.set(P1[16]);
            led2.set(P2[16]);
            led3.set(P3[16]);
            led4.set(P4[16]);
            led5.set(P5[16]);
            led6.set(P6[16]);
            led7.set(P7[16]);
          } else if(currentmin == 50) {
            led1.set(P1[17]);
            led2.set(P2[17]);
            led3.set(P3[17]);
            led4.set(P4[17]);
            led5.set(P5[17]);
            led6.set(P6[17]);
            led7.set(P7[17]);
          }
        }
        else if (currenthour == 3)
        {
          if(currentmin == 0) {
            led1.set(P1[18]);
            led2.set(P2[18]);
            led3.set(P3[18]);
            led4.set(P4[18]);
            led5.set(P5[18]);
            led6.set(P6[18]);
            led7.set(P7[18]);
          }
          else if(currentmin == 10) {
            led1.set(P1[19]);
            led2.set(P2[19]);
            led3.set(P3[19]);
            led4.set(P4[19]);
            led5.set(P5[19]);
            led6.set(P6[19]);
            led7.set(P7[19]);
          } else if(currentmin == 20) {
            led1.set(P1[20]);
            led2.set(P2[20]);
            led3.set(P3[20]);
            led4.set(P4[20]);
            led5.set(P5[20]);
            led6.set(P6[20]);
            led7.set(P7[20]);
          } else if(currentmin == 30) {
            led1.set(P1[21]);
            led2.set(P2[21]);
            led3.set(P3[21]);
            led4.set(P4[21]);
            led5.set(P5[21]);
            led6.set(P6[21]);
            led7.set(P7[21]);
          } else if(currentmin == 40) {
            led1.set(P1[22]);
            led2.set(P2[22]);
            led3.set(P3[22]);
            led4.set(P4[22]);
            led5.set(P5[22]);
            led6.set(P6[22]);
            led7.set(P7[22]);
          } else if(currentmin == 50) {
            led1.set(P1[23]);
            led2.set(P2[23]);
            led3.set(P3[23]);
            led4.set(P4[23]);
            led5.set(P5[23]);
            led6.set(P6[23]);
            led7.set(P7[23]);
          }
        }
        else if (currenthour == 4)
        {
          if(currentmin == 0) {
            led1.set(P1[24]);
            led2.set(P2[24]);
            led3.set(P3[24]);
            led4.set(P4[24]);
            led5.set(P5[24]);
            led6.set(P6[24]);
            led7.set(P7[24]);
          }
          else if(currentmin == 10) {
            led1.set(P1[25]);
            led2.set(P2[25]);
            led3.set(P3[25]);
            led4.set(P4[25]);
            led5.set(P5[25]);
            led6.set(P6[25]);
            led7.set(P7[25]);
          } else if(currentmin == 20) {
            led1.set(P1[26]);
            led2.set(P2[26]);
            led3.set(P3[26]);
            led4.set(P4[26]);
            led5.set(P5[26]);
            led6.set(P6[26]);
            led7.set(P7[26]);
          } else if(currentmin == 30) {
            led1.set(P1[27]);
            led2.set(P2[27]);
            led3.set(P3[27]);
            led4.set(P4[27]);
            led5.set(P5[27]);
            led6.set(P6[27]);
            led7.set(P7[27]);
          } else if(currentmin == 40) {
            led1.set(P1[28]);
            led2.set(P2[28]);
            led3.set(P3[28]);
            led4.set(P4[28]);
            led5.set(P5[28]);
            led6.set(P6[28]);
            led7.set(P7[28]);
          } else if(currentmin == 50) {
            led1.set(P1[29]);
            led2.set(P2[29]);
            led3.set(P3[29]);
            led4.set(P4[29]);
            led5.set(P5[29]);
            led6.set(P6[29]);
            led7.set(P7[29]);
          }
        }
        else if (currenthour == 5)
        {
          if(currentmin == 0) {
            led1.set(P1[30]);
            led2.set(P2[30]);
            led3.set(P3[30]);
            led4.set(P4[30]);
            led5.set(P5[30]);
            led6.set(P6[30]);
            led7.set(P7[30]);
          }
          else if(currentmin == 10) {
            led1.set(P1[31]);
            led2.set(P2[31]);
            led3.set(P3[31]);
            led4.set(P4[31]);
            led5.set(P5[31]);
            led6.set(P6[31]);
            led7.set(P7[31]);
          } else if(currentmin == 20) {
            led1.set(P1[32]);
            led2.set(P2[32]);
            led3.set(P3[32]);
            led4.set(P4[32]);
            led5.set(P5[32]);
            led6.set(P6[32]);
            led7.set(P7[32]);
          } else if(currentmin == 30) {
            led1.set(P1[33]);
            led2.set(P2[33]);
            led3.set(P3[33]);
            led4.set(P4[33]);
            led5.set(P5[33]);
            led6.set(P6[33]);
            led7.set(P7[33]);
          } else if(currentmin == 40) {
            led1.set(P1[34]);
            led2.set(P2[34]);
            led3.set(P3[34]);
            led4.set(P4[34]);
            led5.set(P5[34]);
            led6.set(P6[34]);
            led7.set(P7[34]);
          } else if(currentmin == 50) {
            led1.set(P1[35]);
            led2.set(P2[35]);
            led3.set(P3[35]);
            led4.set(P4[35]);
            led5.set(P5[35]);
            led6.set(P6[35]);
            led7.set(P7[35]);
          }
        }
        else if (currenthour == 6)
        {
          if(currentmin == 0) {
            led1.set(P1[36]);
            led2.set(P2[36]);
            led3.set(P3[36]);
            led4.set(P4[36]);
            led5.set(P5[36]);
            led6.set(P6[36]);
            led7.set(P7[36]);
          }
          else if(currentmin == 10) {
            led1.set(P1[37]);
            led2.set(P2[37]);
            led3.set(P3[37]);
            led4.set(P4[37]);
            led5.set(P5[37]);
            led6.set(P6[37]);
            led7.set(P7[37]);
          } else if(currentmin == 20) {
            led1.set(P1[38]);
            led2.set(P2[38]);
            led3.set(P3[38]);
            led4.set(P4[38]);
            led5.set(P5[38]);
            led6.set(P6[38]);
            led7.set(P7[38]);
          } else if(currentmin == 30) {
            led1.set(P1[39]);
            led2.set(P2[39]);
            led3.set(P3[39]);
            led4.set(P4[39]);
            led5.set(P5[39]);
            led6.set(P6[39]);
            led7.set(P7[39]);
          } else if(currentmin == 40) {
            led1.set(P1[40]);
            led2.set(P2[40]);
            led3.set(P3[40]);
            led4.set(P4[40]);
            led5.set(P5[40]);
            led6.set(P6[40]);
            led7.set(P7[40]);
          } else if(currentmin == 50) {
            led1.set(P1[41]);
            led2.set(P2[41]);
            led3.set(P3[41]);
            led4.set(P4[41]);
            led5.set(P5[41]);
            led6.set(P6[41]);
            led7.set(P7[41]);
          }
        }
        else if (currenthour == 7)
        {
          if(currentmin == 0) {
            led1.set(P1[42]);
            led2.set(P2[42]);
            led3.set(P3[42]);
            led4.set(P4[42]);
            led5.set(P5[42]);
            led6.set(P6[42]);
            led7.set(P7[42]);
          }
          else if(currentmin == 10) {
            led1.set(P1[43]);
            led2.set(P2[43]);
            led3.set(P3[43]);
            led4.set(P4[43]);
            led5.set(P5[43]);
            led6.set(P6[43]);
            led7.set(P7[43]);
          } else if(currentmin == 20) {
            led1.set(P1[44]);
            led2.set(P2[44]);
            led3.set(P3[44]);
            led4.set(P4[44]);
            led5.set(P5[44]);
            led6.set(P6[44]);
            led7.set(P7[44]);
          } else if(currentmin == 30) {
            led1.set(P1[45]);
            led2.set(P2[45]);
            led3.set(P3[45]);
            led4.set(P4[45]);
            led5.set(P5[45]);
            led6.set(P6[45]);
            led7.set(P7[45]);
          } else if(currentmin == 40) {
            led1.set(P1[46]);
            led2.set(P2[46]);
            led3.set(P3[46]);
            led4.set(P4[46]);
            led5.set(P5[46]);
            led6.set(P6[46]);
            led7.set(P7[46]);
          } else if(currentmin == 50) {
            led1.set(P1[47]);
            led2.set(P2[47]);
            led3.set(P3[47]);
            led4.set(P4[47]);
            led5.set(P5[47]);
            led6.set(P6[47]);
            led7.set(P7[47]);
          }
        }
        else if (currenthour == 8)
        {
          if(currentmin == 0) {
            led1.set(P1[48]);
            led2.set(P2[48]);
            led3.set(P3[48]);
            led4.set(P4[48]);
            led5.set(P5[48]);
            led6.set(P6[48]);
            led7.set(P7[48]);
          }
          else if(currentmin == 10) {
            led1.set(P1[49]);
            led2.set(P2[49]);
            led3.set(P3[49]);
            led4.set(P4[49]);
            led5.set(P5[49]);
            led6.set(P6[49]);
            led7.set(P7[49]);
          } else if(currentmin == 20) {
            led1.set(P1[50]);
            led2.set(P2[50]);
            led3.set(P3[50]);
            led4.set(P4[50]);
            led5.set(P5[50]);
            led6.set(P6[50]);
            led7.set(P7[50]);
          } else if(currentmin == 30) {
            led1.set(P1[51]);
            led2.set(P2[51]);
            led3.set(P3[51]);
            led4.set(P4[51]);
            led5.set(P5[51]);
            led6.set(P6[51]);
            led7.set(P7[51]);
          } else if(currentmin == 40) {
            led1.set(P1[52]);
            led2.set(P2[52]);
            led3.set(P3[52]);
            led4.set(P4[52]);
            led5.set(P5[52]);
            led6.set(P6[52]);
            led7.set(P7[52]);
          } else if(currentmin == 50) {
            led1.set(P1[53]);
            led2.set(P2[53]);
            led3.set(P3[53]);
            led4.set(P4[53]);
            led5.set(P5[53]);
            led6.set(P6[53]);
            led7.set(P7[53]);
          }
        }
        else if (currenthour == 9)
        {
          if(currentmin == 0) {
            led1.set(P1[54]);
            led2.set(P2[54]);
            led3.set(P3[54]);
            led4.set(P4[54]);
            led5.set(P5[54]);
            led6.set(P6[54]);
            led7.set(P7[54]);
          }
          else if(currentmin == 10) {
            led1.set(P1[55]);
            led2.set(P2[55]);
            led3.set(P3[55]);
            led4.set(P4[55]);
            led5.set(P5[55]);
            led6.set(P6[55]);
            led7.set(P7[55]);
          } else if(currentmin == 20) {
            led1.set(P1[56]);
            led2.set(P2[56]);
            led3.set(P3[56]);
            led4.set(P4[56]);
            led5.set(P5[56]);
            led6.set(P6[56]);
            led7.set(P7[56]);
          } else if(currentmin == 30) {
            led1.set(P1[57]);
            led2.set(P2[57]);
            led3.set(P3[57]);
            led4.set(P4[57]);
            led5.set(P5[57]);
            led6.set(P6[57]);
            led7.set(P7[57]);
          } else if(currentmin == 40) {
            led1.set(P1[58]);
            led2.set(P2[58]);
            led3.set(P3[58]);
            led4.set(P4[58]);
            led5.set(P5[58]);
            led6.set(P6[58]);
            led7.set(P7[58]);
          } else if(currentmin == 50) {
            led1.set(P1[59]);
            led2.set(P2[59]);
            led3.set(P3[59]);
            led4.set(P4[59]);
            led5.set(P5[59]);
            led6.set(P6[59]);
            led7.set(P7[59]);
          }
        }
        else if (currenthour == 10)
        {
          if(currentmin == 0) {
            led1.set(P1[60]);
            led2.set(P2[60]);
            led3.set(P3[60]);
            led4.set(P4[60]);
            led5.set(P5[60]);
            led6.set(P6[60]);
            led7.set(P7[60]);
          }
          else if(currentmin == 10) {
            led1.set(P1[61]);
            led2.set(P2[61]);
            led3.set(P3[61]);
            led4.set(P4[61]);
            led5.set(P5[61]);
            led6.set(P6[61]);
            led7.set(P7[61]);
          } else if(currentmin == 20) {
            led1.set(P1[62]);
            led2.set(P2[62]);
            led3.set(P3[62]);
            led4.set(P4[62]);
            led5.set(P5[62]);
            led6.set(P6[62]);
            led7.set(P7[62]);
          } else if(currentmin == 30) {
            led1.set(P1[63]);
            led2.set(P2[63]);
            led3.set(P3[63]);
            led4.set(P4[63]);
            led5.set(P5[63]);
            led6.set(P6[63]);
            led7.set(P7[63]);
          } else if(currentmin == 40) {
            led1.set(P1[64]);
            led2.set(P2[64]);
            led3.set(P3[64]);
            led4.set(P4[64]);
            led5.set(P5[64]);
            led6.set(P6[64]);
            led7.set(P7[64]);
          } else if(currentmin == 50) {
            led1.set(P1[65]);
            led2.set(P2[65]);
            led3.set(P3[65]);
            led4.set(P4[65]);
            led5.set(P5[65]);
            led6.set(P6[65]);
            led7.set(P7[65]);
          }
        }
        else if (currenthour == 11)
        {
          if(currentmin == 0) {
            led1.set(P1[66]);
            led2.set(P2[66]);
            led3.set(P3[66]);
            led4.set(P4[66]);
            led5.set(P5[66]);
            led6.set(P6[66]);
            led7.set(P7[66]);
          }
          else if(currentmin == 10) {
            led1.set(P1[67]);
            led2.set(P2[67]);
            led3.set(P3[67]);
            led4.set(P4[67]);
            led5.set(P5[67]);
            led6.set(P6[67]);
            led7.set(P7[67]);
          } else if(currentmin == 20) {
            led1.set(P1[68]);
            led2.set(P2[68]);
            led3.set(P3[68]);
            led4.set(P4[68]);
            led5.set(P5[68]);
            led6.set(P6[68]);
            led7.set(P7[68]);
          } else if(currentmin == 30) {
            led1.set(P1[69]);
            led2.set(P2[69]);
            led3.set(P3[69]);
            led4.set(P4[69]);
            led5.set(P5[69]);
            led6.set(P6[69]);
            led7.set(P7[69]);
          } else if(currentmin == 40) {
            led1.set(P1[70]);
            led2.set(P2[70]);
            led3.set(P3[70]);
            led4.set(P4[70]);
            led5.set(P5[70]);
            led6.set(P6[70]);
            led7.set(P7[70]);
          } else if(currentmin == 50) {
            led1.set(P1[71]);
            led2.set(P2[71]);
            led3.set(P3[71]);
            led4.set(P4[71]);
            led5.set(P5[71]);
            led6.set(P6[71]);
            led7.set(P7[71]);
          }
        }
        else if (currenthour == 12)
        {
          if(currentmin == 0) {
            led1.set(P1[72]);
            led2.set(P2[72]);
            led3.set(P3[72]);
            led4.set(P4[72]);
            led5.set(P5[72]);
            led6.set(P6[72]);
            led7.set(P7[72]);
          }
          else if(currentmin == 10) {
            led1.set(P1[73]);
            led2.set(P2[73]);
            led3.set(P3[73]);
            led4.set(P4[73]);
            led5.set(P5[73]);
            led6.set(P6[73]);
            led7.set(P7[73]);
          } else if(currentmin == 20) {
            led1.set(P1[74]);
            led2.set(P2[74]);
            led3.set(P3[74]);
            led4.set(P4[74]);
            led5.set(P5[74]);
            led6.set(P6[74]);
            led7.set(P7[74]);
          } else if(currentmin == 30) {
            led1.set(P1[75]);
            led2.set(P2[75]);
            led3.set(P3[75]);
            led4.set(P4[75]);
            led5.set(P5[75]);
            led6.set(P6[75]);
            led7.set(P7[75]);
          } else if(currentmin == 40) {
            led1.set(P1[76]);
            led2.set(P2[76]);
            led3.set(P3[76]);
            led4.set(P4[76]);
            led5.set(P5[76]);
            led6.set(P6[76]);
            led7.set(P7[76]);
          } else if(currentmin == 50) {
            led1.set(P1[77]);
            led2.set(P2[77]);
            led3.set(P3[77]);
            led4.set(P4[77]);
            led5.set(P5[77]);
            led6.set(P6[77]);
            led7.set(P7[77]);
          }
        }
        else if (currenthour == 13)
        {
          if(currentmin == 0) {
            led1.set(P1[78]);
            led2.set(P2[78]);
            led3.set(P3[78]);
            led4.set(P4[78]);
            led5.set(P5[78]);
            led6.set(P6[78]);
            led7.set(P7[78]);
          }
          else if(currentmin == 10) {
            led1.set(P1[79]);
            led2.set(P2[79]);
            led3.set(P3[79]);
            led4.set(P4[79]);
            led5.set(P5[79]);
            led6.set(P6[79]);
            led7.set(P7[79]);
          } else if(currentmin == 20) {
            led1.set(P1[80]);
            led2.set(P2[80]);
            led3.set(P3[80]);
            led4.set(P4[80]);
            led5.set(P5[80]);
            led6.set(P6[80]);
            led7.set(P7[80]);
          } else if(currentmin == 30) {
            led1.set(P1[81]);
            led2.set(P2[81]);
            led3.set(P3[81]);
            led4.set(P4[81]);
            led5.set(P5[81]);
            led6.set(P6[81]);
            led7.set(P7[81]);
          } else if(currentmin == 40) {
            led1.set(P1[82]);
            led2.set(P2[82]);
            led3.set(P3[82]);
            led4.set(P4[82]);
            led5.set(P5[82]);
            led6.set(P6[82]);
            led7.set(P7[82]);
          } else if(currentmin == 50) {
            led1.set(P1[83]);
            led2.set(P2[83]);
            led3.set(P3[83]);
            led4.set(P4[83]);
            led5.set(P5[83]);
            led6.set(P6[83]);
            led7.set(P7[83]);
          }
        }
        else if (currenthour == 14)
        {
          if(currentmin == 0) {
            led1.set(P1[84]);
            led2.set(P2[84]);
            led3.set(P3[84]);
            led4.set(P4[84]);
            led5.set(P5[84]);
            led6.set(P6[84]);
            led7.set(P7[84]);
          }
          else if(currentmin == 10) {
            led1.set(P1[85]);
            led2.set(P2[85]);
            led3.set(P3[85]);
            led4.set(P4[85]);
            led5.set(P5[85]);
            led6.set(P6[85]);
            led7.set(P7[85]);
          } else if(currentmin == 20) {
            led1.set(P1[86]);
            led2.set(P2[86]);
            led3.set(P3[86]);
            led4.set(P4[86]);
            led5.set(P5[86]);
            led6.set(P6[86]);
            led7.set(P7[86]);
          } else if(currentmin == 30) {
            led1.set(P1[87]);
            led2.set(P2[87]);
            led3.set(P3[87]);
            led4.set(P4[87]);
            led5.set(P5[87]);
            led6.set(P6[87]);
            led7.set(P7[87]);
          } else if(currentmin == 40) {
            led1.set(P1[88]);
            led2.set(P2[88]);
            led3.set(P3[88]);
            led4.set(P4[88]);
            led5.set(P5[88]);
            led6.set(P6[88]);
            led7.set(P7[88]);
          } else if(currentmin == 50) {
            led1.set(P1[89]);
            led2.set(P2[89]);
            led3.set(P3[89]);
            led4.set(P4[89]);
            led5.set(P5[89]);
            led6.set(P6[89]);
            led7.set(P7[89]);
          }
        }
        else if (currenthour == 15)
        {
          if(currentmin == 0) {
            led1.set(P1[90]);
            led2.set(P2[90]);
            led3.set(P3[90]);
            led4.set(P4[90]);
            led5.set(P5[90]);
            led6.set(P6[90]);
            led7.set(P7[90]);
          }
          else if(currentmin == 10) {
            led1.set(P1[91]);
            led2.set(P2[91]);
            led3.set(P3[91]);
            led4.set(P4[91]);
            led5.set(P5[91]);
            led6.set(P6[91]);
            led7.set(P7[91]);
          } else if(currentmin == 20) {
            led1.set(P1[92]);
            led2.set(P2[92]);
            led3.set(P3[92]);
            led4.set(P4[92]);
            led5.set(P5[92]);
            led6.set(P6[92]);
            led7.set(P7[92]);
          } else if(currentmin == 30) {
            led1.set(P1[93]);
            led2.set(P2[93]);
            led3.set(P3[93]);
            led4.set(P4[93]);
            led5.set(P5[93]);
            led6.set(P6[93]);
            led7.set(P7[93]);
          } else if(currentmin == 40) {
            led1.set(P1[94]);
            led2.set(P2[94]);
            led3.set(P3[94]);
            led4.set(P4[94]);
            led5.set(P5[94]);
            led6.set(P6[94]);
            led7.set(P7[94]);
          } else if(currentmin == 50) {
            led1.set(P1[95]);
            led2.set(P2[95]);
            led3.set(P3[95]);
            led4.set(P4[95]);
            led5.set(P5[95]);
            led6.set(P6[95]);
            led7.set(P7[95]);
          }
        }
        else if (currenthour == 16)
        {
          if(currentmin == 0) {
            led1.set(P1[96]);
            led2.set(P2[96]);
            led3.set(P3[96]);
            led4.set(P4[96]);
            led5.set(P5[96]);
            led6.set(P6[96]);
            led7.set(P7[96]);
          }
          else if(currentmin == 10) {
            led1.set(P1[97]);
            led2.set(P2[97]);
            led3.set(P3[97]);
            led4.set(P4[97]);
            led5.set(P5[97]);
            led6.set(P6[97]);
            led7.set(P7[97]);
          } else if(currentmin == 20) {
            led1.set(P1[98]);
            led2.set(P2[98]);
            led3.set(P3[98]);
            led4.set(P4[98]);
            led5.set(P5[98]);
            led6.set(P6[98]);
            led7.set(P7[98]);
          } else if(currentmin == 30) {
            led1.set(P1[99]);
            led2.set(P2[99]);
            led3.set(P3[99]);
            led4.set(P4[99]);
            led5.set(P5[99]);
            led6.set(P6[99]);
            led7.set(P7[99]);
          } else if(currentmin == 40) {
            led1.set(P1[100]);
            led2.set(P2[100]);
            led3.set(P3[100]);
            led4.set(P4[100]);
            led5.set(P5[100]);
            led6.set(P6[100]);
            led7.set(P7[100]);
          } else if(currentmin == 50) {
            led1.set(P1[101]);
            led2.set(P2[101]);
            led3.set(P3[101]);
            led4.set(P4[101]);
            led5.set(P5[101]);
            led6.set(P6[101]);
            led7.set(P7[101]);
          }
        }
        else if (currenthour == 17)
        {
         if(currentmin == 0) {
            led1.set(P1[102]);
            led2.set(P2[102]);
            led3.set(P3[102]);
            led4.set(P4[102]);
            led5.set(P5[102]);
            led6.set(P6[102]);
            led7.set(P7[102]);
          }
          else if(currentmin == 10) {
            led1.set(P1[103]);
            led2.set(P2[103]);
            led3.set(P3[103]);
            led4.set(P4[103]);
            led5.set(P5[103]);
            led6.set(P6[103]);
            led7.set(P7[103]);
          } else if(currentmin == 20) {
            led1.set(P1[104]);
            led2.set(P2[104]);
            led3.set(P3[104]);
            led4.set(P4[104]);
            led5.set(P5[104]);
            led6.set(P6[104]);
            led7.set(P7[104]);
          } else if(currentmin == 30) {
            led1.set(P1[105]);
            led2.set(P2[105]);
            led3.set(P3[105]);
            led4.set(P4[105]);
            led5.set(P5[105]);
            led6.set(P6[105]);
            led7.set(P7[105]);
          } else if(currentmin == 40) {
            led1.set(P1[106]);
            led2.set(P2[106]);
            led3.set(P3[106]);
            led4.set(P4[106]);
            led5.set(P5[106]);
            led6.set(P6[106]);
            led7.set(P7[106]);
          } else if(currentmin == 50) {
            led1.set(P1[107]);
            led2.set(P2[107]);
            led3.set(P3[107]);
            led4.set(P4[107]);
            led5.set(P5[107]);
            led6.set(P6[107]);
            led7.set(P7[107]);
          }
        }
        else if (currenthour == 18)
        {
         if(currentmin == 0) {
            led1.set(P1[108]);
            led2.set(P2[108]);
            led3.set(P3[108]);
            led4.set(P4[108]);
            led5.set(P5[108]);
            led6.set(P6[108]);
            led7.set(P7[108]);
          }
          else if(currentmin == 10) {
            led1.set(P1[109]);
            led2.set(P2[109]);
            led3.set(P3[109]);
            led4.set(P4[109]);
            led5.set(P5[109]);
            led6.set(P6[109]);
            led7.set(P7[109]);
          } else if(currentmin == 20) {
            led1.set(P1[110]);
            led2.set(P2[110]);
            led3.set(P3[110]);
            led4.set(P4[110]);
            led5.set(P5[110]);
            led6.set(P6[110]);
            led7.set(P7[110]);
          } else if(currentmin == 30) {
            led1.set(P1[111]);
            led2.set(P2[111]);
            led3.set(P3[111]);
            led4.set(P4[111]);
            led5.set(P5[111]);
            led6.set(P6[111]);
            led7.set(P7[111]);
          } else if(currentmin == 40) {
            led1.set(P1[112]);
            led2.set(P2[112]);
            led3.set(P3[112]);
            led4.set(P4[112]);
            led5.set(P5[112]);
            led6.set(P6[112]);
            led7.set(P7[112]);
          } else if(currentmin == 50) {
            led1.set(P1[113]);
            led2.set(P2[113]);
            led3.set(P3[113]);
            led4.set(P4[113]);
            led5.set(P5[113]);
            led6.set(P6[113]);
            led7.set(P7[113]);
          }
        }
        else if (currenthour == 19)
        {
         if(currentmin == 0) {
            led1.set(P1[114]);
            led2.set(P2[114]);
            led3.set(P3[114]);
            led4.set(P4[114]);
            led5.set(P5[114]);
            led6.set(P6[114]);
            led7.set(P7[114]);
          }
          else if(currentmin == 10) {
            led1.set(P1[115]);
            led2.set(P2[115]);
            led3.set(P3[115]);
            led4.set(P4[115]);
            led5.set(P5[115]);
            led6.set(P6[115]);
            led7.set(P7[115]);
          } else if(currentmin == 20) {
            led1.set(P1[116]);
            led2.set(P2[116]);
            led3.set(P3[116]);
            led4.set(P4[116]);
            led5.set(P5[116]);
            led6.set(P6[116]);
            led7.set(P7[116]);
          } else if(currentmin == 30) {
            led1.set(P1[117]);
            led2.set(P2[117]);
            led3.set(P3[117]);
            led4.set(P4[117]);
            led5.set(P5[117]);
            led6.set(P6[117]);
            led7.set(P7[117]);
          } else if(currentmin == 40) {
            led1.set(P1[118]);
            led2.set(P2[118]);
            led3.set(P3[118]);
            led4.set(P4[118]);
            led5.set(P5[118]);
            led6.set(P6[118]);
            led7.set(P7[118]);
          } else if(currentmin == 50) {
            led1.set(P1[119]);
            led2.set(P2[119]);
            led3.set(P3[119]);
            led4.set(P4[119]);
            led5.set(P5[119]);
            led6.set(P6[119]);
            led7.set(P7[119]);
          }
        }
        else if (currenthour == 20)
        {
          if(currentmin == 0) {
            led1.set(P1[120]);
            led2.set(P2[120]);
            led3.set(P3[120]);
            led4.set(P4[120]);
            led5.set(P5[120]);
            led6.set(P6[120]);
            led7.set(P7[120]);
          }
          else if(currentmin == 10) {
            led1.set(P1[121]);
            led2.set(P2[121]);
            led3.set(P3[121]);
            led4.set(P4[121]);
            led5.set(P5[121]);
            led6.set(P6[121]);
            led7.set(P7[121]);
          } else if(currentmin == 20) {
            led1.set(P1[122]);
            led2.set(P2[122]);
            led3.set(P3[122]);
            led4.set(P4[122]);
            led5.set(P5[122]);
            led6.set(P6[122]);
            led7.set(P7[122]);
          } else if(currentmin == 30) {
            led1.set(P1[123]);
            led2.set(P2[123]);
            led3.set(P3[123]);
            led4.set(P4[123]);
            led5.set(P5[123]);
            led6.set(P6[123]);
            led7.set(P7[123]);
          } else if(currentmin == 40) {
            led1.set(P1[124]);
            led2.set(P2[124]);
            led3.set(P3[124]);
            led4.set(P4[124]);
            led5.set(P5[124]);
            led6.set(P6[124]);
            led7.set(P7[124]);
          } else if(currentmin == 50) {
            led1.set(P1[125]);
            led2.set(P2[125]);
            led3.set(P3[125]);
            led4.set(P4[125]);
            led5.set(P5[125]);
            led6.set(P6[125]);
            led7.set(P7[125]);
          }
        }
        else if (currenthour == 21)
        {
          if(currentmin == 0) {
            led1.set(P1[126]);
            led2.set(P2[126]);
            led3.set(P3[126]);
            led4.set(P4[126]);
            led5.set(P5[126]);
            led6.set(P6[126]);
            led7.set(P7[126]);
          }
          else if(currentmin == 10) {
            led1.set(P1[127]);
            led2.set(P2[127]);
            led3.set(P3[127]);
            led4.set(P4[127]);
            led5.set(P5[127]);
            led6.set(P6[127]);
            led7.set(P7[127]);
          } else if(currentmin == 20) {
            led1.set(P1[128]);
            led2.set(P2[128]);
            led3.set(P3[128]);
            led4.set(P4[128]);
            led5.set(P5[128]);
            led6.set(P6[128]);
            led7.set(P7[128]);
          } else if(currentmin == 30) {
            led1.set(P1[129]);
            led2.set(P2[129]);
            led3.set(P3[129]);
            led4.set(P4[129]);
            led5.set(P5[129]);
            led6.set(P6[129]);
            led7.set(P7[129]);
          } else if(currentmin == 40) {
            led1.set(P1[130]);
            led2.set(P2[130]);
            led3.set(P3[130]);
            led4.set(P4[130]);
            led5.set(P5[130]);
            led6.set(P6[130]);
            led7.set(P7[130]);
          } else if(currentmin == 50) {
            led1.set(P1[131]);
            led2.set(P2[131]);
            led3.set(P3[131]);
            led4.set(P4[131]);
            led5.set(P5[131]);
            led6.set(P6[131]);
            led7.set(P7[131]);
          }
        }
        else if (currenthour == 22)
        {
          if(currentmin == 0) {
            led1.set(P1[132]);
            led2.set(P2[132]);
            led3.set(P3[132]);
            led4.set(P4[132]);
            led5.set(P5[132]);
            led6.set(P6[132]);
            led7.set(P7[132]);
          }
          else if(currentmin == 10) {
            led1.set(P1[133]);
            led2.set(P2[133]);
            led3.set(P3[133]);
            led4.set(P4[133]);
            led5.set(P5[133]);
            led6.set(P6[133]);
            led7.set(P7[133]);
          } else if(currentmin == 20) {
            led1.set(P1[134]);
            led2.set(P2[134]);
            led3.set(P3[134]);
            led4.set(P4[134]);
            led5.set(P5[134]);
            led6.set(P6[134]);
            led7.set(P7[134]);
          } else if(currentmin == 30) {
            led1.set(P1[135]);
            led2.set(P2[135]);
            led3.set(P3[135]);
            led4.set(P4[135]);
            led5.set(P5[135]);
            led6.set(P6[135]);
            led7.set(P7[135]);
          } else if(currentmin == 40) {
            led1.set(P1[136]);
            led2.set(P2[136]);
            led3.set(P3[136]);
            led4.set(P4[136]);
            led5.set(P5[136]);
            led6.set(P6[136]);
            led7.set(P7[136]);
          } else if(currentmin == 50) {
            led1.set(P1[137]);
            led2.set(P2[137]);
            led3.set(P3[137]);
            led4.set(P4[137]);
            led5.set(P5[137]);
            led6.set(P6[137]);
            led7.set(P7[137]);
          }
        }
        else if (currenthour == 23)
        {
          if(currentmin == 0) {
            led1.set(P1[138]);
            led2.set(P2[138]);
            led3.set(P3[138]);
            led4.set(P4[138]);
            led5.set(P5[138]);
            led6.set(P6[138]);
            led7.set(P7[138]);
          }
          else if(currentmin == 10) {
            led1.set(P1[139]);
            led2.set(P2[139]);
            led3.set(P3[139]);
            led4.set(P4[139]);
            led5.set(P5[139]);
            led6.set(P6[139]);
            led7.set(P7[139]);
          } else if(currentmin == 20) {
            led1.set(P1[140]);
            led2.set(P2[140]);
            led3.set(P3[140]);
            led4.set(P4[140]);
            led5.set(P5[140]);
            led6.set(P6[140]);
            led7.set(P7[140]);
          } else if(currentmin == 30) {
            led1.set(P1[141]);
            led2.set(P2[141]);
            led3.set(P3[141]);
            led4.set(P4[141]);
            led5.set(P5[141]);
            led6.set(P6[141]);
            led7.set(P7[141]);
          } else if(currentmin == 40) {
            led1.set(P1[142]);
            led2.set(P2[142]);
            led3.set(P3[142]);
            led4.set(P4[142]);
            led5.set(P5[142]);
            led6.set(P6[142]);
            led7.set(P7[142]);
          } else if(currentmin == 50) {
            led1.set(P1[143]);
            led2.set(P2[143]);
            led3.set(P3[143]);
            led4.set(P4[143]);
            led5.set(P5[143]);
            led6.set(P6[143]);
            led7.set(P7[143]);
          }
        }
      }
      else if (PWM_INFO_TESTMODE == "test")
      {
        Serial.printf("test mode .... current sec is: %d the led1's volume is: %d \n",currentsec,P1[TESTMODE_COUNT]);
        if(TESTMODE_COUNT >143) {
          TESTMODE_COUNT = 0;
        }

        led1.set(P1[TESTMODE_COUNT]);
        led2.set(P2[TESTMODE_COUNT]);
        led3.set(P3[TESTMODE_COUNT]);
        led4.set(P4[TESTMODE_COUNT]);
        led5.set(P5[TESTMODE_COUNT]);
        led6.set(P6[TESTMODE_COUNT]);
        led7.set(P7[TESTMODE_COUNT]);
        
        TESTMODE_COUNT = TESTMODE_COUNT +1;
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
      Serial.printf("fix mode ... the led1's volume is: %d \n", P1[144]);
      led1.set(P1[144]);
      led2.set(P2[144]);
      led3.set(P3[144]);
      led4.set(P4[144]);
      led5.set(P5[144]);
      led6.set(P6[144]);
      led7.set(P7[144]);
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