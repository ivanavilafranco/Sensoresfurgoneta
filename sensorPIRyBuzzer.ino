Modulo pir y altavoz
#include <ESP8266WiFi.h>
#include <espnow.h>

#define PIR_PIN     D1   // Señal del PIR
#define BUZZER_PIN  D7   // Buzzer pasivo
#define ID_SENSOR   2    // ID único del módulo PIR

uint8_t macPrincipal[] = {meter la mac};

typedef struct {
  uint8_t id;
  bool pir;   // true = movimiento, false = sin movimiento
} datos_pir;

datos_pir datos;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Estado envío: ");
  Serial.println(sendStatus == 0 ? "OK" : "Fallo");
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN); // Asegurarse apagado al iniciar

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
  datos.pir = false;

  Serial.println("⏳ Esperando estabilización del PIR...");
  delay(30000); // 30 segundos para estabilizar el PIR
  Serial.println("✅ PIR listo");
}

void loop() {
  static bool estadoAnterior = false;
  static unsigned long ultimoEnvio = 0;

  // Leer PIR
  bool estadoActual = digitalRead(PIR_PIN);

  // Activar buzzer mientras haya movimiento
  if (estadoActual) {
    tone(BUZZER_PIN, 2000); // 2000 Hz continuo
  } else {
    noTone(BUZZER_PIN);     // apagar buzzer
  }

  // Enviar datos al principal si cambia o cada 500ms
  if (estadoActual != estadoAnterior || millis() - ultimoEnvio > 500) {
    estadoAnterior = estadoActual;
    ultimoEnvio = millis();

    datos.pir = estadoActual;
    esp_now_send(macPrincipal, (uint8_t *)&datos, sizeof(datos));

    Serial.print("PIR: ");
    Serial.println(estadoActual ? "MOVIMIENTO (INTRUSO)" : "SIN MOVIMIENTO");
  }

  delay(50);
}

