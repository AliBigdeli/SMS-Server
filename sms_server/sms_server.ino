/*
 * author: alibigdeli
 * website: icc-aria.ir
 * linkedin: Ali Bigdeli
 * email: bigdeli.ali3@gmail.com
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServerSecure.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

// varibales to control the service
const char* ssid = "";
const char* password = "";
const char* HostName = "Aria-SMS-Gateway";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

// creating serial object
SoftwareSerial mySerial(D5, D6);

// creating http updater object
ESP8266HTTPUpdateServer httpUpdater;
ESP8266WebServer httpServer(80);

MDNSResponder mdns;

// serail communicator
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}


//function to send sms
void Send_SMS(String number,String msg) { 
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  String send_to = "AT+CMGS=\""+number+"\"";
  mySerial.println(send_to);//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.println(msg); //text 
  updateSerial();
  Serial.println("message sent");
  mySerial.write(26); 
}


void handleRoot() {
 Serial.println("You called root page");
 httpServer.send(200,"text/html","root page");
}


void test(){
  String data = httpServer.arg("plain");
  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);
  String number = jObject["number"];
  String message = jObject["message"];
  Serial.println(number);
  Serial.println(message);
  Send_SMS(number,message);
  httpServer.send(200,"/plain","Message Sent");
}

void setup(void){    
  Serial.begin(9600);
  mySerial.begin(9600);  
  Serial.println("Initializing...");
  delay(1000);  
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  delay(1000);
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  delay(1000);
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  delay(1000);
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  
  Serial.println("====================================");
  Serial.println("|          Web SMS Server          |");
  Serial.println("|        Author: Ali Bigdeli       |");
  Serial.println("|        Known as: BlackFox        |");
  Serial.println("|        version: 1.1.alpha        |");
  Serial.println("====================================");
  Serial.println("***        Server starting       ***");      
  
  //If connection successful show IP address in serial monitor  
  Serial.println("");
  Serial.println("");
  Serial.print(" Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP  
  
  // handeling server urls
  httpServer.on("/", handleRoot);    
  httpServer.on("/plain",test);    
  if(mdns.begin(HostName,WiFi.localIP())){    
    Serial.print(" web address: http://");    
    Serial.print(HostName);
    Serial.print(".local");
    Serial.println("");    
  }
  
  // handeling updater
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  
  MDNS.addService("http","tcp",80);    
  
}
  void loop(void){
    updateSerial();
    httpServer.handleClient();    
  }
