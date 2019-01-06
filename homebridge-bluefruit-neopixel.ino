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

int32_t thermometerService;
int32_t temperatureCharacteristic;

int32_t lightService;
int32_t lightCharacteristic;
int32_t hueCharacteristic;

union float_bytes {
  float value;
  uint8_t bytes[sizeof(float)];
};

   #define PIN                     5
    #define NUMPIXELS              76
/*=========================================================================*/

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW);


void setup(void) {
  Serial.begin(115200);

//  ble.begin(/* true - useful for debugging */);
  ble.begin(1);
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

  uint8_t thermometerServiceUUID[] = {0x1D,0x8A,0x68,0xE0,0xE6,0x8E,0x4F,0xED,0x94,0x3E,0x36,0x90,0x99,0xF5,0xB4,0x99};
  thermometerService = gatt.addService(thermometerServiceUUID);
  uint8_t thermometerCharacteristicUUID[] = {0x1D,0x8A,0x68,0xE1,0xE6,0x8E,0x4F,0xED,0x94,0x3E,0x36,0x90,0x99,0xF5,0xB4,0x99};
  temperatureCharacteristic = gatt.addCharacteristic(thermometerCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(float), sizeof(float), BLE_DATATYPE_BYTEARRAY);



                            //    57   E5   4B   F0-  85   74-  47   BE-  9C   1D-  A0   DB   FC   8F   A1   83
  uint8_t lightServiceUUID[] = {0x57,0xE5,0x4B,0xF0,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
//  uint8_t lightServiceUUID[] = {0x1D,0x8A,0x68,0xE0,0xE6,0x8E,0x4F,0xED,0x94,0x3E,0x36,0x90,0x99,0xF5,0xB4,0x99};
  lightService = gatt.addService(lightServiceUUID);
  uint8_t lightCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF1,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  lightCharacteristic = gatt.addCharacteristic(lightCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(bool),sizeof(bool), BLE_DATATYPE_AUTO, "On");
                                                 
  uint8_t hueCharacteristicUUID[] = {0x57,0xE5,0x4B,0xF2,0x85,0x74,0x47,0xBE,0x9C,0x1D,0xA0,0xDB,0xFC,0x8F,0xA1,0x83};
  hueCharacteristic = gatt.addCharacteristic(hueCharacteristicUUID,
                                                     GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE | GATT_CHARS_PROPERTIES_NOTIFY,
                                                     sizeof(bool),sizeof(bool), BLE_DATATYPE_AUTO, "On");


                                                     
  Serial.print("lightCharacteristic: ");
  Serial.println(lightCharacteristic);
  ble.reset();
  ble.setConnectCallback(centralConnect);
  ble.setDisconnectCallback(centralDisconnect);
  Serial.println("Bluetooth on");


  /* Only one BLE GATT function should be set, it is possible to set it
    multiple times for multiple Chars ID  */
  ble.setBleGattRxCallback(lightCharacteristic, lightRX);
  ble.setBleGattRxCallback(hueCharacteristic, lightRX);

}


void lightRX(int32_t chars_id, uint8_t data[], uint16_t len)
{
  Serial.print( F("[BLE GATT RX] (" ) );
  Serial.print(chars_id);
  
  Serial.print(") ");
  for (int i = 0; i < len; i++) { Serial.print(i); Serial.print(":"); Serial.print(data[i]);Serial.print("  ");}
  if (chars_id == lightCharacteristic)
  {

//    for (int i = 0; i < len; i++) { Serial.print(i); Serial.print(":"); Serial.print(data[i]);Serial.print("  ");}

    Serial.print(len);
    
    Serial.println("ENDDERP");
    setLED(data[0]);
  } 

}
void setLED(bool on) {
  Serial.print("LED | ");
  Serial.print(on);
  if (on) {
    // turn on neopixel to red
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(255,0,0)); // off
    }
  } else {
    // turn off neopixel
    for(uint8_t i=0; i<NUMPIXELS; i++) {
      pixel.setPixelColor(i, pixel.Color(0,0,0)); // off
    }
  }
  pixel.show();
  
}

void loop(void) {
  ble.update();

  static union float_bytes averageCelsius = { .value = 0.0 };
  if (ble.sendCommandCheckOK("AT+HWGETDIETEMP")) {
    float currentCelsius = atof(ble.buffer);
    averageCelsius.value += (currentCelsius - averageCelsius.value) / 30.0;
  }
  union float_bytes previousCelsius = { .value = 0.0 };
  gatt.getChar(temperatureCharacteristic, previousCelsius.bytes, sizeof(previousCelsius));

  if (abs(averageCelsius.value - previousCelsius.value) > 0.50) {
    gatt.setChar(temperatureCharacteristic, averageCelsius.bytes, sizeof(averageCelsius));
    Serial.print("Update temperature | ");
    Serial.println(averageCelsius.value);
  } 

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

