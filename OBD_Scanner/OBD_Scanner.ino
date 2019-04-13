/*CAN receiver*/

#include <SPI.h>
#include "mcp_can.h"

const int spiCSPin =D8;
//const int ledPin = 2;
int ledHIGH    = 1;
int ledLOW     = 0;
int flag = 0;
boolean cnt=0;
MCP_CAN CAN(spiCSPin);
unsigned int byteA=0;
unsigned int byteB=0;
unsigned int speedinkmph=0;
void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN,OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS Init Failed");
        delay(100);
    }
    Serial.println("CAN BUS  Init OK!");
}

unsigned char stmp[8] =  {0X01,0X0C,0X55,0X55,0X55,0X55,0X55,0X55};
unsigned char stmp1[8] ={0X01,0X0D,0X55,0X55,0X55,0X55,0X55,0X55};
unsigned char stmp2[8] = {0X01,0X0E,0X55,0X55,0X55,0X55,0X55,0X55};
unsigned char stmp3[8] = {0X01,0X0F,0X55,0X55,0X55,0X55,0X55,0X55};

void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

      if(flag==0 && cnt==0)
      {
      Serial.println("Requesting Speed Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp);
      cnt=1;
      }
       
    if(CAN_MSGAVAIL == CAN.checkReceive())
    {
        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();
          if(canId=0x70)
          {
        Serial.println("-----------------------------");
        Serial.print("Data from CAN ID: 0x");
        Serial.println(canId, HEX);
          
        for(int i = 0; i<len; i++)
        {
            Serial.print(buf[i],HEX);
            Serial.print("\t");
            if(i==0)
            {
              if(buf[i]==0x41)
              {
                flag=1;
              }
              else if(buf[i]==0x42)
              {
                flag=2;
              }
              else if(buf[i]==0x43)
              {
                flag=3;
              }
              else if(buf[i]==0x44)
              {
                flag=0;
              }
           } 
        }
      }
    }
      if(flag==0)
      {
       Serial.println("Requesting Speed Data");
      CAN.sendMsgBuf(0x71, 0, 8, stmp);
      }
      else if(flag==1)
      {
        Serial.println("Requesting RPM Data");
        CAN.sendMsgBuf(0x71, 0, 8, stmp1);
      }

      else if(flag==2)
      {
        Serial.println("Requesting Engine Load Data");
        CAN.sendMsgBuf(0x71, 0, 8, stmp2);
      }

      else if(flag==3)
      {
        Serial.println("Requesting Throttle Position Data");
        CAN.sendMsgBuf(0x71, 0, 8, stmp3);
      }
      delay(1000);
}
