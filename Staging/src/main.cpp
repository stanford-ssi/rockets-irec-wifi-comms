#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// The number of steps per revolution
// The number of steps per revolution
const int stepsPerRev = 200;
const int stepsNorm = 100;
const int stepsInc = 20;
const int stepsBaby = 5;
const int DELAY_TIME = 5900;
const int ledpin = 0;
const int a1 = 4;
const int a2 = 13;
const int b1 = 12;
const int b2 = 14;
const int enable = 2;
const int avwire = 5;
const char SkybassAP[] = "Skybass";
const char WiFiAPPSK[] = "redshift";
int stepstatus = 0;
int holdstatus = 0;
bool staged = false;

WiFiServer server(80);


void stepMotor(int numSteps) {
  int stepNumber = 0;
  if (numSteps > 0) {
    while (stepNumber <= numSteps) {
      if(stepNumber%100 == 0){
         ESP.wdtFeed();
      }
      digitalWrite(a1, HIGH);
      digitalWrite(a2, LOW);
      digitalWrite(b1, HIGH);
      digitalWrite(b2, LOW);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, LOW);
      digitalWrite(a2, HIGH);
      digitalWrite(b1, HIGH);
      digitalWrite(b2, LOW);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, LOW);
      digitalWrite(a2, HIGH);
      digitalWrite(b1, LOW);
      digitalWrite(b2, HIGH);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, HIGH);
      digitalWrite(a2, LOW);
      digitalWrite(b1, LOW);
      digitalWrite(b2, HIGH);
      delayMicroseconds(DELAY_TIME);
      stepNumber++;
    }
  } else {
    numSteps = -numSteps;
    while (stepNumber <= numSteps) {
      if(stepNumber%100 == 0){
         ESP.wdtFeed();
      }
      digitalWrite(a1, HIGH);
      digitalWrite(a2, LOW);
      digitalWrite(b1, LOW);
      digitalWrite(b2, HIGH);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, LOW);
      digitalWrite(a2, HIGH);
      digitalWrite(b1, LOW);
      digitalWrite(b2, HIGH);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, LOW);
      digitalWrite(a2, HIGH);
      digitalWrite(b1, HIGH);
      digitalWrite(b2, LOW);
      delayMicroseconds(DELAY_TIME);
      digitalWrite(a1, HIGH);
      digitalWrite(a2, LOW);
      digitalWrite(b1, HIGH);
      digitalWrite(b2, LOW);
      delayMicroseconds(DELAY_TIME);
      stepNumber++;
    }
  }
}

void openStepper() {
  stepMotor(stepsNorm);
  staged = true;
}
void closeStepper() {
  stepMotor(-stepsNorm);
  staged = false;
}
void holdStepper() {
  ESP.wdtFeed();
  digitalWrite(a1, LOW);
  digitalWrite(a2, HIGH);
  digitalWrite(b1, LOW);
  digitalWrite(b2, HIGH);
}
void releaseStepper() {
  ESP.wdtFeed();
  digitalWrite(a1, LOW);
  digitalWrite(a2, LOW);
  digitalWrite(b1, LOW);
  digitalWrite(b2, LOW);
}
void tightenStepper() {
  stepMotor(-stepsInc);
}
void loosenStepper() {
  stepMotor(stepsInc);
}
void babyStep() {
  stepMotor(-stepsBaby);
}

void setup() {
  pinMode(ledpin, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT);
  pinMode(b1, OUTPUT);
  pinMode(b2, OUTPUT);
  pinMode(avwire, INPUT);

  digitalWrite(enable, HIGH);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  IPAddress ip(192, 168, 4, 3);
  IPAddress dns(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip,dns,gateway,subnet);
  //WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.begin(SkybassAP, WiFiAPPSK);
  server.begin();
  Serial.println("Ready");
}

void loop() {

  // Check if the wired signal is telling us to stage
  if(digitalRead(avwire) == HIGH && staged != true){
    ESP.wdtDisable();
    openStepper();
    ESP.wdtEnable(1000);
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println("Recvd Request: " + req);
  client.flush();
  String resp = "";

  /*  String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";
    s += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";
    // Match the request
    int val = -1; // We'll use 'val' to keep track of both the
                  // request type (read/set) and value if set.
  */

  if (req.indexOf("/open") != -1) {
    openStepper();
    resp += "Stepper opened";
    stepstatus = 1;
  } else if (req.indexOf("/close") != -1) {
    closeStepper();
    resp += "Stepper closed";
    stepstatus = 0;
  } else if (req.indexOf("/status") != -1){
    resp += "Stepper status: ";
    if (stepstatus == 1) {
      resp += "opened, ";
    } else {
      resp += "closed, ";
    }
    if (holdstatus == 1) {
      resp += "holding";
    } else {
      resp += "not holding";
    }
  } else if (req.indexOf("/hold") != -1) {
    holdStepper();
    holdstatus = 1;
    resp += "Stepper holding";
  } else if (req.indexOf("/release") != -1) {
    releaseStepper();
    holdstatus = 0;
    resp += "Stepper released";
  } else if (req.indexOf("/tighten") != -1) {
    tightenStepper();
    resp += "Stepper tightened";
  } else if (req.indexOf("/loosen") != -1) {
    loosenStepper();
    resp += "Stepper loosened";
  } else if (req.indexOf("/babystep") != -1){
    babyStep();
    resp += "Stepper BABY STEPPED!!!!";
  }

  client.flush();
  /*
    if (val == 0) {
      s += "<p>Stepper is now <b>opened!</b></p>";
    } else if (val == 1) {
      s += "<p>Stepper is now <b>closed</b></p>";
    } else if (val == 2) {
      s += "<p>Status of stepper: ";
      if(stepstatus == 1) {
        s += "<b>opened and ";
      } else {
        s += "<b>closed and ";
      }
      if(holdstatus == 1){
        s += "holding.</b></p>";
      } else {
        s += "not holding.</b></p>";
      }
    } else if (val==3){
      s += "<p>Stepper is now <b>holding!</b></p>";
    } else if (val == 4){
      s += "<p>Stepper is now <b>not holding.</b></p>";
    } else if (val==5){
      s += "<p>Just <b>tightened</b> stepper</p>";
    } else if (val==6){
      s+= "<p>Just <b>loosened</b> stepper</p>";
    } else {
      s += "<p>Invalid Request.<br> Try /open, /close, /hold, /release, /tighten, /loosen, or /status.</p>";
    }
    /*
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
