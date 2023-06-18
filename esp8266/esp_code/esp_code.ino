#include <EEPROM.h>
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>

#define BTN_RESET_PIN 4
#define BLUE_LED_PIN 5
#define RED_LED_PIN 123

//-----------------------------------------------Web-server settings-----------------------------------------------
/* SSID и пароль точки доступа esp8266*/
const char* ssidOfESP = "Teacher";  // SSID that esp8266 will generate
const char* passwordOfESP = "12345678"; // password that esp8266 will generate

/* Настройки IP адреса */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Объект веб-сервера. Будет прослушивать порт 80 (по умолчанию для HTTP)
ESP8266WebServer server(80); 
//-----------------------------------------------------------------------------------------------------------------


//-----------------------------------------------Structure to collect phone params-----------------------------------------------
struct TParams
{
  char teacherSSID[21];
  char teacherPass[21];
  int numberOfModules;
  bool getFlag = false;
} params, newParams;
//-----------------------------------------------------------------------------------------------------------------

//-----------------------------------------------Clear EEPROM and RESET ESP8266-----------------------------------------------
void clearEEPROM()
{
  struct TParams cleanParams;
  for(int i = 0; i < 21; i++)
  {
    cleanParams.teacherSSID[i] = 0;
    cleanParams.teacherPass[i] = 0;
  }
  cleanParams.numberOfModules = 0;
  cleanParams.getFlag = false;

  EEPROM.put(0, cleanParams);   // Put structure "params" on Zero Address
  EEPROM.commit();              //Commit writing to EEPROM
}

void restartESP()
{
  Serial.println("Restarting ESP...");
  ESP.restart();
}
//-----------------------------------------------------------------------------------------------------------------



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
      <p class="commonText">Имя сети: <input type="text" name="SSID" class="commonText" maxlength="20"></p>
      <p class="commonText">Пароль сети: <input type="text" name="pass" class="commonText" maxlength="20"></p>
      <p class="commonText">Число детских модулей: <input type="number" name="number" class="commonText"></p>
      <input type="submit" value="Отправить" class="button">
      </form>
    </center>
  </body>
</html>
)=====";


void handleRoot()
{
  server.send( 200, "text/html", startPageHTML);
  
  
  //Здесь просто чтение EEPROM для отладки
  //По факту структура newParams не нужна
  EEPROM.get(0, newParams);   // прочитать из адреса 0

  Serial.println("-----------------------");
  Serial.print("newParams.teacherSSID: ");
  Serial.println(newParams.teacherSSID);
  Serial.print("newParams.teacherPass: ");
  Serial.println(newParams.teacherPass);
  Serial.print("newParams.numberOfModules: ");
  Serial.println(newParams.numberOfModules);
  Serial.print("newParams.getFlag: ");
  Serial.println(newParams.getFlag);
  Serial.println("-----------------------");
}


void getPhoneData()
{
  byte counter = 0;
  for(int i = 0; i < server.args(); i++)
  {
    if(server.argName(i) == "SSID" && server.arg(i) != "")
    {
      String phoneSSID = "";
      phoneSSID = server.arg(i);
      phoneSSID.toCharArray(params.teacherSSID, 20);
      counter++;
      Serial.print("params.teacherSSID: "); Serial.println(params.teacherSSID);
    }
    else if(server.argName(i) == "pass" && server.arg(i) != "")
    {
      String phonePass = "";
      phonePass = server.arg(i);
      phonePass.toCharArray(params.teacherPass, 20);
      counter++;
      Serial.print("params.teacherPass: "); Serial.println(params.teacherPass);
    }
    else if (server.argName(i) == "number" && server.arg(i) != "")
    {
      String numberModules = "";
      numberModules = server.arg(i);
      params.numberOfModules = numberModules.toInt();
      counter++;
      Serial.print("params.numberOfModules: "); Serial.println(params.numberOfModules);
    }
  }

  if(counter > 2 && params.numberOfModules < 33) params.getFlag = true;

  if(params.getFlag)
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
        <style>
          .commonText {
            font-size: 25px;
          }
        </style>
      </head>
      <body style="background-color: #cccccc; Color: blue; ">
        <center>
          <br><br><br>
          <h1 class="commonText">Параметры сети были успешно переданы</h1><br>
          <h2 class="commonText">Модуль будет подключён через 15 секунд</h2>
        </center>   
      </body>
    </html>
    )=====";
    server.send(200, "text/html", lastPage);    // возвращаем HTTP-ответ


    //Here we need save struct "params" to EEPROM
    EEPROM.put(0, params);   // Put structure "params" on Zero Address
    EEPROM.commit();         //Commit writing to EEPROM
  
   //Delay for 15 seconds
   delay(15000);


   //Reset the esp8266
   restartESP();
    
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
  EEPROM.begin(100);      //Start EEPROM

  pinMode(BTN_RESET_PIN, INPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, LOW);
  delay(100);

  EEPROM.get(0, params);  

  if(params.getFlag)
  {
    //If we have already data to connect to AP of the phone
    //We need to try to connect to it

    Serial.println("Connecting to ");
    Serial.println(params.teacherSSID);

    //const char* newSSID = params.teacherSSID;
    //const char* newPass = params.teacherPass;

    // подключиться к вашей локальной wi-fi сети
    WiFi.begin(params.teacherSSID, params.teacherPass);

    // проверить, подключился ли wi-fi модуль к wi-fi сети
    while (WiFi.status() != WL_CONNECTED) 
    {
      digitalWrite(BLUE_LED_PIN, HIGH);
      delay(500);
      digitalWrite(BLUE_LED_PIN, LOW);
      delay(500);
      Serial.print(".");

      if(digitalRead(BTN_RESET_PIN)== HIGH)
      {
        clearEEPROM();
        restartESP();
      }
    }
    digitalWrite(BLUE_LED_PIN, HIGH);
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

    
  }
  else
  {
    //If we don't have any data for AP of the phone
    //We need to start Web-server
    
    WiFi.softAP(ssidOfESP, passwordOfESP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
  
    server.on("/params", getPhoneData);            // привязать функцию обработчика к URL-пути
    server.on("/", handleRoot);
    server.onNotFound(handle_NotFound);
    
    server.begin();                                // запуск сервера
    Serial.println("HTTP server started");
  }
}

void loop() 
{
  if(params.getFlag)
  {
    //If we have already data to connect to AP of the phone
    //We need to try to connect to it

    Serial.println("Works!!!");
    delay(1000);

    
    if(digitalRead(BTN_RESET_PIN)== HIGH)
    {
      clearEEPROM();
      restartESP();
    }
    
  }
  else
  {
    //If we don't have any data for AP of the phone
    //We need to start Web-server
    server.handleClient();    // обработка входящих запросов
  }
 
}
