#include <ArduinoJson.h>
#include <FastLED.h>
#include "FS.h"
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Esp.h>
#include <ESP8266httpUpdate.h>

#include <Hash.h>
#define ARDUINOJSON_ENABLE_PROGMEM 0

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

const char *MODEL = "esp8266-dual-leds"; // do not change if you want OTA updates
// Fixed definitions cannot change on the fly.
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
#define COLOR_ORDER GRB // It's GRB for WS2812B

#define NUM_STRIPS 2

#define LED_DT_ONE D8    // Serial data pin
#define NUM_LEDS_ONE 180 // Number of LED's

#define LED_DT_TWO D12   // Serial data pin
#define NUM_LEDS_TWO 180 // Number of LED's

CLEDController *controllers[NUM_STRIPS];

// Initialize changeable global variables.
int activeLeds[NUM_STRIPS] = {180, 180};
int animationDuration[NUM_STRIPS] = {6000, 6000};
int brightness[NUM_STRIPS] = {20, 20};
const char *mode[NUM_STRIPS] = {"gradient_rgb", "gradient_rgb"};
const char *animation[NUM_STRIPS] = {"back-and-forth", "back-and-forth"};
bool ledOn[NUM_STRIPS] = {true, true};
// Internal global variables
int maxActiveLeds[NUM_STRIPS] = {180, 180};
int animation_position[NUM_STRIPS] = {0, 0};
bool animation_forward[NUM_STRIPS] = {true, true};
int defaultColors[2][3] = {
    {120, 3, 178},
    {178, 3, 105}};

struct CRGB leds_one[NUM_LEDS_ONE]; // Initialize our LED array.
struct CRGB leds_two[NUM_LEDS_TWO]; // Initialize our LED array.

String previousMessage;
String message = "";

WebSocketsClient webSocket;
// WebSocketsClient devWebSocket;
// char *devHost = "192.168.1.100";
char *host = "192.168.1.223";

int port = 8888;
char *socketPath = "/devices";

void sendDeviceInfo()
{
  StaticJsonBuffer<1500> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["kind"] = "about";
  root["model"] = MODEL;
  root["chipId"] = ESP.getChipId();
  root["freeSketchSpace"] = ESP.getFreeSketchSpace();
  root["sketchMD5"] = ESP.getSketchMD5();
  root["power"] = ESP.getVcc();
  root["coreVersion"] = ESP.getCoreVersion();
  root["sdkVersion"] = ESP.getSdkVersion();
  root["ip"] = IpAddress2String(WiFi.localIP());
  root["mac"] = WiFi.macAddress();

  JsonObject &info = root.createNestedObject("info");

  // made with the help of https://arduinojson.org/v5/assistant/
  JsonArray &actions = info.createNestedArray("actions");
  actions.add("command");
  actions.add("check-for-updates");
  actions.add("send-device-info");
  actions.add("reboot");

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

  // JsonArray &mode = info.createNestedArray("mode");
  // mode.add("color");
  // mode.add("gradient");

  JsonArray &leds = info.createNestedArray("leds");
  JsonArray &status = root.createNestedArray("status");
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    leds.add(i);
    JsonObject &led = status.createNestedObject();
    led["activeLeds"] = activeLeds[i];
    led["brightness"] = brightness[i];
    // led["mode"] = mode[i];
    led["animation"] = animation[i];
    led["animationDuration"] = animationDuration[i];
    led["ledOn"] = ledOn[i];
    JsonArray &color = led.createNestedArray("color");

    JsonObject &color_0 = color.createNestedObject();
    JsonObject &color_0_rgb = color_0.createNestedObject("rgb");
    color_0_rgb["r"] = defaultColors[0][0];
    color_0_rgb["g"] = defaultColors[0][1];
    color_0_rgb["b"] = defaultColors[0][2];

    JsonObject &color_1 = color.createNestedObject();
    JsonObject &color_1_rgb = color_1.createNestedObject("rgb");
    color_1_rgb["r"] = defaultColors[1][0];
    color_1_rgb["g"] = defaultColors[1][1];
    color_1_rgb["b"] = defaultColors[1][2];
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
        activeLeds[i],
        CRGB(
            defaultColors[0][0],
            defaultColors[0][1],
            defaultColors[0][2]),
        CRGB(
            defaultColors[1][0],
            defaultColors[1][1],
            defaultColors[1][2]));
  }

  SPIFFS.begin();
  setupWifi();

  webSocket.onEvent(webSocketEvent);
  webSocket.begin(host, port, socketPath);
  webSocket.setReconnectInterval(5000);
  // devWebSocket.onEvent(webSocketEvent);
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

  EVERY_N_MILLISECONDS(animationDuration[0] / activeLeds[0])
  {
    animate(0);
  }

  EVERY_N_MILLISECONDS(animationDuration[1] / activeLeds[1])
  {
    animate(1);
  }

  controllers[0]->showLeds(brightness[0]);
  controllers[1]->showLeds(brightness[1]);
}

void parseMessage(String message)
{
  StaticJsonBuffer<1000> jb;
  JsonObject &root = jb.parseObject(message);
  if (root.success())
  {
    const char *action = root["action"];
    if (strcmp(action, "command") == 0)
    {
      if (root.containsKey("led"))
      {
        setLeds(root);
        return;
      }
    }
    else if (strcmp(action, "check-for-updates") == 0)
    {
      checkForUpdates();
    }
    else if (strcmp(action, "reboot") == 0)
    {
      Serial.println("Rebooting");
      ESP.restart();
    }
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
    if (strcmp(root["animation"], "") != 0)
    {
      const char *newAnimation = root["animation"];
      animation[led] = newAnimation;
    }
  }

  if (root.containsKey("brightness")) // TODO: think if this is needed
  {
    brightness[led] = root["brightness"];
  }
  if (root.containsKey("animationDuration"))
  {
    animationDuration[led] = root["animationDuration"];
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
      fill_solid(
          controllers[led]->leds(),
          activeLeds[led],
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
      fill_gradient_RGB(
          controllers[led]->leds(),
          activeLeds[led],
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

      fill_gradient_RGB(
          controllers[led]->leds(),
          activeLeds[led],
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

      fill_gradient_RGB(
          controllers[led]->leds(),
          activeLeds[led],
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
    ledOn[led] = root["ledOn"];
    if (!ledOn[led])
    {
      fill_solid(controllers[led]->leds(), maxActiveLeds[led], CRGB(0, 0, 0));
    }
  }
  //  if (root.containsKey("activeLeds"))
  //  {
  //    activeLeds[led] = root["activeLeds"];
  //    setActiveLeds(led);
  //  }
}

//void setActiveLeds(int led)
//{
//  fill_gradient_RGB(&(controllers[led]->leds()[activeLeds[led]]), maxActiveLeds[led] - activeLeds[led], CRGB(0, 0, 0));
//}

void animate(int led)
{
  if (strcmp(animation[led], "back-and-forth") == 0)
  {
    animateBackAndForth(led);
  }
  if (strcmp(animation[led], "forward") == 0)
  {
    animateForward(led);
  }
  if (strcmp(animation[led], "backward") == 0)
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
    if (animation_position[led] == activeLeds[led])
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
  int ledsNum = activeLeds[led];
  for (int i = 0; i < ledsNum - 1; i++)
  {
    controllers[led]->leds()[i] = controllers[led]->leds()[i + 1];
  }
  controllers[led]->leds()[ledsNum - 1] = firstColor;
}

void animateForward(int led)
{
  int ledsNum = activeLeds[led];
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

void showFilesystem()
{
  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    Serial.print(dir.fileName());
    if (dir.fileSize())
    {
      File f = dir.openFile("r");
      Serial.println(f.size());
    }
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
    // Serial.printf("[WSc] Disconnected!\n");
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
    // Serial.printf("[WSc] get text: %s\n", payload);
    parseMessage(payload);
    break;
  }
  case WStype_BIN:
  {
    Serial.printf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);
    break;
  }
  default:
  {
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

void checkForUpdates()
{
  Serial.println("Checking for updates");
  checkForUpdates(host, port);
  // checkForUpdates(devHost, port);
}

void checkForUpdates(char *updateHost, int updatePort)
{
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateHost, updatePort, "/updates", MODEL);
  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.println("[update] Update failed.");
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("[update] Update no Update.");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("[update] Update ok."); // may not called we reboot the ESP
    break;
  default:
    break;
  }
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

/*
  {"action":"command","right":{"mode":"color","r":33,"g":62,"b":207},"left":{"mode":"color","r":33,"g":62,"b":207},"brightness":10}

  {"action":"command","right":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"left":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"brightness":10}

  {"action":"wifi-settings-status"}
  {"action":"show-files"}

*/
