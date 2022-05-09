#include <SPI.h>                                          // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку  для работы с nRF24L01+

#define PIN_LED 5

#define SPEED_2      6  // Motor2
#define DIR_2        7  // Motor2

RF24 radio(9, 10);                              // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
const uint32_t pipe = 111156789; // адрес рабочей трубы;
// byte data[1];                                   // Создаём массив для приёма данных
int recieved_data[1];   // массив принятых данных

void setup(){
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);

  pinMode(6, OUTPUT);  // Motor2
  pinMode(7, OUTPUT);  // Motor2

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
      radio.read(&recieved_data, sizeof(recieved_data));                  // Читаем данные в массив data и указываем сколько байт читать
      Serial.println(recieved_data[0]);
      analogWrite(PIN_LED, recieved_data[0]);
      delay(100);
      if (recieved_data[0] < 0) {
        // устанавливаем направление мотора «M2» в одну сторону
        digitalWrite(DIR_2, LOW);
      }
      else if (recieved_data[0] > 0) {
        digitalWrite(DIR_2, HIGH);
      }
      // включаем второй мотор на максимальной скорости
      analogWrite(SPEED_2, abs(recieved_data[0]));
  } else {
//      Serial.println("No data");
      analogWrite(PIN_LED, 0);
      analogWrite(SPEED_2, 0);
  }

}