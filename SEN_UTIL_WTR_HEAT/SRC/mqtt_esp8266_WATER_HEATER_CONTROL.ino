#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>

const double VCC = 3.3;             // NodeMCU on board 3.3v vcc
const double R2 = 10000;            // 10k ohm series resistor
const double adc_resolution = 1023; // 10-bit adc

const double A = 0.001129148;   // thermistor equation parameters
const double B = 0.000234125;
const double C = 0.0000000876741; 

const char* ssid = "yourSSID_HERE";
const char* password = "yourPASSWORD_HERE";
const char* mqtt_server = "192.168.XXX.YYY"; // IP_ADDR of your raspberry pi with MQTT_BROKER_SOFTWARE installed

const int lightsensorInterval = 700; // number of millisecs between light sensor readings
const int powerbuttonPressed = 700; // number of millisecs between light sensor readings
const int powerbuttonAfterRelease = 400; // number of millisecs between light sensor readings
const int MaxNumberOf_LightSensorChecks = 13;
// ======================================================================

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50], topic_msg[50], topic_send_msg[50], payload_msg[50];

int value = 0, light_sensor_check_effective = 0;
int on_off_status = 0, light_status = 0, current_light_status = 0, LastEffective_light_status = 2, interim_light_status, expected_light_status;

unsigned int NumberOf_LightSensorChecks = 0;
unsigned long previousLightSensorMillis = 0; // time when light sensor D2 pin last checked

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()

double Vout = 0, Rth = 0, temperature, adc_value; 
// ======================================================================

void setup() {
  pinMode(D1,OUTPUT);    
  pinMode(D2,INPUT);  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
//      Serial.println("connected");
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
  digitalWrite(D1,LOW);   //set the digital output value of pin D1 to LOW = NO CONTACT / NO LIGHT FROM THE EXTERNAL LED  
  
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
  on_off_status = 0;    
// current_light_status = 0;
// == LISTEN FOR POSSIBLE COMMANDS <=> ACTIONS: 0=REPORT_TEMP and STATUS(ON_OFF); 1=POWER_ON; 2=POWER_OFF  ==
  snprintf (topic_msg, 75, "/ID_%s_LISTEN", String(ESP.getChipId()).c_str());
  if (strcmp(topic, topic_msg)==0){
      switch ((char)payload_msg[0]) {
        case '0':
            on_off_status = 0;
            expected_light_status = 2;
            NumberOf_LightSensorChecks =0;
            light_sensor_check_effective = 0;
            break;
        case '1':
            on_off_status = 1;
            expected_light_status = 1;
            NumberOf_LightSensorChecks = 0;
            light_sensor_check_effective = 1;
            break;
        case '2':
            on_off_status = 2;
            expected_light_status = 0;
            NumberOf_LightSensorChecks = 0;
            light_sensor_check_effective = 1;
            break;
        case '3':
            light_sensor_check_effective = 1;
            NumberOf_LightSensorChecks = 0;
            expected_light_status = 2;
            on_off_status = 0;
            break;
        default:
        break;
      }
         
      if (on_off_status == 0) {
        snprintf (topic_send_msg, 75, "/ID_%ld_REPORT", ESP.getChipId());
      } else {  
        snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
      }  
      snprintf (msg, 75, "TEMP=%2.1f_ONOFF=%ld_LIGHT=%ld", temperature, LastEffective_light_status, digitalRead(D2)); 
      client.publish(topic_send_msg, msg);
  }
  if (strcmp(topic,"/echo")==0) {
  }
}
// =====================================================

void readLightSensor() {
    
     if (millis() - previousLightSensorMillis >= lightsensorInterval) {
        if (NumberOf_LightSensorChecks >  MaxNumberOf_LightSensorChecks) {
            light_sensor_check_effective = 0;
            snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
            snprintf (msg, 75, "NEGATIVE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks);
            client.publish(topic_send_msg, msg);
            light_sensor_check_effective = 0;
            NumberOf_LightSensorChecks = 0;
            previousLightSensorMillis = millis();
            return;
        }
        if ((NumberOf_LightSensorChecks <=  MaxNumberOf_LightSensorChecks) && (light_sensor_check_effective == 1)) {
               
               NumberOf_LightSensorChecks++;
               
               previousLightSensorMillis += lightsensorInterval;

               current_light_status = digitalRead(D2); 

               if (digitalRead(D2) == HIGH) {   //read the digital value on pin D2 ==== 1. "LOW=0" means that NO LIGHT IS AVAILABLE; 2. "HIGH=1" means that WE DO HAVE LIGHT BEAMING TOWARDS LIGHT SENSOR  
                     LastEffective_light_status = 1;            
                     if (expected_light_status == 1) {
                          light_status = current_light_status || light_status;
                          interim_light_status = light_status;
                          
                            if (light_status == expected_light_status) {
                                  LastEffective_light_status = 1;
                                  snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
                                  snprintf (msg, 75, "DONE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks); 
                                  client.publish(topic_send_msg, msg);
                                  light_sensor_check_effective = 0;
                                  light_status = 0;
                                  // return;
                            } else {
                                  yield();
                            }
                      
                     } else {
                          if (NumberOf_LightSensorChecks >  MaxNumberOf_LightSensorChecks) {

                            LastEffective_light_status = 1;                           
                            light_sensor_check_effective = 0;
                            
                            snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
                            snprintf (msg, 75, "NEGATIVE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks); 
                            client.publish(topic_send_msg, msg);
                            // UNSUCESSFULL light check
                            NumberOf_LightSensorChecks = 0;
                            previousLightSensorMillis = millis();
                            // return;
                          }                      
                     }
                     
               } else {
                     
                     LastEffective_light_status = 0;      
                     
                     if (expected_light_status == 0) {
                          light_status = current_light_status || light_status;
                          
                          if (NumberOf_LightSensorChecks == (MaxNumberOf_LightSensorChecks - 4)) {
                            interim_light_status = light_status;
                          }
                                                    
                          if (NumberOf_LightSensorChecks > (MaxNumberOf_LightSensorChecks - 2)) {
                            if ((light_status == expected_light_status) && (expected_light_status == interim_light_status)) {
                                  LastEffective_light_status = 0;
                                  snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
                                  snprintf (msg, 75, "DONE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks); 
                                  client.publish(topic_send_msg, msg);
                                  light_sensor_check_effective = 0;
                                  light_status = 0;
                                  // return;
                            }
                          }
                      
                      
                     } else {
                          if (NumberOf_LightSensorChecks >  MaxNumberOf_LightSensorChecks) {
                            
                            light_sensor_check_effective = 0;
                            LastEffective_light_status = 0;                            
                            snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
                            snprintf (msg, 75, "NEGATIVE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks); 
                            client.publish(topic_send_msg, msg);
                            
                            NumberOf_LightSensorChecks = 0;
                            previousLightSensorMillis = millis();
                          }
                     }
               }
      
        } else {
              if (light_sensor_check_effective == 1) {
                            
                            previousLightSensorMillis += lightsensorInterval;
                            light_sensor_check_effective = 0;
                            
                            snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());
                            snprintf (msg, 75, "NEGATIVE_ONOFF=%ld_CHECKS=%ld", LastEffective_light_status, NumberOf_LightSensorChecks); 
                            client.publish(topic_send_msg, msg);
                            
                            NumberOf_LightSensorChecks = 0;
                            previousLightSensorMillis = millis();
              }
        }  
        // previousLightSensorMillis += lightsensorInterval;
     }
}

//======================================================
int on_off(int stat_to_go) {
  if (stat_to_go == 0) {
    // digitalWrite(BUILTIN_LED, HIGH); 
   digitalWrite(D1,LOW);   //set the digital output value of pin D1 to that value   
   // on_off_status == !on_off_status;
  } else {  
    digitalWrite(D1,HIGH);   // set the digital output value of pin D1 to HIGH = YES CONTACT / BEAMING LIGHT FROM THE EXTERNAL LED   
    // digitalWrite(BUILTIN_LED, !BUILTIN_LED);  // toggle state
    delay(powerbuttonPressed);  // wait around for 7/10 sec (700 ms)
    digitalWrite(D1,LOW);   // set the digital output value of pin D1 to LOW = NO CONTACT / NO LIGHT FROM THE EXTERNAL LED  
    delay(powerbuttonAfterRelease);  // wait around for 3/10 sec (300 ms)    
  
    if (expected_light_status != current_light_status) { 
      light_sensor_check_effective = 1;
    }
    on_off_status = !on_off_status;
  }
  return on_off_status;
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
 
    adc_value = analogRead(A0);
    Vout = (adc_value * VCC) / adc_resolution;
    Rth = (VCC * R2 / Vout) - R2;
   
    temperature = (1 / (A + (B * log(Rth)) + (C * pow((log(Rth)),3))));   // Temperature in kelvin
    temperature = temperature - 273.15;  // Temperature in degree celsius
  
    int reads = digitalRead(D2); //read the digital value on pin D2 ==== 1. "LOW=0" means that NO LIGHT IS AVAILABLE; 2. "HIGH=1" means that WE DO HAVE LIGHT TOWARDS LIGHT SENSOR  
    digitalWrite(BUILTIN_LED, !reads); // BUILTIN_LED = HIGH means BUILTIN_LED is OFF and BUILTIN_LED = LOW means BUILTIN_LED is ON  

    if (expected_light_status < 2) {
          on_off_status = on_off(on_off_status);  // **************************  THE MAIN FUNC *********************************** 
          readLightSensor();  
    }

  }
}

// ============================================================================================   E N D     E N D    E N D     E N D    ========================================================================================

