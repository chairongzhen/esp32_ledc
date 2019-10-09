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
#include <math.h>
#include <HTTPUpdate.h>
//#include <TaskScheduler.h>

#define RESET_BUTTON 26
#define VERSION_NUM "0.92"
#define ESP_RTC_TICK 1565332650

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
bool isUpaded = false;

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

// int contentLength = 0;
// bool isValidContentType = false;

LED_ESP32 led1(4, 0, 100);
LED_ESP32 led2(12, 1, 100);
LED_ESP32 led3(13, 2, 100);
LED_ESP32 led4(15, 3, 100); // the led15 has trouble when rest the light will lighting a sceond
LED_ESP32 led5(21, 4, 100);
LED_ESP32 led6(22, 5, 100);
LED_ESP32 led7(23, 6, 100);
LED_ESP32 led8(27, 7, 100); // the channel 8 has some trouble for switch the lgiht to 0 , is disable

// 信号灯
LED_ESP32 led0(25, 10, 100);

AsyncWebServer server(80);
WiFiClient espClient;
WiFiClient updateClient;
PubSubClient client(espClient);
//HttpClient HttpClient(espClient,mqttServer);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 28800, 60000);

// // multi callback prototypes
// void reconnectCallback();

// // multi tasks
// Task taskReconnect(5000,TASK_FOREVER,&reconnectCallback);

// void reconnectCallback() {

// }

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

String removeColon(String input)
{
  const String strFind = ":";
  const String strReplace = "";
  input.replace(strFind, strReplace);
  return input;
}

void initFileSystem()
{
  if (SPIFFS.exists("/wifi.ini"))
  {
    Serial.println("delete the wifi config file");
    deleteFile(SPIFFS, "/wifi.ini");
  }
  if (SPIFFS.exists("/mid.ini"))
  {
    deleteFile(SPIFFS, "/mid.ini");
  }

  // if (!SPIFFS.exists("/mid.ini"))
  // {
  Serial.println("init the mid file");
  String midcontent = "{\"mid\":\"{mid}\",\"mac\":\"{mac}\"}";
  midcontent.replace("{mac}", WiFi.macAddress());
  midcontent.replace("{mid}", "esp_" + removeColon(WiFi.macAddress()));
  Serial.println(midcontent);
  writeFile(SPIFFS, "/mid.ini", midcontent.c_str());
  // }

  if (SPIFFS.exists("/pwminfo.ini"))
  {
    deleteFile(SPIFFS, "/pwminfo.ini");
  }

  String pwminfocontent = "{\"showtype\":\"fix\",\"testmode\":\"test\",\"sysdate\":\"{unixtick}\",\"status\":\"stop\",\"conmode\": \"none\",\"version\":\"{version_num}\"}";
  pwminfocontent.replace("{version_num}", VERSION_NUM);
  pwminfocontent.replace("{unixtick}", String(ESP_RTC_TICK));
  writeFile(SPIFFS, "/pwminfo.ini", pwminfocontent.c_str());
  //writeFile(SPIFFS, "/p.ini", "{\"t000\":\"00000000000000\",\"t001\":\"00000000000000\",\"t002\":\"00000000000000\",\"t003\":\"00000000000000\",\"t004\":\"00000000000000\",\"t005\":\"00000000000000\",\"t010\":\"00000000000000\",\"t011\":\"00000000000000\",\"t012\":\"00000000000000\",\"t013\":\"00000000000000\",\"t014\":\"00000000000000\",\"t015\":\"00000000000000\",\"t020\":\"00000000000000\",\"t021\":\"00000000000000\",\"t022\":\"00000000000000\",\"t023\":\"00000000000000\",\"t024\":\"00000000000000\",\"t025\":\"00000000000000\",\"t030\":\"00000000000000\",\"t031\":\"00000000000000\",\"t032\":\"00000000000000\",\"t033\":\"00000000000000\",\"t034\":\"00000000000000\",\"t035\":\"00000000000000\",\"t040\":\"00000000000000\",\"t041\":\"00000000000000\",\"t042\":\"00000000000000\",\"t043\":\"00000000000000\",\"t044\":\"00000000000000\",\"t045\":\"00000000000000\",\"t050\":\"00000000000000\",\"t051\":\"00000000000000\",\"t052\":\"00000000000000\",\"t053\":\"00000000000000\",\"t054\":\"00000000000000\",\"t055\":\"00000000000000\",\"t060\":\"00000000000000\",\"t061\":\"00000000000000\",\"t062\":\"00000000000000\",\"t063\":\"00000000000000\",\"t064\":\"00000000000000\",\"t065\":\"00000000000000\",\"t070\":\"00000000000000\",\"t071\":\"00000000000000\",\"t072\":\"00000000000000\",\"t073\":\"00000000000000\",\"t074\":\"00000000000000\",\"t075\":\"00000000000000\",\"t080\":\"00000000000000\",\"t081\":\"00000000000000\",\"t082\":\"00000000000000\",\"t083\":\"00000000000000\",\"t084\":\"00000000000000\",\"t085\":\"00000000000000\",\"t090\":\"00000000000000\",\"t091\":\"00000000000000\",\"t092\":\"00000000000000\",\"t093\":\"00000000000000\",\"t094\":\"00000000000000\",\"t095\":\"00000000000000\",\"t100\":\"00000000000000\",\"t101\":\"00000000000000\",\"t102\":\"00000000000000\",\"t103\":\"00000000000000\",\"t104\":\"00000000000000\",\"t105\":\"00000000000000\",\"t110\":\"00000000000000\",\"t111\":\"00000000000000\",\"t112\":\"00000000000000\",\"t113\":\"00000000000000\",\"t114\":\"00000000000000\",\"t115\":\"00000000000000\",\"t120\":\"00000000000000\",\"t121\":\"00000000000000\",\"t122\":\"00000000000000\",\"t123\":\"00000000000000\",\"t124\":\"00000000000000\",\"t125\":\"00000000000000\",\"t130\":\"00000000000000\",\"t131\":\"00000000000000\",\"t132\":\"00000000000000\",\"t133\":\"00000000000000\",\"t134\":\"00000000000000\",\"t135\":\"00000000000000\",\"t140\":\"00000000000000\",\"t141\":\"00000000000000\",\"t142\":\"00000000000000\",\"t143\":\"00000000000000\",\"t144\":\"00000000000000\",\"t145\":\"00000000000000\",\"t150\":\"00000000000000\",\"t151\":\"00000000000000\",\"t152\":\"00000000000000\",\"t153\":\"00000000000000\",\"t154\":\"00000000000000\",\"t155\":\"00000000000000\",\"t160\":\"00000000000000\",\"t161\":\"00000000000000\",\"t162\":\"00000000000000\",\"t163\":\"00000000000000\",\"t164\":\"00000000000000\",\"t165\":\"00000000000000\",\"t170\":\"00000000000000\",\"t171\":\"00000000000000\",\"t172\":\"00000000000000\",\"t173\":\"00000000000000\",\"t174\":\"00000000000000\",\"t175\":\"00000000000000\",\"t180\":\"00000000000000\",\"t181\":\"00000000000000\",\"t182\":\"00000000000000\",\"t183\":\"00000000000000\",\"t184\":\"00000000000000\",\"t185\":\"00000000000000\",\"t190\":\"00000000000000\",\"t191\":\"00000000000000\",\"t192\":\"00000000000000\",\"t193\":\"00000000000000\",\"t194\":\"00000000000000\",\"t195\":\"00000000000000\",\"t200\":\"00000000000000\",\"t201\":\"00000000000000\",\"t202\":\"00000000000000\",\"t203\":\"00000000000000\",\"t204\":\"00000000000000\",\"t205\":\"00000000000000\",\"t210\":\"00000000000000\",\"t211\":\"00000000000000\",\"t212\":\"00000000000000\",\"t213\":\"00000000000000\",\"t214\":\"00000000000000\",\"t215\":\"00000000000000\",\"t220\":\"00000000000000\",\"t221\":\"00000000000000\",\"t222\":\"00000000000000\",\"t223\":\"00000000000000\",\"t224\":\"00000000000000\",\"t225\":\"00000000000000\",\"t230\":\"00000000000000\",\"t231\":\"00000000000000\",\"t232\":\"00000000000000\",\"t233\":\"00000000000000\",\"t234\":\"00000000000000\",\"t235\":\"00000000000000\",\"tfix\":\"00000000000000\"}");

  if (SPIFFS.exists("/p.ini"))
  {
    deleteFile(SPIFFS, "/p.ini");
  }
  writeFile(SPIFFS, "/p.ini", "{\"t000\":\"0000000000000000\",\"t001\":\"0000000000000000\",\"t002\":\"0000000000000000\",\"t003\":\"0000000000000000\",\"t004\":\"0000000000000000\",\"t005\":\"0000000000000000\",\"t010\":\"0000000000000000\",\"t011\":\"0000000000000000\",\"t012\":\"0000000000000000\",\"t013\":\"0000000000000000\",\"t014\":\"0000000000000000\",\"t015\":\"0000000000000000\",\"t020\":\"0000000000000000\",\"t021\":\"0000000000000000\",\"t022\":\"0000000000000000\",\"t023\":\"0000000000000000\",\"t024\":\"0000000000000000\",\"t025\":\"0000000000000000\",\"t030\":\"0000000000000000\",\"t031\":\"0000000000000000\",\"t032\":\"0000000000000000\",\"t033\":\"0000000000000000\",\"t034\":\"0000000000000000\",\"t035\":\"0000000000000000\",\"t040\":\"0000000000000000\",\"t041\":\"0000000000000000\",\"t042\":\"0000000000000000\",\"t043\":\"0000000000000000\",\"t044\":\"0000000000000000\",\"t045\":\"0000000000000000\",\"t050\":\"0000000000000000\",\"t051\":\"0000000000000000\",\"t052\":\"0000000000000000\",\"t053\":\"0000000000000000\",\"t054\":\"0000000000000000\",\"t055\":\"0000000000000000\",\"t060\":\"0000000000000000\",\"t061\":\"0000000000000000\",\"t062\":\"0000000000000000\",\"t063\":\"0000000000000000\",\"t064\":\"0000000000000000\",\"t065\":\"0000000000000000\",\"t070\":\"0000000000000000\",\"t071\":\"0000000000000000\",\"t072\":\"0000000000000000\",\"t073\":\"0000000000000000\",\"t074\":\"0000000000000000\",\"t075\":\"0000000000000000\",\"t080\":\"0000000000000000\",\"t081\":\"0000000000000000\",\"t082\":\"0000000000000000\",\"t083\":\"0000000000000000\",\"t084\":\"0000000000000000\",\"t085\":\"0000000000000000\",\"t090\":\"0000000000000000\",\"t091\":\"0000000000000000\",\"t092\":\"0000000000000000\",\"t093\":\"0000000000000000\",\"t094\":\"0000000000000000\",\"t095\":\"0000000000000000\",\"t100\":\"0000000000000000\",\"t101\":\"0000000000000000\",\"t102\":\"0000000000000000\",\"t103\":\"0000000000000000\",\"t104\":\"0000000000000000\",\"t105\":\"0000000000000000\",\"t110\":\"0000000000000000\",\"t111\":\"0000000000000000\",\"t112\":\"0000000000000000\",\"t113\":\"0000000000000000\",\"t114\":\"0000000000000000\",\"t115\":\"0000000000000000\",\"t120\":\"0000000000000000\",\"t121\":\"0000000000000000\",\"t122\":\"0000000000000000\",\"t123\":\"0000000000000000\",\"t124\":\"0000000000000000\",\"t125\":\"0000000000000000\",\"t130\":\"0000000000000000\",\"t131\":\"0000000000000000\",\"t132\":\"0000000000000000\",\"t133\":\"0000000000000000\",\"t134\":\"0000000000000000\",\"t135\":\"0000000000000000\",\"t140\":\"0000000000000000\",\"t141\":\"0000000000000000\",\"t142\":\"0000000000000000\",\"t143\":\"0000000000000000\",\"t144\":\"0000000000000000\",\"t145\":\"0000000000000000\",\"t150\":\"0000000000000000\",\"t151\":\"0000000000000000\",\"t152\":\"0000000000000000\",\"t153\":\"0000000000000000\",\"t154\":\"0000000000000000\",\"t155\":\"0000000000000000\",\"t160\":\"0000000000000000\",\"t161\":\"0000000000000000\",\"t162\":\"0000000000000000\",\"t163\":\"0000000000000000\",\"t164\":\"0000000000000000\",\"t165\":\"0000000000000000\",\"t170\":\"0000000000000000\",\"t171\":\"0000000000000000\",\"t172\":\"0000000000000000\",\"t173\":\"0000000000000000\",\"t174\":\"0000000000000000\",\"t175\":\"0000000000000000\",\"t180\":\"0000000000000000\",\"t181\":\"0000000000000000\",\"t182\":\"0000000000000000\",\"t183\":\"0000000000000000\",\"t184\":\"0000000000000000\",\"t185\":\"0000000000000000\",\"t190\":\"0000000000000000\",\"t191\":\"0000000000000000\",\"t192\":\"0000000000000000\",\"t193\":\"0000000000000000\",\"t194\":\"0000000000000000\",\"t195\":\"0000000000000000\",\"t200\":\"0000000000000000\",\"t201\":\"0000000000000000\",\"t202\":\"0000000000000000\",\"t203\":\"0000000000000000\",\"t204\":\"0000000000000000\",\"t205\":\"0000000000000000\",\"t210\":\"0000000000000000\",\"t211\":\"0000000000000000\",\"t212\":\"0000000000000000\",\"t213\":\"0000000000000000\",\"t214\":\"0000000000000000\",\"t215\":\"0000000000000000\",\"t220\":\"0000000000000000\",\"t221\":\"0000000000000000\",\"t222\":\"0000000000000000\",\"t223\":\"0000000000000000\",\"t224\":\"0000000000000000\",\"t225\":\"0000000000000000\",\"t230\":\"0000000000000000\",\"t231\":\"0000000000000000\",\"t232\":\"0000000000000000\",\"t233\":\"0000000000000000\",\"t234\":\"0000000000000000\",\"t235\":\"0000000000000000\",\"tfix\":\"0000000000000000\"}");

  if (!SPIFFS.exists("/version"))
  {
    String versionContent = "0.90";
    writeFile(SPIFFS, "/version.ini", versionContent.c_str());
  }

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
  String checkversiontopic = "esp32/checkversion";

  if (String(topic) == topic_name_p)
  {
    TESTMODE_COUNT = 0;
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
    Serial.println(filecontent);
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
  else if (String(topic) == checkversiontopic)
  {
    String versionContent = filecontent;
    writeFile(SPIFFS, "/version.ini", versionContent.c_str());
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
    Serial.println(filecontent);
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
      led0.set(100);
      Serial.println("MQTT has connected...");
      // subscribe the checktime service
      client.subscribe("esp32/checktime");

      // subscribe the check version service
      client.subscribe("esp32/checkversion");

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

String getHeaderValue(String header, String headerName)
{
  return header.substring(strlen(headerName.c_str()));
}

void execOTA()
{
}

bool execCheckVersion(String oldversion)
{
  return true;
  // String version = "0.50";
  // bool needtoupdate = false;
  // int err = http.get("/version");
  // if(err == 0) {
  //   err = http.responseStatusCode();
  //   if(err >=0) {
  //     while(http.available() && !http.endOfBodyReached()) {
  //       String line = http.readStringUntil('\n');
  //       line.trim();
  //       if (line.startsWith("version:")) {
  //         line.replace("version:","");
  //         version = line;
  //       }
  //     }
  //   }
  // }
  // if(oldversion != version) {
  //   needtoupdate =  true;
  // }
  // return needtoupdate;
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
  Serial.printf("esp version: %s \r\n", VERSION_NUM);
  RESET_FLAG = false;
  IS_SMART = false;
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON), handleRestButtonChanged, CHANGE);
  WiFi.mode(WIFI_AP_STA);

  // Serial.print("the current version is: ");
  // Serial.println(VERSION_NUM);

  led0.setup();
  led1.setup();
  led2.setup();
  led3.setup();
  led4.setup();
  led5.setup();
  led6.setup();
  led7.setup();
  led8.setup();

  led0.set(0);
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
    // deleteFile(SPIFFS,"/pwminfo.ini");
    // deleteFile(SPIFFS,"/version.ini");
    // initFileSystem();
    if (!SPIFFS.exists("/mid.ini"))
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

      if (!SPIFFS.exists("/version"))
      {
        String versionContent = "0.90";
        writeFile(SPIFFS, "/version.ini", versionContent.c_str());
      }

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
        item = cJSON_GetObjectItem(root, "mac");
        itemstr = cJSON_Print(item);
        itemstr.replace("\"", "");
        ESP_MAC = itemstr;
      }

      //Serial.println("the AP name is : " + String(ESP_HOST_NAME) + " password is: " + String(password));
      //Serial.println("the mac address is: " + String(ESP_MAC));
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
          led0.set(100);
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

          item = cJSON_GetObjectItem(root, "conmode");
          itemstr = cJSON_Print(item);
          PWM_INFO_CONMODE = itemstr;
          PWM_INFO_CONMODE.replace("\"", "");
        }
        else
        {
          WiFi.softAP(ESP_HOST_NAME.c_str(), password);
          led0.set(0);
          Serial.println("please set the wifi");
        }
      }
      else
      {
        WiFi.softAP(ESP_HOST_NAME.c_str(), password);
        led0.set(0);
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
    time_t t;
    struct tm *p;
    t = atoi(PWM_INFO_RTC.c_str()) + 28800;
    p = gmtime(&t);
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
      PWM_INFO_CONMODE = "none";

      struct timeval stime;
      if (WiFi.status() != WL_CONNECTED)
      {
        stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 27726;
        settimeofday(&stime, NULL);
      }
      else
      {
        if (timeClient.update())
        {
          stime.tv_sec = timeClient.getEpochTime();
          settimeofday(&stime, NULL);
          sysdate = timeClient.getEpochTime();
        }
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
    //String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>QRCode</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">设备名:</span></td><td style=\"font-size:14px;\"><input type=\"text\"id=\"txtmid\"onchange=\"midchange()\"/></td></tr><tr><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"submit\"value=\"修改\"id=\"btnModify\"onclick=\"submit()\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/><span style=\"color:red;\">修改完设备名后wifi热点名也会随之改变,需要重新选择wifi热点并刷新页面</span></td></tr></table><hr/><div style=\"font-size:14px;\"id=\"divqrcode\"><span>扫描下方二维码进行绑定,如改名需要重新绑定</span><span style=\"color:red;\">需要联网</span><div id=\"code\"></div></div></div><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script><script src='https://cdn.bootcss.com/jquery.qrcode/1.0/jquery.qrcode.min.js'></script><script>function toUtf8(str){var out,i,len,c;out=\"\";len=str.length;for(i=0;i<len;i++){c=str.charCodeAt(i);if((c>=0x0001)&&(c<=0x007F)){out+=str.charAt(i)}else if(c>0x07FF){out+=String.fromCharCode(0xE0|((c>>12)&0x0F));out+=String.fromCharCode(0x80|((c>>6)&0x3F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}else{out+=String.fromCharCode(0xC0|((c>>6)&0x1F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}}return out}function submit(){var mid=document.getElementById('txtmid').value;var url=\"cmid?mid=\"+mid;window.location.href=url}function midchange(){var mid=document.getElementById('txtmid').value;var qrcode=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').empty();$('#code').qrcode(qrcode)}$(\"#divqrcode\").show();var mid=document.getElementById('txtmid').value;console.log('here it is:',mid);var str=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').qrcode(str);$(\"#offline\").hide();</script></body></html>";
    String rawhtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>QRCode</title></head><body><div><table><tr style=\"height:30px;\"><td style=\"width:80px;\"><span style=\"font-size:14px;\">设备名:</span></td><td style=\"font-size:14px;\"><input type=\"text\"id=\"txtmid\"/></td></tr><tr style=\"display: none;\"><td colspan=\"2\"><input style=\"background:green; color:white; width:80px; height:30px; margin-right:2px;\"type=\"submit\"value=\"修改\"id=\"btnModify\"onclick=\"submit()\"/><input style=\"background:green; color:white; width:80px; height:30px;\"type=\"button\"value=\"返回\"id=\"btnBack\"onclick=\"back()\"/><span style=\"color:red;\">修改完设备名后wifi热点名也会随之改变,需要重新选择wifi热点并刷新页面</span></td></tr></table><hr/><div style=\"font-size:14px;\"id=\"divqrcode\"><span>扫描下方二维码进行绑定,如改名需要重新绑定</span><span style=\"color:red;\">需求联网</span><div id=\"code\"></div></div></div><script src='https://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js'></script><script src='https://cdn.bootcss.com/jquery.qrcode/1.0/jquery.qrcode.min.js'></script><script>function toUtf8(str){var out,i,len,c;out=\"\";len=str.length;for(i=0;i<len;i++){c=str.charCodeAt(i);if((c>=0x0001)&&(c<=0x007F)){out+=str.charAt(i)}else if(c>0x07FF){out+=String.fromCharCode(0xE0|((c>>12)&0x0F));out+=String.fromCharCode(0x80|((c>>6)&0x3F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}else{out+=String.fromCharCode(0xC0|((c>>6)&0x1F));out+=String.fromCharCode(0x80|((c>>0)&0x3F))}}return out}function submit(){var mid=document.getElementById('txtmid').value;var url=\"cmid?mid=\"+mid;window.location.href=url}function midchange(){var mid=document.getElementById('txtmid').value;var qrcode=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').empty();$('#code').qrcode(qrcode)}function back(){var url=\"/\";window.location.href=url}var mid=document.getElementById('txtmid').value;var str=toUtf8(`{mid:${mid},mac:00000000}`);$('#code').qrcode(str);</script></body></html>";

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
      String tpl_mid = "<input type=\"text\"id=\"txtmid\"/>";
      String change_mid = "<input type=\"text\"id=\"txtmid\" value=\"";
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

  // for iso all on
  server.on("/ioson", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      request->send(200, "text/html", "failed");
    }
    writeFile(SPIFFS, "/p.ini", "{\"t000\":\"0000000000000000\",\"t001\":\"0000000000000000\",\"t002\":\"0000000000000000\",\"t003\":\"0000000000000000\",\"t004\":\"0000000000000000\",\"t005\":\"0000000000000000\",\"t010\":\"0000000000000000\",\"t011\":\"0000000000000000\",\"t012\":\"0000000000000000\",\"t013\":\"0000000000000000\",\"t014\":\"0000000000000000\",\"t015\":\"0000000000000000\",\"t020\":\"0000000000000000\",\"t021\":\"0000000000000000\",\"t022\":\"0000000000000000\",\"t023\":\"0000000000000000\",\"t024\":\"0000000000000000\",\"t025\":\"0000000000000000\",\"t030\":\"0000000000000000\",\"t031\":\"0000000000000000\",\"t032\":\"0000000000000000\",\"t033\":\"0000000000000000\",\"t034\":\"0000000000000000\",\"t035\":\"0000000000000000\",\"t040\":\"0000000000000000\",\"t041\":\"0000000000000000\",\"t042\":\"0000000000000000\",\"t043\":\"0000000000000000\",\"t044\":\"0000000000000000\",\"t045\":\"0000000000000000\",\"t050\":\"0000000000000000\",\"t051\":\"0000000000000000\",\"t052\":\"0000000000000000\",\"t053\":\"0000000000000000\",\"t054\":\"0000000000000000\",\"t055\":\"0000000000000000\",\"t060\":\"0000000000000000\",\"t061\":\"0000000000000000\",\"t062\":\"0000000000000000\",\"t063\":\"0000000000000000\",\"t064\":\"0000000000000000\",\"t065\":\"0000000000000000\",\"t070\":\"0000000000000000\",\"t071\":\"0000000000000000\",\"t072\":\"0000000000000000\",\"t073\":\"0000000000000000\",\"t074\":\"0000000000000000\",\"t075\":\"0000000000000000\",\"t080\":\"0000000000000000\",\"t081\":\"0000000000000000\",\"t082\":\"0000000000000000\",\"t083\":\"0000000000000000\",\"t084\":\"0000000000000000\",\"t085\":\"0000000000000000\",\"t090\":\"0000000000000000\",\"t091\":\"0000000000000000\",\"t092\":\"0000000000000000\",\"t093\":\"0000000000000000\",\"t094\":\"0000000000000000\",\"t095\":\"0000000000000000\",\"t100\":\"0000000000000000\",\"t101\":\"0000000000000000\",\"t102\":\"0000000000000000\",\"t103\":\"0000000000000000\",\"t104\":\"0000000000000000\",\"t105\":\"0000000000000000\",\"t110\":\"0000000000000000\",\"t111\":\"0000000000000000\",\"t112\":\"0000000000000000\",\"t113\":\"0000000000000000\",\"t114\":\"0000000000000000\",\"t115\":\"0000000000000000\",\"t120\":\"0000000000000000\",\"t121\":\"0000000000000000\",\"t122\":\"0000000000000000\",\"t123\":\"0000000000000000\",\"t124\":\"0000000000000000\",\"t125\":\"0000000000000000\",\"t130\":\"0000000000000000\",\"t131\":\"0000000000000000\",\"t132\":\"0000000000000000\",\"t133\":\"0000000000000000\",\"t134\":\"0000000000000000\",\"t135\":\"0000000000000000\",\"t140\":\"0000000000000000\",\"t141\":\"0000000000000000\",\"t142\":\"0000000000000000\",\"t143\":\"0000000000000000\",\"t144\":\"0000000000000000\",\"t145\":\"0000000000000000\",\"t150\":\"0000000000000000\",\"t151\":\"0000000000000000\",\"t152\":\"0000000000000000\",\"t153\":\"0000000000000000\",\"t154\":\"0000000000000000\",\"t155\":\"0000000000000000\",\"t160\":\"0000000000000000\",\"t161\":\"0000000000000000\",\"t162\":\"0000000000000000\",\"t163\":\"0000000000000000\",\"t164\":\"0000000000000000\",\"t165\":\"0000000000000000\",\"t170\":\"0000000000000000\",\"t171\":\"0000000000000000\",\"t172\":\"0000000000000000\",\"t173\":\"0000000000000000\",\"t174\":\"0000000000000000\",\"t175\":\"0000000000000000\",\"t180\":\"0000000000000000\",\"t181\":\"0000000000000000\",\"t182\":\"0000000000000000\",\"t183\":\"0000000000000000\",\"t184\":\"0000000000000000\",\"t185\":\"0000000000000000\",\"t190\":\"0000000000000000\",\"t191\":\"0000000000000000\",\"t192\":\"0000000000000000\",\"t193\":\"0000000000000000\",\"t194\":\"0000000000000000\",\"t195\":\"0000000000000000\",\"t200\":\"0000000000000000\",\"t201\":\"0000000000000000\",\"t202\":\"0000000000000000\",\"t203\":\"0000000000000000\",\"t204\":\"0000000000000000\",\"t205\":\"0000000000000000\",\"t210\":\"0000000000000000\",\"t211\":\"0000000000000000\",\"t212\":\"0000000000000000\",\"t213\":\"0000000000000000\",\"t214\":\"0000000000000000\",\"t215\":\"0000000000000000\",\"t220\":\"0000000000000000\",\"t221\":\"0000000000000000\",\"t222\":\"0000000000000000\",\"t223\":\"0000000000000000\",\"t224\":\"0000000000000000\",\"t225\":\"0000000000000000\",\"t230\":\"0000000000000000\",\"t231\":\"0000000000000000\",\"t232\":\"0000000000000000\",\"t233\":\"0000000000000000\",\"t234\":\"0000000000000000\",\"t235\":\"0000000000000000\",\"tfix\":\"6464646464646464\"}");
    P1[144] = 100;
    P2[144] = 100;
    P3[144] = 100;
    P4[144] = 100;
    P5[144] = 100;
    P6[144] = 100;
    P7[144] = 100;
    P8[144] = 100;
    request->send(200, "text/html", "success");
  });

  // for iso all off
  server.on("/iosoff", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      request->send(200, "text/html", "failed");
    }
    writeFile(SPIFFS, "/p.ini", "{\"t000\":\"0000000000000000\",\"t001\":\"0000000000000000\",\"t002\":\"0000000000000000\",\"t003\":\"0000000000000000\",\"t004\":\"0000000000000000\",\"t005\":\"0000000000000000\",\"t010\":\"0000000000000000\",\"t011\":\"0000000000000000\",\"t012\":\"0000000000000000\",\"t013\":\"0000000000000000\",\"t014\":\"0000000000000000\",\"t015\":\"0000000000000000\",\"t020\":\"0000000000000000\",\"t021\":\"0000000000000000\",\"t022\":\"0000000000000000\",\"t023\":\"0000000000000000\",\"t024\":\"0000000000000000\",\"t025\":\"0000000000000000\",\"t030\":\"0000000000000000\",\"t031\":\"0000000000000000\",\"t032\":\"0000000000000000\",\"t033\":\"0000000000000000\",\"t034\":\"0000000000000000\",\"t035\":\"0000000000000000\",\"t040\":\"0000000000000000\",\"t041\":\"0000000000000000\",\"t042\":\"0000000000000000\",\"t043\":\"0000000000000000\",\"t044\":\"0000000000000000\",\"t045\":\"0000000000000000\",\"t050\":\"0000000000000000\",\"t051\":\"0000000000000000\",\"t052\":\"0000000000000000\",\"t053\":\"0000000000000000\",\"t054\":\"0000000000000000\",\"t055\":\"0000000000000000\",\"t060\":\"0000000000000000\",\"t061\":\"0000000000000000\",\"t062\":\"0000000000000000\",\"t063\":\"0000000000000000\",\"t064\":\"0000000000000000\",\"t065\":\"0000000000000000\",\"t070\":\"0000000000000000\",\"t071\":\"0000000000000000\",\"t072\":\"0000000000000000\",\"t073\":\"0000000000000000\",\"t074\":\"0000000000000000\",\"t075\":\"0000000000000000\",\"t080\":\"0000000000000000\",\"t081\":\"0000000000000000\",\"t082\":\"0000000000000000\",\"t083\":\"0000000000000000\",\"t084\":\"0000000000000000\",\"t085\":\"0000000000000000\",\"t090\":\"0000000000000000\",\"t091\":\"0000000000000000\",\"t092\":\"0000000000000000\",\"t093\":\"0000000000000000\",\"t094\":\"0000000000000000\",\"t095\":\"0000000000000000\",\"t100\":\"0000000000000000\",\"t101\":\"0000000000000000\",\"t102\":\"0000000000000000\",\"t103\":\"0000000000000000\",\"t104\":\"0000000000000000\",\"t105\":\"0000000000000000\",\"t110\":\"0000000000000000\",\"t111\":\"0000000000000000\",\"t112\":\"0000000000000000\",\"t113\":\"0000000000000000\",\"t114\":\"0000000000000000\",\"t115\":\"0000000000000000\",\"t120\":\"0000000000000000\",\"t121\":\"0000000000000000\",\"t122\":\"0000000000000000\",\"t123\":\"0000000000000000\",\"t124\":\"0000000000000000\",\"t125\":\"0000000000000000\",\"t130\":\"0000000000000000\",\"t131\":\"0000000000000000\",\"t132\":\"0000000000000000\",\"t133\":\"0000000000000000\",\"t134\":\"0000000000000000\",\"t135\":\"0000000000000000\",\"t140\":\"0000000000000000\",\"t141\":\"0000000000000000\",\"t142\":\"0000000000000000\",\"t143\":\"0000000000000000\",\"t144\":\"0000000000000000\",\"t145\":\"0000000000000000\",\"t150\":\"0000000000000000\",\"t151\":\"0000000000000000\",\"t152\":\"0000000000000000\",\"t153\":\"0000000000000000\",\"t154\":\"0000000000000000\",\"t155\":\"0000000000000000\",\"t160\":\"0000000000000000\",\"t161\":\"0000000000000000\",\"t162\":\"0000000000000000\",\"t163\":\"0000000000000000\",\"t164\":\"0000000000000000\",\"t165\":\"0000000000000000\",\"t170\":\"0000000000000000\",\"t171\":\"0000000000000000\",\"t172\":\"0000000000000000\",\"t173\":\"0000000000000000\",\"t174\":\"0000000000000000\",\"t175\":\"0000000000000000\",\"t180\":\"0000000000000000\",\"t181\":\"0000000000000000\",\"t182\":\"0000000000000000\",\"t183\":\"0000000000000000\",\"t184\":\"0000000000000000\",\"t185\":\"0000000000000000\",\"t190\":\"0000000000000000\",\"t191\":\"0000000000000000\",\"t192\":\"0000000000000000\",\"t193\":\"0000000000000000\",\"t194\":\"0000000000000000\",\"t195\":\"0000000000000000\",\"t200\":\"0000000000000000\",\"t201\":\"0000000000000000\",\"t202\":\"0000000000000000\",\"t203\":\"0000000000000000\",\"t204\":\"0000000000000000\",\"t205\":\"0000000000000000\",\"t210\":\"0000000000000000\",\"t211\":\"0000000000000000\",\"t212\":\"0000000000000000\",\"t213\":\"0000000000000000\",\"t214\":\"0000000000000000\",\"t215\":\"0000000000000000\",\"t220\":\"0000000000000000\",\"t221\":\"0000000000000000\",\"t222\":\"0000000000000000\",\"t223\":\"0000000000000000\",\"t224\":\"0000000000000000\",\"t225\":\"0000000000000000\",\"t230\":\"0000000000000000\",\"t231\":\"0000000000000000\",\"t232\":\"0000000000000000\",\"t233\":\"0000000000000000\",\"t234\":\"0000000000000000\",\"t235\":\"0000000000000000\",\"tfix\":\"0000000000000000\"}");
    P1[144] = 0;
    P2[144] = 0;
    P3[144] = 0;
    P4[144] = 0;
    P5[144] = 0;
    P6[144] = 0;
    P7[144] = 0;
    P8[144] = 0;
    request->send(200, "text/html", "success");
  });

  // for ios wifi setting
  server.on("/online", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid") && request->hasParam("pwd"))
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

      request->send(200, "text/html", "success");
    }
    else
    {
      Serial.println("param error");
      request->send(200, "text/html", "failed");
    }
  });

  // for ios offline post
  server.on("/offlinepost", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("type", true) && request->hasParam("data", true))
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        return;
      }
      String type = request->getParam("type", true)->value();
      String data = request->getParam("data", true)->value();
      if (type == "mode")
      {
        //writeFile(SPIFFS, "/pwminfo.ini", data.c_str());
        Serial.println(data);
      }
      else if (type == "data")
      {
        //writeFile(SPIFFS, "/p.ini", data.c_str());
        Serial.println(data);
      }

      SPIFFS.end();
      request->send(200, "text/html", "success");
    }
    else
    {
      Serial.println("param error");
      request->send(200, "text/html", "failed");
    }
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

  // delete wifi.ini
  server.on("/delwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin(true))
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    deleteFile(SPIFFS, "/wifi.ini");
    SPIFFS.end();
    request->send(200, "text/html", "success");
  });

  // update operation
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!SPIFFS.begin())
    {
      Serial.println("SPIFFS Mount Failed");
      return;
    }
    String pwminfocontent = "{\"showtype\":\"fix\",\"testmode\":\"test\",\"sysdate\":\"{unixtick}\",\"status\":\"stop\",\"conmode\": \"none\",\"version\":\"{version_num}\"}";
    pwminfocontent.replace("{version_num}", VERSION_NUM);
    pwminfocontent.replace("{unixtick}", String(ESP_RTC_TICK));
    writeFile(SPIFFS, "/pwminfo.ini", pwminfocontent.c_str());
    SPIFFS.end();
    ESP.restart();
  },
            onFileUpload);

  server.on("/iosupdatetime", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("timestamp"))
    {
      String timestamp = request->getParam("timestamp")->value();
      ;
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
        request->send(200, "text/html", "failed");
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

      String filecontent;
      filecontent = "{\"showtype\":";
      filecontent = filecontent + showtype;
      filecontent = filecontent + ",\"testmode\":";
      filecontent = filecontent + testmode;
      filecontent = filecontent + ",\"sysdate\":\"";
      filecontent = filecontent + timestamp;
      filecontent = filecontent + "\",\"conmode\":";
      filecontent = filecontent + conmode;
      filecontent = filecontent + "}";
      PWM_INFO_RTC = sysdate;
      writeFile(SPIFFS, "/pwminfo.ini", filecontent.c_str());
      Serial.println(filecontent);
      SPIFFS.end();
      struct timeval stime;
      //stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 27726;
      stime.tv_sec = 1565004710 + 28800;
      settimeofday(&stime, NULL);
      request->send(200, "text/html", "success");
    }
    else
    {
      request->send(200, "text/html", "miss timestamp param");
    }
  });

  server.onNotFound(notFound);
  server.begin();
}

int getCurrentValue(int cval, int nval, int min)
{
  int result = 0;
  int diff = cval - nval;
  int p_diff = abs(cval - nval);
  //int p_val = p_diff / 10 ;
  float p_val = p_diff / 10.0;
  if (p_val < 1)
  {
    p_val = 1;
  }
  if (diff == 0)
  {
    result = cval;
  }
  else
  {
    if (diff > 0)
    {
      switch (min)
      {
      case 0:
        result = cval;
        break;
      case 1:
        result = cval - p_val;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 2:
        result = cval - p_val * 2;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 3:
        result = cval - p_val * 3;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 4:
        result = cval - p_val * 4;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 5:
        result = cval - p_val * 5;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 6:
        result = cval - p_val * 6;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 7:
        result = cval - p_val * 7;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 8:
        result = cval - p_val * 8;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      case 9:
        result = cval - p_val * 9;
        if (result <= nval)
        {
          result = nval + 1;
        }
        break;
      default:
        break;
      }
    }
    else
    {
      switch (min)
      {
      case 0:
        result = cval;
        break;
      case 1:
        result = cval + p_val;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 2:
        result = cval + p_val * 2;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 3:
        result = cval + p_val * 3;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 4:
        result = cval + p_val * 4;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 5:
        result = cval + p_val * 5;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 6:
        result = cval + p_val * 6;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 7:
        result = cval + p_val * 7;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 8:
        result = cval + p_val * 8;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      case 9:
        result = cval + p_val * 9;
        if (result >= nval)
        {
          result = nval - 1;
        }
        break;
      default:
        break;
      }
    }
  }
  //Serial.println(result);
  return result;
}

// esp32 loop operation
void loop()
{
  client.loop();
  // if (RESET_FLAG)
  // {
  //   Serial.println("Start to initialize the system configuration.");
  //   if (!SPIFFS.begin())
  //   {
  //     Serial.println("SPIFFS Mount Failed");
  //     return;
  //   }
  //   initFileSystem();
  //   SPIFFS.end();
  //   RESET_FLAG = false;
  //   ESP.restart();
  // }

  int currenthour;
  int currentmin;
  int currentsec;

  String wifistatus = "online";
  struct timeval stime;
  if (WiFi.status() != WL_CONNECTED)
  {
    if (timeClient.getEpochTime() < 28900 && stime.tv_sec < 28900)
    {
      if (!isUpaded)
      {
        //Serial.println("set time from RTC File");
        stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 27726;
        settimeofday(&stime, NULL);
        isUpaded = true;
        WiFi.softAP(ESP_HOST_NAME.c_str(), password);
      }
    }
    else if (timeClient.getEpochTime() > 1562000000)
    {
      //Serial.println("use offline npt value");
      currenthour = timeClient.getHours();
      currentmin = timeClient.getMinutes();
      currentsec = timeClient.getSeconds();
    }
    else
    {
      //Serial.println("use local time value");
      stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 27726;
      settimeofday(&stime, NULL);
      time_t t = time(NULL);
      struct tm *t_st;
      t_st = localtime(&t);
      currenthour = t_st->tm_hour;
      currentmin = t_st->tm_min;
      currentsec = t_st->tm_sec;
    }
    wifistatus = "offline";
  }
  else
  {
    if (timeClient.update())
    {
      currenthour = timeClient.getHours();
      currentmin = timeClient.getMinutes();
      currentsec = timeClient.getSeconds();
      PWM_INFO_RTC = timeClient.getEpochTime();
      stime.tv_sec = PWM_INFO_RTC.toInt();
      settimeofday(&stime, NULL);
    }
    else
    {
      Serial.println("update ntp failed");
      if (!isUpaded)
      {
        Serial.println("set time from RTC File");
        stime.tv_sec = atoi(PWM_INFO_RTC.c_str()) + 27726;
        settimeofday(&stime, NULL);
        isUpaded = true;
        WiFi.softAP(ESP_HOST_NAME.c_str(), password);
      }
      time_t t = time(NULL);
      struct tm *t_st;
      t_st = localtime(&t);
      currenthour = t_st->tm_hour;
      currentmin = t_st->tm_min;
      currentsec = t_st->tm_sec;
      //Serial.println(t_st, "%A, %B %d %Y %H:%M:%S");
    }
    if (PWM_INFO_CONMODE == "auto")
    {
      if (!SPIFFS.begin())
      {
        Serial.println("SPIFFS Mount Failed");
      }
      else
      {
        String latestversion = getFileString(SPIFFS, "/version.ini");
        SPIFFS.end();
        if (String(VERSION_NUM) != latestversion)
        {
          t_httpUpdate_return ret = httpUpdate.update(updateClient, "http://www.polypite.com/public/bin/firmware.bin");
          switch (ret)
          {
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;
          case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
          }
        }
      }
    }
    wifistatus = "online";
  }

  Serial.print(currenthour);
  Serial.print(":");
  Serial.print(currentmin);
  // Serial.print(":");
  // Serial.print(currentsec);
  Serial.println(wifistatus);
  // Serial.println("ntp:" + String(timeClient.getEpochTime()));
  // Serial.println("system:"+String(stime.tv_sec));

  if (currentmin == 0 || currentmin == 5 || currentmin == 10 || currentmin == 15 || currentmin == 20 || currentmin == 25 || currentmin == 30 || currentmin == 35 || currentmin == 40 || currentmin == 45 || currentmin == 50 || currentmin == 55)
  {
    if (true)
    {
      if (WiFi.status() != WL_CONNECTED && SSID != "")
      {
        led0.set(0);
        Serial.printf("try to reconnecting ..\n");
        WiFi.begin(SSID.c_str(), SSID_PWD.c_str());
        int i = 30;
        while ((WiFi.status() != WL_CONNECTED) && i > 0)
        {
          led0.set(100);
          delay(500);
          led0.set(0);
          Serial.printf(".");
          i = i - 1;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println(WiFi.localIP());
          led0.set(100);
          client.setServer(mqttServer, mqttPort);
          client.setCallback(callback);
          mqttconn();
        }
        else
        {
          WiFi.softAP(ESP_HOST_NAME.c_str(), password);
        }
      }
    }
  }

  if (true)
  {
    if (PWM_INFO_SHOWTYPE == "repeat")
    {

      if (PWM_INFO_TESTMODE == "production")
      {
        //Serial.printf("product mode .... current hour is: %d:%d:%d \r\n", currenthour,currentmin,currentsec);
        if (currenthour == 0)
        {
          if (currentmin < 10)
          {
            led1.set(getCurrentValue(P1[0], P1[1], currentmin));
            led2.set(getCurrentValue(P2[0], P2[1], currentmin));
            led3.set(getCurrentValue(P3[0], P3[1], currentmin));
            led4.set(getCurrentValue(P4[0], P4[1], currentmin));
            led5.set(getCurrentValue(P5[0], P5[1], currentmin));
            led6.set(getCurrentValue(P6[0], P6[1], currentmin));
            led7.set(getCurrentValue(P7[0], P7[1], currentmin));
            led8.set(getCurrentValue(P8[0], P8[1], currentmin));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[1], P1[2], min));
            led2.set(getCurrentValue(P2[1], P2[2], min));
            led3.set(getCurrentValue(P3[1], P3[2], min));
            led4.set(getCurrentValue(P4[1], P4[2], min));
            led5.set(getCurrentValue(P5[1], P5[2], min));
            led6.set(getCurrentValue(P6[1], P6[2], min));
            led7.set(getCurrentValue(P7[1], P7[2], min));
            led8.set(getCurrentValue(P8[1], P8[2], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[2], P1[3], min));
            led2.set(getCurrentValue(P2[2], P2[3], min));
            led3.set(getCurrentValue(P3[2], P3[3], min));
            led4.set(getCurrentValue(P4[2], P4[3], min));
            led5.set(getCurrentValue(P5[2], P5[3], min));
            led6.set(getCurrentValue(P6[2], P6[3], min));
            led7.set(getCurrentValue(P7[2], P7[3], min));
            led8.set(getCurrentValue(P8[2], P8[3], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {

            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[3], P1[4], min));
            led2.set(getCurrentValue(P2[3], P2[4], min));
            led3.set(getCurrentValue(P3[3], P3[4], min));
            led4.set(getCurrentValue(P4[3], P4[4], min));
            led5.set(getCurrentValue(P5[3], P5[4], min));
            led6.set(getCurrentValue(P6[3], P6[4], min));
            led7.set(getCurrentValue(P7[3], P7[4], min));
            led8.set(getCurrentValue(P8[3], P8[4], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[4], P1[5], min));
            led2.set(getCurrentValue(P2[4], P2[5], min));
            led3.set(getCurrentValue(P3[4], P3[5], min));
            led4.set(getCurrentValue(P4[4], P4[5], min));
            led5.set(getCurrentValue(P5[4], P5[5], min));
            led6.set(getCurrentValue(P6[4], P6[5], min));
            led7.set(getCurrentValue(P7[4], P7[5], min));
            led8.set(getCurrentValue(P8[4], P8[5], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[5], P1[6], min));
            led2.set(getCurrentValue(P2[5], P2[6], min));
            led3.set(getCurrentValue(P3[5], P3[6], min));
            led4.set(getCurrentValue(P4[5], P4[6], min));
            led5.set(getCurrentValue(P5[5], P5[6], min));
            led6.set(getCurrentValue(P6[5], P6[6], min));
            led7.set(getCurrentValue(P7[5], P7[6], min));
            led8.set(getCurrentValue(P8[5], P8[6], min));
          }
        }
        else if (currenthour == 1)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[6], P1[7], min));
            led2.set(getCurrentValue(P2[6], P2[7], min));
            led3.set(getCurrentValue(P3[6], P3[7], min));
            led4.set(getCurrentValue(P4[6], P4[7], min));
            led5.set(getCurrentValue(P5[6], P5[7], min));
            led6.set(getCurrentValue(P6[6], P6[7], min));
            led7.set(getCurrentValue(P7[6], P7[7], min));
            led8.set(getCurrentValue(P8[6], P8[7], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[7], P1[8], min));
            led2.set(getCurrentValue(P2[7], P2[8], min));
            led3.set(getCurrentValue(P3[7], P3[8], min));
            led4.set(getCurrentValue(P4[7], P4[8], min));
            led5.set(getCurrentValue(P5[7], P5[8], min));
            led6.set(getCurrentValue(P6[7], P6[8], min));
            led7.set(getCurrentValue(P7[7], P7[8], min));
            led8.set(getCurrentValue(P8[7], P8[8], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[8], P1[9], min));
            led2.set(getCurrentValue(P2[8], P2[9], min));
            led3.set(getCurrentValue(P3[8], P3[9], min));
            led4.set(getCurrentValue(P4[8], P4[9], min));
            led5.set(getCurrentValue(P5[8], P5[9], min));
            led6.set(getCurrentValue(P6[8], P6[9], min));
            led7.set(getCurrentValue(P7[8], P7[9], min));
            led8.set(getCurrentValue(P8[8], P8[9], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[9], P1[10], min));
            led2.set(getCurrentValue(P2[9], P2[10], min));
            led3.set(getCurrentValue(P3[9], P3[10], min));
            led4.set(getCurrentValue(P4[9], P4[10], min));
            led5.set(getCurrentValue(P5[9], P5[10], min));
            led6.set(getCurrentValue(P6[9], P6[10], min));
            led7.set(getCurrentValue(P7[9], P7[10], min));
            led8.set(getCurrentValue(P8[9], P8[10], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[10], P1[11], min));
            led2.set(getCurrentValue(P2[10], P2[11], min));
            led3.set(getCurrentValue(P3[10], P3[11], min));
            led4.set(getCurrentValue(P4[10], P4[11], min));
            led5.set(getCurrentValue(P5[10], P5[11], min));
            led6.set(getCurrentValue(P6[10], P6[11], min));
            led7.set(getCurrentValue(P7[10], P7[11], min));
            led8.set(getCurrentValue(P8[10], P8[11], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[11], P1[12], min));
            led2.set(getCurrentValue(P2[11], P2[12], min));
            led3.set(getCurrentValue(P3[11], P3[12], min));
            led4.set(getCurrentValue(P4[11], P4[12], min));
            led5.set(getCurrentValue(P5[11], P5[12], min));
            led6.set(getCurrentValue(P6[11], P6[12], min));
            led7.set(getCurrentValue(P7[11], P7[12], min));
            led8.set(getCurrentValue(P8[11], P8[12], min));
          }
        }
        else if (currenthour == 2)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[12], P1[13], min));
            led2.set(getCurrentValue(P2[12], P2[13], min));
            led3.set(getCurrentValue(P3[12], P3[13], min));
            led4.set(getCurrentValue(P4[12], P4[13], min));
            led5.set(getCurrentValue(P5[12], P5[13], min));
            led6.set(getCurrentValue(P6[12], P6[13], min));
            led7.set(getCurrentValue(P7[12], P7[13], min));
            led8.set(getCurrentValue(P8[12], P8[13], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[13], P1[14], min));
            led2.set(getCurrentValue(P2[13], P2[14], min));
            led3.set(getCurrentValue(P3[13], P3[14], min));
            led4.set(getCurrentValue(P4[13], P4[14], min));
            led5.set(getCurrentValue(P5[13], P5[14], min));
            led6.set(getCurrentValue(P6[13], P6[14], min));
            led7.set(getCurrentValue(P7[13], P7[14], min));
            led8.set(getCurrentValue(P8[13], P8[14], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[14], P1[15], min));
            led2.set(getCurrentValue(P2[14], P2[15], min));
            led3.set(getCurrentValue(P3[14], P3[15], min));
            led4.set(getCurrentValue(P4[14], P4[15], min));
            led5.set(getCurrentValue(P5[14], P5[15], min));
            led6.set(getCurrentValue(P6[14], P6[15], min));
            led7.set(getCurrentValue(P7[14], P7[15], min));
            led8.set(getCurrentValue(P8[14], P8[15], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[15], P1[16], min));
            led2.set(getCurrentValue(P2[15], P2[16], min));
            led3.set(getCurrentValue(P3[15], P3[16], min));
            led4.set(getCurrentValue(P4[15], P4[16], min));
            led5.set(getCurrentValue(P5[15], P5[16], min));
            led6.set(getCurrentValue(P6[15], P6[16], min));
            led7.set(getCurrentValue(P7[15], P7[16], min));
            led8.set(getCurrentValue(P8[15], P8[16], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[16], P1[17], min));
            led2.set(getCurrentValue(P2[16], P2[17], min));
            led3.set(getCurrentValue(P3[16], P3[17], min));
            led4.set(getCurrentValue(P4[16], P4[17], min));
            led5.set(getCurrentValue(P5[16], P5[17], min));
            led6.set(getCurrentValue(P6[16], P6[17], min));
            led7.set(getCurrentValue(P7[16], P7[17], min));
            led8.set(getCurrentValue(P8[16], P8[17], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[17], P1[18], min));
            led2.set(getCurrentValue(P2[17], P2[18], min));
            led3.set(getCurrentValue(P3[17], P3[18], min));
            led4.set(getCurrentValue(P4[17], P4[18], min));
            led5.set(getCurrentValue(P5[17], P5[18], min));
            led6.set(getCurrentValue(P6[17], P6[18], min));
            led7.set(getCurrentValue(P7[17], P7[18], min));
            led8.set(getCurrentValue(P8[17], P8[18], min));
          }
        }
        else if (currenthour == 3)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[18], P1[19], min));
            led2.set(getCurrentValue(P2[18], P2[19], min));
            led3.set(getCurrentValue(P3[18], P3[19], min));
            led4.set(getCurrentValue(P4[18], P4[19], min));
            led5.set(getCurrentValue(P5[18], P5[19], min));
            led6.set(getCurrentValue(P6[18], P6[19], min));
            led7.set(getCurrentValue(P7[18], P7[19], min));
            led8.set(getCurrentValue(P8[18], P8[19], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[19], P1[20], min));
            led2.set(getCurrentValue(P2[19], P2[20], min));
            led3.set(getCurrentValue(P3[19], P3[20], min));
            led4.set(getCurrentValue(P4[19], P4[20], min));
            led5.set(getCurrentValue(P5[19], P5[20], min));
            led6.set(getCurrentValue(P6[19], P6[20], min));
            led7.set(getCurrentValue(P7[19], P7[20], min));
            led8.set(getCurrentValue(P8[19], P8[20], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[20], P1[21], min));
            led2.set(getCurrentValue(P2[20], P2[21], min));
            led3.set(getCurrentValue(P3[20], P3[21], min));
            led4.set(getCurrentValue(P4[20], P4[21], min));
            led5.set(getCurrentValue(P5[20], P5[21], min));
            led6.set(getCurrentValue(P6[20], P6[21], min));
            led7.set(getCurrentValue(P7[20], P7[21], min));
            led8.set(getCurrentValue(P8[20], P8[21], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[21], P1[22], min));
            led2.set(getCurrentValue(P2[21], P2[22], min));
            led3.set(getCurrentValue(P3[21], P3[22], min));
            led4.set(getCurrentValue(P4[21], P4[22], min));
            led5.set(getCurrentValue(P5[21], P5[22], min));
            led6.set(getCurrentValue(P6[21], P6[22], min));
            led7.set(getCurrentValue(P7[21], P7[22], min));
            led8.set(getCurrentValue(P8[21], P8[22], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[22], P1[23], min));
            led2.set(getCurrentValue(P2[22], P2[23], min));
            led3.set(getCurrentValue(P3[22], P3[23], min));
            led4.set(getCurrentValue(P4[22], P4[23], min));
            led5.set(getCurrentValue(P5[22], P5[23], min));
            led6.set(getCurrentValue(P6[22], P6[23], min));
            led7.set(getCurrentValue(P7[22], P7[23], min));
            led8.set(getCurrentValue(P8[22], P8[23], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[23], P1[24], min));
            led2.set(getCurrentValue(P2[23], P2[24], min));
            led3.set(getCurrentValue(P3[23], P3[24], min));
            led4.set(getCurrentValue(P4[23], P4[24], min));
            led5.set(getCurrentValue(P5[23], P5[24], min));
            led6.set(getCurrentValue(P6[23], P6[24], min));
            led7.set(getCurrentValue(P7[23], P7[24], min));
            led8.set(getCurrentValue(P8[23], P8[24], min));
          }
        }
        else if (currenthour == 4)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[24], P1[25], min));
            led2.set(getCurrentValue(P2[24], P2[25], min));
            led3.set(getCurrentValue(P3[24], P3[25], min));
            led4.set(getCurrentValue(P4[24], P4[25], min));
            led5.set(getCurrentValue(P5[24], P5[25], min));
            led6.set(getCurrentValue(P6[24], P6[25], min));
            led7.set(getCurrentValue(P7[24], P7[25], min));
            led8.set(getCurrentValue(P8[24], P8[25], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[25], P1[26], min));
            led2.set(getCurrentValue(P2[25], P2[26], min));
            led3.set(getCurrentValue(P3[25], P3[26], min));
            led4.set(getCurrentValue(P4[25], P4[26], min));
            led5.set(getCurrentValue(P5[25], P5[26], min));
            led6.set(getCurrentValue(P6[25], P6[26], min));
            led7.set(getCurrentValue(P7[25], P7[26], min));
            led8.set(getCurrentValue(P8[25], P8[26], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[26], P1[27], min));
            led2.set(getCurrentValue(P2[26], P2[27], min));
            led3.set(getCurrentValue(P3[26], P3[27], min));
            led4.set(getCurrentValue(P4[26], P4[27], min));
            led5.set(getCurrentValue(P5[26], P5[27], min));
            led6.set(getCurrentValue(P6[26], P6[27], min));
            led7.set(getCurrentValue(P7[26], P7[27], min));
            led8.set(getCurrentValue(P8[26], P8[27], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[27], P1[28], min));
            led2.set(getCurrentValue(P2[27], P2[28], min));
            led3.set(getCurrentValue(P3[27], P3[28], min));
            led4.set(getCurrentValue(P4[27], P4[28], min));
            led5.set(getCurrentValue(P5[27], P5[28], min));
            led6.set(getCurrentValue(P6[27], P6[28], min));
            led7.set(getCurrentValue(P7[27], P7[28], min));
            led8.set(getCurrentValue(P8[27], P8[28], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[28], P1[29], min));
            led2.set(getCurrentValue(P2[28], P2[29], min));
            led3.set(getCurrentValue(P3[28], P3[29], min));
            led4.set(getCurrentValue(P4[28], P4[29], min));
            led5.set(getCurrentValue(P5[28], P5[29], min));
            led6.set(getCurrentValue(P6[28], P6[29], min));
            led7.set(getCurrentValue(P7[28], P7[29], min));
            led8.set(getCurrentValue(P8[28], P8[29], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[29], P1[30], min));
            led2.set(getCurrentValue(P2[29], P2[30], min));
            led3.set(getCurrentValue(P3[29], P3[30], min));
            led4.set(getCurrentValue(P4[29], P4[30], min));
            led5.set(getCurrentValue(P5[29], P5[30], min));
            led6.set(getCurrentValue(P6[29], P6[30], min));
            led7.set(getCurrentValue(P7[29], P7[30], min));
            led8.set(getCurrentValue(P8[29], P8[30], min));
          }
        }
        else if (currenthour == 5)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[30], P1[31], min));
            led2.set(getCurrentValue(P2[30], P2[31], min));
            led3.set(getCurrentValue(P3[30], P3[31], min));
            led4.set(getCurrentValue(P4[30], P4[31], min));
            led5.set(getCurrentValue(P5[30], P5[31], min));
            led6.set(getCurrentValue(P6[30], P6[31], min));
            led7.set(getCurrentValue(P7[30], P7[31], min));
            led8.set(getCurrentValue(P8[30], P8[31], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[31], P1[32], min));
            led2.set(getCurrentValue(P2[31], P2[32], min));
            led3.set(getCurrentValue(P3[31], P3[32], min));
            led4.set(getCurrentValue(P4[31], P4[32], min));
            led5.set(getCurrentValue(P5[31], P5[32], min));
            led6.set(getCurrentValue(P6[31], P6[32], min));
            led7.set(getCurrentValue(P7[31], P7[32], min));
            led8.set(getCurrentValue(P8[31], P8[32], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[32], P1[33], min));
            led2.set(getCurrentValue(P2[32], P2[33], min));
            led3.set(getCurrentValue(P3[32], P3[33], min));
            led4.set(getCurrentValue(P4[32], P4[33], min));
            led5.set(getCurrentValue(P5[32], P5[33], min));
            led6.set(getCurrentValue(P6[32], P6[33], min));
            led7.set(getCurrentValue(P7[32], P7[33], min));
            led8.set(getCurrentValue(P8[32], P8[33], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[33], P1[34], min));
            led2.set(getCurrentValue(P2[33], P2[34], min));
            led3.set(getCurrentValue(P3[33], P3[34], min));
            led4.set(getCurrentValue(P4[33], P4[34], min));
            led5.set(getCurrentValue(P5[33], P5[34], min));
            led6.set(getCurrentValue(P6[33], P6[34], min));
            led7.set(getCurrentValue(P7[33], P7[34], min));
            led8.set(getCurrentValue(P8[33], P8[34], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[34], P1[35], min));
            led2.set(getCurrentValue(P2[34], P2[35], min));
            led3.set(getCurrentValue(P3[34], P3[35], min));
            led4.set(getCurrentValue(P4[34], P4[35], min));
            led5.set(getCurrentValue(P5[34], P5[35], min));
            led6.set(getCurrentValue(P6[34], P6[35], min));
            led7.set(getCurrentValue(P7[34], P7[35], min));
            led8.set(getCurrentValue(P8[34], P8[35], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[35], P1[36], min));
            led2.set(getCurrentValue(P2[35], P2[36], min));
            led3.set(getCurrentValue(P3[35], P3[36], min));
            led4.set(getCurrentValue(P4[35], P4[36], min));
            led5.set(getCurrentValue(P5[35], P5[36], min));
            led6.set(getCurrentValue(P6[35], P6[36], min));
            led7.set(getCurrentValue(P7[35], P7[36], min));
            led8.set(getCurrentValue(P8[35], P8[36], min));
          }
        }
        else if (currenthour == 6)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[36], P1[37], min));
            led2.set(getCurrentValue(P2[36], P2[37], min));
            led3.set(getCurrentValue(P3[36], P3[37], min));
            led4.set(getCurrentValue(P4[36], P4[37], min));
            led5.set(getCurrentValue(P5[36], P5[37], min));
            led6.set(getCurrentValue(P6[36], P6[37], min));
            led7.set(getCurrentValue(P7[36], P7[37], min));
            led8.set(getCurrentValue(P8[36], P8[37], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[37], P1[38], min));
            led2.set(getCurrentValue(P2[37], P2[38], min));
            led3.set(getCurrentValue(P3[37], P3[38], min));
            led4.set(getCurrentValue(P4[37], P4[38], min));
            led5.set(getCurrentValue(P5[37], P5[38], min));
            led6.set(getCurrentValue(P6[37], P6[38], min));
            led7.set(getCurrentValue(P7[37], P7[38], min));
            led8.set(getCurrentValue(P8[37], P8[38], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[38], P1[39], min));
            led2.set(getCurrentValue(P2[38], P2[39], min));
            led3.set(getCurrentValue(P3[38], P3[39], min));
            led4.set(getCurrentValue(P4[38], P4[39], min));
            led5.set(getCurrentValue(P5[38], P5[39], min));
            led6.set(getCurrentValue(P6[38], P6[39], min));
            led7.set(getCurrentValue(P7[38], P7[39], min));
            led8.set(getCurrentValue(P8[38], P8[39], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[39], P1[40], min));
            led2.set(getCurrentValue(P2[39], P2[40], min));
            led3.set(getCurrentValue(P3[39], P3[40], min));
            led4.set(getCurrentValue(P4[39], P4[40], min));
            led5.set(getCurrentValue(P5[39], P5[40], min));
            led6.set(getCurrentValue(P6[39], P6[40], min));
            led7.set(getCurrentValue(P7[39], P7[40], min));
            led8.set(getCurrentValue(P8[39], P8[40], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[40], P1[41], min));
            led2.set(getCurrentValue(P2[40], P2[41], min));
            led3.set(getCurrentValue(P3[40], P3[41], min));
            led4.set(getCurrentValue(P4[40], P4[41], min));
            led5.set(getCurrentValue(P5[40], P5[41], min));
            led6.set(getCurrentValue(P6[40], P6[41], min));
            led7.set(getCurrentValue(P7[40], P7[41], min));
            led8.set(getCurrentValue(P8[40], P8[41], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[41], P1[42], min));
            led2.set(getCurrentValue(P2[41], P2[42], min));
            led3.set(getCurrentValue(P3[41], P3[42], min));
            led4.set(getCurrentValue(P4[41], P4[42], min));
            led5.set(getCurrentValue(P5[41], P5[42], min));
            led6.set(getCurrentValue(P6[41], P6[42], min));
            led7.set(getCurrentValue(P7[41], P7[42], min));
            led8.set(getCurrentValue(P8[41], P8[42], min));
          }
        }
        else if (currenthour == 7)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[42], P1[43], min));
            led2.set(getCurrentValue(P2[42], P2[43], min));
            led3.set(getCurrentValue(P3[42], P3[43], min));
            led4.set(getCurrentValue(P4[42], P4[43], min));
            led5.set(getCurrentValue(P5[42], P5[43], min));
            led6.set(getCurrentValue(P6[42], P6[43], min));
            led7.set(getCurrentValue(P7[42], P7[43], min));
            led8.set(getCurrentValue(P8[42], P8[43], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[43], P1[44], min));
            led2.set(getCurrentValue(P2[43], P2[44], min));
            led3.set(getCurrentValue(P3[43], P3[44], min));
            led4.set(getCurrentValue(P4[43], P4[44], min));
            led5.set(getCurrentValue(P5[43], P5[44], min));
            led6.set(getCurrentValue(P6[43], P6[44], min));
            led7.set(getCurrentValue(P7[43], P7[44], min));
            led8.set(getCurrentValue(P8[43], P8[44], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[44], P1[45], min));
            led2.set(getCurrentValue(P2[44], P2[45], min));
            led3.set(getCurrentValue(P3[44], P3[45], min));
            led4.set(getCurrentValue(P4[44], P4[45], min));
            led5.set(getCurrentValue(P5[44], P5[45], min));
            led6.set(getCurrentValue(P6[44], P6[45], min));
            led7.set(getCurrentValue(P7[44], P7[45], min));
            led8.set(getCurrentValue(P8[44], P8[45], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[45], P1[46], min));
            led2.set(getCurrentValue(P2[45], P2[46], min));
            led3.set(getCurrentValue(P3[45], P3[46], min));
            led4.set(getCurrentValue(P4[45], P4[46], min));
            led5.set(getCurrentValue(P5[45], P5[46], min));
            led6.set(getCurrentValue(P6[45], P6[46], min));
            led7.set(getCurrentValue(P7[45], P7[46], min));
            led8.set(getCurrentValue(P8[45], P8[46], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[46], P1[47], min));
            led2.set(getCurrentValue(P2[46], P2[47], min));
            led3.set(getCurrentValue(P3[46], P3[47], min));
            led4.set(getCurrentValue(P4[46], P4[47], min));
            led5.set(getCurrentValue(P5[46], P5[47], min));
            led6.set(getCurrentValue(P6[46], P6[47], min));
            led7.set(getCurrentValue(P7[46], P7[47], min));
            led8.set(getCurrentValue(P8[46], P8[47], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[47], P1[48], min));
            led2.set(getCurrentValue(P2[47], P2[48], min));
            led3.set(getCurrentValue(P3[47], P3[48], min));
            led4.set(getCurrentValue(P4[47], P4[48], min));
            led5.set(getCurrentValue(P5[47], P5[48], min));
            led6.set(getCurrentValue(P6[47], P6[48], min));
            led7.set(getCurrentValue(P7[47], P7[48], min));
            led8.set(getCurrentValue(P8[47], P8[48], min));
          }
        }
        else if (currenthour == 8)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[48], P1[49], min));
            led2.set(getCurrentValue(P2[48], P2[49], min));
            led3.set(getCurrentValue(P3[48], P3[49], min));
            led4.set(getCurrentValue(P4[48], P4[49], min));
            led5.set(getCurrentValue(P5[48], P5[49], min));
            led6.set(getCurrentValue(P6[48], P6[49], min));
            led7.set(getCurrentValue(P7[48], P7[49], min));
            led8.set(getCurrentValue(P8[48], P8[49], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[49], P1[50], min));
            led2.set(getCurrentValue(P2[49], P2[50], min));
            led3.set(getCurrentValue(P3[49], P3[50], min));
            led4.set(getCurrentValue(P4[49], P4[50], min));
            led5.set(getCurrentValue(P5[49], P5[50], min));
            led6.set(getCurrentValue(P6[49], P6[50], min));
            led7.set(getCurrentValue(P7[49], P7[50], min));
            led8.set(getCurrentValue(P8[49], P8[50], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[50], P1[51], min));
            led2.set(getCurrentValue(P2[50], P2[51], min));
            led3.set(getCurrentValue(P3[50], P3[51], min));
            led4.set(getCurrentValue(P4[50], P4[51], min));
            led5.set(getCurrentValue(P5[50], P5[51], min));
            led6.set(getCurrentValue(P6[50], P6[51], min));
            led7.set(getCurrentValue(P7[50], P7[51], min));
            led8.set(getCurrentValue(P8[50], P8[51], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[51], P1[52], min));
            led2.set(getCurrentValue(P2[51], P2[52], min));
            led3.set(getCurrentValue(P3[51], P3[52], min));
            led4.set(getCurrentValue(P4[51], P4[52], min));
            led5.set(getCurrentValue(P5[51], P5[52], min));
            led6.set(getCurrentValue(P6[51], P6[52], min));
            led7.set(getCurrentValue(P7[51], P7[52], min));
            led8.set(getCurrentValue(P8[51], P8[52], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[52], P1[53], min));
            led2.set(getCurrentValue(P2[52], P2[53], min));
            led3.set(getCurrentValue(P3[52], P3[53], min));
            led4.set(getCurrentValue(P4[52], P4[53], min));
            led5.set(getCurrentValue(P5[52], P5[53], min));
            led6.set(getCurrentValue(P6[52], P6[53], min));
            led7.set(getCurrentValue(P7[52], P7[53], min));
            led8.set(getCurrentValue(P8[52], P8[53], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[53], P1[54], min));
            led2.set(getCurrentValue(P2[53], P2[54], min));
            led3.set(getCurrentValue(P3[53], P3[54], min));
            led4.set(getCurrentValue(P4[53], P4[54], min));
            led5.set(getCurrentValue(P5[53], P5[54], min));
            led6.set(getCurrentValue(P6[53], P6[54], min));
            led7.set(getCurrentValue(P7[53], P7[54], min));
            led8.set(getCurrentValue(P8[53], P8[54], min));
          }
        }
        else if (currenthour == 9)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[54], P1[55], min));
            led2.set(getCurrentValue(P2[54], P2[55], min));
            led3.set(getCurrentValue(P3[54], P3[55], min));
            led4.set(getCurrentValue(P4[54], P4[55], min));
            led5.set(getCurrentValue(P5[54], P5[55], min));
            led6.set(getCurrentValue(P6[54], P6[55], min));
            led7.set(getCurrentValue(P7[54], P7[55], min));
            led8.set(getCurrentValue(P8[54], P8[55], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[55], P1[56], min));
            led2.set(getCurrentValue(P2[55], P2[56], min));
            led3.set(getCurrentValue(P3[55], P3[56], min));
            led4.set(getCurrentValue(P4[55], P4[56], min));
            led5.set(getCurrentValue(P5[55], P5[56], min));
            led6.set(getCurrentValue(P6[55], P6[56], min));
            led7.set(getCurrentValue(P7[55], P7[56], min));
            led8.set(getCurrentValue(P8[55], P8[56], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[56], P1[57], min));
            led2.set(getCurrentValue(P2[56], P2[57], min));
            led3.set(getCurrentValue(P3[56], P3[57], min));
            led4.set(getCurrentValue(P4[56], P4[57], min));
            led5.set(getCurrentValue(P5[56], P5[57], min));
            led6.set(getCurrentValue(P6[56], P6[57], min));
            led7.set(getCurrentValue(P7[56], P7[57], min));
            led8.set(getCurrentValue(P8[56], P8[57], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[57], P1[58], min));
            led2.set(getCurrentValue(P2[57], P2[58], min));
            led3.set(getCurrentValue(P3[57], P3[58], min));
            led4.set(getCurrentValue(P4[57], P4[58], min));
            led5.set(getCurrentValue(P5[57], P5[58], min));
            led6.set(getCurrentValue(P6[57], P6[58], min));
            led7.set(getCurrentValue(P7[57], P7[58], min));
            led8.set(getCurrentValue(P8[57], P8[58], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[58], P1[59], min));
            led2.set(getCurrentValue(P2[58], P2[59], min));
            led3.set(getCurrentValue(P3[58], P3[59], min));
            led4.set(getCurrentValue(P4[58], P4[59], min));
            led5.set(getCurrentValue(P5[58], P5[59], min));
            led6.set(getCurrentValue(P6[58], P6[59], min));
            led7.set(getCurrentValue(P7[58], P7[59], min));
            led8.set(getCurrentValue(P8[58], P8[59], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[59], P1[60], min));
            led2.set(getCurrentValue(P2[59], P2[60], min));
            led3.set(getCurrentValue(P3[59], P3[60], min));
            led4.set(getCurrentValue(P4[59], P4[60], min));
            led5.set(getCurrentValue(P5[59], P5[60], min));
            led6.set(getCurrentValue(P6[59], P6[60], min));
            led7.set(getCurrentValue(P7[59], P7[60], min));
            led8.set(getCurrentValue(P8[59], P8[60], min));
          }
        }
        else if (currenthour == 10)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[60], P1[61], min));
            led2.set(getCurrentValue(P2[60], P2[61], min));
            led3.set(getCurrentValue(P3[60], P3[61], min));
            led4.set(getCurrentValue(P4[60], P4[61], min));
            led5.set(getCurrentValue(P5[60], P5[61], min));
            led6.set(getCurrentValue(P6[60], P6[61], min));
            led7.set(getCurrentValue(P7[60], P7[61], min));
            led8.set(getCurrentValue(P8[60], P8[61], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[61], P1[62], min));
            led2.set(getCurrentValue(P2[61], P2[62], min));
            led3.set(getCurrentValue(P3[61], P3[62], min));
            led4.set(getCurrentValue(P4[61], P4[62], min));
            led5.set(getCurrentValue(P5[61], P5[62], min));
            led6.set(getCurrentValue(P6[61], P6[62], min));
            led7.set(getCurrentValue(P7[61], P7[62], min));
            led8.set(getCurrentValue(P8[61], P8[62], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[62], P1[63], min));
            led2.set(getCurrentValue(P2[62], P2[63], min));
            led3.set(getCurrentValue(P3[62], P3[63], min));
            led4.set(getCurrentValue(P4[62], P4[63], min));
            led5.set(getCurrentValue(P5[62], P5[63], min));
            led6.set(getCurrentValue(P6[62], P6[63], min));
            led7.set(getCurrentValue(P7[62], P7[63], min));
            led8.set(getCurrentValue(P8[62], P8[63], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[63], P1[64], min));
            led2.set(getCurrentValue(P2[63], P2[64], min));
            led3.set(getCurrentValue(P3[63], P3[64], min));
            led4.set(getCurrentValue(P4[63], P4[64], min));
            led5.set(getCurrentValue(P5[63], P5[64], min));
            led6.set(getCurrentValue(P6[63], P6[64], min));
            led7.set(getCurrentValue(P7[63], P7[64], min));
            led8.set(getCurrentValue(P8[63], P8[64], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[64], P1[65], min));
            led2.set(getCurrentValue(P2[64], P2[65], min));
            led3.set(getCurrentValue(P3[64], P3[65], min));
            led4.set(getCurrentValue(P4[64], P4[65], min));
            led5.set(getCurrentValue(P5[64], P5[65], min));
            led6.set(getCurrentValue(P6[64], P6[65], min));
            led7.set(getCurrentValue(P7[64], P7[65], min));
            led8.set(getCurrentValue(P8[64], P8[65], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[65], P1[66], min));
            led2.set(getCurrentValue(P2[65], P2[66], min));
            led3.set(getCurrentValue(P3[65], P3[66], min));
            led4.set(getCurrentValue(P4[65], P4[66], min));
            led5.set(getCurrentValue(P5[65], P5[66], min));
            led6.set(getCurrentValue(P6[65], P6[66], min));
            led7.set(getCurrentValue(P7[65], P7[66], min));
            led8.set(getCurrentValue(P8[65], P8[66], min));
          }
        }
        else if (currenthour == 11)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[66], P1[67], min));
            led2.set(getCurrentValue(P2[66], P2[67], min));
            led3.set(getCurrentValue(P3[66], P3[67], min));
            led4.set(getCurrentValue(P4[66], P4[67], min));
            led5.set(getCurrentValue(P5[66], P5[67], min));
            led6.set(getCurrentValue(P6[66], P6[67], min));
            led7.set(getCurrentValue(P7[66], P7[67], min));
            led8.set(getCurrentValue(P8[66], P8[67], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[67], P1[68], min));
            led2.set(getCurrentValue(P2[67], P2[68], min));
            led3.set(getCurrentValue(P3[67], P3[68], min));
            led4.set(getCurrentValue(P4[67], P4[68], min));
            led5.set(getCurrentValue(P5[67], P5[68], min));
            led6.set(getCurrentValue(P6[67], P6[68], min));
            led7.set(getCurrentValue(P7[67], P7[68], min));
            led8.set(getCurrentValue(P8[67], P8[68], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[68], P1[69], min));
            led2.set(getCurrentValue(P2[68], P2[69], min));
            led3.set(getCurrentValue(P3[68], P3[69], min));
            led4.set(getCurrentValue(P4[68], P4[69], min));
            led5.set(getCurrentValue(P5[68], P5[69], min));
            led6.set(getCurrentValue(P6[68], P6[69], min));
            led7.set(getCurrentValue(P7[68], P7[69], min));
            led8.set(getCurrentValue(P8[68], P8[69], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[69], P1[70], min));
            led2.set(getCurrentValue(P2[69], P2[70], min));
            led3.set(getCurrentValue(P3[69], P3[70], min));
            led4.set(getCurrentValue(P4[69], P4[70], min));
            led5.set(getCurrentValue(P5[69], P5[70], min));
            led6.set(getCurrentValue(P6[69], P6[70], min));
            led7.set(getCurrentValue(P7[69], P7[70], min));
            led8.set(getCurrentValue(P8[69], P8[70], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[70], P1[71], min));
            led2.set(getCurrentValue(P2[70], P2[71], min));
            led3.set(getCurrentValue(P3[70], P3[71], min));
            led4.set(getCurrentValue(P4[70], P4[71], min));
            led5.set(getCurrentValue(P5[70], P5[71], min));
            led6.set(getCurrentValue(P6[70], P6[71], min));
            led7.set(getCurrentValue(P7[70], P7[71], min));
            led8.set(getCurrentValue(P8[70], P8[71], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[71], P1[72], min));
            led2.set(getCurrentValue(P2[71], P2[72], min));
            led3.set(getCurrentValue(P3[71], P3[72], min));
            led4.set(getCurrentValue(P4[71], P4[72], min));
            led5.set(getCurrentValue(P5[71], P5[72], min));
            led6.set(getCurrentValue(P6[71], P6[72], min));
            led7.set(getCurrentValue(P7[71], P7[72], min));
            led8.set(getCurrentValue(P8[71], P8[72], min));
          }
        }
        else if (currenthour == 12)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[72], P1[73], min));
            led2.set(getCurrentValue(P2[72], P2[73], min));
            led3.set(getCurrentValue(P3[72], P3[73], min));
            led4.set(getCurrentValue(P4[72], P4[73], min));
            led5.set(getCurrentValue(P5[72], P5[73], min));
            led6.set(getCurrentValue(P6[72], P6[73], min));
            led7.set(getCurrentValue(P7[72], P7[73], min));
            led8.set(getCurrentValue(P8[72], P8[73], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[73], P1[74], min));
            led2.set(getCurrentValue(P2[73], P2[74], min));
            led3.set(getCurrentValue(P3[73], P3[74], min));
            led4.set(getCurrentValue(P4[73], P4[74], min));
            led5.set(getCurrentValue(P5[73], P5[74], min));
            led6.set(getCurrentValue(P6[73], P6[74], min));
            led7.set(getCurrentValue(P7[73], P7[74], min));
            led8.set(getCurrentValue(P8[73], P8[74], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[74], P1[75], min));
            led2.set(getCurrentValue(P2[74], P2[75], min));
            led3.set(getCurrentValue(P3[74], P3[75], min));
            led4.set(getCurrentValue(P4[74], P4[75], min));
            led5.set(getCurrentValue(P5[74], P5[75], min));
            led6.set(getCurrentValue(P6[74], P6[75], min));
            led7.set(getCurrentValue(P7[74], P7[75], min));
            led8.set(getCurrentValue(P8[74], P8[75], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[75], P1[76], min));
            led2.set(getCurrentValue(P2[75], P2[76], min));
            led3.set(getCurrentValue(P3[75], P3[76], min));
            led4.set(getCurrentValue(P4[75], P4[76], min));
            led5.set(getCurrentValue(P5[75], P5[76], min));
            led6.set(getCurrentValue(P6[75], P6[76], min));
            led7.set(getCurrentValue(P7[75], P7[76], min));
            led8.set(getCurrentValue(P8[75], P8[76], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[76], P1[77], min));
            led2.set(getCurrentValue(P2[76], P2[77], min));
            led3.set(getCurrentValue(P3[76], P3[77], min));
            led4.set(getCurrentValue(P4[76], P4[77], min));
            led5.set(getCurrentValue(P5[76], P5[77], min));
            led6.set(getCurrentValue(P6[76], P6[77], min));
            led7.set(getCurrentValue(P7[76], P7[77], min));
            led8.set(getCurrentValue(P8[76], P8[77], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[77], P1[78], min));
            led2.set(getCurrentValue(P2[77], P2[78], min));
            led3.set(getCurrentValue(P3[77], P3[78], min));
            led4.set(getCurrentValue(P4[77], P4[78], min));
            led5.set(getCurrentValue(P5[77], P5[78], min));
            led6.set(getCurrentValue(P6[77], P6[78], min));
            led7.set(getCurrentValue(P7[77], P7[78], min));
            led8.set(getCurrentValue(P8[77], P8[78], min));
          }
        }
        else if (currenthour == 13)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[78], P1[79], min));
            led2.set(getCurrentValue(P2[78], P2[79], min));
            led3.set(getCurrentValue(P3[78], P3[79], min));
            led4.set(getCurrentValue(P4[78], P4[79], min));
            led5.set(getCurrentValue(P5[78], P5[79], min));
            led6.set(getCurrentValue(P6[78], P6[79], min));
            led7.set(getCurrentValue(P7[78], P7[79], min));
            led8.set(getCurrentValue(P8[78], P8[79], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[79], P1[80], min));
            led2.set(getCurrentValue(P2[79], P2[80], min));
            led3.set(getCurrentValue(P3[79], P3[80], min));
            led4.set(getCurrentValue(P4[79], P4[80], min));
            led5.set(getCurrentValue(P5[79], P5[80], min));
            led6.set(getCurrentValue(P6[79], P6[80], min));
            led7.set(getCurrentValue(P7[79], P7[80], min));
            led8.set(getCurrentValue(P8[79], P8[80], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[80], P1[81], min));
            led2.set(getCurrentValue(P2[80], P2[81], min));
            led3.set(getCurrentValue(P3[80], P3[81], min));
            led4.set(getCurrentValue(P4[80], P4[81], min));
            led5.set(getCurrentValue(P5[80], P5[81], min));
            led6.set(getCurrentValue(P6[80], P6[81], min));
            led7.set(getCurrentValue(P7[80], P7[81], min));
            led8.set(getCurrentValue(P8[80], P8[81], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[81], P1[82], min));
            led2.set(getCurrentValue(P2[81], P2[82], min));
            led3.set(getCurrentValue(P3[81], P3[82], min));
            led4.set(getCurrentValue(P4[81], P4[82], min));
            led5.set(getCurrentValue(P5[81], P5[82], min));
            led6.set(getCurrentValue(P6[81], P6[82], min));
            led7.set(getCurrentValue(P7[81], P7[82], min));
            led8.set(getCurrentValue(P8[81], P8[82], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[82], P1[83], min));
            led2.set(getCurrentValue(P2[82], P2[83], min));
            led3.set(getCurrentValue(P3[82], P3[83], min));
            led4.set(getCurrentValue(P4[82], P4[83], min));
            led5.set(getCurrentValue(P5[82], P5[83], min));
            led6.set(getCurrentValue(P6[82], P6[83], min));
            led7.set(getCurrentValue(P7[82], P7[83], min));
            led8.set(getCurrentValue(P8[82], P8[83], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[83], P1[84], min));
            led2.set(getCurrentValue(P2[83], P2[84], min));
            led3.set(getCurrentValue(P3[83], P3[84], min));
            led4.set(getCurrentValue(P4[83], P4[84], min));
            led5.set(getCurrentValue(P5[83], P5[84], min));
            led6.set(getCurrentValue(P6[83], P6[84], min));
            led7.set(getCurrentValue(P7[83], P7[84], min));
            led8.set(getCurrentValue(P8[83], P8[84], min));
          }
        }
        else if (currenthour == 14)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[84], P1[85], min));
            led2.set(getCurrentValue(P2[84], P2[85], min));
            led3.set(getCurrentValue(P3[84], P3[85], min));
            led4.set(getCurrentValue(P4[84], P4[85], min));
            led5.set(getCurrentValue(P5[84], P5[85], min));
            led6.set(getCurrentValue(P6[84], P6[85], min));
            led7.set(getCurrentValue(P7[84], P7[85], min));
            led8.set(getCurrentValue(P8[84], P8[85], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[85], P1[86], min));
            led2.set(getCurrentValue(P2[85], P2[86], min));
            led3.set(getCurrentValue(P3[85], P3[86], min));
            led4.set(getCurrentValue(P4[85], P4[86], min));
            led5.set(getCurrentValue(P5[85], P5[86], min));
            led6.set(getCurrentValue(P6[85], P6[86], min));
            led7.set(getCurrentValue(P7[85], P7[86], min));
            led8.set(getCurrentValue(P8[85], P8[86], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[86], P1[87], min));
            led2.set(getCurrentValue(P2[86], P2[87], min));
            led3.set(getCurrentValue(P3[86], P3[87], min));
            led4.set(getCurrentValue(P4[86], P4[87], min));
            led5.set(getCurrentValue(P5[86], P5[87], min));
            led6.set(getCurrentValue(P6[86], P6[87], min));
            led7.set(getCurrentValue(P7[86], P7[87], min));
            led8.set(getCurrentValue(P8[86], P8[87], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[87], P1[88], min));
            led2.set(getCurrentValue(P2[87], P2[88], min));
            led3.set(getCurrentValue(P3[87], P3[88], min));
            led4.set(getCurrentValue(P4[87], P4[88], min));
            led5.set(getCurrentValue(P5[87], P5[88], min));
            led6.set(getCurrentValue(P6[87], P6[88], min));
            led7.set(getCurrentValue(P7[87], P7[88], min));
            led8.set(getCurrentValue(P8[87], P8[88], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[88], P1[89], min));
            led2.set(getCurrentValue(P2[88], P2[89], min));
            led3.set(getCurrentValue(P3[88], P3[89], min));
            led4.set(getCurrentValue(P4[88], P4[89], min));
            led5.set(getCurrentValue(P5[88], P5[89], min));
            led6.set(getCurrentValue(P6[88], P6[89], min));
            led7.set(getCurrentValue(P7[88], P7[89], min));
            led8.set(getCurrentValue(P8[88], P8[89], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[89], P1[90], min));
            led2.set(getCurrentValue(P2[89], P2[90], min));
            led3.set(getCurrentValue(P3[89], P3[90], min));
            led4.set(getCurrentValue(P4[89], P4[90], min));
            led5.set(getCurrentValue(P5[89], P5[90], min));
            led6.set(getCurrentValue(P6[89], P6[90], min));
            led7.set(getCurrentValue(P7[89], P7[90], min));
            led8.set(getCurrentValue(P8[89], P8[90], min));
          }
        }
        else if (currenthour == 15)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[90], P1[91], min));
            led2.set(getCurrentValue(P2[90], P2[91], min));
            led3.set(getCurrentValue(P3[90], P3[91], min));
            led4.set(getCurrentValue(P4[90], P4[91], min));
            led5.set(getCurrentValue(P5[90], P5[91], min));
            led6.set(getCurrentValue(P6[90], P6[91], min));
            led7.set(getCurrentValue(P7[90], P7[91], min));
            led8.set(getCurrentValue(P8[90], P8[91], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[91], P1[92], min));
            led2.set(getCurrentValue(P2[91], P2[92], min));
            led3.set(getCurrentValue(P3[91], P3[92], min));
            led4.set(getCurrentValue(P4[91], P4[92], min));
            led5.set(getCurrentValue(P5[91], P5[92], min));
            led6.set(getCurrentValue(P6[91], P6[92], min));
            led7.set(getCurrentValue(P7[91], P7[92], min));
            led8.set(getCurrentValue(P8[91], P8[92], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[92], P1[93], min));
            led2.set(getCurrentValue(P2[92], P2[93], min));
            led3.set(getCurrentValue(P3[92], P3[93], min));
            led4.set(getCurrentValue(P4[92], P4[93], min));
            led5.set(getCurrentValue(P5[92], P5[93], min));
            led6.set(getCurrentValue(P6[92], P6[93], min));
            led7.set(getCurrentValue(P7[92], P7[93], min));
            led8.set(getCurrentValue(P8[92], P8[93], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[93], P1[94], min));
            led2.set(getCurrentValue(P2[93], P2[94], min));
            led3.set(getCurrentValue(P3[93], P3[94], min));
            led4.set(getCurrentValue(P4[93], P4[94], min));
            led5.set(getCurrentValue(P5[93], P5[94], min));
            led6.set(getCurrentValue(P6[93], P6[94], min));
            led7.set(getCurrentValue(P7[93], P7[94], min));
            led8.set(getCurrentValue(P8[93], P8[94], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[94], P1[95], min));
            led2.set(getCurrentValue(P2[94], P2[95], min));
            led3.set(getCurrentValue(P3[94], P3[95], min));
            led4.set(getCurrentValue(P4[94], P4[95], min));
            led5.set(getCurrentValue(P5[94], P5[95], min));
            led6.set(getCurrentValue(P6[94], P6[95], min));
            led7.set(getCurrentValue(P7[94], P7[95], min));
            led8.set(getCurrentValue(P8[94], P8[95], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[95], P1[96], min));
            led2.set(getCurrentValue(P2[95], P2[96], min));
            led3.set(getCurrentValue(P3[95], P3[96], min));
            led4.set(getCurrentValue(P4[95], P4[96], min));
            led5.set(getCurrentValue(P5[95], P5[96], min));
            led6.set(getCurrentValue(P6[95], P6[96], min));
            led7.set(getCurrentValue(P7[95], P7[96], min));
            led8.set(getCurrentValue(P8[95], P8[96], min));
          }
        }
        else if (currenthour == 16)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[96], P1[97], min));
            led2.set(getCurrentValue(P2[96], P2[97], min));
            led3.set(getCurrentValue(P3[96], P3[97], min));
            led4.set(getCurrentValue(P4[96], P4[97], min));
            led5.set(getCurrentValue(P5[96], P5[97], min));
            led6.set(getCurrentValue(P6[96], P6[97], min));
            led7.set(getCurrentValue(P7[96], P7[97], min));
            led8.set(getCurrentValue(P8[96], P8[97], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[97], P1[98], min));
            led2.set(getCurrentValue(P2[97], P2[98], min));
            led3.set(getCurrentValue(P3[97], P3[98], min));
            led4.set(getCurrentValue(P4[97], P4[98], min));
            led5.set(getCurrentValue(P5[97], P5[98], min));
            led6.set(getCurrentValue(P6[97], P6[98], min));
            led7.set(getCurrentValue(P7[97], P7[98], min));
            led8.set(getCurrentValue(P8[97], P8[98], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[98], P1[99], min));
            led2.set(getCurrentValue(P2[98], P2[99], min));
            led3.set(getCurrentValue(P3[98], P3[99], min));
            led4.set(getCurrentValue(P4[98], P4[99], min));
            led5.set(getCurrentValue(P5[98], P5[99], min));
            led6.set(getCurrentValue(P6[98], P6[99], min));
            led7.set(getCurrentValue(P7[98], P7[99], min));
            led8.set(getCurrentValue(P8[98], P8[99], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[99], P1[100], min));
            led2.set(getCurrentValue(P2[99], P2[100], min));
            led3.set(getCurrentValue(P3[99], P3[100], min));
            led4.set(getCurrentValue(P4[99], P4[100], min));
            led5.set(getCurrentValue(P5[99], P5[100], min));
            led6.set(getCurrentValue(P6[99], P6[100], min));
            led7.set(getCurrentValue(P7[99], P7[100], min));
            led8.set(getCurrentValue(P8[99], P8[100], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[100], P1[101], min));
            led2.set(getCurrentValue(P2[100], P2[101], min));
            led3.set(getCurrentValue(P3[100], P3[101], min));
            led4.set(getCurrentValue(P4[100], P4[101], min));
            led5.set(getCurrentValue(P5[100], P5[101], min));
            led6.set(getCurrentValue(P6[100], P6[101], min));
            led7.set(getCurrentValue(P7[100], P7[101], min));
            led8.set(getCurrentValue(P8[100], P8[101], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[101], P1[102], min));
            led2.set(getCurrentValue(P2[101], P2[102], min));
            led3.set(getCurrentValue(P3[101], P3[102], min));
            led4.set(getCurrentValue(P4[101], P4[102], min));
            led5.set(getCurrentValue(P5[101], P5[102], min));
            led6.set(getCurrentValue(P6[101], P6[102], min));
            led7.set(getCurrentValue(P7[101], P7[102], min));
            led8.set(getCurrentValue(P8[101], P8[102], min));
          }
        }
        else if (currenthour == 17)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[102], P1[103], min));
            led2.set(getCurrentValue(P2[102], P2[103], min));
            led3.set(getCurrentValue(P3[102], P3[103], min));
            led4.set(getCurrentValue(P4[102], P4[103], min));
            led5.set(getCurrentValue(P5[102], P5[103], min));
            led6.set(getCurrentValue(P6[102], P6[103], min));
            led7.set(getCurrentValue(P7[102], P7[103], min));
            led8.set(getCurrentValue(P8[102], P8[103], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[103], P1[104], min));
            led2.set(getCurrentValue(P2[103], P2[104], min));
            led3.set(getCurrentValue(P3[103], P3[104], min));
            led4.set(getCurrentValue(P4[103], P4[104], min));
            led5.set(getCurrentValue(P5[103], P5[104], min));
            led6.set(getCurrentValue(P6[103], P6[104], min));
            led7.set(getCurrentValue(P7[103], P7[104], min));
            led8.set(getCurrentValue(P8[103], P8[104], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[104], P1[105], min));
            led2.set(getCurrentValue(P2[104], P2[105], min));
            led3.set(getCurrentValue(P3[104], P3[105], min));
            led4.set(getCurrentValue(P4[104], P4[105], min));
            led5.set(getCurrentValue(P5[104], P5[105], min));
            led6.set(getCurrentValue(P6[104], P6[105], min));
            led7.set(getCurrentValue(P7[104], P7[105], min));
            led8.set(getCurrentValue(P8[104], P8[105], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[105], P1[106], min));
            led2.set(getCurrentValue(P2[105], P2[106], min));
            led3.set(getCurrentValue(P3[105], P3[106], min));
            led4.set(getCurrentValue(P4[105], P4[106], min));
            led5.set(getCurrentValue(P5[105], P5[106], min));
            led6.set(getCurrentValue(P6[105], P6[106], min));
            led7.set(getCurrentValue(P7[105], P7[106], min));
            led8.set(getCurrentValue(P8[105], P8[106], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[106], P1[107], min));
            led2.set(getCurrentValue(P2[106], P2[107], min));
            led3.set(getCurrentValue(P3[106], P3[107], min));
            led4.set(getCurrentValue(P4[106], P4[107], min));
            led5.set(getCurrentValue(P5[106], P5[107], min));
            led6.set(getCurrentValue(P6[106], P6[107], min));
            led7.set(getCurrentValue(P7[106], P7[107], min));
            led8.set(getCurrentValue(P8[106], P8[107], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[107], P1[108], min));
            led2.set(getCurrentValue(P2[107], P2[108], min));
            led3.set(getCurrentValue(P3[107], P3[108], min));
            led4.set(getCurrentValue(P4[107], P4[108], min));
            led5.set(getCurrentValue(P5[107], P5[108], min));
            led6.set(getCurrentValue(P6[107], P6[108], min));
            led7.set(getCurrentValue(P7[107], P7[108], min));
            led8.set(getCurrentValue(P8[107], P8[108], min));
          }
        }
        else if (currenthour == 18)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[108], P1[109], min));
            led2.set(getCurrentValue(P2[108], P2[109], min));
            led3.set(getCurrentValue(P3[108], P3[109], min));
            led4.set(getCurrentValue(P4[108], P4[109], min));
            led5.set(getCurrentValue(P5[108], P5[109], min));
            led6.set(getCurrentValue(P6[108], P6[109], min));
            led7.set(getCurrentValue(P7[108], P7[109], min));
            led8.set(getCurrentValue(P8[108], P8[109], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[109], P1[110], min));
            led2.set(getCurrentValue(P2[109], P2[110], min));
            led3.set(getCurrentValue(P3[109], P3[110], min));
            led4.set(getCurrentValue(P4[109], P4[110], min));
            led5.set(getCurrentValue(P5[109], P5[110], min));
            led6.set(getCurrentValue(P6[109], P6[110], min));
            led7.set(getCurrentValue(P7[109], P7[110], min));
            led8.set(getCurrentValue(P8[109], P8[110], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[110], P1[111], min));
            led2.set(getCurrentValue(P2[110], P2[111], min));
            led3.set(getCurrentValue(P3[110], P3[111], min));
            led4.set(getCurrentValue(P4[110], P4[111], min));
            led5.set(getCurrentValue(P5[110], P5[111], min));
            led6.set(getCurrentValue(P6[110], P6[111], min));
            led7.set(getCurrentValue(P7[110], P7[111], min));
            led8.set(getCurrentValue(P8[110], P8[111], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[111], P1[112], min));
            led2.set(getCurrentValue(P2[111], P2[112], min));
            led3.set(getCurrentValue(P3[111], P3[112], min));
            led4.set(getCurrentValue(P4[111], P4[112], min));
            led5.set(getCurrentValue(P5[111], P5[112], min));
            led6.set(getCurrentValue(P6[111], P6[112], min));
            led7.set(getCurrentValue(P7[111], P7[112], min));
            led8.set(getCurrentValue(P8[111], P8[112], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[112], P1[113], min));
            led2.set(getCurrentValue(P2[112], P2[113], min));
            led3.set(getCurrentValue(P3[112], P3[113], min));
            led4.set(getCurrentValue(P4[112], P4[113], min));
            led5.set(getCurrentValue(P5[112], P5[113], min));
            led6.set(getCurrentValue(P6[112], P6[113], min));
            led7.set(getCurrentValue(P7[112], P7[113], min));
            led8.set(getCurrentValue(P8[112], P8[113], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[113], P1[114], min));
            led2.set(getCurrentValue(P2[113], P2[114], min));
            led3.set(getCurrentValue(P3[113], P3[114], min));
            led4.set(getCurrentValue(P4[113], P4[114], min));
            led5.set(getCurrentValue(P5[113], P5[114], min));
            led6.set(getCurrentValue(P6[113], P6[114], min));
            led7.set(getCurrentValue(P7[113], P7[114], min));
            led8.set(getCurrentValue(P8[113], P8[114], min));
          }
        }
        else if (currenthour == 19)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[114], P1[115], min));
            led2.set(getCurrentValue(P2[114], P2[115], min));
            led3.set(getCurrentValue(P3[114], P3[115], min));
            led4.set(getCurrentValue(P4[114], P4[115], min));
            led5.set(getCurrentValue(P5[114], P5[115], min));
            led6.set(getCurrentValue(P6[114], P6[115], min));
            led7.set(getCurrentValue(P7[114], P7[115], min));
            led8.set(getCurrentValue(P8[114], P8[115], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[115], P1[116], min));
            led2.set(getCurrentValue(P2[115], P2[116], min));
            led3.set(getCurrentValue(P3[115], P3[116], min));
            led4.set(getCurrentValue(P4[115], P4[116], min));
            led5.set(getCurrentValue(P5[115], P5[116], min));
            led6.set(getCurrentValue(P6[115], P6[116], min));
            led7.set(getCurrentValue(P7[115], P7[116], min));
            led8.set(getCurrentValue(P8[115], P8[116], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[116], P1[117], min));
            led2.set(getCurrentValue(P2[116], P2[117], min));
            led3.set(getCurrentValue(P3[116], P3[117], min));
            led4.set(getCurrentValue(P4[116], P4[117], min));
            led5.set(getCurrentValue(P5[116], P5[117], min));
            led6.set(getCurrentValue(P6[116], P6[117], min));
            led7.set(getCurrentValue(P7[116], P7[117], min));
            led8.set(getCurrentValue(P8[116], P8[117], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[117], P1[118], min));
            led2.set(getCurrentValue(P2[117], P2[118], min));
            led3.set(getCurrentValue(P3[117], P3[118], min));
            led4.set(getCurrentValue(P4[117], P4[118], min));
            led5.set(getCurrentValue(P5[117], P5[118], min));
            led6.set(getCurrentValue(P6[117], P6[118], min));
            led7.set(getCurrentValue(P7[117], P7[118], min));
            led8.set(getCurrentValue(P8[117], P8[118], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[118], P1[119], min));
            led2.set(getCurrentValue(P2[118], P2[119], min));
            led3.set(getCurrentValue(P3[118], P3[119], min));
            led4.set(getCurrentValue(P4[118], P4[119], min));
            led5.set(getCurrentValue(P5[118], P5[119], min));
            led6.set(getCurrentValue(P6[118], P6[119], min));
            led7.set(getCurrentValue(P7[118], P7[119], min));
            led8.set(getCurrentValue(P8[118], P8[119], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[119], P1[120], min));
            led2.set(getCurrentValue(P2[119], P2[120], min));
            led3.set(getCurrentValue(P3[119], P3[120], min));
            led4.set(getCurrentValue(P4[119], P4[120], min));
            led5.set(getCurrentValue(P5[119], P5[120], min));
            led6.set(getCurrentValue(P6[119], P6[120], min));
            led7.set(getCurrentValue(P7[119], P7[120], min));
            led8.set(getCurrentValue(P8[119], P8[120], min));
          }
        }
        else if (currenthour == 20)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[120], P1[121], min));
            led2.set(getCurrentValue(P2[120], P2[121], min));
            led3.set(getCurrentValue(P3[120], P3[121], min));
            led4.set(getCurrentValue(P4[120], P4[121], min));
            led5.set(getCurrentValue(P5[120], P5[121], min));
            led6.set(getCurrentValue(P6[120], P6[121], min));
            led7.set(getCurrentValue(P7[120], P7[121], min));
            led8.set(getCurrentValue(P8[120], P8[121], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[121], P1[122], min));
            led2.set(getCurrentValue(P2[121], P2[122], min));
            led3.set(getCurrentValue(P3[121], P3[122], min));
            led4.set(getCurrentValue(P4[121], P4[122], min));
            led5.set(getCurrentValue(P5[121], P5[122], min));
            led6.set(getCurrentValue(P6[121], P6[122], min));
            led7.set(getCurrentValue(P7[121], P7[122], min));
            led8.set(getCurrentValue(P8[121], P8[122], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[122], P1[123], min));
            led2.set(getCurrentValue(P2[122], P2[123], min));
            led3.set(getCurrentValue(P3[122], P3[123], min));
            led4.set(getCurrentValue(P4[122], P4[123], min));
            led5.set(getCurrentValue(P5[122], P5[123], min));
            led6.set(getCurrentValue(P6[122], P6[123], min));
            led7.set(getCurrentValue(P7[122], P7[123], min));
            led8.set(getCurrentValue(P8[122], P8[123], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[123], P1[124], min));
            led2.set(getCurrentValue(P2[123], P2[124], min));
            led3.set(getCurrentValue(P3[123], P3[124], min));
            led4.set(getCurrentValue(P4[123], P4[124], min));
            led5.set(getCurrentValue(P5[123], P5[124], min));
            led6.set(getCurrentValue(P6[123], P6[124], min));
            led7.set(getCurrentValue(P7[123], P7[124], min));
            led8.set(getCurrentValue(P8[123], P8[124], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[124], P1[125], min));
            led2.set(getCurrentValue(P2[124], P2[125], min));
            led3.set(getCurrentValue(P3[124], P3[125], min));
            led4.set(getCurrentValue(P4[124], P4[125], min));
            led5.set(getCurrentValue(P5[124], P5[125], min));
            led6.set(getCurrentValue(P6[124], P6[125], min));
            led7.set(getCurrentValue(P7[124], P7[125], min));
            led8.set(getCurrentValue(P8[124], P8[125], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[125], P1[126], min));
            led2.set(getCurrentValue(P2[125], P2[126], min));
            led3.set(getCurrentValue(P3[125], P3[126], min));
            led4.set(getCurrentValue(P4[125], P4[126], min));
            led5.set(getCurrentValue(P5[125], P5[126], min));
            led6.set(getCurrentValue(P6[125], P6[126], min));
            led7.set(getCurrentValue(P7[125], P7[126], min));
            led8.set(getCurrentValue(P8[125], P8[126], min));
          }
        }
        else if (currenthour == 21)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[126], P1[127], min));
            led2.set(getCurrentValue(P2[126], P2[127], min));
            led3.set(getCurrentValue(P3[126], P3[127], min));
            led4.set(getCurrentValue(P4[126], P4[127], min));
            led5.set(getCurrentValue(P5[126], P5[127], min));
            led6.set(getCurrentValue(P6[126], P6[127], min));
            led7.set(getCurrentValue(P7[126], P7[127], min));
            led8.set(getCurrentValue(P8[126], P8[127], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[127], P1[128], min));
            led2.set(getCurrentValue(P2[127], P2[128], min));
            led3.set(getCurrentValue(P3[127], P3[128], min));
            led4.set(getCurrentValue(P4[127], P4[128], min));
            led5.set(getCurrentValue(P5[127], P5[128], min));
            led6.set(getCurrentValue(P6[127], P6[128], min));
            led7.set(getCurrentValue(P7[127], P7[128], min));
            led8.set(getCurrentValue(P8[127], P8[128], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[128], P1[129], min));
            led2.set(getCurrentValue(P2[128], P2[129], min));
            led3.set(getCurrentValue(P3[128], P3[129], min));
            led4.set(getCurrentValue(P4[128], P4[129], min));
            led5.set(getCurrentValue(P5[128], P5[129], min));
            led6.set(getCurrentValue(P6[128], P6[129], min));
            led7.set(getCurrentValue(P7[128], P7[129], min));
            led8.set(getCurrentValue(P8[128], P8[129], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[129], P1[130], min));
            led2.set(getCurrentValue(P2[129], P2[130], min));
            led3.set(getCurrentValue(P3[129], P3[130], min));
            led4.set(getCurrentValue(P4[129], P4[130], min));
            led5.set(getCurrentValue(P5[129], P5[130], min));
            led6.set(getCurrentValue(P6[129], P6[130], min));
            led7.set(getCurrentValue(P7[129], P7[130], min));
            led8.set(getCurrentValue(P8[129], P8[130], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[130], P1[131], min));
            led2.set(getCurrentValue(P2[130], P2[131], min));
            led3.set(getCurrentValue(P3[130], P3[131], min));
            led4.set(getCurrentValue(P4[130], P4[131], min));
            led5.set(getCurrentValue(P5[130], P5[131], min));
            led6.set(getCurrentValue(P6[130], P6[131], min));
            led7.set(getCurrentValue(P7[130], P7[131], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[131], P1[132], min));
            led2.set(getCurrentValue(P2[131], P2[132], min));
            led3.set(getCurrentValue(P3[131], P3[132], min));
            led4.set(getCurrentValue(P4[131], P4[132], min));
            led5.set(getCurrentValue(P5[131], P5[132], min));
            led6.set(getCurrentValue(P6[131], P6[132], min));
            led7.set(getCurrentValue(P7[131], P7[132], min));
            led8.set(getCurrentValue(P8[131], P8[132], min));
          }
        }
        else if (currenthour == 22)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[132], P1[133], min));
            led2.set(getCurrentValue(P2[132], P2[133], min));
            led3.set(getCurrentValue(P3[132], P3[133], min));
            led4.set(getCurrentValue(P4[132], P4[133], min));
            led5.set(getCurrentValue(P5[132], P5[133], min));
            led6.set(getCurrentValue(P6[132], P6[133], min));
            led7.set(getCurrentValue(P7[132], P7[133], min));
            led8.set(getCurrentValue(P8[132], P8[133], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[133], P1[134], min));
            led2.set(getCurrentValue(P2[133], P2[134], min));
            led3.set(getCurrentValue(P3[133], P3[134], min));
            led4.set(getCurrentValue(P4[133], P4[134], min));
            led5.set(getCurrentValue(P5[133], P5[134], min));
            led6.set(getCurrentValue(P6[133], P6[134], min));
            led7.set(getCurrentValue(P7[133], P7[134], min));
            led8.set(getCurrentValue(P8[133], P8[134], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[134], P1[135], min));
            led2.set(getCurrentValue(P2[134], P2[135], min));
            led3.set(getCurrentValue(P3[134], P3[135], min));
            led4.set(getCurrentValue(P4[134], P4[135], min));
            led5.set(getCurrentValue(P5[134], P5[135], min));
            led6.set(getCurrentValue(P6[134], P6[135], min));
            led7.set(getCurrentValue(P7[134], P7[135], min));
            led8.set(getCurrentValue(P8[134], P8[135], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[135], P1[136], min));
            led2.set(getCurrentValue(P2[135], P2[136], min));
            led3.set(getCurrentValue(P3[135], P3[136], min));
            led4.set(getCurrentValue(P4[135], P4[136], min));
            led5.set(getCurrentValue(P5[135], P5[136], min));
            led6.set(getCurrentValue(P6[135], P6[136], min));
            led7.set(getCurrentValue(P7[135], P7[136], min));
            led8.set(getCurrentValue(P8[135], P8[136], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[136], P1[137], min));
            led2.set(getCurrentValue(P2[136], P2[137], min));
            led3.set(getCurrentValue(P3[136], P3[137], min));
            led4.set(getCurrentValue(P4[136], P4[137], min));
            led5.set(getCurrentValue(P5[136], P5[137], min));
            led6.set(getCurrentValue(P6[136], P6[137], min));
            led7.set(getCurrentValue(P7[136], P7[137], min));
            led8.set(getCurrentValue(P8[136], P8[137], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[137], P1[138], min));
            led2.set(getCurrentValue(P2[137], P2[138], min));
            led3.set(getCurrentValue(P3[137], P3[138], min));
            led4.set(getCurrentValue(P4[137], P4[138], min));
            led5.set(getCurrentValue(P5[137], P5[138], min));
            led6.set(getCurrentValue(P6[137], P6[138], min));
            led7.set(getCurrentValue(P7[137], P7[138], min));
            led8.set(getCurrentValue(P8[137], P8[138], min));
          }
        }
        else if (currenthour == 23)
        {
          if (currentmin < 10)
          {
            int min = currentmin;
            led1.set(getCurrentValue(P1[138], P1[139], min));
            led2.set(getCurrentValue(P2[138], P2[139], min));
            led3.set(getCurrentValue(P3[138], P3[139], min));
            led4.set(getCurrentValue(P4[138], P4[139], min));
            led5.set(getCurrentValue(P5[138], P5[139], min));
            led6.set(getCurrentValue(P6[138], P6[139], min));
            led7.set(getCurrentValue(P7[138], P7[139], min));
            led8.set(getCurrentValue(P8[138], P8[139], min));
          }
          else if (currentmin >= 10 and currentmin < 20)
          {
            int min = currentmin - 10;
            led1.set(getCurrentValue(P1[139], P1[140], min));
            led2.set(getCurrentValue(P2[139], P2[140], min));
            led3.set(getCurrentValue(P3[139], P3[140], min));
            led4.set(getCurrentValue(P4[139], P4[140], min));
            led5.set(getCurrentValue(P5[139], P5[140], min));
            led6.set(getCurrentValue(P6[139], P6[140], min));
            led7.set(getCurrentValue(P7[139], P7[140], min));
            led8.set(getCurrentValue(P8[139], P8[140], min));
          }
          else if (currentmin >= 20 and currentmin < 30)
          {
            int min = currentmin - 20;
            led1.set(getCurrentValue(P1[140], P1[141], min));
            led2.set(getCurrentValue(P2[140], P2[141], min));
            led3.set(getCurrentValue(P3[140], P3[141], min));
            led4.set(getCurrentValue(P4[140], P4[141], min));
            led5.set(getCurrentValue(P5[140], P5[141], min));
            led6.set(getCurrentValue(P6[140], P6[141], min));
            led7.set(getCurrentValue(P7[140], P7[141], min));
            led8.set(getCurrentValue(P8[140], P8[141], min));
          }
          else if (currentmin >= 30 and currentmin < 40)
          {
            int min = currentmin - 30;
            led1.set(getCurrentValue(P1[141], P1[142], min));
            led2.set(getCurrentValue(P2[141], P2[142], min));
            led3.set(getCurrentValue(P3[141], P3[142], min));
            led4.set(getCurrentValue(P4[141], P4[142], min));
            led5.set(getCurrentValue(P5[141], P5[142], min));
            led6.set(getCurrentValue(P6[141], P6[142], min));
            led7.set(getCurrentValue(P7[141], P7[142], min));
            led8.set(getCurrentValue(P8[141], P8[142], min));
          }
          else if (currentmin >= 40 and currentmin < 50)
          {
            int min = currentmin - 40;
            led1.set(getCurrentValue(P1[142], P1[0], min));
            led2.set(getCurrentValue(P2[142], P2[0], min));
            led3.set(getCurrentValue(P3[142], P3[0], min));
            led4.set(getCurrentValue(P4[142], P4[0], min));
            led5.set(getCurrentValue(P5[142], P5[0], min));
            led6.set(getCurrentValue(P6[142], P6[0], min));
            led7.set(getCurrentValue(P7[142], P7[0], min));
            led8.set(getCurrentValue(P8[142], P7[0], min));
          }
          else if (currentmin >= 50)
          {
            int min = currentmin - 50;
            led1.set(getCurrentValue(P1[143], P1[0], min));
            led2.set(getCurrentValue(P2[143], P2[0], min));
            led3.set(getCurrentValue(P3[143], P3[0], min));
            led4.set(getCurrentValue(P4[143], P4[0], min));
            led5.set(getCurrentValue(P5[143], P5[0], min));
            led6.set(getCurrentValue(P6[143], P6[0], min));
            led7.set(getCurrentValue(P7[143], P7[0], min));
            led8.set(getCurrentValue(P8[143], P7[0], min));
          }
        }
      }
      else if (PWM_INFO_TESTMODE == "test")
      {
        //Serial.printf("test mode .... current sec is: %d:%d:%d the led1's volume is: %d \r\n", currenthour, currentmin, currentsec, P1[TESTMODE_COUNT]);
        Serial.printf("text mode ... current index is: %d . led1's volumne is: %d \r\n", TESTMODE_COUNT,P1[TESTMODE_COUNT]);
        if (TESTMODE_COUNT == 144)
        {
          TESTMODE_COUNT = 143;
        }
        
        if(TESTMODE_COUNT > 143) {
          TESTMODE_COUNT = 0;
        }



        led1.set(P1[TESTMODE_COUNT]);
        led2.set(P2[TESTMODE_COUNT]);
        led3.set(P3[TESTMODE_COUNT]);
        led4.set(P4[TESTMODE_COUNT]);
        led5.set(P5[TESTMODE_COUNT]);
        led6.set(P6[TESTMODE_COUNT]);
        led7.set(P7[TESTMODE_COUNT]);
        led8.set(P8[TESTMODE_COUNT]);

        TESTMODE_COUNT = TESTMODE_COUNT + 6;
      }

      if (true)
      {
        //if (WiFi.status() == WL_CONNECTED  && !client.connected())
        if (wifistatus == "online")
        {
          if (!client.connected())
          {
            Serial.println("repeat here it is");
            led0.set(5);
            Serial.println("the mqtt service is disconnected");
            mqttconn();
            client.publish("esp32/disnotify", ESP_HOST_NAME.c_str());
          }
        }
      }
      if (client.connected())
      {
        client.publish("esp32/heart", ESP_HOST_NAME.c_str());
      }
    }
    else if (PWM_INFO_SHOWTYPE == "fix")
    {
      //Serial.printf("%s,%s,%s,%s,%s,%s,%s,%s\r\n",P1[144],P2[144],P3[144],P4[144],P5[144],P6[144],P7[144],P8[144]);
      //Serial.printf("%d:%d:%d:%d:%d:%d:%d:%d \r\n", P1[144],P2[144],P3[144],P4[144],P5[144],P6[144],P7[144],P8[144]);
      led1.set(P1[144]);
      led2.set(P2[144]);
      led3.set(P3[144]);
      led4.set(P4[144]);
      led5.set(P5[144]);
      led6.set(P6[144]);
      led7.set(P7[144]);
      led8.set(P8[144]);
      //String topiccontent = ESP_HOST_NAME;
      //Serial.printf("the connect status is: %d, the smart status is: %d, the mqtt is: %d, the ssid is: %s \r\n", WiFi.status(), WiFi.smartConfigDone(), client.connected(), SSID.c_str());
      //if(WiFi.status() == WL_CONNECTED) {
      if (wifistatus == "online")
      {
        if (!client.connected())
        {
          led0.set(5);
          Serial.println("fix here it is");
          Serial.println("the mqtt service is disconnected");
          mqttconn();
          client.publish("esp32/disnotify", ESP_HOST_NAME.c_str());
        }
        else
        {
          client.publish("esp32/heart", ESP_HOST_NAME.c_str());
        }
      }
    }
  }
  delay(1000);
}