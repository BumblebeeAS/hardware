#include <asv2telemetry.h>

void asv2telemetry::init() {
	Wire.begin();
	Serial.begin(9600);
	Serial2.begin(9600);
}

void asv2telemetry::writeDAC(uint16 voltage) {
	
	if (voltage > 4095) { voltage = 4095; }

	Wire.beginTransmission(DAC_addr);
	Wire.write(voltage >> 8);     // top 4 bit of the 12bit voltage
	Wire.write(voltage >> 4);     // bot 8 bit of the 12bit voltage
	Wire.endTransmission(true);
}

void asv2telemetry::readCPPM(int arr[], ) {

}