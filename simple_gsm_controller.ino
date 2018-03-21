#include <SimpleDHT.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>

SoftwareSerial mySerial(2, 3); // RX, TX
int ch = 0;
int led = 13;
String val = "";
String temperaturePattern = "Temperature = ";
String humidityPattern = "Humidity = ";
String temperatureInfo = "";
String humidityInfo = "";

int pinDHT = 10;
SimpleDHT22 dht22;
DS1307 RTC;

void setup() {
  delay(2000);  //время на инициализацию модуля
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  Serial.begin(9600);  //скорость порта
  Serial.println("GSM tester v1.0");
  mySerial.begin(9600);
  mySerial.println("AT+CLIP=1");  //включаем АОН
  delay(100);
  mySerial.println("AT+CMGF=1");  //режим кодировки СМС - обычный (для англ.)
  delay(100);
  mySerial.println("AT+CSCS=\"GSM\"");  //режим кодировки текста
  delay(100);
  
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning())
  {
    Serial.println(F("RTC is not running!"));
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop() {
  if (mySerial.available()) {  //если GSM модуль что-то послал нам, то
    while (mySerial.available()) {  //сохраняем входную строку в переменную val
      ch = mySerial.read();
      val += char(ch);
      delay(10);
    }
    if (val.indexOf("RING") > -1) {  //если звонок обнаружен, то проверяем номер
      if (val.indexOf("79536169000") > -1) {  //если номер звонящего наш. Укажите свой номер без "+"
        Serial.println("--- MASTER RING DETECTED ---");
        mySerial.println("ATH0");  //разрываем связь
        digitalWrite(led, HIGH);  //включаем светодиод на 3 сек
        delay(3000);
        digitalWrite(led, LOW);  //выключаем реле
      }
    } else
      Serial.println(val);  //печатаем в монитор порта пришедшую строку
    val = "";
  }
  if (Serial.available()) {  //если в мониторе порта ввели что-то
    while (Serial.available()) {  //сохраняем строку в переменную val
      ch = Serial.read();
      val += char(ch);
      delay(10);
    }
    //mySerial.println(val);  //передача всех команд, набранных в мониторе порта в GSM модуль
    if (val.indexOf("sendsms") > -1) {  //если увидели команду отправки СМС
      getTemperature();
      sms(String("message"), String("+79536169000"));  //отправляем СМС на номер +71234567890
    }
    val = "";  //очищаем
  }
}

void sms(String text, String phone)  //процедура отправки СМС
{
  Serial.println("SMS send started");
  mySerial.println("AT+CMGS=\"" + phone + "\"");
  delay(500);
  mySerial.println(temperatureInfo+"C");
  //mySerial.println("C");
  delay(500);
  mySerial.print(humidityInfo);
  mySerial.println("%");
  delay(500);
  mySerial.print((char)26);
  delay(500);
  Serial.println("SMS send complete");
  delay(2000);
}

void getTemperature() {
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read(pinDHT, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.println(err);delay(1000);
    return;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidity); Serial.println(" H");

  temperatureInfo = temperaturePattern+temperature;
  humidityInfo = humidityPattern+humidity;

  DateTime datetime = RTC.now();
  Serial.print(datetime.year());
  Serial.print(":");
  Serial.println(datetime.day());
}

