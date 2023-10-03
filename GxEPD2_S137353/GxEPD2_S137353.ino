// GxEPD2_HelloWorld.ino by Jean-Marc Zingg

// see GxEPD2_wiring_examples.h for wiring suggestions and examples
// if you use a different wiring, you need to adapt the constructor parameters!

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <rn2xx3.h>
#include <SoftwareSerial.h>

//     __  _          ___        __   
//    / / | |    ___ | _ \ __ _  \ \  
//   < <  | |__ / _ \|   // _` |  > > 
//    \_\ |____|\___/|_|_\\__,_| /_/  
//                                    
SoftwareSerial mySerial(6, 5); // RX, TX
//create an instance of the rn2xx3 library,
//giving the software serial as port to use
rn2xx3 myLora(mySerial);

//     __   ___       ___  ___  ___  ___  __   
//    / /  / __|__ __| __|| _ \|   \|_  ) \ \  
//   < <  | (_ |\ \ /| _| |  _/| |) |/ /   > > 
//    \_\  \___|/_\_\|___||_|  |___//___| /_/  
//                                             
// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"
//GxEPD2_BW<GxEPD2_154_D67, MAX_HEIGHT(GxEPD2_154_D67)> display(GxEPD2_154_D67(/*CS=10*/ SS, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7)); // GDEH0154D67 200x200, SSD1681

// or select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
#include "GxEPD2_display_selection_added.h"

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
// e.g. for Wemos D1 mini:
//GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEH0154D67

/*void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}*/

//    ___        _  _    _        _  _            ___           _  _      
//   |_ _| _ _  (_)| |_ (_) __ _ | |(_) ___ ___  | _ \ __ _  __| |(_) ___ 
//    | | | ' \ | ||  _|| |/ _` || || ||_ // -_) |   // _` |/ _` || |/ _ \
//   |___||_||_||_| \__||_|\__,_||_||_|/__|\___| |_|_\\__,_|\__,_||_|\___/
//                                                                        
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
  const char *appKey = "134F4BDC388A586AD0837CF1FF706F56";

  join_result = myLora.initOTAA(appEui, appKey);


  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have Helium coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined Helium");
}


void setup()
{
  //display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  helloWorld();
  display.hibernate();
  
  
  //output LED pin
  //pinMode(13, OUTPUT);
  //led_on();
  // Open serial communications and wait for port to open:
  Serial.begin(115200); //serial port to computer
  mySerial.begin(9600); //serial port to radio
  Serial.println("Startup");

  initialize_radio();

  //transmit a startup message
  myLora.tx("Helium Mapper on Helium Enschede node");

  //led_off();
  delay(2000);
  
}

const char HelloWorld[] = "S137353";

void helloWorld()
{
  display.setRotation(2);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, 10);
    display.print(HelloWorld);
  }
  while (display.nextPage());
}

void loop() {

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
  //refresh(Temp1,Temp2);

  Serial.println("TXing");
  myLora.tx("!"); //one byte, blocking function

  //led_off();
  delay(2000);        // delay in between reads for stability


};
