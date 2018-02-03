#ifndef _WIND_SENSOR_H_
#define _WIND_SENSOR_H_

class WindSensor {
private:
    double _windDirection;
    double _windSpeed;
    boolean _startReading;
    String _stringValues;

    void writeValues();

public:
    WindSensor();

    double getDirection();
    double getWindSpeed();
    void readValues();
};

#endif
