#pragma once

#include <Arduino.h>
#include "ObjectDetection/BackgroundSubtractor.h"

template <typename T, int SIZE, int MAX_FEATURES>
class BackgroundSubtractorGMG : public BackgroundSubtractor<T>
{
public:
    BackgroundSubtractorGMG()
    {
        frameNum_ = 0;
        learningRate = 0.025;
        numInitializationFrames = 120;
        quantizationLevels = 16;
        backgroundPrior = 0.8;
        decisionThreshold = 0.9;
        smoothingRadius = 7;
        updateBackgroundModel = true;
        minVal_ = maxVal_ = 0;
    }

    ~BackgroundSubtractorGMG()
    {
    }
    /**
     * Performs single-frame background subtraction and builds up a statistical background image
     * model.
     * @param image Input image
     * @param fgmask Output mask image representing foreground and background pixels
     */
    
    virtual void init(T minVal, T maxVal);

    virtual FGResult isFG(uint16_t i, T val) override;
 
    virtual double getDefaultLearningRate() const  { return learningRate; }
    virtual void setDefaultLearningRate(double lr)  { learningRate = lr; }

    virtual int getNumFrames() const  { return numInitializationFrames; }
    virtual void setNumFrames(int nframes)  { numInitializationFrames = nframes; }

    virtual uint8_t getQuantizationLevels() const  { return quantizationLevels; }
    virtual void setQuantizationLevels(uint8_t nlevels)  { quantizationLevels = nlevels; }

    virtual double getBackgroundPrior() const  { return backgroundPrior; }
    virtual void setBackgroundPrior(double bgprior)  { backgroundPrior = bgprior; }

    virtual int getSmoothingRadius() const  { return smoothingRadius; }
    virtual void setSmoothingRadius(int radius)  { smoothingRadius = radius; }

    virtual double getDecisionThreshold() const  { return decisionThreshold; }
    virtual void setDecisionThreshold(double thresh)  { decisionThreshold = thresh; }

    virtual bool getUpdateBackgroundModel() const  { return updateBackgroundModel; }
    virtual void setUpdateBackgroundModel(bool update)  { updateBackgroundModel = update; }

    virtual T getMinVal() const  { return minVal_; }
    virtual void setMinVal(T val)  { minVal_ = val; }

    virtual T getMaxVal() const  { return maxVal_; }
    virtual void setMaxVal(T val)  { maxVal_ = val; }

    //! Total number of distinct colors to maintain in histogram.
    //! Set between 0.0 and 1.0, determines how quickly features are "forgotten" from histograms.
    double  learningRate;
    //! Number of frames of video to use to initialize histograms.
    int     numInitializationFrames;
    //! Number of discrete levels in each channel to be used in histograms.
    uint8_t     quantizationLevels;
    //! Prior probability that any given pixel is a background pixel. A sensitivity parameter.
    double  backgroundPrior;
    //! Value above which pixel is determined to be FG.
    double  decisionThreshold;
    //! Smoothing radius, in pixels, for cleaning up FG image.
    int     smoothingRadius;
    //! Perform background model update
    bool updateBackgroundModel;

private:
    T maxVal_;
    T minVal_;

    int frameNum_;

    float weights_[SIZE][MAX_FEATURES];
    uint8_t colors_[SIZE][MAX_FEATURES];
    int nfeatures_[SIZE];
};

static float findFeature(uint8_t color, const uint8_t* colors, const float* weights, int nfeatures)
{
    for (int i = 0; i < nfeatures; ++i)
    {
        if (color == colors[i])

            return weights[i];
    }

    // not in histogram, so return 0.
    return 0.0f;
}

static void normalizeHistogram(float* weights, int nfeatures)
{
    float total = 0.0f;
    for (int i = 0; i < nfeatures; ++i)
        total += weights[i];

    if (total != 0.0f)
    {
        for (int i = 0; i < nfeatures; ++i)
            weights[i] /= total;
    }
}

static bool insertFeature(uint8_t color, float weight, uint8_t* colors, float* weights, int& nfeatures, int maxFeatures)
{
    int idx = -1;
    for (int i = 0; i < nfeatures; ++i)
    {
        if (color == colors[i])
        {
            // feature in histogram
            weight += weights[i];
            idx = i;
            break;
        }
    }

    if (idx >= 0)
    {
        // move feature to beginning of list

        memmove(colors + 1, colors, idx * sizeof(uint8_t));
        memmove(weights + 1, weights, idx * sizeof(float));

        colors[0] = color;
        weights[0] = weight;
    }
    else if (nfeatures == maxFeatures)
    {
        // discard oldest feature

        memmove(colors + 1, colors, (nfeatures - 1) * sizeof(uint8_t));
        memmove(weights + 1, weights, (nfeatures - 1) * sizeof(float));

        colors[0] = color;
        weights[0] = weight;
    }
    else
    {
        colors[nfeatures] = color;
        weights[nfeatures] = weight;

        ++nfeatures;

        return true;
    }

    return false;
}

template <typename T, int SIZE, int MAX_FEATURES>
void BackgroundSubtractorGMG<T, SIZE, MAX_FEATURES>::init(T minVal, T maxVal)
{
    minVal_ = minVal;
    maxVal_ = maxVal;

    for (int i = 0; i < SIZE; ++i)
    {
        nfeatures_[i] = 0;
    }
}


template <typename T>
static uint8_t quantize(T val, T minVal, T maxVal, int levels)
{
// return a value between 0 and 0xff that represents the quantized value of val
// between minVal and maxVal.  The number of quantization levels is specified
// by levels.  The minVal and maxVal can have any range
    val = constrain(val, minVal, maxVal);
    float normalisedVal = (val - minVal) / (float)(maxVal - minVal);
    return (uint8_t)(normalisedVal * (levels - 1));
}

template <typename T , int SIZE, int MAX_FEATURES>
FGResult BackgroundSubtractorGMG<T, SIZE, MAX_FEATURES>::isFG(uint16_t i, T val)
{
    uint8_t color = quantize<T>(val, minVal_, maxVal_, quantizationLevels);
    float confidence = 0.0f;
    bool isFG = false;
    int nfeatures = nfeatures_[i];
    uint8_t* colors = colors_[i];
    float* weights = weights_[i];

    if (frameNum_ >= numInitializationFrames)
    {
        const float weight = findFeature(color, colors, weights, nfeatures);

        const float posterior = (weight * backgroundPrior) / (weight * backgroundPrior + (1.0f - weight) * (1.0f - backgroundPrior));

        confidence = 1.0f - posterior;
        isFG = (1.0f - posterior) > decisionThreshold;
        
        if (!isFG)
        {
            for (int i = 0; i < nfeatures; ++i)
            {
                weights[i] *= (float)(1.0f - learningRate);
            }

            if (insertFeature(color, learningRate, colors, weights, nfeatures, MAX_FEATURES))
            {
                normalizeHistogram(weights, nfeatures);
                nfeatures_[i] = nfeatures;
            }
        }

    }
    else
    {
        insertFeature(color, 1.0f, colors, weights, nfeatures, MAX_FEATURES);
        if (frameNum_ == numInitializationFrames - 1)
        {
            normalizeHistogram(weights, nfeatures);
        }
    }

    if (i == 0)
    {
        frameNum_++;
    }

    return FGResult{isFG, confidence};
}

void apply()
{
    
}