#include <Wire.h>
#include <Adafruit_PN532.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define Rxp1 16
#define Txp1 17 //The only one that really matters for our case

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

void setup()
{
  Serial.begin(115200);
  Serial2.begin(96000, SERIAL_8N1, Rxp1, Txp1);
  Serial.println("Initializing PN532...");
  
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();
  if (!version)
  {
    Serial.println("Didn't find PN532 board");
    while (1)
      ;
  }

  nfc.SAMConfig();
  Serial.println("PN532 initialized!");
  
}

void loop()
{  
  Serial.println("Waiting for NFC tag...");

  uint8_t uid[7]; // enough room for full UID
  uint8_t uidLength = 0;
  bool success = nfc.readPassiveTargetID(
      PN532_MIFARE_ISO14443A,
      uid,
      &uidLength);

  if (success)
  {
    Serial.print("Found NFC tag with UID (");
    Serial.print(uidLength);
    Serial.print(" bytes): ");
    uint8_t sent = 0;
    
    for (uint8_t i = 0; i < uidLength; i++)
    {
      if (uid[i] < 0x10)
      {
        Serial.print("0");
        Serial2.print("0");
      }
      Serial.print(uid[i], HEX);
      Serial2.print(uid[i], HEX);
      if (i < uidLength - 1)
        Serial.print(" ");
      sent++;
    }
    // Adding zeroes as receiver expects 7 bytes of data
    if (sent < 7)
    {
      for (uint8_t i = sent; i < 7; i++)
      {
        
      Serial.print("00");
      Serial2.print("00");
      }
    }
    Serial.println();
  }
  
  delay(500);
}
