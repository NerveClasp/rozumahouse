// abandoning this for now
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "FS.h"
#include <esp_wifi.h>
#include <WebSocketsClient.h>
#include <Esp.h>
#include <Update.h>

//#define ARDUINOJSON_ENABLE_PROGMEM 0

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

const char *MODEL = "esp32-dual-leds"; // do not change if you want OTA updates
// Fixed definitions cannot change on the fly.
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
#define COLOR_ORDER GRB // It's GRB for WS2812B

#define NUM_STRIPS 2

#define LED_DT_ONE 26    // Serial data pin
#define NUM_LEDS_ONE 180 // Number of LED's
#define LED_DT_TWO 27    // Serial data pin
#define NUM_LEDS_TWO 180 // Number of LED's

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

Settings config[NUM_STRIPS] = {
    {180,           // activeLeds strip 1
     6000,          // animationDuration
     10,            // brightness
     "forward",     // animation
     true,          // ledOn
     {{20, 3, 178}, // RGBColor
      {178, 3, 105},
      {20, 3, 178},
      {-1, -1, -1}}},
    {180,           // activeLeds strip 2
     6000,          // animationDuration
     10,            // brightness
     "forward",     // animation
     true,          // ledOn
     {{20, 3, 178}, // RGBColor
      {178, 3, 105},
      {20, 3, 178},
      {-1, -1, -1}}}};

// Internal global variables
int maxActiveLeds[NUM_STRIPS] = {180, 180};
int animation_position[NUM_STRIPS] = {0, 0};
bool animation_forward[NUM_STRIPS] = {true, true};

struct CRGB leds_one[NUM_LEDS_ONE]; // Initialize our LED array.
struct CRGB leds_two[NUM_LEDS_TWO]; // Initialize our LED array.

String previousMessage;
String message = "";

WebSocketsClient webSocket;
// WebSocketsClient devWebSocket;
// char *devHost = "192.168.1.100";
// char *host = "192.168.1.223";
char *host = "192.168.1.100";

//bool socketConnected = false;

int port = 8888;
char *socketPath = "/devices";

void sendDeviceInfo()
{
  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["kind"] = "about";
  root["model"] = MODEL;
  root["ip"] = IpAddress2String(WiFi.localIP());
  root["mac"] = WiFi.macAddress();

  JsonObject &info = root.createNestedObject("info");

  // made with the help of https://arduinojson.org/v5/assistant/
  JsonArray &actions = info.createNestedArray("actions");
  actions.add("command");
  actions.add("send-device-info");

  JsonArray &commands = info.createNestedArray("commands");
  commands.add("led");
  commands.add("brightness");
  commands.add("animation");
  commands.add("color");
  commands.add("ledOn");
  commands.add("animation");
  commands.add("animationDuration");
  commands.add("activeLeds");

  JsonArray &animations = info.createNestedArray("animations");
  animations.add("none");
  animations.add("forward");
  animations.add("backward");
  animations.add("back-and-forth");

  JsonArray &leds = info.createNestedArray("leds");
  JsonArray &s = root.createNestedArray("status");
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    leds.add(i);
    JsonObject &led = status.createNestedObject();
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

  sendMessage(root);
}

void setup()
{
  Serial.begin(115200); // Initialize serial port for debugging.
  delay(1000);          // Soft startup to ease the flow of electrons.

  controllers[0] = &FastLED.addLeds<LED_TYPE, LED_DT_ONE, COLOR_ORDER>(leds_one, NUM_LEDS_ONE);
  controllers[1] = &FastLED.addLeds<LED_TYPE, LED_DT_TWO, COLOR_ORDER>(leds_two, NUM_LEDS_TWO);
  set_max_power_in_volts_and_milliamps(5, 1000); // FastLED power management set at 5V, 500mA
  // FastLED.clear();
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    fill_gradient_RGB(
        controllers[i]->leds(),
        config[i].activeLeds,
        CRGB(
            config[i].color[0].r,
            config[i].color[0].g,
            config[i].color[0].b),
        CRGB(
            config[i].color[1].r,
            config[i].color[1].g,
            config[i].color[1].b),
        CRGB(
            config[i].color[2].r,
            config[i].color[2].g,
            config[i].color[2].b));
  }

  SPIFFS.begin();
  setupWifi();

  // setupSockets();
  webSocket.onEvent(webSocketEvent);
  // devWebSocket.onEvent(webSocketEvent);

  webSocket.begin(host, port, socketPath);
  webSocket.setReconnectInterval(5000);
  // devWebSocket.begin(devHost, port, socketPath);
  // devWebSocket.setReconnectInterval(5000);
}

void loop()
{
  webSocket.loop();
  // devWebSocket.loop();
  while (Serial.available() > 0)
  {
    previousMessage = message;
    message = Serial.readString();
    Serial.println(message);
    parseMessage(message);
  }

  EVERY_N_MILLISECONDS(config[0].animationDuration / config[0].activeLeds)
  {
    animate(0);
  }

  EVERY_N_MILLISECONDS(config[1].animationDuration / config[1].activeLeds)
  {
    animate(1);
  }

  controllers[0]->showLeds(config[0].brightness);
  controllers[1]->showLeds(config[1].brightness);
}

void parseMessage(String message)
{
  StaticJsonBuffer<1000> jb;
  JsonObject &root = jb.parseObject(message);
  if (root.success())
  {
    Serial.println();
    root.printTo(Serial);
    const char *action = root["action"];
    if (strcmp(action, "command") == 0)
    {
      if (root.containsKey("led"))
      {
        setLeds(root);
        return;
      }
    }
    //    else if (strcmp(action, "check-for-updates") == 0)
    //    {
    //      checkForUpdates();
    //    }
    //    else if (strcmp(action, "reboot") == 0)
    //    {
    //      Serial.println("Rebooting");
    //      ESP.restart();
    //    }
    else if (strcmp(action, "send-device-info") == 0)
    {
      sendDeviceInfo();
    }
    else
    {
      Serial.println("action not found");
    }
  }
  else
  {
    Serial.println("Could not parse JSON");
  }
}

void parseMessage(uint8_t *messageUint)
{
  String message = (char *)messageUint;
  parseMessage(message);
}

void setLeds(JsonObject &root)
{
  const int led = root["led"];
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
    resetAnimationPos(led);
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
      config[led].color[0] = {r_0, g_0, b_0};
      config[led].color[1] = {-1, -1, -1};
      config[led].color[2] = {-1, -1, -1};
      config[led].color[3] = {-1, -1, -1};
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
      config[led].color[0] = {r_0, g_0, b_0};
      config[led].color[1] = {r_1, g_1, b_1};
      config[led].color[2] = {-1, -1, -1};
      config[led].color[3] = {-1, -1, -1};
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
      config[led].color[0] = {r_0, g_0, b_0};
      config[led].color[1] = {r_1, g_1, b_1};
      config[led].color[2] = {r_2, g_2, b_2};
      config[led].color[3] = {-1, -1, -1};

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
      config[led].color[0] = {r_0, g_0, b_0};
      config[led].color[1] = {r_1, g_1, b_1};
      config[led].color[2] = {r_2, g_2, b_2};
      config[led].color[3] = {r_3, g_3, b_3};

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
  //  if (root.containsKey("activeLeds"))
  //  {
  //    config[led].activeLeds = root["activeLeds"];
  //    setActiveLeds(led);
  //  }
}

//void setActiveLeds(int led)
//{
//  fill_gradient_RGB(&(controllers[led]->leds()[config[led].activeLeds]), maxActiveLeds[led] -config[led].activeLeds, CRGB(0, 0, 0));
//}

void animate(int led)
{
  if (config[led].animation == "back-and-forth")
  {
    animateBackAndForth(led);
  }
  if (config[led].animation == "forward")
  {
    animateForward(led);
  }
  if (config[led].animation == "backward")
  {
    animateBackward(led);
  }
}

void resetAnimationPos(int led)
{
  animation_forward[led] = true;
  animation_position[led] = 0;
}

void animateBackAndForth(int led)
{
  if (animation_forward[led])
  {
    if (animation_position[led] == config[led].activeLeds)
    {
      animation_forward[led] = false;
      animateBackward(led);
    }
    else
    {
      animateForward(led);
      animation_position[led] += 1;
    }
  }
  else
  {
    if (animation_position[led] == 1)
    {
      animation_forward[led] = true;
      animateForward(led);
    }
    else
    {
      animateBackward(led);
      animation_position[led] -= 1;
    }
  }
}

void animateBackward(int led)
{
  CRGB firstColor = controllers[led]->leds()[0];
  int ledsNum = config[led].activeLeds;
  for (int i = 0; i < ledsNum - 1; i++)
  {
    controllers[led]->leds()[i] = controllers[led]->leds()[i + 1];
  }
  controllers[led]->leds()[ledsNum - 1] = firstColor;
}

void animateForward(int led)
{
  int ledsNum = config[led].activeLeds;
  CRGB lastColor = controllers[led]->leds()[ledsNum - 1];
  for (int i = ledsNum - 1; i > 0; i--)
  {
    controllers[led]->leds()[i] = controllers[led]->leds()[i - 1];
  }
  controllers[led]->leds()[0] = lastColor;
}

// SETUP
void setupWifi()
{
  if (SPIFFS.exists("/wifi.txt"))
  {
    File f = SPIFFS.open("/wifi.txt", "r");
    if (!f)
    {
      Serial.println("Wifi settings file failed to open!");
    }
    else
    {
      String wifiSettings = fileRead("/wifi.txt");
      StaticJsonBuffer<1000> jb;
      JsonObject &root = jb.parseObject(wifiSettings);
      if (root.success())
      {
        const char *ssid = root["name"];
        const char *password = root["password"];
        WiFi.begin(ssid, password);

        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.print(".");
        }
        Serial.println();

        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());
      }
      else
      {
        Serial.println("Could not parse wifi.txt as JSON");
      }
    }
  }
  else
  {
    Serial.println("wifi.json file not found!\nPlease change wifi_example.txt and rename it to wifi.txt");
  }
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

// Socket stuff
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    Serial.println("Socket disconnected");
    break;
  }
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    // send message to server when Connected
    // socket.io upgrade confirmation message (required)
    // sendMessage("5");
    // webSocket.sendTXT("5");
    sendDeviceInfo();
    break;
  }
  case WStype_TEXT:
  {
    parseMessage(payload);
    break;
  }
  case WStype_BIN:
  {
    Serial.printf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);
    // send data to server
    // webSocket.sendBIN(payload, length);
    break;
  }
  }
}

String IpAddress2String(const IPAddress &ipAddress)
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}

String jsonToString(JsonObject obj)
{
  String result;
  obj.printTo(result);
  return result;
}
// Send Json via Socket connection
void sendMessage(String message)
{
  webSocket.sendTXT(message);
  // devWebSocket.sendTXT(message);
}

void sendMessage(JsonObject &object)
{
  String message;
  object.printTo(message);
  sendMessage(message);
}
