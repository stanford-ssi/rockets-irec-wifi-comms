
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
const char APName[] = "Skybass";
const int onpin = 4; // IO5 on the Esp8266 WROOM 02
const int ledpin = 5;
String payload_ip = "192.168.4.2";
String a = "Armed";
String d = "Disarmed";
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(onpin, OUTPUT);
  digitalWrite(onpin, LOW);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, HIGH);
  WiFi.mode(WIFI_AP);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  IPAddress ip(192, 168, 4, 1);
  IPAddress dns(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(APName, WiFiAPPSK);
  server.begin();
}

String send_request(String ip, String command) {
  String resp = "";
  String url = "http://" + ip + "/" + command;
  Serial.println("Sending request: " + url); //DBG
  HTTPClient stHttp;
  stHttp.begin(url);
  int stHttpCode = stHttp.GET();
  if (stHttpCode == HTTP_CODE_OK)
  {
    String payload = stHttp.getString();
    Serial.println("Request to " + url + " OK. Response: " + payload); //DBG
    resp += "\n" + ip + "/" + command + " response: " + payload;
  }
  else
  {
    Serial.println("Request to " + url + " Failed. Code: " + stHttpCode); //DBG
  }
  stHttp.end();
  return resp;
}

void loop() {
  /**
    Takes in "Staging", "Arm", or "Disarm" from Skybass Teensy. Sends HTTP request
    to appropriate IP address. Prints response back to Teensy.
  **/

  String out = "";

  if (Serial.read() == 0xAA)
  {
    digitalWrite(ledpin, LOW);
    String pr = send_request(payload_ip, "arm");
    out += "Payload: " + pr;
  }
  if (Serial.read() == 0xAB)
  {
  digitalWrite(ledpin, LOW);
  String pr = send_request(payload_ip, "disarm");
  out += "Payload: " + pr;
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: " + req); //DBG
  client.flush();
  String resp = "";

  if (req.indexOf("/disarm") != -1)
  {
    digitalWrite(onpin, 0);
    resp = d;
  }
  else if (req.indexOf("/arm") != -1)
  {
    digitalWrite(onpin, 1);
    resp = a;
  }

  /*
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
  */
  Serial.println("Serial Out: " + out);

}
