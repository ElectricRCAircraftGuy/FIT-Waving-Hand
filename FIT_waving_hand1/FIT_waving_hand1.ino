/*
FIT_waving_hand
 - a little code to control the device in the window of the lab to wave a little hand when people walk by

By Gabriel Staples 
www.ElectricRCAircraftGuy.com
- for email see my "Contact Me" tab at top of website 

Started: 21 July 2017
Last Updated: 21 July 2017 

Circuit:
Servos x 2
 - servos x 2 to 5V and GND as required
 - Servo signals connected to pins as defined below

Sharp 2Y0A02 F 5Y IR distance sensor 
 - 5V and GND as required 
 - signal as specified below 
 
LDR (Light Dependent Resistor; AKA: photoresistor)
5V--4.4k--(sense_pin)--LDR--GND


*/

//libraries:
#include <Servo.h>
#include <eRCaGuy_NewAnalogRead.h>

//global vars & consts
const byte SERVO_UPDOWN_PIN = 5;
const byte SERVO_WAVE_PIN = 6;
const byte IR_DIST_SENSOR_PIN = A0;
const byte LDR_PIN = A1;

//analog readings
byte bitsOfResolution = 12; //commanded oversampled resolution
unsigned long numSamplesToAvg = 5; //number of samples AT THE OVERSAMPLED RESOLUTION that you want to take and average
ADC_speed_t ADCSpeed = ADC_FAST; 

//-------------------------------------------------------------------------------------------------
//setup 
//-------------------------------------------------------------------------------------------------
void setup()
{
  pinMode(IR_DIST_SENSOR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  
  Serial.begin(115200);
  delay(100); //give time for serial debugger turn on and stabilize 
  Serial.println(F("\nBegin"));
  
  //Configure the adc how you want it
  adc.setADCSpeed(ADCSpeed);
  adc.setBitsOfResolution(bitsOfResolution);
  adc.setNumSamplesToAvg(numSamplesToAvg);
}

//-------------------------------------------------------------------------------------------------
//loop
//-------------------------------------------------------------------------------------------------
void loop()
{
  static bool triggerNow = false;
  sampleLightSensor(&triggerNow);
}

//---------------------------------------------------------------------------------------
//sampleLightSensor
//-sample the light sensor at the desired rate, and update the triggerNow variable if the light is turned off
//---------------------------------------------------------------------------------------
void sampleLightSensor(bool *triggerNow_p)
{
  //sample the light sensor at desired freq.
  static unsigned long t_start = micros(); //us 
  //set desired sample time pd, dt_des
  //-NB: Use *LARGER* values for *MORE SENSITIVITY* AND *LESS NOISE* (longer sample period gives the light sensor time to fully change to a new light condition, thereby giving a larger delta between readings), and use smaller values for a faster sample rate 
  const unsigned long dt_des = 40000; //us; 10000us --> 100 Hz; 20000 --> 50 Hz, 40000 --> 25 Hz, 50000 --> 20 Hz, etc 
  unsigned long t_now = micros(); //us 
  if (t_now - t_start >= dt_des)
  {
    t_start = t_now; //us; update 
    
    //local constants and variables 
    static unsigned int lightReading = adc.newAnalogRead(LDR_PIN);
    // Serial.println(lightReading); //FOR DEBUGGING 
    static unsigned int lightReading_old = lightReading;
    const int LIGHT_READING_DELTA_TRIGGER = -50; //<--GS TUNE THIS VALUE 
    
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
    
    //test to see if the light was turned OFF or ON
    int lightReadingChange = lightReading - lightReading_old; 
    lightReading_old = lightReading; //update 
    if (lightReadingChange <= LIGHT_READING_DELTA_TRIGGER)
    {
      //light was just turned OFF, so swing the servo arm to trigger the motion sensor!
      *triggerNow_p = true; 
      // Serial.println(F("triggerNow==true"));
    }
    else if (lightReadingChange >= -LIGHT_READING_DELTA_TRIGGER) 
    {
      //light was just turned back ON, so stop triggering it to turn back on
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
    Serial.print(LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot lower trigger threshold
    Serial.print(-LIGHT_READING_DELTA_TRIGGER); Serial.print(", "); //plot upper trigger threshold
    Serial.println(lightReadingChange); //plot derivative value 
    
  } //end of sample at desired freq.
}