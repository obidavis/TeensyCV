#pragma once

#include <stdint.h>

// hsv to rgb565 conversion
uint16_t hsv2rgb565(uint16_t hue, uint8_t sat, uint8_t val)
{
  uint16_t r, g, b;
  uint16_t rgb565;

  hue %= 360;
  sat = sat * 255 / 100;
  val = val * 255 / 100;

  if (hue < 60)
  {
    r = 255;
    g = hue * 255 / 60;
    b = 0;
  }
  else if (hue < 120)
  {
    r = (120 - hue) * 255 / 60;
    g = 255;
    b = 0;
  }
  else if (hue < 180)
  {
    r = 0;
    g = 255;
    b = (hue - 120) * 255 / 60;
  }
  else if (hue < 240)
  {
    r = 0;
    g = (240 - hue) * 255 / 60;
    b = 255;
  }
  else if (hue < 300)
  {
    r = (hue - 240) * 255 / 60;
    g = 0;
    b = 255;
  }
  else
  {
    r = 255;
    g = 0;
    b = (360 - hue) * 255 / 60;
  }

  r = (r * sat) / 255;
  g = (g * sat) / 255;
  b = (b * sat) / 255;

  rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

  return rgb565;
}


void populate_rgb565_palette(uint16_t *palette, uint16_t palette_len, uint16_t low_hue, uint16_t high_hue, bool short_hues = false, uint8_t saturation = 100, uint8_t value = 100)
{
  uint16_t hue_delta = (360 + high_hue - low_hue) % 360;
  bool forward_hues;
  if (short_hues)
  {
    forward_hues = true;
    if (hue_delta > 180)
    {
      forward_hues = false;
    }
  }
  if (!short_hues)
  {
    forward_hues = true;
    if (hue_delta < 179)
    {
      forward_hues = false;
    }
  }
  if (forward_hues)
  {
    for (uint16_t i = 0; i < palette_len; i++)
    {
      uint16_t hue = (low_hue + i * hue_delta / palette_len) % 360;
      palette[i] = hsv2rgb565(hue, saturation, value);
    }
  }
  else
  {
    hue_delta = 360 - hue_delta;
    for (uint16_t i = 0; i < palette_len; i++)
    {
      uint16_t hue = (360 + low_hue - i * hue_delta / palette_len) % 360;
      palette[i] = hsv2rgb565(hue, saturation, value);
    }
  }
}

