#include <RGBmatrixPanel.h>
#define CLK 11 // USE THIS ON ARDUINO MEGA
#define OE   9
#define LAT A3
#define A   A0
#define B   A1
#define C   A2

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);
int color[3] = {-1,-1,-1};
String colorName[3] = {"Red","Green","Blue"};
int colorVal[3] = {matrix.Color333(7, 0, 0),matrix.Color333(0, 7, 0),matrix.Color333(0, 0, 7)};

// Coded based on page 17 of the link below
//https://robonation.org/app/uploads/sites/2/2022/03/2022-RobotX_Team-Handbook_v2.pdf

void setup() {
  matrix.begin();
  Serial.begin(115200);
  randomSeed(analogRead(A7));

  Serial.print("Color: ");

  // generate first color
  int tempColor = random(0,3);
  Serial.print(tempColor);
  color[0] = tempColor;
  int prevColor = tempColor;
  
  Serial.print(colorName[color[0]]);
  Serial.print(", ");

  // generate next 2 color
  for (int i = 1; i < 3; i++){
    while (tempColor == prevColor){
      tempColor = random(0,3);
    }
    color[i] = tempColor;
    prevColor = tempColor;
    Serial.print(colorName[color[i]]);
    Serial.print(", ");
  }
  Serial.println(); 

}

void loop() {
  // print each color for one sec
  for (int i = 0; i < 3; i++){
    matrix.fillRect(0, 0, 32, 16, colorVal[color[i]]);
    Serial.println(colorName[color[i]]);
    delay(1000);
      }

  // display black for 2 sec
  matrix.fillRect(0, 0, 32, 16, matrix.Color333(0, 0, 0));
  delay(2000);
}
