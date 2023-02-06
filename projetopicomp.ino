#include <SoftwareSerial.h>

#include <TinyGPS++.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <FirebaseESP8266.h> 

TinyGPSPlus gps;

SoftwareSerial ss(4,5); // aqui conecta no wifi

const char* ssid = "HRS";
const char* password = "sabrinaa123";
#define FIREBASE_HOST "esp8266-cb627-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "WgzS3oG5DR4dgUXjBtzE0eXp9YKL2XgF2hya3sfe"

FirebaseData firebaseData;

FirebaseJson json;

float latitude , longitude;

int year,month,date,hour,minute,second;
String date_str,time_str,lat_str,lng_str;

int pm;

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  ss.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  Serial.println("Connecting Firebase.....");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase OK.");
  
  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  server.begin();
  Serial.println("Start Started");

  //s

  Serial.println(WiFi.localIP());

}

void loop()

{
  while (ss.available()>0)
  if(gps.encode(ss.read()))
  {
    if(gps.location.isValid())
    {
      latitude = gps.location.lat();
      lat_str = String(latitude , 6);
      longitude = gps.location.lng();
      lng_str = String(longitude , 6);

      if(Firebase.setFloat(firebaseData, "/GPS/f_latitude", latitude))
      {print_ok();}
    else
      {print_fail();}
    //-------------------------------------------------------------
    if(Firebase.setFloat(firebaseData, "/GPS/f_longitude", longitude))
      {print_ok();}
    else{
      print_fail();
}

    }

    if(gps.date.isValid())

    {
      date_str = "";
      date = gps.date.day();
      month = gps.date.month();
      year = gps.date.year();

      if(date<10)
      date_str = '0';
      date_str += String(date);

      date_str += "/";

      if(month < 10)
      date_str += '0';
      date_str += String(month);

      date_str += "/";

      if(year<10)
      date_str += '0';
      date_str += String(year);
    }

    if(gps.time.isValid())
    {
      time_str = "";
      hour = gps.time.hour();
      minute = gps.time.minute();
      second = gps.time.second();

      minute = (minute + 30);
      if(minute>59)
      {
        minute = minute - 60;
        hour = hour + 1;
      }
      hour = (hour + 5);
      if (hour > 23)
      hour = hour - 24;

      if(hour>= 12)
      pm = 1;

      else 

      pm = 0;

      hour = hour % 12;

      if(hour < 10)
     time_str ='0';
     time_str += String(hour);

     time_str += ":";

     if(minute<10)
     time_str ='0';
     time_str += String(minute);

     time_str += ":";

     if(second<10)
     time_str ='0';
     time_str += String(second);

     if (pm == 1)
     time_str += "PM";
     else
     time_str += "AM";

    }

  }

  // verifica se esta conectado on wifi

  WiFiClient client = server.available();
  if(!client)
  {
    return;

  }

  //codigo html para criação do webserver

  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html><html><head><title>K7 de p.i</title><style>";
  s += "a:link {background-color: YELLOW;text-decoration: none;}";
  s += "table,th,td {border: 1px solid black;}</style></head><body><h1 style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER> BusLocation</h1>";
  s += "<p ALIGN=CENTER style=""font-size:150%;""";  
  s += "> <b>Localizacao</b></p><table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER>";
  s += lat_str;
  s += "</td></tr><tr><th>Longitude</th><td ALIGN=CENTER>";
  s += lng_str;
  s += "</td></tr><tr><th>Data</th><td ALIGN=CENTER>";
  s += date_str;
  s += "</td></tr><tr><th>Horário</th><td ALIGN=CENTER>";
  s += time_str;
  s += "</td></tr></table>";

  if(gps.location.isValid())
  {
   s += "<a href=\"http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
   s += lat_str;
   s += '+';
   s += lng_str;
   s += "\">Clique aqui para descobrir a localizacao do veiculo.</a>";
  }

   s += "</body> </html> \n";

  client.print(s);
  delay(100);
}

void print_ok()
{
    Serial.println("------------------------------------");
    Serial.println("OK");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
}

void print_fail()
{
    Serial.println("------------------------------------");
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
}

void firebaseReconnect()
{
  Serial.println("Trying to reconnect");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
