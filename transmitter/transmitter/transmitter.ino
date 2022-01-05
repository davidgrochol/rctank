#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
#include <SSD1306AsciiSoftSpi.h>
#include <SSD1306AsciiSpi.h>
#include <SSD1306AsciiWire.h>
#include <SSD1306init.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// display
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
// Define proper RST_PIN if required.
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

// radio
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";


// data structure

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

//Inputs outputs
int buttonsAnalogIn = A6;           //Analog in from all the push buttons
int joyLeftXIn = A3; 
int joyLeftYIn = A2;
int joyRightXIn = A1;
int joyRightYIn = A0;
int switchLeftIn = 2;
int switchRightIn = 3;
int toggleLeft = A7;

int joyLeftX;
int joyLeftY;
int joyRightX;
int joyRightY;

// callibration
int joyLeftXTrim = 0; 
int joyLeftYTrim = 0;
int joyRightXTrim = 0;
int joyRightYTrim = 0;

bool jRXTI = false;
bool jRXTD = false;
bool jRYTI = false;
bool jRYTD = false;
bool jLXTI = false;
bool jLXTD = false;
bool jLYTI = false;
bool jLYTD = false;
bool modeChanging = false;

bool mode = true; // true - exponential / false - normal 


//------------------------------------------------------------------------------



void setup() {

  Serial.begin(9600);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);

  oled.setFont(Adafruit5x7);

  uint32_t m = micros();

  oled.clear();
  // second row
  oled.set1X();
  oled.println("");
  oled.set2X();
  oled.println("Loading....");
  oled.set1X();

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  pinMode(buttonsAnalogIn, INPUT);
  pinMode(switchLeftIn, INPUT);
  pinMode(switchRightIn, INPUT);


}

void resetData() 
{   
  data.joyLeftX = 127; // right joystick X
  data.joyLeftY = 127; // right joystick Y
  data.joyRightX = 127; // left joystick X
  data.joyRightY = 127; // left joystick Y
  data.toggleLeft = 0; // toggle left
  data.toggleRight = 0; // toggle right
  data.switchLeft = false; // switch right
  data.switchRight = false; // switch left 
}

int map_normal(int val, int lower, int middle, int upper, bool reverse = true)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

// Returns a corrected value for a joystick position that takes into account
// the values of the outer extents and the middle of the joystick range.
int map_exponential(int val, bool reverse = true)
{
  val = constrain(val, 0, 1023);
  float cube = ((pow((val - 512),3)/520200) + 258.012) / 2; 
  return ( reverse ? 255 - cube : cube );
}



void loop() 
{
  
  int buttonRead = analogRead(buttonsAnalogIn);
  
  oled.clear();
  //oled.print("buttons: ");
  //oled.println(buttonRead);

  if(buttonRead > 650)
  {
    jRXTI = false;
    jRXTD = false;
    jRYTI = false;
    jRYTD = false;
    jLXTI = false;
    jLXTD = false;
    jLYTI = false;
    jLYTD = false;
    modeChanging = false;
  }
  else if(buttonRead > 480 && buttonRead < 600 && !modeChanging) // mode button
  {
    modeChanging = true;
    mode = !mode;
  }
  else if(buttonRead > 450 && buttonRead < 470 && !jRXTI) // right X increase
  {
    jRXTI = true;
    joyRightXTrim++;
  }
  else if(buttonRead > 420  && buttonRead < 440 && !jRXTD) // right X decrease
  {
    jRXTD = true;
    joyRightXTrim--;
  }
  else if(buttonRead > 330  && buttonRead < 360 && !jRYTI ) // right Y increase
  {
    jRYTI = true;
    joyRightYTrim++;
  }
  else if(buttonRead > 380  && buttonRead < 400 && !jRYTD) // right Y decrease
  {
    jRYTD = true;
    joyRightYTrim--;
  }
  else if(buttonRead > 160 && buttonRead < 190 && !jLXTI) // left X increase
  {
    jLXTI = true;
    joyLeftXTrim++;
  }
  else if(buttonRead > 70  && buttonRead < 110 && !jLXTD) // left X decrease
  { 
    jLXTD = true;
    joyLeftXTrim--;
  }
  else if(buttonRead > 280 && buttonRead < 310 && !jLYTI) // left Y increase
  {
    jLYTI = true;
    joyLeftYTrim++;
  }
  else if(buttonRead > 220 && buttonRead < 260 && !jLYTD) // left Y decrease
  { 
    jLYTD = true;
    joyLeftYTrim--;
  }

  oled.print("Mode: ");
  if(mode)
    oled.println("exponential");
  else
    oled.println("normal");
  
      

  //Mode select  
  if(mode)
  {
    joyRightX = map_exponential(analogRead(joyRightXIn));
    joyRightY = map_exponential(analogRead(joyRightYIn));
    joyLeftX = map_exponential(analogRead(joyLeftXIn));
    joyLeftY = map_exponential(analogRead(joyLeftYIn)); 
  }
  else
  {
    joyRightX = map_normal(analogRead(joyRightXIn), 0, 512, 1023);
    joyRightY = map_normal(analogRead(joyRightYIn), 0, 512, 1023);
    joyLeftX = map_normal(analogRead(joyLeftXIn), 0, 512, 1023);
    joyLeftY = map_normal(analogRead(joyLeftYIn), 0, 512, 1023);  
  }


  
  joyRightX = joyRightX + joyRightXTrim;
  joyRightY = joyRightY + joyRightYTrim;
  joyLeftX = joyLeftX + joyLeftXTrim;
  joyLeftY = joyLeftY + joyLeftYTrim;

  oled.print("joyL X: ");
  oled.print(joyLeftX);
  oled.print(" Y: ");
  oled.println(joyLeftY);
  oled.print("joyR X: ");
  oled.print(joyRightX);
  oled.print(" Y: ");
  oled.println(joyRightY);

  data.joyRightX = constrain(joyRightX,0,255);
  data.joyRightY = constrain(joyRightY,0,255);
  data.joyLeftX = constrain(joyLeftX,0,255);
  data.joyLeftY = constrain(joyLeftY,0,255);
  data.toggleRight = map_normal(analogRead(toggleLeft), 0, 512, 1023);
  data.switchRight = !digitalRead(switchRightIn);
  data.switchLeft = !digitalRead(switchLeftIn);


  Serial.print("data.joyRightX: ") ;
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
  

  radio.write(&data, sizeof(SendData));


  delay(10);

}

















