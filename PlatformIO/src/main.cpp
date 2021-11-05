#include <WiFi.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define DISPLAY_SCAN_RESULTS (0)
#define SAVE_LOG (1)

#define RX_PIN (16)
#define TX_PIN (17)

#define SD_CS_PIN (5)

#define SERIAL_BAUD (115200)

const String logName = "/log.csv";

static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

String getEncryption(wifi_auth_mode_t encryptionType)
{
  switch (encryptionType)
  {
  case (0):
    return "Open";
  case (1):
    return "WEP";
  case (2):
    return "WPA_PSK";
  case (3):
    return "WPA2_PSK";
  case (4):
    return "WPA_WPA2_PSK";
  case (5):
    return "WPA2_ENTERPRISE";
  default:
    return "UNKOWN";
  }
}

void getData()
{
  if (Serial.availableForWrite() > 0)
  {
    File file = SD.open(logName, FILE_READ);
    while (file.available())
    {
      Serial.write(file.read());
    }
    file.close();
  }
}

void serialCommand(){
  if (Serial.available() > 0)
  {
    String data = Serial.readString();

    if (data == String("get\n"))
    {
      getData();
    }
    else if (data == "clear\n")
    {
      SD.remove(logName);
    }
  }
}

void logWritter(int wifiAddress)
{
  #if SAVE_LOG == 1
  File file = SD.open(logName, FILE_APPEND);

  file.print(WiFi.SSID(wifiAddress));
  file.print("|");
  file.print(WiFi.BSSIDstr(wifiAddress));
  file.print("|");
  file.print(WiFi.RSSI(wifiAddress));
  file.print("|");
  file.print(getEncryption(WiFi.encryptionType(wifiAddress)));
  file.print("|");
  file.print(WiFi.channel(wifiAddress));
  file.print("|");
  file.print(gps.location.lat(), 6);
  file.print("|");
  file.print(gps.location.lng(), 6);
  file.print("|");
  file.print(String(gps.speed.kmph()));
  file.print("|");
  file.print(gps.altitude.meters(), 2);
  file.print("|");
  file.print(gps.hdop.value(), 1);
  file.print("|");
  file.print(gps.satellites.value());
  file.println();

  file.close();

  #endif
}

void displayScan(int wifiAddress)
{
  #if DISPLAY_SCAN_RESULTS == 1
  Serial.println("\n=================================================");
  Serial.println("SSID       : " + WiFi.SSID(wifiAddress));
  Serial.println("MAC        : " + WiFi.BSSIDstr(wifiAddress));
  Serial.println("RSSI       : " + WiFi.BSSIDstr(wifiAddress) + "  ( " + WiFi.RSSI(wifiAddress) + " )");
  Serial.println("Encryption : " + getEncryption(WiFi.encryptionType(wifiAddress)));
  Serial.println("Channel    : " + String(WiFi.channel(wifiAddress)));
  Serial.print("Latitiude  : ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude  : ");
  Serial.println(gps.location.lng(), 6);
  Serial.println("Speed      : " + String(int(gps.speed.kmph())) + " Km/h");
  Serial.print("Altitude   : ");
  Serial.println(gps.altitude.meters(), 2);
  Serial.print("HDOP       : ");
  Serial.println(gps.hdop.value(), 1);
  Serial.print("Satelite   : ");
  Serial.println(gps.satellites.value());
  #endif
}

void scan()
{
  int totalNetwork = WiFi.scanNetworks();

  if (totalNetwork == 0)
  {
    Serial.println(F("No networks found"));
  }
  else
  {
    for (int i = 0; i < totalNetwork; i++)
    {
      displayScan(i);
      logWritter(i);
    }
  }
}

void scanner()
{
  if (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
      if (gps.location.isValid() && gps.location.isUpdated() && gps.altitude.isUpdated())
      {
        scan();
      }
    }
  }
}

void setup()
{
  delay(500);
  Serial.begin(SERIAL_BAUD);
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RX_PIN, TX_PIN, false);
  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println(F("Failed to start SD Card"));
  }
}

void loop()
{
  serialCommand();
  scanner();
}