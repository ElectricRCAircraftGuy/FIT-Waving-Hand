/*
FIT_waving_hand
 - a little code to control the device in the window of the lab to wave a little hand when people walk by

By Gabriel Staples 
www.ElectricRCAircraftGuy.com
- for email see my "Contact Me" tab at top of website 

Started: 21 July 2017
Last Updated: 25 Sept. 2017 

Circuit:

5V 2A power supply to power the Arduino 5V rail directly (since servos can draw a fair amount of current)

Servos x 2
 - servos x 2 to 5V and GND as required
 - Servo signals connected to pins as defined below

Sharp 2Y0A02 F 5Y IR distance sensor [UNUSED AT THIS TIME]
 - 5V and GND as required 
 - signal as specified below 
 
LDR (Light Dependent Resistor; AKA: photoresistor)
5V--4.4k--(sense_pin)--LDR--GND

FTDI TTL-232RG 0-5V Serial to USB UART adapter to Arduino Tx and Rx pins for serial debugging. 
 - Datasheet: http://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232RG_CABLES.pdf
 - Buy here for $25: https://www.digikey.com/product-detail/en/ftdi-future-technology-devices-international-ltd/TTL-232RG-VREG1V8-WE/768-1070-ND/2441359
Note: If you buy this stuff yourself, don't waste your money on a $25 FTDI USB serial converter UART! Instead, buy one on Ebay for <$2 with FREE shipping
 - Ex: Ebay search for "usb uart ttl 6pin cp2102"
   - ex: $1.47 w/FREE shipping - http://www.ebay.com/itm/CP2102-USB-2-0-to-TTL-UART-Module-6Pin-Serial-Converter-STC-Replace-FT232-Module-/141976891775
   - Better yet: just buy an Arduino Nano on Ebay, with built-in USB to serial UART, for ~$2.31 with FREE shipping, and avoid the whole external UART thing altogether!

*/

//libraries:
#include <Servo.h>
#include <eRCaGuy_NewAnalogRead.h>

//global vars & consts
const byte SERVO_UPDOWN_PIN = 5;
const byte SERVO_LEFTRIGHT_PIN = 6;
// const byte IR_DIST_SENSOR_PIN = A0;
const byte LDR_PIN = A1;

//analog readings
byte bitsOfResolution = 12; //commanded oversampled resolution
unsigned long numSamplesToAvg = 5; //number of samples AT THE OVERSAMPLED RESOLUTION that you want to take and average
ADC_speed_t ADCSpeed = ADC_FAST; 

//Servos
Servo upDownServo;
//lower vals go lower; higher vals go higher; ~110 is straight up now, since the straws are curved backwards. ~5 is sitting down on the pedestal 
#define SERVO_DOWN (5)
#define SERVO_UP (107)

Servo leftRightServo;
//lower vals to to the left, higher vals go to the right 
#define SERVO_LEFT (28)
#define SERVO_CENTER (80) //center position to sit on the pedestal 
#define SERVO_RIGHT (100)

const byte SERVO_SLEW_RATE = 5; //deg/sec 

//-------------------------------------------------------------------------------------------------
//writeServos
//-a command to write both servos at once, while slewing them too to force a reasonable response!
//-Note: "_des" stands for "desired"
//-------------------------------------------------------------------------------------------------
void writeServos(byte upDownServoPos_des, byte leftRightServoPos_des, byte servoSlewRate)
{
  byte upDownServoPos_slewed = upDownServo.read(); //deg 
  byte leftRightServoPos_slewed = leftRightServo.read(); //deg 
  //keep looping (in a *blocking* fashion) so long as at least one of the servos still has more distance to go 
  while (upDownServoPos_slewed!=upDownServoPos_des || leftRightServoPos_slewed!=leftRightServoPos_des)
  {
    //calculate slewed commands 
    upDownServoPos_slewed = slewServo();
    leftRightServoPos_slewed = slewServo();
    //write slewed commands to servos 
    upDownServo.write(upDownServoPos_slewed);
    leftRightServo.write(leftRightServoPos_slewed);
  }
}

//---------------------------------------------------------------------------------------
//slewServo
//-determine an actual, slewed servo command, based on a desired servoCommand 
//-return the slewed command 
//---------------------------------------------------------------------------------------
byte slewServo(byte servoCommand, byte servoPosition)
{
  static byte slewedCommand = SERVO_START_POS; //deg
  static byte lastSlewedCommand = slewedCommand; //deg 
  
  static unsigned long tStart = millis(); //ms 
  if (startOver)
    tStart = millis(); //ms 
  unsigned long tNow = millis(); //ms 
  unsigned long dt = tNow - tStart; //ms
  unsigned int permittedDegMovement = dt*SERVO_SLEW_RATE/1000; //deg movement permitted since last movement
  if (permittedDegMovement > 0)
  {
    // Serial.print(F("permittedDegMove = ")); Serial.println(permittedDegMovement); //FOR DEBUGGING 
    tStart = tNow; //ms; update 
    
    //determine new, slewed servo command 
    if ((int)servoCommand - (int)lastSlewedCommand >= (int)permittedDegMovement)
    {
      // Serial.println(F("going up")); //FOR DEBUGGING
      slewedCommand = lastSlewedCommand + permittedDegMovement; //deg
    }
    else if ((int)servoCommand - (int)lastSlewedCommand <= -(int)permittedDegMovement)
    {
      Serial.println(F("going down")); //FOR DEBUGGING
      slewedCommand = lastSlewedCommand - permittedDegMovement; //deg
    }
    else //servoCommand - lastSlewedCommand > -permittedDegMovement *and* it is < permittedDegMovement
    {
      // Serial.println(F("setting slewedCommand to servoCommand")); //FOR DEBUGGING
      slewedCommand = servoCommand; //deg
    }
    // Serial.println(slewedCommand); //deg; FOR DEBUGGING 
    lastSlewedCommand = slewedCommand; //deg; update 
  }
  
  return slewedCommand; //deg
}

//-------------------------------------------------------------------------------------------------
//setup 
//-------------------------------------------------------------------------------------------------
void setup()
{
  // pinMode(IR_DIST_SENSOR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  Serial.begin(115200);
  delay(100); //give time for serial debugger to turn on and stabilize 
  Serial.println(F("\nBegin"));
  
  //Configure the adc how you want it
  adc.setADCSpeed(ADCSpeed);
  adc.setBitsOfResolution(bitsOfResolution);
  adc.setNumSamplesToAvg(numSamplesToAvg);
  
  //Servos
  upDownServo.attach(SERVO_UPDOWN_PIN);
  leftRightServo.attach(SERVO_LEFTRIGHT_PIN);
  
  upDownServo.write(SERVO_UP); 
  leftRightServo.write(SERVO_LEFT); 
  delay(1000);
  leftRightServo.write(SERVO_RIGHT);
  delay(1000);
  leftRightServo.write(SERVO_CENTER);
  delay(1000);
  upDownServo.write(SERVO_DOWN);
}

//-------------------------------------------------------------------------------------------------
//loop
//-------------------------------------------------------------------------------------------------
void loop()
{
  static bool personArrived = false;
  sampleLightSensor(&personArrived);
  
  if (personArrived)
  {
    waveHello();
  }
}

//-------------------------------------------------------------------------------------------------
//waveHello 
//-blocking code is OK
//-------------------------------------------------------------------------------------------------
void waveHello()
{
  
}

//-------------------------------------------------------------------------------------------------
//sampleLightSensor
//-sample the light sensor at the desired rate, and update the triggerNow variable if the light is diminished 
//-use basic DSP to get good light readings 
//-------------------------------------------------------------------------------------------------
void sampleLightSensor(bool *triggerNow_p)
{
  //sample the light sensor at desired freq.
  static unsigned long t_start = micros(); //us 
  //set desired sample time pd, dt_des
  //-NB: Generally speaking, use *LARGER* values for *MORE SENSITIVITY* AND *LESS NOISE* (longer sample period gives the light sensor time to fully change to a new light condition, thereby giving a larger delta between readings), and use smaller values for a faster sample rate 
  const unsigned long dt_des = 40000; //us; 10000us --> 100 Hz; 20000 --> 50 Hz, 40000 --> 25 Hz, 50000 --> 20 Hz, etc 
  unsigned long t_now = micros(); //us 
  if (t_now - t_start >= dt_des)
  {
    t_start = t_now; //us; update 
    
    //local constants and variables 
    static unsigned int lightReading = adc.newAnalogRead(LDR_PIN);
    // Serial.println(lightReading); //FOR DEBUGGING 
    static unsigned int lightReading_old = lightReading;
    const int LIGHT_READING_DELTA_TRIGGER = 25; //<--GS TUNE THIS VALUE 
    
    //take new reading & do some smoothing
    //-see here for an excellent smoothing example: https://www.arduino.cc/en/Tutorial/Smoothing
    //-TODO-FINISH YOUR SMOOTHING LIBRARY SO YOU CAN JUST START USING IT INSTEAD IN THE FUTURE! ~GS
    const byte NUM_READINGS = 5;
    static unsigned int readings[NUM_READINGS];
    static unsigned int readIndex = 0;
    static unsigned long total = 0;
    //gracefully fill the readings array the first time we run 
    static bool firstItn = true;
    if (firstItn)
    {
      firstItn = false; //update 
      for (byte i=0; i<NUM_READINGS; i++)
      {
        readings[NUM_READINGS] = lightReading;
        total += lightReading;
      }
    }
    //subtract the last reading:
    total = total - readings[readIndex];
    //read from the sensor:
    readings[readIndex] = adc.newAnalogRead(LDR_PIN); 
    //add the reading to the total:
    total = total + readings[readIndex];
    //advance to the next position in the array:
    readIndex++;
    //if we're at the end of the array...
    if (readIndex >= NUM_READINGS) 
    {
      //...wrap around to the beginning:
      readIndex = 0;
    }
    //calculate the average:
    lightReading = total/NUM_READINGS;
    // Serial.println(lightReading); //FOR DEBUGGING 
    
    //test to see if the light intensity *decreased* or *increased* 
    int lightReadingChange = lightReading - lightReading_old; 
    lightReading_old = lightReading; //update 
    if (lightReadingChange >= LIGHT_READING_DELTA_TRIGGER)
    {
      //a shadow was just cast onto the sensor 
      // Serial.println(F("shadow")); //FOR DEBUGGING
      *triggerNow_p = true; 
    }
    else if (lightReadingChange <= -LIGHT_READING_DELTA_TRIGGER) 
    {
      //light just returned to the sensor 
      // Serial.println(F("light")); //FOR DEBUGGING
      *triggerNow_p = false;
    }
    
    //print debugging data to human
    // Serial.print(F("lightReading = ")); Serial.print(lightReading); 
    // Serial.print(F(", lightReadingChange = ")); Serial.println(lightReadingChange);
    
    //print debugging data for serial plotter
    #define SCALING_FACTOR 100
    Serial.print(LIGHT_READING_DELTA_TRIGGER*SCALING_FACTOR); Serial.print(", "); //plot lower trigger threshold
    Serial.print(-LIGHT_READING_DELTA_TRIGGER*SCALING_FACTOR); Serial.print(", "); //plot upper trigger threshold
    Serial.print(lightReadingChange*SCALING_FACTOR); Serial.print(", "); //plot derivative value 
    Serial.println(lightReading); //plot raw light reading 
    //OR
    // Serial.print(LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot lower trigger threshold
    // Serial.print(-LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot upper trigger threshold
    // Serial.println(lightReadingChange); //plot derivative value 
    
  } //end of sample at desired freq.
}



