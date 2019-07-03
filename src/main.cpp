#include <SPIFFS.h>
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <cJSON.h>
#include <time.h>
#include <sys/time.h>
#include "LED_ESP32.h"
#include <ESPmDNS.h>
#include <Update.h>
#include <PubSubClient.h>
#include <ArduinoHttpClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define RESET_BUTTON 16
#define VERSION_NUM "0.60"
// #define ESP_HOST_NAME "esp1006"
#define ESP_RTC_TICK 1542012457

String ESP_HOST_NAME = "esp004";
String ESP_MAC = "00000000";
String PWM_INFO_SHOWTYPE, PWM_INFO_TESTMODE, PWM_INFO_CONMODE, PWM_INFO_RTC, PWM_INFO_VERSION;

int P1[145] = {};
int P2[145] = {};
int P3[145] = {};
int P4[145] = {};
int P5[145] = {};
int P6[145] = {};
int P7[145] = {};
int P8[145] = {};

int TESTMODE_COUNT = 0;

String SSID, SSID_PWD;

bool IS_SMART = false;
bool RESET_FLAG = false;

const char *ssid = ESP_HOST_NAME.c_str();
const char *password = "11111111";
const char *host = ESP_HOST_NAME.c_str();
// const char *mqttServer = "m12.cloudmqtt.com";
// const int mqttPort = 16610;
// const char *mqttuser = "cqyjmitd";
// const char *mqttpwd = "SXLMuaorn881";
const char *mqttServer = "www.polypite.com";
const int mqttPort = 1883;
const char *mqttuser = "";
const char *mqttpwd = "";

LED_ESP32 led1(4, 7, 100);
LED_ESP32 led2(12, 1, 100);
LED_ESP32 led3(13, 2, 100);
LED_ESP32 led4(15, 3, 100);
LED_ESP32 led5(21, 4, 100);
LED_ESP32 led6(22, 5, 100);
LED_ESP32 led7(27, 6, 100);
LED_ESP32 led8(25, 0, 100);

// LED_ESP32 led8(2, 7, 100);
// LED_ESP32 led1(4, 0, 100);
// LED_ESP32 led2(12, 1, 100);
// LED_ESP32 led3(13, 2, 100);
// LED_ESP32 led4(15, 3, 100);
// LED_ESP32 led5(22, 4, 100);
// LED_ESP32 led6(23, 5, 100);
// LED_ESP32 led7(25, 6, 100);

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
HttpClient http(espClient,mqttServer);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"cn.pool.ntp.org",28800,60000);



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
  deleteFile(SPIFFS, "/mid.ini");
  if (!SPIFFS.exists("/mid.ini"))
  {
    Serial.println("init the mid file");
    String midcontent = "{\"mid\":\"{mid}\",\"mac\":\"{mac}\"}";
    // int s;
    // srand(time(NULL));
    // s = 1000 + rand() % 9999;
    // char c[8];
    // itoa(s, c, 10);
    // String mid = "esp_{mid}";
    // mid.replace("{mid}", c);
    midcontent.replace("{mac}", WiFi.macAddress());
    midcontent.replace("{mid}", "esp_" + WiFi.macAddress());
    Serial.println(midcontent);
    writeFile(SPIFFS, "/mid.ini", midcontent.c_str());
  }
  String pwminfocontent = "{\"showtype\":\"fix\",\"testmode\":\"test\",\"sysdate\":\"{unixtick}\",\"status\":\"stop\",\"conmode\": \"local\",\"version\":\"{version_num}\"}";
  pwminfocontent.replace("{version_num}", VERSION_NUM);
  pwminfocontent.replace("{unixtick}", String(ESP_RTC_TICK));
  writeFile(SPIFFS, "/pwminfo.ini", pwminfocontent.c_str());
  //writeFile(SPIFFS, "/p.ini", "{\"t000\":\"00000000000000\",\"t001\":\"00000000000000\",\"t002\":\"00000000000000\",\"t003\":\"00000000000000\",\"t004\":\"00000000000000\",\"t005\":\"00000000000000\",\"t010\":\"00000000000000\",\"t011\":\"00000000000000\",\"t012\":\"00000000000000\",\"t013\":\"00000000000000\",\"t014\":\"00000000000000\",\"t015\":\"00000000000000\",\"t020\":\"00000000000000\",\"t021\":\"00000000000000\",\"t022\":\"00000000000000\",\"t023\":\"00000000000000\",\"t024\":\"00000000000000\",\"t025\":\"00000000000000\",\"t030\":\"00000000000000\",\"t031\":\"00000000000000\",\"t032\":\"00000000000000\",\"t033\":\"00000000000000\",\"t034\":\"00000000000000\",\"t035\":\"00000000000000\",\"t040\":\"00000000000000\",\"t041\":\"00000000000000\",\"t042\":\"00000000000000\",\"t043\":\"00000000000000\",\"t044\":\"00000000000000\",\"t045\":\"00000000000000\",\"t050\":\"00000000000000\",\"t051\":\"00000000000000\",\"t052\":\"00000000000000\",\"t053\":\"00000000000000\",\"t054\":\"00000000000000\",\"t055\":\"00000000000000\",\"t060\":\"00000000000000\",\"t061\":\"00000000000000\",\"t062\":\"00000000000000\",\"t063\":\"00000000000000\",\"t064\":\"00000000000000\",\"t065\":\"00000000000000\",\"t070\":\"00000000000000\",\"t071\":\"00000000000000\",\"t072\":\"00000000000000\",\"t073\":\"00000000000000\",\"t074\":\"00000000000000\",\"t075\":\"00000000000000\",\"t080\":\"00000000000000\",\"t081\":\"00000000000000\",\"t082\":\"00000000000000\",\"t083\":\"00000000000000\",\"t084\":\"00000000000000\",\"t085\":\"00000000000000\",\"t090\":\"00000000000000\",\"t091\":\"00000000000000\",\"t092\":\"00000000000000\",\"t093\":\"00000000000000\",\"t094\":\"00000000000000\",\"t095\":\"00000000000000\",\"t100\":\"00000000000000\",\"t101\":\"00000000000000\",\"t102\":\"00000000000000\",\"t103\":\"00000000000000\",\"t104\":\"00000000000000\",\"t105\":\"00000000000000\",\"t110\":\"00000000000000\",\"t111\":\"00000000000000\",\"t112\":\"00000000000000\",\"t113\":\"00000000000000\",\"t114\":\"00000000000000\",\"t115\":\"00000000000000\",\"t120\":\"00000000000000\",\"t121\":\"00000000000000\",\"t122\":\"00000000000000\",\"t123\":\"00000000000000\",\"t124\":\"00000000000000\",\"t125\":\"00000000000000\",\"t130\":\"00000000000000\",\"t131\":\"00000000000000\",\"t132\":\"00000000000000\",\"t133\":\"00000000000000\",\"t134\":\"00000000000000\",\"t135\":\"00000000000000\",\"t140\":\"00000000000000\",\"t141\":\"00000000000000\",\"t142\":\"00000000000000\",\"t143\":\"00000000000000\",\"t144\":\"00000000000000\",\"t145\":\"00000000000000\",\"t150\":\"00000000000000\",\"t151\":\"00000000000000\",\"t152\":\"00000000000000\",\"t153\":\"00000000000000\",\"t154\":\"00000000000000\",\"t155\":\"00000000000000\",\"t160\":\"00000000000000\",\"t161\":\"00000000000000\",\"t162\":\"00000000000000\",\"t163\":\"00000000000000\",\"t164\":\"00000000000000\",\"t165\":\"00000000000000\",\"t170\":\"00000000000000\",\"t171\":\"00000000000000\",\"t172\":\"00000000000000\",\"t173\":\"00000000000000\",\"t174\":\"00000000000000\",\"t175\":\"00000000000000\",\"t180\":\"00000000000000\",\"t181\":\"00000000000000\",\"t182\":\"00000000000000\",\"t183\":\"00000000000000\",\"t184\":\"00000000000000\",\"t185\":\"00000000000000\",\"t190\":\"00000000000000\",\"t191\":\"00000000000000\",\"t192\":\"00000000000000\",\"t193\":\"00000000000000\",\"t194\":\"00000000000000\",\"t195\":\"00000000000000\",\"t200\":\"00000000000000\",\"t201\":\"00000000000000\",\"t202\":\"00000000000000\",\"t203\":\"00000000000000\",\"t204\":\"00000000000000\",\"t205\":\"00000000000000\",\"t210\":\"00000000000000\",\"t211\":\"00000000000000\",\"t212\":\"00000000000000\",\"t213\":\"00000000000000\",\"t214\":\"00000000000000\",\"t215\":\"00000000000000\",\"t220\":\"00000000000000\",\"t221\":\"00000000000000\",\"t222\":\"00000000000000\",\"t223\":\"00000000000000\",\"t224\":\"00000000000000\",\"t225\":\"00000000000000\",\"t230\":\"00000000000000\",\"t231\":\"00000000000000\",\"t232\":\"00000000000000\",\"t233\":\"00000000000000\",\"t234\":\"00000000000000\",\"t235\":\"00000000000000\",\"tfix\":\"00000000000000\"}");
  deleteFile(SPIFFS, "/p.ini");
  writeFile(SPIFFS, "/p.ini", "{\"t000\":\"0000000000000000\",\"t001\":\"0000000000000000\",\"t002\":\"0000000000000000\",\"t003\":\"0000000000000000\",\"t004\":\"0000000000000000\",\"t005\":\"0000000000000000\",\"t010\":\"0000000000000000\",\"t011\":\"0000000000000000\",\"t012\":\"0000000000000000\",\"t013\":\"0000000000000000\",\"t014\":\"0000000000000000\",\"t015\":\"0000000000000000\",\"t020\":\"0000000000000000\",\"t021\":\"0000000000000000\",\"t022\":\"0000000000000000\",\"t023\":\"0000000000000000\",\"t024\":\"0000000000000000\",\"t025\":\"0000000000000000\",\"t030\":\"0000000000000000\",\"t031\":\"0000000000000000\",\"t032\":\"0000000000000000\",\"t033\":\"0000000000000000\",\"t034\":\"0000000000000000\",\"t035\":\"0000000000000000\",\"t040\":\"0000000000000000\",\"t041\":\"0000000000000000\",\"t042\":\"0000000000000000\",\"t043\":\"0000000000000000\",\"t044\":\"0000000000000000\",\"t045\":\"0000000000000000\",\"t050\":\"0000000000000000\",\"t051\":\"0000000000000000\",\"t052\":\"0000000000000000\",\"t053\":\"0000000000000000\",\"t054\":\"0000000000000000\",\"t055\":\"0000000000000000\",\"t060\":\"0000000000000000\",\"t061\":\"0000000000000000\",\"t062\":\"0000000000000000\",\"t063\":\"0000000000000000\",\"t064\":\"0000000000000000\",\"t065\":\"0000000000000000\",\"t070\":\"0000000000000000\",\"t071\":\"0000000000000000\",\"t072\":\"0000000000000000\",\"t073\":\"0000000000000000\",\"t074\":\"0000000000000000\",\"t075\":\"0000000000000000\",\"t080\":\"0000000000000000\",\"t081\":\"0000000000000000\",\"t082\":\"0000000000000000\",\"t083\":\"0000000000000000\",\"t084\":\"0000000000000000\",\"t085\":\"0000000000000000\",\"t090\":\"0000000000000000\",\"t091\":\"0000000000000000\",\"t092\":\"0000000000000000\",\"t093\":\"0000000000000000\",\"t094\":\"0000000000000000\",\"t095\":\"0000000000000000\",\"t100\":\"0000000000000000\",\"t101\":\"0000000000000000\",\"t102\":\"0000000000000000\",\"t103\":\"0000000000000000\",\"t104\":\"0000000000000000\",\"t105\":\"0000000000000000\",\"t110\":\"0000000000000000\",\"t111\":\"0000000000000000\",\"t112\":\"0000000000000000\",\"t113\":\"0000000000000000\",\"t114\":\"0000000000000000\",\"t115\":\"0000000000000000\",\"t120\":\"0000000000000000\",\"t121\":\"0000000000000000\",\"t122\":\"0000000000000000\",\"t123\":\"0000000000000000\",\"t124\":\"0000000000000000\",\"t125\":\"0000000000000000\",\"t130\":\"0000000000000000\",\"t131\":\"0000000000000000\",\"t132\":\"0000000000000000\",\"t133\":\"0000000000000000\",\"t134\":\"0000000000000000\",\"t135\":\"0000000000000000\",\"t140\":\"0000000000000000\",\"t141\":\"0000000000000000\",\"t142\":\"0000000000000000\",\"t143\":\"0000000000000000\",\"t144\":\"0000000000000000\",\"t145\":\"0000000000000000\",\"t150\":\"0000000000000000\",\"t151\":\"0000000000000000\",\"t152\":\"0000000000000000\",\"t153\":\"0000000000000000\",\"t154\":\"0000000000000000\",\"t155\":\"0000000000000000\",\"t160\":\"0000000000000000\",\"t161\":\"0000000000000000\",\"t162\":\"0000000000000000\",\"t163\":\"0000000000000000\",\"t164\":\"0000000000000000\",\"t165\":\"0000000000000000\",\"t170\":\"0000000000000000\",\"t171\":\"0000000000000000\",\"t172\":\"0000000000000000\",\"t173\":\"0000000000000000\",\"t174\":\"0000000000000000\",\"t175\":\"0000000000000000\",\"t180\":\"0000000000000000\",\"t181\":\"0000000000000000\",\"t182\":\"0000000000000000\",\"t183\":\"0000000000000000\",\"t184\":\"0000000000000000\",\"t185\":\"0000000000000000\",\"t190\":\"0000000000000000\",\"t191\":\"0000000000000000\",\"t192\":\"0000000000000000\",\"t193\":\"0000000000000000\",\"t194\":\"0000000000000000\",\"t195\":\"0000000000000000\",\"t200\":\"0000000000000000\",\"t201\":\"0000000000000000\",\"t202\":\"0000000000000000\",\"t203\":\"0000000000000000\",\"t204\":\"0000000000000000\",\"t205\":\"0000000000000000\",\"t210\":\"0000000000000000\",\"t211\":\"0000000000000000\",\"t212\":\"0000000000000000\",\"t213\":\"0000000000000000\",\"t214\":\"0000000000000000\",\"t215\":\"0000000000000000\",\"t220\":\"0000000000000000\",\"t221\":\"0000000000000000\",\"t222\":\"0000000000000000\",\"t223\":\"0000000000000000\",\"t224\":\"0000000000000000\",\"t225\":\"0000000000000000\",\"t230\":\"0000000000000000\",\"t231\":\"0000000000000000\",\"t232\":\"0000000000000000\",\"t233\":\"0000000000000000\",\"t234\":\"0000000000000000\",\"t235\":\"0000000000000000\",\"tfix\":\"0000000000000000\"}");
  
  SPIFFS.end();
}

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
    sval = content.substring(14, 16);
    sscanf(sval.c_str(), "%x", &intval);
    P8[i] = intval;
  }

  cJSON_Delete(root);
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
  String topic_name_setp = ESP_HOST_NAME;
  topic_name_setp = topic_name_setp + "/setp";
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

  char *p_content;
  char dst[30][80];
  int cnt;
  cJSON *root = NULL;
  cJSON *item = NULL;
  const char *jsonstr = filecontent.c_str();
  root = cJSON_Parse(jsonstr);
  String itemstr;
  String checktimetopic = "esp32/checktime";

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
    Serial.print("the sysdate is: ");
    Serial.println(PWM_INFO_RTC);
    PWM_INFO_RTC.replace("\"", "");

    // struct timeval stime;
    // stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
    // Serial.print("the sysdate add 28816 = ");
    // Serial.println(stime.tv_sec);
    // settimeofday(&stime, NULL);
    
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
    //stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
    stime.tv_sec = PWM_INFO_RTC.toInt();
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
  else if (String(topic) == topic_name_setp)
  {
    lightopr(filecontent);
    writeFile(SPIFFS, "/p.ini", filecontent.c_str());
  }
  else
  {
    Serial.println("mqtt action not found!");
  }

  cJSON_Delete(root);
  SPIFFS.end();
}

// mqtt service
void mqttconn()
{
  while (!client.connected())
  {
    Serial.println("MQTT is connecting...");
    //if (client.connect(ESP_HOST_NAME, mqttuser, mqttpwd))
    if (client.connect(ESP_MAC.c_str()))
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

      // subscribe the setp service
      String recv_topic_setp = ESP_HOST_NAME;
      recv_topic_setp = recv_topic_setp + "/setp";
      client.subscribe(recv_topic_setp.c_str());
      // // subscibe the p1 light service
      // String recv_topic_p1 = ESP_HOST_NAME;
      // recv_topic_p1 = recv_topic_p1 + "/p1";
      // client.subscribe(recv_topic_p1.c_str());
      // // subscibe the p2 light service
      // String recv_topic_p2 = ESP_HOST_NAME;
      // recv_topic_p2 = recv_topic_p2 + "/p2";
      // client.subscribe(recv_topic_p2.c_str());
      // // subscibe the p3 light service
      // String recv_topic_p3 = ESP_HOST_NAME;
      // recv_topic_p3 = recv_topic_p3 + "/p3";
      // client.subscribe(recv_topic_p3.c_str());
      // // subscibe the p4 light service
      // String recv_topic_p4 = ESP_HOST_NAME;
      // recv_topic_p4 = recv_topic_p4 + "/p4";
      // client.subscribe(recv_topic_p4.c_str());
      // // subscibe the p5 light service
      // String recv_topic_p5 = ESP_HOST_NAME;
      // recv_topic_p5 = recv_topic_p5 + "/p5";
      // client.subscribe(recv_topic_p5.c_str());
      // // subscibe the p6 light service
      // String recv_topic_p6 = ESP_HOST_NAME;
      // recv_topic_p6 = recv_topic_p6 + "/p6";
      // client.subscribe(recv_topic_p6.c_str());
      // // subscibe the p7 light service
      // String recv_topic_p7 = ESP_HOST_NAME;
      // recv_topic_p7 = recv_topic_p7 + "/p7";
      // client.subscribe(recv_topic_p7.c_str());

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

String getHeaderValue(String header, String headerName)
{
  return header.substring(strlen(headerName.c_str()));
}


void execOTA()
{
  Serial.println("Connecting to: www.polypite.com");

  int contentLength = 0;
  bool isValidContentType = false;
  // Connect to S3
  if (espClient.connect("www.polypite.com", 80))
  {
    // Connection Succeed.
    // Fecthing the bin
    Serial.println("Fetching Bin: /public/bin/firmware.bin");

    // Get the contents of the bin file
    espClient.print(String("GET ") + "/public/bin/firmware.bin" + " HTTP/1.1\r\n" +
                    "Host: " + "www.polypite.com" + "\r\n" +
                    "Cache-Control: no-cache\r\n" +
                    "Connection: close\r\n\r\n");

    // Check what is being sent
    //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
    //                 "Host: " + host + "\r\n" +
    //                 "Cache-Control: no-cache\r\n" +
    //                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (espClient.available() == 0)
    {
      if (millis() - timeout > 5000)
      {
        Serial.println("Client Timeout !");
        espClient.stop();
        return;
      }
    }
    // Once the response is available,
    // check stuff

    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3
                                   
        {{BIN FILE CONTENTS}}
 
    */
    while (espClient.available())
    {
      // read line till /n
      String line = espClient.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty,
      // this is end of headers
      // break the while and feed the
      // remaining `client` to the
      // Update.writeStream();
      if (!line.length())
      {
        //headers ended
        break; // and get the OTA started
      }

      // Check if the HTTP Response is 200
      // else break and Exit Update
      if (line.startsWith("HTTP/1.1"))
      {
        if (line.indexOf("200") < 0)
        {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }
      // extract headers here
      // Start with content length
      if (line.startsWith("Content-Length: "))
      {
        contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }
      // Next, the content type
      if (line.startsWith("Content-Type: "))
      {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream")
        {
          isValidContentType = true;
        }
      }
    }
  }
  else
  {
    // Connect to S3 failed
    // May be try?
    // Probably a choppy network?
    Serial.println("Connection to www.polypite.com failed. Please check your setup");
    // retry??
    // execOTA();
  }
  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));
  // check contentLength and content type
  if (contentLength && isValidContentType)
  {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);
    // If yes, begin
    if (canBegin)
    {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      size_t written = Update.writeStream(espClient);
      if (written == contentLength)
      {
        Serial.println("Written : " + String(written) + " successfully");
      }
      else
      {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
        // retry??
        // execOTA();
      }
      if (Update.end())
      {
        Serial.println("OTA done!");
        if (Update.isFinished())
        {
          Serial.println("Update successfully completed. Rebooting.");
          ESP.restart();
        }
        else
        {
          Serial.println("Update not finished? Something went wrong!");
        }
      }
      else
      {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    }
    else
    {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      espClient.flush();
    }
  }
  else
  {
    Serial.println("There was no content in the response");
    espClient.flush();
  }
}

bool execCheckVersion(String oldversion)
{
  String version = "0.50";
  bool needtoupdate = false;
  int err = http.get("/version");
  if(err == 0) {
    err = http.responseStatusCode();
    if(err >=0) {
      while(http.available() && !http.endOfBodyReached()) {
        String line = http.readStringUntil('\n');
        line.trim();
        if (line.startsWith("version:")) {
          line.replace("version:","");
          version = line;
        }
      }
    }
  }
  if(oldversion != version) {
    needtoupdate =  true;
  }
  return needtoupdate;
}

// void setup() {
//   Serial.begin(115200);
//   WiFi.begin("cg-livingroom","13501983117");
//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.print("."); // Keep the serial monitor lit!
//     delay(500);
//   }

//   execOTA();

// }

// esp32 init config
void setup()
{
  Serial.begin(115200);
  RESET_FLAG = false;
  IS_SMART = false;
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON), handleRestButtonChanged, CHANGE);
  WiFi.mode(WIFI_AP_STA);

  // Serial.print("the current version is: ");
  // Serial.println(VERSION_NUM);

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
    //deleteFile(SPIFFS,"/wifi.ini");

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
      cJSON *root = NULL;
      cJSON *item = NULL;

      if (SPIFFS.exists("/mid.ini"))
      {
        String midcontent;
        midcontent = getFileString(SPIFFS, "/mid.ini");
        const char *jsonstr = midcontent.c_str();
        root = cJSON_Parse(jsonstr);
        String itemstr;
        item = cJSON_GetObjectItem(root, "mid");
        itemstr = cJSON_Print(item);
        itemstr.replace("\"", "");
        ESP_HOST_NAME = itemstr;
        Serial.println("the host name is" + ESP_HOST_NAME);
        item = cJSON_GetObjectItem(root, "mac");
        itemstr = cJSON_Print(item);
        itemstr.replace("\"", "");
        ESP_MAC = itemstr;
      }

      Serial.println("the AP name is : " + String(ESP_HOST_NAME) + " password is: " + String(password));
      Serial.println("the mac address is: " + String(ESP_MAC));
      if (SPIFFS.exists("/wifi.ini"))
      {
        String filestr;
        filestr = getFileString(SPIFFS, "/wifi.ini");

        // cJSON *root = NULL;
        // cJSON *item = NULL;
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

        // cJSON_Delete(root);

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
          if (MDNS.begin(host))
          {
            Serial.println("MDNS responder started");
          }
          IS_SMART = true;
          Serial.println(WiFi.localIP());
          led8.set(100);
          client.setServer(mqttServer, mqttPort);
          client.setCallback(callback);
          mqttconn();

          String filestr = getFileString(SPIFFS, "/pwminfo.ini");
          const char *jsonstr = filestr.c_str();
          root = cJSON_Parse(jsonstr);
          String itemstr;
          item = cJSON_GetObjectItem(root, "version");
          itemstr = cJSON_Print(item);
          PWM_INFO_VERSION = itemstr;
          PWM_INFO_VERSION.replace("\"", "");         
        } else {
          WiFi.softAP(ESP_HOST_NAME.c_str(), password);
          led8.set(0);
          Serial.println("please set the wifi");
        }
      } else {
        WiFi.softAP(ESP_HOST_NAME.c_str(), password);
        led8.set(0);
        Serial.println("please set the wifi");
      }
      // online or offline light operation
      String strlvs = getFileString(SPIFFS, "/p.ini");
      
      lightopr(strlvs);
    }
  }

  SPIFFS.end();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "";
    html = html + "<html><head><meta http-equiv=\"Content-Type\"content=\"text/html; charset=utf-8\"/><meta http-equiv=\"X-UA-Compatible\"content=\"IE=edge,Chrome=1\"/><meta name=\"format-detection\" content=\"telephone=no\" /><meta http-equiv=\"pragma\"content=\"no-cache\"><meta http-equiv=\"cache-control\"content=\"no-cache\"><meta http-equiv=\"expires\"content=\"0\"><meta name=\"viewport\"content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\"><style>";
    html = html + ".w-header{height:50px;background-color:#eee;line-height:50px;border-bottom:1px solid #999}";
    html = html + ".w-header-brand{font-size:16px;font-weight:bold;float:left;margin-left:20px;}";
    html = html + ".w-body{clear:both}";
    html = html + ".w-body .w-menu{padding:5px 0px 0px 10px;margin:0;font-size:16px}";
    html = html + ".w-body .w-menu,.w-body .w-menu li{height:30px;list-style-type:none;text-align:center;line-height:30px}";
    html = html + ".w-body .w-menu li{width:80px;float:left}";
    html = html + ".w-body .w-menu li a{color: green;text-decoration:none}";
    html = html + ".w-body .w-menu li.divider{width:20px}";
    html = html + ".w-body .w-menu li.selected{background-color:#B91818}";
    html = html + ".w-body .w-menu li.selected a{color:#fff}";
    html = html + ".w-body iframe{padding:0;margin:0;border:0;overflow-x:hidden}";
    //html = html +  "</style><title>后台管理</title></head><body style=\"overflow:hidden\"><div class=\"w-header\"><div class=\"w-header-brand\">后台管理</div></div><div class=\"w-body\"><ul class=\"w-menu\"><li id=\"basic\"><a target=\"iframe\"href=\"/basic\">基本设置</a></li><li class=\"divider\">|</li><li id=\"light\"><a target=\"iframe\"href=\"/p\">亮度设置</a></li><li class=\"divider\">|</li><li id=\"basic\"><a target=\"iframe\"href=\"/upload\">固件升级</a></li></ul><div id='grid'></div><hr/><iframe name=\"iframe\"width=\"100%\"height=\"80%\"frameborder=\"no\"border=\"0\"scrolling=\"auto\"src=\"/basic\"></iframe></div><div id=\"w-footer\"></div></body></html>";
    html = html + "</style><title>后台管理</title></head><body style=\"overflow:hidden\"><div class=\"w-header\"><div class=\"w-header-brand\">后台管理</div></div><div class=\"w-body\"><ul class=\"w-menu\"><li id=\"basic\"><a target=\"iframe\"href=\"/basic\">基本设置</a></li><li class=\"divider\">|</li><li id=\"light\"><a target=\"iframe\"href=\"/p\">亮度设置</a></li><li class=\"divider\">|</li><li id=\"upload\"><a target=\"iframe\"href=\"/upload\">固件更新</a></li></ul><hr/><iframe name=\"iframe\"width=\"100%\"height=\"80%\"frameborder=\"no\"border=\"0\"scrolling=\"auto\"src=\"./basic\"></iframe></div><div id=\"w-footer\"></div></body></html>";

    request->send(200, "text/html", html);
  });

  // index page
  server.on("/basic", HTTP_GET, [](AsyncWebServerRequest *request) {
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


    // struct timeval stime;
    // stime.tv_sec = PWM_INFO_RTC.toInt() + 28816;
    // settimeofday(&stime, NULL);


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
    //html = html + "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>NodeMCU Control Page</title><!--<script type=\"text/javascript\"src=\"jquery.js\"></script>--></head><body><div><h1 id=\"title\">基本信息</h1><table><tr><th style=\"text-align:left;\">版本号</th></tr><tr><td><span id=\"spversion\"></span></td></tr><tr><th style=\"text-align:left;\">系统时间</th></tr><tr><td><span id=\"spCurrent\"></span></td></tr><tr><th style=\"text-align: left;\">修改时间</th></tr><tr><td><input type=\"text\"value=\"2018-03-03 00:00:00\"id=\"txtsysdate\"/>*时间格式:2018-03-03 00:00:00</td></tr><tr><th style=\"text-align:left;\">循环模式</th></tr><tr><td><input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>循环模式<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>固定模式</td></tr><tr id=\"thtest\"><th style=\"text-align:left;\">是否测试</th></tr><tr id=\"tdtest\"><td><input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>否<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>是</td></tr><tr id=\"thonline\"style=\"display: none;\"><th style=\"text-align:left;\">云端控制</th></tr><tr id=\"tdonline\"style=\"display: none;\"><td><input value=\"local\"type=\"radio\"name=\"onlineMode\"id=\"rdlocal\"/>否<input value=\"online\"type=\"radio\"name=\"onlineMode\"id=\"rdonline\"/>是</td></tr><tr><td><input type=\"button\"value=\"联网设置\"id=\"btnwifi\"onclick=\"wifi();\"/><input type=\"submit\"value=\"保存\"id=\"submit\"onclick=\"submit();\"/><input type=\"button\"value=\"恢复出厂\"id=\"btnstop\"onclick=\"init();\"/><input type=\"button\"value=\"重启\"id=\"btnReset\"onclick=\"reset();\"/><input type=\"button\"value=\"更新固件\"id=\"btnupload\"onclick=\"upload();\"/></td></tr></table><hr/><h1 id=\"title\">灯光控制</h1><table><tr><td><a href=\"/p\">进入设置页面</a></td></tr></table></div><script>function submit(){var selectshowtype=document.getElementsByName('showtype');var showtypevalue=\"\";for(var i=0;i<selectshowtype.length;i++){if(selectshowtype[i].checked){showtypevalue=selectshowtype[i].value;break}}var selecttestmode=document.getElementsByName('testMode');var testmodevalue=\"\";for(var i=0;i<selecttestmode.length;i++){if(selecttestmode[i].checked){testmodevalue=selecttestmode[i].value;break}}var str=document.getElementById('txtsysdate').value;str=str.replace(/-/g,\"/\");var date=new Date(str);var unixDate=date.getTime()/1000|0;console.log(unixDate);var selectedconnectionmode=document.getElementsByName(\"onlineMode\");var connectionmodevalue=\"\";for(var i=0;i<selectedconnectionmode.length;i++){if(selectedconnectionmode[i].checked){connectionmodevalue=selectedconnectionmode[i].value;break}}alert('保存成功');var url=\"pwmopr?showtype=\"+showtypevalue+\"&testmode=\"+testmodevalue+\"&sysdate=\"+unixDate+\"&conmode=\"+connectionmodevalue;window.location.href=url}function init(){alert('已恢复出厂设置!');var url=\"init\";window.location.href=url}function wifi(){var url=\"wifi\";window.location.href=url}function reset(){var url=\"reset\";alert(\"已重启,请关闭当前页面\");window.location.href=url}function upload(){var url=\"upload\";window.location.href=url}</script></body></html>";
    //html = html + "<!DOCTYPE html><html><head><meta name=\"format-detection\" content=\"telephone=no\" /><meta http-equiv=\"Content-Type\"content=\"text/html; charset=utf-8\"/><meta http-equiv=\"X-UA-Compatible\"content=\"IE=edge,Chrome=1\"/><meta http-equiv=\"pragma\"content=\"no-cache\"><!--HTTP 1.1--><meta http-equiv=\"cache-control\"content=\"no-cache\"><!--HTTP 1.0--><meta http-equiv=\"expires\"content=\"0\"><!--Prevent caching at the proxy server--><meta name=\"viewport\"content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\"><title>基本信息</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">版本号</span></td><td style=\"font-size:14px;\"><span id=\"spversion\"></span></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">系统时间</span></td><td style=\"font-size:14px;\"><span id=\"spCurrent\"></span></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">修改时间</span></td><td style=\"font-size:14px;\"><input type=\"text\"value=\"2018-12-20 00:00:00\"id=\"txtsysdate\"/></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">循环模式</span></td><td style=\"font-size:14px;\"><input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>循环模式<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>固定模式</td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">是否测试</span></td><td style=\"font-size:14px;\"><input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>否<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>是</td></tr><tr style=\"height:50px;\"><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"button\"value=\"联网设置\"id=\"btnwifi\"onclick=\"wifi();\"/><input style=\"background:green; color:white; width:80px; height:30px;margin-right:2px;\"type=\"submit\"value=\"保存\"id=\"submit\"onclick=\"submit();\"/><input style=\"background:green; color:white; width:80px; height:30px;margin-right:2px;\"type=\"button\"value=\"恢复出厂\"id=\"btnstop\"onclick=\"init();\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"重启\"id=\"btnReset\"onclick=\"reset();\"/></td></tr></table><hr/></div><script>var now=new Date();var year=now.getFullYear();var month=now.getMonth();var date=now.getDate();var day=now.getDay();var hour=now.getHours();var minu=now.getMinutes();var sec=now.getSeconds();month=month+1;if(month<10)month=\"0\"+month;if(date<10)date=\"0\"+date;if(hour<10)hour=\"0\"+hour;if(minu<10)minu=\"0\"+minu;if(sec<10)sec=\"0\"+sec;var time=\"\";time=year+\"-\"+month+\"-\"+date+\" \"+hour+\":\"+minu+\":\"+sec;document.getElementById(\"txtsysdate\").value=time;function submit(){var selectshowtype=document.getElementsByName('showtype');var showtypevalue=\"\";for(var i=0;i<selectshowtype.length;i++){if(selectshowtype[i].checked){showtypevalue=selectshowtype[i].value;break}}var selecttestmode=document.getElementsByName('testMode');var testmodevalue=\"\";for(var i=0;i<selecttestmode.length;i++){if(selecttestmode[i].checked){testmodevalue=selecttestmode[i].value;break}}var str=document.getElementById('txtsysdate').value;str=str.replace(/-/g,\"/\");var date=new Date(str);var unixDate=date.getTime()/1000|0;console.log(unixDate);alert('保存成功');var url=\"pwmopr?showtype=\"+showtypevalue+\"&testmode=\"+testmodevalue+\"&sysdate=\"+unixDate+\"&conmode=local\";window.location.href=url}function init(){alert('已恢复出厂设置!');var url=\"init\";window.location.href=url}function wifi(){var url=\"wifi\";window.location.href=url}function reset(){var url=\"reset\";alert(\"已重启,请关闭当前页面\");window.location.href=url}function upload(){var url=\"upload\";window.location.href=url}</script></body></html>";
    html = html + "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\"content=\"text/html; charset=utf-8\"/><meta http-equiv=\"X-UA-Compatible\"content=\"IE=edge,Chrome=1\"/><meta http-equiv=\"pragma\"content=\"no-cache\"><!--HTTP 1.1--><meta http-equiv=\"cache-control\"content=\"no-cache\"><!--HTTP 1.0--><meta http-equiv=\"expires\"content=\"0\"><!--Prevent caching at the proxy server--><meta name=\"viewport\"content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\"><title>基本信息</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">版本号</span></td><td style=\"font-size:14px;\"><span id=\"spversion\"></span></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">系统时间</span></td><td style=\"font-size:14px;\"><span id=\"spCurrent\"></span></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">修改时间</span></td><td style=\"font-size:14px;\"><input type=\"text\"value=\"2018-12-20 00:00:00\"id=\"txtsysdate\"/></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">循环模式</span></td><td style=\"font-size:14px;\"><input type=\"radio\"value=\"repeat\"name=\"showtype\"id=\"rdRpt\"/>循环模式<input type=\"radio\"value=\"fix\"name=\"showtype\"id=\"rdFix\"/>固定模式</td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">是否测试</span></td><td style=\"font-size:14px;\"><input value=\"production\"type=\"radio\"name=\"testMode\"id=\"rdPrd\"/>否<input value=\"test\"type=\"radio\"name=\"testMode\"id=\"rdTest\"/>是</td></tr><tr style=\"height:50px;\"><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"button\"value=\"联网设置\"id=\"btnwifi\"onclick=\"wifi();\"/><input style=\"background:green; color:white; width:80px; height:30px;margin-right:2px;\"type=\"submit\"value=\"保存\"id=\"submit\"onclick=\"submit();\"/><input style=\"background:green; color:white; width:80px; height:30px;margin-right:2px;\"type=\"button\"value=\"恢复出厂\"id=\"btnstop\"onclick=\"init();\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"重启\"id=\"btnReset\"onclick=\"reset();\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"二维码\"id=\"btnQrcode\"onclick=\"qrcode();\"/></td></tr></table><hr/></div><script>var now=new Date();var year=now.getFullYear();var month=now.getMonth();var date=now.getDate();var day=now.getDay();var hour=now.getHours();var minu=now.getMinutes();var sec=now.getSeconds();month=month+1;if(month<10)month=\"0\"+month;if(date<10)date=\"0\"+date;if(hour<10)hour=\"0\"+hour;if(minu<10)minu=\"0\"+minu;if(sec<10)sec=\"0\"+sec;var time=\"\";time=year+\"-\"+month+\"-\"+date+\" \"+hour+\":\"+minu+\":\"+sec;document.getElementById(\"txtsysdate\").value=time;function submit(){var selectshowtype=document.getElementsByName('showtype');var showtypevalue=\"\";for(var i=0;i<selectshowtype.length;i++){if(selectshowtype[i].checked){showtypevalue=selectshowtype[i].value;break}}var selecttestmode=document.getElementsByName('testMode');var testmodevalue=\"\";for(var i=0;i<selecttestmode.length;i++){if(selecttestmode[i].checked){testmodevalue=selecttestmode[i].value;break}}var str=document.getElementById('txtsysdate').value;str=str.replace(/-/g,\"/\");var date=new Date(str);var unixDate=date.getTime()/1000|0;console.log(unixDate);alert('保存成功');var url=\"pwmopr?showtype=\"+showtypevalue+\"&testmode=\"+testmodevalue+\"&sysdate=\"+unixDate+\"&conmode=local\";window.location.href=url}function init(){alert('已恢复出厂设置!');var url=\"init\";window.location.href=url}function wifi(){var url=\"wifi\";window.location.href=url}function reset(){var url=\"reset\";alert(\"已重启,请关闭当前页面\");window.location.href=url}function upload(){var url=\"upload\";window.location.href=url}function qrcode(){var url=\"mid\";window.location.href=url}function update(){var url=\"onlineupdate\";window.location.href=url}</script></body></html>";

    String tpl_currentdate = "<span id=\"spCurrent\"></span>";
    String change_currentdate = "<span id=\"spCurrent\">";


    // int currenthour = t_st->tm_hour;
    // int currentmin = t_st->tm_min;
    // int currentsec = t_st->tm_sec;
    //time_t t = time(NULL);
    // char nowtime[24];  
    // struct tm *t_st;
    // t_st = localtime(&t);
    // memset(nowtime, 0, sizeof(nowtime));
    // strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", t_st);
    // String strDate = nowtime;
    // //getEpochTime
    // Serial.println(PWM_INFO_RTC);
    time_t t;
    struct tm *p;
    t= atoi(PWM_INFO_RTC.c_str()) + 28800;
    p=gmtime(&t);
    char s[80];
    strftime(s, 80, "%Y-%m-%d %H:%M:%S", p);
    String strDate = s;

    change_currentdate = change_currentdate + strDate;
    change_currentdate = change_currentdate + "</span>";
    html.replace(tpl_currentdate, change_currentdate);

    String tpl_txtcurrentdate = "<input type=\"text\"value=\"2018-12-20 00:00:00\"id=\"txtsysdate\"/>";
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
        //String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>操作</title></head><body><h1><span id=\"sptitle\">亮度控制</span><span id=\"lightvalues\"style=\"display:none;\"></span></h1><Table><tr><td colspan=\"6\"><input type=\"radio\"name=\"line\"value=\"1\"checked onchange=\"linechange(1)\"/>第一排<input type=\"radio\"name=\"line\"value=\"2\"onchange=\"linechange(2)\"/>第二排<input type=\"radio\"name=\"line\"value=\"3\"onchange=\"linechange(3)\"/>第三排<input type=\"radio\"name=\"line\"value=\"4\"onchange=\"linechange(4)\"/>第四排<input type=\"radio\"name=\"line\"value=\"5\"onchange=\"linechange(5)\"/>第五排<input type=\"radio\"name=\"line\"value=\"6\"onchange=\"linechange(6)\"/>第六排<input type=\"radio\"name=\"line\"value=\"7\"onchange=\"linechange(7)\"/>第七排<input type=\"hidden\"id=\"hidline\"value=\"1\"/></td></tr><tr><th>0：00</th><th>0:10</th><th>0:20</th><th>0:30</th><th>0:40</th><th>0:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl000\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl001\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl002\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl003\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl004\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl005\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>1：00</th><th>1:10</th><th>1:20</th><th>1:30</th><th>1:40</th><th>1:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl010\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl011\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl012\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl013\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl014\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl015\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>2：00</th><th>2:10</th><th>2:20</th><th>2:30</th><th>2:40</th><th>2:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl020\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl021\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl022\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl023\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl024\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl025\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>3：00</th><th>3:10</th><th>3:20</th><th>3:30</th><th>3:40</th><th>3:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl030\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl031\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl032\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl033\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl034\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl035\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>4：00</th><th>4:10</th><th>4:20</th><th>4:30</th><th>4:40</th><th>4:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl040\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl041\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl042\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl043\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl044\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl045\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>5：00</th><th>5:10</th><th>5:20</th><th>5:30</th><th>5:40</th><th>5:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl050\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl051\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl052\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl053\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl054\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl055\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>6：00</th><th>6:10</th><th>6:20</th><th>6:30</th><th>6:40</th><th>6:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl060\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl061\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl062\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl063\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl064\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl065\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>7：00</th><th>7:10</th><th>7:20</th><th>7:30</th><th>7:40</th><th>7:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl070\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl071\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl072\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl073\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl074\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl075\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>8：00</th><th>8:10</th><th>8:20</th><th>8:30</th><th>8:40</th><th>8:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl080\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl081\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl082\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl083\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl084\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl085\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>9：00</th><th>9:10</th><th>9:20</th><th>9:30</th><th>9:40</th><th>9:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl090\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl091\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl092\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl093\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl094\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl095\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>10：00</th><th>10:10</th><th>10:20</th><th>10:30</th><th>10:40</th><th>10:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl100\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl101\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl102\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl103\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl104\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl105\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>11：00</th><th>11:10</th><th>11:20</th><th>11:30</th><th>11:40</th><th>11:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl110\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl111\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl112\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl113\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl114\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl115\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>12：00</th><th>12:10</th><th>12:20</th><th>12:30</th><th>12:40</th><th>12:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl120\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl121\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl122\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl123\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl124\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl125\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>13：00</th><th>13:10</th><th>13:20</th><th>13:30</th><th>13:40</th><th>13:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl130\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl131\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl132\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl133\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl134\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl135\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>14：00</th><th>14:10</th><th>14:20</th><th>14:30</th><th>14:40</th><th>14:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl140\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl141\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl142\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl143\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl144\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl145\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>15：00</th><th>15:10</th><th>15:20</th><th>15:30</th><th>15:40</th><th>15:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl150\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl151\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl152\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl153\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl154\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl155\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>16：00</th><th>16:10</th><th>16:20</th><th>16:30</th><th>16:40</th><th>16:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl160\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl161\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl162\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl163\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl164\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl165\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>17：00</th><th>17:10</th><th>17:20</th><th>17:30</th><th>17:40</th><th>17:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl170\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl171\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl172\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl173\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl174\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl175\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>18：00</th><th>18:10</th><th>18:20</th><th>18:30</th><th>18:40</th><th>18:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl180\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl181\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl182\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl183\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl184\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl185\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>19：00</th><th>19:10</th><th>19:20</th><th>19:30</th><th>19:40</th><th>19:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl190\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl191\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl192\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl193\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl194\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl195\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>20：00</th><th>20:10</th><th>20:20</th><th>20:30</th><th>20:40</th><th>20:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl200\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl201\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl202\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl203\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl204\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl205\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>21：00</th><th>21:10</th><th>21:20</th><th>21:30</th><th>21:40</th><th>21:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl210\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl211\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl212\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl213\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl214\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl215\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>22：00</th><th>22:10</th><th>22:20</th><th>22:30</th><th>22:40</th><th>22:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl220\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl221\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl222\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl223\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl224\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl225\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>23：00</th><th>23:10</th><th>23:20</th><th>23:30</th><th>23:40</th><th>23:50</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtl230\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl231\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl232\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl233\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl234\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"width=\"50\"id=\"txtl235\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><th>固定</th></tr><tr><td><input type=\"text\"width=\"50\"id=\"txtlfix\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td><input onclick=\"submit();\"type=\"submit\"id=\"submit\"value=\"保存\"/><input onclick=\"back();\"type=\"button\"id=\"btnback\"value=\"返回\"/></td></tr><tr><td sylte=\"color:red\">取值范围0~255</td></tr></Table></body><script language=\"javascript\">function submit(){var savedata=document.getElementById(\"lightvalues\").innerHTML;var url=\"setp?t=\"+savedata;console.log(url);alert('保存成功');window.location.href=url}function tohex(str){if(str==\"0\"){return\"00\"}else{var intval=parseInt(str);if(intval){var hexval=intval.toString(16);if(hexval.length==1){var res=\"0\";return res+hexval}else{return hexval}}else{return\"00\"}}}function back(){var url=\"/\";window.location.href=url}function setlinevalue(pnum,lvs){var startIndex;switch(pnum){case 1:startIndex=0;break;case 2:startIndex=2;break;case 3:startIndex=4;break;case 4:startIndex=6;break;case 5:startIndex=8;break;case 6:startIndex=10;break;case 7:startIndex=12;break;default:break}for(var key in lvs){var resval=lvs[key];resval=resval.substr(startIndex,2);var res=parseInt(resval,16);var cid=key.replace(\"t\",\"\");cid=\"txtl\"+cid;document.getElementById(cid).value=res}}function linechange(val){document.getElementById(\"hidline\").value=val;var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(val,lvs)}function checkinput(input){var res=parseInt(input.value);if(res){if(res<0||res>255){document.getElementById(input.id).value=\"0\";alert('输入有误')}}else{document.getElementById(input.id).value=\"0\";alert('输入有误')}}function changevalue(input){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);var cid=input.id;var contentarr=tohex(input.value).split('');var tid=cid.replace(\"txtl\",\"t\");var line=parseInt(document.getElementById(\"hidline\").value);var arrcontent=lvs[tid].split('');var startIndex,endIndex;switch(line){case 1:startIndex=0;endIndex=1;break;case 2:startIndex=2;endIndex=3;break;case 3:startIndex=4;endIndex=5;break;case 4:startIndex=6;endIndex=7;break;case 5:startIndex=8;endIndex=9;break;case 6:startIndex=10;endIndex=11;break;case 7:startIndex=12;endIndex=13;break;default:break}arrcontent[startIndex]=contentarr[0];arrcontent[endIndex]=contentarr[1];lvs[tid]=arrcontent.join('');var updstr=JSON.stringify(lvs);document.getElementById('lightvalues').innerHTML=updstr}window.onload=function(){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(1,lvs)}</script></html>";
        String rawhtml = "<html><head><meta charset=\"UTF-8\"><meta name=\"format-detection\" content=\"telephone=no\" /><title>操作</title><style>";
        rawhtml = rawhtml + "body{font-size:14px}";
        rawhtml = rawhtml + "table tr{text-align:center;height:25px}";
        rawhtml = rawhtml + ".inputnumber{width:50px;background:transparent;border:0px;border-bottom:1px solid green;text-align:center}";
        rawhtml = rawhtml + ".buttonstyle{background:green;color:white;width:80px;height:30px}";
        //rawhtml = rawhtml + "</style></head><body><span id=\"lightvalues\"style=\"display:none;\"></span><div style=\"height: 40px;\"><input type=\"radio\"name=\"line\"value=\"1\"checked onchange=\"linechange(1)\"/>第一排<input type=\"radio\"name=\"line\"value=\"2\"onchange=\"linechange(2)\"/>第二排<input type=\"radio\"name=\"line\"value=\"3\"onchange=\"linechange(3)\"/>第三排<input type=\"radio\"name=\"line\"value=\"4\"onchange=\"linechange(4)\"/>第四排<input type=\"radio\"name=\"line\"value=\"5\"onchange=\"linechange(5)\"/>第五排<input type=\"radio\"name=\"line\"value=\"6\"onchange=\"linechange(6)\"/>第六排<input type=\"radio\"name=\"line\"value=\"7\"onchange=\"linechange(7)\"/>第七排<input type=\"hidden\"id=\"hidline\"value=\"1\"/></div><Table><tr><td>0：00</td><td>0:10</td><td>0:20</td><td>0:30</td><td>0:40</td><td>0:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl000\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl001\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl002\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl003\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl004\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl005\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>1：00</td><td>1:10</td><td>1:20</td><td>1:30</td><td>1:40</td><td>1:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl010\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl011\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl012\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl013\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl014\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl015\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>2：00</td><td>2:10</td><td>2:20</td><td>2:30</td><td>2:40</td><td>2:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl020\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl021\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl022\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl023\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl024\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl025\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>3：00</td><td>3:10</td><td>3:20</td><td>3:30</td><td>3:40</td><td>3:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl030\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl031\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl032\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl033\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl034\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl035\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>4：00</td><td>4:10</td><td>4:20</td><td>4:30</td><td>4:40</td><td>4:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl040\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl041\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl042\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl043\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl044\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl045\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>5：00</td><td>5:10</td><td>5:20</td><td>5:30</td><td>5:40</td><td>5:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl050\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl051\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl052\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl053\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl054\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl055\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>6：00</td><td>6:10</td><td>6:20</td><td>6:30</td><td>6:40</td><td>6:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl060\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl061\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl062\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl063\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl064\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl065\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>7：00</td><td>7:10</td><td>7:20</td><td>7:30</td><td>7:40</td><td>7:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl070\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl071\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl072\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl073\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl074\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl075\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>8：00</td><td>8:10</td><td>8:20</td><td>8:30</td><td>8:40</td><td>8:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl080\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl081\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl082\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl083\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl084\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl085\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>9：00</td><td>9:10</td><td>9:20</td><td>9:30</td><td>9:40</td><td>9:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl090\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl091\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl092\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl093\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl094\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl095\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>10：00</td><td>10:10</td><td>10:20</td><td>10:30</td><td>10:40</td><td>10:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl100\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl101\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl102\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl103\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl104\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl105\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>11：00</td><td>11:10</td><td>11:20</td><td>11:30</td><td>11:40</td><td>11:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl110\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl111\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl112\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl113\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl114\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl115\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>12：00</td><td>12:10</td><td>12:20</td><td>12:30</td><td>12:40</td><td>12:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl120\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl121\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl122\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl123\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl124\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl125\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>13：00</td><td>13:10</td><td>13:20</td><td>13:30</td><td>13:40</td><td>13:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl130\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl131\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl132\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl133\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl134\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl135\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>14：00</td><td>14:10</td><td>14:20</td><td>14:30</td><td>14:40</td><td>14:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl140\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl141\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl142\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl143\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl144\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl145\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>15：00</td><td>15:10</td><td>15:20</td><td>15:30</td><td>15:40</td><td>15:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl150\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl151\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl152\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl153\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl154\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl155\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>16：00</td><td>16:10</td><td>16:20</td><td>16:30</td><td>16:40</td><td>16:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl160\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl161\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl162\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl163\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl164\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl165\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>17：00</td><td>17:10</td><td>17:20</td><td>17:30</td><td>17:40</td><td>17:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl170\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl171\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl172\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl173\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl174\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl175\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>18：00</td><td>18:10</td><td>18:20</td><td>18:30</td><td>18:40</td><td>18:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl180\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl181\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl182\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl183\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl184\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl185\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>19：00</td><td>19:10</td><td>19:20</td><td>19:30</td><td>19:40</td><td>19:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl190\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl191\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl192\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl193\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl194\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl195\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>20：00</td><td>20:10</td><td>20:20</td><td>20:30</td><td>20:40</td><td>20:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl200\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl201\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl202\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl203\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl204\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl205\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>21：00</td><td>21:10</td><td>21:20</td><td>21:30</td><td>21:40</td><td>21:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl210\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl211\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl212\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl213\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl214\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl215\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>22：00</td><td>22:10</td><td>22:20</td><td>22:30</td><td>22:40</td><td>22:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl220\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl221\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl222\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl223\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl224\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl225\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>23：00</td><td>23:10</td><td>23:20</td><td>23:30</td><td>23:40</td><td>23:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl230\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl231\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl232\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl233\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl234\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl235\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td colspan=\"6\">固定</td></tr><tr><td colspan=\"6\"><input type=\"text\"class=\"inputnumber\"id=\"txtlfix\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td colspan=\"6\"><input onclick=\"submit();\"type=\"submit\"id=\"submit\"value=\"保存\"/><input onclick=\"back();\"type=\"button\"id=\"btnback\"value=\"返回\"/></td></tr><tr><td sylte=\"color:red\"colspan=\"6\">取值范围0~100</td></tr></Table></body><script language=\"javascript\">function submit(){var savedata=document.getElementById(\"lightvalues\").innerHTML;var url=\"setp?t=\"+savedata;console.log(url);alert('保存成功');window.location.href=url}function tohex(str){if(str==\"0\"){return\"00\"}else{var intval=parseInt(str);if(intval){var hexval=intval.toString(16);if(hexval.length==1){var res=\"0\";return res+hexval}else{return hexval}}else{return\"00\"}}}function back(){var url=\"/\";window.location.href=url}function setlinevalue(pnum,lvs){var startIndex;switch(pnum){case 1:startIndex=0;break;case 2:startIndex=2;break;case 3:startIndex=4;break;case 4:startIndex=6;break;case 5:startIndex=8;break;case 6:startIndex=10;break;case 7:startIndex=12;break;default:break}for(var key in lvs){var resval=lvs[key];resval=resval.substr(startIndex,2);var res=parseInt(resval,16);var cid=key.replace(\"t\",\"\");cid=\"txtl\"+cid;document.getElementById(cid).value=res}}function linechange(val){document.getElementById(\"hidline\").value=val;var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(val,lvs)}function checkinput(input){var res=parseInt(input.value);if(res){if(res<0||res>100){document.getElementById(input.id).value=\"0\";alert('输入有误')}}else{document.getElementById(input.id).value=\"0\";alert('输入有误')}}function changevalue(input){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);var cid=input.id;var contentarr=tohex(input.value).split('');var tid=cid.replace(\"txtl\",\"t\");var line=parseInt(document.getElementById(\"hidline\").value);var arrcontent=lvs[tid].split('');var startIndex,endIndex;switch(line){case 1:startIndex=0;endIndex=1;break;case 2:startIndex=2;endIndex=3;break;case 3:startIndex=4;endIndex=5;break;case 4:startIndex=6;endIndex=7;break;case 5:startIndex=8;endIndex=9;break;case 6:startIndex=10;endIndex=11;break;case 7:startIndex=12;endIndex=13;break;default:break}arrcontent[startIndex]=contentarr[0];arrcontent[endIndex]=contentarr[1];lvs[tid]=arrcontent.join('');var updstr=JSON.stringify(lvs);document.getElementById('lightvalues').innerHTML=updstr}window.onload=function(){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(1,lvs)}</script></html>";
        rawhtml = rawhtml + "</style></head><body><span id=\"lightvalues\"style=\"display:none;\"></span><div style=\"height: 40px;\"><input type=\"radio\"name=\"line\"value=\"1\"checked onchange=\"linechange(1)\"/>第一排<input type=\"radio\"name=\"line\"value=\"2\"onchange=\"linechange(2)\"/>第二排<input type=\"radio\"name=\"line\"value=\"3\"onchange=\"linechange(3)\"/>第三排<input type=\"radio\"name=\"line\"value=\"4\"onchange=\"linechange(4)\"/>第四排<input type=\"radio\"name=\"line\"value=\"5\"onchange=\"linechange(5)\"/>第五排<input type=\"radio\"name=\"line\"value=\"6\"onchange=\"linechange(6)\"/>第六排<input type=\"radio\"name=\"line\"value=\"7\"onchange=\"linechange(7)\"/>第七排<input type=\"radio\"name=\"line\"value=\"8\"onchange=\"linechange(8)\"/>第八排<input type=\"hidden\"id=\"hidline\"value=\"1\"/></div><Table><tr><td>0：00</td><td>0:10</td><td>0:20</td><td>0:30</td><td>0:40</td><td>0:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl000\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl001\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl002\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl003\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl004\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl005\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>1：00</td><td>1:10</td><td>1:20</td><td>1:30</td><td>1:40</td><td>1:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl010\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl011\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl012\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl013\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl014\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl015\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>2：00</td><td>2:10</td><td>2:20</td><td>2:30</td><td>2:40</td><td>2:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl020\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl021\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl022\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl023\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl024\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl025\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>3：00</td><td>3:10</td><td>3:20</td><td>3:30</td><td>3:40</td><td>3:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl030\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl031\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl032\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl033\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl034\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl035\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>4：00</td><td>4:10</td><td>4:20</td><td>4:30</td><td>4:40</td><td>4:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl040\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl041\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl042\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl043\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl044\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl045\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>5：00</td><td>5:10</td><td>5:20</td><td>5:30</td><td>5:40</td><td>5:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl050\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl051\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl052\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl053\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl054\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl055\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>6：00</td><td>6:10</td><td>6:20</td><td>6:30</td><td>6:40</td><td>6:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl060\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl061\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl062\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl063\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl064\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl065\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>7：00</td><td>7:10</td><td>7:20</td><td>7:30</td><td>7:40</td><td>7:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl070\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl071\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl072\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl073\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl074\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl075\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>8：00</td><td>8:10</td><td>8:20</td><td>8:30</td><td>8:40</td><td>8:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl080\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl081\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl082\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl083\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl084\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl085\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>9：00</td><td>9:10</td><td>9:20</td><td>9:30</td><td>9:40</td><td>9:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl090\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl091\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl092\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl093\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl094\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl095\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>10：00</td><td>10:10</td><td>10:20</td><td>10:30</td><td>10:40</td><td>10:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl100\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl101\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl102\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl103\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl104\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl105\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>11：00</td><td>11:10</td><td>11:20</td><td>11:30</td><td>11:40</td><td>11:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl110\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl111\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl112\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl113\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl114\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl115\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>12：00</td><td>12:10</td><td>12:20</td><td>12:30</td><td>12:40</td><td>12:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl120\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl121\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl122\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl123\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl124\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl125\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>13：00</td><td>13:10</td><td>13:20</td><td>13:30</td><td>13:40</td><td>13:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl130\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl131\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl132\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl133\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl134\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl135\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>14：00</td><td>14:10</td><td>14:20</td><td>14:30</td><td>14:40</td><td>14:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl140\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl141\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl142\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl143\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl144\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl145\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>15：00</td><td>15:10</td><td>15:20</td><td>15:30</td><td>15:40</td><td>15:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl150\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl151\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl152\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl153\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl154\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl155\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>16：00</td><td>16:10</td><td>16:20</td><td>16:30</td><td>16:40</td><td>16:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl160\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl161\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl162\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl163\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl164\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl165\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>17：00</td><td>17:10</td><td>17:20</td><td>17:30</td><td>17:40</td><td>17:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl170\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl171\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl172\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl173\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl174\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl175\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>18：00</td><td>18:10</td><td>18:20</td><td>18:30</td><td>18:40</td><td>18:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl180\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl181\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl182\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl183\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl184\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl185\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>19：00</td><td>19:10</td><td>19:20</td><td>19:30</td><td>19:40</td><td>19:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl190\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl191\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl192\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl193\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl194\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl195\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>20：00</td><td>20:10</td><td>20:20</td><td>20:30</td><td>20:40</td><td>20:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl200\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl201\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl202\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl203\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl204\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl205\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>21：00</td><td>21:10</td><td>21:20</td><td>21:30</td><td>21:40</td><td>21:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl210\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl211\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl212\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl213\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl214\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl215\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>22：00</td><td>22:10</td><td>22:20</td><td>22:30</td><td>22:40</td><td>22:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl220\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl221\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl222\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl223\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl224\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl225\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td>23：00</td><td>23:10</td><td>23:20</td><td>23:30</td><td>23:40</td><td>23:50</td></tr><tr><td><input type=\"text\"class=\"inputnumber\"id=\"txtl230\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl231\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl232\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl233\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl234\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td><td><input type=\"text\"class=\"inputnumber\"id=\"txtl235\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td colspan=\"6\">固定</td></tr><tr><td colspan=\"6\"><input type=\"text\"class=\"inputnumber\"id=\"txtlfix\"value='0'oninput=\"checkinput(this)\"onchange=\"changevalue(this)\"/></td></tr><tr><td colspan=\"6\"><input onclick=\"submit();\"type=\"submit\"id=\"submit\"value=\"保存\"/><input onclick=\"back();\"type=\"button\"id=\"btnback\"value=\"返回\"/></td></tr><tr><td sylte=\"color:red\"colspan=\"6\">取值范围0~100</td></tr></Table></body><script language=\"javascript\">function submit(){var savedata=document.getElementById(\"lightvalues\").innerHTML;var url=\"setp?t=\"+savedata;console.log(url);alert('保存成功');window.location.href=url}function tohex(str){if(str==\"0\"){return\"00\"}else{var intval=parseInt(str);if(intval){var hexval=intval.toString(16);if(hexval.length==1){var res=\"0\";return res+hexval}else{return hexval}}else{return\"00\"}}}function back(){var url=\"/\";window.location.href=url}function setlinevalue(pnum,lvs){var startIndex;switch(pnum){case 1:startIndex=0;break;case 2:startIndex=2;break;case 3:startIndex=4;break;case 4:startIndex=6;break;case 5:startIndex=8;break;case 6:startIndex=10;break;case 7:startIndex=12;break;case 8:startIndex=14;break;default:break}for(var key in lvs){var resval=lvs[key];resval=resval.substr(startIndex,2);var res=parseInt(resval,16);var cid=key.replace(\"t\",\"\");cid=\"txtl\"+cid;document.getElementById(cid).value=res}}function linechange(val){document.getElementById(\"hidline\").value=val;var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(val,lvs)}function checkinput(input){var res=parseInt(input.value);if(res){if(res<0||res>100){document.getElementById(input.id).value=\"0\";alert('输入有误')}}else{document.getElementById(input.id).value=\"0\";alert('输入有误')}}function changevalue(input){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);var cid=input.id;var contentarr=tohex(input.value).split('');var tid=cid.replace(\"txtl\",\"t\");var line=parseInt(document.getElementById(\"hidline\").value);var arrcontent=lvs[tid].split('');var startIndex,endIndex;switch(line){case 1:startIndex=0;endIndex=1;break;case 2:startIndex=2;endIndex=3;break;case 3:startIndex=4;endIndex=5;break;case 4:startIndex=6;endIndex=7;break;case 5:startIndex=8;endIndex=9;break;case 6:startIndex=10;endIndex=11;break;case 7:startIndex=12;endIndex=13;break;case 8:startIndex=14;endIndex=15;default:break}arrcontent[startIndex]=contentarr[0];arrcontent[endIndex]=contentarr[1];lvs[tid]=arrcontent.join('');var updstr=JSON.stringify(lvs);document.getElementById('lightvalues').innerHTML=updstr}window.onload=function(){var lvs=JSON.parse(document.getElementById(\"lightvalues\").innerHTML);setlinevalue(1,lvs)}</script></html>";

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
      //conmode = request->getParam("conmode")->value();

      Serial.println("the show type is: " + showtype + " and the testmode is: " + testmode + " and sysdate is: " + sysdate);

      PWM_INFO_SHOWTYPE = showtype;
      PWM_INFO_TESTMODE = testmode;
      PWM_INFO_CONMODE = "local";

      struct timeval stime;
      if (WiFi.status() != WL_CONNECTED) { 
        stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 28800;
        settimeofday(&stime, NULL);
      } else {
        timeClient.update();        
        stime.tv_sec = timeClient.getEpochTime();        
        settimeofday(&stime, NULL);
        sysdate = timeClient.getEpochTime() - 28800;
      }

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

      writeFile(SPIFFS, "/pwminfo.ini", filecontent.c_str());
      SPIFFS.end();
    }
    request->redirect("/basic");
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
    request->redirect("/basic");
  });

  // change mid & qrcode
  server.on("/mid", HTTP_GET, [](AsyncWebServerRequest *request) {
    String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>QRCode</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">设备名:</span></td><td style=\"font-size:14px;\"><input type=\"text\"id=\"txtmid\"onchange=\"midchange()\"/></td></tr><tr><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"submit\"value=\"修改\"id=\"btnModify\"onclick=\"submit()\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/><span style=\"color:red;\">修改完设备名后wifi热点名也会随之改变,需要重新选择wifi热点并刷新页面</span></td></tr></table><hr/><div style=\"font-size:14px;\"id=\"divqrcode\"><span>扫描下方二维码进行绑定,如改名需要重新绑定</span><span style=\"color:red;\">需要联网</span><div id=\"code\"></div></div></div><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script><script src='https://cdn.bootcss.com/jquery.qrcode/1.0/jquery.qrcode.min.js'></script><script>function toUtf8(str){var out,i,len,c;out=\"\";len=str.length;for(i=0;i<len;i++){c=str.charCodeAt(i);if((c>=0x0001)&&(c<=0x007F)){out+=str.charAt(i)}else if(c>0x07FF){out+=String.fromCharCode(0xE0|((c>>12)&0x0F));out+=String.fromCharCode(0x80|((c>>6)&0x3F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}else{out+=String.fromCharCode(0xC0|((c>>6)&0x1F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}}return out}function submit(){var mid=document.getElementById('txtmid').value;var url=\"cmid?mid=\"+mid;window.location.href=url}function midchange(){var mid=document.getElementById('txtmid').value;var qrcode=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').empty();$('#code').qrcode(qrcode)}$(\"#divqrcode\").show();var mid=document.getElementById('txtmid').value;var str=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').qrcode(str);$(\"#offline\").hide();</script></body></html>";

    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }

    if (SPIFFS.exists("/mid.ini"))
    {
      String filestr;
      filestr = getFileString(SPIFFS, "/mid.ini");
      cJSON *root = NULL;
      cJSON *item = NULL;
      const char *jsonstr = filestr.c_str();
      root = cJSON_Parse(jsonstr);
      String itemstr;
      item = cJSON_GetObjectItem(root, "mid");
      itemstr = cJSON_Print(item);
      String mid = itemstr;
      mid.replace("\"", "");
      String tpl_mid = "<input type=\"text\"id=\"txtmid\"onchange=\"midchange()\"/>";
      String change_mid = "<input type=\"text\"id=\"txtmid\"onchange=\"midchange()\" value=\"";
      change_mid = change_mid + mid;
      change_mid = change_mid + "\"/>";
      Serial.println(change_mid);
      rawhtml.replace(tpl_mid, change_mid);
    }
    SPIFFS.end();
    request->send(200, "text/html", rawhtml);
  });

  // change mid handle
  server.on("/cmid", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mid"))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      String mid = request->getParam("mid")->value();
      String filecontent;
      filecontent = filecontent + "{\"mid\":\"";
      filecontent = filecontent + mid;
      filecontent = filecontent + "\",\"mac\":\"";
      filecontent = filecontent + ESP_MAC.c_str();
      filecontent = filecontent + "\"}";
      writeFile(SPIFFS, "/mid.ini", filecontent.c_str());
      SPIFFS.end();
      Serial.println("Begin to reboot!");
      ESP.restart();
    }
    else
    {
      Serial.println("param error");
    }
    request->redirect("/mid");
  });

  // wifi page
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    //String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>WiFi Setup Page</title></head><body><div><h1 id=\"title\">联网设置</h1><table><tr><th style=\"text-align:left;\">WIFI名称</th></tr><tr><td><input type=\"text\"id=\"txtssid\"/></td></tr><tr><th style=\"text-align:left;\">WIFI密码</th></tr><tr><td><input type=\"text\"id=\"txtpwd\"/></td></tr><tr><td><input type=\"submit\"value=\"连接\"id=\"btnConnect\"onclick=\"submit()\"/><input type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/></td></tr><tr><td><span id=\"spResult\"></span></td></tr></table><hr/><h1 id=\"title\">WIFI热点扫描</h1><table id=\"wifilist\"></table></div><script>function submit(){var ssid=document.getElementById('txtssid').value;var pwd=document.getElementById('txtpwd').value;alert(\"正在连接热点,请关闭此页面并切换选择的热点，通过http://esp32访问\");var url=\"savewifi?ssid=\"+ssid+\"&pwd=\"+pwd;window.location.href=url}function back(){var url=\"/\";window.location.href=url}</script></body></html>";
    String rawhtml = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>WiFi Setup Page</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">WIFI名称</span></td><td style=\"font-size:14px;\"><input type=\"text\"id=\"txtssid\"/></td></tr><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">WIFI密码</span></td><td style=\"font-size:14px;\"><input type=\"text\"id=\"txtpwd\"/></td></tr><tr><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"submit\"value=\"连接\"id=\"btnConnect\"onclick=\"submit()\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/></td></tr><tr><td><span id=\"spResult\"style=\"font-size:14px;\"></span></td></tr></table><hr/><div style=\"font-size:14px;\"><span></span><div><table id=\"wifilist\"></table></div></div></div><script>function submit(){var ssid=document.getElementById('txtssid').value;var pwd=document.getElementById('txtpwd').value;alert(\"正在连接热点,请关闭此页面并切换选择的热点，通过http://esp32访问\");var url=\"savewifi?ssid=\"+ssid+\"&pwd=\"+pwd;window.location.href=url}function back(){var url=\"/basic\";window.location.href=url}</script></body></html>";

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

    // String tpl_scanresult = "<table id=\"wifilist\"></table>";
    // String change_scanresult = "";
    // int n = WiFi.scanNetworks();
    // if (n == 0)
    // {
    //   change_scanresult = "<table id=\"wifilist\"><tr><td>wifi not found</td></tr></table>";
    //   rawhtml.replace(tpl_scanresult, change_scanresult);
    // }
    // else
    // {
    //   String change_scanresult = "<table id=\"wifilist\">";
    //   for (int i = 0; i < n; ++i)
    //   {
    //     change_scanresult = change_scanresult + "<tr><td>";
    //     change_scanresult = change_scanresult + WiFi.SSID(i);
    //     change_scanresult = change_scanresult + "</td></tr>";
    //   }
    //   change_scanresult = change_scanresult + "</table>";
    //   rawhtml.replace(tpl_scanresult, change_scanresult);
    // }

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
    request->redirect("/basic");
  });

  // reset operation
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Start to reset....");
    ESP.restart();
  });

  // bin upload page
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
    //String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Update System</title><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script></head><body><div><h1 id=\"title\">更新固件</h1><hr/><form method='POST'action='#'enctype='multipart/form-data'id='upload_form'><input type='file'name='update'/><input type='submit'value='更新'/></form><div id='prg'>progress:0%</div></div><script>$('form').submit(function(e){e.preventDefault();var form=$('#upload_form')[0];var data=new FormData(form);$.ajax({url:'/update',type:'POST',data:data,contentType:false,processData:false,xhr:function(){var xhr=new window.XMLHttpRequest();xhr.upload.addEventListener('progress',function(evt){if(evt.lengthComputable){var per=evt.loaded/evt.total;$('#prg').html('progress: '+Math.round(per*100)+'%')}},false);return xhr},success:function(d,s){console.log('success!')},error:function(a,b,c){}})});</script></body></html>";
    String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Update System</title><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script></head><body><div><form method='POST'action='#'enctype='multipart/form-data'id='upload_form'><input type='file'name='update'/><input style=\"background:green; color:white; width:80px; height:30px;\"type='submit'value='更新'/></form><div id='prg'style=\"font-size:14px; margin-top:10px;\">上传进步:0%</div></div><script>$('form').submit(function(e){e.preventDefault();var form=$('#upload_form')[0];var data=new FormData(form);$.ajax({url:'/update',type:'POST',data:data,contentType:false,processData:false,xhr:function(){var xhr=new window.XMLHttpRequest();xhr.upload.addEventListener('progress',function(evt){if(evt.lengthComputable){var per=evt.loaded/evt.total;$('#prg').html('progress: '+Math.round(per*100)+'%')}},false);return xhr},success:function(d,s){console.log('success!')},error:function(a,b,c){}})});</script></body></html>";
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

  server.on("/onlineupdate", HTTP_GET, [](AsyncWebServerRequest *request) {
    execOTA();
  });

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


  // Serial.println(timeClient.getFormattedTime());
  // Serial.print(timeClient.getHours());
  // Serial.print(":");
  // Serial.print(timeClient.getMinutes());
  // Serial.print(":");
  // Serial.print(timeClient.getSeconds());
  // Serial.println("..");


  
  int currenthour;
  int currentmin;
  int currentsec;

  String wifistatus = "online";

  if (WiFi.status() != WL_CONNECTED) {
    time_t t = time(NULL);
    struct tm *t_st;
    t_st = localtime(&t);
    currenthour = t_st->tm_hour;
    currentmin = t_st->tm_min;
    currentsec = t_st->tm_sec;
    wifistatus = "offline";
  } else {
    timeClient.update();
    currenthour = timeClient.getHours();
    currentmin = timeClient.getMinutes();
    currentsec = timeClient.getSeconds();
    wifistatus = "online";
  }

  // Serial.print(currenthour);
  // Serial.print(":");
  // Serial.print(currentmin);
  // Serial.print(":");
  // Serial.print(currentsec);
  // Serial.println(wifistatus);
  // Serial.println(timeClient.getEpochTime());
  

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
        Serial.printf("product mode .... current hour is: %d:%d:%d \r\n", currenthour,currentmin,currentsec);
        if (currenthour == 0)
        {
          if (currentmin <10)
          {
            led1.set(P1[0]);
            led2.set(P2[0]);
            led3.set(P3[0]);
            led4.set(P4[0]);
            led5.set(P5[0]);
            led6.set(P6[0]);
            led7.set(P7[0]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[1]);
            led2.set(P2[1]);
            led3.set(P3[1]);
            led4.set(P4[1]);
            led5.set(P5[1]);
            led6.set(P6[1]);
            led7.set(P7[1]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[2]);
            led2.set(P2[2]);
            led3.set(P3[2]);
            led4.set(P4[2]);
            led5.set(P5[2]);
            led6.set(P6[2]);
            led7.set(P7[2]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[3]);
            led2.set(P2[3]);
            led3.set(P3[3]);
            led4.set(P4[3]);
            led5.set(P5[3]);
            led6.set(P6[3]);
            led7.set(P7[3]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[4]);
            led2.set(P2[4]);
            led3.set(P3[4]);
            led4.set(P4[4]);
            led5.set(P5[4]);
            led6.set(P6[4]);
            led7.set(P7[4]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[6]);
            led2.set(P2[6]);
            led3.set(P3[6]);
            led4.set(P4[6]);
            led5.set(P5[6]);
            led6.set(P6[6]);
            led7.set(P7[6]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[7]);
            led2.set(P2[7]);
            led3.set(P3[7]);
            led4.set(P4[7]);
            led5.set(P5[7]);
            led6.set(P6[7]);
            led7.set(P7[7]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[8]);
            led2.set(P2[8]);
            led3.set(P3[8]);
            led4.set(P4[8]);
            led5.set(P5[8]);
            led6.set(P6[8]);
            led7.set(P7[8]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[9]);
            led2.set(P2[9]);
            led3.set(P3[9]);
            led4.set(P4[9]);
            led5.set(P5[9]);
            led6.set(P6[9]);
            led7.set(P7[9]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[10]);
            led2.set(P2[10]);
            led3.set(P3[10]);
            led4.set(P4[10]);
            led5.set(P5[10]);
            led6.set(P6[10]);
            led7.set(P7[10]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[12]);
            led2.set(P2[12]);
            led3.set(P3[12]);
            led4.set(P4[12]);
            led5.set(P5[12]);
            led6.set(P6[12]);
            led7.set(P7[12]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[13]);
            led2.set(P2[13]);
            led3.set(P3[13]);
            led4.set(P4[13]);
            led5.set(P5[13]);
            led6.set(P6[13]);
            led7.set(P7[13]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[14]);
            led2.set(P2[14]);
            led3.set(P3[14]);
            led4.set(P4[14]);
            led5.set(P5[14]);
            led6.set(P6[14]);
            led7.set(P7[14]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[15]);
            led2.set(P2[15]);
            led3.set(P3[15]);
            led4.set(P4[15]);
            led5.set(P5[15]);
            led6.set(P6[15]);
            led7.set(P7[15]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[16]);
            led2.set(P2[16]);
            led3.set(P3[16]);
            led4.set(P4[16]);
            led5.set(P5[16]);
            led6.set(P6[16]);
            led7.set(P7[16]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[18]);
            led2.set(P2[18]);
            led3.set(P3[18]);
            led4.set(P4[18]);
            led5.set(P5[18]);
            led6.set(P6[18]);
            led7.set(P7[18]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[19]);
            led2.set(P2[19]);
            led3.set(P3[19]);
            led4.set(P4[19]);
            led5.set(P5[19]);
            led6.set(P6[19]);
            led7.set(P7[19]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[20]);
            led2.set(P2[20]);
            led3.set(P3[20]);
            led4.set(P4[20]);
            led5.set(P5[20]);
            led6.set(P6[20]);
            led7.set(P7[20]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[21]);
            led2.set(P2[21]);
            led3.set(P3[21]);
            led4.set(P4[21]);
            led5.set(P5[21]);
            led6.set(P6[21]);
            led7.set(P7[21]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[22]);
            led2.set(P2[22]);
            led3.set(P3[22]);
            led4.set(P4[22]);
            led5.set(P5[22]);
            led6.set(P6[22]);
            led7.set(P7[22]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[24]);
            led2.set(P2[24]);
            led3.set(P3[24]);
            led4.set(P4[24]);
            led5.set(P5[24]);
            led6.set(P6[24]);
            led7.set(P7[24]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[25]);
            led2.set(P2[25]);
            led3.set(P3[25]);
            led4.set(P4[25]);
            led5.set(P5[25]);
            led6.set(P6[25]);
            led7.set(P7[25]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[26]);
            led2.set(P2[26]);
            led3.set(P3[26]);
            led4.set(P4[26]);
            led5.set(P5[26]);
            led6.set(P6[26]);
            led7.set(P7[26]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[27]);
            led2.set(P2[27]);
            led3.set(P3[27]);
            led4.set(P4[27]);
            led5.set(P5[27]);
            led6.set(P6[27]);
            led7.set(P7[27]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[28]);
            led2.set(P2[28]);
            led3.set(P3[28]);
            led4.set(P4[28]);
            led5.set(P5[28]);
            led6.set(P6[28]);
            led7.set(P7[28]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[30]);
            led2.set(P2[30]);
            led3.set(P3[30]);
            led4.set(P4[30]);
            led5.set(P5[30]);
            led6.set(P6[30]);
            led7.set(P7[30]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[31]);
            led2.set(P2[31]);
            led3.set(P3[31]);
            led4.set(P4[31]);
            led5.set(P5[31]);
            led6.set(P6[31]);
            led7.set(P7[31]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[32]);
            led2.set(P2[32]);
            led3.set(P3[32]);
            led4.set(P4[32]);
            led5.set(P5[32]);
            led6.set(P6[32]);
            led7.set(P7[32]);
          }
          else if (currentmin >= 30 and currentmin <40)
          {
            led1.set(P1[33]);
            led2.set(P2[33]);
            led3.set(P3[33]);
            led4.set(P4[33]);
            led5.set(P5[33]);
            led6.set(P6[33]);
            led7.set(P7[33]);
          }
          else if (currentmin >= 40 and currentmin <50)
          {
            led1.set(P1[34]);
            led2.set(P2[34]);
            led3.set(P3[34]);
            led4.set(P4[34]);
            led5.set(P5[34]);
            led6.set(P6[34]);
            led7.set(P7[34]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[36]);
            led2.set(P2[36]);
            led3.set(P3[36]);
            led4.set(P4[36]);
            led5.set(P5[36]);
            led6.set(P6[36]);
            led7.set(P7[36]);
          }
          else if (currentmin >= 10 and currentmin <20)
          {
            led1.set(P1[37]);
            led2.set(P2[37]);
            led3.set(P3[37]);
            led4.set(P4[37]);
            led5.set(P5[37]);
            led6.set(P6[37]);
            led7.set(P7[37]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[38]);
            led2.set(P2[38]);
            led3.set(P3[38]);
            led4.set(P4[38]);
            led5.set(P5[38]);
            led6.set(P6[38]);
            led7.set(P7[38]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[39]);
            led2.set(P2[39]);
            led3.set(P3[39]);
            led4.set(P4[39]);
            led5.set(P5[39]);
            led6.set(P6[39]);
            led7.set(P7[39]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[40]);
            led2.set(P2[40]);
            led3.set(P3[40]);
            led4.set(P4[40]);
            led5.set(P5[40]);
            led6.set(P6[40]);
            led7.set(P7[40]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[42]);
            led2.set(P2[42]);
            led3.set(P3[42]);
            led4.set(P4[42]);
            led5.set(P5[42]);
            led6.set(P6[42]);
            led7.set(P7[42]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[43]);
            led2.set(P2[43]);
            led3.set(P3[43]);
            led4.set(P4[43]);
            led5.set(P5[43]);
            led6.set(P6[43]);
            led7.set(P7[43]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[44]);
            led2.set(P2[44]);
            led3.set(P3[44]);
            led4.set(P4[44]);
            led5.set(P5[44]);
            led6.set(P6[44]);
            led7.set(P7[44]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[45]);
            led2.set(P2[45]);
            led3.set(P3[45]);
            led4.set(P4[45]);
            led5.set(P5[45]);
            led6.set(P6[45]);
            led7.set(P7[45]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[46]);
            led2.set(P2[46]);
            led3.set(P3[46]);
            led4.set(P4[46]);
            led5.set(P5[46]);
            led6.set(P6[46]);
            led7.set(P7[46]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[48]);
            led2.set(P2[48]);
            led3.set(P3[48]);
            led4.set(P4[48]);
            led5.set(P5[48]);
            led6.set(P6[48]);
            led7.set(P7[48]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[49]);
            led2.set(P2[49]);
            led3.set(P3[49]);
            led4.set(P4[49]);
            led5.set(P5[49]);
            led6.set(P6[49]);
            led7.set(P7[49]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[50]);
            led2.set(P2[50]);
            led3.set(P3[50]);
            led4.set(P4[50]);
            led5.set(P5[50]);
            led6.set(P6[50]);
            led7.set(P7[50]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[51]);
            led2.set(P2[51]);
            led3.set(P3[51]);
            led4.set(P4[51]);
            led5.set(P5[51]);
            led6.set(P6[51]);
            led7.set(P7[51]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[52]);
            led2.set(P2[52]);
            led3.set(P3[52]);
            led4.set(P4[52]);
            led5.set(P5[52]);
            led6.set(P6[52]);
            led7.set(P7[52]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[54]);
            led2.set(P2[54]);
            led3.set(P3[54]);
            led4.set(P4[54]);
            led5.set(P5[54]);
            led6.set(P6[54]);
            led7.set(P7[54]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[55]);
            led2.set(P2[55]);
            led3.set(P3[55]);
            led4.set(P4[55]);
            led5.set(P5[55]);
            led6.set(P6[55]);
            led7.set(P7[55]);
          }
          else if (currentmin >= 20 and currentmin <30)
          {
            led1.set(P1[56]);
            led2.set(P2[56]);
            led3.set(P3[56]);
            led4.set(P4[56]);
            led5.set(P5[56]);
            led6.set(P6[56]);
            led7.set(P7[56]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[57]);
            led2.set(P2[57]);
            led3.set(P3[57]);
            led4.set(P4[57]);
            led5.set(P5[57]);
            led6.set(P6[57]);
            led7.set(P7[57]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[58]);
            led2.set(P2[58]);
            led3.set(P3[58]);
            led4.set(P4[58]);
            led5.set(P5[58]);
            led6.set(P6[58]);
            led7.set(P7[58]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[60]);
            led2.set(P2[60]);
            led3.set(P3[60]);
            led4.set(P4[60]);
            led5.set(P5[60]);
            led6.set(P6[60]);
            led7.set(P7[60]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[61]);
            led2.set(P2[61]);
            led3.set(P3[61]);
            led4.set(P4[61]);
            led5.set(P5[61]);
            led6.set(P6[61]);
            led7.set(P7[61]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[62]);
            led2.set(P2[62]);
            led3.set(P3[62]);
            led4.set(P4[62]);
            led5.set(P5[62]);
            led6.set(P6[62]);
            led7.set(P7[62]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[63]);
            led2.set(P2[63]);
            led3.set(P3[63]);
            led4.set(P4[63]);
            led5.set(P5[63]);
            led6.set(P6[63]);
            led7.set(P7[63]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[64]);
            led2.set(P2[64]);
            led3.set(P3[64]);
            led4.set(P4[64]);
            led5.set(P5[64]);
            led6.set(P6[64]);
            led7.set(P7[64]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[66]);
            led2.set(P2[66]);
            led3.set(P3[66]);
            led4.set(P4[66]);
            led5.set(P5[66]);
            led6.set(P6[66]);
            led7.set(P7[66]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[67]);
            led2.set(P2[67]);
            led3.set(P3[67]);
            led4.set(P4[67]);
            led5.set(P5[67]);
            led6.set(P6[67]);
            led7.set(P7[67]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[68]);
            led2.set(P2[68]);
            led3.set(P3[68]);
            led4.set(P4[68]);
            led5.set(P5[68]);
            led6.set(P6[68]);
            led7.set(P7[68]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[69]);
            led2.set(P2[69]);
            led3.set(P3[69]);
            led4.set(P4[69]);
            led5.set(P5[69]);
            led6.set(P6[69]);
            led7.set(P7[69]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[70]);
            led2.set(P2[70]);
            led3.set(P3[70]);
            led4.set(P4[70]);
            led5.set(P5[70]);
            led6.set(P6[70]);
            led7.set(P7[70]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[72]);
            led2.set(P2[72]);
            led3.set(P3[72]);
            led4.set(P4[72]);
            led5.set(P5[72]);
            led6.set(P6[72]);
            led7.set(P7[72]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[73]);
            led2.set(P2[73]);
            led3.set(P3[73]);
            led4.set(P4[73]);
            led5.set(P5[73]);
            led6.set(P6[73]);
            led7.set(P7[73]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[74]);
            led2.set(P2[74]);
            led3.set(P3[74]);
            led4.set(P4[74]);
            led5.set(P5[74]);
            led6.set(P6[74]);
            led7.set(P7[74]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[75]);
            led2.set(P2[75]);
            led3.set(P3[75]);
            led4.set(P4[75]);
            led5.set(P5[75]);
            led6.set(P6[75]);
            led7.set(P7[75]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[76]);
            led2.set(P2[76]);
            led3.set(P3[76]);
            led4.set(P4[76]);
            led5.set(P5[76]);
            led6.set(P6[76]);
            led7.set(P7[76]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[78]);
            led2.set(P2[78]);
            led3.set(P3[78]);
            led4.set(P4[78]);
            led5.set(P5[78]);
            led6.set(P6[78]);
            led7.set(P7[78]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[79]);
            led2.set(P2[79]);
            led3.set(P3[79]);
            led4.set(P4[79]);
            led5.set(P5[79]);
            led6.set(P6[79]);
            led7.set(P7[79]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[80]);
            led2.set(P2[80]);
            led3.set(P3[80]);
            led4.set(P4[80]);
            led5.set(P5[80]);
            led6.set(P6[80]);
            led7.set(P7[80]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[81]);
            led2.set(P2[81]);
            led3.set(P3[81]);
            led4.set(P4[81]);
            led5.set(P5[81]);
            led6.set(P6[81]);
            led7.set(P7[81]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[82]);
            led2.set(P2[82]);
            led3.set(P3[82]);
            led4.set(P4[82]);
            led5.set(P5[82]);
            led6.set(P6[82]);
            led7.set(P7[82]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[84]);
            led2.set(P2[84]);
            led3.set(P3[84]);
            led4.set(P4[84]);
            led5.set(P5[84]);
            led6.set(P6[84]);
            led7.set(P7[84]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[85]);
            led2.set(P2[85]);
            led3.set(P3[85]);
            led4.set(P4[85]);
            led5.set(P5[85]);
            led6.set(P6[85]);
            led7.set(P7[85]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[86]);
            led2.set(P2[86]);
            led3.set(P3[86]);
            led4.set(P4[86]);
            led5.set(P5[86]);
            led6.set(P6[86]);
            led7.set(P7[86]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[87]);
            led2.set(P2[87]);
            led3.set(P3[87]);
            led4.set(P4[87]);
            led5.set(P5[87]);
            led6.set(P6[87]);
            led7.set(P7[87]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[88]);
            led2.set(P2[88]);
            led3.set(P3[88]);
            led4.set(P4[88]);
            led5.set(P5[88]);
            led6.set(P6[88]);
            led7.set(P7[88]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[90]);
            led2.set(P2[90]);
            led3.set(P3[90]);
            led4.set(P4[90]);
            led5.set(P5[90]);
            led6.set(P6[90]);
            led7.set(P7[90]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[91]);
            led2.set(P2[91]);
            led3.set(P3[91]);
            led4.set(P4[91]);
            led5.set(P5[91]);
            led6.set(P6[91]);
            led7.set(P7[91]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[92]);
            led2.set(P2[92]);
            led3.set(P3[92]);
            led4.set(P4[92]);
            led5.set(P5[92]);
            led6.set(P6[92]);
            led7.set(P7[92]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[93]);
            led2.set(P2[93]);
            led3.set(P3[93]);
            led4.set(P4[93]);
            led5.set(P5[93]);
            led6.set(P6[93]);
            led7.set(P7[93]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[94]);
            led2.set(P2[94]);
            led3.set(P3[94]);
            led4.set(P4[94]);
            led5.set(P5[94]);
            led6.set(P6[94]);
            led7.set(P7[94]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[96]);
            led2.set(P2[96]);
            led3.set(P3[96]);
            led4.set(P4[96]);
            led5.set(P5[96]);
            led6.set(P6[96]);
            led7.set(P7[96]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[97]);
            led2.set(P2[97]);
            led3.set(P3[97]);
            led4.set(P4[97]);
            led5.set(P5[97]);
            led6.set(P6[97]);
            led7.set(P7[97]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[98]);
            led2.set(P2[98]);
            led3.set(P3[98]);
            led4.set(P4[98]);
            led5.set(P5[98]);
            led6.set(P6[98]);
            led7.set(P7[98]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[99]);
            led2.set(P2[99]);
            led3.set(P3[99]);
            led4.set(P4[99]);
            led5.set(P5[99]);
            led6.set(P6[99]);
            led7.set(P7[99]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[100]);
            led2.set(P2[100]);
            led3.set(P3[100]);
            led4.set(P4[100]);
            led5.set(P5[100]);
            led6.set(P6[100]);
            led7.set(P7[100]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[102]);
            led2.set(P2[102]);
            led3.set(P3[102]);
            led4.set(P4[102]);
            led5.set(P5[102]);
            led6.set(P6[102]);
            led7.set(P7[102]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[103]);
            led2.set(P2[103]);
            led3.set(P3[103]);
            led4.set(P4[103]);
            led5.set(P5[103]);
            led6.set(P6[103]);
            led7.set(P7[103]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[104]);
            led2.set(P2[104]);
            led3.set(P3[104]);
            led4.set(P4[104]);
            led5.set(P5[104]);
            led6.set(P6[104]);
            led7.set(P7[104]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[105]);
            led2.set(P2[105]);
            led3.set(P3[105]);
            led4.set(P4[105]);
            led5.set(P5[105]);
            led6.set(P6[105]);
            led7.set(P7[105]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[106]);
            led2.set(P2[106]);
            led3.set(P3[106]);
            led4.set(P4[106]);
            led5.set(P5[106]);
            led6.set(P6[106]);
            led7.set(P7[106]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[108]);
            led2.set(P2[108]);
            led3.set(P3[108]);
            led4.set(P4[108]);
            led5.set(P5[108]);
            led6.set(P6[108]);
            led7.set(P7[108]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[109]);
            led2.set(P2[109]);
            led3.set(P3[109]);
            led4.set(P4[109]);
            led5.set(P5[109]);
            led6.set(P6[109]);
            led7.set(P7[109]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[110]);
            led2.set(P2[110]);
            led3.set(P3[110]);
            led4.set(P4[110]);
            led5.set(P5[110]);
            led6.set(P6[110]);
            led7.set(P7[110]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[111]);
            led2.set(P2[111]);
            led3.set(P3[111]);
            led4.set(P4[111]);
            led5.set(P5[111]);
            led6.set(P6[111]);
            led7.set(P7[111]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[112]);
            led2.set(P2[112]);
            led3.set(P3[112]);
            led4.set(P4[112]);
            led5.set(P5[112]);
            led6.set(P6[112]);
            led7.set(P7[112]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[114]);
            led2.set(P2[114]);
            led3.set(P3[114]);
            led4.set(P4[114]);
            led5.set(P5[114]);
            led6.set(P6[114]);
            led7.set(P7[114]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[115]);
            led2.set(P2[115]);
            led3.set(P3[115]);
            led4.set(P4[115]);
            led5.set(P5[115]);
            led6.set(P6[115]);
            led7.set(P7[115]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[116]);
            led2.set(P2[116]);
            led3.set(P3[116]);
            led4.set(P4[116]);
            led5.set(P5[116]);
            led6.set(P6[116]);
            led7.set(P7[116]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[117]);
            led2.set(P2[117]);
            led3.set(P3[117]);
            led4.set(P4[117]);
            led5.set(P5[117]);
            led6.set(P6[117]);
            led7.set(P7[117]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[118]);
            led2.set(P2[118]);
            led3.set(P3[118]);
            led4.set(P4[118]);
            led5.set(P5[118]);
            led6.set(P6[118]);
            led7.set(P7[118]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[120]);
            led2.set(P2[120]);
            led3.set(P3[120]);
            led4.set(P4[120]);
            led5.set(P5[120]);
            led6.set(P6[120]);
            led7.set(P7[120]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[121]);
            led2.set(P2[121]);
            led3.set(P3[121]);
            led4.set(P4[121]);
            led5.set(P5[121]);
            led6.set(P6[121]);
            led7.set(P7[121]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[122]);
            led2.set(P2[122]);
            led3.set(P3[122]);
            led4.set(P4[122]);
            led5.set(P5[122]);
            led6.set(P6[122]);
            led7.set(P7[122]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[123]);
            led2.set(P2[123]);
            led3.set(P3[123]);
            led4.set(P4[123]);
            led5.set(P5[123]);
            led6.set(P6[123]);
            led7.set(P7[123]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[124]);
            led2.set(P2[124]);
            led3.set(P3[124]);
            led4.set(P4[124]);
            led5.set(P5[124]);
            led6.set(P6[124]);
            led7.set(P7[124]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[126]);
            led2.set(P2[126]);
            led3.set(P3[126]);
            led4.set(P4[126]);
            led5.set(P5[126]);
            led6.set(P6[126]);
            led7.set(P7[126]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[127]);
            led2.set(P2[127]);
            led3.set(P3[127]);
            led4.set(P4[127]);
            led5.set(P5[127]);
            led6.set(P6[127]);
            led7.set(P7[127]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[128]);
            led2.set(P2[128]);
            led3.set(P3[128]);
            led4.set(P4[128]);
            led5.set(P5[128]);
            led6.set(P6[128]);
            led7.set(P7[128]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[129]);
            led2.set(P2[129]);
            led3.set(P3[129]);
            led4.set(P4[129]);
            led5.set(P5[129]);
            led6.set(P6[129]);
            led7.set(P7[129]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[130]);
            led2.set(P2[130]);
            led3.set(P3[130]);
            led4.set(P4[130]);
            led5.set(P5[130]);
            led6.set(P6[130]);
            led7.set(P7[130]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[132]);
            led2.set(P2[132]);
            led3.set(P3[132]);
            led4.set(P4[132]);
            led5.set(P5[132]);
            led6.set(P6[132]);
            led7.set(P7[132]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[133]);
            led2.set(P2[133]);
            led3.set(P3[133]);
            led4.set(P4[133]);
            led5.set(P5[133]);
            led6.set(P6[133]);
            led7.set(P7[133]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[134]);
            led2.set(P2[134]);
            led3.set(P3[134]);
            led4.set(P4[134]);
            led5.set(P5[134]);
            led6.set(P6[134]);
            led7.set(P7[134]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[135]);
            led2.set(P2[135]);
            led3.set(P3[135]);
            led4.set(P4[135]);
            led5.set(P5[135]);
            led6.set(P6[135]);
            led7.set(P7[135]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[136]);
            led2.set(P2[136]);
            led3.set(P3[136]);
            led4.set(P4[136]);
            led5.set(P5[136]);
            led6.set(P6[136]);
            led7.set(P7[136]);
          }
          else if (currentmin >= 50)
          {
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
          if (currentmin < 10)
          {
            led1.set(P1[138]);
            led2.set(P2[138]);
            led3.set(P3[138]);
            led4.set(P4[138]);
            led5.set(P5[138]);
            led6.set(P6[138]);
            led7.set(P7[138]);
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            led1.set(P1[139]);
            led2.set(P2[139]);
            led3.set(P3[139]);
            led4.set(P4[139]);
            led5.set(P5[139]);
            led6.set(P6[139]);
            led7.set(P7[139]);
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            led1.set(P1[140]);
            led2.set(P2[140]);
            led3.set(P3[140]);
            led4.set(P4[140]);
            led5.set(P5[140]);
            led6.set(P6[140]);
            led7.set(P7[140]);
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            led1.set(P1[141]);
            led2.set(P2[141]);
            led3.set(P3[141]);
            led4.set(P4[141]);
            led5.set(P5[141]);
            led6.set(P6[141]);
            led7.set(P7[141]);
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            led1.set(P1[142]);
            led2.set(P2[142]);
            led3.set(P3[142]);
            led4.set(P4[142]);
            led5.set(P5[142]);
            led6.set(P6[142]);
            led7.set(P7[142]);
          }
          else if (currentmin >= 50)
          {
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
        Serial.printf("test mode .... current sec is: %d:%d:%d the led7's volume is: %d \r\n", currenthour,currentmin,currentsec, P7[TESTMODE_COUNT]);
        if (TESTMODE_COUNT > 143)
        {
          TESTMODE_COUNT = 0;
        }

        led1.set(P1[TESTMODE_COUNT]);
        led2.set(P2[TESTMODE_COUNT]);
        led3.set(P3[TESTMODE_COUNT]);
        led4.set(P4[TESTMODE_COUNT]);
        led5.set(P5[TESTMODE_COUNT]);
        led6.set(P6[TESTMODE_COUNT]);
        led7.set(P7[TESTMODE_COUNT]);

        TESTMODE_COUNT = TESTMODE_COUNT + 1;
      }

      if (IS_SMART)
      {

        if ((WiFi.status() == WL_CONNECTED || WiFi.smartConfigDone()) && !client.connected())
        {
          led8.set(5);
          Serial.println("the mqtt service is disconnected");
          mqttconn();
          client.publish("esp32/disnotify", ESP_HOST_NAME.c_str());
        }
      }
      if (client.connected())
      {
        client.publish("esp32/heart", ESP_HOST_NAME.c_str());
      }
    }
    else if (PWM_INFO_SHOWTYPE == "fix")
    {
      //Serial.printf("fix mode ... the led1's volume is: %d \n", P7[144]);
      led1.set(P1[144]);
      led2.set(P2[144]);
      led3.set(P3[144]);
      led4.set(P4[144]);
      led5.set(P5[144]);
      led6.set(P6[144]);
      led7.set(P7[144]);
      //String topiccontent = ESP_HOST_NAME;
      //Serial.printf("the connect status is: %d, the smart status is: %d, the mqtt is: %d, the ssid is: %s \n", WiFi.status(), WiFi.smartConfigDone(), client.connected(), SSID.c_str());
        if(WiFi.status() == WL_CONNECTED) {
          if (!client.connected())
          {
            led8.set(5);
            Serial.println("the mqtt service is disconnected");
            mqttconn();
            client.publish("esp32/disnotify", ESP_HOST_NAME.c_str());
          } else {
            client.publish("esp32/heart", ESP_HOST_NAME.c_str());
          }
        }

    }
  }
  delay(1000);
}