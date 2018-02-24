#ifndef _WIND_SENSOR_H_
#define _WIND_SENSOR_H_

#define PACKET_START 2
#define PACKET_END 3

class WindSensor {
private:
    uint16_t _windDirection;
	uint16_t _windSpeed;
    boolean _startReading;
    String _stringValues;

    void writeValues();

public:
    WindSensor();

	void init();

	uint16_t getDirection();
	uint16_t getWindSpeed();
    void readValues();
};

#endif
