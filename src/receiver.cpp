#include <SPI.h>                                          // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку  для работы с nRF24L01+

#define PIN_LED 5

RF24 radio(9, 10);                              // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
const uint32_t pipe = 111156789; // адрес рабочей трубы;
byte data[1];                                   // Создаём массив для приёма данных

void setup(){
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);

  pinMode(8, OUTPUT);     // voltage pin for the LED
  digitalWrite(8, HIGH);  // voltage pin for the LED

  Serial.println("Receiver ON");

  radio.begin();  // инициализация
  delay(2000);
  radio.setAutoAck(1);              // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);          // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();         // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);         // размер пакета, в байтах
  radio.openReadingPipe(1, pipe);   // открыть трубу на приём
  radio.setChannel(0x6f);           // установка канала
  
  radio.setPALevel (RF24_PA_HIGH);   // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_1MBPS);    // скорость обмена данными RF24_1MBPS или RF24_2MBPS
//  radio.setCRCLength(RF24_CRC_8); // размер контрольной суммы 8 bit или 16 bit
  radio.powerUp();                  // начать работу
  radio.startListening();           // приём
  Serial.println("Receiver READY");

}

void loop(){
  if (radio.available()) {                                // Если в буфере имеются принятые данные
      radio.read(&data, sizeof(data));                  // Читаем данные в массив data и указываем сколько байт читать
      Serial.println(data[0]);
      analogWrite(PIN_LED, data[0]);
      delay(100);
  } else {
//      Serial.println("No data");
      analogWrite(PIN_LED, 0);
  }

}