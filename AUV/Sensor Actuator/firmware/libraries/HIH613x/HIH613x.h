#ifndef HIH613x_h
#define HIH613x_h

#include <Arduino.h>
#include <Wire.h>

class HIH613x
{
public:
    // init/delete
    HIH613x(byte address);
    ~HIH613x();

    // simple api
    byte update();
    float getHumidity() const { return humidity; }
    float getTemperature() const { return temperature; }

    // advanced api
    void measurementRequest() const;
    byte dataFetch();

private:
    // core
    byte address;
    float temperature;
    float humidity;
};

#endif
