 /*********************************************************************
 * 
 * All functions defined here written by and (c) Davis Haupt under the MIT License.
 * 
 *
 * Graphics and Display Library Written by Limor Fried/Ladyada  for Adafruit Industries.  
 * BSD license, check license.txt for more information
 * All text above, and the splash screen must be included in any redistribution
 *********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_TCS34725.h"
#define OLED_RESET 12
//#include "RTClib.h"

Adafruit_SSD1306 display(OLED_RESET);
//RTC_DS1307 rtc;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

int bpins[] = {5, 4, 3};
int buttons[] = {0, 0, 0};
long last = millis();

unsigned long sinceLast = millis();

int hours = 1;
int minutes = 0;
int seconds = 0;
int ms = 0;

    int lastButtonStates[] = {0, 0, 0};
    long lastDebounceTimes[] = {0, 0, 0};
    long debounceDelay = 50;
    int lastReadings[] = {0,0,0};

void setup() { 
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(10, LOW);
  digitalWrite(11, HIGH);
  
  Serial.begin(9600);
  char h1[] = {__TIME__[0], __TIME__[1], '\0'};
  char m1[] = {__TIME__[3], __TIME__[4], '\0'};
  char s1[] = {__TIME__[6], __TIME__[7], '\0'};
  hours = atoi(h1);
  minutes = atoi(m1);
  seconds = atoi(s1);

//  Wire.begin();
//  rtc.begin();
//  
//  if (! rtc.isrunning()) {
//    Serial.println("RTC is NOT running!");
//    // following line sets the RTC to the date & time this sketch was compiled
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//    // This line sets the RTC with an explicit date & time, for example to set
//    // January 21, 2014 at 3am you would call:
//    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
//  }
  
  
  
  // initialize the display.
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display(); // display the splash screen that we have to
  delay(500);
  display.clearDisplay();
  // text display tests
  display.setTextColor(WHITE);  
  // INPUT_PULLUP because it's one less wire to solder per switch;
  // INPUT needs a connection to 3V and GND as well as the pin, INPUT_PULLUP you just attach to GND.
  pinMode(bpins[0], INPUT_PULLUP); 
  pinMode(bpins[1], INPUT_PULLUP);
  pinMode(bpins[2], INPUT_PULLUP);
    
    Serial.println("Color View Test!");

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
 
  }
  tcs.setInterrupt(true);  // turn off LED
  
}

int mnu = 0;
int timeMode = 0;
int adjustMenu = 0;
long stopWatchStart = -1;
long lastLoop = millis();
boolean paused = false;

void loop() {
  updateInternalTime();
  display.clearDisplay();
  
  if (pressedOnce(1)) {
    mnu = (mnu + 1) % 4;
  }
  if (mnu == 0) {
    if (pressedOnce(0)) {
      timeMode = abs(timeMode-1) % 2;
    } else if (pressedOnce(2)) {
      timeMode = abs(timeMode+1) % 2;
    }
    if (millis() - last >= 40) {
      if (timeMode == 0)
        analogTime();
      else if (timeMode == 1)
        digitalTime();
      last = millis(); 
    }
  }
  else if (mnu == 1) {
    colourview();
  } 
  else if (mnu == 2) {
    stopwatch();
  }
  else if (mnu == 3) {
    adjustTime();
  }
  display.display();
}

void updateButtonStates() {
      for (int i = 0; i < 3; ++i) {
        int reading = digitalRead(bpins[i]);
        if (reading != lastButtonStates[i]) {
          lastDebounceTimes[i] = millis();
        }
        if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
          if (reading == LOW) {
           buttons[i] = 1;
          } else {
           buttons[i] = 0;
          } 
        }
        lastButtonStates[i] = reading;
      }
    }

boolean pressedOnce(int b) {
      updateButtonStates();
      boolean r;
      if (buttons[b] != lastReadings[b] && buttons[b])
        r = true;
      else
        r = false;
      lastReadings[b] = buttons[b];
      return r;
    }


/*
 * The code that keeps the time, displays it, plus a demo function to just run the clock up.
 * Stopwatch included.
*/

void demo() {
  if (minutes >= 60) {
    hours++;
    minutes = 0;
  }
  if (hours > 23)
    hours = 1;
  minutes++;
}

void updateInternalTime() {
  ms += millis() - sinceLast;
  sinceLast = millis();
  if (ms >= 1000) {
    seconds += ms / 1000;
    ms = ms % 1000;
  }
  if (seconds >= 60) {
    minutes += seconds/60;
    seconds = seconds % 60;
  }
  if (minutes >= 60) {
    hours += minutes / 60;
    minutes = minutes % 60;
  }
  if (hours >= 24) {
    hours = 0;
  }
}

void digitalTime() {
//  display.clearDisplay();
  display.setTextSize(4);

  int h = hours % 12;
  if (h == 0) h = 12;

  if (h < 10)
    display.setCursor(17,10);
  else
    display.setCursor(5,10);
  
  display.print(h, DEC);
  display.print(":");
  if (minutes < 10)
    display.print("0");
  display.print(minutes, DEC);
  display.println();
  
  display.setCursor(110, 50);
  display.setTextSize(1);
  display.print(seconds, DEC);
  display.setCursor(0, 50);
  display.print((hours > 11) ? "PM" : "AM");
}


int mins[][2] = {
                {64, 10}, {69, 11}, // :00
                {74, 12}, {79, 16}, // :05
                {83, 20}, {85, 26}, // :10
                {85, 32}, {84, 38}, // :15
                {83, 43}, {78, 49}, // :20
                {73, 52}, {68, 53}, // :25
                {64, 54}, {59, 53}, // :30
                {55, 52}, {50, 47}, // :35
                {45, 43}, {43, 38}, // :40
                {42, 32}, {44, 26}, // :45
                {45, 20}, {50, 16}, // :50
                {53, 14}, {59, 12}  // :55
};
           
int hors[][2] = {  
                {64, 21}, // 12
                {70, 23}, // 1
                {73, 27}, // 2
                {75, 32}, // 3
                {73, 37}, // 4
                {70, 42}, // 5
                {64, 43}, // 6
                {58, 40}, // 7
                {55, 37}, // 8
                {53, 32}, // 9
                {54, 27}, // 10
                {59, 24} // 11
                
};


void analogTime() {
  display.drawCircle(64, 32, 27, WHITE); // clock face
//  int mi = round((minutes/60*24)%23)%60;
  int mi = round(minutes/60.0*23.0);
  int hi = (hours % 12);
  
  display.drawLine(64, 32, mins[mi][0], mins[mi][1], WHITE);
  display.drawLine(64, 32, hors[hi][0], hors[hi%12][1], WHITE);
  display.setCursor(110, 50);
  display.setTextSize(1);
  display.print(seconds, DEC);
  
  display.setCursor(0, 50);
  display.print((hours > 11) ? "PM" : "AM");
}

void adjustTime() {
  
  if (pressedOnce(0)) {
    switch(adjustMenu) {
      case 0:
        hours++;
        break;
      case 1:
        minutes++;
        break;
    }
  }
  else if (pressedOnce(2)) {
    adjustMenu = (adjustMenu + 1) % 2;
  }
  
  if (minutes >= 60) {
    minutes = 0;
    hours++;
  }
  if (hours >= 24)
    hours = 0;
  display.setTextSize(4);
  int h = hours;
  if (hours == 0) {
    h = 12;
  }
  if (h < 10)
    display.setCursor(17,20);
  else
    display.setCursor(5,20);
  
  display.print(h, DEC);
  display.print(":");
  if (minutes < 10)
    display.print("0");
  display.print(minutes, DEC);
  display.println();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print((adjustMenu == 0) ? "H" : "M");
  display.setCursor(20, 0);
  display.print("ADJUST TIME");
  
}


void stopwatch() {
  unsigned long stopTime;
  

  if (pressedOnce(0)) {
    if (stopWatchStart == -1) {
      stopWatchStart = millis();
    } else {
      paused = !paused;
    }
  } 
  else if (pressedOnce(2) && paused) {
    stopWatchStart = -1;
    paused = false;
  }
  
  if (paused) {
    stopWatchStart += (millis() - lastLoop);
  }
  lastLoop = millis();
  if (stopWatchStart != -1) {
    stopTime = millis() - stopWatchStart; 
  }
  else {
    stopTime = 0;
  }
  
  int days, hours, mins, secs;
  int fractime;
  days = hours = mins = secs = fractime = 0;
  unsigned long inttime;

  /*
    davekw7x from Arduino forms wrote the code to get mins, secs, etc. from milliseconds.
   http://forum.arduino.cc/index.php?topic=18588.0
  */
  inttime  = stopTime / 1000;
  fractime = stopTime % 1000;
  // inttime is the total number of number of seconds
  // fractimeis the number of thousandths of a second
  // number of days is total number of seconds divided by 24 divided by 3600
  days     = inttime / (24*3600);
  inttime  = inttime % (24*3600);
  // Now, inttime is the remainder after subtracting the number of seconds
  // in the number of days
  hours    = inttime / 3600;
  inttime  = inttime % 3600;
  // Now, inttime is the remainder after subtracting the number of seconds
  // in the number of days and hours
  mins     = inttime / 60;
  inttime  = inttime % 60;
  // Now inttime is the number of seconds left after subtracting the number
  // in the number of days, hours and minutes. In other words, it is the
  // number of seconds.
  secs = inttime;

  display.setTextSize(2);
  display.setCursor(17,20);
  if (mins < 10)
    display.print("0");
  display.print(mins, DEC);
  display.print(":");
  if (secs < 10)
    display.print("0");
  display.print(secs, DEC);
  display.print(":");
  int centisec = (int)floor(fractime / 10);
  if (centisec < 10)
    display.print("0");
  display.print(centisec);
  display.println();
}

void colourview() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Press the top button    to ID a colour   ");

  if (pressedOnce(0)) {
    uint16_t clear, red, green, blue;
    tcs.setInterrupt(false);      // turn on LED
    delay(60);  // takes 50ms to read 
    tcs.getRawData(&red, &green, &blue, &clear);
    tcs.setInterrupt(true);  // turn off LED

  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  
  display.setTextSize(1);
  display.setCursor(40,20);
  display.clearDisplay();
  display.print('#'); display.print((int)r, HEX); display.print((int)g, HEX); display.println((int)b, HEX);

  display.setCursor(0,40);
  display.print("  R:"); display.print(red);
  display.print(" G:"); display.print(green);
  display.print(" B:"); display.print(blue); display.print("  ");
  //display.println("\tR:\t"); display.print(red); display.print("\tG:\t"); display.print(green); display.print("\tB:\t"); display.print(blue);
  display.display();
 // delay(4500);
 
  while (digitalRead(5) == HIGH) {
  // Do nothing
  if (digitalRead(4) == LOW) {
  //if (pressedOnce(2)) {
    break;
    display.clearDisplay();
      //delay(2250);
      adjustMenu = (adjustMenu + 1) % 2;  
  } }
  }

}
