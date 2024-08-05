#include "sbus.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <GyverMotor.h>

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


//Моторы обозначены по расположению пинов подключения на плате, слева направо 
GMotor motor1(DRIVER2WIRE, 4, 2); 
GMotor motor2(DRIVER2WIRE, 22, 18); 
GMotor motor3(DRIVER2WIRE, 19, 21); 
GMotor motor4(DRIVER2WIRE, 32, 33);
GMotor motor5(DRIVER2WIRE, 25, 26);
GMotor motor6(DRIVER2WIRE, 27, 12);

int speed_motor;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  /* Begin the SBUS communication */
  sbus_rx.Begin();
  sbus_tx.Begin();

  /*while(WiFi.softAPgetStationNum()==0);
  Serial.println("TRACK connected");*/

  motor1.setMode(FORWARD);
  motor2.setMode(FORWARD);
  motor3.setMode(FORWARD); 
  motor4.setMode(FORWARD);
  motor5.setMode(FORWARD);
  motor6.setMode(FORWARD);
}

void loop () {
  
//Читаем SBUS канал
  if (sbus_rx.Read()) {
    data = sbus_rx.data();

  //Выбираем режим  
    for (int8_t i = 4; i < 8; i++) {
      toggle_switch[i-4]= map(data.ch[i],1811,172,1,0);
    }
    //buffer = ((toggle_switch[0]==0 && toggle_switch[1]==0) || (toggle_switch[0]==1 && toggle_switch[1]==1) ) ? "0": (toggle_switch[0]==1 && toggle_switch[1]==0) ? "1": "2";
    buffer = ((toggle_switch[0]+toggle_switch[1]+toggle_switch[2])!=1) ? "0": "SEARCH";
    if (buffer=="SEARCH"){
      for (int8_t i = 0; i < 2; i++) {
        if (toggle_switch[i]==1){
          buffer =String(i+1);
        }
      }
    }
  //Устанавливаем скорость  
    if (buffer=="1"){
      buffer=String(map(data.ch[0],172,1811,-255,255));
      buffer+=",";
      buffer+=String(map(data.ch[1],172,1811,-255,255));
      buffer+=",";
      buffer+=String(map(data.ch[2],172,1811,-255,255));
    }
    else if (buffer=="2"){
      buffer+=" - mode CRANE";
      speed_motor=map(data.ch[0],172,1811,-255,255);
      speed_motor = (speed_motor>=-10 && speed_motor<=10) ? 0 : speed_motor;
    }
    else{buffer="0,0,0";}
    Serial.println(buffer);

  }

  if (buffer.startsWith("2")){
    Serial.println(speed_motor);

    motor1.setSpeed(speed_motor);
    motor2.setSpeed(speed_motor);
    motor3.setSpeed(speed_motor);
    motor4.setSpeed(speed_motor);
    motor5.setSpeed(speed_motor);
    motor6.setSpeed(speed_motor);

  }
    udp.begin(udpServerPort);
    udp.beginPacket(udpServerIP, udpServerPort);
    udp.printf("%s",buffer);
    udp.endPacket();
  

  buffer="";
}

