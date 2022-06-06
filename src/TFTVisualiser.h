#include <SparkFun_GridEYE_Arduino_Library.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "GMGBackgroundSubtractor.h"
#include "utils.h"

class TFTVisualiser
{
public:
  TFTVisualiser(
      Adafruit_ST7735 *tft_ptr,
      GridEYE *grideye_ptr,
      GMGBackgroundSubtractor<float, 8, 8, 32> *bg_subtractor_ptr)
      : tft(tft_ptr),
        grideye(grideye_ptr),
        bg_subtractor(bg_subtractor_ptr)
  {
    // default values
    refresh_rate = 50;
    isTraining = false;
  }

  void init()
  {
    tft->initR(INITR_144GREENTAB);
    tft->setRotation(0);
    
    populate_rgb565_palette(palette, sizeof(palette) / sizeof(palette[0]), 245, 0, false, 95);
    for (uint16_t i = 0; i < sizeof(palette) / sizeof(palette[0]); i++)
    {
      tft->fillRect(
        0,
        i * tft->height() / (sizeof(palette) / sizeof(palette[0])),
        tft->width(),
        tft->height() / (sizeof(palette) / sizeof(palette[0])),
        palette[i]);
    }
    delay(2000);


    tft->fillScreen(ST77XX_BLACK);
    tft->setTextSize(1);

    int16_t x, y;
    uint16_t w, h;
    tft->getTextBounds("0", 0, 0, &x, &y, &w, &h);
    top_buffer_size = h + 1;
    step_x = tft->width() / 8;
    step_y = (tft->height() - top_buffer_size) / 8;

    ms += refresh_rate; // force refresh
    update();
  }

  void clear_top_portion(void)
  {
    tft->fillRect(0, 0, tft->width(), top_buffer_size, ST77XX_BLACK);
  }

  void clear_bottom_portion(void)
  {
    tft->fillRect(0, top_buffer_size, tft->width(), tft->height() - top_buffer_size, ST77XX_BLACK);
  }

  void print_bg_reset(void)
  {
    clear_bottom_portion();
    tft->setTextColor(ST77XX_CYAN);
    tft->setTextSize(2);
    tft->setCursor(0, top_buffer_size);
    tft->println("\n\nRESETTING BACKGROUND");
    delay(500);
    tft->print("3");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print("2");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print("1");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print(".");
    delay(500);
    tft->print(".");
    delay(500);
  }

  void print_is_training(void)
  {
    char text[] = "TRAINING";
    clear_bottom_portion();
    tft->setTextColor(ST77XX_CYAN);
    tft->setTextSize(2);
    int16_t x, y;
    uint16_t w, h;
    tft->getTextBounds(text, 0, 0, &x, &y, &w, &h);
    tft->setCursor((tft->width() - w) / 2, (tft->height() - h) / 2);
    tft->println(text);
  }

  void print_temp_info(float temp)
  {
    clear_top_portion();
    tft->setTextColor(ST77XX_CYAN);
    tft->setTextSize(1);
    tft->fillRect(0, 0, tft->width(), top_buffer_size, ST77XX_BLACK);
    tft->setCursor(0, 0);
    tft->println("INTERNAL TEMP: " + String(deviceTemp, 1) + " C");
  }

  static uint16_t get_temp_colour(uint16_t *palette, uint16_t palette_len, float temp, float min_temp, float max_temp)
  {
    float temp_range = max_temp - min_temp;
    float temp_normalised = (temp - min_temp) / temp_range;
    temp_normalised = constrain(temp_normalised, 0.0f, 1.0f);
    uint16_t temp_colour = (uint16_t)(temp_normalised * (palette_len - 1));
    return palette[temp_colour];
  }

  void print_fg(void)
  {
    // display IR image and fg predictions
    for (uint16_t i = 0; i < 8 * 8; i++)
    {
      FGResult res = bg_subtractor->isFG(i);
      uint16_t fill_colour = get_temp_colour(palette, sizeof(palette) / sizeof(palette[0]), temp_data_buf[i], deviceTemp * 0.85, deviceTemp *1.25);
      tft->fillRect(
          (i % 8) * step_x,
          top_buffer_size + ((i / 8) * step_y),
          step_x,
          step_y,
          fill_colour);

      if (res.isFG)
      {
        tft->drawRect(
            (i % 8) * step_x,
            top_buffer_size + ((i / 8) * step_y),
            step_x,
            step_y,
            ST77XX_WHITE);
        tft->drawRect(
            ((i % 8) * step_x) + 1,
            (top_buffer_size + ((i / 8) * step_y)) + 1,
            step_x - 2,
            step_y - 2,
            ST77XX_WHITE);
        tft->drawRect(
            ((i % 8) * step_x) + 2,
            (top_buffer_size + ((i / 8) * step_y)) + 2,
            step_x - 4,
            step_y - 4,
            ST77XX_WHITE);
      }
    }

  }

  void update(void)
  {
    if (ms > refresh_rate)
    {

      // only update if temp has changed
      if (grideye->getDeviceTemperature() != deviceTemp)
      {
        deviceTemp = grideye->getDeviceTemperature();
        print_temp_info(deviceTemp);
      }

      // get IR image
      for (uint16_t i = 0; i < 8 * 8; i++)
      {
        float temp_data = grideye->getPixelTemperature(i);
        temp_data_buf[i] = temp_data;
      }

      // update bg model
      bg_subtractor->update(temp_data_buf);


      if (isTraining != bg_subtractor->isTraining())
      {
        isTraining = bg_subtractor->isTraining();
        if (isTraining)
        {
          print_is_training();
        }

      }
      
      if (!isTraining)
      {
        print_fg();
      }
      ms = 0;
    }
  }

protected:
  Adafruit_ST7735 *tft;
  GridEYE *grideye;
  GMGBackgroundSubtractor<float, 8, 8, 32> *bg_subtractor;
  uint16_t palette[16];        // palette to use for temperature visualisation
  uint16_t step_x;          // size of each IR pixel on TFT screen
  uint16_t step_y;          // size of each IR pixel on TFT screen
  uint16_t top_buffer_size; // size of top buffer reserved for printing temp info
  elapsedMillis ms;
  uint16_t refresh_rate;
  float deviceTemp;        // internal temp of grideye
  float temp_data_buf[64]; // buffer for IR data
  bool isTraining;
};
