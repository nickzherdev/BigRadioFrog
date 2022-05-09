#include <SPI.h>                                          // Подключаем библиотеку для работы с шиной SPI
#include <RF24.h>                                         // Подключаем библиотеку для работы с nRF24L01+
#include "GyverButton.h"
#include "printf.h"
#include <GyverJoy.h>
#include "RadioHelper.h"
#include "JoystickHelper.h"

#define LED_PIN 5

GyverJoy jx(0);   // джойстик на пине A0
GButton jsw(3);   // кнопка джойстика

RF24 radio(9, 10);  // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ порты D9, D10: CSN CE
const uint32_t pipe = 111156789; // адрес рабочей трубы;

struct DataPack {
  int motorSpeed;
  byte LED;
};

DataPack Signal;  // control values
int user_input_jx = 0;
bool flag = 0;

void setup(){

  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  setupJoystick();
  setupRadio();
}

void loop() {

  if (jx.tick()) {                  // тикер опрашивает АЦП по своему таймеру
    Signal.motorSpeed = jx.value();
  }

  if (abs(Signal.motorSpeed) > 20 ) {  // Deadzone
    Signal.LED = (byte)abs(Signal.motorSpeed);
    radio.powerUp();
    radio.write(&Signal, sizeof(Signal));
    radio.powerDown();
    Serial.println("Sent");
  } else {
      analogWrite(LED_PIN, 0);
  }

  jsw.tick();  // обязательная функция отработки. Должна постоянно опрашиваться

  if (jsw.isClick()) {
    Serial.println("Joystick Click");
    flag = 1;
  }

  if (flag) {
    Signal.LED = 100;
    radio.powerUp();
    radio.write(&Signal, sizeof(Signal));
    radio.powerDown();
    Serial.println("Sent from click");
    flag = 0;
  }

  if (jsw.isHold()) {
    Serial.println("Holding ");
    Signal.LED = 100;
    Signal.motorSpeed = 0;
    radio.powerUp();
    radio.write(&Signal, sizeof(Signal));
    radio.powerDown();
    Serial.println("Sent from click");
  }
}

void setupJoystick(void) {
  // HIGH_PULL - (по умолчанию) кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND)
  // LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
  // NORM_OPEN - (по умолчанию) нормально-разомкнутая кнопка
  // NORM_CLOSE - нормально-замкнутая кнопка
  jsw.setType(HIGH_PULL);
  jsw.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  jsw.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  jsw.setClickTimeout(300);   // настройка таймаута между кликами (по умолчанию 300 мс)

  jx.calibrate();   // калибровка нуля при запуске
  jx.deadzone(20);  // мёртвая зона
  jx.exponent(GJ_CUBIC);  // экспонента для плавности
}

void setupRadio(void) {
  radio.begin();                    // инициализация
  Serial.println("Transmitter ON");
  delay(2000);
  radio.setAutoAck(1);              // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);          // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();         // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);         // размер пакета, в байтах
  radio.openWritingPipe(pipe);      // открыть трубу на отправку
  radio.setDataRate(RF24_1MBPS);    // скорость обмена данными RF24_1MBPS или RF24_2MBPS
  radio.setCRCLength(RF24_CRC_16);  // размер контрольной суммы 8 bit или 16 bit
  radio.setPALevel(RF24_PA_MIN);    // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setChannel(0x6f);           // установка канала
  radio.powerUp();                  // включение или пониженное потребление powerDown - powerUp
  radio.stopListening();            // радиоэфир не слушаем, только передача
  Serial.println("TransmitterTester READY");
}