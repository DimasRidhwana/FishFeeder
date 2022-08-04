/* 
  Water Level
  sensorPinWL 27
  sensorPower 26

  Ultrasonic
  trig 12
  echo 13

  pHmeter
  potPin 34

  Water Temp
  Sensor pin  25

  Servo
  signal 14

  FlowMeter
  interrupt 33
  Sensor pin 33
  
 */


#include <HCSR04.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define sensorPinWL 27
#define sensorPower 26



// Water Level
int val = 0;

// Ultrasonic
HCSR04 hc(12, 13); //initialisation class HCSR04 (trig pin , echo pin)

// Servo
Servo myservo;  // create servo object to control a servo

// pHmeter
const int potPin=34;
float ph;
float Value=0;

// Water Temperature
const int SENSOR_PIN = 25; 
OneWire oneWire(SENSOR_PIN);         
DallasTemperature tempSensor(&oneWire); 
float tempCelsius;    

// FlowMeter 
byte sensorInterrupt = 33; 
byte sensorPin       = 33;
float calibrationFactor = 4.5;
volatile byte pulseCount;  
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;


void setup()
{
    // Water Level
    pinMode(sensorPower, OUTPUT);
    digitalWrite(sensorPower, LOW);
  
    //Servo
    myservo.attach(14);  // attaches the servo on pin 9 to the servo object
    myservo.write(180);
    delay(1000);
    myservo.write(0);
    delay(100);
    myservo.write(180);
    delay(100);
    myservo.write(0);
    delay(1000);
    myservo.write(180);
    delay(1000);

    // pHmeter
    pinMode(potPin,INPUT);
    delay(1000);

    // Water Temp
    tempSensor.begin();

    //FlowMeter
     pinMode(sensorPin, INPUT);
     digitalWrite(sensorPin, HIGH);
     pulseCount        = 0;
     flowRate          = 0.0;
     flowMilliLitres   = 0;
     totalMilliLitres  = 0;
     oldTime           = 0;
     attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    

    Serial.begin(9600);
}

void loop()
{
    Water_Level();
    Ultrasonic();
    pHmeter();
    WaterTemp();
    FlowMeter();
}

void Water_Level()
{
    int level = readSensor();
    if (level < 3000){
      Serial.print("Kering");
      }
    else {
      Serial.print("Basah");
      }
      delay(1000);
}

void Ultrasonic() 
{
    Serial.println(hc.dist()); 
    delay(60);
}


void pHmeter()
{
    Value= analogRead(potPin);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage=Value*(3.3/4095.0);
    ph=(3.3*voltage);
    Serial.println(ph);
    delay(500);
}

void WaterTemp()
{
  tempSensor.requestTemperatures();             // send the command to get temperatures
  tempCelsius = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(tempCelsius);    // print the temperature in Celsius
  Serial.print("°C");
  Serial.print("  ~  ");        // separator between Celsius and Fahrenheit

  delay(500);
}


void FlowMeter()
{
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");           // Print tab space

    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.println();
    

    pulseCount = 0;
    
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}



// ADITIONAL CODE

// FlowMeter
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}


// Water Level
int readSensor() {
  digitalWrite(sensorPower, HIGH);  
  delay(10);                        
  val = analogRead(sensorPinWL);      
  digitalWrite(sensorPower, LOW);  
  return val;                      
};
