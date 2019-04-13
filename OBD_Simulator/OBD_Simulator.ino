
/*CAN Transmitter*/

#include <SPI.h>
#include <mcp_can.h>

const int spiCSPin = D8;
boolean ledON = 1;
MCP_CAN CAN(spiCSPin);

void setup()
{
    Serial.begin(115200);
    
    while (CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS init Failed");
        delay(100);
    }
    Serial.println("CAN BUS Shield Init OK!");
}

unsigned char stmp[8] = {0X41,0X0C,0X43,0X12,0X55,0X55,0X55,0X55};
unsigned char stmp1[8] ={0X42,0X0D,0XAA,0X88,0X55,0X55,0X55,0X55};
unsigned char stmp2[8] = {0X43,0X0E,0X54,0X47,0X55,0X55,0X55,0X55};
unsigned char stmp3[8] = {0X44,0X0F,0X69,0X09,0X55,0X55,0X55,0X55};
    
void loop()
{   
  if(CAN_MSGAVAIL == CAN.checkReceive())
    {

          unsigned char len = 0;
          unsigned char buf[8];

        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();

        Serial.println("-----------------------------");
        Serial.print("Data from ID: 0x");
        Serial.println(canId, HEX);

        for(int i = 0; i<len; i++)
        {
            Serial.print(buf[i]);
            Serial.print("\t");

            if(i==1)
            {
             if(buf[i]==0x0C)
             {
              Serial.println("Sending Speed Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp);
              delay(100);
             }
             else if(buf[i]==0x0D)
             { 
              Serial.println("Sending RPM Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp1);
             }
              
              else if(buf[i]==0x0E)
              {
              Serial.println("Sending Engine Load Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp2);
              }
              
              else if(buf[i]==0x0F)
              {
                Serial.println("Sending Throttle Data");
                CAN.sendMsgBuf(0x70, 0, 8, stmp3);
              }
            }
        }
        Serial.println();
}
}
