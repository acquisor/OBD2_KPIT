/*CAN Transmitter*/
#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>

#include <SPI.h>
#include <mcp_can.h>

#include "acQuisor_WiFi.h"

char *host = "172.22.25.3";
const int httpPort = 80;

ESP8266WebServer server(80);

String wifiSsid = "anjali", wifiPass = "12345678", apSsid = "OBD_Simulator", apPass = "acquisor123";
acQuisorWiFi acqWifi(wifiSsid, wifiPass, apSsid, apPass, host);

const int spiCSPin = D8;

MCP_CAN CAN(spiCSPin);

void data();
void handleRoot();

String speedDec, rpmADec, rpmBDec, throttleDec, loadDec, DTCflag;

byte Speed,RpmA,RpmB,Throttle,Load;

void setup()
{
    Serial.begin(115200);

    Serial.print("Configuring access point...");
    WiFi.softAP(acqWifi.device_SSID.c_str(), acqWifi.device_PASS.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    server.on("/", handleRoot);
    server.on("/androdata",data);
    server.begin();

    if (acqWifi.customerWifiSsid != "default" && acqWifi.customerWifiSsid != "")
      acqWifi.Wconnect();

    while(CAN_OK != CAN.begin(CAN_500KBPS))
    {
        Serial.println("CAN BUS init Failed");
      //  delay(100);
    }
    Serial.println("CAN BUS Shield Init OK!");
}

void loop()
{   
  server.handleClient();

  unsigned char stmp[8]  = {0X41,0X0D,Speed,0X55,0X55,0X55,0X55,0X55};   // Vehicle Speed
  unsigned char stmp1[8] = {0X41,0X0C,RpmA,RpmB,0X55,0X55,0X55,0X55};   // RPM
  unsigned char stmp2[8] = {0X41,0X04,Load,0X55,0X55,0X55,0X55,0X55};   // Engine Load
  unsigned char stmp3[8] = {0X41,0X11,Throttle,0X55,0X55,0X55,0X55,0X55};   // Throttle Position   
  unsigned char stmp4[8] = {0x43,0X06,0X54,0X41,0X61,0X81,0X60,0X00};   // p0654-rpm op ckt failure,C0161-abs/tcs brake sw ckt malfunction ,b0160 - ambient air temp sensor ckt
  unsigned char stmp5[8] = {0X43,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   // No DTC Response
  unsigned char stmp6[8] = {0X44,0X00,0X00,0X00,0X00,0X00,0X00,0X00};   // DTC Cleared Response
  
  Serial.print("\nSpeed CAN Frame: ");  
  for(int j=0;j<8;j++)
  {
      Serial.print(stmp[j]);
      Serial.print("\t");  
  }
  
  Serial.print("\nRPM CAN Frame: ");  
  for(int j=0;j<8;j++)
  {
      Serial.print(stmp1[j]);
      Serial.print("\t");  
  }

  Serial.print("\nEngine Load CAN Frame: ");  
  for(int j=0;j<8;j++)
  {
      Serial.print(stmp2[j]);
      Serial.print("\t");  
  }

  Serial.print("\nThrottle CAN Frame: ");  
  for(int j=0;j<8;j++)
  {
      Serial.print(stmp3[j]);
      Serial.print("\t");  
  }
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
            if(DTCflag=="1")
            {
            Serial.println("Sending DTC Code");
            CAN.sendMsgBuf(0x70, 0, 8, stmp4);
            }
            
            else if(DTCflag=="0")
            {
              Serial.println("Sending DTC Code");
              CAN.sendMsgBuf(0x70, 0, 8, stmp5);
            }
        }
        else if(i==0 && buf[i]==0X04)
        {
            Serial.println("Cleared DTC");
            DTCflag="0";
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
        delay(100);
  }
  
}

void data()
{
  speedDec = String(server.arg("speed"));
  rpmADec = String(server.arg("rpma"));
  rpmBDec = String(server.arg("rpmb"));
  throttleDec = String(server.arg("throttle"));
  loadDec = String(server.arg("load"));
  DTCflag = String(server.arg("dtc"));

  
  Speed=speedDec.toInt();
  RpmA=rpmADec.toInt();
  RpmB=rpmBDec.toInt();
  Throttle=throttleDec.toInt();
  Load=loadDec.toInt();

  Serial.print("Speed: ");
  Serial.println(Speed);
  Serial.print("RpmA: ");
  Serial.println(RpmA);
  Serial.print("RpmB: ");
  Serial.println(RpmB);
  Serial.print("Load: ");
  Serial.println(Load);
  Serial.print("Throttle: ");
  Serial.println(Throttle);
  
  if(DTCflag=="true")
  {
    DTCflag="1";
    Serial.print("\n DTC is Set!!");
  }
    
   else
   {
     DTCflag="0";
     Serial.print("\n DTC is Cleared!!");
   }
   
  String msg = "Speed:";
  msg += speedDec;
  msg += ", \tThrottle: ";
  msg += throttleDec;
  msg += ", \tEngine Load: ";
  msg += loadDec;
  msg += ", \nRPM_A: ";
  msg += rpmADec;
  msg += ", \tRPM_B: ";
  msg += rpmBDec;
  msg += ", \tDTC: ";
  msg += DTCflag;

  Serial.println("Simulation details received: ");
  Serial.println(msg);
  
  server.send(200, "text/html", msg);  

}

void handleRoot()
{
  String message2 = "<center><h1> Welcome to acQuisor OBD simulator </h1><br><br> The IP address of your simulator is:" ;
  message2 += acqWifi.deviceIP;
  message2 += "<br><br>Thank you for buying acQuisor OBD.<br><br><b>Contact us at 1505051@ritindia.edu </b></center>";
  
  server.send(200, "text/html", message2);
}
