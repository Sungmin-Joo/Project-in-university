#include <IRrecv.h>
#include <DHT.h>
#define DHTTYPE DHT22
// DHT22 is attached to GPIO14 on this Kit
#define DHTPIN  14
#define interval  2000
#define TRIG 12
#define ECHO 13

int count = 0;

DHT                 dht(DHTPIN, DHTTYPE, 11);
char                dht_buffer[10];
float               humidity;
float               temp_f;
unsigned long       lastDHTReadMillis = 0;
// will store last temp was read

int RECV_PIN = 02;//시그널 핀을 연결
const short int RELAY = 15;
const unsigned int LINE_1 = 16724175; 
short int power_flag = 0;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
    pinMode(RELAY, OUTPUT);

    pinMode(TRIG,OUTPUT);
    pinMode(ECHO,INPUT);
    
    irrecv.enableIRIn();
    
    Serial.begin(115200);
    Serial.println("Starting");

     
}

void loop() {
    if(irrecv.decode(&results)){
        unsigned int ircode = results.value;
        Serial.println(ircode);
        irrecv.resume();//다음 데이터를 수신
        
        if(ircode == LINE_1){
            power_flag = ~power_flag;
        }
    }
    if(power_flag){
        digitalWrite(RELAY, HIGH);
        Serial.println("Relay on");            
    } else {
        digitalWrite(RELAY, LOW);
        Serial.println("Relay off");
    }
    delay(30);
    gettemperature();
    Serial.print("Temperature : ");
    Serial.print(temp_f);
    Serial.print(", Humidity : ");
    Serial.println(humidity);
    Serial.println("------------------------------");

    digitalWrite(TRIG,LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG,HIGH);
    delayMicroseconds(5);
    digitalWrite(TRIG,LOW);

    long distance = pulseIn(ECHO,HIGH,5800)/58;
 
    // Serial 모니터에 거리값을 표시합니다
    if(distance <= 10)
    {
        Serial.print("d: ");
        Serial.print(distance);
        Serial.print("cm\n");
        Serial.println("Line 1 operating ");    
        count = 0;
    }
    else
    {
        count++;
        Serial.print("counting..");
        Serial.println(count);
        Serial.println(distance);
        if(count > 10)
            Serial.println("Line 1 stop!! hey!!");
    }
    
    // loop를 도는 속도가 너무 빠르므로 delay로 0.015정도로 늦춰줍니다
    delay(1000);
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
