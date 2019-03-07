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

// Fixed definitions cannot change on the fly.
#define LED_DT_LEFT D8   // Serial data pin
#define LED_DT_RIGHT D12 // Serial data pin

#define COLOR_ORDER GRB // It's GRB for WS2812B
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
// #define NUM_LEDS 16     // Number of LED's
#define NUM_LEDS 32 // Number of LED's
// #define NUM_LEDS 200 // Number of LED's

const char *MODEL = "esp8266-dual-leds"; // do not change if you want OTA updates

// Initialize changeable global variables.
// int brightness = 10; // Overall brightness definition. It can be changed on the fly.
int animation_delay = 24;
int brightness = 254; // Overall brightness definition. It can be changed on the fly.

struct CRGB leds_left[NUM_LEDS];  // Initialize our LED array.
struct CRGB leds_right[NUM_LEDS]; // Initialize our LED array.

String previousMessage;
String message = "";
const char *left_mode = "solid_rgb";
const char *right_mode = "solid_rgb";
const char *animation = "none";
const char *left_animation = "none";
const char *right_animation = "none";

int left_r = 33;
int left_g = 62;
int left_b = 207;

int right_r = 33;
int right_g = 62;
int right_b = 207;

int animation_position = 0;
bool animation_forward = true;

WebSocketsClient webSocket;
char *host = "192.168.1.100";
int port = 8888;
char *socketPath = "/devices";

void setup()
{
  Serial.begin(115200); // Initialize serial port for debugging.
  delay(1000);         // Soft startup to ease the flow of electrons.

  LEDS.addLeds<LED_TYPE, LED_DT_LEFT, COLOR_ORDER>(leds_left, NUM_LEDS);
  LEDS.addLeds<LED_TYPE, LED_DT_RIGHT, COLOR_ORDER>(leds_right, NUM_LEDS);

  FastLED.setBrightness(brightness);
  set_max_power_in_volts_and_milliamps(5, 1000); // FastLED power management set at 5V, 500mA
  FastLED.clear();
  fill_solid(leds_right, NUM_LEDS, CRGB(right_r, right_g, right_b));
  fill_solid(leds_left, NUM_LEDS, CRGB(left_r, left_g, left_b));

  SPIFFS.begin();
  setupWifi();

  webSocket.onEvent(webSocketEvent);
  webSocket.begin(host, port, socketPath);
}

void loop()
{
  webSocket.loop();
  while (Serial.available() > 0)
  {
    previousMessage = message;
    message = Serial.readString();
    Serial.println(message);
    parseMessage(message);
  }

  animate();

  FastLED.show();
  delay(animation_delay); // TODO: set with JSON
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
      if (root.containsKey("right"))
      {
        JsonObject &right = root["right"];
        // Serial.println("right");
        // right.printTo(Serial);
        setLeds(right, "right");
      }
      if (root.containsKey("left"))
      {
        JsonObject &left = root["left"];
        // Serial.println("left");
        // left.printTo(Serial);
        setLeds(left, "left");
      }
      if (root.containsKey("brightness"))
      {
        brightness = root["brightness"]; // 254
        FastLED.setBrightness(brightness);
      }
      if (root.containsKey("animation"))
      {
        animation = root["animation"];
      }
      if (root.containsKey("animation_delay"))
      {
        animation_delay = root["animation_delay"];
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

void setLeds(JsonObject &config, String which)
{
  if (config.containsKey("animation")) // TODO: think if this is needed
  {
    const char *animation = config["animation"];
    // Serial.println(animation);
    if (which == "left")
    {
      left_animation = animation;
    }
    else
    {
      right_animation = animation;
    }
  }

  if (config.containsKey("mode"))
  {
    const char *mode = config["mode"];
    if (strcmp(mode, "solid_rgb") == 0)
    {
      if (which == "left")
      {
        left_mode = "solid_rgb";
        fill_solid(leds_left, NUM_LEDS, CRGB(config["r"], config["g"], config["b"]));
      }
      else
      {
        left_mode = "solid_rgb";
        fill_solid(leds_right, NUM_LEDS, CRGB(config["r"], config["g"], config["b"]));
      }
    }

    if (strcmp(mode, "gradient_rgb") == 0)
    {
      JsonObject &from = config["from"];
      JsonObject &to = config["to"];

      if (which == "left")
      {
        left_mode = "gradient_rgb";
        fill_gradient_RGB(
            leds_left,
            NUM_LEDS,
            CRGB(from["r"], from["g"], from["b"]),
            CRGB(to["r"], to["g"], to["b"]));
      }
      else
      {
        right_mode = "gradient_rgb";
        fill_gradient_RGB(
            leds_right,
            NUM_LEDS,
            CRGB(from["r"], from["g"], from["b"]),
            CRGB(to["r"], to["g"], to["b"]));
      }
      resetAnimationPos();
    }
    if (strcmp(mode, "off") == 0)
    {
      if (which == "left")
      {
        left_mode = "off";
        fill_solid(leds_left, NUM_LEDS, CHSV(0, 0, 0));
      }
      else
      {
        right_mode = "off";
        fill_solid(leds_right, NUM_LEDS, CHSV(0, 0, 0));
      }
      resetAnimationPos();
    }
  }
  delay(animation_delay);
}

void animate()
{
  if (strcmp(animation, "none") == 0)
  {
    resetAnimationPos();
    return;
  }
  if (strcmp(animation, "back-and-forth") == 0)
  {
    animateBackAndForth();
  }
  if (strcmp(animation, "forward") == 0)
  {
    left_animation = "forward";
    right_animation = "forward";
    animateForward();
  }
  if (strcmp(animation, "backward") == 0)
  {
    left_animation = "backward";
    right_animation = "backward";
    animateBackward();
  }

  // delay(animation_delay); // TODO: set with JSON
}

void resetAnimationPos()
{
  animation_forward = true;
  animation_position = 0;
}

void animateBackAndForth()
{
  if (animation_forward)
  {
    if (animation_position == NUM_LEDS)
    {
      animation_forward = false;
      animation_position = NUM_LEDS - 1;
    }
    else
    {
      left_animation = "forward";
      right_animation = "forward";
      animateForward();
      animation_position += 1;
    }
  }
  else
  {
    if (animation_position < 0)
    {
      animation_forward = true;
      animation_position = 0;
    }
    else
    {
      left_animation = "backward";
      right_animation = "backward";
      animateBackward();
      animation_position -= 1;
    }
  }
}

void animateForward()
{
  if (strcmp(left_animation, "forward") == 0)
  {
    CRGB firstColor = leds_left[0];
    for (int i = 0; i < NUM_LEDS - 1; i++)
    {
      leds_left[i] = leds_left[i + 1];
    }
    leds_left[NUM_LEDS - 1] = firstColor;
  }
  if (strcmp(right_animation, "forward") == 0)
  {
    CRGB firstColor = leds_right[0];
    for (int i = 0; i < NUM_LEDS - 1; i++)
    {
      leds_right[i] = leds_right[i + 1];
    }
    leds_right[NUM_LEDS - 1] = firstColor;
  }
}

void animateBackward()
{
  if (strcmp(left_animation, "backward") == 0)
  {
    CRGB lastColor = leds_left[NUM_LEDS - 1];
    for (int i = NUM_LEDS - 1; i > 0; i--)
    {
      leds_left[i] = leds_left[i - 1];
    }
    leds_left[0] = lastColor;
  }
  if (strcmp(right_animation, "backward") == 0)
  {
    CRGB lastColor = leds_right[NUM_LEDS - 1];
    for (int i = NUM_LEDS - 1; i > 0; i--)
    {
      leds_right[i] = leds_right[i - 1];
    }
    leds_right[0] = lastColor;
  }
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

void checkWifiSettingsFile()
{
  String wifiSettings = fileRead("/wifi.txt");

  Serial.println(wifiSettings);
  StaticJsonBuffer<1000> jb;
  JsonObject &root = jb.parseObject(wifiSettings);
  if (root.success())
  {
    const char *name = root["name"];
    const char *password = root["password"];
    // root.printTo(Serial);
  }
  else
  {
    Serial.println("Could not parse JSON");
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
    // Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);

    // send message to server when Connected
    // socket.io upgrade confirmation message (required)
    // sendMessage("5");
    webSocket.sendTXT("5");
    sendDeviceInfo();
  }
  break;
  case WStype_TEXT:
    // Serial.printf("[WSc] get text: %s\n", payload);
    parseMessage(payload);
    // send message to server
    // webSocket.sendTXT("message here");
    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);

    // send data to server
    // webSocket.sendBIN(payload, length);
    break;
  }
}

void sendDeviceInfo()
{
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject &info = jsonBuffer.createObject();
  info["kind"] = "about";
  info["model"] = MODEL;
  info["chipId"] = ESP.getChipId();
  info["freeSketchSpace"] = ESP.getFreeSketchSpace();
  info["sketchMD5"] = ESP.getSketchMD5();
  info["power"] = ESP.getVcc();
  info["coreVersion"] = ESP.getCoreVersion();
  info["sdkVersion"] = ESP.getSdkVersion();
  info["ip"] = IpAddress2String(WiFi.localIP());
  info["mac"] = WiFi.macAddress();

  // made with the help of https://arduinojson.org/v5/assistant/
  JsonArray &action = info.createNestedArray("action");
  action.add("command");
  action.add("check-for-updates");
  action.add("send-device-info");
  action.add("reboot");

  JsonArray &command = info.createNestedArray("command");
  command.add("right");
  command.add("left");
  command.add("brightness");
  command.add("animation");
  command.add("animation_delay");

  JsonArray &animation = info.createNestedArray("animation");
  animation.add("forward");
  animation.add("backward");
  animation.add("back-and-forth");

  JsonArray &left = info.createNestedArray("left");
  left.add("solid_rgb");
  left.add("gradient_rgb");
  left.add("off");

  JsonArray &right = info.createNestedArray("right");
  right.add("solid_rgb");
  right.add("gradient_rgb");
  right.add("off");

  sendMessage(info);
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
  t_httpUpdate_return ret = ESPhttpUpdate.update(host, port, "/updates", MODEL);
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
}

void sendMessage(JsonObject &object)
{
  String message;
  object.printTo(message);
  sendMessage(message);
}

/*
  {"action":"command","right":{"mode":"solid_rgb","r":33,"g":62,"b":207},"left":{"mode":"solid_rgb","r":33,"g":62,"b":207},"brightness":10}

  {"action":"command","right":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"left":{"mode":"gradient_rgb", "from":{"r":254,"g":254,"b":0}, "to":{"r":254,"g":0,"b":254}},"brightness":10}

  {"action":"wifi-settings-status"}
  {"action":"show-files"}

*/
