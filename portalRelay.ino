#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>



//------------------------------------------
//SETUP LED
long time_led;
long time_nb_led;
long time_ip_led;
int count;
#define LED_DELAY 5000
#define LED_IP_DELAY 500
#define LED_NB_DELAY 100
#define LED_PIN 2



//------------------------------------------
//SETUP RELAY
#define RELAY3_PIN D2

//------------------------------------------
//SETUP BUTTON
#define BUTTON_PIN D3
#define BUTTON_DELAY 300


    


long t_button;

//------------------------------------------
//WIFI i server HTTP
ESP8266WebServer server(80);
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define HOST_NAME "onoff"
// Set your Static IP address



bool button_last;
long t_now;


void setup() {
  //Setup pin mode
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT); //Initialize onboard LED as output
  pinMode(RELAY3_PIN, OUTPUT);
  SW_all_off(); //Turn off all relays
  button_last = HIGH;

  //Setup UART
  Serial.begin(115200);
  Serial.println("");
  Serial.println("START");

  //Setup WIFI 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname( HOST_NAME );
  Serial.print("Connecting to WIFI");

  //Wait for WIFI connection
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println( WIFI_SSID );
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIP()[3]);


  //Setup HTTP server
  server.on("/", handleRoot);
  server.on("/sw", handleSW);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(RELAY3_PIN, LOW);
  digitalWrite(BUTTON_PIN, HIGH);
  
}
 
void loop() {
  
  t_now = millis();
  
  server.handleClient();

  //Blinking an LED
  if( t_now - time_led > LED_DELAY ){
    if( t_now - time_nb_led > LED_NB_DELAY ){
      if( count<2*9 ){
        time_nb_led = t_now; 
        Serial.println("lefPin");
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        count ++;
      }
    }
    time_led = t_now;
    count = 0;
  }
  

  //Check button
  bool b = digitalRead( BUTTON_PIN );
  //Serial.println("50HZ");
  if( b != button_last && t_now - t_button > BUTTON_DELAY ){
    if( b==HIGH && button_last==LOW ){
      Serial.println("BUTTON");
    }

    t_button = t_now;
    button_last = b;
  }
}



void SW_all_off(){

Serial.println("50HZ");
  //50 Hz PWM
    for (int i = 0; i <= 10; i++) {
  digitalWrite(RELAY3_PIN, HIGH); 
  delay(30);
  digitalWrite(RELAY3_PIN, LOW); 
  delay(30);
  }
}




void handleRoot(){
  // Read battery voltage
  int adcValue = analogRead(A0); // Read the ADC value (0-1023)
  float voltage = adcValue* 2 /333 ; // Calculate the battery voltage

  String html = "<!DOCTYPE html>\r\n";
  html += "<html>\r\n";
  html += "<head>\r\n";
  html += "<meta charset='UTF-8'>\r\n";
  html += "<title>ESP8266</title>\r\n";
  html += "<body>\r\n";

  html += "ESP8266 - ";
  html += HOST_NAME;
  html += "\r\n";
  // Add battery voltage to the HTML
  html += "<p>Battery Voltage: ";
  html += String(voltage);
  html += " V</p>\r\n";
  if( !digitalRead(RELAY3_PIN) ){
    html += "<a href='sw?sw=off' style='display: inline-block; width: 100px; background-color: red; text-align: center;'>OFF</a>\r\n";
  }else{
    html += "<a href='sw?sw=on' style='display: inline-block; width: 100px; background-color: green; text-align: center;'>ON</a>\r\n";
  }
  
  //html += "<br>\r\n";
  //html += "<a href='sw?sw=on'>LED On</a>\r\n";
  //html += "<a href='sw?sw=off'>LED Off</a>\r\n";
  
  html += "</body>\r\n";
  html += "</html>\r\n";

  
  server.send(200, "text/html", html);
}

void handleSW(){

  if (server.arg("sw")== "on"){  
Serial.println("ifHigh");
pinMode(BUTTON_PIN, LOW); 
SW_all_off();


  }else{  
Serial.println("elseLow");
SW_all_off();
digitalWrite(RELAY3_PIN, LOW); 
delay(30);
  }

  String ip = WiFi.localIP().toString();
  server.sendHeader("Location", String("http://") + ip, true);
  server.send ( 302, "text/plain", "");
  server.client().stop();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
