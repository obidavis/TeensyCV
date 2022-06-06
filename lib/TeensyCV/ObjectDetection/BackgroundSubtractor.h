#pragma once

#include <Arduino.h>

// return value from background subtractors.
struct FGResult
{
  bool isFG; // is this pixel a foreground pixel?
  float confidence; // raw confidence value
};

template <typename T>
class BackgroundSubtractor
{
public:
  virtual void setMinVal(T val) = 0;
  virtual void setMaxVal(T val) = 0;
  virtual FGResult isFG(size_t index) = 0;
  virtual FGResult isFG(size_t x, size_t y) = 0;
  virtual void update(T *src) = 0;
};

template <typename T, size_t N>
class RGABackgroundSubtractor
{
public:
  RGABackgroundSubtractor(float p = 0.01, float k = 2.5)
      : p(p), k(k)
  {
  }
  void init_pixel(T pixel, int index)
  {
    pixels[index] = pixel;
    means[index] = pixel;
    stddevs[index] = 1.0f;
  }
  bool is_foreground(T pixel, int index)
  {
    float mean = (p * pixel) + ((1.0f - p) * means[index]);
    float d = abs(pixel - mean);
    float stddev = sqrtf((pow(d, 2) * p) + (pow(stddevs[index], 2) * (1.0f - p)));
    pixels[index] = pixel;
    means[index] = mean;
    stddevs[index] = stddev;
    return d > k * stddev;
  }

protected:
  T pixels[N];
  T means[N];
  T stddevs[N];
  float p;
  float k;
};


template <typename T, size_t N>
class RGABackgroundSubtractorV2 : public RGABackgroundSubtractor<T, N>
{
public:
  RGABackgroundSubtractorV2(float p = 0.01, float k = 2.5)
      : RGABackgroundSubtractor<T, N>(p, k)
  {
  }
  bool is_foreground(T pixel, int index)
  {
    // float pixel_smoothing = 0.01;
    // pixel = (pixel * pixel_smoothing) + ((1.0f - pixel_smoothing) * this->pixels[index]);
    float d = abs(pixel - this->means[index]);
    float stddev = sqrtf((pow(d, 2) * this->p) + (pow(this->stddevs[index], 2) * (1.0f - this->p)));
    this->pixels[index] = pixel;
    this->stddevs[index] = stddev;
    if (d > this->k * stddev)
    {
      float mean = (this->p * pixel) + ((1.0f - this->p) * this->means[index]);
      this->means[index] = mean;
      return true;
    }
    return false;
  }
};

template <typename T, size_t N>
class HeuristicObjectDetector
{
public:
  HeuristicObjectDetector(float tolerance = 0.1f)
      : tolerance(tolerance)
  {
  }
  bool isPixelAboveThreshold(float pixel)
  {
    return pixel * (1.0f + tolerance) > threshold;
  }
  float calcStdDev(T pixel, int index)
  {
    return sqrtf(
      (pow(abs(pixel - this->means[index]), 2) * this->window_size) + 
      (pow(this->stddevs[index], 2) * (1.0f - this->window_size)));
  }
  float calcMean(T pixel, int index)
  {
    return (pixel * this->window_size) + 
      (this->means[index] * (1.0f - this->window_size));
  }
  bool isPixelForeground(T pixel, int index)
  {
    float mean = calcMean(pixel, index);
    float d = abs(pixel - mean);
    float stddev = calcStdDev(pixel, index);
    return d > this->confidence_threshold * stddev;
  }
  bool isObject(T pixel, int index)
  {
    float mean = calcMean(pixel, index);
    float d = abs(pixel - mean);
    float stddev = calcStdDev(pixel, index);

    if (isPixelAboveThreshold(pixel))
    {
      if (isPixelForeground(pixel, index))
      {

      }
    }
    return false;
  }
protected:
  float threshold;
  float tolerance;
  T data[N];
  float means[N];
  float stddevs[N];
  float window_size;
  float confidence_threshold;
};
