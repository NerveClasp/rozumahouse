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
#define LED_DT_LEFT D8   // Serial data pin
#define LED_DT_RIGHT D12 // Serial data pin

#define COLOR_ORDER GRB // It's GRB for WS2812B
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
// #define NUM_LEDS 16     // Number of LED's
#define NUM_LEDS 180 // Number of LED's
#define NUM_STRIPS 2
CLEDController *controllers[NUM_STRIPS];
int activeLeds = 180;
// #define NUM_LEDS 200 // Number of LED's

// Initialize changeable global variables.
// int brightness = 10; // Overall brightness definition. It can be changed on the fly.
int animation_delay = 24;
int brightness[NUM_STRIPS] = {20, 20}; // Overall brightness definition. It can be changed on the fly.

struct CRGB leds_one[NUM_LEDS];  // Initialize our LED array.
struct CRGB leds_two[NUM_LEDS]; // Initialize our LED array.

String previousMessage;
String message = "";
const char *mode[NUM_STRIPS] = {"gradient_rgb", "gradient_rgb"};
const char *animation[NUM_STRIPS] = {"back-and-forth", "back-and-forth"};

int animation_position[NUM_STRIPS] = {0, 0};
bool animation_forward[NUM_STRIPS] = {true, true};

WebSocketsClient webSocket;
WebSocketsClient devWebSocket;
char *devHost = "192.168.1.100";
char *host = "192.168.1.223";

int port = 8888;
char *socketPath = "/devices";

void setup()
{
  Serial.begin(115200); // Initialize serial port for debugging.
  delay(1000);          // Soft startup to ease the flow of electrons.

  controllers[0] = &FastLED.addLeds<LED_TYPE, LED_DT_LEFT, COLOR_ORDER>(leds_one, NUM_LEDS);
  controllers[1] = &FastLED.addLeds<LED_TYPE, LED_DT_RIGHT, COLOR_ORDER>(leds_two, NUM_LEDS);
  //  LEDS.addLeds<LED_TYPE, LED_DT_LEFT, COLOR_ORDER>(leds_one, NUM_LEDS);
  //  LEDS.addLeds<LED_TYPE, LED_DT_RIGHT, COLOR_ORDER>(leds_two, NUM_LEDS);

  // FastLED.setBrightness(brightness);
  set_max_power_in_volts_and_milliamps(5, 1000); // FastLED power management set at 5V, 500mA
  FastLED.clear();
  /*
startColor: { r: 120, g: 3, b: 178 },
      endColor: { r: 178, g: 3, b: 105 },
  */
  fill_gradient_RGB(leds_two, NUM_LEDS, CRGB(120, 3, 178), CRGB(178, 3, 105));
  fill_gradient_RGB(leds_one, NUM_LEDS, CRGB(120, 3, 178), CRGB(178, 3, 105));

  // fill_solid(leds_two, NUM_LEDS, CRGB(right_r, right_g, right_b));
  // fill_solid(leds_one, NUM_LEDS, CRGB(left_r, left_g, left_b));

  SPIFFS.begin();
  setupWifi();

  webSocket.onEvent(webSocketEvent);
  webSocket.begin(host, port, socketPath);
  devWebSocket.onEvent(webSocketEvent);
  devWebSocket.begin(devHost, port, socketPath);
}

void loop()
{
  webSocket.loop();
  devWebSocket.loop();
  while (Serial.available() > 0)
  {
    previousMessage = message;
    message = Serial.readString();
    Serial.println(message);
    parseMessage(message);
  }

  animate(0);
  controllers[0]->showLeds(brightness[0]);

  //  FastLED[0].show(brightness_left);
  //  FastLED[1].show(brightness_right);
  //  controllers[0]->
  animate(1);
  controllers[1]->showLeds(brightness[1]);

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
      if (root.containsKey("which"))
      {
        setLeds(root);
        return;
      }
      if (root.containsKey("animation_delay"))
      {
        animation_delay = root["animation_delay"];
      }
      // if (root.containsKey("activeLeds"))
      // {
      //   int newActiveLeds = root["activeLeds"];
      //   activeLeds = newActiveLeds;
      //   setActiveLeds();
      // }
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
  const int which = root["which"];
  if (root.containsKey("animation")) // TODO: think if this is needed
  {
    animation[which] = root["animation"];
  }

  if (root.containsKey("brightness")) // TODO: think if this is needed
  {
    brightness[which] = root["brightness"];
  }

  if (root.containsKey("mode"))
  {
    const char *mode = root["mode"];
    if (strcmp(mode, "solid_rgb") == 0)
    {
      fill_solid(controllers[which]->leds(), activeLeds, CRGB(root["r"], root["g"], root["b"]));
    }

    if (strcmp(mode, "gradient_rgb") == 0)
    {
      JsonObject &from = root["from"];
      JsonObject &to = root["to"];

      fill_gradient_RGB(
          controllers[which]->leds(),
          activeLeds,
          CRGB(from["r"], from["g"], from["b"]),
          CRGB(to["r"], to["g"], to["b"]));
      // resetAnimationPos();
    }
    if (strcmp(mode, "off") == 0)
    {
      fill_solid(controllers[which]->leds(), activeLeds, CRGB(0, 0, 0));
      // resetAnimationPos();
    }
  }
  // setActiveLeds();
  FastLED.delay(animation_delay);
}

void setActiveLeds()
{
  if (NUM_LEDS > activeLeds)
  {
    //    leds_one(activeLeds, NUM_LEDS - activeLeds).fillSolid(CHSV(0, 0, 0));
    //    leds_two(activeLeds, NUM_LEDS - activeLeds).fillSolid(CHSV(0, 0, 0));
    fill_solid(&(leds_one[activeLeds]), NUM_LEDS - activeLeds, CRGB(0, 0, 0));
    fill_solid(&(leds_two[activeLeds]), NUM_LEDS - activeLeds, CRGB(0, 0, 0));
  }
}

void animate(int which)
{
  if (strcmp(animation[which], "none") == 0)
  {
    resetAnimationPos(which);
    return;
  }
  if (strcmp(animation[which], "back-and-forth") == 0)
  {
    animateBackAndForth(which);
  }
  if (strcmp(animation[which], "forward") == 0)
  {
    animateForward(which);
  }
  if (strcmp(animation[which], "backward") == 0)
  {
    animateBackward(which);
  }
}

void resetAnimationPos(int which)
{
  animation_forward[which] = true;
  animation_position[which] = 0;
}

void animateBackAndForth(int which)
{
  if (animation_forward[which])
  {
    if (animation_position[which] == activeLeds)
    {
      animation_forward[which] = false;
      animation_position[which] = activeLeds - 1;
    }
    else
    {
      animation[which] = "forward";
      animateForward(which);
      animation_position[which] += 1;
    }
  }
  else
  {
    if (animation_position[which] < 0)
    {
      animation_forward[which] = true;
      animation_position[which] = 0;
    }
    else
    {
      animation[which] = "backward";
      animateBackward(which);
      animation_position[which] -= 1;
    }
  }
}

void animateForward(int which)
{
  if (strcmp(animation[which], "forward") == 0)
  {
    CRGB firstColor = controllers[which]->leds()[0];
    for (int i = 0; i < activeLeds - 1; i++)
    {
      controllers[which]->leds()[i] = controllers[which]->leds()[i + 1];
    }
    controllers[which]->leds()[activeLeds - 1] = firstColor;
  }
}

void animateBackward(int which)
{
  if (strcmp(animation[which], "backward") == 0)
  {
    CRGB lastColor = controllers[which]->leds()[activeLeds - 1];
    for (int i = activeLeds - 1; i > 0; i--)
    {
      controllers[which]->leds()[i] = controllers[which]->leds()[i - 1];
    }
    controllers[which]->leds()[0] = lastColor;
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
    // webSocket.sendTXT("5");
    sendDeviceInfo();
  }
  break;
  case WStype_TEXT:
    // Serial.printf("[WSc] get text: %s\n", payload);
    parseMessage(payload);
    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);
    break;
  default:
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
  info["activeLeds"] = activeLeds;

  // made with the help of https://arduinojson.org/v5/assistant/
  JsonArray &action = info.createNestedArray("action");
  action.add("command");
  action.add("check-for-updates");
  action.add("send-device-info");
  action.add("reboot");

  JsonArray &command = info.createNestedArray("command");
  command.add("which");
  command.add("brightness");
  command.add("animation");
  command.add("mode");
  // command.add("animation_delay");
  // command.add("activeLeds");

  JsonArray &animation = info.createNestedArray("animation");
  animation.add("none");
  animation.add("forward");
  animation.add("backward");
  animation.add("back-and-forth");

  JsonArray &which = info.createNestedArray("which");
  which.add(0);
  which.add(1);
  // which.add("off");

  JsonArray &mode = info.createNestedArray("mode");
  mode.add("solid_rgb");
  mode.add("gradient_rgb");
  mode.add("off");

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
  //  checkForUpdates(host, port); // turn off updates from prod for now
  checkForUpdates(devHost, port);
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
  devWebSocket.sendTXT(message);
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
