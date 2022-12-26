#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include <math.h>


RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";

struct SendData
{
  int joyLeftX; // right joystick X
  int joyLeftY; // right joystick Y
  int joyRightX; // left joystick X
  int joyRightY; // left joystick Y
  int toggleLeft; // toggle left
  int toggleRight; // toggle right
  bool switchLeft; // switch left
  bool switchRight; // switch rigth 
};
SendData data;

Servo servoRight;
Servo servoLeft;

void setup() {
  Serial.begin(9600);

  servoRight.attach(4,1000,2000);
  servoLeft.attach(3,1000,2000);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}


void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(SendData));  
    /*Serial.print("data.joyRightX: ") ;
    Serial.println(data.joyRightX);
    Serial.print("data.joyRightY: ") ;
    Serial.println(data.joyRightY);
    Serial.print("data.joyLeftX: ") ;
    Serial.println(data.joyLeftX);
    Serial.print("data.joyLeftY: ") ;
    Serial.println(data.joyLeftY);
    Serial.print("data.switchLeft: ") ;
    Serial.println(data.switchLeft);
    Serial.print("data.switchRight: ") ;
    Serial.println(data.switchRight);
    Serial.print("data.toggleLeft: ") ;
    Serial.println(data.toggleLeft);
    Serial.print("data.toggleRight: ") ;
    Serial.println(data.toggleRight);
    Serial.println();
*/
    Serial.println();
    
    int limitToggleUp = 180;
    int limitToggleDown = 0;
    if( data.switchLeft )
    {
      limitToggleUp = 140;
      limitToggleDown = 40;
    }
    int joyLeftY = map(data.joyLeftY, 0, 255, limitToggleDown, limitToggleUp);
    int joyRightX = 90-map(data.joyRightX, 0, 255, limitToggleDown, limitToggleUp);


    int leftMotor = constrain(joyLeftY + joyRightX,0,180);
    Serial.print("leftMotor: ");
    Serial.println(leftMotor);
    int rightMotor = constrain(joyLeftY - joyRightX,0,180);
    Serial.print("rightMotor: ");
    Serial.println(rightMotor);


    
    //servoRight.write( rightMotor );
    //servoLeft.write( leftMotor ); 

    servoRight.writeMicroseconds( map(rightMotor,0,180,1000,2000 ) );
    servoLeft.writeMicroseconds( map(leftMotor,0,180,1000,2000) ); 

    delay(10);
  }
}