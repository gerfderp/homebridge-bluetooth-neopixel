#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_NeoPixel.h>

Adafruit_BluefruitLE_SPI ble(8, 7, 4); // Firmware > 0.7.0
Adafruit_BLEGatt gatt(ble);
int32_t informationService;
int32_t modelCharacteristic;
int32_t manufacturerCharacteristic;
int32_t serialNumberCharacteristic;

int32_t lightService;
int32_t lightCharacteristic;
int32_t hueCharacteristic;
int32_t satCharacteristic;
int32_t brightCharacteristic;


union float_bytes {
  float value;
  uint8_t bytes[sizeof(float)];
};

#define PIN                     5
#define NUMPIXELS              76
/*=========================================================================*/

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW);
bool on = false;
float hue = 0.0;
float saturation = 100.0;
int brightness = 100;

void setup(void) {
  Serial.begin(115200);

  ble.begin(/* true - useful for debugging */);
//  ble.begin(1);
  ble.factoryReset();
  ble.info();

  // turn off neopixel
  pixel.begin(); // This initializes the NeoPixel library.
  for(uint8_t i=0; i<NUMPIXELS; i++) {
    pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
  }
  pixel.show();

  informationService = gatt.addService(0x180A);
  char model[] = "Micro LE";
  modelCharacteristic = gatt.addCharacteristic(0x2A24, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(model)+1, sizeof(model)+1, BLE_DATATYPE_STRING);
  gatt.setChar(modelCharacteristic, model);
  char manufacturer[] = "Bluefruit";
  manufacturerCharacteristic = gatt.addCharacteristic(0x2A29, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(manufacturer), sizeof(manufacturer), BLE_DATATYPE_STRING);
  gatt.setChar(manufacturerCharacteristic, manufacturer);
  char serialNumber[] = "2.71828";
  serialNumberCharacteristic = gatt.addCharacteristic(0x2A25, GATT_CHARS_PROPERTIES_READ,
                                               sizeof(serialNumber), sizeof(serialNumber), BLE_DATATYPE_STRING);
  gatt.setChar(serialNumberCharacteristic, serialNumber);

                            //    57   E5   4B   F0-  85   74-  47   BE-  9C   1D-  A0   DB   FC   8F   A1   83
  uint8_t lightServiceUUID[] = {0x57,0xE5,0x4B,0xF0,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};

  lightService = gatt.addService(lightServiceUUID);
  uint8_t lightCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF1,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  lightCharacteristic = gatt.addCharacteristic(lightCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(bool),sizeof(bool), BLE_DATATYPE_BYTEARRAY, "On");
                                                 
  uint8_t hueCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF2,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  hueCharacteristic = gatt.addCharacteristic(hueCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(float),sizeof(float), BLE_DATATYPE_BYTEARRAY, "Hue");

  uint8_t satCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF3,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  satCharacteristic = gatt.addCharacteristic(satCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(float),sizeof(float), BLE_DATATYPE_BYTEARRAY, "Sat");

  uint8_t brightCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF4,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  brightCharacteristic = gatt.addCharacteristic(brightCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(int),sizeof(int), BLE_DATATYPE_INTEGER, "Bright");


                                                     
  Serial.print("hueCharacteristic: ");
  Serial.println(hueCharacteristic);
  Serial.print("brightCharacteristic: ");
  Serial.println(brightCharacteristic);
  ble.reset();
  ble.setConnectCallback(centralConnect);
  ble.setDisconnectCallback(centralDisconnect);
  Serial.println("Bluetooth on");


  /* Only one BLE GATT function should be set, it is possible to set it
    multiple times for multiple Chars ID  */
  ble.setBleGattRxCallback(lightCharacteristic, lightRX);
  ble.setBleGattRxCallback(hueCharacteristic, lightRX);
  ble.setBleGattRxCallback(satCharacteristic, lightRX);
  ble.setBleGattRxCallback(brightCharacteristic, lightRX);

}


void lightRX(int32_t chars_id, uint8_t data[], uint16_t len)
{
  Serial.print( F("[BLE GATT RX] (" ) );
  Serial.print(chars_id);
  Serial.print(") ");
  for (int i = 0; i < len; i++) { Serial.print(i); Serial.print(":"); Serial.print(data[i]);Serial.print(" | ");}
  Serial.println("  <-- end data");
  if (chars_id == lightCharacteristic) {
      on = data[0];
      String onstr = String(on);
//      gatt.setChar(lightCharacteristic, onstr[0], sizeof(onstr));
  } else if (chars_id == hueCharacteristic) {
    union float_bytes hueval = { .value = hue };
    gatt.getChar(hueCharacteristic, hueval.bytes, sizeof(hueval));
    hue = hueval.value;
  } else if (chars_id == satCharacteristic) {
    union float_bytes satval = { .value = saturation };
    gatt.getChar(satCharacteristic, satval.bytes, sizeof(satval));
    saturation = satval.value;
  } else if (chars_id == brightCharacteristic) {
    brightness = parseHex(data, len);
  } else {
      Serial.print("chars_id: ");
      Serial.println(chars_id);
  }
  Serial.println("ENDERP");
  setLED(on, hue, saturation, brightness);  

}


void setLED(bool on, float hue, float saturation, int brightness) {
  Serial.print("LED: ");
  Serial.print(on);
  Serial.print(" | HUE: ");
  Serial.print(hue);
  Serial.print(" | SAT: ");
  Serial.print(saturation);
  Serial.print(" | BRI: ");
  Serial.println(brightness);
  hue = max(0.0, min(360.0, hue));
  saturation = max(0.0, min(100.0, saturation));
  brightness = max(0, min(100, brightness));

  unsigned int red, green, blue;
  hsv2rgb(hue, saturation/100.0, brightness/100.0, &red, &green, &blue);

  Serial.print("RGB LED | ");
  Serial.print(on);
  Serial.print(" HSB(");
  Serial.print(hue);
  Serial.print(",");
  Serial.print(saturation);
  Serial.print(",");
  Serial.print(brightness);
  Serial.print(") -> RGB(");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.print(blue);
  Serial.println(")");
  
  for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(red,green,blue)); // off
  }
  if (! on) {
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
    }
    pixel.show();
    return;
  } else {
    pixel.show();
  }
 
  
}

void loop(void) {
  ble.update();
  delay(1000);
}

void centralConnect(void) {
  Serial.print("Central connected | ");
  if (ble.sendCommandCheckOK("AT+BLEGETPEERADDR")) {
    Serial.println(ble.buffer);
  }
}

void centralDisconnect(void) {
  Serial.print("Central disconnected | ");
  if (ble.sendCommandCheckOK("AT+BLEGETPEERADDR")) {
    Serial.println(ble.buffer);
  }
}

void hsv2rgb(float hue, float saturation, float value,
             unsigned int* red, unsigned int* green, unsigned int* blue) {
  if (saturation == 0.0) {
    *red   = (unsigned int) round(value * 255.0);
    *green = (unsigned int) round(value * 255.0);
    *blue  = (unsigned int) round(value * 255.0);
  } else {
    int h = ((int) floor(hue / 60.0) % 6);
    float f = (hue / 60.0) - floor(hue / 60.0);
    float p = value * (1.0 - saturation);
    float q = value * (1.0 - saturation * f);
    float t = value * (1.0 - (saturation * (1.0 - f)));
    switch (h) {
      case 0:
        *red   = (unsigned int) round(value * 255.0);
        *green = (unsigned int) round(t * 255.0);
        *blue  = (unsigned int) round(p * 255.0);
        break;
      case 1:
        *red   = (unsigned int) round(q * 255.0);
        *green = (unsigned int) round(value * 255.0);
        *blue  = (unsigned int) round(p * 255.0);
        break;
      case 2:
        *red   = (unsigned int) round(p * 255.0);
        *green = (unsigned int) round(value * 255.0);
        *blue  = (unsigned int) round(t * 255.0);
        break;
      case 3:
        *red   = (unsigned int) round(p * 255.0);
        *green = (unsigned int) round(q * 255.0);
        *blue  = (unsigned int) round(value * 255.0);
        break;
      case 4:
        *red   = (unsigned int) round(t * 255.0);
        *green = (unsigned int) round(p * 255.0);
        *blue  = (unsigned int) round(value * 255.0);
        break;
      case 5:
        *red   = (unsigned int) round(value * 255.0);
        *green = (unsigned int) round(p * 255.0);
        *blue  = (unsigned int) round(q * 255.0);
        break;
    }
  }
}

int parseHex(const uint8_t * data, const uint32_t numBytes)
{
  uint32_t szPos;
  String hex = "";

  for (szPos=0; szPos < numBytes; szPos++) 
  {
    if (data[szPos] <= 0xF)
    {
      hex += String(data[szPos] & 0xf, HEX); 
    }
    else
    {
      hex += String(data[szPos] & 0xff, HEX); 
    }
  }
  Serial.println("parseHex");
  Serial.print("hex String: ");
  Serial.println(hex);
  int  len = hex.length();
  char charbuff[len+2];
  hex.toCharArray(charbuff, len +2);
  Serial.print("charbuff: ");
  Serial.println(charbuff);
  unsigned long ul = strtoul(charbuff, 0,16);
  int i = (int) ul;
  Serial.print("unsigned long: ");
  Serial.println(ul);
  Serial.print("int: ");
  Serial.println(i);
  return i;
}

