#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include "pitches.h"
#include <Wire.h>
RFID rfid(10, 5); 

byte card[5] = {0x4D, 0x55, 0xAD, 0xD3, 0x66};

byte serNum[5];
byte data[5];

int access_melody[] = {NOTE_G4, 0, NOTE_A4, 0, NOTE_B4, 0, NOTE_A4,
                       0, NOTE_B4, 0, NOTE_C5, 0
                      };
int access_noteDurations[] = {8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 4};
int fail_melody[] = {NOTE_G2, 0, NOTE_F2, 0, NOTE_D2, 0};
int fail_noteDurations[] = {8, 8, 8, 8, 8, 4};
int LED_access = 2; 
int LED_intruder = 3; 
int speaker_pin = 8; 
int servoPin = 9; 
Servo doorLock; 
void setup() {
  doorLock.attach(servoPin); 
  Serial.begin(9600);
  SPI.begin(); 
  rfid.init();
  Serial.println("Arduino card reader");
  delay(1000);
  pinMode(LED_access, OUTPUT);
  pinMode(LED_intruder, OUTPUT);
  pinMode(speaker_pin, OUTPUT);
  pinMode(servoPin, OUTPUT);
}
void loop() { 
  boolean card_card = true; 
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      delay(1000);
      data[0] = rfid.serNum[0];
      data[1] = rfid.serNum[1];
      data[2] = rfid.serNum[2];
      data[3] = rfid.serNum[3];
      data[4] = rfid.serNum[4];
    }
    Serial.print("Card found - code:");
    for (int i = 0; i < 5; i++) {
      if (data[i] != card[i]) card_card = false;
    }
    Serial.println();
    if (card_card) { 
      Serial.println("Hello Mark!"); 
      for (int i = 0; i < 12; i++) { 
        int access_noteDuration = 1000 / access_noteDurations[i];
        tone(speaker_pin, access_melody[i], access_noteDuration);
        int access_pauseBetweenNotes = access_noteDuration * 1.30;
        delay(access_pauseBetweenNotes);
        noTone(speaker_pin);
      }
    }
    else { 
      Serial.println("Card not recognized! Contact administrator!");
      digitalWrite(LED_intruder, HIGH); 
      for (int i = 0; i < 6; i++) { 
        int fail_noteDuration = 1000 / fail_noteDurations[i];
        tone(speaker_pin, fail_melody[i], fail_noteDuration);
        int fail_pauseBetweenNotes = fail_noteDuration * 1.30;
        delay(fail_pauseBetweenNotes);
        noTone(speaker_pin);
      }
      delay(1000);
      digitalWrite(LED_intruder, LOW);
    }
    if (card_card) { 
      Serial.println("Access granted.......Welcome!");
      digitalWrite(LED_access, HIGH); 
      doorLock.write(180); 
      delay(5000); 
      doorLock.write(0); 
      digitalWrite(LED_access, LOW);
    }
    Serial.println();
    delay(500);
    rfid.halt();
  }
}
