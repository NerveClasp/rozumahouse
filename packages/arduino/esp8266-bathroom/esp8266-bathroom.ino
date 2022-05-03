#include <ArduinoJson.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE

//#include "FS.h"
#include "LittleFS.h" // LittleFS is declared
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <Esp.h>
#include <ESP8266httpUpdate.h>

#include <Hash.h>
#define ARDUINOJSON_ENABLE_PROGMEM 0

const char *MODEL = "esp8266-bathroom"; // do not change if you want OTA updates
const char *DEVICENAME = "bathroom-leds";
const char *DEVCHANNEL = "/bathroom-leds";
// Fixed definitions cannot change on the fly.
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
#define COLOR_ORDER GRB // It's GRB for WS2812B

#define NUM_STRIPS 2

// #define LED_DT_ONE D8 // Serial data pin
#define LED_DT_ONE D1    // Serial data pin
#define NUM_LEDS_ONE 5 // Number of LED's

// #define LED_DT_TWO D12 // Serial data pin
#define LED_DT_TWO D2    // Serial data pin
#define NUM_LEDS_TWO 5 // Number of LED's

#define MAIN_BUTTON A0 // input to read led button high or low state

WiFiClient net;
MQTTClient client(256);

unsigned long lastMillis = 0;

CLEDController *controllers[NUM_STRIPS];

// Initialize led configuration.
typedef struct RGBColor
{
  int r;
  int g;
  int b;
};

typedef struct Settings
{
  int activeLeds;
  int animationDuration;
  int brightness;
  String animation;
  bool ledOn;
  RGBColor color[4];
};

String color1 = "#ac7a19";
String color2 = "#b04737";
String color3 = "#3c78a9";


RGBColor hexToRgb(String hex) {
  return {
    hex.substring(1, 3).toInt(),
    hex.substring(3, 5).toInt(),
    hex.substring(5, 7).toInt()
  };
}

Settings config[NUM_STRIPS] = {
  { 5,           // activeLeds strip 1
    3000,          // animationDuration
    255,            // brightness
    "static",     // animation
    true,          // ledOn
    { { 255, 255, 255 }, // RGBColor
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 } }
    /* { hexToRgb(color1), // RGBColor */
      /* hexToRgb(color2), */
      /* hexToRgb(color3), */
      /* hexToRgb(color1) */
    /* } */
  },
  { 5,           // activeLeds strip 2
    3000,          // animationDuration
    255,            // brightness
    "static",     // animation
    true,          // ledOn
    { { 255, 255, 255 }, // RGBColor
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 } }
    /* { hexToRgb(color1), // RGBColor */
    /*   hexToRgb(color2), */
    /*   hexToRgb(color3), */
    /*   hexToRgb(color1) */
    /* } */
  }
};

// Internal global variables
int maxActiveLeds[NUM_STRIPS] = { 300, 300 };
int animation_position[NUM_STRIPS] = { 0, 0 };
bool animation_forward[NUM_STRIPS] = { true, true };

struct CRGB leds_one[NUM_LEDS_ONE]; // Initialize our LED array.
struct CRGB leds_two[NUM_LEDS_TWO]; // Initialize our LED array.

String previousMessage;
String message = "";
bool settingsHaveChanged = false;

char *host = "192.168.1.111";

int port = 8888;

// button sends 0 when not lit (default on power on)
// sends 1 when lit
// it is 0 by default, I want it to work as a night button so that you see it
// when you want to turn on the lights
// TODO: figure out how to reset the button when sending on/off signal from network
// maybe connect power to digital out
// although that works only for cases when led was off (button lit) and we turned it on
// the other way around is tricky
void ledOnOrOff(int led, int offOrOn){
  if(offOrOn == 0){
    fill_solid(
        controllers[led]->leds(),
        config[led].activeLeds,
        CRGB(
          config[led].color[0].r,
          config[led].color[0].g,
          config[led].color[0].b)
        );
  }else {
    fill_solid(
        controllers[led]->leds(),
        config[led].activeLeds,
        CRGB(0,0,0)
        );
  }
}

void setup()
{
  Serial.begin(115200); // Initialize serial port for debugging.
  delay(100);          // Soft startup to ease the flow of electrons.
  Serial.println("Starting..");
  pinMode(D8, INPUT);

  controllers[0] = &FastLED.addLeds<LED_TYPE, LED_DT_ONE, COLOR_ORDER>(leds_one, NUM_LEDS_ONE);
  controllers[1] = &FastLED.addLeds<LED_TYPE, LED_DT_TWO, COLOR_ORDER>(leds_two, NUM_LEDS_TWO);
  //set_max_power_in_volts_and_milliamps(5, 15000); // FastLED power management set at 5V, 500mA
  FastLED.clear();
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    fill_solid(
        controllers[i]->leds(),
        config[i].activeLeds,
        CRGB(
          config[i].color[0].r,
          config[i].color[0].g,
          config[i].color[0].b)
        );
  }

  SPIFFS.begin();
  Serial.println("Reading settings from disk");
  readSettingsFromDisk();
}

int toiletLedsOn = 0;

void loop()
{
  while (Serial.available() > 0)
  {
    previousMessage = message;
    message = Serial.readString();
    Serial.println(message);
    //parseMessage(message);
  }
  int bathroomLedsOn = digitalRead(D8);
  //Serial.println(val);

  ledOnOrOff(0, bathroomLedsOn);
  ledOnOrOff(1, toiletLedsOn);

  controllers[0]->showLeds(config[0].brightness);
  controllers[1]->showLeds(config[1].brightness);

  EVERY_N_MINUTES(1)
  {
    if (settingsHaveChanged)
    {
      Serial.println("settings have changed. Writing to disk.");
      settingsHaveChanged = false;
      writeSettingsToDisk();
    }
  }

  delay(10);

  /* client.loop(); */
  /* delay(10); */
}

// settings leds from saved config file
void setLeds(JsonArray &settings)
{
  int size = settings.size();
  for (int i = 0; i < size; i++)
  {
    setLeds(settings[i], i, false);
  }
}

// setting leds after receiving a command from server
void setLeds(JsonObject &root)
{
  const int led = root["led"];
  setLeds(root, led, true);
}

void setLeds(JsonObject &root, int led, bool saveNewSettings)
{
  if (root.containsKey("animation")) // TODO: think if this is needed
  {
    String newAnimation = root["animation"].as<String>();
    if (newAnimation != "")
    {
      config[led].animation = newAnimation;
    }
  }

  if (root.containsKey("brightness")) // TODO: think if this is needed
  {
    config[led].brightness = root["brightness"];
  }
  if (root.containsKey("animationDuration"))
  {
    config[led].animationDuration = root["animationDuration"];
  }
  if (root.containsKey("color"))
  {
    JsonArray &color = root["color"];
    int size = color.size();
    switch (size)
    {
      case 1:
        {
          JsonObject &color_0 = color[0]["rgb"];
          int r_0 = color_0["r"];
          int g_0 = color_0["g"];
          int b_0 = color_0["b"];
          config[led].color[0] = { r_0, g_0, b_0 };
          config[led].color[1] = { -1, -1, -1 };
          config[led].color[2] = { -1, -1, -1 };
          config[led].color[3] = { -1, -1, -1 };
          fill_solid(
            controllers[led]->leds(),
            config[led].activeLeds,
            CRGB(r_0, g_0, b_0));
          break;
        }
      case 2:
        {
          JsonObject &color_0 = color[0]["rgb"];
          JsonObject &color_1 = color[1]["rgb"];
          int r_0 = color_0["r"];
          int g_0 = color_0["g"];
          int b_0 = color_0["b"];
          int r_1 = color_1["r"];
          int g_1 = color_1["g"];
          int b_1 = color_1["b"];
          config[led].color[0] = { r_0, g_0, b_0 };
          config[led].color[1] = { r_1, g_1, b_1 };
          config[led].color[2] = { -1, -1, -1 };
          config[led].color[3] = { -1, -1, -1 };
          fill_gradient_RGB(
            controllers[led]->leds(),
            config[led].activeLeds,
            CRGB(r_0, g_0, b_0),
            CRGB(r_1, g_1, b_1));
          break;
        }
      case 3:
        {
          JsonObject &color_0 = color[0]["rgb"];
          JsonObject &color_1 = color[1]["rgb"];
          JsonObject &color_2 = color[2]["rgb"];
          int r_0 = color_0["r"];
          int g_0 = color_0["g"];
          int b_0 = color_0["b"];
          int r_1 = color_1["r"];
          int g_1 = color_1["g"];
          int b_1 = color_1["b"];
          int r_2 = color_2["r"];
          int g_2 = color_2["g"];
          int b_2 = color_2["b"];
          config[led].color[0] = { r_0, g_0, b_0 };
          config[led].color[1] = { r_1, g_1, b_1 };
          config[led].color[2] = { r_2, g_2, b_2 };
          config[led].color[3] = { -1, -1, -1 };

          fill_gradient_RGB(
            controllers[led]->leds(),
            config[led].activeLeds,
            CRGB(r_0, g_0, b_0),
            CRGB(r_1, g_1, b_1),
            CRGB(r_2, g_2, b_2));
          break;
        }
      case 4:
        {
          JsonObject &color_0 = color[0]["rgb"];
          JsonObject &color_1 = color[1]["rgb"];
          JsonObject &color_2 = color[2]["rgb"];
          JsonObject &color_3 = color[3]["rgb"];

          int r_0 = color_0["r"];
          int g_0 = color_0["g"];
          int b_0 = color_0["b"];
          int r_1 = color_1["r"];
          int g_1 = color_1["g"];
          int b_1 = color_1["b"];
          int r_2 = color_2["r"];
          int g_2 = color_2["g"];
          int b_2 = color_2["b"];
          int r_3 = color_3["r"];
          int g_3 = color_3["g"];
          int b_3 = color_3["b"];
          config[led].color[0] = { r_0, g_0, b_0 };
          config[led].color[1] = { r_1, g_1, b_1 };
          config[led].color[2] = { r_2, g_2, b_2 };
          config[led].color[3] = { r_3, g_3, b_3 };

          fill_gradient_RGB(
            controllers[led]->leds(),
            config[led].activeLeds,
            CRGB(r_0, g_0, b_0),
            CRGB(r_1, g_1, b_1),
            CRGB(r_2, g_2, b_2),
            CRGB(r_3, g_3, b_3));
          break;
        }

      default:
        {
          Serial.println("Too few or too many colors");
          break;
        }
    }
  }
  if (root.containsKey("ledOn"))
  {
    config[led].ledOn = root["ledOn"];
    if (!config[led].ledOn)
    {
      fill_solid(controllers[led]->leds(), maxActiveLeds[led], CRGB(0, 0, 0));
    }
  }
  if (saveNewSettings)
  {
    // there's a check for this boolean in the loop every minute to save settings to disk if this is true
    settingsHaveChanged = true;
  }
}

// Settings
JsonObject &settingsToJson()
{
  StaticJsonBuffer<2000> jb;
  JsonObject &root = jb.createObject();
  JsonArray &leds = root.createNestedArray("leds");
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    JsonObject &led = leds.createNestedObject();
    led["activeLeds"] = config[i].activeLeds;
    led["brightness"] = config[i].brightness;
    led["animation"] = config[i].animation;
    led["animationDuration"] = config[i].animationDuration;
    led["ledOn"] = config[i].ledOn;
    JsonArray &color = led.createNestedArray("color");

    for (int j = 0; j < 4; j++) // 4 here is the max number of CRGB colors fill_gradient_RGB takes
    {
      if (config[i].color[j].r != -1)
      {
        JsonObject &color_0 = color.createNestedObject();
        JsonObject &color_0_rgb = color_0.createNestedObject("rgb");
        color_0_rgb["r"] = config[i].color[j].r;
        color_0_rgb["g"] = config[i].color[j].g;
        color_0_rgb["b"] = config[i].color[j].b;
      }
    }
  }
  return root;
}

String settingsToText()
{
  String result;
  JsonObject &leds = settingsToJson();
  leds.printTo(result);
  return result;
}

void writeSettingsToDisk()
{
  SPIFFS.begin();
  File f = SPIFFS.open("/settings.txt", "w");
  if (!f)
  {
    Serial.println("settings.txt file failed to open");
    return;
  }
  f.print(settingsToText());
  f.close();
}

void readSettingsFromDisk()
{
  SPIFFS.begin();
  File f = SPIFFS.open("/settings.txt", "r");
  if (!f)
  {
    Serial.println("settings.txt file failed to open");
    return;
  }
  String settings = fileRead("/settings.txt");
  StaticJsonBuffer<2000> jb;
  JsonObject &root = jb.parseObject(settings);
  if (root.success())
  {
    JsonArray &leds = root["leds"];
    setLeds(leds);
  }
  else
  {
    Serial.println("Could not parse settings.txt as JSON");
  }
  f.close();
}

// IO
String fileRead(String name)
{
  SPIFFS.begin();
  //read file from SPIFFS and store it as a String variable
  String contents;
  File file = SPIFFS.open(name, "r");
  if (!file)
  {
    String errorMessage = "Can't open '" + name + "' !\r\n";
    Serial.println(errorMessage);
    return "FILE ERROR";
  }
  else
  {
    while (file.available())
    {
      //Lets read line by line from the file
      String line = file.readStringUntil('\n');
      contents += line;
    }
    file.close();
    return contents;
  }
}

// MQTT stuff
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming! topic: " + topic + " - payload: " + payload);
  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

String IpAddress2String(const IPAddress &ipAddress)
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}

void checkForUpdates()
{
  Serial.println("Checking for updates");
  // checkForUpdates(devHost, port);
}

String jsonToString(JsonObject obj)
{
  String result;
  obj.printTo(result);
  return result;
}

void sendHello()
{
  Serial.println("sending a hello");
  if (client.connected()) {
    Serial.println("client is connected");
    client.publish("/leds", "{\"hello\": \"there\"}");
    client.publish(DEVCHANNEL, "{\"hello\": \"there\"}");
  }
}

void sendMessage(String message)
{
  Serial.println("sending a message");
  Serial.println(message);
  if (client.connected()) {
    Serial.println("client is connected");
    client.publish("/leds", message);
    //    client.publish(DEVCHANNEL, message);
  }
}

void sendMessage(JsonObject &object)
{
  String message;
  object.printTo(message);
  sendMessage(message);
}

/*
  {"action":"command","right":{"mode":"color","r":33,"g":62,"b":207},"left":{"mode":"color","r":33,"g":62,"b":207},"brightness":10}

  {"action":"command","right":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"left":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"brightness":10}

  {"action":"wifi-settings-status"}
  {"action":"show-files"}

*/
