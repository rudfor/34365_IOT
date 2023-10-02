#include <SPI.h>
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>

Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1

void demo()
{
  paint.SetWidth(200);
  paint.SetHeight(24);

  Serial.println("e-Paper paint");
  paint.Clear(COLORED);
  paint.DrawStringAt(30, 4, "Hello world!", &Font16, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 10, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(30, 4, "e-Paper Demo", &Font16, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 30, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(64);
  paint.SetHeight(64);

  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 130, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 130, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  delay(2000);

  Serial.println("e-Paper show pic");
  epd.HDirInit();
  // epd.Display(IMAGE_DATA);

  //Part display
  epd.HDirInit();
  epd.DisplayPartBaseImage(IMAGE_DATA);

  paint.SetWidth(50);
  paint.SetHeight(60);
  paint.Clear(UNCOLORED);

  char i = 0;
  char str[10][10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  for (i = 0; i < 10; i++) {
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(10, 10, str[i], &Font24, COLORED);
    epd.SetFrameMemoryPartial(paint.GetImage(), 80, 70, paint.GetWidth(), paint.GetHeight());
    epd.DisplayPartFrame();
    delay(100);
  }

  Serial.println("e-Paper clear and goto sleep");
  epd.HDirInit();
  epd.Clear();
  epd.Sleep();
}

void refresh(float temp1, float temp2){
  char tempbuff1[14];
  char tempbuff2[14];
  //snprintf(tempbuff1, sizeof(tempbuff1), "%f", temp1);
  //snprintf(tempbuff2, sizeof(tempbuff2), "%f", temp2);
  dtostrf(temp1, 4, 2, tempbuff1);
  dtostrf(temp2, 4, 2, tempbuff2);
  Serial.println(tempbuff1);
  Serial.println(tempbuff2);

  Serial.println("e-Paper paint");
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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();
  paint.SetWidth(200);
  paint.SetHeight(24);
  refresh(0.0,0.0);


  //demo();
}

void loop()
{
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

  delay(2000);        // delay in between reads for stability

}
