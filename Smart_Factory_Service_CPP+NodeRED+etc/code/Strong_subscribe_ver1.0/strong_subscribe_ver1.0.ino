// Adafruit IO Subscription Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
char wifi_flag = 1;
/************************ Example Starts Here *******************************/

// set up the 'counter' feed
AdafruitIO_Feed *counter = io.feed("onoff");

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // start MQTT connection to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  counter->onMessage(handleMessage);
  int i = 0;
  // wait for an MQTT connection
  // NOTE: when blending the HTTP and MQTT API, always use the mqttStatus
  // method to check on MQTT connection status specifically
  while(io.mqttStatus() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
    i++;
    if(i > 20){
      wifi_flag = 0;
      break;   
    }
  }
  if(wifi_flag)
  {
  // Because Adafruit IO doesn't support the MQTT retain flag, we can use the
  // get() function to ask IO to resend the last value for this feed to just
  // this MQTT client after the io client is connected.
  counter->get();

  // we are connected
  Serial.println();
  Serial.println(io.statusText());  
  }
  else
  {
    Serial.println();
    Serial.println("fffffffffffffff");
  }

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  if(wifi_flag)
  {
      io.run();
      for(int i = 0; i <10; i++)
      {
        Serial.println(i);
        delay(200);
      }
  }
  else
  {
     Serial.println("nn");
     int i = 0;
     wifi_flag = 1;
     while(io.mqttStatus() < AIO_CONNECTED) {
        Serial.print(".");
        delay(50);
        i++;
        if(i > 5){
            wifi_flag = 0;
            break;   
    }
  }
  }
  // Because this sketch isn't publishing, we don't need
  // a delay() in the main program loop.

}

// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());

}
