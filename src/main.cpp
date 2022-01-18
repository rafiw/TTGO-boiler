#include <Arduino.h>

#include <SPI.h>
#include <TimeLib.h>
#include <TFT_eSPI.h>
#include <EspMQTTClient.h>
#include <ArduinoJson.h>

#define BUTTON1PIN  35
#define BUTTON2PIN  0
#define ORANGE_TEMP 50
#define RED_TEMP    70

/**
 * connection variables
 */
const char* WIFI_NAME      = "";
const char* WIFI_PASS      = "";
const char* MQTT_IP        = "";
const char* MQTT_USER      = "";
const char* MQTT_PASS      = "";
const char* CLIENT         = "TTGO_BOILER";
const char* SUB_MQTT_TOPIC = "tele/boiler-sensor/SENSOR"; // the tasmota publish path, you need to call the tasmota device boiler-sensor
const char* PUB_MQTT_TOPIC = ""; // use this path to publish string to MQTT and trigger something

/**
 * declarations
 */
void onTestMessageReceived(const String& message);
void clearScreenAndPrint(const String& msg, bool is_error);
void printTemprature();
void IRAM_ATTR toggleButton2();
void IRAM_ATTR toggleButton1();

/**
 * Globals
 */ 
EspMQTTClient client(
  WIFI_NAME,
  WIFI_PASS,
  MQTT_IP,     // MQTT Broker server ip
  MQTT_USER,   // Can be omitted if not needed
  MQTT_PASS,   // Can be omitted if not needed
  CLIENT       // Client name that uniquely identify your device
);
TFT_eSPI tft = TFT_eSPI();

int top_temperature      = 0;
int bottom_temperature   = 0;
bool time_is_valid       = true;
bool temp_was_updated    = false;
long lastDebounceButton1 = 0;    // Holds Button1 last debounce
long lastDebounceButton2 = 0;    // Holds Button2 last debounce
long debounceDelay       = 200;  // 200ms between re-polling
bool inverted_display    = false;

void setup()
{
  Serial.begin(9600);
  client.enableDebuggingMessages();
  client.enableDrasticResetOnConnectionFailures();
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  pinMode(BUTTON1PIN, INPUT);
  pinMode(BUTTON2PIN, INPUT);
  attachInterrupt(BUTTON1PIN, toggleButton2, FALLING);
  attachInterrupt(BUTTON2PIN, toggleButton1, FALLING);
  clearScreenAndPrint("Connecting!", false);
  client.setMaxPacketSize(512);
}

// INTRPT Function to execute when Button 1 is Pushed
void IRAM_ATTR toggleButton1()
{
  if ((millis() - lastDebounceButton1) > debounceDelay)
  { 

  }
}

void publish_message()
{
  client.publish(PUB_MQTT_TOPIC, "START");
}

// INTRPT Function to execute when Button 2 is Pushed, publish message to MQTT can be used to automate something
void IRAM_ATTR toggleButton2()
{
  if ((millis() - lastDebounceButton2) > debounceDelay)
  { 
    client.executeDelayed(1, publish_message);
    lastDebounceButton2 = millis();
  }
}

void clearScreenAndPrint(const String& msg, bool is_error)
{
  if (tft.getCursorY() > 200)
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(2, 10);
  }
  if(is_error)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }
  else
  {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  }
  tft.println(msg);
}

void onConnectionEstablished()
{
  clearScreenAndPrint("CONNECTED!", false);
  client.subscribe(SUB_MQTT_TOPIC, onTestMessageReceived);
}

void parse_time(const char *str)
{
  tmElements_t tm;
  int Year, Month, Day, Hour, Minute, Second ;
  sscanf(str, "%d-%d-%dT%d:%d:%d", &Year, &Month, &Day, &Hour, &Minute, &Second);
  tm.Year = CalendarYrToTm(Year);
  tm.Month = Month;
  tm.Day = Day;
  tm.Hour = Hour;
  tm.Minute = Minute;
  tm.Second = Second;
  long millis = makeTime(tm);
  long now_millis = now();
  if(now_millis - millis > (3 * 50 * 1000)) /* 3 min */
  {
    clearScreenAndPrint("Old Data", true);
  }
}

void onTestMessageReceived(const String& message) {
  Serial.print("message received from test/mytopic: " + message);
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  if(error != DeserializationError::Ok)
  {
    clearScreenAndPrint("Failed to parse message" + message, true);
    Serial.print("Failed to parse message" + message);
    return;
  }
  int new_bottom_temperature = (int)doc["DS18B20-1"]["Temperature"];
  int new_top_temperature    = (int)doc["DS18B20-2"]["Temperature"];
  parse_time(doc["Time"]);
  if(new_bottom_temperature != bottom_temperature || new_top_temperature != top_temperature)
  {
    temp_was_updated = true;
    bottom_temperature = new_bottom_temperature;
    top_temperature = new_top_temperature;
  }
}

void printTempratureHelper(int temperature)
{
  if(temperature > RED_TEMP)
  {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  }
  else if(temperature > ORANGE_TEMP)
  {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  }
  else
  {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  }
  tft.setFreeFont(&Orbitron_Light_32);
  tft.print(temperature);
  tft.setTextFont(2);
  tft.setCursor(tft.getCursorX(), tft.getCursorY() - 60);
  tft.print("`c");
}

void printTemprature()
{
  if(top_temperature < 1 && bottom_temperature < 1)
  {
    clearScreenAndPrint("No Data Yet", false);
    return;
  }
  if(!temp_was_updated)
    return;
  temp_was_updated = false;
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(2, 80);
  printTempratureHelper(top_temperature);
  tft.setCursor(2, 230);
  printTempratureHelper(bottom_temperature);
}

void loop()
{
  client.loop();
  delay(2000);
  if(!client.isConnected())
  {
    clearScreenAndPrint("DISCONNECTED!!", true);
  }
  printTemprature();
}
