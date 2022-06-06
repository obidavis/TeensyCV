#pragma once

template <typename T>
class ThresholdObjectDetector
{
public:
  ThresholdObjectDetector(float tolerance = 0.1f)
      : tolerance(tolerance) 
  {
  }
  void updateThreshold(T thresh)
  {
    threshold = thresh;
  }
  bool isObjectPresent(T pixel)
  {
    return pixel * (1.0f + tolerance) > threshold;
  }

protected:
  T threshold;
  float tolerance;
};

