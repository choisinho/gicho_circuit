#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include "pitches.h"
#include <Wire.h>
#define EEPROM_I2C_ADDRESS 0x50// eeprom의 정보가 있는 주소
RFID rfid(10, 5); // 몇 번 핀을 사용할 것인가를 정한다. 10핀: SDA, 5핀: reset

// 태그 식별에 사용되는 정보의 길이(8비트 5개의 수)
byte card[5];
byte serNum[5];
byte data[5];

int access_melody[] = {NOTE_G4, 0, NOTE_A4, 0, NOTE_B4, 0, NOTE_A4,
                       0, NOTE_B4, 0, NOTE_C5, 0
                      };// 올바른 카드를 인식했을 때 나오는 소리
int access_noteDurations[] = {8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 4};// 소리의 지속 시간
int fail_melody[] = {NOTE_G2, 0, NOTE_F2, 0, NOTE_D2, 0};// 소리의 음정

int fail_noteDurations[] = {8, 8, 8, 8, 8, 4};// 틀린 카드를 인식했을 때 나오는 소리의 지속 시간

// LED와 부저, 서보모터를 작동시키는데 쓰이는 UNO 핀
int LED_access = 2; 
int LED_intruder = 3; 
int speaker_pin = 8; 
int servoPin = 9; 
Servo doorLock; // 서보 모터 설정
void setup() {

  doorLock.attach(servoPin); // 서보모터를 UNO 에 연결
  Wire.begin();
  Serial.begin(9600);
  SPI.begin(); 

  rfid.init();
  Serial.println("Arduino card reader");
  delay(1000);

  // 이하는 핀의 상태를 출력가능한 상태로 변화하는 것.
  pinMode(LED_access, OUTPUT);
  pinMode(LED_intruder, OUTPUT);
  pinMode(speaker_pin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  
  // eeprom 내에 있는 주소 0~4에 있는 정보를 가져온다.  
  for(int i=0;i<5;i++){
    byte a= readAddress(i);
    card[i]=a;
    delay(100);
  }
}
void loop() { 
  boolean card_card = true; //올바른 태그를 사용했는가를 판단
  delay(500);
  if (rfid.isCard()) {// rfid 태그를 인식했을 때
    if (rfid.readCardSerial()) {// 태그 내의 정보를 읽는다.
      delay(500);
      
      // 태그 내의 정보를 입력받음
      data[0] = rfid.serNum[0];
      data[1] = rfid.serNum[1];
      data[2] = rfid.serNum[2];
      data[3] = rfid.serNum[3];
      data[4] = rfid.serNum[4];

    }
    Serial.print("Card found - code:");
    for (int i = 0; i < 5; i++) {// 저장된 정보와 카드의 정보를 비교한다.  
      if (data[i] != card[i]) card_card = false;// 서로 다르면 잘못된 카드를 인식했다고 판단한다.
    }
    Serial.println();
    if (card_card) { // 부저 소리가 난다.
      Serial.println("Hello!"); 
      for (int i = 0; i < 12; i++) { 
        int access_noteDuration = 1000 / access_noteDurations[i];
        tone(speaker_pin, access_melody[i], access_noteDuration);
        int access_pauseBetweenNotes = access_noteDuration * 1.30;
        delay(access_pauseBetweenNotes);
        noTone(speaker_pin);
      }
    }
    // eeprom 내의 정보와 태그의 정보와 다를 때
    else { 
      Serial.println("Wrong Tag.");
      digitalWrite(LED_intruder, HIGH); 

    // 부저 소리
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
    if (card_card) { //저장한 정보와 태그의 정보와 같을 경우의 녹색 LED가 켜진다.
      Serial.println("Welcome!");
      digitalWrite(LED_access, HIGH); 
      doorLock.write(180); 
      delay(3000); 
      doorLock.write(0); 
      digitalWrite(LED_access, LOW);
    }
    Serial.println();
    delay(500);

    // rfid.halt();// rfid 태그 중복해서 읽는 것을 방지 한다.
  }
}

byte readAddress(int address)// eeprom에서 읽을 주소와 읽을 바이트 수
{
  byte rData = 0xFF;
  Wire.beginTransmission(EEPROM_I2C_ADDRESS);
  
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.endTransmission();  

  Wire.requestFrom(EEPROM_I2C_ADDRESS, 1);  

  rData =  Wire.read();// 지정한 주소에서 값을 읽어들인다.

  return rData;
}
