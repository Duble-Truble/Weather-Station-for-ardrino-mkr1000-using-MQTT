#include <PubSubClient.h>
#include <WiFi101.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HIH4030.h>
#include <limits.h>
#include <math.h>
//______________________________________
// WiFi
const char* ssid = "";// wifi name
const char* wifi_password = "";//wifi password
//MQTT______________________________________
const char* mqtt_server = "";                               //mqtt broker ip address
const char* Temp1 = "veterna/streha/temeeratura_celice";    // mqtt subscriptions
const char* Temp2 = "veterna/streha/zunanja_temperatura";
const char* Temp3 = "veterna/streha/temperatura_konzole";
const char* Vlaga = "veterna/streha/vlaga";
const char* Veter = "veterna/streha/veter";
const char* mqtt_username = "";                             //mqtt username
const char* mqtt_password = "";                             //mqtt password
const char* clientID = "";                                  //mqtt id


//______________________________________
const byte pin = A1;//wind sensor
const unsigned int measurementDelaySeconds = 10;
volatile unsigned int pulseCount = 0;
const unsigned int debounceDelayMillis = 10;
unsigned long lastPulse = 0;


const float maxImpulsesPerSecond// wind speed sensor based on Hall-effect sensor, 2 pulses per revolution
  = ((float)1000 / debounceDelayMillis) * 0.5f;
//____________________________________
byte ledState = LOW;
#define ONE_WIRE_BUS A2
HIH4030 hygrometer(A3, 5.0, 5.0);// humidity sensors comuncasion pin; supplyed voltage, output volatage.
OneWire oneWire(ONE_WIRE_BUS);//temperature sensors, also see DallasTemperature librery for calculatins
DallasTemperature sensors(&oneWire);
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient);
//___________________________________________________________

void connect_MQTT() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

}
//________________________________________________
void setup(void)
{
  connect_MQTT();
  Serial.begin(9600);
  pinMode(pin, INPUT);
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), countPulse, FALLING);
  noInterrupts();
  pinMode(LED_BUILTIN, OUTPUT);// build in led wind speed sensor, used for debuging
  digitalWrite(LED_BUILTIN, ledState);
  Serial.println("Arduino Digital Temperature // Serial Monitor Version");
  sensors.begin();
}
//__________________________________________________
void countPulse() {
 

  unsigned long time = millis();
  unsigned long timeSinceLastPulse;

  if (time >= lastPulse)
    timeSinceLastPulse = time - lastPulse;
  else {
    timeSinceLastPulse = (ULONG_MAX - lastPulse) + time;
  }
  if (timeSinceLastPulse <= debounceDelayMillis)
    return;

  lastPulse = time;
  ++pulseCount;
  ledState = (ledState == HIGH ? LOW : HIGH);
  digitalWrite(LED_BUILTIN, ledState);
}
void loop(void)
{
   if(WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, wifi_password);
  }
  float vlaga = hygrometer.getSensorRH();
  float temp1 = sensors.getTempCByIndex(0);
  float temp2 = sensors.getTempCByIndex(1);
  float temp3 = sensors.getTempCByIndex(2);
  sensors.requestTemperatures();
  pulseCount = 0;
  interrupts();

  delay((unsigned long)measurementDelaySeconds * 1000);



  float pulsesPerSecond = (float)pulseCount / measurementDelaySeconds;
  float pulsesPerMinute = (float)pulseCount * 60 / measurementDelaySeconds;
  int displayedDecimals = numberOfDecimalsNeeded((float)1 / maxImpulsesPerSecond);
  float veter=(pulsesPerSecond/2*62*3.14159265359*2/100);//seting base for wind calculations
  String veters = "veter: " + String((float)veter) + " % ";
  String vlagas = "Vlaga: " + String((float)vlaga) + " % ";
  String temp1s = "Temperatura 1: " + String((float)temp1) + " % ";
  String temp2s = "Tempperatura 2: " + String((float)temp2) + " % ";
  String temp3s = "Tempperatura 3: " + String((float)temp3) + " % ";


  if (client.publish(Vlaga, String(vlaga).c_str())) {
    Serial.println("vlaga poslana!");
  }
  else {
    Serial.println("Cold not connect to MQTT broker!");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(Vlaga, String(vlaga).c_str());
  }
  if (client.publish(Temp1, String(temp1).c_str())) {
    Serial.println("Temperatura1  successfully sent!");
  }
  else {
    Serial.println("Cold not connect to MQTT broker!");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(Temp1, String(temp1).c_str());
  }
  if (client.publish(Temp2, String(temp2).c_str())) {
    Serial.println("Temperatura2  successfully sent!");
  }
  else {
    Serial.println("Cold not connect to MQTT broker!");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(Temp2, String(temp2).c_str());
  }
   if (client.publish(Temp3, String(temp3).c_str())) {
    Serial.println("Temperatura3  successfully sent!");
  }
  else {
    Serial.println("Cold not connect to MQTT broker!");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(Temp3, String(temp3).c_str());
  }
  if (client.publish(Veter, String(veter).c_str())) {
    Serial.println("Veter  successfully sent!");
  }
  else {
    Serial.println("Cold not connect to MQTT broker!");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10);
    client.publish(Veter, String(veter).c_str());
  }/* Serial prints used for debuging
  Serial.print("Povprečna vlaga: ");
  Serial.print(vlaga);
  Serial.println("%");
  Serial.print("Temperatura 1: ");
  Serial.println(temp1);
  Serial.print("Temperatura 2: ");
  Serial.println(temp2);
  Serial.print("Temperatura 3: ");
  Serial.println(temp3);
  Serial.print("zaznanih pulzov: ");
  Serial.println(pulseCount);
  Serial.print("km/h: ");
  Serial.println(veter);
*/
  
  if (pulsesPerSecond >= maxImpulsesPerSecond) {
    Serial.println("ERROR: Debounce delay too high for impulse speed!");
  }
  Serial.println("-----------------------------------------------");
}
int numberOfDecimalsNeeded(float smallestNumber) {
  return ceil(abs(log10(smallestNumber)));
}


//___________________________________________________
