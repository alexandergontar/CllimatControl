#include<ESP8266WiFi.h>
#include<DHT.h>
#include<EEPROM.h>

#define DHTPIN D7
#define DHTTYPE DHT22

WiFiServer server(80);
WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
int ledPin = D3;
int value = LOW;
int i = 0;
int pos1, pos2;
int temp = 0, humid = 0;
bool Auto = false;
int x;
int setTemp = 0;
int clk = 0;
String conStatus = "", showForm = "hidden";
String wifiid = "", wifipass = "ddd";
String prefix = "";
String refresh_page = "";
//const char* ssid = "test"; //your WiFi Name
//const char* password = "crisismanagement01";  //Your Wifi Password

void writeString(char addr, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.write(addr + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String read_String(char addr)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(addr);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(addr + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

void wifiConnect(String ssidStr, String passStr)
{
  WiFi.mode(WIFI_STA);
  int i = 0;
  char ssidBuf[ssidStr.length() + 1]; ssidStr.toCharArray(ssidBuf, ssidStr.length() + 1);
  char passBuf[passStr.length() + 1]; passStr.toCharArray(passBuf, passStr.length() + 1);
  const char* ssid = "test"; //your WiFi Name
  const char* password = "crisismanagement01";  //Your Wifi Password
  WiFi.begin(ssidBuf, passBuf);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
    delay(500);
    i++;
    if (i == 30) {
      IPAddress ip = WiFi.localIP();
      
      conStatus = "wi-fi not connected "+(String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3]+" error: "+(String)WiFi.status();
      WiFi.mode(WIFI_AP);
      return;
    }
  }
  IPAddress ip = WiFi.localIP();
  WiFi.mode(WIFI_AP_STA);
  delay(500);
  conStatus = "wi-fi connected: " + (String)ip[0] + "." + (String)ip[1] + "." + (String)ip[2] + "." + (String)ip[3];
}
void setup()
{
  EEPROM.begin(512);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);
  server.begin();
  wifiid = read_String(10);
  wifipass =  read_String(60);
  //WiFi.mode(WIFI_AP_STA);

  wifiConnect(wifiid, wifipass);
  WiFi.softAP("ESP8266", "Suchara1968");

}
void loop()
{
  //Serial.println(Auto);
  //Serial.println(wifiid);
  //Serial.println(wifipass);
  if (Auto)
  {
    delay(1);
    if (clk >= 15000) {
      clk = 0;
      temp = dht.readTemperature();
      delay(50);
      if (temp >= setTemp + 2) {
        value = LOW;
        digitalWrite(ledPin, LOW);
      }
      if (temp <= setTemp - 2) {
        value = HIGH;
        digitalWrite(ledPin, HIGH);
      }
    }
    clk++;
  }
  //-----------------------------------------
  if (!client)   client = server.available();
  if (!client)  return;
  i = 1;
  while (!client.available())
  {
    delay(1);
    i++;
    if (i == 3000) {
      i = 0;
      client.stop();
      return;
    }
  }
  client.setTimeout(500);
  String request = client.readString();
  // Serial.print(request);
  char inputChar;
  String strDig = "";
  for (int k = 0; k < 15; k++) {
    inputChar = request[k]; if (isDigit(inputChar)) strDig += inputChar;
  }
  if (strDig.length() > 0)x = strDig.toInt();
  // Serial.println(x);

  temp = dht.readTemperature();
  delay(50);
  humid = dht.readHumidity();
  delay(50);
  if (request.indexOf("HTTP/1.1") != -1)
  {
    //Auto=false;
    if (request.indexOf("?K=555") != -1)
    {
      showForm = "visible";
    }
    if (request.indexOf("?L=6") != -1 && request.indexOf("?L=6") <= 20)
    {
      Serial.println(wifiid); Serial.println(wifipass);
      pos1 = request.indexOf("&username=") + 10;
      pos2 = request.indexOf("&password=");
      wifiid = request.substring(pos1, pos2);
      wifiid.replace("+"," ");
      pos1 = request.indexOf("&password=") + 10;
      pos2 = request.indexOf("&endmark=777");
      wifipass = request.substring(pos1, pos2);
      showForm = "hidden";
      Serial.println(wifiid);
      Serial.println(wifipass);
      wifiConnect(wifiid, wifipass);
      writeString(10, wifiid);
      writeString(60, wifipass);
    }
    if (request.indexOf("?H=111") != -1 && request.indexOf("?H=111") <= 20) {
      Auto = false;
      value = HIGH;
      digitalWrite(ledPin, HIGH);
    }
    if (request.indexOf("?H=000") != -1 && request.indexOf("?H=000") <= 20) {
      Auto = false;
      value = LOW;
      digitalWrite(ledPin, LOW);
    }
    if (request.indexOf("?S=") != -1 && request.indexOf("?S=") <= 20) {
      Auto = true;
      setTemp = x;
    }
    if (request.indexOf("?refresh=on") != -1 && request.indexOf("?refresh=on") <= 20) {
      refresh_page = "checked";
    }
    else refresh_page = "";
    String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
    s += "<html>\r\n";
    s += "<head><title> Web Port </title> <style> #popup{visibility: " + showForm + "; border: solid#000000 2px; width: 20%; padding: 10px;}form {display: inline;} div {border: 3px solid #000; margin: 10%; padding: 10px;} body {margin: 0;}.parent{margin: 10px;}</style>";
    s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\" >";
    s += "<meta charset=\"UTF-8\" >";
    if (refresh_page == "checked") s += "<meta  http-equiv=\"refresh\" content=\"10\">";
    s += " </head><body><div>\r\n";
    // начало тела web-страницы
    s += "<center><h1 style=\"color:#00ff00; font-size:20px;\">Arduino ESP8266 Web Interface</h1></center>\r\n";
    s += "<center><h2 style=\"color:#ff0000; font-family:verdana, sans-serif;\">Temperature is:  " + (String)temp + " grad. C.</h2></center>\r\n";
    s += "<center><h2 style=\"color:#0000ff; background:gold;\">Humidity is: " + (String)humid + " %</h2></center>\r\n";
    s += "<center><h3 style=\"color:#00ff00; font-size:20px;\">" + conStatus + "</h3></center>\r\n";
    s += "<form action='' method='get'><input type='hidden' name='K' value='555' /><input type='submit' value='Wi-Fi options'  /></form>";
    if (showForm == "visible")
    {
      s += "<center><form action='' id='popup' method='get' style =\"display : block;\"><input type='hidden' name='L' value='6' /><br>";
      s += "SSID:<br> <input type=\"text\" name=\"username\"><br>";
      s += "Password:<br> <input type=\"password\" name=\"password\"><br>";
      s += "<input type='hidden' name='endmark' value='777' />";
      s += "<input type='submit' value='Save' style=\"font-size:13pt; color:white; background-color:green;\" />\n</form></center></h2>\n";
    }

    s += "<h2><form action='' method='get'><input type='hidden' name='H' value='111' /><input type='submit' value='Heater ON' style=\"font-size:13pt; color:white; background-color:green;\" /></form>  ";
    s += "<form action='' method='get'><input type='hidden' name='H' value='000' /><input type='submit' value='Heater OFF' style=\"font-size:13pt; color:white; background-color:red;\" />\n</form></h2>\n";
    s += "<span id=\"slider_value\" style=\"color:red;\"></span>";
    s += "<h2><form action='' method='get' ><input type='range' name='S' min='0' max='100' step='1'value='0' oninput=\"show_value(this.value);\" /><input type='submit' value='Set Temperature' style=\"font-size:13pt; color:white; background-color:blue;\" />\n</form></h2>\n";
    s += "<h2><form action='' method='get' ><input type='checkbox' name='refresh'" + refresh_page + " \" /><input type='submit' value='Refresh automatically' style=\"font-size:13pt; color:white; background-color:brown;\" />\n</form></h2>\n";
    s += "<script> function show_value(x){document.getElementById(\"slider_value\").innerHTML=x;}</script>";
    if (Auto)s += "<h2 style=\"color:#0000ff; background:gold;\">Auto mode, Set Temperature: " + (String)setTemp + " </h2>\r\n";
    if (value == HIGH) {
      s += "<center><h2 style=\"color:#ff0000; background:gold;\">Now Heater is ON</h2></center>\r\n";
    }
    else {
      s += "<center><h2 style=\"color:#000000; background:gold;\">Now Heater is OFF</h2></center>\r\n";
    }
    // s+=request;
    //s+="\n\n..."+wifiid+"...<br>..."+wifipass+"...";
    // конец тела web-страницы
    s += "</div></body></html>\n";
    client.print(s);
    delay(10);
    client.stop();
    //Serial.println(Auto);
    return;
  }

  if (request.indexOf("/LED=ON") != -1)
  {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
    Auto = false;
  }
  if (request.indexOf("/LED=OFF") != -1)
  {
    digitalWrite(ledPin, LOW);
    value = LOW;
    Auto = false;
  }
  if (value == HIGH) {
    digitalWrite(ledPin, HIGH);
    prefix = "LED is ON, ";
  } else {
    digitalWrite(ledPin, LOW);
    prefix = "LED is OFF, ";
  }
  if (request.indexOf("/LED=Auto") != -1)
  {
    Auto = true;
    setTemp = x;
    prefix += " Auto, Set Point is:" + (String)setTemp;
  }

  String data = prefix + " temperature is: " + (String)temp + " grad. C, Humidity is: " + (String)humid + " %";
  client.println(data);
  delay(10);
  client.flush();
  client.stop();
}
