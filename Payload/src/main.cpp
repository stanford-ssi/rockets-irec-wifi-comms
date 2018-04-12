/*
  ESP WiFi Arming Code Rev 01
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
#include <ESP8266HTTPClient.h>

const char SkybassAP[] = "Skybass";
const char WiFiAPPSK[] = "redshift";
const int onpin = 5; // IO5 on the Esp8266 WROOM 02
String a = "Armed";
String d = "Disarmed";

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    pinMode(onpin, OUTPUT);
    digitalWrite(onpin, LOW);
    WiFi.mode(WIFI_STA);
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    IPAddress ip(192,168,4,4);
    IPAddress dns(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,dns,gateway,subnet);
    server.begin();
    WiFi.begin(SkybassAP,WiFiAPPSK);
}

void loop() {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: "+req); //DBG
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
  else if (req.indexOf("/status") != -1)
  {
    if(digitalRead(onpin))
    {
      resp=a;
    }
    else
    {
      resp= d;
    }
  }
  client.flush();
  // Prepare the response. Start with the common header:
  //String s = "HTTP/1.1 200 OK\r\n";
  //s += "Content-Type: text/html\r\n\r\n";
  //s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  //s += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";
  // If we're setting the LED, print out a message saying we did
  //if (val == 0)
  //{
  /*  s += "<p>Board is now <b>disarmed</b></p>";
  }
  else if (val == 1)
  {
    s += "<p>Board is now <b>armed!</b></p>";
  }
  else if (val == -2)
  {
    // s += "<br>"; // Go to the next line.
    s += "<p>Status of output pin: ";
    if(digitalRead(onpin))
    {
      s += "<b>armed!</b></p>";
    }
    else
    {
      s += "<b>disarmed.</b></p>";
    }
  }
  else
  {
    s += "<p>Invalid Request.<br> Try /disarm, /arm, or /status.</p>";
  }
  if(req.indexOf("_s")!=-1)
  { //clean out HTML tags for Skybass
    s.replaceAll("<b>","");
    s.replaceAll("</b>","");
    s.replaceAll("<p>","");
    s.replaceAll("</p>","");
    s.replaceAll("HTTP/1.1 200 OK","");
    s.replaceAll("<!DOCTYPE HTML>","");
    s.replaceAll("<html>","");
    s.replaceAll("<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>","");
  }
  else
  { //continue normally
    s += "</body></html>\n";
  }
*/
  // Send the response to the client
  client.print(resp);
  delay(1);
  Serial.println("Client disconnected"); //DBG

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}