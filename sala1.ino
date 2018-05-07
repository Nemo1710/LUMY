#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>


#define D0 16
#define D2 4
//#define D3 0
#define D4 2
#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15
//#define D9 3

bool flaginterr = false;
const int interruptPin = 5; //GPIO 0 (Flash Button) 
int8_t ret;
void MQTT_connect();
const char* nameOta = "myespLUMINARIAS";

//char __ssssid[] = "RODRIGO_ROMAN"; //  your network SSID (name)
//char __spasswd[] = "rodrigo1964";    // your network password (use for WPA, or use as key for WEP)

char __ssssid[] = "VILLACIS"; //  your network SSID (name)
char __spasswd[] = "Ambato2019";    // your network password (use for WPA, or use as key for WEP)

//char __ssssid[] = "IOT-IEEE"; //  your network SSID (name)
//char __spasswd[] = "IOTIEEEUNLP";    // your network password (use for WPA, or use as key for WEP)

WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client,"159.203.139.127", 1883);
Adafruit_MQTT_Client mqtt(&client,"192.168.0.90", 1883);

Adafruit_MQTT_Publish focoestado = Adafruit_MQTT_Publish(&mqtt,"home/master/switch");
// Setup a feed called 'time' for subscribing to current time
Adafruit_MQTT_Subscribe foco = Adafruit_MQTT_Subscribe(&mqtt, "home/master/switch1/sette");

//Adafruit_MQTT_Subscribe keepalive = Adafruit_MQTT_Subscribe(&mqtt, "/keepalive", MQTT_QOS_1);

//void keepalive1(double x) {
//  Serial.print("Hey we're in a slider callback, the slider value is: ");
//  Serial.println(x);
//}

void fococallback(char *x, uint16_t len) {
  noInterrupts();
  Serial.println(x);
  //Serial.println(flaginterr);
  
  String mijin=String(x);
  if(mijin=="ON"){
    digitalWrite(D0,HIGH);
    focoestado.publish("ON");
    //Serial.println("envio ON"); 
    
  }else if(mijin=="OFF"){
    digitalWrite(D0,LOW);
    //Serial.println("envio OFF");
    focoestado.publish("OFF");
  }
 interrupts();
}

void handleInterrupt() { 
 // accion momento de una interrupcion
  //Serial.println("Interrupt");
  if (flaginterr == false){
    flaginterr = true;  
  }
  
}

void setup() {
  //noInterrupts();
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE); 
  EEPROM.begin(400);
  Serial.begin(115200);
  pinMode(interruptPin,INPUT); 
  delay(500);
  pinMode(D0,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  digitalWrite(D2,HIGH);
  digitalWrite(D4,LOW);
  digitalWrite(D5,LOW);
//  digitalWrite(10,HIGH);
//  delay(250);
//  digitalWrite(10,LOW);
//  delay(250);
//  digitalWrite(10,HIGH);
//  delay(250);
//  digitalWrite(10,LOW);
//  delay(250);
//  digitalWrite(10,HIGH);

  //attachInterrupt(interruptPin, handleInterrupt, RISING); 
  if (flaginterr == true)
    {
      Accion();
    }
  delay(50);
  if (flaginterr == true)
    {
      Accion();
    }
  Serial.println(F("Iniciando...."));
  //Conectando a red WiFI
  OTA_set();
  wifi_conection();
  //keepalive.setCallback(keepalive1);
  foco.setCallback(fococallback);
  //Setup MQTT subscription for time feed.
  mqtt.subscribe(&foco);
  MQTT_connect(); // una vez conectado a la red WiFi, busca coneccion al servidor MQTT
  
  //interrupts();
  //FIN SETUP
}




void OTA_set(){
  ArduinoOTA.setHostname(nameOta);
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"yotec");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
}


void wifi_conection(){
  
  Serial.println("Conectando a una red WiFi");
  WiFi.begin(__ssssid,__spasswd);
  
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && retries <= 70) 
  {
    retries++;
    digitalWrite(10,LOW);
    //digitalWrite(D2,LOW);
    Serial.print("*");
    WiFi.begin(__ssssid,__spasswd);
    //delay(350);
    //digitalWrite(10,LOW);
    delay(250);
    Serial.print(".");
    if (flaginterr == true)
       {
         Accion();
       }
  }
  
  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println(F("WiFi conectado exitosamente"));
    digitalWrite(10,HIGH);
    digitalWrite(D2,HIGH);
  }
  
  //Serial.println(F("Conectado")); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
 
//FIN WIFI
}



void loop() {
   ArduinoOTA.handle();
// Valida estar conectado a una red WiFI
  mqtt.processPackets(900);
  if (WiFi.status() != WL_CONNECTED)
    { 
      digitalWrite(10,LOW);
      Serial.println("Conectando a red WiFi");
      wifi_conection();
      if(digitalRead(D0) == HIGH){
        focoestado.publish("ON");
      }else if(digitalRead(D0) == LOW){
        focoestado.publish("OFF");
      }
    } 
// Valida estar conectado al servidor MQTT  
  if ((ret = mqtt.connect()) != 0) 
  {
    digitalWrite(D5,LOW);
    Serial.println("Conectando a servidor MQTT");
    MQTT_connect();
    if(digitalRead(D0) == HIGH){
       focoestado.publish("ON");
    }else if(digitalRead(D0) == LOW){
       focoestado.publish("OFF");
    }
  }

// Cambio de estado de switch segun flag de interrupcion
  if (flaginterr == true)
  {
   Accion();
  }

//FIN LOOP
}



void Accion(){
  noInterrupts();
   if (digitalRead(D0) == HIGH){
     //Serial.println("interrupcion rele cambia a abierto");  
     //fococ=false;
     digitalWrite(D0,LOW);
     focoestado.publish("OFF");
     //Serial.println("enviado OFF");
     
    }else if (digitalRead(D0) == LOW){
      //Serial.println("interrupcion rele cambia a cerrado");  
      //fococ=true;
      digitalWrite(D0,HIGH);
      focoestado.publish("ON");
      //Serial.println("enviado ON");
    }
    delay(700);
    flaginterr = false;
    interrupts();
  
}

void MQTT_connect() {
  // Coneccion a servidor MQTT
  //digitalWrite(D2,LOW);
  //int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    Serial.println("Conectado a servidor");
    digitalWrite(D5,HIGH);
    return;
  }

  //digitalWrite(D2,HIGH);
  Serial.print("Conectando a  MQTT... ");
  int retries=10;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       digitalWrite(D4,LOW);
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Reintentado Conexion...");
       mqtt.disconnect();
       if (flaginterr == true)
        {
          Accion();
        }
       delay(2000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
          Serial.println("Validando conexion a WiFi");
          delay(200);
          if (WiFi.status() != WL_CONNECTED)
            { 
              Serial.println("Reconectando a red WiFi");
              wifi_conection();
              
            } else {
              Serial.println("Conexion a wifi estable");
            }
          retries =10;
      }
  }
  Serial.println("Conectado al servidor MQTT!");
  digitalWrite(D5,HIGH);
  digitalWrite(D2,HIGH);
  
}
