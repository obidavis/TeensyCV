#pragma once

// return value from background subtractors.
struct FGResult
{
  bool isFG; // is this pixel a foreground pixel?
  float confidence; // raw confidence value
};


// Class for performing background subtraction following "Visual Tracking of Human Visitors under
// Variable-Lighting Conditions for a Responsive Audio Art Installation," A. Godbehere,
// A. Matsukawa, K. Goldberg, American Control Conference, Montreal, June 2012.
// @tparam T The type of the input image.
// @tparam X The width of the input image.
// @tparam Y The height of the input image.
// @tparam F_MAX The maximum number of features in the background model for each pixel.
template <typename T, size_t X, size_t Y, size_t F_MAX>
class GMGBackgroundSubtractor
{
public:
    GMGBackgroundSubtractor(void);
    ~GMGBackgroundSubtractor(void);

    // @brief Sets the model parameters to zero and begins the process at training mode. It is called
    // automatically by the constructor but can be called manually to reset
    void init(void);

    // @brief Update the model and foreground predictions
    // @param image The image to update the model with
    void update(T *src);

    FGResult isFG(size_t idx);
    FGResult isFG(size_t x, size_t y);

    // @brief Is the model in initial training mode?
    bool isTraining(void) const { return _frameNum < _numInitialisationFrames; }

    // getters and setters

    void setNumInitialisationFrames(uint64_t numInitialisationFrames) { _numInitialisationFrames = numInitialisationFrames; }
    uint64_t getNumInitialisationFrames(void) { return _numInitialisationFrames; }

    void setBackgroundPrior(float backgroundPrior) { _backgroundPrior = backgroundPrior; }
    float getBackgroundPrior(void) { return _backgroundPrior; }

    void setLearningRate(float learningRate) { _learningRate = learningRate; }
    float getLearningRate(void) { return _learningRate; }

    void setMinVal(T val) { _minVal = val; }
    T getMinVal(void) { return _minVal; }

    void setMaxVal(T val) { _maxVal = val; }
    T getMaxVal(void) { return _maxVal; }

    void setDecisionThreshold(float val) { _decisionThreshold = val; }
    float getDecisionThreshold(void) { return _decisionThreshold; }

    void setQuantisationLevels(uint16_t val) { _quantisationLevels = val; }
    uint16_t getQuantisationLevels(void) { return _quantisationLevels; }

private:
    // @brief Quantise values according to the minimum and maximum values
    // and the number of quantisation levels
    // @param src The input value
    // @param min The minimum of the range of input values
    // @param max The maximum of the range of input values
    // @param levels The number of quantisation levels
    // @return The quantised value
    uint8_t quantize(T val, T min, T max, uint16_t levels);

    // @brief Update the model and foreground predictions in training mode.
    // This is called by the update function and should not be called directly.
    // Normalises the model after _numInitFrames frames have been processed.
    void train(void);
    
    // @brief Update the internal quantised representation of the image.
    // This is called by the update function and should not be called directly.
    void updateQuantisedImage(T *src);
    
    // @brief Update the internal foreground and background predictions.
    // This is called by the update function and should not be called directly.
    void updatePosteriorImage(void);
    
    // @brief Update the internal foreground and background predictions.
    // This is called by the update function and should not be called directly.
    void updateBinaryImage(void);
    
    // @brief Update the background model during runtime. Only updates
    // if the pixel is not foreground. This is called by the update function and should not be called directly.
    void updateHistogram(void);

    // @brief Placeholder function for smoothing operations on the posterior image.
    // This is called by the update function and should not be called directly.
    // TODO - implement smoothing
    void smoothPosteriorImage(void);

    // @brief Perform smoothing on the binary prediction image. 
    // This is called by the update function and should not be called directly.
    void smoothBinaryImage(void);

    // @brief Print the current state of the model to the serial port.
    // Useful for debugging/insight.
    void printFeatures(void);

    // @brief Representation of a single feature/bin in the model.
    struct Feature
    {
        uint8_t pixelValue; // Quantised pixel value
        float probability;  // Associated weight
    };

    // @brief The background model interpreted as a Probability Mass Function
    // or sparse histogram. Also maintains a count of the number of non-zero bins.
    struct PMF
    {
        size_t featureCount;        // Number of non-zero bins
        Feature features[F_MAX];    // The feature set
    };

    // @brief The background model. Contains a PMF for each pixel.
    PMF pmf[X][Y];

    // @brief Representation of the input image as a quantised image.
    uint8_t _quantisedImage[X][Y];

    // @brief Representation of the input image as the probability of each pixel being foreground.
    float _posteriorImage[X][Y];

    // @brief Representation of the input image as a binary image representing foreground/background.
    bool _binaryImage[X][Y];

    // TODO - There are a lot of different/expensive representations here.
    // This should be optimised for both speed and RAM usage.

    // @brief Current frame index. Incremented each time the update function is called.
    // Used to determine when to exit training mode.
    uint64_t _frameNum;


    // CHANGEABLE PARAMETERS
    // These are given default values in the constructor. Use the setter functions to change them.

    // The number of frames to wait before ending training mode.
    uint64_t _numInitialisationFrames;
    // Prior probability of a pixel being background. Used in Bayes' Rule
    float _backgroundPrior;
    // How quickly the background model updates (EMA).
    float _learningRate;
    // Probability threshold over which a pixel is considered foreground.
    float _decisionThreshold;
    // Number of quantisation levels. Represents the maximum possible features in the model.
    uint16_t _quantisationLevels;
    // Minimum value of the input image.
    T _minVal;
    // Maximum value of the input image.
    T _maxVal;

};

template <typename T, size_t X, size_t Y, size_t F_MAX>
GMGBackgroundSubtractor<T, X, Y, F_MAX>::GMGBackgroundSubtractor(void)
{
    // default values
    _backgroundPrior = 0.8f;
    _learningRate = 0.025f;
    _decisionThreshold = 0.9f;
    _quantisationLevels = 32;
    _minVal = 0.0f;
    _maxVal = 1.0f;
    _numInitialisationFrames = 240;

    init();
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
GMGBackgroundSubtractor<T, X, Y, F_MAX>::~GMGBackgroundSubtractor(void)
{
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::init(void)
{
    for (size_t i = 0; i < X; ++i)
    {
        for (size_t j = 0; j < Y; ++j)
        {
            for (size_t k = 0; k < F_MAX; ++k)
            {
                pmf[i][j].features[k].pixelValue = 0;
                pmf[i][j].features[k].probability = 0.0f;
            }
            pmf[i][j].featureCount = 0;
        }
    }
    _frameNum = 0;
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
uint8_t GMGBackgroundSubtractor<T, X, Y, F_MAX>::quantize(T val, T min, T max, uint16_t levels)
{
    val = constrain(val, min, max);
    float normalisedVal = (val - min) / (float)(max - min);
    return (uint8_t)(normalisedVal * (levels - 1));
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::train(void)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            uint8_t pixelValue = _quantisedImage[x][y];

            // Find and update the PMF for this pixel
            // (Note adding 1.0f is arbitrary as it is normalised later)

            bool present = false; // Is the pixel value already present in the model?
            for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
            {
                if (pmf[x][y].features[i].pixelValue == pixelValue)
                {
                    pmf[x][y].features[i].probability += 1.0f;
                    present = true;
                    break;
                }
            }

            // If the pixel value is not already present, add it to the model
            if (!present)
            {

                // If the model is not full, add the pixel value to the end
                if (pmf[x][y].featureCount < F_MAX)
                {
                    pmf[x][y].features[pmf[x][y].featureCount].pixelValue = pixelValue;
                    pmf[x][y].features[pmf[x][y].featureCount].probability = 1.0f;
                    pmf[x][y].featureCount++;
                }

                // Otherwise discard the oldest feature and add the new one
                else
                {
                    // remove the first element of the array,
                    // shift all elements down by one, and insert the new element
                    // at the end of the array
                    for (size_t i = 0; i < F_MAX - 1; ++i)
                    {
                        pmf[x][y].features[i].pixelValue = pmf[x][y].features[i + 1].pixelValue;
                        pmf[x][y].features[i].probability = pmf[x][y].features[i + 1].probability;
                    }
                    pmf[x][y].features[F_MAX - 1].pixelValue = pixelValue;
                    pmf[x][y].features[F_MAX - 1].probability = 1.0f;
                }
            }
        }
    }

    // The last frame is the end of training
    // Normalise the histogram to get a PMF
    if (_frameNum == _numInitialisationFrames - 1)
    {
        float total;
        for (size_t x = 0; x < X; ++x)
        {
            for (size_t y = 0; y < Y; ++y)
            {
                total = 0.0f;
                for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
                {
                    total += pmf[x][y].features[i].probability;
                }
                if (total != 0.0f)
                {
                    for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
                    {
                        pmf[x][y].features[i].probability /= total;
                    }
                }
            }
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::printFeatures()
{
    Serial.printf("====== FEATURES AT FRAME %d======\n\n", _frameNum);
    for (size_t y = 0; y < Y; ++y)
    {
        for (size_t x = 0; x < X; ++x)
        {
            Serial.printf("(%d, %d)", x, y);
            Serial.print("\t\t\t");
        }
        Serial.println();
        for (size_t i = 0; i < F_MAX; ++i)
        {
            bool isFeatureToPrint = false;
            for (size_t x = 0; x < X; ++x)
            {
                if (pmf[x][y].featureCount > i)
                {
                    isFeatureToPrint = true;
                    Serial.printf("%02d, %.2f\t\t",
                                  pmf[x][y].features[i].pixelValue,
                                  pmf[x][y].features[i].probability);
                }
                else
                {
                    Serial.print("\t\t\t");
                }
            }
            Serial.print("\n");
            if (!isFeatureToPrint)
            {
                break;
            }
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::updateQuantisedImage(T *src)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            _quantisedImage[x][y] = quantize(src[y * Y + x], _minVal, _maxVal, _quantisationLevels);
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::updatePosteriorImage(void)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            uint8_t pixelValue = _quantisedImage[x][y];

            // Attempt to find the pixel value in the model,
            // if it is not present, then 0.0f
            float pPixelGivenBackground = 0.0f;
            for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
            {
                if (pmf[x][y].features[i].pixelValue == pixelValue)
                {
                    pPixelGivenBackground = pmf[x][y].features[i].probability;
                    break;
                }
            }

            // Use Bayes' theorem to calculate the posterior probability
            float pPixelGivenForeground = 1.0f - pPixelGivenBackground;
            float foregroundPrior = 1.0f - _backgroundPrior;
            float pBackgroundGivenPixel = (pPixelGivenBackground * _backgroundPrior) / (pPixelGivenBackground * _backgroundPrior + pPixelGivenForeground * foregroundPrior);
            float pForegroundGivenPixel = 1.0f - pBackgroundGivenPixel;

            // Update the posterior image
            _posteriorImage[x][y] = pForegroundGivenPixel;
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::updateBinaryImage(void)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            _binaryImage[x][y] = _posteriorImage[x][y] > _decisionThreshold;
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::smoothPosteriorImage(void)
{
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::smoothBinaryImage(void)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            // if no neighbouring pixles are foreground, then the pixel is background
            // i.e. mask with
            // 0 1 0
            // 1 0 1
            // 0 1 0
            _binaryImage[x][y] = _binaryImage[x][y] && (((x > 0) ? _binaryImage[x - 1][y] : false) ||
                                                        ((x < X - 1) ? _binaryImage[x + 1][y] : false) ||
                                                        ((y > 0) ? _binaryImage[x][y - 1] : false) ||
                                                        ((y < Y - 1) ? _binaryImage[x][y + 1] : false));
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::updateHistogram(void)
{
    for (size_t x = 0; x < X; ++x)
    {
        for (size_t y = 0; y < Y; ++y)
        {
            // don't update if the pixel has been identified as foreground
            if (_binaryImage[x][y])
            {
                continue;
            }

            uint8_t pixelValue = _quantisedImage[x][y];
            bool present = false;
            float total = 0.0f;
            float minProbability = 1.0f;
            size_t minIndex = 0;

            // find the minimum weight and check whether the current pixel value is present
            for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
            {
                total += pmf[x][y].features[i].probability;
                if (pmf[x][y].features[i].probability < minProbability)
                {
                    minProbability = pmf[x][y].features[i].probability;
                    minIndex = i;
                }

                if (pmf[x][y].features[i].pixelValue == pixelValue)
                {
                    present = true;
                }
            }

            // if the pixel is not present, add it to the histogram
            // either by removing the lowest weighted (if max features has been reached)
            // or by adding it to the histogram
            if (!present)
            {
                if (pmf[x][y].featureCount < F_MAX)
                {
                    pmf[x][y].features[pmf[x][y].featureCount].pixelValue = pixelValue;
                    pmf[x][y].featureCount++;
                }
                else
                {
                    pmf[x][y].features[minIndex].pixelValue = pixelValue;
                    pmf[x][y].features[minIndex].probability = 0.0f;
                    total -= minProbability;
                }
            }

            // update the histogram weights
            float newProbability;
            for (size_t i = 0; i < pmf[x][y].featureCount; ++i)
            {
                newProbability = pmf[x][y].features[i].pixelValue == pixelValue ? 1.0f : 0.0f;

                pmf[x][y].features[i].probability =
                    ((1.0f - _learningRate) * pmf[x][y].features[i].probability +
                     _learningRate * newProbability) /
                    total;
            }
        }
    }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
void GMGBackgroundSubtractor<T, X, Y, F_MAX>::update(T *src)
{
    updateQuantisedImage(src);
    if (_frameNum < _numInitialisationFrames)
    {
        train();
    }
    else
    {
        updatePosteriorImage();
        // smoothPosteriorImage();
        updateBinaryImage();
        smoothBinaryImage();
        updateHistogram();
    }
    _frameNum++;
    // if (_frameNum % _numInitialisationFrames == 0)
    // {
    //     printFeatures();
    // }
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
FGResult GMGBackgroundSubtractor<T, X, Y, F_MAX>::isFG(size_t idx)
{
    return FGResult{_binaryImage[idx % X][idx / Y], _posteriorImage[idx % X][idx / Y]};
}

template <typename T, size_t X, size_t Y, size_t F_MAX>
FGResult GMGBackgroundSubtractor<T, X, Y, F_MAX>::isFG(size_t x, size_t y)
{
    return FGResult{_binaryImage[x][y], _posteriorImage[x][y]};
}
