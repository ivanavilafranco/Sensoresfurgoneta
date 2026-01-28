
#include <ESP8266WiFi.h>
#include <espnow.h>

// ================= PINES =================
#define PIR_SALON_PIN 12   // D6
#define PIR_TRAS_PIN  13   // D7
#define BUZZER_PIN    14   // D5

#define ID_SENSOR 2       // ID del módulo PIR

uint8_t macPrincipal[] = {mac};

// ================= ESTRUCTURA DE DATOS =================
typedef struct {
  uint8_t id;
  bool salon;
  bool trasera;
} datos_pir;

datos_pir datos;

// ================= CALLBACK DE ENVÍO =================
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Estado envío PIR: ");
  Serial.println(sendStatus == 0 ? "OK" : "Fallo");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(PIR_SALON_PIN, INPUT);
  pinMode(PIR_TRAS_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

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
  datos.salon = false;
  datos.trasera = false;

  Serial.println("⏳ Esperando estabilización de PIR...");
  delay(30000);
  Serial.println("✅ PIR listos");
}

// ================= LOOP =================
void loop() {
  static bool estadoSalonAnterior = false;
  static bool estadoTrasAnterior = false;
  static unsigned long ultimoEnvio = 0;

  bool estadoSalon = digitalRead(PIR_SALON_PIN);
  bool estadoTras = digitalRead(PIR_TRAS_PIN);

  // ================= Activar buzzer con tonos distintos =================
  if (estadoSalon && estadoTras) {
    // Ambos activados → alternar tonos rápido
    for(int i=0;i<5;i++){
      tone(BUZZER_PIN, 2000);
      delay(50);
      tone(BUZZER_PIN, 3000);
      delay(50);
    }
  } else if (estadoSalon) tone(BUZZER_PIN, 2000); // Salón
  else if (estadoTras)  tone(BUZZER_PIN, 3000);    // Trasera
  else noTone(BUZZER_PIN);

  // ================= Enviar datos al principal =================
  if (estadoSalon != estadoSalonAnterior || estadoTras != estadoTrasAnterior || millis() - ultimoEnvio > 500) {
    estadoSalonAnterior = estadoSalon;
    estadoTrasAnterior = estadoTras;
    ultimoEnvio = millis();

    datos.salon = estadoSalon;
    datos.trasera = estadoTras;

    esp_now_send(macPrincipal, (uint8_t *)&datos, sizeof(datos));

    Serial.print("SALON: ");
    Serial.print(estadoSalon ? "MOVIMIENTO" : "SIN MOVIMIENTO");
    Serial.print(" | TRASERA: ");
    Serial.println(estadoTras ? "MOVIMIENTO" : "SIN MOVIMIENTO");
  }

  delay(50);
}
