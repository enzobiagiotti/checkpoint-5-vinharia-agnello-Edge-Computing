/*
 * Vinheria Agnello - ESP32 no Wokwi (CP5/CP6)
 * Lê temperatura e umidade (DHT22) e luminosidade (LDR) e publica no
 * IoT Agent MQTT do FIWARE no formato UltraLight 2.0.
 * Também recebe comandos (on/off) para o LED de alerta, que podem ser
 * enviados pelo MyMQTT ou pelo Orion.
 *
 * Publica:  /TEF/vinheriaENZ09/attrs        ->  t|<temp>|h|<umid>|l|<lum>
 *           /TEF/vinheriaENZ09/attrs/s      ->  on | off   (estado do LED)
 * Assina:   /TEF/vinheriaENZ09/cmd          <-  vinheriaENZ09@on|  ou  vinheriaENZ09@off|
 *
 * O timestamp de cada leitura é gerado pelo IoT Agent (atributo TimeInstant).
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ---------- Configurações (ALTERE AQUI) ----------
const char* WIFI_SSID     = "Wokwi-GUEST";   // rede padrão do simulador
const char* WIFI_PASSWORD = "";
const char* BROKER_IP     = "3.135.234.101";  // servidor FIWARE disponibilizado pelo professor
const int   BROKER_PORT   = 1883;
const char* APIKEY        = "TEF";
const char* DEVICE_ID     = "vinheriaENZ09";   // único por grupo (servidor compartilhado)

#define DHT_PIN   15
#define DHT_TYPE  DHT22
#define LDR_PIN   34
#define LED_PIN   2
const unsigned long INTERVALO_MS = 5000;
// -------------------------------------------------

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);

char topicoAttrs[64], topicoEstado[64], topicoCmd[64];
unsigned long ultimoEnvio = 0;

void conectaWiFi() {
  Serial.printf("Conectando ao Wi-Fi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.printf("\nWi-Fi OK. IP: %s\n", WiFi.localIP().toString().c_str());
}

// Recebe comandos: "vinheriaENZ09@on|" ou "vinheriaENZ09@off|" (a barra final é opcional)
void aoReceber(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  Serial.printf("Comando recebido em %s: %s\n", topic, msg.c_str());

  if (msg.startsWith(String(DEVICE_ID) + "@on")) {
    digitalWrite(LED_PIN, HIGH);
    mqtt.publish(topicoEstado, "on");
  } else if (msg.startsWith(String(DEVICE_ID) + "@off")) {
    digitalWrite(LED_PIN, LOW);
    mqtt.publish(topicoEstado, "off");
  }
}

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    String clientId = String("esp32-") + DEVICE_ID + "-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(topicoCmd);
      Serial.printf("Inscrito em %s\n", topicoCmd);
    } else {
      Serial.printf("falhou (rc=%d), tentando em 2s\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  snprintf(topicoAttrs,  sizeof(topicoAttrs),  "/%s/%s/attrs",   APIKEY, DEVICE_ID);
  snprintf(topicoEstado, sizeof(topicoEstado), "/%s/%s/attrs/s", APIKEY, DEVICE_ID);
  snprintf(topicoCmd,    sizeof(topicoCmd),    "/%s/%s/cmd",     APIKEY, DEVICE_ID);

  conectaWiFi();
  mqtt.setServer(BROKER_IP, BROKER_PORT);
  mqtt.setCallback(aoReceber);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectaWiFi();
  if (!mqtt.connected()) conectaMQTT();
  mqtt.loop();

  if (millis() - ultimoEnvio >= INTERVALO_MS) {
    ultimoEnvio = millis();

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    // No módulo LDR do Wokwi a tensão cai quando a luz aumenta,
    // por isso o mapeamento é invertido: 4095 (escuro) -> 0 %, 0 (claro) -> 100 %
    int lum = map(analogRead(LDR_PIN), 4095, 0, 0, 100);

    if (isnan(temp) || isnan(umid)) {
      Serial.println("Falha ao ler o DHT22");
      return;
    }

    char payload[64];
    snprintf(payload, sizeof(payload), "t|%.1f|h|%.1f|l|%d", temp, umid, lum);
    mqtt.publish(topicoAttrs, payload);
    Serial.printf("Publicado em %s -> %s\n", topicoAttrs, payload);
  }
}
