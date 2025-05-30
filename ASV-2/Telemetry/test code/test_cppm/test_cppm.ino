volatile int ppm[6] ={0};

const int cppm = 19; // pin in for cppm

volatile unsigned long prev = 1000000;  // to always discard first frame
volatile unsigned long delta = 0;
volatile bool sync = false;
volatile int ch = 0;

void setup() {
  Serial.begin(115200);
  pinMode(cppm,INPUT);
  attachInterrupt(digitalPinToInterrupt(cppm), readppm, RISING); 
}

void printppm() {
  for (int i=1; i<=6; i++) {
    Serial.print("CH ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(ppm[i-1]);
    Serial.print("  ");
  }
  Serial.println("");
//  delay(500);
}
long long time = 0;
void loop() {
  if (millis() - time > 500) {
    checkppm();
    printppm(); 
    time = millis();
  }
//  if (ppm[0] > 1200) {
//    Serial.println("**************DED**************");
//    //Serial.println(micros());
//  }
}

void checkppm() {
  for (int i = 0; i < 6; i++) {
    if (ppm[i] > 2500) {
      for (int j = 0; j < 6; j++) {
        ppm[j] = 1500;
      }
      break;
    }
  }
}

void readppm() {
  if ( micros() - prev < 0 ) {  //if overflow, discard this 18ms frame
    prev = micros();
    sync = false;
    return;
  }

  delta = micros() - prev; 
  prev = micros();

  if (sync) {
    switch (ch) {
      case 0:
        ppm[0] = delta;
        ch++;
        break;
      case 1:
        ppm[1] = delta;
        ch++;
        break;
      case 2:
        ppm[2] = delta;
        ch++;
        break;
      case 3:
        ppm[3] = delta;
        ch++;
        break;
      case 4:
        ppm[4] = delta;
        ch++;
        break;
      case 5:
        ppm[5] = delta;
        ch = 0;
        sync = false;
        break;
      default:
        sync = false;
        break;
    }
  } else {
    if (delta > 3000) {
      sync = true;
    } else {
      sync = false;
    }
  }
}
