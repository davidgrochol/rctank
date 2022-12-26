#include <SPI.h>
#include <Servo.h>
#include <math.h>


Servo servoTest;
int pot = A0; 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(pot, INPUT);
  servoTest.attach(3,1000,2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(pot);

  int angle = map(val, 0, 1023, 1000,2000);
  servoTest.writeMicroseconds(angle);
}
