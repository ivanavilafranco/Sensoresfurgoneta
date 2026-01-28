#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include "SSD1306Wire.h"

// ================= CONFIGURACIÓN PANTALLA =================
SSD1306Wire display(0x3C, 14, 12); // SDA=D5=14, SCL=D6=12

// ================= MAC de la pantalla =================
String macPantalla = "";

// ================= ESTRUCTURA DE DATOS =================
typedef struct {
  uint8_t id;
  float temperatura;
  float humedad;
} datos_dht;

typedef struct {
  uint8_t id;
  bool salon;    // PIR salón
  bool trasera;  // PIR trasera
} datos_pir;

// Variables globales
datos_dht dhtRecibido;
datos_pir pirRecibido;

// ================= CALLBACK RECEPCIÓN =================
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  uint8_t id = incomingData[0];
  if (id == 1) memcpy(&dhtRecibido, incomingData, sizeof(datos_dht));
  else if (id == 2) memcpy(&pirRecibido, incomingData, sizeof(datos_pir));
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  display.init();
  display.clear();
  display.flipScreenVertically();

  macPantalla = WiFi.macAddress();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error ESP-NOW");
    while(true);
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

// ================= LOOP =================
void loop() {
  display.clear();

  // ---------------- Mostrar alarma ----------------
  if (pirRecibido.salon) {
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "ALERTA");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 32, "SALON");
    display.display();
    return;
  } 
  else if (pirRecibido.trasera) {
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, "ALERTA");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 32, "TRASERA");
    display.display();
    return;
  }

  // ---------------- Mostrar información normal ----------------
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "MAC: " + macPantalla);
  display.drawLine(0, 12, 127, 12);
  display.drawString(0, 14, "Sensor DHT11:");
  display.drawString(0, 24, "Temp: " + String(dhtRecibido.temperatura) + " C");
  display.drawString(0, 36, "Humedad: " + String(dhtRecibido.humedad) + " %");

  display.display();
  delay(200);
}
