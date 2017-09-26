/*
FIT_waving_hand
 - a little code to control the device in the window of the lab to wave a little hand when people walk by
 - Use the serial plotter while running the code to see a nice plot of the light output!
 - Tune the "LIGHT_READING_DELTA_TRIGGER" value in the "sampleLightSensor()" function for best results 

By Gabriel Staples 
www.ElectricRCAircraftGuy.com
- for email see my "Contact Me" tab at top of website 

Started: 21 July 2017
Last Updated: 25 Sept. 2017 

Circuit:

Arduino Pro Mini (though a Nano is preferred since it has a built in USB to serial adapter) 
NB: DESPITE THE FACT THAT I'M CURRENTLY USING A PRO MINI, I'VE UPLOADED THE UNO BOOTLOADER ONTO THE BOARD, SO MAKE SURE YOU SET THE BOARD AS THE ARDUINO UNO IN ORDER TO UPLOAD CODE THROUGH ITS BOOTLOADER. 

(For the Pro Mini):
TTL 0-5V Serial to USB UART adapter to Arduino Tx and Rx pins for code uploading & serial debugging. 
 - Buy one on Ebay for <$2 with FREE shipping
 - Ex: Ebay search for "usb uart ttl 6pin cp2102"
   - ex: $1.47 w/FREE shipping - http://www.ebay.com/itm/CP2102-USB-2-0-to-TTL-UART-Module-6Pin-Serial-Converter-STC-Replace-FT232-Module-/141976891775
   - Better yet: just buy an Arduino Nano on Ebay, with built-in USB to serial UART, for ~$2.31 with FREE shipping, and avoid the whole external UART thing altogether!

5V 2A power supply to power the Arduino 5V rail directly (since servos can draw a fair amount of current)

9g Servos x 2 (from HobbyKing, Ebay, or other suppliers)
 - servos x 2 to 5V and GND as required
 - Servo signals connected to pins as defined below

Sharp 2Y0A02 F 5Y IR distance sensor [UNUSED AT THIS TIME]
 - 5V and GND as required 
 - signal as specified below 
 
LDR (Light Dependent Resistor; AKA: photoresistor) [replaces the Sharp IR distance sensor]
5V--4.4k--(sense_pin)--LDR--GND

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
//TUNE THESE PARAMETERS TO PRODUCE THE BEST LIGHT SENSOR SENSITIVITY
byte bitsOfResolution = 13; //commanded oversampled resolution
unsigned long numSamplesToAvg = 5; //number of samples AT THE OVERSAMPLED RESOLUTION that you want to take and average
ADC_speed_t ADCSpeed = ADC_FAST; 
//For use inside the "sampleLightSensor()" function
const int LIGHT_READING_DELTA_TRIGGER = 5; //person detection trigger setting <--GS TUNE THIS VALUE 
const byte NUM_READINGS = 10; //for data smoothing 

//Servos
const byte NUMSERVOS = 2;
//Servo indices 
const byte UPDOWNSERVO = 0;
const byte LEFTRIGHTSERVO = 1;
#define STARTOVER (true)

Servo upDownServo;
//lower vals go lower; higher vals go higher; ~110 is straight up now, since the straws are curved backwards. ~5 is sitting down on the pedestal 
#define SERVO_DOWN (5)
#define SERVO_UP (107)

Servo leftRightServo;
//lower vals to to the left, higher vals go to the right 
#define SERVO_LEFT (28)
#define SERVO_CENTER (80) //center position to sit on the pedestal 
#define SERVO_RIGHT (100)

const unsigned int SERVO_SLEW_RATE = 200; //deg/sec 

//---------------------------------------------------------------------------------------
//slewServo
//-determine an actual, slewed servo command, based on a desired servoCommand (newCommand)
//-write the slewedCommand to the servo, and return the slewed command that was written 
//-set startOver to true at the start of each new servo movement 
//---------------------------------------------------------------------------------------
byte slewServo(Servo& servo, const byte SERVO_INDEX, byte newCommand, unsigned int servoSlewRate, bool startOver = false)
{
  byte oldCommand = servo.read(); //deg 
  byte slewedCommand = oldCommand; //deg
  
  unsigned long tNow = millis(); //ms 
  static unsigned long tStart[NUMSERVOS]; //ms 
  if (startOver)
    tStart[SERVO_INDEX] = tNow; //ms 
  unsigned long dt = tNow - tStart[SERVO_INDEX]; //ms
  unsigned int permittedDegMovement = dt*servoSlewRate/1000; //deg movement permitted since last movement
  if (permittedDegMovement > 0)
  {
    // Serial.print(F("permittedDegMovement = ")); Serial.println(permittedDegMovement); //FOR DEBUGGING 
    tStart[SERVO_INDEX] = tNow; //ms; update 
    
    //determine new, slewed servo command 
    if ((int)newCommand - (int)oldCommand >= (int)permittedDegMovement)
    {
      // Serial.println(F("going up")); //FOR DEBUGGING
      slewedCommand = oldCommand + permittedDegMovement; //deg
    }
    else if ((int)newCommand - (int)oldCommand <= -(int)permittedDegMovement)
    {
      // Serial.println(F("going down")); //FOR DEBUGGING
      slewedCommand = oldCommand - permittedDegMovement; //deg
    }
    else //newCommand - oldCommand > -permittedDegMovement *and* it is < permittedDegMovement
    {
      // Serial.println(F("setting slewedCommand to newCommand")); //FOR DEBUGGING
      slewedCommand = newCommand; //deg
    }
  }
  servo.write(slewedCommand); //deg 
  
  return slewedCommand; //deg
}

//-------------------------------------------------------------------------------------------------
//writeServos
//-a command to write both servos at once, while slewing them too to force a reasonable response!
//-blocking (via the while loop) is OK for now; you can always change this in the future if needed
//-------------------------------------------------------------------------------------------------
void writeServos(byte upDownServoCommand, byte leftRightServoCommand, unsigned int servoSlewRate = SERVO_SLEW_RATE)
{
  slewServo(upDownServo, UPDOWNSERVO, upDownServoCommand, servoSlewRate, STARTOVER);
  slewServo(leftRightServo, LEFTRIGHTSERVO, leftRightServoCommand, servoSlewRate, STARTOVER);
  
  //keep looping (in a *blocking* fashion for now) so long as at least one of the servos still has more distance to go 
  while (upDownServo.read()!=upDownServoCommand || leftRightServo.read()!=leftRightServoCommand)
  {
    slewServo(upDownServo, UPDOWNSERVO, upDownServoCommand, servoSlewRate);
    slewServo(leftRightServo, LEFTRIGHTSERVO, leftRightServoCommand, servoSlewRate);
  }
}

//-------------------------------------------------------------------------------------------------
//waveHand
//-NB: beware: this is currently a BLOCKING method!
//-------------------------------------------------------------------------------------------------
void waveHand()
{
  //Old method:
  // upDownServo.write(SERVO_UP); 
  // leftRightServo.write(SERVO_LEFT); 
  // delay(1000);
  // leftRightServo.write(SERVO_RIGHT);
  // delay(1000);
  // leftRightServo.write(SERVO_CENTER);
  // delay(1000);
  // upDownServo.write(SERVO_DOWN);
  
  //New method:
  writeServos(SERVO_UP, SERVO_CENTER);
  //wave a few times 
  for (byte i=0; i<5; i++)
  {
    writeServos(SERVO_UP, SERVO_LEFT);
    writeServos(SERVO_UP, SERVO_RIGHT);
  }
  writeServos(SERVO_UP, SERVO_CENTER);
  writeServos(SERVO_DOWN, SERVO_CENTER);
  
  // delay(100); //settling delay to let servos totally stop moving, so they don't produce noise on the Vcc line which affects the light sensor readings 
  initializeLightSensor(700); //NB: if the hand keeps waving again every time it finishes waving, for no visible reason whatsoever, increase this time period; test this by quickly blocking the light sensor by holding your hand in front of it. The hand should not keep repeatedly waving each time it finishes a wave. If it does, increase this time period to let the filter take effect, accumulating new data without continually waving. Quickly blocking the light and holding your hand there forever should induce one single wave function cycle, not repeated calls to the waveHand() function.
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
  //set initial position
  upDownServo.write(SERVO_DOWN);
  leftRightServo.write(SERVO_CENTER);
  
  //Initialize light sensor 
  //-take samples for a while to first let the light filters and moving averages and all kick in and take effect; otherwise, it will be trying to trigger from the get-go 
  initializeLightSensor(1000); //NB: if the hand keeps waving again every time it finishes waving, for no visible reason whatsoever, increase this time period 
  
  waveHand(); //indicate initialization is over!
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
    //count and print how often this happens 
    static unsigned long count = 0;
    count++;
    Serial.print(F("Person Arrived; count = ")); Serial.println(count);
    personArrived = false; //reset 
    waveHand();
  }
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
    // const int LIGHT_READING_DELTA_TRIGGER = 5; //<--GS TUNE THIS VALUE 
    
    //take new reading & do some smoothing
    //-see here for an excellent smoothing example: https://www.arduino.cc/en/Tutorial/Smoothing
    //-TODO-FINISH YOUR SMOOTHING LIBRARY SO YOU CAN JUST START USING IT INSTEAD IN THE FUTURE! ~GS
    // const byte NUM_READINGS = 5;
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
    #define SCALING_FACTOR 50L
    Serial.print((long)LIGHT_READING_DELTA_TRIGGER*SCALING_FACTOR); Serial.print(", "); //plot lower trigger threshold
    Serial.print(-(long)LIGHT_READING_DELTA_TRIGGER*SCALING_FACTOR); Serial.print(", "); //plot upper trigger threshold
    Serial.print((long)lightReadingChange*SCALING_FACTOR); Serial.print(", "); //plot derivative value 
    Serial.println((long)lightReading/10); //plot raw light reading 
    //OR
    // Serial.print(LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot lower trigger threshold
    // Serial.print(-LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot upper trigger threshold
    // Serial.println(lightReadingChange); //plot derivative value 
    
  } //end of sample at desired freq.
}

//-------------------------------------------------------------------------------------------------
//initializeLightSensor
//-------------------------------------------------------------------------------------------------
void initializeLightSensor(unsigned long dt)
{
  //Initialize light sensor 
  //-take samples for a while to first let the light filters and moving averages and all kick in and take effect; otherwise, it will be trying to trigger from the get-go 
  unsigned long tStart = millis();
  while (millis() - tStart < dt)
  {
    bool unused;
    sampleLightSensor(&unused);
  }
}
