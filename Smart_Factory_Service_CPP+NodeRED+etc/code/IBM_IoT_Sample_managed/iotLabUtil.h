#include <EEPROM.h>

#define             EEPROM_LENGTH 512
StaticJsonBuffer<EEPROM_LENGTH> jsonConfigBuffer;
JsonObject*         ptr_config;
char                cfgBuffer[EEPROM_LENGTH];
const int           RESET_PIN = 0;

void eeprom_begin() {
    EEPROM.begin(EEPROM_LENGTH);
}

void chkFactoryReset() {
    pinMode(RESET_PIN, INPUT_PULLUP);
    if( digitalRead(RESET_PIN) == 0 ) {
        unsigned long t1 = millis();
        //for (;digitalRead(RESET_PIN) == 0;){
        while(digitalRead(RESET_PIN) == 0) {
            delay(500);
            Serial.print(".");
        }
        unsigned long t2 = millis();
        if ((t2 - t1) > 5000) {
            Serial.printf("\nErasing EEPROM\n");
            EEPROM.write(0, '\0');
            EEPROM.commit();
        }
    }
    attachInterrupt(RESET_PIN, []() { ESP.restart(); }, FALLING);
}

char* maskConfig(char* buff) {
    StaticJsonBuffer<EEPROM_LENGTH> jsonTempBuffer;
    char msgBuffer[EEPROM_LENGTH];

    yield();
    ptr_config->printTo(msgBuffer, sizeof(msgBuffer));

    JsonObject& temp_cfg = jsonTempBuffer.parseObject(String(msgBuffer));
    if (temp_cfg.containsKey("w_pw")) {
        temp_cfg["w_pw"] = "********";
        temp_cfg["token"] = "********";
    }
    temp_cfg.printTo(buff, EEPROM_LENGTH);
    return buff;
}

void init_config_json() {
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
    } else {
        ptr_config = &(jsonConfigBuffer.createObject());
        Serial.println("Initializing CONFIG JSON");
    }
}

void save_config_json(){
    int i, len;
    JsonObject& cfg = *ptr_config;
    cfg["config"] = "done";
    ptr_config->printTo(cfgBuffer, sizeof(cfgBuffer));
    len=strlen(cfgBuffer);
    for(i = 0 ;i < len;i++) {
        EEPROM.write(i, cfgBuffer[i]);
    }
    EEPROM.write(i, '\0');
    EEPROM.commit();
}

void reset_config() {
    int i, len;
    char* empty="{}";
    len=strlen(empty);
    for(i = 0 ;i < len;i++) {
        EEPROM.write(i, empty[i]);
    }
    EEPROM.write(i, '\0');
    EEPROM.commit();
}