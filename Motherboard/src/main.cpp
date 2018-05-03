/*
  ESP WiFi Arming Code Rev 02
  Created by Tylor Jilk, February 2018
  A part of the IREC Avionics Team of Stanford SSI

  Once the ESP is running, connect to its Wi-Fi network
  and go to one of 3 websites to do exactly what they
  sound like: ipaddress/arm, /disarm, or /status

  The GPIO pin which is controlled by these commands
  is called 'onpin'.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPUtils.hpp"

const char MBWifi[] = "Motherboard";
const char MBWifiPwd[] = "redshift";

const int onpin = 5; // IO5 on the Esp8266 WROOM 02

String armed = "Armed";
String disarmed = "Disarmed";
String invalid = "Invalid Request";

WiFiServer server(80);
IPAddress ip(192, 168, 4, 1);
IPAddress dns(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

void setup()
{
  Serial.begin(115200);
  pinMode(onpin, OUTPUT);
  digitalWrite(onpin, LOW);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.config(ip, dns, gateway, subnet);
  server.begin();
  WiFi.softAP(MBWifi, MBWifiPwd);

}

void loop(){
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
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
    httputils::HTTPRespond(client,disarmed);
  }
  else if (req.indexOf("/arm") != -1)
  {
    digitalWrite(onpin, 1);
    httputils::HTTPRespond(client,armed);
  }
  else if (req.indexOf("/status") != -1)
  {
    if (digitalRead(onpin))
    {
      httputils::HTTPRespond(client,armed);
    }
    else
    {
      httputils::HTTPRespond(client,disarmed);
    }
  }
  else{
    httputils::HTTPRespond(client,invalid);
  }
}
