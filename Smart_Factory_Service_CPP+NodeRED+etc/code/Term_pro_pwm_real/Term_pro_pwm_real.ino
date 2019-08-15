#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "iotLabUtil.h"

ESP8266WebServer    webServer(80);
WiFiClientSecure    sslClient;
const char*         fingerprint = "16 51 E3 C2 67 C6 AD 23 9C 1A 70 5C 22 A3 B8 C1 7B 7C A6 1D";
PubSubClient        mqClient(sslClient);

char                iot_server[200];

long                pubInterval;
long                lastPublishMillis = 0;
int a = 0;
const char*         publishTopic  = "iot-2/evt/status/fmt/json";
const char*         commandTopic  = "iot-2/cmd/+/fmt/+";
const char*         responseTopic = "iotdm-1/response";
const char*         manageTopic   = "iotdevice-1/mgmt/manage";
const char*         updateTopic   = "iotdm-1/device/update";
const char*         rebootTopic   = "iotdm-1/mgmt/initiate/device/reboot";
const char*         resetTopic    = "iotdm-1/mgmt/initiate/device/factory_reset";
const char*         switchTopic     = "iot-2/evt/switch/fmt/json";
const char*         TempTopic     = "iot-2/evt/slider/fmt/json";
const char*         TampTopic_cmd     = "iot-2/cmd/slider/fmt/json";

char TempBuffer[512];
StaticJsonBuffer<512> jsonOutBuffer;
JsonObject& JSON = jsonOutBuffer.createObject();
JsonObject& d = JSON.createNestedObject("d");

String initHTML = ""
    "<html><head><title>IOT Device Setup</title></head>"
    "<body><center><h1>Device Setup Page</h1>"
        "<style>"
            "input {font-size:3em; width:90%; text-align:center;}"
            "button { border:0;border-radius:0.3rem;background-color:#1fa3ec;"
                "color:#fff; line-height:2em;font-size:3em;width:90%;}"
        "</style>"
        "<form action='/save'>"
            "<p><input type='text' name='ssid' placeholder='SSID'>"
            "<p><input type='text' name='w_pw'placeholder='password'>"
            "<p><input type='text' name='org'placeholder='Org Id'>"
            "<p><input type='text' name='devType'placeholder='Device Type'>"
            "<p><input type='text' name='devId'placeholder='Device Id'>"
            "<p><input type='text' name='token'placeholder='Auth Token'>"
            "<p><input type='text' name='pInterval'placeholder='Publish Interval'>"
            "<p><button type='submit'>Save</button>"
        "</form>"
    "</center></body></html>";

void setup() {
    Serial.begin(115200);
    eeprom_begin();
    chkFactoryReset();
    pinMode(13,OUTPUT);
    pinMode(12,OUTPUT);
    init_config_json();
    JsonObject& cfg = *ptr_config;
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
         configDevice();
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nDevice is ready on IP address : "); Serial.println(WiFi.localIP()); 

    sprintf(iot_server, "%s.messaging.internetofthings.ibmcloud.com", (const char*)cfg["org"]);
    if (!sslClient.connect(iot_server, 8883)) {
        Serial.println("connection failed");
        return;
    }
    if (!sslClient.verify(fingerprint, iot_server)) {
        Serial.println("certificate doesn't match");
        return;
    }
    
    mqClient.setServer(iot_server, 8883);   //IOT
    mqClient.setCallback(msgHandler);
    reconnect();
    
    JsonObject& meta = cfg["meta"];
    pubInterval = meta.containsKey("PubInterval") ? meta["PubInterval"] : 0;
    
}

void loop() {
    // main loop
    if (!mqClient.connected()) {
        reconnect();
    }
    
    if(a == 0){
        digitalWrite(13,HIGH);
        digitalWrite(12,LOW);
        analogWrite(14,0);
    }
    else if(a == 1) {
        digitalWrite(13,HIGH);
        digitalWrite(12,LOW);
        analogWrite(14,500);
    }
    else if(a == 2){
       digitalWrite(13,HIGH);
       digitalWrite(12,LOW);
       analogWrite(14,700);
    }
    else if(a == 3){
       digitalWrite(13,HIGH);
       digitalWrite(12,LOW);
       analogWrite(14,1023);
        
    }
    else{
       digitalWrite(13,HIGH);
       digitalWrite(12,LOW);
       analogWrite(14,1023);
    }
    mqClient.loop();
}

void msgHandler(char* topic, byte* payload, unsigned int payloadLength) {   
    if (!strcmp(responseTopic, topic)) {                                  // strcmp return 0 if both string matches
        Serial.println("Process Response");
        return;                                                             // just print of response for now 
    } else if (!strcmp(rebootTopic, topic)) {
        Serial.println("Rebooting...");
        WiFi.disconnect();
        ESP.restart();
    } else if (!strcmp(resetTopic, topic)) {
        // factory reset
        ptr_config = &(jsonConfigBuffer.parseObject("{\"config\":\"\",\"meta\":{}}"));
        save_config_json();
        WiFi.disconnect();
        ESP.restart();
    } else if (!strcmp(updateTopic, topic)) {
        handleUpdate(payload); 
    } else if (!strncmp(commandTopic, topic, 10)) {
        if (!strcmp(TampTopic_cmd, topic)){
            char temp[15];
            for(int i = 0; i < payloadLength; i++)
                temp[i] = payload[i];
            temp[payloadLength+1] = '\0';
            a = atoi(temp); 
            Serial.println(a);
        }
        else{
            handleCommand(payload, payloadLength);
        }
    } 
}

void handleCommand(byte* payload, unsigned int msgLen) {
    // command handle logic here
}

void reconnect() {
    char idBuf[100];
    
    JsonObject& cfg = *ptr_config;

    while (!mqClient.connected()) {
        sprintf(idBuf, "d:%s:%s:%s", (const char*)cfg["org"], (const char*)cfg["devType"], (const char*)cfg["devId"]);
        Serial.print("Attempting MQTT connection...");
        if (mqClient.connect(idBuf,"use-token-auth",(const char*)cfg["token"])) {
            Serial.println("connected");
        } else {
            Serial.printf("failed(rc=%d), try again in 5 seconds\n", mqClient.state());
            delay(5000);
        }
    }
    
    mqClient.subscribe(responseTopic);
    mqClient.subscribe(commandTopic);
    mqClient.subscribe(rebootTopic);
    mqClient.subscribe(resetTopic);
    mqClient.subscribe(updateTopic);
    
    StaticJsonBuffer<512> jsonOutBuffer;
    JsonObject& meta = cfg["meta"];
    JsonObject& root = jsonOutBuffer.createObject();
    JsonObject& d = root.createNestedObject("d");
    JsonObject& metadata = d.createNestedObject("metadata");
    for (JsonObject::iterator it=meta.begin(); it!=meta.end(); ++it) {
        metadata[String(it->key)] = it->value.asString();
    }
    
    
    JsonObject& supports = d.createNestedObject("supports");
    supports["deviceActions"] = true;

    char msgBuffer[512];
    root.printTo(msgBuffer, sizeof(msgBuffer));
    Serial.println("publishing device metadata:"); Serial.println(msgBuffer);
    if (mqClient.publish(manageTopic, msgBuffer)) {
        d.printTo(msgBuffer, sizeof(msgBuffer));
        String info = String("{\"info\":") + String(msgBuffer) + String("}");
        mqClient.publish(publishTopic, info.c_str());
        
        
    } else {
        Serial.print("device Publish failed:");
    }
}

void configDevice() {
    DNSServer   dnsServer;
    const byte  DNS_PORT = 53;
    IPAddress   apIP(192, 168, 1, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    char ap_name[100];
    sprintf(ap_name, "jsm_%08X", ESP.getChipId());
    WiFi.softAP(ap_name);
    dnsServer.start(DNS_PORT, "*", apIP);

    webServer.on("/save", saveEnv);
    webServer.onNotFound([]() {
        webServer.send(200, "text/html", initHTML);
    });
    webServer.begin();
    Serial.println("starting the config");
    while(1) {
        yield();
        dnsServer.processNextRequest();
        webServer.handleClient();
    }
}

void saveEnv() {
    JsonObject& cfg = *ptr_config;

    cfg["ssid"] = webServer.arg("ssid");
    cfg["w_pw"] = webServer.arg("w_pw");
    cfg["org"] = webServer.arg("org");
    cfg["devType"] = webServer.arg("devType");
    cfg["devId"] = webServer.arg("devId");
    cfg["token"] = webServer.arg("token");
    cfg.createNestedObject("meta");
    cfg["meta"]["PubInterval"] = webServer.arg("pInterval");

    save_config_json();
    webServer.send(200, "text/html", "ok");
    ESP.restart();
}

void handleUpdate(byte* payload) {
  JsonObject& cfg = *ptr_config;
  StaticJsonBuffer<512> jsonInBuffer;
  
  JsonObject& root = jsonInBuffer.parseObject((char*)payload);

  Serial.println("handleUpdate payload:"); 
  root.prettyPrintTo(Serial); Serial.println();
  JsonObject& d = root["d"];
  JsonArray& fields = d["fields"];
  for(JsonArray::iterator it=fields.begin(); it!=fields.end(); ++it) {
    JsonObject& field = *it;
    const char* fieldName = field["field"];
    if (strcmp (fieldName, "metadata") == 0) {
      JsonObject& fieldValue = field["value"];
      cfg.remove("meta");
      JsonObject& meta = cfg.createNestedObject("meta");
      for (JsonObject::iterator fv=fieldValue.begin(); fv!=fieldValue.end(); ++fv) {
        meta[String(fv->key)] = fv->value.asString();
      }
      pubInterval = meta["PubInterval"];
    }
  }
  save_config_json();
}
