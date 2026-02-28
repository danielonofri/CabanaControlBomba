#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

const char* ssid = "Ali";
const char* password = "tanguito2";

ESP8266WebServer server(80);

// CAMBIO: Ahora usamos GPIO2 (que tiene tu R de 10k externa)
const int relayPin = 2;  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000); // GMT-3

String lastOn = "Esperando dato...";
String lastOff = "Esperando dato...";
String connectTime = "";

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='10'>"; // Auto-refresh cada 10s
  html += "<title>Cabaña Bomba</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', sans-serif; text-align: center; background-color: #f4f4f9; color: #333; }";
  html += ".card { background: white; padding: 20px; border-radius: 15px; display: inline-block; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }";
  html += ".on { color: #27ae60; font-weight: bold; font-size: 1.5em; }";
  html += ".off { color: #7f8c8d; font-weight: bold; font-size: 1.5em; }";
  html += "h2 { border-bottom: 2px solid #3498db; padding-bottom: 5px; }";
  html += "</style></head><body>";
  
  html += "<div class='card'><h1>Control Bomba Cabaña</h1>";

  // LEER ESTADO (Lógica Inversa)
  int bombaState = digitalRead(relayPin);
  
  if (bombaState == LOW) { // El pin está en 0V -> Relé activo
    html += "Estado actual: <span class='on'>● ENCENDIDA</span><br>";
  } else {                 // El pin está en 3.3V (vía R 10k) -> Relé inactivo
    html += "Estado actual: <span class='off'>○ APAGADA</span><br>";
  }

  html += "<h2>Historial de Ciclo</h2>";
  html += "<b>Último Encendido:</b> " + lastOn + "<br>";
  html += "<b>Último Apagado:</b> " + lastOff + "<br>";

  html += "<h2>Sistema</h2>";
  html += "Conectado desde: " + connectTime + "<br>";
  html += "Tiempo activo: <span id='uptime' style='font-weight:bold;'></span>";
  html += "</div>";

  // Script mejorado para Uptime
  html += "<script>";
  html += "var startTime = new Date('" + connectTime + "');";
  html += "function updateUptime(){";
  html += "  var now = new Date();";
  html += "  var diff = Math.floor((now - startTime)/1000);";
  html += "  var days = Math.floor(diff/86400); diff%=86400;";
  html += "  var hours = Math.floor(diff/3600); diff%=3600;";
  html += "  var minutes = Math.floor(diff/60); diff%=60;";
  html += "  var seconds = diff;";
  html += "  document.getElementById('uptime').innerHTML = days+'d '+hours+'h '+minutes+'m '+seconds+'s';";
  html += "}";
  html += "setInterval(updateUptime,1000); updateUptime();";
  html += "</script>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  // GPIO2 como entrada. 
  // Nota: Al arrancar, el ESP8266 necesita GPIO2 en HIGH. Tu R de 10k lo garantiza.
  pinMode(relayPin, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  timeClient.begin();
  timeClient.update();

  // Guardar hora de conexión inicial
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeInfo = localtime(&rawTime);
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeInfo);
  connectTime = String(buffer);

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();

  // Lógica para detectar el cambio de estado y guardar la hora
  int currentRelayState = digitalRead(relayPin);
  static int lastRelayState = -1;

  if (currentRelayState != lastRelayState) {
    time_t rawTime = timeClient.getEpochTime();
    struct tm* timeInfo = localtime(&rawTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", timeInfo);

    if (currentRelayState == LOW) { // Cambió a Encendido
      lastOn = String(buffer);
    } else if (lastRelayState != -1) { // Cambió a Apagado (evitamos el primer arranque)
      lastOff = String(buffer);
    }
    lastRelayState = currentRelayState;
  }
  delay(100); // Pequeña pausa para estabilidad
}