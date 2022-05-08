// While button is Holded - LED is ON
// When button is released - LED is off

/*   Данный скетч делает следующее: передатчик (TX) отправляет массив
     данных, который генерируется согласно показаниям с кнопки. 
     Это может быть нажатие кнопки или удержание.
     
     Приёмник (RX) получает массив и генерирует ШИМ сигнал на светодиод.
     Зажигание один раз или удержание в светящем состоянии.
    by AlexGyver 2016
*/

#include <SPI.h>                                          // Подключаем библиотеку для работы с шиной SPI
#include <RF24.h>                                         // Подключаем библиотеку для работы с nRF24L01+
#include "GyverButton.h"
#include "printf.h"
#include <GyverJoy.h>

#define LED_PIN 5
#define BTN_PIN 4  // кнопка подключена сюда (BTN_PIN --- КНОПКА --- GND)

#define JSW 2

GyverJoy jx(0);   // джойстик на пине A0
GyverJoy jy(1);   // джойстик на пине A1
// GButton jsw(2);   // кнопка джойстика на пине 2

GButton butt1(BTN_PIN);  // Trema-кнопка со светодиодом

RF24 radio(9, 10);  // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ порты D9, D10: CSN CE
const uint32_t pipe = 111156789; // адрес рабочей трубы;

byte data[1];  // Создаём массив для приёма данных
int flag = 0;
int value = 0;

int JSW_state = 0;

void setup(){

  Serial.begin(9600);

                            //////////////////////////////
                            //////// SET UP BUTTON ///////
                            //////////////////////////////
  butt1.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  butt1.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt1.setClickTimeout(200);   // настройка таймаута между кликами (по умолчанию 300 мс)

  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND)
  // LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
  // по умолчанию стоит HIGH_PULL
  butt1.setType(LOW_PULL);

  // NORM_OPEN - нормально-разомкнутая кнопка
  // NORM_CLOSE - нормально-замкнутая кнопка
  // по умолчанию стоит NORM_OPEN
  butt1.setDirection(NORM_OPEN);

  // Joystick button
  // jsw.setType(LOW_PULL);
  // jsw.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  // jsw.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  // jsw.setClickTimeout(200);   // настройка таймаута между кликами (по умолчанию 300 мс)
  // jsw.setDirection(NORM_OPEN);
  pinMode(JSW, INPUT_PULLUP);

  jx.calibrate();   // калибровка нуля при запуске
  jx.deadzone(10);  // мёртвая зона
  jx.exponent(GJ_CUBIC);  // экспонента для плавности
 
  jy.calibrate();   // калибровка нуля при запуске
  jy.deadzone(10);  // мёртвая зона
  jy.exponent(GJ_CUBIC);  // экспонента для плавности

  //////////////////////////////
  ///////// SET UP LED /////////
  //////////////////////////////
  pinMode(LED_PIN, OUTPUT);

  //////////////////////////////
  //////// SET UP RADIO ////////
  //////////////////////////////
  Serial.println("Transmitter ON");
  radio.begin();                // инициализация
  delay(2000);
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);   // размер пакета, в байтах
  radio.openWritingPipe(pipe);   // открыть трубу на отправку

  radio.setDataRate(RF24_1MBPS); // скорость обмена данными RF24_1MBPS или RF24_2MBPS
//  radio.setCRCLength(RF24_CRC_8); // размер контрольной суммы 8 bit или 16 bit
  radio.setPALevel(RF24_PA_HIGH); // уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setChannel(0x6f);         // установка канала
  radio.powerUp();               // включение или пониженное потребление powerDown - powerUp
  radio.stopListening();  //радиоэфир не слушаем, только передача
  Serial.println("TransmitterTester READY");

}
void loop() {

  // // тикер опрашивает АЦП по своему таймеру
  // if (jx.tick()) {
  //   // выводим значение
  //   Serial.println(jx.value());
  // }

  // // тикер опрашивает АЦП по своему таймеру
  // if (jy.tick()) {
  //   // выводим значение
  //   Serial.println(jy.value());
  // }

  // if (jsw.isClick()) {
  //   Serial.println("Joystick Click");         // проверка на один клик
  // }

  JSW_state = digitalRead(JSW);
  // Serial.print(" | Button: ");
  Serial.println(!JSW_state);

  // Serial.print(map(analogRead(J_VRX), 0, 1023, -100, 100));
  // Serial.print(',');
  // Serial.print(map(analogRead(J_VRY), 0, 1023, -100, 100));
  // Serial.print(',');
  // Serial.println(digitalRead(J_SW) * 100);

  data[0] = 50;

  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться

  if (butt1.isClick()) {
    Serial.println("Click");         // проверка на один клик
    flag = 1;
  }

  if (flag == 1) {
    radio.powerUp();    // включить передатчик
    radio.write(&data, sizeof(data));
    flag = 0;           //опустить флаг
    radio.powerDown();  // выключить передатчик
    Serial.println("Sent from click");         // проверка на один клик
  }

  if (butt1.isStep()) {            // обработчик удержания с шагами
    value++;                       // увеличивать/уменьшать переменную value с шагом и интервалом!
    Serial.println(value);         // для примера выведем в порт
  }
  if (butt1.isRelease()) {
    value = 0;
    Serial.println("Release");         // для примера выведем в порт
  }

  if (butt1.isHold()) {                                  // если кнопка удерживается
      Serial.println("Holding ");                  // выводим пока удерживается
      analogWrite(LED_PIN, 20);
      radio.powerUp();    // включить передатчик
      radio.write(&data, sizeof(data));
      radio.powerDown();  // выключить передатчик
      Serial.println("Sent");         // проверка на один клик
    } else {
      analogWrite(LED_PIN, 0);
  }
}