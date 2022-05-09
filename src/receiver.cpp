#include <SPI.h>                                          // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку  для работы с nRF24L01+
#include "RadioHelper.h"

#define PIN_LED 5
#define LED_POWER 8
#define SPEED_2 6  // Motor2
#define DIR_2 7  // Motor2

RF24 radio(9, 10);  // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
const uint32_t pipe = 111156789; // адрес рабочей трубы;

struct DataPack {
  int motorSpeed;
  byte LED;
};
DataPack Signal;  // control values
// DataPack telemetry;  // control values

// int telemetry[2];       // массив данных телеметрии (то что шлём на передатчик)

void setup(){
  Serial.begin(9600);

  // Motor
  pinMode(SPEED_2, OUTPUT);
  pinMode(DIR_2, OUTPUT);

  pinMode(PIN_LED, OUTPUT);
  pinMode(LED_POWER, OUTPUT);  // voltage pin for the LED
  digitalWrite(LED_POWER, HIGH);

  setupRadio();
}

void loop(){

  // DOESN'T WORK PROPERLY -- enableAckPayload
  // while (radio.available()) {                    // слушаем эфир
  //   radio.read(&Signal, sizeof(Signal));  // чиатем входящий сигнал
  //   analogWrite(PIN_LED, Signal.LED);

  //   // формируем пакет данных телеметрии
  //   telemetry.motorSpeed = 11;
  //   telemetry.LED = 22;

  //   // отправляем пакет телеметрии
  //   radio.writeAckPayload(pipe, &telemetry, sizeof(telemetry));
  //   Serial.println("AckPayload sent");
  // }
  // analogWrite(PIN_LED, 0);

  if (radio.available()) {                                
      radio.read(&Signal, sizeof(Signal));
      analogWrite(PIN_LED, Signal.LED);
      delay(100);
      Signal.motorSpeed < 0 ? digitalWrite(DIR_2, LOW) : digitalWrite(DIR_2, HIGH);
      analogWrite(SPEED_2, abs(Signal.motorSpeed)/3);
  } else {
      analogWrite(PIN_LED, 0);
      analogWrite(SPEED_2, 0);
  }
}

void setupRadio(void) {
  radio.begin();  // инициализация
  Serial.println("Receiver ON");
  delay(2000);
  radio.setAutoAck(1);              // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);          // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();         // разрешить отсылку данных в ответ на входящий сигнал, не работает на скорости 250 kbps!
  radio.setPayloadSize(32);         // размер пакета, в байтах
  radio.openReadingPipe(1, pipe);   // открыть трубу на приём
  radio.setChannel(0x6f);           // установка канала
  radio.setPALevel (RF24_PA_MIN);   // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS);    // скорость обмена данными RF24_1MBPS или RF24_2MBPS или RF24_250KBPS (не поддерживает enableAckPayload)
  radio.setCRCLength(RF24_CRC_16); // размер контрольной суммы 8 bit или 16 bit,  
  radio.powerUp();                  // начать работу
  radio.startListening();           // приём
  Serial.println("Receiver READY");
}