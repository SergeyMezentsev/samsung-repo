#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>

/* SSID и пароль точки доступа esp8266*/
const char* ssidOfESP = "Teacher";  // SSID that esp8266 will generate
const char* passwordOfESP = "12345678"; // password that esp8266 will generate

/* Настройки IP адреса */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Объект веб-сервера. Будет прослушивать порт 80 (по умолчанию для HTTP)
ESP8266WebServer server(80); 



//variables for data collecting
String phoneSSID = "";
String phonePass = "";
bool getPararmsFlag = false;

String startPageHTML = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta charset = "utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>MainModule</title>
    <style type="text/css">
      .button {
        background-color: #4CAF50;
        border: none;
        color: white;
        border-radius: 6px;
        padding: 12px 24px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
      }

      .commonText {
        font-size: 18px;
      }
    </style>
  </head>
  <body style="background-color: #cccccc; Color: blue; ">
    <center>
      <h1>Настройка параметров подключения</h1>
      <form action="params">
      <p class="commonText">Имя сети: <input type="text" name="SSID" class="commonText"></p>
      <p class="commonText">Пароль сети: <input type="text" name="pass" class="commonText"></p>
      <input type="submit" value="Отправить" class="button">
      </form>
    </center>
  </body>
</html>
)=====";


void handleRoot()
{
server.send( 200, "text/html", startPageHTML);
}


void getPhoneData()
{
 
  for(int i = 0; i < server.args(); i++)
  {
    if(server.argName(i) == "SSID" && server.arg(i) != "")
    {
      phoneSSID = server.arg(i);
      getPararmsFlag= true;
      Serial.print("SSID: "); Serial.println(phoneSSID);
    }
    if(server.argName(i) == "pass" && server.arg(i) != "")
    {
      phonePass = server.arg(i);
      getPararmsFlag= true;
      Serial.print("Pass: "); Serial.println(phonePass);
    }
  }

  if(getPararmsFlag)
  {
    String lastPage = R"=====(
    <!DOCTYPE html>
    <html>
      <head>
        <meta charset="utf-8">
        <meta name="author" content="Serjuice">
        <meta name="description" content="Samsung project page">
        <meta name="keywords" content="html, Samsung, test">
        <title>Завершение</title>
      </head>
      <body style="background-color: #cccccc; Color: blue; ">
        <center>
          <br><br><br>
          <h1>Параметры сети были успешно переданы</h1><br>
          <h2>Модуль будет подключён через 15 секунд</h2>
        </center>   
      </body>
    </html>
    )=====";
    server.send(200, "text/html", lastPage);    // возвращаем HTTP-ответ


    //Here we need save phoneSSID, phonePass and getParamFlag in EEprom

  
   //Delay for 15 seconds
   //delay(15000);


   //Reset the esp8266

    
  }
  else
  {
    server.send( 200, "text/html", startPageHTML);
  }
}


void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}




  

void setup() 
{
  Serial.begin(115200);
  delay(100);

  WiFi.softAP(ssidOfESP, passwordOfESP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/params", getPhoneData);            // привязать функцию обработчика к URL-пути
  server.on("/", handleRoot);
  server.onNotFound(handle_NotFound);
  
  server.begin();                                // запуск сервера
  Serial.println("HTTP server started");
}

void loop() 
{
  server.handleClient();    // обработка входящих запросов

}
