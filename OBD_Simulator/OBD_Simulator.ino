
/*CAN Transmitter*/

#include <SPI.h>
#include <mcp_can.h>

const int spiCSPin = D8;
boolean ledON = 1;
boolean DTCflag=1;
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

unsigned char stmp[8]  = {0X41,0X0D,0X43,0X12,0X55,0X55,0X55,0X55};   // Vehicle Speed
unsigned char stmp1[8] = {0X41,0X0C,0XAA,0X88,0X55,0X55,0X55,0X55};   // RPM
unsigned char stmp2[8] = {0X41,0X04,0X54,0X47,0X55,0X55,0X55,0X55};   // Engine Load
unsigned char stmp3[8] = {0X41,0X11,0X69,0X09,0X55,0X55,0X55,0X55};   // Throttle Position
unsigned char stmp4[8] = {0x43,0X06,0X54,0X41,0X61,0X81,0X60,0X00};   // p0654-rpm op ckt failure,C0161-abs/tcs brake sw ckt malfunction ,b0160 - ambient air temp sensor ckt
unsigned char stmp5[8] = {0X43,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   // No DTC Response
unsigned char stmp6[8] = {0X44,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   // DTC Cleared Response

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
            
            if(i==0 && buf[i]==0X03)
            {
                if(DTCflag==1)
                {
                Serial.println("Sending DTC Code");
                CAN.sendMsgBuf(0x70, 0, 8, stmp4);
                }
                
                else if(DTCflag==0)
                {
                  Serial.println("Sending DTC Code");
                  CAN.sendMsgBuf(0x70, 0, 8, stmp5);
                }
            }
            else if(i==0 && buf[i]==0X04)
            {
                Serial.println("Cleared DTC");
                DTCflag=0;
                CAN.sendMsgBuf(0x70, 0, 8, stmp6);
            }
            
            if(i==1)
            {
             if(buf[i]==0x0D)
             {
              Serial.println("Sending Vehicle Speed Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp);
             }
             else if(buf[i]==0x0C)
             { 
              Serial.println("Sending RPM Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp1);
             }
              
              else if(buf[i]==0x04)
              {
              Serial.println("Sending Engine Load Data");
              CAN.sendMsgBuf(0x70, 0, 8, stmp2);
              }
              
              else if(buf[i]==0x11)
              {
                Serial.println("Sending Throttle Data");
                CAN.sendMsgBuf(0x70, 0, 8, stmp3);
              }
            }
        }
        Serial.println();
}
}
