#include <Wire.h>
#include "math.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Mitsubishi.h>

#include "DHT.h"

#define DHTPIN 5     // = D1 what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

DHT dht(DHTPIN, DHTTYPE, 35);

#define IR_LED 4  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRMitsubishiAC mitsubir(IR_LED);  // Set the GPIO used for sending messages.

const char* ssid = "yourSSID_HERE";
const char* password = "yourPASSWORD_HERE";
const char* mqtt_server = "192.168.XXX.YYY";

const int powerbuttonPressed = 700; // number of millisecs 
const int powerbuttonAfterRelease = 400; // number of millisecs 
const int max_timeAirConditionerCommandToBeSent = 30; // 30 multiplied by 2 seconds = 60 secs
// ======================================================================

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50], topic_msg[50], topic_send_msg[50], payload_msg[50];

int value = 0, callback_flag = 0;
int tobeMode = 0;

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
// int timeSinceLastRead = 0;
int timeAirConditionerCommandToBeSent = 0;

int tobeTemp = 0, tobeFanSpeed = 4, PersistYes = 0;

float h, t, f, hic, hif, dew;
// ======================================================================

void setup() {

    pinMode(DHTPIN, OUTPUT);     
   digitalWrite(DHTPIN, HIGH);
   delayMicroseconds(40);
    // Now start reading the data line to get the value from the DHT sensor.
    pinMode(DHTPIN, INPUT_PULLUP);
    delayMicroseconds(50);  // Delay a bit to let sensor pull data line low. - adjusted from 10 us

    
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  mitsubir.begin();

  Serial.begin(19200);
  delay(200);

  Serial.println("DHT22 test!");
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

// ======================  DHT SETUP =======================
  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");
  dht.begin();
// ======================  DHT SETUP END =======================  
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.println("OK!");
  digitalWrite(BUILTIN_LED, HIGH); 
  
// ==================   MITSU SETUP ================================
  // Set up what we want to send. See ir_Mitsubishi.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  mitsubir.off();
  mitsubir.setFan(2);
  // mitsubir.setFan(5);  
  mitsubir.setMode(MITSUBISHI_AC_AUTO);
  mitsubir.setTemp(22);
  mitsubir.setVane(MITSUBISHI_AC_VANE_AUTO);
// ==================   MITSU SETUP END ================================

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
 
    tobeMode = payload_msg[0] - 48; // + 1;
    tobeFanSpeed = payload_msg[1] - 48; 
    
    if (tobeFanSpeed == 9) {
        PersistYes = 1;
        tobeMode = 0;
        tobeFanSpeed = 0;
    }

    if (tobeFanSpeed == 8) {
        PersistYes = 0;
        tobeMode = 0;
        tobeFanSpeed = 0;
    }
    
    if ((tobeMode == 0) && (tobeFanSpeed == 0)) {
        Serial.println(" ====+++++++++==== CURRENT state of the remote. ======++++++++++===== ");
        printState();
        delay(500);
        // un-comment the above 3 lines for testing purposes 
        
        snprintf (topic_send_msg, 75, "/ID_%ld_REPORT", ESP.getChipId());
      } else {  
        callback_flag = 1;
        tobeTemp = (payload_msg[2] - 48)*10 +  (payload_msg[3] - 48); 

        // ==================   MITSU NEW SETUP ================================
        // Set up what we want to send. See ir_Mitsubishi.cpp for all the options.
        Serial.println("Default state of the remote.");
        printState();

        Serial.println(" ===========>   Setting desired state for A/C. <===============");
      
        switch (tobeMode) {
        case 0:
            if (tobeFanSpeed == 1) {
                mitsubir.off();
                mitsubir.setMode(MITSUBISHI_AC_AUTO);
            }
            break;
        case 1:
            mitsubir.on();
            mitsubir.setMode(MITSUBISHI_AC_AUTO);
            break;
        case 2:
            mitsubir.on();
            mitsubir.setMode(MITSUBISHI_AC_HEAT);
            break;
        case 3:
            mitsubir.on();
            mitsubir.setMode(MITSUBISHI_AC_DRY);
            break;
        case 4:
            mitsubir.on();
            mitsubir.setMode(MITSUBISHI_AC_COOL);
            break;            
        default:
            mitsubir.off();
            mitsubir.setMode(MITSUBISHI_AC_AUTO);
        }

        // mitsubir.setFan(5);  
        if (tobeTemp > 0) {
            mitsubir.setFan(tobeFanSpeed);
            mitsubir.setTemp(tobeTemp);
        }
        mitsubir.setVane(MITSUBISHI_AC_VANE_AUTO);
        // ==================   MITSU SETUP END ================================

        Serial.printf("tobeMode: %d  tobeFanSpeed: %d  tobeTemp: %d\n", tobeMode, tobeFanSpeed, tobeTemp);    
        
        // Now send the IR signal.
        #if SEND_MITSUBISHI_AC
          Serial.println("=====>>>>>>> Sending IR command to A/C ...");
          mitsubir.send();
        #endif  // SEND_MITSUBISHI_AC
        
        Serial.println(" ========== NEW state of the remote. ================= ");
        printState();
        delay(500);
        // un-comment the above for testing purposes
        // ==================   MITSU CMD END ================================      

        snprintf (topic_send_msg, 75, "/ID_%ld_TOGGLE", ESP.getChipId());

        digitalWrite(BUILTIN_LED, LOW);  // toggle state
        delay(300);  // wait around for 3/10 sec (300 ms)
        digitalWrite(BUILTIN_LED, HIGH);  // toggle state       
    }  
 
    // Serial.printf("TempC=%2.2f_Humudity=%2.2f_Hic=%2.2f_Dew=%2.2f \n", t, h, hic, dew);
    snprintf (msg, 75, "TempC=%2.2f_Humudity=%2.2f_Hic=%2.2f_Dew=%2.2f \n", t, h, hic, dew); 
    client.publish(topic_send_msg, msg);
    
  } else callback_flag = 0;

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
  
  if (now - lastMsg > 1800) {  // two seconds between checks of DHT22-temp sensor, etc..
    lastMsg = now;
    ++value;
// =========================================================================================
    if (timeAirConditionerCommandToBeSent == 4) {
      if (callback_flag == 1) {
          // ==== MITSU CMD  =====
          // Now send the IR signal.
          #if SEND_MITSUBISHI_AC
            Serial.println(" +++++++++++++++++++++++ Sending IR command to A/C ... ++++++++++++++++++++++++");
            mitsubir.send();
          #endif  // SEND_MITSUBISHI_AC
          printState();
          delay(500);
          // un-comment the above for testing purposes
          // ==== MITSU LOOP END ===== 
      }
      
      if (PersistYes == 0) {
          callback_flag = 0;
      } else {
          callback_flag = 1;
      }
      
    } 

    if (timeAirConditionerCommandToBeSent >= max_timeAirConditionerCommandToBeSent) {
      timeAirConditionerCommandToBeSent = 0;
    } else ++timeAirConditionerCommandToBeSent;

// ==========================================================================================
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    delay(500);
    // Read temperature as Celsius (the default)
    t = dht.readTemperature();
    delay(500);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      // timeSinceLastRead = 0;
      delay(300);
      return;
    }

    // Compute heat index in Celsius (isFahreheit = false)
    hic = dht.computeHeatIndex(t, h, false);
    dew = dewPoint(t, h);

    // Serial.println("===================");
    // Serial.printf("TempC=%2.2f_Humudity=%2.2f_Hic=%2.2f_Dew=%2.2f Call_Back=%d PersistYes=%d \n", t, h, hic, dew, callback_flag, PersistYes);
    // Serial.printf("TempC=%2.2f_Humudity=%2.2f_Hic=%2.2f_Dew=%2.2f \n", t, h, hic, dew);
    delay(200);
  }
  delay(300);
}



// ============================================================================================   E N D     E N D    E N D     E N D    ========================================================================================
void printState() {
  // Display the settings.
  // Serial.println("Mitsubishi A/C remote is in the following state:");
  Serial.printf("  Power: %d,  Mode: %d, Temp: %dC, Fan Speed: %d," \
                    " Vane Mode: %d\n",
                mitsubir.getPower(), mitsubir.getMode(), mitsubir.getTemp(),
                mitsubir.getFan(), mitsubir.getVane());
  // Display the encoded IR sequence.
  unsigned char* ir_code = mitsubir.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < MITSUBISHI_AC_STATE_LENGTH; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();
}


// John Main added dewpoint code from : http://playground.arduino.cc/main/DHT11Lib
// Also added DegC output for Heat Index.
// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//

double dewPoint(double celsius, double humidity)
{
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

  // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

  // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}




