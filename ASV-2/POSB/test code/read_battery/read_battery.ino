#include "Arduino.h"
#include "Torqeedo.h"

#define DATA_UPDATE_INTERVAL 25

uint32_t BattUpdateTime;

Torqeedo Battery1(TORQEEDO1_RXEN, TORQEEDO1_DXEN, TORQEEDO1_ON, 1);
//Torqeedo Battery1(TORQEEDO2_RXEN, TORQEEDO2_DXEN, TORQEEDO2_ON, 2);
int averageCurr[10] = {0};
int32_t totalCurr = 0;
int currIdx = 0;

void setup()
{
	Serial.begin(115200);

	Battery1.init();
	Serial.flush();
	BattUpdateTime = millis();
}

void loop()
{
	Battery1.checkBatteryConnected();
	Battery1.checkBatteryOnOff();
	Battery1.readMessage();

	if (Serial.available()){
		char temp = Serial.read();
		switch (temp){
		case '0':
			Battery1.onBattery(false);
			break;

		case '1':
			Battery1.onBattery(true);
			break;

		default:
			//error
			Serial.println("Serial input error");
			break;
		}
	}

	//
	// REQUEST FOR DATA UPDATE AT FIXED TIME INTERVAL only if battery not toggling
	//
	if (!Battery1.powerSeq) {
		if (millis() < BattUpdateTime)
		{
			BattUpdateTime = millis();
		}
		if ( millis() - BattUpdateTime > DATA_UPDATE_INTERVAL)
		{
			Battery1.requestUpdate();
			BattUpdateTime = millis();
			/*Serial.print("V: ");
			Serial.print(Battery1.battData.voltage);
			Serial.print(" C1: ");
			Serial.print(Battery1.battData.voltCell1);
			Serial.print(" C2: ");
			Serial.print(Battery1.battData.voltCell2);
			Serial.print(" C3: ");
			Serial.print(Battery1.battData.voltCell3);
			Serial.print(" C4: ");
			Serial.print(Battery1.battData.voltCell4);
			Serial.print(" C5: ");
			Serial.print(Battery1.battData.voltCell5);
			Serial.print(" C6: ");
			Serial.print(Battery1.battData.voltCell6);
			Serial.print(" C7: ");
			Serial.println(Battery1.battData.voltCell7);*/
			Serial.print("V: ");
			Serial.print(Battery1.getVoltage()/10.0);
			Serial.print(", C: ");
			Serial.print(Battery1.getCapacity());
			Serial.print("%, T: ");
			Serial.print(Battery1.getTemperature()/10.0);
			Serial.print("dnoC, A: ");
			int16_t temp = Battery1.getCurrent();
			Serial.print(temp/10.0);
			totalCurr -= averageCurr[currIdx];
			averageCurr[currIdx] = temp;
			totalCurr += averageCurr[currIdx];
			currIdx++;
			currIdx %= 10;
			Serial.print(", avg A: ");
			Serial.println((totalCurr/100.0));
		}
	}
}
