#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SdFat.h>
#include <WebServer.h>
#include <DebugUtil.h>
#include <StringUtil.h>
#include <Yudino.h>
#include "Config.h"
#include "MyHeater.h"
#include <utility/w5100.h>

MyHeater *myHeater;

SdFat sd;

static byte socketStat[MAX_SOCK_NUM];
static unsigned long nextSockStat = 0;

static void showSockStatus(void)
{
  for (int i = 0; i < MAX_SOCK_NUM; i++) {
    Serial.print(F("Socket#"));
    Serial.print(i);
    uint8_t s = W5100.readSnSR(i);
    socketStat[i] = s;
    Serial.print(F(":0x"));
    Serial.print(s,16);
    Serial.print(F(" "));
    Serial.print(W5100.readSnPORT(i));
    Serial.print(F(" D:"));
    uint8_t dip[4];
    W5100.readSnDIPR(i, dip);
    for (int j=0; j<4; j++) {
      Serial.print(dip[j],10);
      if (j<3) Serial.print(".");
    }
    Serial.print(F("("));
    Serial.print(W5100.readSnDPORT(i));
    Serial.println(F(")"));
  }
}

void dateTime(uint16_t* date, uint16_t* time) {
    time_t t = now();

    *date = FAT_DATE(year(t), month(t), day(t));
    *time = FAT_TIME(hour(t), minute(t), second(t));
}

void setup() {
    Serial.begin(9600);

    DEBUG("My heater");

    // Use external 3.3 volts for temperature sensor
    analogReference(EXTERNAL);

    // Init Ethernet shield
    // deselect SDcard
    pinMode(ETHERNET_SHIELD_SDCARD_PIN, OUTPUT);
    digitalWrite(ETHERNET_SHIELD_SDCARD_PIN, HIGH);
    // deselect W5100
    // On both boards, pin 10 is used as SS
    pinMode(ETHERNET_SHIELD_W5100_PIN, OUTPUT);
    digitalWrite(ETHERNET_SHIELD_W5100_PIN, HIGH);
    // On the Mega, the hardware SS pin, 53, is not used to select the W5100,
    // but it must be kept as an output or the SPI interface won't work.
    //pinMode(SS, OUTPUT);
    pinMode(53, OUTPUT);

    // Init SD card
    DEBUG("init SDcard");
    if (!sd.begin(ETHERNET_SHIELD_SDCARD_PIN, SPI_HALF_SPEED)) {
        DEBUG("Failed to init SDcard!");
    }
    SdFile::dateTimeCallback(dateTime);

    MaintainedCollection *heaters = new MaintainedCollection(MAINTAIN_HEATERS_INTERVAL);
    heaters->add(new UnderfloorHeater(
        "kitchen",
        new Relay(KITCHEN_HEATER_PIN, true),
        new Thermistor(KITCHEN_SENSOR_PIN, TERMISTOR_NOMINAL, SENSOR_SERIES_RESISTOR, SENSOR_ADC_CORRECTION, SENSOR_NUM_SAMPLES),
        new EnergyMeter(KITCHEN_HEATER_POWER),
        new DataLogger(&sd, "/log/kitchen/%Y-%M.txt", LOG_HEADER), LOG_INTERVAL
    ));
    heaters->add(new UnderfloorHeater(
        "bedroom",
        new Relay(BEDROOM_HEATER_PIN, true),
        new Thermistor(BEDROOM_SENSOR_PIN, TERMISTOR_NOMINAL, SENSOR_SERIES_RESISTOR, SENSOR_ADC_CORRECTION, SENSOR_NUM_SAMPLES),
        new EnergyMeter(BEDROOM_HEATER_POWER),
        new DataLogger(&sd, "/log/bedroom/%Y-%M.txt", LOG_HEADER), LOG_INTERVAL
    ));
    heaters->add(new UnderfloorHeater(
        "hall",
        new Relay(HALL_HEATER_PIN, true),
        new Thermistor(HALL_SENSOR_PIN, TERMISTOR_NOMINAL_HALL, SENSOR_SERIES_RESISTOR, SENSOR_ADC_CORRECTION, SENSOR_NUM_SAMPLES),
        new EnergyMeter(HALL_HEATER_POWER),
        new DataLogger(&sd, "/log/hall/%Y-%M.txt", LOG_HEADER), LOG_INTERVAL
    ));

    myHeater = new MyHeater(
        new MyHeaterConfig(CONFIG_VERSION, UPDATE_CONFIG_INTERVAL),
        new MyHeaterNetwork(
            MAC_ADDRESSS,
#ifdef WITH_ICMP
            { PING_SERVER },
#endif
            CONNECTION_CHECK_INTERVAL
        ),
        new MyHeaterTime(
            new IPAddress(NTP_SERVER),
            NTP_FIRST_SYNC_INTERVAL,
            NTP_SYNC_INTERVAL,
            NTP_LOCAL_PORT,
            NTP_TIME_ZONE
        ),
        new MyHeaterUI(WEBSERVER_PREFIX, WEBSERVER_PORT,
            WEBSERVER_AUTH_CREDENTIALS, &sd),
        heaters
    );
}

void loop() {
    myHeater->maintain();
    if (millis() >= nextSockStat) {
        showSockStatus();
        nextSockStat = millis() + 10000;
        DEBUG("ramFree: %d", DebugUtil::ramFree());
    }

    //delay(100);
}
