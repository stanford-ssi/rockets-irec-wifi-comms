
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

#include "min.h"
#include "min.c"

//port 0 is skybass serial
//port 1 is payload client
struct min_context skyb_ctx;
struct min_context payload_ctx;

const char WiFiAPPSK[] = "redshift";
const char APName[] = "Skybass";
const int onpin = 4; // IO5 on the Esp8266 WROOM 02
const int ledpin = 5;
String payload_ip = "192.168.4.2";
String a = "Armed";
String d = "Disarmed";
WiFiServer server(80);
WiFiClient client;
uint8_t skyb_data;
uint16_t min_tx_space(uint8_t port)
{
  // Ignore 'port' because we have just one context. But in a bigger application
  // with multiple ports we could make an array indexed by port to select the serial
  // port we need to use.
  uint16_t n =-1;
  switch(port)
  {

    case 0:
    n=Serial.available();
    break;
    case 1:
    n=client.available();
    break;

  }

  return n;
}

void min_tx_byte(uint8_t port, uint8_t byte)
{
  switch(port)
  {
    case 0:
    Serial.write(&byte,1U);
    break;
    case 1:
    client.write(&byte, 1U);
    break;

  }

}
uint32_t min_time_ms(void)
{
  return millis();
}


void min_application_handler(uint8_t min_id, uint8_t *min_payload, uint8_t len_payload, uint8_t port)
{
  // In this simple example application we just echo the frame back when we get one, with the MIN ID
  // one more than the incoming frame.
  //
  // We ignore the port because we have one context, but we could use it to index an array of
  // contexts in a bigger application.
  uint32_t now = millis();
  switch(port)
  {
    case 0:
    //RECVD FROM SKYBASS
      memcpy(&skyb_data, min_payload, len_payload);
      if(skyb_data == 0xAA)
      {
        digitalWrite(ledpin, LOW);
        if(!min_queue_frame(&payload_ctx, 0x33U, (uint8_t *)&now, 4U))
        {
          // The queue has overflowed for some reason
          Serial.print("Can't queue at time ");
          Serial.println(millis());
        }
        //String pr = send_request(payload_ip, "arm");
        //out += "Payload: " + pr;
      }
      if(skyb_data == 0xAB)
      {
        digitalWrite(ledpin, LOW);
        if(!min_queue_frame(&payload_ctx, 0x33U, (uint8_t *)&now, 4U))
        {
          // The queue has overflowed for some reason
          Serial.print("Can't queue at time ");
          Serial.println(millis());
        }
        //String pr = send_request(payload_ip, "arm");
        //out += "Payload: " + pr;
      }
    break;

    case 1:
    //STATUS OF PAYLOAD
    break;

  }
//  Serial.print("MIN frame with ID ");
//  Serial.print(min_id);
//  Serial.print(" received at ");
//  Serial.println(millis());
//  min_id++;
  // The frame echoed back doesn't go through the transport protocol: it's send back directly
  // as a datagram (and could be lost if there were noise on the serial line).
  //min_send_frame(&min_ctx, min_id, min_payload, len_payload);
}

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
  min_init_context(&payload_ctx, 0);
  min_init_context(&skyb_ctx, 0);
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


    char buf1[32];
    size_t buf_len1;

    // Read some bytes from the USB serial port..
    if(Serial.available() > 0) {
      buf_len1 = Serial.readBytes(buf1, 32U);
    }
    else {
      buf_len1 = 0;
    }
    // .. and push them into MIN. It doesn't matter if the bytes are read in one by
    // one or in a chunk (other than for efficiency) so this can match the way in which
    // serial handling is done (e.g. in some systems the serial port hardware register could
    // be polled and a byte pushed into MIN as it arrives).
    min_poll(&skyb_ctx, (uint8_t *)buf1, (uint8_t)buf_len1);

    char buf2[32];
    size_t buf_len2;

    // Read some bytes from the USB serial port..
    if(client.available() > 0) {
      buf_len2 = client.readBytes(buf2, 32U);
    }
    else {
      buf_len2 = 0;
    }
    // .. and push them into MIN. It doesn't matter if the bytes are read in one by
    // one or in a chunk (other than for efficiency) so this can match the way in which
    // serial handling is done (e.g. in some systems the serial port hardware register could
    // be polled and a byte pushed into MIN as it arrives).
    min_poll(&payload_ctx, (uint8_t *)buf2, (uint8_t)buf_len2);



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
