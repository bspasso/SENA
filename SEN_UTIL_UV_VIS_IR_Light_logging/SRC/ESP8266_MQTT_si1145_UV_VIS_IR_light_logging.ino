/*************************************************** 
  This is a library for the Si1145 UV/IR/Visible Light Sensor

  Designed specifically to work with the Si1145 sensor in the
  adafruit shop
  ----> https://www.adafruit.com/products/1777

  These sensors use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include "Adafruit_SI1145.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "yourSSID_HERE";
const char* password = "yourPASSWORD_HERE";
const char* mqtt_server = "192.168.XXX.YYY";

const int powerbuttonPressed = 700; // number of millisecs 
const int powerbuttonAfterRelease = 400; // number of millisecs 
// ======================================================================

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_SI1145 uv = Adafruit_SI1145();

long lastMsg = 0;
char msg[50], topic_msg[50], topic_send_msg[50], payload_msg[50];

int value = 0, callback_flag = 0;
int toggle_btn_NO = 0, effective_btn_pressed = 0;

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()

float UVindex;
int Vis_Light, IR_Light;
// ======================================================================

void setup() {
//  pinMode(D3,OUTPUT);    
  pinMode(D3,OUTPUT);    
 // pinMode(D4,OUTPUT);    
  pinMode(D5,OUTPUT);    
  pinMode(D6,OUTPUT);    
  
  pinMode(D2,INPUT);  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.println("Adafruit SI1145 test");
  if (! uv.begin()) {
    Serial.println("Didn't find Si1145");
    while (1);
  }
  Serial.println("OK!");
  digitalWrite(BUILTIN_LED, HIGH); 
}

void setup_wifi() {
  delay(10);
// We start by connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
//  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(String(ESP.getChipId()).c_str())) {
      Serial.println("\nConnected, OK!");
      // Once connected, publish an announcement...
      // client.publish("/DEMO02", "hello reconnect!!!!!");
//      client.publish("/CLNT_ID", String(ESP.getChipId()).c_str()); 
      // ... and resubscribe
// ================================================================================
  for (int i = 0; i < 10; i++) {
//    Serial.print((char)payload[i]);
    msg[i] = (char)String(ESP.getChipId()).c_str()[i];
  }
  client.publish("/IAM_HERE", msg);

  digitalWrite(BUILTIN_LED, HIGH); 
  digitalWrite(D3,LOW);   //set the digital output value of pin D3 to LOW = NO CONTACT 
  
  snprintf (msg, 75, "/ID_%s_LISTEN", String(ESP.getChipId()).c_str());
  // client.publish("/", msg);
  client.subscribe(msg);
// ================================================================================
    } else {
//    Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      client.publish("/DEMO", "Wait 5 seconds before retrying!!!!!");
      delay(5000);
    }
  }
}

// ===========================================================
// ===============   S   T    A    R    T   ==================
// ===========================================================
void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
  for (int i = 0; i < 50; i++) {
    payload_msg[i] = NULL;
  }
  for (int i = 0; i < length; i++) {
    payload_msg[i] = (char)payload[i];
  }
  
  snprintf (topic_msg, 75, "/ID_%s_LISTEN", String(ESP.getChipId()).c_str());
  if (strcmp(topic, topic_msg)==0){

    callback_flag = 1;
    toggle_btn_NO = payload_msg[0] - 48; // + 1;
    effective_btn_pressed = toggle_btn_NO;
 
    if (toggle_btn_NO == 0) {
        snprintf (topic_send_msg, 75, "/ID_%ld_REPORT", ESP.getChipId());
      } else {  
      if (callback_flag == 1) {
          callback_flag == 0;
          effective_btn_pressed = toggle_btn_NO;
          toggle_btn_NO = on_off(toggle_btn_NO);  // ******
      }
        snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
    }  

    snprintf (msg, 75, "Button=%ld_UVLight=%2.2f_VisLight=%ld_IRLight=%ld", effective_btn_pressed, UVindex, Vis_Light, IR_Light); 
    client.publish(topic_send_msg, msg);
  }

}

// =====================================================
int on_off(int stat_to_go) {

      switch (stat_to_go) {
        case 1:
            toggle_btn_NO = 1;
                digitalWrite(D3,HIGH);   // set the digital output value of pin D3 to HIGH = YES CONTACT / BEAMING LIGHT FROM THE EXTERNAL LED   
                digitalWrite(BUILTIN_LED, LOW);  // toggle state
                delay(powerbuttonPressed);  // wait around for 7/10 sec (700 ms)
                digitalWrite(D3,LOW);   // set the digital output value of pin D3 to LOW = NO CONTACT / NO LIGHT FROM THE EXTERNAL LED  
                delay(powerbuttonAfterRelease);  // wait around for 3/10 sec (300 ms)    
                digitalWrite(BUILTIN_LED, HIGH);  // toggle state
            break;
        case 2:
            toggle_btn_NO = 2;
                digitalWrite(D5,HIGH);   // set the digital output value of pin D3 to HIGH = YES CONTACT / BEAMING LIGHT FROM THE EXTERNAL LED   
                digitalWrite(BUILTIN_LED, LOW);  // toggle state
                delay(powerbuttonPressed);  // wait around for 7/10 sec (700 ms)
                digitalWrite(D5,LOW);   // set the digital output value of pin D3 to LOW = NO CONTACT / NO LIGHT FROM THE EXTERNAL LED  
                delay(powerbuttonAfterRelease);  // wait around for 3/10 sec (300 ms)    
                digitalWrite(BUILTIN_LED, HIGH);  // toggle state            
            break;
        case 3:
            toggle_btn_NO = 3;
                digitalWrite(D6,HIGH);   // set the digital output value of pin D3 to HIGH = YES CONTACT / BEAMING LIGHT FROM THE EXTERNAL LED   
                digitalWrite(BUILTIN_LED, LOW);  // toggle state
                delay(powerbuttonPressed);  // wait around for 7/10 sec (700 ms)
                digitalWrite(D6,LOW);   // set the digital output value of pin D3 to LOW = NO CONTACT / NO LIGHT FROM THE EXTERNAL LED  
                delay(powerbuttonAfterRelease);  // wait around for 3/10 sec (300 ms)    
                digitalWrite(BUILTIN_LED, HIGH);  // toggle state            
            break;
        default:
            toggle_btn_NO = 0;
            digitalWrite(D3,LOW);   //set the digital output value of pin D3 to that value   
         //   digitalWrite(D4,LOW);   //set the digital output value of pin D4 to that value   
            digitalWrite(D5,LOW);   //set the digital output value of pin D5 to that value   
            digitalWrite(D6,LOW);   //set the digital output value of pin D6 to that value   
            digitalWrite(BUILTIN_LED, HIGH);  // toggle state             
        break;
      }
  
  toggle_btn_NO = 0;
  return toggle_btn_NO;
}

// =====================================================
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  delay(500);
  client.loop();
  
  long now = millis();
  currentMillis = now;   // capture the latest value of millis()
  
  if (now - lastMsg > 500) {
    lastMsg = now;
    ++value;

    Vis_Light = uv.readVisible();
    IR_Light = uv.readIR();
    UVindex = uv.readUV();
      // float UVindex = uv.readUV();
      // the index is multiplied by 100 so to get the
      // integer index, divide by 100!
    UVindex /= 100.0; 

  }
/*
  Serial.println("===================");
  Serial.print("Vis: "); Serial.println(Vis_Light);
  Serial.print("IR: "); Serial.println(IR_Light);
  Serial.print("UV: ");  Serial.println(UVindex);
  delay(1000);
*/  
}

// ============================================================================================   E N D     E N D    E N D     E N D    ========================================================================================

