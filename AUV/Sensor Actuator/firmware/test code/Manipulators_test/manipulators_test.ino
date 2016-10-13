#define MANI_1 30
#define MANI_2 31
#define MANI_3 32
#define MANI_4 33
#define MANI_5 34
#define MANI_6 41
#define MANI_7 40
#define MANI_8 37
#define MANI_9 36

void setup()
{

	//Initialize Manipulators
	pinMode(MANI_1, OUTPUT);
	pinMode(MANI_2, OUTPUT);
	pinMode(MANI_3, OUTPUT);
	pinMode(MANI_4, OUTPUT);
	pinMode(MANI_5, OUTPUT);
	pinMode(MANI_6, OUTPUT);
	pinMode(MANI_7, OUTPUT);
	pinMode(MANI_8, OUTPUT);
	pinMode(MANI_9, OUTPUT);

}

void loop()
{
  /* add main program code here */
	//digitalWrite(MANI_1, HIGH);
	//digitalWrite(MANI_2, HIGH);
	digitalWrite(MANI_3, HIGH);
	//digitalWrite(MANI_4, HIGH);
	digitalWrite(MANI_5, HIGH);
	//digitalWrite(MANI_6, HIGH);
	digitalWrite(MANI_7, HIGH);
	//digitalWrite(MANI_8, HIGH);
	digitalWrite(MANI_9, HIGH);
	
	delay(1000);
	digitalWrite(MANI_1, LOW);
	digitalWrite(MANI_2, LOW);
	digitalWrite(MANI_3, LOW);
	digitalWrite(MANI_4, LOW);
	digitalWrite(MANI_5, LOW);
	digitalWrite(MANI_6, LOW);
	digitalWrite(MANI_7, LOW);
	digitalWrite(MANI_8, LOW);
	digitalWrite(MANI_9, LOW);
	delay(1000);
}
