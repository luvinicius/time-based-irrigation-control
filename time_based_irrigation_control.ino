
/*
  Time Based Irrigation Control
  AUTOR:        Lucas Vinicius Pimenta
  LINKS:        https://github.com/luvinicius/time-based-irrigation-control.git
                https://br.linkedin.com/in/lucas-vinicius-pimenta-59212b49
  SKETCH:       Time Based Irrigation Control
  DESCRIPTION:  A DS1307RTC arduino watering schedule controller to domestic gardens.
*/

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <math.h>
#include "types.h"

// ---------------------------------------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------------------------------------
#define NUMBER_OF_RELAYS 2
#define MAX_SCHEDULING_NUMBER 6

#define RELAY_PORT_1 7
#define RELAY_PORT_2 8

// ---------------------------------------------------------------------------------------------------------
// Structs
// ---------------------------------------------------------------------------------------------------------

Relay buildRelay(int port) {
  return {port, HIGH}; // Always start turn-off
}

Scheduling buildScheduling(int Hour, int Minute, int Second, int Duration) {
  if (validateSchedulingTime(Hour, Minute, Second)) {
    return {Hour, Minute, Second, Duration};
  } else {
    return {0, 0, 0, 0};
  }
}

// Scheduling Functions
boolean validateSchedulingTime(int Hour, int Minute, int Second) {
  return      Hour >= 0    && Hour < 24
              &&  Minute >= 0  && Minute < 60
              &&  Second >= 0 && Second < 60;
}

boolean validateScheduling(Scheduling scheduling) {
  return validateSchedulingTime(scheduling.Hour, scheduling.Minute, scheduling.Second)
         && scheduling.Duration > 0;
}

bool isItInScheduledTime(tmElements_t tm, Scheduling scheduling) {
  int startingHour = scheduling.Hour;
  int startingMinute = scheduling.Minute;
  int startingSecond = scheduling.Second;

  Serial.print(" turn on at ");
  print2digits(startingHour);
  Serial.write(':');
  print2digits(startingMinute);
  Serial.write(':');
  print2digits(startingSecond);


  int finalSecond = startingSecond + scheduling.Duration;

  int finalMinute = startingMinute;
  if (finalSecond > 60) {
    finalMinute += round(finalSecond / 60);
    finalSecond %= 60;
  }

  int finalHour = startingHour;
  if (finalMinute > 60) {
    finalHour += round(finalMinute / 60);
    finalMinute %= 60;
  }

  Serial.print(" and turn out at ");
  print2digits(finalHour);
  Serial.write(':');
  print2digits(finalMinute);
  Serial.write(':');
  print2digits(finalSecond);

  return tm.Hour >= startingHour && tm.Hour <= finalHour
         && tm.Minute >= startingMinute && tm.Minute <= finalMinute
         && tm.Second >= startingSecond && tm.Second <= finalSecond;
}


// ---------------------------------------------------------------------------------------------------------
// Declaring relays
// ---------------------------------------------------------------------------------------------------------
Relay relays[NUMBER_OF_RELAYS];

// ---------------------------------------------------------------------------------------------------------
// Declaring schedulings
// ---------------------------------------------------------------------------------------------------------
Scheduling schedulings[NUMBER_OF_RELAYS][MAX_SCHEDULING_NUMBER];


// ---------------------------------------------------------------------------------------------------------
// Logica Arduino
// ---------------------------------------------------------------------------------------------------------
void setup() {
  // Defining relays and yours schedulings
  relays[0] = buildRelay(RELAY_PORT_1);
  schedulings[0][0] = buildScheduling(6, 40, 0, 60);

  relays[1] = buildRelay(RELAY_PORT_2);
  schedulings[1][0] = buildScheduling(6, 40, 0, 60);
  schedulings[1][1] = buildScheduling(10, 30, 0, 60);

  // Relays pin modes setup
  for (int i = 0; i < NUMBER_OF_RELAYS; i++) {
    Relay relay = relays[i];
    pinMode(relay.port, OUTPUT);
    digitalWrite(relay.port, relay.currentState);
  }

  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);
  Serial.println("-------------------");
}

void loop() {
  tmElements_t tm;

  if (RTC.read(tm)) {
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.println();

    for (int i = 0; i < NUMBER_OF_RELAYS; i++) {
      Relay relay = relays[i];
      Serial.print("Relay on port ");
      Serial.print(relay.port);
      Serial.print(" is ");
      Serial.println(relay.currentState == LOW ? "ON" : "OFF");

      for (int j = 0; j < MAX_SCHEDULING_NUMBER; j++) {
        Scheduling scheduling = schedulings[i][j];
        if (validateScheduling(scheduling)) {
          Serial.print("    ");
          Serial.print(j + 1);
          Serial.print("º schedule to ");

          if (isItInScheduledTime(tm, scheduling)) {
            Serial.print("    It's on time!");
            if (relay.currentState == HIGH) {
              Serial.print("    Relay turns on!");
              relay.currentState = LOW;
              digitalWrite(relay.port, relay.currentState);
              
            } else {
              Serial.print("    just wait...");
            }
          } else {
            Serial.print("   Is not in time!");
            if (relay.currentState == LOW) {
              Serial.print("    Relay turns off!");
              relay.currentState = HIGH;
              digitalWrite(relay.port, relay.currentState);
            } else {
              Serial.print("    just wait...");
            }
          }
          Serial.println();
        } else {
          Serial.println("    End of schedulings.");
          break;
        }
      }
      relays[i] = relay;
      Serial.println();
    }

    delay(1000);
  }
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
