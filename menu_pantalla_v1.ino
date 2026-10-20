esphome:
  name: sensor-camper
  friendly_name: Panel Camper

esp8266:
  board: nodemcuv2

# ==================================================
# CONEXION WIFI
# ==================================================
wifi:
  ssid: "DIGIFIBRA-Eb3A"
  password: !secret wifi_password

  ap:
    ssid: "Panel-Camper-Rescate"
    password: "Camper12345"

captive_portal:

logger:

# Actualizaciones posteriores por WiFi.
ota:
  - platform: esphome

# Panel de control desde el navegador.
web_server:
  port: 80

# ==================================================
# MQTT - RASPBERRY / VENUS OS
# ==================================================
mqtt:
  id: conexion_mqtt
  broker: 192.168.1.135
  port: 1883
  discovery: false
  topic_prefix: camper/panel
  reboot_timeout: 0s

# ==================================================
# OLED INTEGRADA
# SDA = D5 / GPIO14
# SCL = D6 / GPIO12
# No conectar otros componentes a estos pines,
# excepto dispositivos compatibles en el mismo bus I2C.
# ==================================================
i2c:
  id: bus_i2c
  sda: GPIO14
  scl: GPIO12
  scan: true
  frequency: 100kHz

font:
  - file: "gfonts://Roboto"
    id: fuente_12
    size: 12

  - file: "gfonts://Roboto"
    id: fuente_18
    size: 18

# ==================================================
# DIAGNOSTICO
# Aqui añadiremos los sensores fisicos.
# ==================================================
sensor:
  - platform: wifi_signal
    id: senal_wifi
    name: "Senal WiFi"
    update_interval: 15s

  - platform: uptime
    id: tiempo_encendido
    name: "Tiempo encendido"
    update_interval: 30s

text_sensor:
  - platform: wifi_info
    ip_address:
      id: ip_panel
      name: "Direccion IP"

    mac_address:
      name: "Direccion MAC"

# ==================================================
# MENUS AMPLIABLES
# Se pueden añadir nuevas opciones en esta lista.
# ==================================================
select:
  - platform: template
    id: menu_principal
    name: "Menu principal"
    optimistic: true
    restore_value: false
    initial_option: "Inicio"

    options:
      - "Inicio"
      - "Ambiente"
      - "Energia"
      - "Agua"
      - "Luces"
      - "Calefaccion"
      - "Trampillas"
      - "Seguridad"
      - "Ampliaciones"
      - "Sistema"

    on_value:
      then:
        - switch.turn_on: pantalla_encendida
        - component.update: oled

  # Selecciona cual de las seis trampillas visualizar.
  # No envia ordenes a motores en esta primera version.
  - platform: template
    id: trampilla_seleccionada
    name: "Trampilla seleccionada"
    optimistic: true
    restore_value: false
    initial_option: "Trampilla 1"

    options:
      - "Trampilla 1"
      - "Trampilla 2"
      - "Trampilla 3"
      - "Trampilla 4"
      - "Trampilla 5"
      - "Trampilla 6"

    on_value:
      then:
        - select.set:
            id: menu_principal
            option: "Trampillas"

# ==================================================
# CONTROL DE LA PANTALLA
# Apagar la OLED no apaga el ESP8266.
# ==================================================
switch:
  - platform: template
    id: pantalla_encendida
    name: "Pantalla encendida"
    optimistic: true
    restore_mode: ALWAYS_ON

    turn_on_action:
      - lambda: |-
          id(oled).turn_on();
      - component.update: oled

    turn_off_action:
      - lambda: |-
          id(oled).turn_off();

# ==================================================
# ACCIONES REUTILIZABLES
# Los futuros botones fisicos ejecutaran estos scripts.
# ==================================================
script:
  - id: menu_siguiente
    then:
      - select.next:
          id: menu_principal
          cycle: true

  - id: menu_anterior
    then:
      - select.previous:
          id: menu_principal
          cycle: true

  - id: volver_inicio
    then:
      - select.set:
          id: menu_principal
          option: "Inicio"

  - id: alternar_pantalla
    then:
      - switch.toggle: pantalla_encendida

  - id: siguiente_trampilla
    then:
      - select.next:
          id: trampilla_seleccionada
          cycle: true

# ==================================================
# BOTONES VIRTUALES DISPONIBLES EN LA WEB
# No requieren pulsadores conectados.
# ==================================================
button:
  - platform: template
    name: "Menu siguiente"
    on_press:
      - script.execute: menu_siguiente

  - platform: template
    name: "Menu anterior"
    on_press:
      - script.execute: menu_anterior

  - platform: template
    name: "Volver al inicio"
    on_press:
      - script.execute: volver_inicio

  - platform: template
    name: "Alternar pantalla"
    on_press:
      - script.execute: alternar_pantalla

  - platform: template
    name: "Siguiente trampilla"
    on_press:
      - script.execute: siguiente_trampilla

# ==================================================
# DIBUJO DE LA PANTALLA
# ==================================================
display:
  - platform: ssd1306_i2c
    id: oled
    i2c_id: bus_i2c
    model: "SSD1306 128x64"
    address: 0x3C

    # Si aparece invertida, cambiar a 0.
    rotation: 180

    contrast: 70%
    update_interval: 1s

    lambda: |-
      const std::string pagina = id(menu_principal).state;

      // Cabecera.
      it.print(0, 0, id(fuente_12), pagina.c_str());
      it.line(0, 15, 127, 15);

      // Indicadores de conexion:
      // W = conectado al WiFi.
      // M = conectado al broker MQTT.
      it.print(101, 0, id(fuente_12),
               WiFi.status() == WL_CONNECTED ? "W" : "-");

      it.print(116, 0, id(fuente_12),
               id(conexion_mqtt).is_connected() ? "M" : "-");

      if (pagina == "Inicio") {
        it.print(0, 18, id(fuente_18), "CAMPER");

        if (WiFi.status() == WL_CONNECTED) {
          it.print(0, 43, id(fuente_12),
                   id(ip_panel).state.c_str());
        } else {
          it.print(0, 43, id(fuente_12),
                   "Conectando WiFi...");
        }

      } else if (pagina == "Ambiente") {
        it.print(0, 20, id(fuente_12), "Temperatura: --");
        it.print(0, 39, id(fuente_12), "Humedad: --");

      } else if (pagina == "Energia") {
        it.print(0, 20, id(fuente_12), "Bateria: --");
        it.print(0, 39, id(fuente_12), "Solar: --");

      } else if (pagina == "Agua") {
        it.print(0, 20, id(fuente_12), "Limpias: --");
        it.print(0, 39, id(fuente_12), "Grises: --");

      } else if (pagina == "Luces") {
        it.print(0, 20, id(fuente_12), "Control de luces");
        it.print(0, 39, id(fuente_12), "Sin integrar");

      } else if (pagina == "Calefaccion") {
        it.print(0, 20, id(fuente_12), "Calefaccion");
        it.print(0, 39, id(fuente_12), "Sin integrar");

      } else if (pagina == "Trampillas") {
        it.print(0, 20, id(fuente_18),
                 id(trampilla_seleccionada).state.c_str());
        it.print(0, 43, id(fuente_12), "Sin integrar");

      } else if (pagina == "Seguridad") {
        it.print(0, 20, id(fuente_12), "Sensores y avisos");
        it.print(0, 39, id(fuente_12), "Sin integrar");

      } else if (pagina == "Ampliaciones") {
        it.print(0, 20, id(fuente_12), "Nuevos sensores");
        it.print(0, 39, id(fuente_12), "Sin integrar");

      } else if (pagina == "Sistema") {
        if (WiFi.status() == WL_CONNECTED) {
          it.print(0, 19, id(fuente_12),
                   id(ip_panel).state.c_str());
        } else {
          it.print(0, 19, id(fuente_12), "WiFi desconectado");
        }

        if (!isnan(id(senal_wifi).state)) {
          it.printf(0, 36, id(fuente_12), "WiFi: %.0f dBm",
                    id(senal_wifi).state);
        }

        it.print(0, 51, id(fuente_12),
                 id(conexion_mqtt).is_connected()
                 ? "MQTT: conectado"
                 : "MQTT: desconectado");

      } else {
        // Cualquier nueva opcion añadida al menu
        // tendra pantalla aunque aun no tenga contenido.
        it.print(0, 20, id(fuente_12), "Nueva seccion");
        it.print(0, 39, id(fuente_12), "Sin configurar");
      }
