#include <ArduinoJson.h>
#include <iotUtil.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
bool wifi_connected = false;

String responseHTML = ""
"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body><center>"
"<p>Device Setup Page</p>"
"<form action='/button'>"
"<p><input type='text' name='ssid' placeholder='SSID' onblur='this.value=removeSpaces(this.value);'></p>"
"<p><input type='text' name='password' placeholder='WLAN Password'></p>"
"<p><input type='submit' value='Submit'></p></form>"
"<p>This is 5/21 lab3</p></center></body>"
"<script>function removeSpaces(string) {"
"   return string.split(' ').join('');"
"}</script></html>";

void setup() {    
    Serial.begin(115200);   
    eeprom_begin();
    chkFactoryReset();  
    //init_config_json();
    JsonObject& cfg = *ptr_config;
    int i;
    for ( i = 0 ; EEPROM.read(i) != '\0'; i++ ) {
        cfgBuffer[i] = EEPROM.read(i);
    }
    cfgBuffer[i] = '\0';

    ptr_config = &(jsonConfigBuffer.parseObject(String(cfgBuffer)));
    if (ptr_config->success() ) {
        Serial.println("CONFIG JSON Successfully loaded");
        char maskBuffer[EEPROM_LENGTH];
        maskConfig(maskBuffer);
        Serial.println(String(maskBuffer));
        setup_runtime();
    } else {
        ptr_config = &(jsonConfigBuffer.createObject());
        Serial.println("Initializing CONFIG JSON");
        setup_captive();
    }
}

void setup_runtime() {
    JsonObject& cfg = *ptr_config;
    String ID = cfg["ssid"];
    String PW = cfg["w_pw"];  
    WiFi.mode(WIFI_STA);
    WiFi.begin(ID.c_str(),PW.c_str());  
    Serial.println("");
    // Wait for connection
    int i = 0; 
    while (WiFi.status() != WL_CONNECTED) {   
        delay(500);       
        Serial.print(".");
        if(i++ > 15) {       
            //setup_captive();
            return;        
        }
    }
    Serial.println(""); 
    Serial.print("Connected to "); Serial.println(ID); 
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    if (MDNS.begin("YourNameHere")) {    
        Serial.println("MDNS responder started"); 
    }
    Serial.println("WiFi connected!!");
}

void setup_captive() {
    JsonObject& cfg = *ptr_config;
    cfg["config"] = "none"; 
    WiFi.mode(WIFI_AP);  
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); 
    WiFi.softAP("Joduino");  
    dnsServer.start(DNS_PORT, "*", apIP); 
    webServer.on("/button", button);   
    webServer.onNotFound([]() {      
        webServer.send(200, "text/html", responseHTML); 
    });  
    webServer.begin(); 
    Serial.println("Captive Portal Started");
}

void loop() {
    JsonObject& cfg = *ptr_config;
    if ( cfg["config"] == "none") {    
        dnsServer.processNextRequest();
        webServer.handleClient();
    }
    else{
        if(WiFi.status() != WL_NO_SSID_AVAIL){
            Serial.println("WiFi sill alive!!!");    
        }
        delay(2000);
        Serial.println("A code that doesn't matter if the wifi is broken!");
     
           
    }
}

void button(){   
    Serial.println("button pressed");
    reset_config();
    //save_config_json()
    int i, len;
    JsonObject& cfg = *ptr_config;
    cfg["config"] = "done";
    cfg["ssid"] = webServer.arg("ssid");
    cfg["w_pw"] = webServer.arg("password");
    ptr_config->printTo(cfgBuffer, sizeof(cfgBuffer));
    len=strlen(cfgBuffer);
    for(i = 0 ;i < len;i++) {
        EEPROM.write(i, cfgBuffer[i]);
    }
    EEPROM.write(i, '\0');
    EEPROM.commit();
    webServer.send(200, "text/plain", "OK");   
    ESP.restart(); 
} // Saves string to EEPROM
