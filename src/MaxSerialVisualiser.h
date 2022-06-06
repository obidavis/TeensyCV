#include "GMGBackgroundSubtractor.h"
#include <SparkFun_GridEYE_Arduino_Library.h>

class MaxSerialVisualiser
{
public:
    MaxSerialVisualiser(
        GridEYE *grideye_ptr,
        GMGBackgroundSubtractor<float, 8, 8, 32> *bg_subtractor_ptr)
        : grideye(grideye_ptr),
          bg_subtractor(bg_subtractor_ptr)
    {
        // default values
        refresh_rate = 100;
    }
    void init(void) 
    { 
        tempoutbuffer[0] = 't';
        tempoutbuffer[1] = ' ';
        fgoutbuffer[0] = 'f';
        fgoutbuffer[1] = ' ';
    }
    static uint8_t quantise(float val, float min, float max, size_t levels)
    {
        return (uint8_t)(constrain(val, min, max) - min) / (max - min) * (levels - 1);
    }
    void update(void)
    {
        if (ms > refresh_rate)
        {
            float deviceTemp = grideye->getDeviceTemperature();
            ms = 0;
            for (int i = 0; i < 64; i++)
            {
                temp_data_buffer[i] = grideye->getPixelTemperature(i);
            }
            bg_subtractor->update(temp_data_buffer);

            for (int i = 0, buf_idx = 2; i < 64; i++, buf_idx+=2)
            {
                sprintf(tempoutbuffer + buf_idx, "%d ", quantise(temp_data_buffer[i], deviceTemp * 0.85, deviceTemp * 1.25, 10));
                sprintf(fgoutbuffer + buf_idx, "%d ", bg_subtractor->isFG(i).isFG);
            }
            fgoutbuffer[char_buf_len - 1] = '\n';
            tempoutbuffer[char_buf_len - 1] = '\n';
            Serial.write(fgoutbuffer, char_buf_len);
            Serial.write(tempoutbuffer, char_buf_len);
        }
    }
private:
    GridEYE *grideye;
    GMGBackgroundSubtractor<float, 8, 8, 32> *bg_subtractor;
    elapsedMillis ms;
    uint16_t refresh_rate;
    float temp_data_buffer[64];
    bool bg_data_buffer[64];
    char separator1 = ',';
    char separator2 = ';';
    const static unsigned int char_buf_len = 64 * 2 + 3;
    char tempoutbuffer[char_buf_len];
    char fgoutbuffer[char_buf_len];
    size_t buf_idx = 0;

};