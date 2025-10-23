#include <main.h>
#include <WiFi.h>
#include <PubSubClient.h>

/* WIFI Configuration */
//const char* ssid     = "Fibertel WiFi449 2.4GHz"; //Marcelo T
const char* ssid     = "ClaroFibra6226"; //Florida
const char* password = "12345678";
const char* HostName = "joacoBerg";

/* MQTT Broker (TCP normal para ESP32) */
const char *mqtt_broker = "192.168.100.108";
const int   mqtt_port   = 1883;

/* Heartbeat */
const char *topic_hello = "emqx/esp32/hello";

/* Tópicos MQTT (convención nueva, monitor) */
const char *t_DI_1 = "esp32/digital/input/DI1";
const char *t_DI_2 = "esp32/digital/input/DI2";
const char *t_DI_3 = "esp32/digital/input/DI3";

const char *t_DO_1 = "esp32/digital/output/DO1";
const char *t_DO_2 = "esp32/digital/output/DO2";
const char *t_DO_3 = "esp32/digital/output/DO3";

const char *t_AI_1 = "esp32/analog/input/AI1";
const char *t_AI_2 = "esp32/analog/input/AI2";
const char *t_AI_3 = "esp32/analog/input/AI3";
const char *t_AI_4 = "esp32/analog/input/AI4";

const char *t_AO_1 = "esp32/analog/output/AO1";
const char *t_AO_2 = "esp32/analog/output/AO2";

/* Estado del dispositivo (LWT) */
const char *t_STATUS = "esp32/status"; // "online"/"offline" (retain)

/* Objetos HW */
I2CScanner scanner;
BuiltInLed  led = BuiltInLed();
I2C         i2c = I2C();
DAC         dac = DAC(); // 0x60 0x61
ADC         adc = ADC(); // 0x48

WiFiClient   espClient;
PubSubClient client(espClient);

/* ===== Helpers de escala =====
   ADC: 16-bit (0..65535) -> 10-bit (0..1023)
   DAC: 12-bit (0..4095)  ->  8-bit (0..255)
   y viceversa si luego querés escribir el DAC desde 8-bit. */
inline uint16_t u16_to_u10(uint16_t v) { return (uint32_t(v) * 1023u + 32767u) / 65535u; }
inline uint8_t  u12_to_u8 (uint16_t v) { return (uint32_t(v) * 255u  + 2047u)  / 4095u; }
inline uint16_t u8_to_u12 (uint8_t v)  { return (uint32_t(v) * 4095u + 127u)   / 255u; }

/* ===== Publicación con retain ===== */
inline void pubRetain(const char* topic, const String& val) {
  client.publish(topic, val.c_str(), true);
}

/* ===== Callback MQTT (monitor-only: no usamos /set por ahora) ===== */
void callback(char *t, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(t);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
  Serial.println("-----------------------");
}

/* ===== WiFi ===== */
void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HostName);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi OK - IP: %s\n", WiFi.localIP().toString().c_str());
}

/* ===== MQTT con LWT por connect() ===== */
void mqttReconnect() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  String client_id = "esp32-client-";
  client_id += WiFi.macAddress();

  while (!client.connected()) {
    Serial.printf("MQTT conectando como %s ... ", client_id.c_str());
    // LWT: si se cae inesperadamente, broker publica "offline" en t_STATUS (retain)
    if (client.connect(
          client_id.c_str(),   // client id
          NULL, NULL,          // user/pass (no usados)
          t_STATUS,            // will topic
          1,                   // will QoS
          true,                // will retain
          "offline"            // will payload
        )) {
      Serial.println("OK");
      // Anuncio de presencia (retain)
      client.publish(t_STATUS, "online", true);

      // Monitor-only: si luego querés control, acá agregarías subscribes a ".../set"
      // client.subscribe("esp32/digital/output/DO1/set");
    } else {
      Serial.printf("Fallo rc=%d; reintento en 2s\n", client.state());
      delay(2000);
    }
  }
}

/* ===== Config del loop de IO ===== */
#define RETAIN_TELEM true
const unsigned long IO_PERIOD_MS = 1000; // 1 segundo

/* AO (12-bit) retenemos último valor escrito para poder publicar escalado */
volatile uint16_t ao1_val12 = 2048; // 0..4095 (inicial)
volatile uint16_t ao2_val12 = 1024; // 0..4095 (inicial)

/* Snapshot de I/O (escalado para el dashboard actual) */
struct IoSnapshot {
  int di1, di2, di3;         // 0/1
  int do1, do2, do3;         // 0/1
  uint16_t ai1, ai2, ai3, ai4; // 0..1023 (ADC 16-bit escalado)
  uint8_t ao1, ao2;            // 0..255 (DAC 12-bit escalado)
};
static IoSnapshot io;

/* Máquina de estados */
enum IoState { ST_INIT, ST_READ, ST_PUBLISH, ST_WAIT };
static IoState ioState = ST_INIT;
static unsigned long ioTs = 0;

/* ===== Lecturas ===== */
inline void readDigitals() {
  io.di1 = digitalRead(DI_1);
  io.di2 = digitalRead(DI_2);
  io.di3 = digitalRead(DI_3);

  io.do1 = digitalRead(DO_1);
  io.do2 = digitalRead(DO_2);
  io.do3 = digitalRead(DO_3);
}

inline void readAnalogs() {
  // Leemos crudo de tu ADC I2C (debe devolver 0..65535)
  // Ajustá el nombre del método si tu clase expone otro (readChannel/read16/etc.)
  uint16_t raw1 = adc.read_voltage(0);
  uint16_t raw2 = adc.read_voltage(1);
  uint16_t raw3 = adc.read_voltage(2);
  uint16_t raw4 = adc.read_voltage(3);

  io.ai1 = u16_to_u10(raw1);
  io.ai2 = u16_to_u10(raw2);
  io.ai3 = u16_to_u10(raw3);
  io.ai4 = u16_to_u10(raw4);

  // (opcional) publicar crudos paralelos para depurar
  // client.publish("esp32/analog/input_raw/AI1", String(raw1).c_str(), false);
}

inline void readAOs() {
  io.ao1 = u12_to_u8(ao1_val12);
  io.ao2 = u12_to_u8(ao2_val12);
}

/* ===== Publicación ===== */
inline void publishAll() {
  // Digital Inputs
  client.publish(t_DI_1, String(io.di1).c_str(), RETAIN_TELEM);
  client.publish(t_DI_2, String(io.di2).c_str(), RETAIN_TELEM);
  client.publish(t_DI_3, String(io.di3).c_str(), RETAIN_TELEM);

  // Digital Outputs
  client.publish(t_DO_1, String(io.do1).c_str(), RETAIN_TELEM);
  client.publish(t_DO_2, String(io.do2).c_str(), RETAIN_TELEM);
  client.publish(t_DO_3, String(io.do3).c_str(), RETAIN_TELEM);

  // Analog Inputs (0..1023)
  client.publish(t_AI_1, String(io.ai1).c_str(), RETAIN_TELEM);
  client.publish(t_AI_2, String(io.ai2).c_str(), RETAIN_TELEM);
  client.publish(t_AI_3, String(io.ai3).c_str(), RETAIN_TELEM);
  client.publish(t_AI_4, String(io.ai4).c_str(), RETAIN_TELEM);

  // Analog Outputs (0..255)
  client.publish(t_AO_1, String(io.ao1).c_str(), RETAIN_TELEM);
  client.publish(t_AO_2, String(io.ao2).c_str(), RETAIN_TELEM);
}

/* ===== FSM de I/O ===== */
void ioFsmTick() {
  switch (ioState) {
    case ST_INIT:
      ioTs = millis();
      ioState = ST_READ;
      break;

    case ST_READ:
      readDigitals();
      readAnalogs();
      readAOs();
      ioState = ST_PUBLISH;
      break;

    case ST_PUBLISH:
      if (client.connected()) publishAll();
      ioTs = millis();
      ioState = ST_WAIT;
      break;

    case ST_WAIT:
      if (millis() - ioTs >= IO_PERIOD_MS)
        ioState = ST_READ;
      break;
  }
}

/* ===== Setup ===== */
void setup() {
  Serial.begin(115200);
  Serial.println("\nBoot...");

  // Pines
  pinMode(DO_1, OUTPUT);
  pinMode(DO_2, OUTPUT);
  pinMode(DO_3, OUTPUT);
  pinMode(DI_1, INPUT);
  pinMode(DI_2, INPUT);
  pinMode(DI_3, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // I2C y periféricos
  scanner.Init();
  scanner.Scan();

  i2c.init();
  dac.init(&i2c);
  adc.init(&i2c);
  led.init();
  led.blink(500);

  // Red
  wifiConnect();
  mqttReconnect();

  // Hola
  client.publish(topic_hello, "Hi, I'm ESP32 ^^");

  // Publicación inicial con lecturas reales
  readDigitals();
  readAnalogs();
  readAOs();
  publishAll();
}

/* ===== Loop ===== */
void loop() {
  // Reconexiones
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, reconectando...");
    wifiConnect();
    return;
  }
  if (!client.connected()) {
    Serial.println("MQTT desconectado, reconectando...");
    mqttReconnect();
  }

  // MQTT
  client.loop();

  // FSM de I/O (1 Hz)
  ioFsmTick();

  // Heartbeat opcional cada 2 s
  static unsigned long lastPub = 0;
  if (millis() - lastPub >= 2000) {
    lastPub = millis();
    client.publish(topic_hello, "Hi, I'm ESP32 ^^");
  }
}