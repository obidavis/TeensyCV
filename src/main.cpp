// #include <Wire.h>
// #include <SPI.h>

// #include <SparkFun_GridEYE_Arduino_Library.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ST7735.h>

// #include "GMGBackgroundSubtractor.h"
// #include "TFTVisualiser.h"
// #include "MaxSerialVisualiser.h"
// #include "TerminalSerialVisualiser.h"

// #define TFT_CS 10
// #define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
// #define TFT_DC 8

// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// GridEYE grideye = GridEYE();
// GMGBackgroundSubtractor<float, 8, 8, 32> gmg_bg_subtractor;
// TFTVisualiser visualiser(&tft, &grideye, &gmg_bg_subtractor);
// // MaxSerialVisualiser visualiser(&grideye, &gmg_bg_subtractor);
// // TerminalSerialVisualiser visualiser(&grideye, &gmg_bg_subtractor);

// void setup(void)
// {
//   Serial.begin(9600);
//   Wire.begin();
//   grideye.begin();
  
//   float deviceTemp = grideye.getDeviceTemperature();
//   gmg_bg_subtractor.setMinVal(deviceTemp - 8.0f);
//   gmg_bg_subtractor.setMaxVal(deviceTemp + 8.0f);
  
//   visualiser.init();
// }

// void loop()
// {
//   visualiser.update();
// }
