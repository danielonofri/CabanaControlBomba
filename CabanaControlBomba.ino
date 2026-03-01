#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

const char* ssid = "Ali";
const char* password = "tanguito2";

// El puerto 82 que definiste
ESP8266WebServer server(82);

// CAMBIO VITAL: Usamos el pin RX (GPIO3) para que el ESP arranque siempre
const int relayPin = 3;  

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000); // GMT-3

String lastOn = "Esperando dato...";
String lastOff = "Esperando dato...";
String connectTime = "Aún no conectado";
bool tiempoInicializado = false;

// Variables para Wi-Fi no bloqueante
unsigned long tiempoAnteriorWiFi = 0;
const long intervaloWiFi = 10000;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='10'>";
  html += "<title>Cabana Bomba</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; background-color: #f4f4f9; margin: 0; padding: 20px; }";
  html += ".card { background: white; padding: 25px; border-radius: 20px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); max-width: 400px; margin: auto; }";
  html += "h1 { font-size: 24px; color: #2c3e50; }";
  html += ".status { font-size: 28px; font-weight: bold; margin: 20px 0; padding: 15px; border-radius: 10px; }";
  html += ".on { background-color: #d4edda; color: #155724; }";
  html += ".off { background-color: #e2e3e5; color: #383d41; }";
  html += ".data-box { text-align: left; background: #f8f9fa; padding: 15px; border-radius: 10px; font-size: 16px; line-height: 1.6; }";
  html += "</style></head><body>";
  
  html += "<div class='card'><h1>Monitor Cabaña</h1>";

  int bombaState = digitalRead(relayPin); // Lógica Inversa en el pin RX
  if (bombaState == LOW) { 
    html += "<div class='status on'>Bomba: ENCENDIDA</div>";
  } else {
    html += "<div class='status off'>Bomba: APAGADA</div>";
  }

  html += "<div class='data-box'>";
  html += "<b>Últimos Eventos:</b><br>";
  html += "🟢 On: " + lastOn + "<br>";
  html += "⚪ Off: " + lastOff + "<br><hr>";
  html += "⏱️ Uptime: <span id='uptime'></span>";
  html += "</div></div>";
  
  // Acá está el script de Uptime rescatado para que funcione en pantalla
  html += "<script>";
  html += "var startTime = new Date('" + connectTime + "');";
  html += "function updateUptime(){";
  html += "  var now = new Date();";
  html += "  if(isNaN(startTime)) { document.getElementById('uptime').innerHTML = 'Esperando red...'; return; }";
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
  // Desconectamos la "oreja" del RX para evitar cuelgues, pero nos sigue hablando por Serial
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  
  pinMode(relayPin, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Chau al while(WiFi.status() != WL_CONNECTED). Ahora no se bloquea.

  timeClient.begin();
  server.on("/", handleRoot);
  server.begin();
  
  Serial.println("\nSistema Iniciado.");
}

void loop() {
  // --- 1. GESTIÓN WI-FI (SEGUNDO PLANO) ---
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
    timeClient.update();
    
    // Solo toma la hora de conexión la primera vez que se engancha a la red
    if (!tiempoInicializado && timeClient.isTimeSet()) {
      time_t rawTime = timeClient.getEpochTime();
      struct tm* timeInfo = localtime(&rawTime);
      char buffer[25];
      strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeInfo);
      connectTime = String(buffer);
      tiempoInicializado = true;
    }
  } else {
    // Si se corta el Wi-Fi, intenta reconectar sin frenar el programa
    unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnteriorWiFi >= intervaloWiFi) {
      WiFi.begin(ssid, password);
      tiempoAnteriorWiFi = tiempoActual;
    }
  }

  // --- 2. LECTURA DE LA BOMBA (Funciona SIEMPRE, con o sin red) ---
  int currentRelayState = digitalRead(relayPin);
  static int lastRelayState = -1;

  if (currentRelayState != lastRelayState) {
    String horaEvento = "Guardado sin red"; // Mensaje por defecto si falla el Wi-Fi
    
    // Solo pide la hora real si hay internet
    if (WiFi.status() == WL_CONNECTED && timeClient.isTimeSet()) {
      time_t rawTime = timeClient.getEpochTime();
      struct tm* timeInfo = localtime(&rawTime);
      char buffer[20];
      strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", timeInfo);
      horaEvento = String(buffer);
    }

    if (currentRelayState == LOW) { 
      lastOn = horaEvento; // Cambió a Encendido
    } else if (lastRelayState != -1) { 
      lastOff = horaEvento; // Cambió a Apagado
    }
    lastRelayState = currentRelayState;
  }
  
  delay(100); // Pequeña pausa para estabilidad
}