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

byte mode;
byte number_motor;
int speed_motor;

void Stop(){
  motor1.smoothTick(0);
  motor2.smoothTick(0);
  motor3.smoothTick(0);
  motor4.smoothTick(0);
  motor5.smoothTick(0);
  motor6.smoothTick(0);

  buffer = "0,0,0";
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  /* Begin the SBUS communication */
  sbus_rx.Begin();
  sbus_tx.Begin();
  udp.begin(udpServerPort);
  /*while(WiFi.softAPgetStationNum()==0);
  Serial.println("TRACK connected");*/

  motor1.setMode(FORWARD);
  motor1.setSmoothSpeed(10);
  motor2.setMode(FORWARD);
  motor2.setSmoothSpeed(10);
  motor3.setMode(FORWARD); 
  motor3.setSmoothSpeed(10);
  motor4.setMode(FORWARD);
  motor4.setSmoothSpeed(10);
  motor5.setMode(FORWARD);
  motor5.setSmoothSpeed(10);
  motor6.setMode(FORWARD);
  motor6.setSmoothSpeed(10);
}

void loop () {
  
//Читаем SBUS канал
  if (sbus_rx.Read()) {
    data = sbus_rx.data();

  //Выбираем режим  
    for (int8_t i = 4; i < 8; i++) {
      toggle_switch[i-4]= map(data.ch[i],1811,172,1,0);
    }
    bool start = ((toggle_switch[0]+toggle_switch[1])!=1) ? 0 : 1;
   
    if (start==1){
      for (int8_t i = 0; i < 2; i++) {
        if (toggle_switch[i]==1){
          mode = i+1;
        }
      }
      if (mode == 1){  // режим TRACK
        speed_motor = 0;

        buffer=String(map(data.ch[0],172,1811,-255,255));
        buffer+=",";
        buffer+=String(map(data.ch[1],172,1811,-255,255));
        buffer+=",";
        buffer+=String(map(data.ch[2],172,1811,-255,255));
      }
      else if (mode == 2){ // режим CRANE
        buffer = "0,0,0";

        speed_motor=map(data.ch[0],172,1811,-255,255);
        speed_motor = (speed_motor>=-10 && speed_motor<=10) ? 0 : speed_motor;
        
        if ((map(data.ch[6], 172,1811,0,2)!=0) && (map(data.ch[7], 172,1811,0,2)==0)){
          number_motor= (map(data.ch[6], 172,1811,0,2)==1) ? 1 : 2;
        }else if (map(data.ch[6], 172,1811,0,2)==0){
          number_motor = (map(data.ch[7], 172,1811,0,2)==0) ? 5 : (map(data.ch[7], 172,1811,0,2)== 1) ? 6 : 7; // 7 - это 5 и 6 одновременно 
        }else{number_motor =0;}
    
      }
    }else{
      buffer = "0,0,0";
      Stop();
      mode = 0;
    }  

  }

  if (mode==1){
    udp.beginPacket(udpServerIP, udpServerPort);
    udp.printf("%s",buffer);
    udp.endPacket();
    buffer="";
  }else if (mode==2){
    Serial.println(number_motor);
    if (number_motor == 0){
      Stop();
    }else{
      if (number_motor == 1){
        motor1.smoothTick(speed_motor);
      }else if (number_motor == 2){
        motor2.smoothTick(speed_motor);
      }else if (number_motor == 3){
        motor3.smoothTick(speed_motor);
      }else if (number_motor == 4){
        motor4.smoothTick(speed_motor);
      }else if (number_motor == 5){
        motor5.smoothTick(speed_motor);
      }else if (number_motor == 6){
        motor6.smoothTick(speed_motor);
      }else if (number_motor == 7){
        motor5.smoothTick(speed_motor);
        motor6.smoothTick(speed_motor);
      }
    }
  }
  
}



