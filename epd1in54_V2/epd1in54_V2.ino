#include <SPI.h>
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>
#include <rn2xx3.h>
#include <SoftwareSerial.h>

// LoRa 
SoftwareSerial mySerial(6, 5); // RX, TX

//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

// ePaper
Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1

void refresh(float temp1, float temp2){
  char tempbuff1[14];
  char tempbuff2[14];
  dtostrf(temp1, 4, 2, tempbuff1);
  dtostrf(temp2, 4, 2, tempbuff2);
  Serial.println(tempbuff1);
  Serial.println(tempbuff2);

  Serial.println(F("e-Paper paint"));
  epd.Clear();
  paint.Clear(COLORED);
  paint.DrawStringAt(0, 0, "< S137353 >", &Font24, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 10, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 4, "Temperature 1", &Font20, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 40, paint.GetWidth(), paint.GetHeight());
  paint.Clear(COLORED);
  paint.DrawStringAt(0, 5, tempbuff1, &Font24, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 80, paint.GetWidth(), paint.GetHeight());
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 4, "Temperature 2", &Font20, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 120, paint.GetWidth(), paint.GetHeight());
  paint.Clear(COLORED);
  paint.DrawStringAt(0, 5, tempbuff2, &Font24, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 160, paint.GetWidth(), paint.GetHeight());
  delay(50);        // delay in between reads for stability

  epd.DisplayPartFrame();
}

void initialize_radio()
{
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //Autobaud the rn2483 module to 9600. The default would otherwise be 57600.
  myLora.autobaud();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println(F("Communication with RN2xx3 unsuccessful. Power cycle the board."));
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println(F("When using OTAA, register this DevEUI: "));
  Serial.println(myLora.hweui());
  Serial.println(F("RN2xx3 firmware version:"));
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println(F("Trying to join Helium"));
  bool join_result = false;

  /*
   * ABP: initABP(String addr, String AppSKey, String NwkSKey);
   * Paste the example code from the TTN console here:
   */
  /*const char *devAddr = "02017201";
  const char *nwkSKey = "AE17E567AECC8787F749A62F5541D522";
  const char *appSKey = "8D7FFEF938589D95AAD928C2E2E7E48F";*/

  //join_result = myLora.initABP(devAddr, appSKey, nwkSKey);

  /*
   * OTAA: initOTAA(String AppEUI, String AppKey);
   * If you are using OTAA, paste the example code from the TTN console here:
   */
  //   6081F906DF388C4B
  //   15FAEF89ACF5342CF3E1F86BE430CB19
  const char *appEui = "6081F906DF388C4B";
  const char *appKey = "15FAEF89ACF5342CF3E1F86BE430CB19";

  join_result = myLora.initOTAA(appEui, appKey);


  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have Helium coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined Helium");
}

/*void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}*/

void setup()
{
  //output LED pin
  //pinMode(13, OUTPUT);
  //led_on();

  // Open serial communications and wait for port to open:
  Serial.begin(115200); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  Serial.println("Startup");
  //Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();
  paint.SetWidth(200);
  paint.SetHeight(24);
  refresh(0.0,0.0);

  initialize_radio();

  //transmit a startup message
  myLora.tx(F("Helium Mapper on Helium Enschede node"));

  //led_off();
  delay(2000);
}

void loop()
{
  //led_on();

  int sensorValue1 = analogRead(A0);
  int sensorValue2 = analogRead(A1);
  float voltage1 = sensorValue1 * (3.3 / 1023.0);
  float voltage2 = sensorValue2 * (3.3 / 1023.0);

  float R1 = 10;
  //Voltage divider fixed R
  float Vsupply = 3.3;
  int beta = 3435;
  //Calculated from exercise
  float T00 = 273.0;
  // Temperature at 0 degrees
  float R0 = 27.70;
  //Resistance at 0 degrees
  float Rsensor1 = (voltage1 * R1) / (Vsupply - voltage1) ;
  float Temp1 = ((T00*beta) / (beta - T00*(log(R0/Rsensor1))) -273);
  float Rsensor2 = (voltage2 * R1) / (Vsupply - voltage2) ;
  float Temp2 = ((T00*beta) / (beta - T00*(log(R0/Rsensor2))) -273);
  Serial.print(Rsensor1);
  Serial.print(" : ");
  Serial.print(Temp1);
  Serial.println(" Degrees");
  Serial.print(Rsensor2);
  Serial.print(" : ");
  Serial.print(Temp2);
  Serial.println(" Degrees");
  refresh(Temp1,Temp2);

  Serial.println("TXing");
  myLora.tx("!"); //one byte, blocking function

  //led_off();
  delay(2000);        // delay in between reads for stability

}
