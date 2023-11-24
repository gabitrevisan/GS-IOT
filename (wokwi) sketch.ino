#include <PubSubClient.h>
#include <WiFi.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>

const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";
const char *BROKER_MQTT = "broker.hivemq.com";
const int BROKER_PORT = 1883;
const char *ID_MQTT = "Rafa_mqtt";
const char *TOPIC_PUBLISH_MSG = "fiap/iot/GS";

WiFiClient espClient;
PubSubClient MQTT(espClient);

LiquidCrystal lcd(16, 4, 23, 22, 21, 5); 

const int buttonPin1 = 19;
const int buttonPin2 = 18;

int lastButtonState1 = LOW;
int lastButtonState2 = LOW;

int count1 = 0;
int count2 = 0;

void publishButtonCount(const char *button, int count) {
  DynamicJsonDocument doc(128);
  doc["botao"] = button;
  doc["count"] = count;

  char buffer[128];
  serializeJson(doc, buffer);
  MQTT.publish(TOPIC_PUBLISH_MSG, buffer);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT...");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao broker MQTT!");
    } else {
      Serial.print("Falha na conexão com MQTT. Tentando novamente em 2 segundos...");
      delay(2000);
    }
  }
}

void setup() {
  lcd.begin(16, 2); // Inicializa o LCD com 16 colunas e 2 linhas
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  initWiFi();
  initMQTT();
}

void initWiFi() {
  Serial.begin(9600);
  Serial.print("Conectando com a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso: ");
  Serial.println(SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void loop() {
  if (!MQTT.connected()) {
    reconnectMQTT();
  }

  MQTT.loop();

  int buttonState1 = digitalRead(buttonPin1);
  int buttonState2 = digitalRead(buttonPin2);
 
  if (buttonState1 != lastButtonState1 && buttonState1 == HIGH) {
    count1++;
    publishButtonCount("botao1", count1);
    lcd.setCursor(0, 0);
    lcd.print("Button 1: " + String(count1));
  }

  if (buttonState2 != lastButtonState2 && buttonState2 == HIGH) {
    count2++;
    publishButtonCount("botao2", count2);
    lcd.setCursor(0, 1);
    lcd.print("Button 2: " + String(count2));
  }

  // Enviando as informações dos botões 1 e 2 juntos para o Node-RE
  DynamicJsonDocument doc(128);
  doc["botao1"] = count1;
  doc["botao2"] = count2;

  char buffer[128];
  serializeJson(doc, buffer);
  MQTT.publish(TOPIC_PUBLISH_MSG, buffer);

  lastButtonState1 = buttonState1;
  lastButtonState2 = buttonState2;

  delay(100);
}
