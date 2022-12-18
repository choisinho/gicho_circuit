#include <SPI.h> // 마스터와 슬레이브 사용 가능한 통신. 마스터와 슬레이브 간 4개의 선으로 연결. 전 이중 방식의 통신으로 동시 통신이 가능.
#include <RFID.h>//                                       miso mosi SCK(클럭) ss(slave select)
#include <Servo.h>
#include "pitches.h"
#include <Wire.h>// I2C 통신을 구현하기위한 헤더파일. I2C 통신-마스터와 슬레이브 간 2개의 선으로 연결. 동시 송수신이 불가능.
#define EEPROM_I2C_ADDRESS 0x50// 기본 I2C 주소. 
RFID rfid(10, 5); // 몇 번 핀을 사용할 것인가를 정한다. 10번 핀: SDA, 5번 핀: reset

// 태그 식별에 사용되는 정보의 길이(8비트짜리 5개의 수)
byte data[5]; //eeprom에 저장된 정보를 담을 배열
byte serNum[5]; // 태그로부터 읽어들일 정보
byte tag_data[5];// 태그로부터 읽은 정보

int access_melody[] = {NOTE_G4, 0, NOTE_A4, 0, NOTE_B4, 0, NOTE_A4,
                       0, NOTE_B4, 0, NOTE_C5, 0
                      };// 주파수 배열, 올바른 카드를 인식했을 때 나오는 소리
int access_noteDurations[] = {8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 4};// 소리의 지속 시간 배열, 8분음표와 4분음표
int fail_melody[] = {NOTE_G2, 0, NOTE_F2, 0, NOTE_D2, 0};

int fail_noteDurations[] = {8, 8, 8, 8, 8, 4};


// LED와 부저, 서보모터를 작동시키는데 쓰이는 UNO의 핀 숫자
int LED_access = 2; // 등록된 사용자 태그를 인식하면 켜지는  led
int LED_intruder = 3;  // 미등록된 사용자 태그를 인식하면 켜지는 led
int speaker_pin = 8;  //피에조 부저 출력
int servoPin = 9;  // 서보모터 출력
Servo doorLock; // 사용할 서보모터를 변수로 선언

void setup() {

  doorLock.attach(servoPin); // 서보모터를 UNO 에 연결
  Wire.begin(); //wire 통신 시작
  Serial.begin(9600); // 보드 레이트 9600으로 설정
  SPI.begin();  //SPI 통신 시작

  rfid.init(); //RFID 초기화
  Serial.println("Arduino card reader");
  delay(1000);

  // 이하의 코드는 핀의 상태를 출력가능한 상태로 변화시키는 코드이다.
  pinMode(LED_access, OUTPUT);  // 2번 핀
  pinMode(LED_intruder, OUTPUT); // 3번 핀
  pinMode(speaker_pin, OUTPUT); // 8번 핀
  pinMode(servoPin, OUTPUT); // 9번 핀
  
  // eeprom 내에 있는 주소 0~4에 있는 정보를 하나씩 가져온다.  
  for(int i=0;i<5;i++){
    byte a= readAddress(i);
    data[i]=a;
    delay(100);
  }
}
void loop() { 
  boolean card_card = true; //올바른 태그를 사용했는가를 판단하는 변수
  delay(500);
  if (rfid.isCard()) {// rfid 태그의 존재를 인식했을 때
    if (rfid.readCardSerial()) {
      // 태그 내의 정보를 읽는다.
      delay(500);
      
      // 태그 내의 저장된 정보를 입력받음
      tag_data[0] = rfid.serNum[0];
      tag_data[1] = rfid.serNum[1];
      tag_data[2] = rfid.serNum[2];
      tag_data[3] = rfid.serNum[3];
      tag_data[4] = rfid.serNum[4];

    }
    Serial.print("Card found - code:"); 

    for (int i = 0; i < 5; i++) {// 저장된 정보와 카드의 정보를 비교한다.  
      if (tag_data[i] != data[i]) card_card = false;// 만약 두 정보가 서로 다르면 등록되지 않은 카드를 인식했다고 판단한다.
    }
    Serial.println();
    if (card_card) { // 등록된 정상적인 정보를 인식했다고 알리는 부저 소리 발생
      Serial.println("Hello!"); 
      for (int i = 0; i < 12; i++) { 
        int access_noteDuration = 1000 / access_noteDurations[i];// 부저의 지속 시간
        tone(speaker_pin, access_melody[i], access_noteDuration);// 부저에서 소리 발생 (핀, 주파수 , 길이)
        int access_pauseBetweenNotes = access_noteDuration * 1.30;// 부저에서 소리를 멈추는 시간을 설정
        delay(access_pauseBetweenNotes);// 일정 시간동안 대기한다.
        noTone(speaker_pin);// 소리 발생을 중단한다.
      }
    }
    // eeprom 내의 정보와 태그의 정보와 다를 때 실행
    else { 
      Serial.println("Wrong Tag.");
      digitalWrite(LED_intruder, HIGH); // 3번 핀에 전원을 공급 (RED LED)

      // 미등록된 틀린 정보를 인식했다는 부저 소리 발생
      for (int i = 0; i < 6; i++) { 
        int fail_noteDuration = 1000 / fail_noteDurations[i];// 부저의 지속 시간
        tone(speaker_pin, fail_melody[i], fail_noteDuration);// 부저에서 소리 발생(핀, 주파수, 길이)
        int fail_pauseBetweenNotes = fail_noteDuration * 1.30;// 부저에서 소리를 멈추는 시간을 설정
        delay(fail_pauseBetweenNotes); // 일정 시간동안 대기한다.
        noTone(speaker_pin); // 소리 발생을 중단한다.
    }
    delay(1000);
    digitalWrite(LED_intruder, LOW); //RED led 꺼짐
    }

    if (card_card) { //등록된 정보와 태그의 정보와 같을 경우의 녹색 LED가 켜진다.
      Serial.println("Welcome!");
      digitalWrite(LED_access, HIGH); 
      
      // 서보모터를 180도 돌린 후, 3초 뒤에 원상태로 돌린다.
      doorLock.write(180); 
      delay(3000); 
      doorLock.write(0); 
      digitalWrite(LED_access, LOW);
    }
    Serial.println();
    delay(500);
  }
}
//  eeprom  값을 읽어들이는 파트
byte readAddress(int address)// eeprom에서 읽을 주소
{
  byte rData = 0xFF; // UNO에서 eeprom의 주소에 있는 값 데이터 버퍼에 저장한다.
  Wire.beginTransmission(EEPROM_I2C_ADDRESS); // 마스터에서 전송하기위한 슬레이브의 주소 값, 포인터 위치 설정.
  Wire.write((int)(address >> 8));   //MSB. 데이터 형에서 가장 높은 위치의 비트
  Wire.write((int)(address & 0xFF)); //LSB. 데이터 형에서 가장 낮은 위치의 비트
  Wire.endTransmission();    // 데이터 버퍼에 있는 값을 전송한다.


  // eeprom 내 지정한 주소 내의 값을 1바이트만큼 요청
  Wire.requestFrom(EEPROM_I2C_ADDRESS, 1);  
  

  rData =  Wire.read();// 지정한 주소에서 정해진 범위의 값을 읽어들인다.

  return rData;// eeprom 에서 읽어들인 값 반환
}
