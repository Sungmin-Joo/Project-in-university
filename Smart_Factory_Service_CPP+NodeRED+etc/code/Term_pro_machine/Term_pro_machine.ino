#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "iotLabUtil.h"
#include <Wire.h>
#include <OLED32.h>
#include <DHT.h> 


#define             DHTTYPE DHT22
#define             DHTPIN  14
DHT                 dht(DHTPIN, DHTTYPE, 11);
char                dht_buffer[10];
float               humidity;
float               temp_f;
unsigned long       lastDHTReadMillis = 0;
#define             interval  2000
#define             pushSW  2
const int trigPin = 12;    //Trig 핀 할당
const int echoPin = 13;    //Echo 핀 할당
#define             RELAY   15
int run_flag = 0;
OLED display(4,5);
int                 kill_flag = 0;
// will store last temp was read

ESP8266WebServer    webServer(80);
WiFiClientSecure    sslClient;
const char*         fingerprint = "16 51 E3 C2 67 C6 AD 23 9C 1A 70 5C 22 A3 B8 C1 7B 7C A6 1D";
PubSubClient        mqClient(sslClient);

char                iot_server[200];
long                pubInterval;
long                lastPublishMillis = 0;

const char*         publishTopic  = "iot-2/evt/status/fmt/json";
const char*         commandTopic  = "iot-2/cmd/+/fmt/+";
const char*         responseTopic = "iotdm-1/response";
const char*         manageTopic   = "iotdevice-1/mgmt/manage";
const char*         updateTopic   = "iotdm-1/device/update";
const char*         rebootTopic   = "iotdm-1/mgmt/initiate/device/reboot";
const char*         resetTopic   = "iotdm-1/mgmt/initiate/device/factory_reset";
const char*         switchTopic   = "iot-2/evt/switch/fmt/json";
const char*         tempTopic   = "iot-2/evt/temperature/fmt/json";
const char*         humidTopic   = "iot-2/evt/humidity/fmt/json";
const char*         lineTopic   = "iot-2/evt/line1/fmt/json";
const char*         fireTopic   = "iot-2/evt/fire/fmt/json";
const char*         killTopic_receive   = "iot-2/cmd/kill/fmt/json";
const char*         twittTopic_receive   = "iot-2/cmd/twitter/fmt/json";


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
AdafruitIO_Feed *flag = io.feed("twitter_warn");
AdafruitIO_Feed *kill = io.feed("kill");
AdafruitIO_Feed *stoped = io.feed("fire");

void setup() {
    Serial.begin(115200);
    
    pinMode(RELAY, OUTPUT);
    pinMode(trigPin, OUTPUT);    //Trig 핀 output으로 세팅
    pinMode(echoPin, INPUT);    //Echo 핀 input으로 세팅
    pinMode(pushSW, INPUT_PULLUP);
    attachInterrupt(pushSW, buttonClicked, FALLING);
    eeprom_begin();
    chkFactoryReset();

    init_config_json();
    JsonObject& cfg = *ptr_config;
    // *** If no "config" is found or "config" is not "done", run configDevice ***
    if(!cfg.containsKey("config") || strcmp((const char*)cfg["config"], "done")) {
         configDevice();
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)cfg["ssid"], (const char*)cfg["w_pw"]);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(i++ > 15) {       
            break;     
        }
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
    lastPublishMillis = 0;
    
    io.connect();
    kill->onMessage(handleMessage);
    
    while(io.status() < AIO_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    kill->get();

    Serial.println(io.statusText());
}

void loop() {
    // main loop
    io.run();
    if (!mqClient.connected()) {
        reconnect();
    }
    else if ((pubInterval !=0) && (millis() - lastPublishMillis > pubInterval)) {
        delay(10);
        lastPublishMillis = millis();
        gettemperature();
        Joo_iot_publish(humidTopic, String(humidity), "humidity");
        delay(10);
        Joo_iot_publish(tempTopic, String(temp_f), "temperature");
        delay(10);
        long duration, distance;    //기본 변수 선언


        if(!digitalRead(RELAY)){
            long duration, distance;    //기본 변수 선언
            //Trig 핀으로 10us의 pulse 발생
            digitalWrite(trigPin, LOW);        //Trig 핀 Low
            delayMicroseconds(2);            //2us 유지
            digitalWrite(trigPin, HIGH);    //Trig 핀 High
            delayMicroseconds(10);            //10us 유지
            digitalWrite(trigPin, LOW);        //Trig 핀 Low
            //Echo 핀으로 들어오는 펄스의 시간 측정
            duration = pulseIn(echoPin, HIGH);        //pulseIn함수가 호출되고 펄스가 입력될 때까지의 시간. us단위로 값을 리턴.
         
            //음파가 반사된 시간을 거리로 환산
            //음파의 속도는 340m/s 이므로 1cm를 이동하는데 약 29us.
            //따라서, 음파의 이동거리 = 왕복시간 / 1cm 이동 시간 / 2 이다.
            distance = duration / 29 / 2;        //센치미터로 환산
            Joo_iot_publish(lineTopic, String(distance), "distance");
            Serial.println(distance);
        }
    }
    if(kill_flag){
        if(digitalRead(RELAY)){
            delay(1000);
            digitalWrite(RELAY, LOW);
            Joo_iot_publish(switchTopic, String("revival"), String("kill_switch"));
            pubInterval = 5000;
        } else {
            delay(1000);
            digitalWrite(RELAY, HIGH);
            Joo_iot_publish(switchTopic, String("killed"), String("kill_switch"));
            pubInterval = 2200;
        }
        kill_flag = 0;
    }
    
    
    mqClient.loop();
    
}

void handleMessage(AdafruitIO_Data *data) {

 
}

void buttonClicked() {
    kill_flag = 1;
    //Joo_iot_publish(switchTopic, String("pushed"), String("sw1"));
}

void Joo_iot_publish(const char* topic, String val, String key){
    char msgBuffer[512];
    StaticJsonBuffer<512> jsonOutBufferIot;
    JsonObject& joo = jsonOutBufferIot.createObject();
    JsonObject& d_ata = joo.createNestedObject("d");
    d_ata[key] = val;
    joo.printTo(msgBuffer, sizeof(msgBuffer));
    Serial.println("publishing data :"); Serial.println(msgBuffer);
    mqClient.publish(topic, msgBuffer);    
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
        if (!strcmp(killTopic_receive, topic))
            handleCommand_kill(payload, payloadLength);
        else if(!strcmp(twittTopic_receive, topic))
            handleCommand_twitter(payload, payloadLength);
        else
            handleCommand(payload, payloadLength);
    }
}

void handleCommand_twitter(byte* payload, unsigned int msgLen) {
    char temp[msgLen+1];
    StaticJsonBuffer<512> jsonInBuffer;
    JsonObject& root = jsonInBuffer.parseObject((char*)payload);
    JsonObject& d = root["d"];
    d.prettyPrintTo(Serial); Serial.println();
    //Serial.println(String((const char*)d["lamp"]));
    if(String((const char*)d["twitter"]) == "warn"){ 
        flag->save("1");
        Serial.println("saved");
        //경고 한번 주는 거여서 별개의 퍼블리쉬 생략
    }
}

void handleCommand_kill(byte* payload, unsigned int msgLen) {
    char temp[msgLen+1];
    StaticJsonBuffer<512> jsonInBuffer;
    JsonObject& root = jsonInBuffer.parseObject((char*)payload);
    JsonObject& d = root["d"];
    d.prettyPrintTo(Serial); Serial.println();
    //Serial.println(String((const char*)d["lamp"]));
    if(String((const char*)d["kill"]) == "kill"){ 
        digitalWrite(RELAY, HIGH);
        pubInterval = 2200;
        Joo_iot_publish(switchTopic, String("killed"), String("kill_switch"));
    } else {
        digitalWrite(RELAY, LOW);
        pubInterval = 5000;
        Joo_iot_publish(switchTopic, String("revival"), String("kill_switch"));
    }
}
void handleCommand(byte* payload, unsigned int msgLen) {
    // command handle logic here
    //GPIO 명령어 여기다 씀
}

void reconnect() {
    char idBuf[100];
    JsonObject& cfg = *ptr_config;

    while (!mqClient.connected()) {
        yield();
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
        mqClient.publish(publishTopic, info.c_str());//여기가 퍼블리쉬
        Serial.println("device Publish ok");
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
void gettemperature() {
    unsigned long currentMillis = millis();
    if(currentMillis - lastDHTReadMillis >= interval) { 
        lastDHTReadMillis = currentMillis;
        humidity = dht.readHumidity();
        // Read humidity (percent)
        temp_f = dht.readTemperature();
        // Read temperature as Fahrenheit
        // Check if any reads failed and exit early (to try again).
        if (isnan(humidity) || isnan(temp_f)) {
            return;         
            }    
        } 
}
