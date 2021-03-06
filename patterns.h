/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "palettes.h"
#include "twinkleFox.h"
#include "xy.h"
#include "readFile.h"

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, speed);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(speed, 0, NUM_LEDS - 1);
  static int prevpos = 0;
  CRGB color = ColorFromPalette(palettes[currentPaletteIndex], gHue, 255);
  if ( pos < prevpos ) {
    fill_solid( leds + pos, (prevpos - pos) + 1, color);
  } else {
    fill_solid( leds + prevpos, (pos - prevpos) + 1, color);
  }
  prevpos = pos;
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t beat = beatsin8( speed, 64, 255);
  CRGBPalette16 palette = palettes[currentPaletteIndex];
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + speed, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void showSolidColor()
{
  fill_solid(leds, NUM_LEDS, solidColor);
}

// My first go at flying the Union Jack flag
// Read csv file from SPIFFS to hold the colour anchors for each pixel and then poll through the colour
// gradients to give the impression the flag is waving in the wind
// Inputs:
// Filename refers to a csv file that holds a palette index for eaxh pixel you have in your matrix
// palette represents the colours referenced by the csv file
void waveFlag(char fileName[50], CRGBPalette16 palette) {
  static uint8_t strip[NUM_LEDS];
  static uint8_t firstTime = 1;
  const uint8_t freq = 4;  // wave frequency 
  const uint8_t slope = 5; //higher = milder
  
  if (firstTime) {
    //load up strip with color indices
    FetchColorsFromFile("/unionJack.csv", strip);
    firstTime = 0;
    
    //Add some sloped waves to the flag
    for ( int i = 1; i < NUM_TOTAL_STRIPS; i++) {
      for (int j = 1; j < NUM_LEDS_PER_LONG_STRIP; j++) {
        strip[XY(i,j)]+= mod8((i*freq) + (j/slope), 32);
      }
    }
  }
  else {
    for ( int i = 0; i < NUM_LEDS; i++) {
      //Advance colours
      strip[i] = mod8((strip[i] + freq), 32);
      //assign colors to leds
      leds[i] = ColorFromPalette(palette, strip[i]);
    }
  }
}

void unionJack()
{
  waveFlag("/unionJack.csv", UnionJack_p);
}


// My first stab at emulating the iconic raining code from the movie The Matrix
// The Palette parameter allows you to use an alternative to the traditional green monitor effect
void codeFall(CRGBPalette16 palette) {
  //Render all the neopixels Black
  fill_solid(leds, NUM_LEDS, CRGB::Black);  

  //Declare a Trail type
  // Trail comprises of x, y coords, governing throttle, and life span
  typedef struct Trail{
    uint8_t x;
    uint8_t y;
    uint8_t throttle; //higher reduces speed, lower increases speed (timer reset value)
    uint8_t timer;  //advance every time timer expires 
    uint8_t lifeSpan; // determines if trail reaches the bottom
  } ;

  // Declare a list of Trails that we will track
  Trail trailList[NUM_TRAILS];
  
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  //color picker for our pallette
  uint8_t colorIndex;

  // Dim every cell by 10% (26/256) each time
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeLightBy(26);
  }

  //Go through the trails and advance them forward where timers have expired
  for (uint8_t i = 0; i < NUM_TRAILS; i++) {
    //Check if Trail is active
    if (trailList[i].lifeSpan > 0) {
      //check if wait is over
      if (trailList[i].timer > 0) {
        //coontinue with countdown of waiting
        trailList[i].timer--;
      }
      else {
        //Advance trail down one
        trailList[i].y --;
        // Deplete the life span
        trailList[i].lifeSpan --;
        //Randomly select a colour from the first four palette colours
        colorIndex = random8(4);
        //Assign the color to the head of the trail
        leds[XY(trailList[i].x,trailList[i].y)] = ColorFromPalette(palette, colorIndex);
        //Set the tail of the trail 
        leds[XY(trailList[i].x, trailList[i].y + 1)] = ColorFromPalette(palette, colorIndex + 4);
        leds[XY(trailList[i].x, trailList[i].y + 2)] = ColorFromPalette(palette, colorIndex + 8);
        // The remaining part of the tail will fade as part of the general dimming

        //Reset the timer
        trailList[i].timer = trailList[i].throttle;
        //Trail's life ends at reaching the bottom 
        if (trailList[i].y == 0) {trailList[i].lifeSpan = 0;}
      }
    }
    else {
      //Create a new Trail
      //Pick a random column
      trailList[i].x = random8(NUM_TOTAL_STRIPS);
      //Built in random pause by placing starting height position beyond top led)
      trailList[i].y = random8(NUM_LEDS_PER_LONG_STRIP/3) + NUM_LEDS_PER_LONG_STRIP;
      trailList[i].throttle = random8(31) + 20; // Higher is slower; Lower is faster (Range:20-50)
      trailList[i].timer = 0;
      //Default lifespan for Trail to reach the bottom
      trailList[i].lifeSpan = trailList[i].y;
      //  Once in a while shorten the lifespan so that the trail stops short
      if (!(random8(5)%5)) {trailList[i].lifeSpan -= NUM_LEDS_PER_LONG_STRIP * random8(75) / 100;}
    }
  }
}

void codeFall(){
  codeFall(Code_Fall_gp);
}

// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
void heatMap(CRGBPalette16 palette, bool up)
{
  //Render all the neopixels Black
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(256));

  // Array of temperature readings at each simulation cell (missing cells included)
  static byte heat[NUM_TOTAL_STRIPS][NUM_LEDS_PER_LONG_STRIP];

  byte colorindex[NUM_TOTAL_STRIPS];

  // Step 1.  Cool down every cell a little
  for ( uint16_t i = 0; i < NUM_TOTAL_STRIPS; i++) {
    for ( uint16_t j = 0; j < NUM_TOTAL_STRIPS; j++) {
       heat[i][j] = qsub8( heat[i][j],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
    }
  }

  // Step 2.  For each strip heat from each cell drifts 'up' and diffuses a little
  for ( uint16_t i = 0; i < NUM_TOTAL_STRIPS; i++) {
    for ( uint16_t k = NUM_LEDS_PER_LONG_STRIP - 1; k >= 2; k--) {
      heat[i][k] = (heat[i][k - 1] + heat[i][k - 2] + heat[i][k - 2] ) / 3;
    }
  }

  // Step 3. For each strip, randomly ignite new 'sparks' of heat near the bottom
  for ( uint16_t i = 0; i < NUM_TOTAL_STRIPS; i++) {
    if ( random8() < sparking ) {
      int y = random8(NUM_LEDS_PER_LONG_STRIP/6);
      heat[i][y] = qadd8( heat[i][y], random8(160, 255) );
    }
  }

  // Step 4.  For each strip, map from heat cells to LED colors
  
  for ( uint16_t i = 0; i < NUM_TOTAL_STRIPS; i++) {
    for ( uint16_t j = 0; j < NUM_LEDS_PER_LONG_STRIP; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      colorindex[i] = scale8(heat[i][j], 190);
  
      CRGB color = ColorFromPalette(palette, colorindex[i]);
      // Apply heat matrix to full length of leds 
      // And rely upon the XY function to ignore coords that do not have a corresponding neopixel 
      if (up) {
        leds[XY(i,j)] = color;
      }
      else {
        leds[XY(i,NUM_LEDS_PER_LONG_STRIP - 1 - j)] = color;
      }
    }
  }
}

void fire()
{
  heatMap(HeatColors_p, true);
}

void water()
{
  heatMap(IceColors_p, false);
}



// Pride2015 by Mark Kriegsman: https://gist.github.com/kriegsman/964de772d64c502760e5
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}

// ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette)
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  // uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8( index);
    index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette( palette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds - 1) - pixelnumber;

    nblend( ledarray[pixelnumber], newcolor, 128);
  }
}

void colorWaves()
{
  colorwaves(leds, NUM_LEDS, currentPalette);
}

typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

PatternAndNameList patterns = {
  { pride,                  "Pride" },
  { colorWaves,             "Color Waves" },

  // TwinkleFOX patterns
  { drawTwinkles, "Twinkles" },

  // Fire & Water
  { fire, "Fire" },
  { water, "Water" },

  // DemoReel100 patterns
  { rainbow, "rainbow" },
  { rainbowWithGlitter, "rainbowWithGlitter" },
  { confetti, "confetti" },
  { sinelon, "sinelon" },
  { juggle, "juggle" },
  { bpm, "bpm" },

  { showSolidColor,         "Solid Color" },
  
  // Home made patterns
  { unionJack,      "Fly Union Jack" },
  { codeFall,        "Matrix code drop" },
};

const uint8_t patternCount = ARRAY_SIZE(patterns);
