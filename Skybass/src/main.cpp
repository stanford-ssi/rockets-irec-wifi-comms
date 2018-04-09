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


const char WiFiAPPSK[] = "redshift";
const int onpin = 5; // IO5 on the Esp8266 WROOM 02
const int stagePin = 7; //CHECK

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    pinMode(onpin, OUTPUT);
    digitalWrite(onpin, LOW);

    WiFi.mode(WIFI_AP);

    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "Thing-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    IPAddress ip(192,168,0,1);
    IPAddress dns(192,168,0,1);
    IPAddress gateway(192,168,0,1);
    IPAddress subnet(255,255,255,0);
    WiFi.config(ip,dns,gateway,subnet);


    //String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
    //                String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    //macID.toUpperCase();
    String AP_NameString = "Skybass";// + macID;

    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);

    for (int i=0; i<AP_NameString.length(); i++)
        AP_NameChar[i] = AP_NameString.charAt(i);

    WiFi.softAP(AP_NameChar, WiFiAPPSK);

    server.begin();
}
String motherboard_ip = "192.168.0.2";
String staging_ip = "192.168.0.3";
void loop() {
  String send_to_teensy ="";



  //connect to motherboard, get Status
  HTTPClient mbHttp;
  mbHttp.begin("http:/"+motherboard_ip+":80/status_s");
  mbHttp.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int mbHttpCode = mbHttp.GET();
  if (mbHttpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // file found at server
      if (mbHttpCode == HTTP_CODE_OK) {
        String payload = mbHttp.getString();
        send_to_teensy +="Motherboard: "+payload;
      }
    } else {
    }

    mbHttp.end();

    HTTPClient stHttp;
    stHttp.begin("http:/"+staging_ip+":80/status_s");
    stHttp.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int stHttpCode = stHttp.GET();
    if (stHttpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        // file found at server
        if (stHttpCode == HTTP_CODE_OK) {
          String payload = stHttp.getString();
          send_to_teensy +="\nStaging: "+payload;
        }
      } else {
      }

      stHttp.end();



//SEND TO TEENSY - send_to_teensy


if(digitalRead(stagePin)==HIGH)
{
  String resp = "";
  HTTPClient stHttp;
  stHttp.begin("http:/"+staging_ip+":80/open_s");
  stHttp.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int stHttpCode = stHttp.GET();
  if (stHttpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // file found at server
      if (stHttpCode == HTTP_CODE_OK) {
        String payload = stHttp.getString();
        resp +="\nStaging: "+payload;
      }
    } else {
    }

    stHttp.end();
    send_to_teensy += resp;
}





// Check if a client has connected
WiFiClient client = server.available();
if (!client) {
  return;
}

// Read the first line of the request
String req = client.readStringUntil('\r');
Serial.println(req);
client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/disarm") != -1)
    val = 0; // Will write LED low
  else if (req.indexOf("/arm") != -1)
    val = 1; // Will write LED high
  else if (req.indexOf("/status") != -1)
    val = -2; // Will print pin reads
  // Otherwise request will be invalid. We'll say as much in HTML




  // Set GPIO5 according to the request
  if (val >= 0)
    digitalWrite(onpin, val);

  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";

  // If we're setting the LED, print out a message saying we did
  if (val == 0)
  {
    s += "<p>Board is now <b>disarmed</b></p>";
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

  s += "</body></html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  delay(1000);
  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}
