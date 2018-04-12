/*
ESP WiFi Arming Code Rev 01
Created by Tylor Jilk, February 2018
A part of the IREC Avionics Team of Stanford SSI

Once the ESP is running, connect to its Wi-Fi network
and go to one of 3 websites to do exactly what they
sound like: ipaddress/arm, /disarm, or /status

The GPIO pin which is controlled by these commands
is called 'onpin'.


If the serial into the ESP contains "Arm"/"Disarm", it will tell motherboard ESP to arm/disarm. If it contains
"Stage", it will tell the Staging ESP to trigger staging
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


const char WiFiAPPSK[] = "redshift";
const int onpin = 5; // IO5 on the Esp8266 WROOM 02
String motherboard_ip = "192.168.4.2";
String staging_ip = "192.168.4.3";
String payload_ip = "192.168.4.4";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(onpin, OUTPUT);
  digitalWrite(onpin, LOW);
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  IPAddress ip(192,168,4,1);
  IPAddress dns(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  WiFi.config(ip,dns,gateway,subnet);
  WiFi.softAPConfig(ip,gateway,subnet);
  String AP_NameString = "Skybass";
  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);
  for (int i=0; i<AP_NameString.length(); i++)
  AP_NameChar[i] = AP_NameString.charAt(i);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
  server.begin();
}

String send_request(String ip, String command){
  String resp = "";
  String url = "http://"+ip+"/"+command;
  Serial.println("Sending request: "+ url); //DBG
  HTTPClient stHttp;
  stHttp.begin(url);
  int stHttpCode = stHttp.GET();
  if (stHttpCode == HTTP_CODE_OK)
  {
      String payload = stHttp.getString();
      Serial.println("Request to "+url+" OK. Response: "+payload); //DBG
      resp +="\n"+ ip + "/" + command + " response: "+payload;
  }
  else
  {
    Serial.println("Request to "+url+" Failed. Code: " + stHttpCode); //DBG
  }
  stHttp.end();
  return resp;
}

void loop() {
  /**
* Takes in "Staging", "Arm", or "Disarm" from Skybass Teensy. Sends HTTP request
* to appropriate IP address. Prints response back to Teensy.
*
**/

  String out ="";
  String esp_cmd = Serial.readString();  //TO DO - check this
  Serial.println("Serial in: "+esp_cmd);


  if(esp_cmd.indexOf("Stage")!=-1)
  {
    String staging_response = send_request(staging_ip,"open");
    out+="Staging Resp. to Open: "+staging_response;
  }
  else
  {
    String staging_response = send_request(staging_ip,"status");
    out+="Staging Resp. to Status: "+staging_response;
  }

  if(esp_cmd.indexOf("Arm")!=-1)
  {
    String motherboard_arm_response = send_request(motherboard_ip,"arm");
    out+="Motherboard Resp. to Arm: "+motherboard_arm_response;
  }
  else if(esp_cmd.indexOf("Disarm")!=-1)
  {
    String motherboard_disarm_response = send_request(motherboard_ip,"disarm");
    out+="Motherboard Resp. to Disarm: "+motherboard_disarm_response;
  }
  else
  {
    String motherboard_status = send_request(motherboard_ip,"status");
    out+="Motherboard Resp. to Status: "+motherboard_status;
  }

  if(esp_cmd.indexOf("ArmPayload")!=-1)
  {
    String payload_arm_response = send_request(payload_ip,"arm");
    out+="Payload Resp. to Arm: "+payload_arm_response;
  }
  else if(esp_cmd.indexOf("DisarmPayload")!=-1)
  {
    String payload_disarm_response = send_request(payload_ip,"disarm");
    out+="Payload Resp. to Disarm: "+payload_disarm_response;
  }
  else
  {
    String payload_status = send_request(payload_ip,"status");
    out+="Payload Resp. to Status: "+payload_status;
  }


Serial.println("Serial Out: "+out);

}
