#include <SoftwareSerial.h>
#include <mqtt.h>
#include <stdlib.h>
#include <string.h>
#include "TinyGPS.h"
#define BMP085_ADDRESS 0x77  // I2C address of BMP085
#include <DHT22.h>
#define DHT22_PIN 7//7号脚不接任何电阻
DHT22 myDHT22(DHT22_PIN);
float h,t;
const unsigned char OSS = 0;  // Oversampling Setting
int test=0;//测试模式开关
int ac1;
int ac2;
int ac3;
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int md;
long b5;
float p; 
TinyGPS gps;
SoftwareSerial GPRS(6,5); //RX, TX
SoftwareSerial ss(4,3);
String TEMPdata="";
#define apikey "oQ********************************fjgKEA"
unsigned long deviceid=411;
int Cnt = 1;
String sensorid="TEMP";
String sensor1id="PM25";
#define Apikey "tX***************************q2iNA4A"
unsigned long device1id = 05;
String sensor11id="GPSdata";
        bool newData = false; 
        char gps_year[8];  
        char gps_mon[3];  
        char gps_day[3];  
        char gps_hour[3];  
        char gps_min[3];  
        char gps_sec[3];        
        char gps_lon[20]={"\0"};  
        char gps_lat[20]={"\0"}; 
        char gps_heigh[20]={"\0"};
        char gps_sms[100];
        char Time_sms[20]={"\0"};
        char Date_sms[20]={"\0"};
        char Lat_sms[20]={"\0"};
        char Lon_sms[20]={"\0"};



int c=2110;//轮子周长（2110）
int a=0;//计数器
#define NUMSAMPLES 5

int samplesT [NUMSAMPLES];
int samplesH [NUMSAMPLES];
String gprsStr = "";
int index = 0;
byte data1;
char atCommand[50];
byte mqttMessage[127];
int mqttMessageLength = 0;
//bolean flags
boolean smsReady = false;
boolean smsSent = false;
boolean gprsReady = false;
boolean mqttSent = false;


void setup() {
 pinMode(2, INPUT); 
 Serial.begin(115200);
 ss.begin(9600);
 h=0,t=0;
 GPRS.begin(9600);

}

void loop() {
 DHT22_ERROR_t errorCode;if( myDHT22.readData()== DHT_ERROR_NONE){t=myDHT22.getTemperatureC(); h=myDHT22.getHumidity();}  //读取DHT22的数据判断并赋予h,t.
 p= Speed();
 ss.listen();
 Check_gps();
 char ch=Serial.read();
 switch (ch){
    case '1':test=1;break;
    case '0':test=0;break;
    default:break;
}
if(test==0&&h>0){
   Serial.print("o");
   Serial.print(gps_lon);
   Serial.print("a");
   Serial.print(gps_lat);
   Serial.print("v");
   Serial.print(p);
   Serial.print("#"); 
   Serial.print(t);
   Serial.print("c");
   Serial.print(h);
   Serial.println("%"); 

 
}
  delay (15000); //Time to get GPRS Satable
  GPRS.listen();  
  Serial.println("Checking if GPRS is READy?");
  GPRS.println("AT");
  delay(1000);
  
  gprsReady= isGPRSReady();
  if(gprsReady == true){
    Serial.println("GPRS READY!");  
    String json= buildJson();
    char jsonStr[300];
    json.toCharArray(jsonStr,300);
    
    //*arguments in function are:
    //clientID, IP, Port, Topic, Message
  
   sendMQTTMessage("zuccmdf4", "thingworx.zucc.edu.cn", "1883", "zuccmqtt", jsonStr);
  }
  delay (15000);
}




float Speed(){
    unsigned long duration;
  float k;
  double sec=0;
  long last = millis();
  long nownow = millis();//记录当前时间
  while (last-nownow< 3000) {//连续测速3秒钟左右
  last = millis();
   duration = pulseIn(2, 0); //读取引脚上的低电平脉冲,且把结果赋值给duration变量
  sec= duration/1000000.0;
 if(sec>0&&sec<0.9){a++;}
 k=v(last,nownow,a);
  }
  a=0;
  return k;
  }


float  v(long last,long b,int a){
    long m=last-b;
   float k=(60000.0/m)*a*c*60/1000000;
    return k;
  }

  
void Check_gps()  
{   
  newData=false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      if (gps.encode(c)) 
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon, alti;
    unsigned long age;
    int _year;
    byte _month, _day,_hour,_minute,_second,_hundredths;
    gps.f_altitude();         
    gps.f_get_position(&flat, &flon, &age);
    gps.crack_datetime(&_year,&_month,&_day,&_hour,&_minute,&_second,&_hundredths,&age);
    flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
    flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
    alti=gps.f_altitude(); 
    dtostrf(flat, 11, 6, gps_lat); 
    dtostrf(flon, 10, 6, gps_lon); 
    dtostrf(alti, 0, 3, gps_heigh); 
    
        strcpy(gps_sms,"lat:");
        strcat(gps_sms,gps_lat);
        strcat(gps_sms,"\n");
        strcat(gps_sms,"lon:");
        strcat(gps_sms,gps_lon);
        strcat(gps_sms,"\n");
        strcat(gps_sms,"time:");
        
    itoa(_year,gps_year,10);
    strcat(gps_sms,gps_year);
        
    itoa(_month,gps_mon,10);
    if(strlen(gps_mon)==1)
      strcat(gps_sms,"0");
    strcat(gps_sms,gps_mon);
        
    itoa(_day,gps_day,10);
        if(strlen(gps_day)==1)
      strcat(gps_sms,"0");
    strcat(gps_sms,gps_day);
        
    itoa(_hour,gps_hour,10);
        if(strlen(gps_hour)==1)
      strcat(gps_sms,"0");
    strcat(gps_sms,gps_hour);
        
    itoa(_minute,gps_min,10);
        if(strlen(gps_min)==1)
      strcat(gps_sms,"0");
    strcat(gps_sms,gps_min);
        
    itoa(_second,gps_sec,10);
        if(strlen(gps_sec)==1)
      strcat(gps_sms,"0");
    strcat(gps_sms,gps_sec);        
    if(Cnt = 1)     Cnt++;
  }
}
boolean isGPRSReady(){
 GPRS.println("AT");
 GPRS.println("AT");
 GPRS.println("AT+CGATT?");
 index = 0;
 while (GPRS.available()){
    data1 = (char)GPRS.read();
    Serial.write(data1);
    gprsStr[index++] = data1;
    }
 if (data1 > -1){
  return true;
 }
 else {
  return false;
 }
}

void sendMQTTMessage(char* clientId, char* brokerUrl, char* brokerPort, char* topic, char* message){
 GPRS.println("AT"); // Sends AT command to wake up cell phone
 Serial.println("send AT to wake up GPRS");
 delay(1000); // Wait a second
// digitalWrite(13, HIGH);
 GPRS.println("AT+CSTT=\"CMENT\""); // Puts phone into GPRS mode
 Serial.println("AT+CSTT=\"CMENT\"");
 delay(3000); // Wait a second
 GPRS.println("AT+CIICR");
 Serial.println("AT+CIICR");
 delay(4000);
 GPRS.println("AT+CIFSR");
 Serial.println("AT+CIFSR");
 delay(3000);
 strcpy(atCommand, "AT+CIPSTART=\"TCP\",\"");
 strcat(atCommand, brokerUrl);
 strcat(atCommand, "\",\"");
 strcat(atCommand, brokerPort);
 strcat(atCommand, "\"");
 GPRS.println(atCommand);
 Serial.println(atCommand);
 // Serial.println("AT+CIPSTART=\"TCP\",\"mqttdashboard.com\",\"1883\"");
 delay(3000);
 GPRS.println("AT+CIPSEND");
 Serial.println("AT+CIPSEND");
 delay(2000);
 mqttMessageLength = 16 + strlen(clientId);
 Serial.println(mqttMessageLength);
 mqtt_connect_message(mqttMessage, clientId);
 for (int j = 0; j < mqttMessageLength; j++) {
 GPRS.write(mqttMessage[j]); // Message contents
 Serial.write(mqttMessage[j]); // Message contents
// Serial.println("");
 }
 GPRS.write(0x1a); // (signals end of message)
 Serial.println("Sent");
 delay(10000);
 GPRS.println("AT+CIPSEND");
 Serial.println("AT+CIPSEND");
 delay(2000);
 mqttMessageLength = 4 + strlen(topic) + strlen(message);
 Serial.println(mqttMessageLength);
 mqtt_publish_message(mqttMessage, topic, message);
 for (int k = 0; k < mqttMessageLength; k++) {
 GPRS.write(mqttMessage[k]);
 Serial.write((byte)mqttMessage[k]);
 }
 GPRS.write(0x1a); // (signals end of message)
 Serial.println("-------------Sent-------------"); // Message contents
 delay(5000);
 GPRS.println("AT+CIPCLOSE");
 Serial.println("AT+CIPCLOSE");
 delay(2000);
}

String buildJson() {
  String data = "{";
  data+="\"HTU_T\":\"";
  data+=String(t);
  data+= "\",";
  
  data+="\"HTU_H\":\"";
  data+=String(h);
  data+= "\",";
 
  data+="\"firstspeed\":\"";
  data+=String(p);
  data+= "\",";

  data+="\"lastspeed\":\"";
  data+=String(p);
  data+= "\",";

  data+="\"gps\":\"";
  data+=String(gps_lat);
  data+= ",";
  data+=String(gps_lon);
  data+="\"";
  data+="}";
  
  return data;
}

