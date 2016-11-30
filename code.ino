#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include "call.h"
#include <TinyGPS++.h>

//To change pins for Software/Hardware Serial, use 27, 28 lines in GSM.cpp.
//To use Arduino Mega please define MEGA in line 5 in GSM.h.

#include <SPI.h>
#include <SD.h>

//Mega CS pin
const int chipSelect = 53;


//We have to create the classes for SMSs and calls.
CallGSM call;
SMSGSM sms;

char number[20];
byte stat=0;
int value=0;

char value_str[5];

//GPS Variable

// The TinyGPS++ object
TinyGPSPlus gps;

const char *googlePrefix = "http://maps.google.com/maps?q=";


double Lat;
double Long;
int day, month, year;
int hour, minute, second;

int num_sat;

boolean one_point_true=false;



void setup() 
{

  //Serial connection.
  Serial.begin(9600);
  
  Serial.println("Starting GSM...");
  
  //Start configuration of shield with baudrate.
  if (gsm.begin(9600))
    Serial.println("READY");
  else Serial.println("IDLE");

  Serial.println("Starting GPS");
  //GPS com port
  Serial2.begin(9600);

  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(53, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }else Serial.println("card initialized.");

}



void loop() 
{

  Get_GPS();

  Check_call();


};

////////////////////////////////////////////////////////////////////////////////\

void Get_GPS()
{

  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))

      num_sat = gps.satellites.value();
  //Serial.println(num_sat);

  if (gps.location.isValid()==1){

    Lat = gps.location.lat();
    Long = gps.location.lng();

  if (Lat != 0 && Long != 0) one_point_true=true;

  }




  if (gps.date.isValid())
  {
    day = gps.date.day();
    month = gps.date.month();
    year = gps.date.year();
  }

  if (gps.time.isValid())
  {

    hour= gps.time.hour();
    minute = gps.time.minute();
    second = gps.time.second();
  }

  smartDelay(500);


  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }


}

//*******************************************************************************************
// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } 
  while (millis() - start < ms);
}


////////////////////////////////////////////////////////////////////////////////\  


void Check_call()
{
  //Checks status of call
  stat=call.CallStatusWithAuth(number,0,0); //I'm acepting all calls!!!!!!!!!

  //If the any incoming call 
  if(stat==CALL_INCOM_VOICE_AUTH){
    delay(100);
    //Hang up the call.
    call.HangUp();

    //Send an SMS with the position and store data in SD Card
    Send_SMS();

  }
}

//////////////////////////////////////////////////////////////\

void Send_SMS()
{

  int error;

  char Date[13];
  sprintf(Date, "%02d/%02d/%04d ", day, month, year);

  char time1[10];
  sprintf(time1, "%02d:%02d:%02d ", hour, minute, second);

  //Serial print GPS data
  /*
   Serial.print(Date);
   Serial.print("  ");
   Serial.print(time1);
   Serial.print("  ");
   Serial.print("Lat: ");
   Serial.print(Lat, 6);
   Serial.print("  ");
   Serial.print("Long: ");
   Serial.println(Long, 6);
   */

  delay(100);

  //Prepare to send data to GPRS and SD Card
  char lat_print[10];
  dtostrf(Lat,5,6, lat_print);

  char Long_print[10];
  dtostrf(Long,5,6, Long_print);


  if (num_sat >= 3 && one_point_true==true) //We have GPS signal and one valid GPS position
  {
    char sms_OK[160];
    sprintf(sms_OK, "Hugo location is now: Lat: %s, Log: %s. %s%s,+%s\n", lat_print, Long_print,googlePrefix,lat_print,Long_print);
    Serial.println(sms_OK);

    error=sms.SendSMS(number,sms_OK);
    delay(500);  
    if (error==0)  //Check status
    {
      Serial.println("SMS ERROR \n");
    }
    else
    {
      Serial.println("SMS OK \n");             
    }
  }

  else if (num_sat<3 && one_point_true==true) // No signal
  {

    char sms_NOK[150];
    sprintf(sms_NOK, "I can not seen to find Hugo. Last time that i've saw him, was in: Lat: %s, Log: %s. %s%s,+%s\n", lat_print, Long_print,googlePrefix,lat_print,Long_print);
    Serial.println(sms_NOK);

    int error=sms.SendSMS(number,sms_NOK);  
    if (error==0)  //Check status
    {
      Serial.println("SMS ERROR \n");
    }
    else
    {
      Serial.println("SMS OK \n");             
    }
  }

  else if (one_point_true==false)
  {
    Serial.println("No valid GPS point");
  }


  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {

    dataFile.print(Date);
    dataFile.print(" ");
    dataFile.print(time1);
    dataFile.print(" ");
    dataFile.print(number);
    dataFile.print(" ");
    dataFile.print(lat_print);
    dataFile.print(", ");
    dataFile.print(Long_print);
    dataFile.print(" ");
    dataFile.print(error);
    dataFile.println();    

    dataFile.close();

  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 

}


























