#include "GMGBackgroundSubtractor.h"
#include <SparkFun_GridEYE_Arduino_Library.h>

class TerminalSerialVisualiser
{
public:
    TerminalSerialVisualiser(
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

    }
    void update(void)
    {
        if (ms > refresh_rate)
        {
            ms = 0;
            for (int i = 0; i < 64; i++)
            {
                temp_data_buffer[i] = grideye->getPixelTemperature(i);
            }
            bg_subtractor->update(temp_data_buffer);
            
            for(int x = 0; x < 8; x++)
            {
                for(int y = 0; y < 8; y++)
                {
                    Serial.print(bg_subtractor->isFG(x, y).isFG ? "0 " : ". ");
                }
                Serial.println();
            }
            Serial.println();
            Serial.println();
            Serial.println();
        }
    }
private:
    GridEYE *grideye;
    GMGBackgroundSubtractor<float, 8, 8, 32> *bg_subtractor;
    elapsedMillis ms;
    uint16_t refresh_rate;
    float temp_data_buffer[64];
};