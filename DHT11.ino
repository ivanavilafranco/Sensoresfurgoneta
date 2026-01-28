#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>

// ================= CONFIGURACIÓN =================
#define DHTPIN D2
#define DHTTYPE DHT11
#define ID_SENSOR 1

uint8_t macPrincipal[] = {mac};

// ================= ESTRUCTURA DE DATOS =================
typedef struct {
  uint8_t id;
  float temperatura;
  float humedad;
} datos_dht;

datos_dht datos;

// ================= CALLBACK DE ENVÍO =================
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Estado envío DHT: ");
  Serial.println(sendStatus == 0 ? "OK" : "Fallo");
}

// ================= SETUP =================
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error inicializando ESP-NOW");
    while(true);
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(macPrincipal, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  datos.id = ID_SENSOR;
  datos.temperatura = 0;
  datos.humedad = 0;
}

// ================= LOOP =================
void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    datos.temperatura = temp;
    datos.humedad = hum;

    esp_now_send(macPrincipal, (uint8_t *)&datos, sizeof(datos));

    Serial.print("Temperatura: "); Serial.print(temp);
    Serial.print(" C, Humedad: "); Serial.print(hum); Serial.println(" %");
  } else {
    Serial.println("Error leyendo DHT11");
  }

  delay(2000);
}
