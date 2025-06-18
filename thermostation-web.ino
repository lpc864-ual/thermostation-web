// ========================================
// LIBRERÍAS
// ========================================
#include <DHT.h>             // Para leer el sensor
#include <WiFi.h>            // Para conectar a WiFi
#include <ESPAsyncWebSrv.h>  // Para el servidor web

// ========================================
// CONFIGURACIONES
// ========================================
// Configuración del sensor
#define DHT_PIN 4            // Pin donde conectamos DATA del DHT22
#define DHT_TYPE DHT22       // Tipo de sensor /
DHT dht(DHT_PIN, DHT_TYPE);  // Crear objeto sensor

// Configuración WiFi
const char* ssid = "Pon el nombre de la red";
const char* password = "Pon la contraseña de la red";

// Crear servidor en puerto 80 (HTTP estándar)
AsyncWebServer server(80);

// ========================================
// PÁGINA WEB
// ========================================
const char* pagina_web = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ThermoStation Web</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial; text-align: center; background: #f0f0f0; }
        .container { max-width: 400px; margin: 50px auto; }
        .sensor-card { 
            background: white; 
            margin: 20px; 
            padding: 20px; 
            border-radius: 10px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .valor { font-size: 2em; font-weight: bold; color: #2196F3; }
        .temperatura { color: #FF5722; }
        .humedad { color: #2196F3; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌡️ ThermoStation Web</h1>
        
        <div class="sensor-card">
            <h2>🌡️ Temperatura</h2>
            <div id="temperatura" class="valor temperatura">--°C</div>
        </div>
        
        <div class="sensor-card">
            <h2>💧 Humedad</h2>
            <div id="humedad" class="valor humedad">--%</div>
        </div>
        
        <p>Última actualización: <span id="ultima-actualizacion">--</span></p>
    </div>

    <script>
        function actualizarDatos() {
            fetch('/api/datos')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temperatura').innerHTML = data.temperatura + '°C';
                    document.getElementById('humedad').innerHTML = data.humedad + '%';
                    document.getElementById('ultima-actualizacion').innerHTML = new Date().toLocaleTimeString();
                })
                .catch(error => {
                    console.log('Error:', error);
                    document.getElementById('temperatura').innerHTML = 'Error';
                    document.getElementById('humedad').innerHTML = 'Error';
                });
        }
        
        // Actualizar al cargar la página
        actualizarDatos();
        
        // Actualizar cada 5 segundos
        setInterval(actualizarDatos, 5000);
    </script>
</body>
</html>
)";

// ====================
// FUNCIONES AUXILIARES
// ====================
String obtenerDatosSensor() {
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // Crear JSON simple
  String json = "{";
  json += "\"temperatura\":" + String(temperatura) + ",";
  json += "\"humedad\":" + String(humedad);
  json += "}";

  return json;
}

// ========================================
// SETUP - Se ejecuta UNA vez al encender
// ========================================
void setup() {
  // Establecemos conexión entre el PC y el chip
  Serial.begin(115200);

  // Esperar a que se establezca la conexión serie
  delay(2000);

  Serial.println("🌡️ Iniciando ThermoStation Web...");

  dht.begin();
  Serial.println("✅ Sensor DHT11 inicializado");

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("🔗 Conectando a WiFi");

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 10) {
    delay(1000);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi conectado!");
    Serial.print("🌐 IP del servidor: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("❌ No se pudo conectar al WiFi");
    return;
  }


  // Ruta principal: mostrar página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", pagina_web);
  });

  // Ruta API: enviar datos en formato JSON
  server.on("/api/datos", HTTP_GET, [](AsyncWebServerRequest* request) {
    String json = obtenerDatosSensor();
    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("🚀 Servidor web iniciado en puerto 80");
  Serial.println("💡 Abre tu navegador y ve a: http://" + WiFi.localIP().toString());
}

// ========================================
// LOOP - Se ejecuta continuamente
// =======================================
void loop() {

}
