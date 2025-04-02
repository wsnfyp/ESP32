#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>

#define NODEID "KK1"

#define SS 2
#define RST 25
#define DIO0 26
#define WSN_LORA_FREQ 433E6
#define WSN_LORA_SYNC_WORD 0x39
#define WSN_LORA_BANDWIDTH 125E3
#define WSN_LORA_SPREADING_FACTOR 7

#define DHTPIN 13
#define DHTTYPE DHT11 
DHT_Unified dht(DHTPIN, DHTTYPE);

#define JSN_TRIG 4 
#define JSN_ECHO 5

#define YFS_DATA 32
volatile unsigned yfs_pulse = 0;

Adafruit_BMP280 bmp;

typedef struct {
  uint32_t rhumidity;
  float temperature;
  float depth;
  float flow_rate;
  float surface_pressure;
} wsn_data_t;

// ----INIT FUNCTIONS----
void init_dht11() {
  dht.begin();
}

void init_lora() {
  Serial.begin(9600);
  while (!Serial);
  LoRa.setPins(SS, RST, DIO0);
  //  Customize (Refer https://github.com/rpsreal/pySX127x/tree/e1de274942920b4b6c6d2a56ed4a6c30b444aa04
  // and https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md)
  // and compare.
  LoRa.setFrequency(WSN_LORA_FREQ);
  LoRa.setSyncWord(WSN_LORA_SYNC_WORD);
  LoRa.setSignalBandwidth(WSN_LORA_BANDWIDTH);
  LoRa.setSpreadingFactor(WSN_LORA_SPREADING_FACTOR);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void init_jsn() {
  pinMode(JSN_TRIG, OUTPUT);
  pinMode(JSN_ECHO, INPUT);
}

void init_bmp() {
  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println("Device not fuond");
    while (1) delay(10);
  }
}

void init_yfs(){
  pinMode(YFS_DATA, INPUT);
}

// ----SENSOR READ FUNCTIONS----
void read_jsn(wsn_data_t *data) {
  digitalWrite(JSN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(JSN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(JSN_TRIG, LOW);
  long duration = pulseIn(JSN_ECHO, HIGH);
  long distance = duration * 0.034 / 2;
  if (distance > 600 || distance < 20) {
    Serial.println("JSN out of range");
    return;
  }
  data->depth = distance;
}

void ICACHE_RAM_ATTR yfs_isr() {
  yfs_pulse++;
}

void read_yfs(wsn_data_t *data) {
  float flow_rate = 0;
  yfs_pulse = 0;
  Serial.println("Reading YFS...");
  attachInterrupt(digitalPinToInterrupt(YFS_DATA), yfs_isr, RISING);
  delay(3000);
  detachInterrupt(digitalPinToInterrupt(YFS_DATA));
  Serial.println("Stopping YFS...");
  flow_rate = ((float)yfs_pulse / (7.5 * 3));
  data->flow_rate = flow_rate;
}

void read_dht11(wsn_data_t *data) {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading DHT-t"));
  }
  else {
    data->temperature = event.temperature;
  }
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading DHT-h"));
  }
  else {
    data->rhumidity = event.relative_humidity;
  }
}

void read_bmp(wsn_data_t *data) {
  data->surface_pressure = (bmp.readPressure())/100;
}

// ----LORA FUNCTIONS----
void send_lora_packet(wsn_data_t *data) {
  Serial.println("Sending packet: ");
  LoRa.beginPacket();
  LoRa.print("{");

  LoRa.print("\"NODEID\":\"");
  LoRa.print(NODEID);
  LoRa.print("\",");

  LoRa.print("\"TEMP\":");
  LoRa.print(data->temperature);
  LoRa.print(",");

  LoRa.print("\"HUM\":");
  LoRa.print(data->rhumidity);
  LoRa.print(",");
  
  LoRa.print("\"DEPTH\":");
  LoRa.print(data->depth);
  LoRa.print(",");

  LoRa.print("\"PRES\":");
  LoRa.print(data->surface_pressure);
  LoRa.print(",");

  LoRa.print("\"FLOW\":");
  LoRa.print(data->flow_rate);

  LoRa.print("}");
  LoRa.endPacket();
}


void setup() {
  Serial.begin(9600);
  init_dht11();
  init_jsn();
  init_lora();
  init_bmp();
  init_yfs();
}

void loop() {
  wsn_data_t wsn_data = {0, 0.0, 0.0, 0.0, 0.0};
  read_dht11(&wsn_data);
  read_jsn(&wsn_data);
  read_bmp(&wsn_data);
  read_yfs(&wsn_data);
  send_lora_packet(&wsn_data);
  Serial.print(" TEMP: ");
  Serial.print(wsn_data.temperature);
  Serial.print(" HUM: ");
  Serial.print(wsn_data.rhumidity);
  Serial.print(" DEPTH: ");
  Serial.print(wsn_data.depth);
  Serial.print(" FLOW: ");
  Serial.print(wsn_data.flow_rate);
  Serial.print(" PRES: ");
  Serial.print(wsn_data.surface_pressure);
  Serial.println();
  delay(5000);
}