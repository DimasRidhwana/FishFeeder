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

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;


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
byte sensorInterrupt = 33;  // 0 = digital pin 2
byte sensorPin       = 33;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
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
  
    // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

    Serial.begin(115200);
}

void loop()
{
    Water_Level();
    Ultrasonic();
    pHmeter();
    WaterTemp();
    FlowMeter();

        
    if (bootCount%8 == 0) {
    // run code if test expression is true
      servo();
    }
    deepsleep();
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
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
  Serial.println();
    

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

void servo(){
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
  Serial.println("servo work");

}

void deepsleep(){
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
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
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
};
