
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

typedef enum DeviceStatus {
  Running = HIGH,
  Stopped = LOW
};

//Network config
const char* ssid = "ASUS2";
const char* password = "segismundo";
//Broker config
const char* mqtt_server = "192.168.0.30";
const unsigned int mqtt_port = 1883;
const char* OUT_TOPIC = "home/alarms";
const char* IN_TOPIC = "home/alarms/reset";
//Board config
const unsigned int ALARM_PIN = 12;
const unsigned int ALARM_RELAY = 13;
//Message definitions
const String DEVICE = "Pool pump";
const String ALARM_MESSAGE = "High Amperage";

WiFiClient espClient;
PubSubClient client(espClient);
bool messageSent = false;
int alarm = 0;
char data[80];
DeviceStatus deviceStatus;
StaticJsonBuffer<200> jsonBuffer;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client_pumpAlert";
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(IN_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: ");
  char inData[80];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    inData[i] = (char)payload[i];
  }
  JsonObject& root = jsonBuffer.parseObject(inData); 
  String device = root["device"];
  bool reset = root["reset"];

  if (device == DEVICE && reset) {
    deviceStatus = Running;   
    messageSent = false;
    Serial.println("Alarm reset!!");
  }
}

boolean sendMqttMessage(String device, String alarmMessage, DeviceStatus deviceStatus,const char* topic) {
      String payload = "{ \"device\": \"" + device + "\",\"alarm\": \"" + alarmMessage + "\", \"status\":\"" + deviceStatus + "\"}";
      payload.toCharArray(data, (payload.length() + 1));
      Serial.println("Publish message: " + payload);
      return client.publish(topic, data);     
}

//Implement your alarm inspection here
bool detectAlarm() {
  return digitalRead(ALARM_PIN);
}

void updateRelayState() {
  digitalWrite(ALARM_RELAY, deviceStatus);
}

void setup() {
  pinMode(ALARM_PIN, INPUT);  
  pinMode(ALARM_RELAY, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  updateRelayState();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  alarm = detectAlarm();
  if (alarm == true) {
    if (!messageSent) {
      Serial.println("Alarm detected");
      deviceStatus = Stopped;
      messageSent = sendMqttMessage(DEVICE, ALARM_MESSAGE, deviceStatus, OUT_TOPIC);
    } 
  }
}
