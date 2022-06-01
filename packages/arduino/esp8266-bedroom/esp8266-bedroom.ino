#include <FastLED.h>
FASTLED_USING_NAMESPACE

#include <Esp.h>

// Fixed definitions cannot change on the fly.
#define LED_TYPE WS2812 // What kind of strip are you using (APA102, WS2801 or WS2812B)?
#define COLOR_ORDER GRB // It's GRB for WS2812B

#define NUM_STRIPS 2

// #define LED_DT_ONE D8 // Serial data pin
#define LED_DT_ONE D3    // Serial data pin
#define NUM_LEDS_ONE 240 // Number of LED's

// #define LED_DT_TWO D12 // Serial data pin
#define LED_DT_TWO D4    // Serial data pin
#define NUM_LEDS_TWO 240 // Number of LED's

#define BUTTON_1 A0    // Serial data pin
int button_1 = 0;

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

int ledsOnePreviousBrightness = 10;
int ledsTwoPreviousBrightness = 10;

Settings config[NUM_STRIPS] = {
  { 240,           // activeLeds strip 1
    3000,          // animationDuration
    ledsOnePreviousBrightness,            // brightness
    "forward",     // animation
    true,          // ledOn
    { { 255, 0, 225 }, // RGBColor
    { 0, 0, 255 },
    { 255, 111, 0 },
    { 255, 0, 225 } }
  },
  { 240,           // activeLeds strip 2
    3000,          // animationDuration
    ledsTwoPreviousBrightness,            // brightness
    "forward",     // animation
    true,          // ledOn
    { { 255, 0, 225 }, // RGBColor
    { 0, 0, 255 },
    { 255, 111, 0 },
    { 255, 0, 225 } }
  }
};

// Internal global variables
int maxActiveLeds[NUM_STRIPS] = { 240, 240 };
int animation_position[NUM_STRIPS] = { 0, 0 };
bool animation_forward[NUM_STRIPS] = { true, true };

struct CRGB leds_one[NUM_LEDS_ONE]; // Initialize our LED array.
struct CRGB leds_two[NUM_LEDS_TWO]; // Initialize our LED array.

void setup()
{
  Serial.begin(115200); // Initialize serial port for debugging.
  Serial.println("Starting..");
  pinMode(BUTTON_1, INPUT); 

  controllers[0] = &FastLED.addLeds<LED_TYPE, LED_DT_ONE, COLOR_ORDER>(leds_one, NUM_LEDS_ONE);
  controllers[1] = &FastLED.addLeds<LED_TYPE, LED_DT_TWO, COLOR_ORDER>(leds_two, NUM_LEDS_TWO);
  set_max_power_in_volts_and_milliamps(5, 2000); // FastLED power management set at 5V, 500mA
  FastLED.clear();
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
        config[i].color[2].b),
      CRGB(
        config[i].color[3].r,
        config[i].color[3].g,
        config[i].color[3].b));
  }
}

void loop()
{
  EVERY_N_MILLISECONDS(config[0].animationDuration / config[0].activeLeds)
  {
    animate(0);
  }

  EVERY_N_MILLISECONDS(config[1].animationDuration / config[1].activeLeds)
  {
    animate(1);
  }

  button_1 = analogRead(BUTTON_1);
  if(button_1 > 400){
    if(config[0].brightness != 0){
      ledsOnePreviousBrightness = config[0].brightness;
      ledsTwoPreviousBrightness = config[1].brightness;
    }
    config[0].brightness = 0;
    config[1].brightness = 0;
  }else{
    config[0].brightness = ledsOnePreviousBrightness;
    config[1].brightness = ledsTwoPreviousBrightness;
  }
  controllers[0]->showLeds(config[0].brightness);
  controllers[1]->showLeds(config[1].brightness);

  delay(10);
}

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
