#define SONAR 40
#define NAV 37
#define ACOU 41

void setup() {
  // put your setup code here, to run once:
  DDRD = DDRD | B01000000; //set PD6 as output 
  pinMode(38, OUTPUT); //sa en
  pinMode(37, OUTPUT); //nav en
  pinMode(40, OUTPUT); //sonar en
  pinMode(41, OUTPUT); //aco en
  Serial.begin(9600);
  digitalWrite(37, HIGH); //disable POE
  digitalWrite(40, HIGH); //same
  digitalWrite(41, HIGH); //same
  Serial.println("Start");
  //char a;
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    char a = Serial.read();
    switch(a){
      case '1':
      digitalWrite(NAV, HIGH);
      break;

      case '2':
      digitalWrite(NAV, LOW);
      break;

      case '3':
//      PORTD |= B01000000; //set PD6 high
      digitalWrite(SONAR, HIGH);
      break;

      case '4':
//      PORTD &= ~(B01000000); //clear PD6
      digitalWrite(SONAR, LOW);
      break;

      case '5':
      digitalWrite(ACOU, HIGH);
      break;

      case '6':
      digitalWrite(ACOU, LOW);
      break;
    }
    /*
    if(Serial.read() == '1'){
      digitalWrite(38, HIGH);
    }else if(Serial.read() == '2'){
      digitalWrite(38, LOW);
    }else if(Serial.read() == '3'){
      PORTD = PORTD | B01000000; //set PD6 high
    }else if(Serial.read() == '4'){
      PORTD = PORTD & ~(B01000000); //clear PD6
    }else if(Serial.read() == '5'){
      digitalWrite(37, HIGH);
    }else if(Serial.read() == '6'){
      digitalWrite(37, LOW);
    }
    */
  }



  ============================================================

  
}
