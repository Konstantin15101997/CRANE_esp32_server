#include "sbus.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor2.h>

//Общение по UDP

const char* ssid = "Сrane";             // Ваш SSID
const char* password = "12Qwerty";       // Ваш пароль
const char* udpServerIP = "192.168.4.2"; // IP получателя
const int udpServerPort = 1234;            // Порт получателя

WiFiUDP udp;

String buffer;  //"режим, скорость" - режимы: 1 - вперед-назад, 2 - поворот гусениц, 3 - вращение крана
int toggle_switch[4]={0,0,0,0}; //Положение тумблеров

/* SBUS object, reading SBUS */
bfs::SbusRx sbus_rx(&Serial2, 16,  17, true); 
/* SBUS object, writing SBUS */
bfs::SbusTx sbus_tx(&Serial2, 16,  17, true); 
/* SBUS data */
bfs::SbusData data;

GMotor2<DRIVER2WIRE> motor1(2, 16); //D4, tx - Гусеница левая
GMotor2<DRIVER2WIRE> motor2(12, 14); //D5, D6 - Гусеника правая
GMotor2<DRIVER2WIRE> motor3(5, 4); //D1, D2 - Вращение крана

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  /* Begin the SBUS communication */
  sbus_rx.Begin();
  sbus_tx.Begin();

  while(WiFi.softAPgetStationNum()==0);
  Serial.println("TRACK connected");
  
}

void loop () {

//Читаем SBUS канал
  if (sbus_rx.Read()) {
    data = sbus_rx.data();

  //Выбираем режим  
    for (int8_t i = 4; i < 8; i++) {
      toggle_switch[i-4]= map(data.ch[i],1811,172,1,0);
    }
    buffer = ((toggle_switch[0]==0 && toggle_switch[1]==0) || (toggle_switch[0]==1 && toggle_switch[1]==1) ) ? "0": (toggle_switch[0]==1 && toggle_switch[1]==0) ? "1": "2";
  
  //Устанавливаем скорость  
    if (buffer=="1"){
      buffer+=",";
      buffer+=String(data.ch[0]);
      buffer+=",";
      buffer+=String(data.ch[1]);
    }else if (buffer=="2"){
      buffer+=",";
      buffer+=String(992);
      buffer+=",";
      buffer+=String(data.ch[1]);
    }
    else{buffer="0,992,992";}
    Serial.println(buffer);

  }
  udp.begin(udpServerPort);
  udp.beginPacket(udpServerIP, udpServerPort);
  udp.printf("%s",buffer);
  udp.endPacket();

  buffer="";
}

