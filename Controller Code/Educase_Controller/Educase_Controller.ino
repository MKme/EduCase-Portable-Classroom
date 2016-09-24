/* 
Erics Open Source Educase Portable classroom
This code is for the Arduino monitoring and control board
Add relays as you wish to control the power on board or additional systems
My Youtube Channel  : http://www.youtube.com/mkmeorg
If you use this code or personalize it etc- please consider sharing it back with the world via the forum at http://www.mkme.org/forum

Notes:
 RTC support
 http://www.hobbyist.co.nz/?q=real_time_clock
 RTC Connected to VCC and Ground
 I2C Connections:
 RTC SDA connected to pin Analog 4
 RTC SCL connected to pin Analog 5

Used I2CScanner.ino to find I2C addresses- 0x50 (EEPROM?)  and 0x68 found
Used SetRTC.ino to manually set proper date/date on RTC module- This wil set the date/time as soon as serial 
is initiated so set ahead by a minute and wait till time matches then open serial window to set.
Downloaded from http://projectsfromtech.blogspot.com/
dht11:
Connect pin 1 (on the left) of the sensor to +5V
Connect pin 2 of the sensor to whatever your DHTPIN is
Connect pin 4 (on the right) of the sensor to GROUND
Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

Pin 11 for contrast on the Nokia 5110
*/

#include "Wire.h" //RTC
#define DS1307_ADDRESS 0x68 //For RTC Support
byte zero = 0x00; //workaround for issue #527 on RTC
#include "DHT.h"  //You need this library
#define DHTPIN 2     // DHT11 Humidity Pin
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);//unknown needed by DHT11
#include "Adafruit_GFX.h"  //Awesome Adafruit libraries- download from their site
#include "Adafruit_PCD8544.h"  //Awesome Adafruit libraries- download from their site
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);//4 and 3 are reversed on the cheap eBay models

//LCD display & backlight control 
#define backlightpin 11 //Pin for LCD backlight
int backlightlevel = 220; //Level for backlight PWM
int backlightoff = 255; //Level which turns backlight off completely
int displaycontrast = 60; //***** Set this contrast to suit your display- they are indeed different

//Ouput relay
#define LIGHTPIN 10 //Pin for output relay

//Ouput relay
#define TempPin 12 // Relay pin for Ouput relay
//Globalize variables
int second, minute, hour, weekDay, monthDay, month, year;
int h;//humidity used to be float for decimal places
int t;//temperature used to be float for decimal places

//Alarms
int alarmseconds; // Used for overtemp alarm seconds since active for display flashing
int AlarmTemp = 4; // Alarm temp OVER or UNDER setTemp value
int ActiveAlarm; // Used for overtemp alarms to notify etc

int Voltage;
int Voltage2;
int Voltage3;
int Voltage4;
#define batteryPin 3 //pin for voltage measurement
const float referenceVolts = 12.6;  
//voltage divider with 2 10Kohm resistors to monitor the 12.6V supply from battery
//http://forum.arduino.cc/index.php/topic,13728.0.html
//currently rersistors not included on PCB so wire externally



/*
Alarm Codes:
0 = No active alarm
1 = Internal Temp <> AlarmTemp from setTemp value (over or under temp condition)
2 = TBD (Previously na)
3 = TBD (Previously na)
4 = TBD (Previously na)
5 = TBD (Previously na)
6 = TBD (Previously na)
7 = TBD (na)
8 = TBD (Previously Watchdog/Reboot Triggered)
*/




void setup(){  //Do setup type stuff to make cooler stuff work later--------------------------
  Wire.begin();
  Serial.begin(9600);
  dht.begin();//initialize temp and humidity
  analogWrite(backlightpin,backlightlevel);// PWM of LCD backlight but ebay unit is backwards-   //must go high + cycle to dim   //Very Dim=230
  display.begin();//Display code
  display.setContrast(displaycontrast);//Nokia 5110 works best at 50- no more flicker
  delay(1000);  //party foul I know but it is only setup
  display.clearDisplay();     // clears the screen and buffer- makes gooder stuff show up later
  display.setTextSize(1);     // set text size
  display.setTextColor(BLACK);
  
 // Splash Screen - 
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("    Eric's");
  display.println("");
  display.println("   Educase");
  display.println("  Portable");
  display.println(" Classroom");
  display.display();
  delay(2000);
  display.clearDisplay();     // clears the screen and buffer
  
 }

void loop(){
  TimeFunctions();   //Do the time related stuff
  TempFunctions();   //Do the temp related stuff
  VoltageFunctions();   //Do the temp related stuff
  debug();           //Figure out what has broken type of stuffs
  LCDDisplay();      //Send everything to the LCD display
  Alarm();           //Activate alarm if applicable
  DisplayFlash();    //Flash display if alarm active
  
}


//------------------------------This needed by RTC
byte decToBcd(byte val){  //Needed for RTC
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)  {  //Needed for RTC
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}
//------------------------------End needed by RTC


//Do time related stuffs so cool stuffs work gooder------------------------------
void TimeFunctions(){ 
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}

//Do temperature related stuffs----------------------------------------------------------
void TempFunctions(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  t = dht.readTemperature();
}

void VoltageFunctions(){
int val = analogRead(batteryPin);  // read the value from the A0 battery monitoring pin with voltage divider
float volts = (val / 511.0) * referenceVolts ; // divide val by (1023/2)because the resistors divide the voltage in half
  
}
//Do debugging stuffs for nerds only-----------------------------------------------------------------
void debug(){  //This area only used for debugging any changes to the code- handy dandy sometimes :)
  if (Serial.available()){ //Can leave debug active as no resources used when not connected
  //Date and Time Values   3/1/11 23:59:59
  Serial.print(month);
  Serial.print("/");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  if (hour < 10) Serial.print("0");
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) Serial.print("0");
  Serial.print(minute);
  Serial.print(":");
  if (second < 10) Serial.print("0");
  Serial.println(second);
    //humidity Sensor Values
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
      Serial.println("Failed to read from DHT");} 
    else {
      Serial.print("Humidity: "); 
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: "); 
      Serial.print(t);
      Serial.println(" *C"); }

//Alarm Status
  Serial.print("Active Alarm "); 
  Serial.println(ActiveAlarm);
}}

//Write everything to the LCD so folks can see what's up---------------------------------
void LCDDisplay(){
  display.clearDisplay();              // clears the screen and buffer
  display.setCursor(0,0);
  display.setTextSize(1);     // set text size
  display.print(month);
  display.print("/");
  display.print(monthDay);
  //display.print("/");
  //display.print(year);
  display.print(" ");
  if (hour < 10) display.print("0");
  display.print(hour);
  display.print(":");
  if (minute < 10) display.print("0");
  display.println(minute);
  //display.print(":");
  //if (second < 10) display.print("0");
  //display.println(second);
  display.println("");
  display.print(t);
  display.print("*C  ");
  display.print(h);
  display.println("% RH");
  display.print("Batt V: ");
  display.println(Voltage);
  display.print("Aux V: ");
  display.println(Voltage2);
  display.display();
  }
   
void Alarm(){
  //Overtemp OR Undertemp alarm--------------------------------------
//  if (t >= setTemp + AlarmTemp  || t <= setTemp - AlarmTemp && second != alarmseconds  ) {ActiveAlarm = 1;} //Checks over or under alarm limit and setas alarm #1
 // else   ActiveAlarm = 0; //No active alarm criteria met
  
  //Insert Alarm here
  
  //Insert Alarm here
  
  //Insert non desired output alarms here
  
  //Insert SkyNet has teaken over the world via my garden alarm here

}

//Flash the display if any alarm is active------------------------------
void DisplayFlash(){
  if (ActiveAlarm >0  && second != alarmseconds){
  alarmseconds = second; 
  analogWrite(backlightpin,backlightoff);// Turn off backlight so flashing can begin
  }
  //Else Normal backlight- no more flashing
  else 
  analogWrite(backlightpin,backlightlevel);// Normal PWM of LCD backlight
  }
