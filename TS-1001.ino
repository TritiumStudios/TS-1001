// Bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Flash storage
#include <Preferences.h>

// Timer
#include "esp_system.h"

// LEDs
#include <Adafruit_NeoPixel.h>

#define SWITCH_PIN 13

#define LED_PIN 14
#define LED_COUNT 38

#define SERVICE_UUID              "b8bcb500-7296-4f1d-9a0e-25c2b7d5878a"
#define COLOR_CHARACTERISTIC_UUID "b8bcb501-7296-4f1d-9a0e-25c2b7d5878a"
#define POWER_CHARACTERISTIC_UUID "b8bcb502-7296-4f1d-9a0e-25c2b7d5878a"
#define BLE_NAME "TS-1001"

#define DEFAULT_RED 0xff
#define DEFAULT_GREEN 0xff
#define DEFAULT_BLUE 0xff
#define DEFAULT_COLOR DEFAULT_RED << 16 | DEFAULT_GREEN << 8 | DEFAULT_BLUE

#define DEFAULT_POWER 0x01

Preferences preferences;

const int wdtTimeout = 5000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;

static uint8_t current_color[3] = {DEFAULT_RED, DEFAULT_GREEN, DEFAULT_BLUE};
static uint8_t current_power = DEFAULT_POWER;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

BLECharacteristic *colorCharacteristic;
BLECharacteristic *powerCharacteristic;

int switchState = 0;
int prevSwitchState = 0;

void storeData() {
  uint32_t color = current_color[0] << 16 | current_color[1] << 8 | current_color[2];

  preferences.begin("led", false);

  // Serial.print("storing color: ");
  // Serial.println(color, HEX);
  preferences.putUInt("color", color);

  // Serial.print("storing power: ");
  // Serial.println(current_power, HEX);
  preferences.putUChar("power", current_power);

  preferences.end();
}

void ARDUINO_ISR_ATTR onTimer(){
  timerStop(timer);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void restartTimer() {
  timerStop(timer);
  timerRestart(timer);
  timerStart(timer);
}

void setLedColor() {
  for(int i=0; i < LED_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(current_color[0], current_color[1], current_color[2]));
    pixels.show();
  }
}

void setLedOff() {
  for(int i=0; i < LED_COUNT; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

//function takes String and adds manufacturer code at the beginning 
void setBleData() {
  std::string value = {0xff, 0xff, current_power, current_color[0], current_color[1], current_color[2]};

  // Serial.print("Manufacturing Data: ");
  // for (int i = 0; i < value.length(); i++) {
  //   Serial.printf("%02X", value[i]);
  // }
  // Serial.println();
  
  BLEAdvertising *pAdvertising;
  BLEAdvertisementData advData;
  
  pAdvertising->stop();
  pAdvertising = BLEDevice::getAdvertising();
  advData.setCompleteServices(BLEUUID::fromString(SERVICE_UUID));
  advData.setManufacturerData(value);
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponse(false);
  pAdvertising->start();
}

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (pCharacteristic->getUUID().equals(colorCharacteristic->getUUID()))
    {
      std::string value = pCharacteristic->getValue();
      if (value.length() == 3) {
        restartTimer();

        Serial.print("New color value: #");
        for (int i = 0; i < value.length(); i++) {
          Serial.printf("%02X", value[i]);
        }

        Serial.println();

        current_color[0] = (uint8_t)value[0];
        current_color[1] = (uint8_t)value[1];
        current_color[2] = (uint8_t)value[2];
        current_power = 0x01;

        setLedColor();

        setBleData();
      }
    } else if (pCharacteristic->getUUID().equals(powerCharacteristic->getUUID())) {
      std::string value = pCharacteristic->getValue();
      if (value.length() == 1) {
        restartTimer();

        current_power = (uint8_t)value[0];
        if (current_power == 0x00) {
          Serial.println("Power off");
          setLedOff();
        } else if (current_power == 0x01) {
          Serial.println("Power on");
          setLedColor();
        }

        setBleData();
      }
    }
  }
};


void setup() {
  Serial.begin(115200);

  pixels.begin();
  pixels.clear();

  preferences.begin("led", false);
  uint32_t initial_color = preferences.getUInt("color", DEFAULT_COLOR);
  uint8_t initial_power = preferences.getUChar("power", DEFAULT_POWER);
  preferences.end();

  current_color[0] = initial_color >> 16;
  current_color[1] = initial_color >> 8;
  current_color[2] = initial_color;
  current_power = initial_power;

  if (current_power == 0x00) {
    Serial.println("Initial power off");
    setLedOff();
  } else if (current_power == 0x01) {
    Serial.println("Initial power on");
    Serial.print("Initial color: #");
    for (int i = 0; i < 3; i++) {
      Serial.printf("%02X", current_color[i]);
    }

    Serial.println();    
    setLedColor();
  }

  timerSemaphore = xSemaphoreCreateBinary();

  timer = timerBegin(0, 80, true);                  // timer 0, div 80
  timerAttachInterrupt(timer, &onTimer, true);    // attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, true);  // set time in us
  timerAlarmEnable(timer);                          // enable interrupt


  BLEDevice::init(BLE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // color
  colorCharacteristic = pService->createCharacteristic(
                                         COLOR_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  colorCharacteristic->setCallbacks(new MyCallbacks());
  uint8_t color_value[3] = {current_color[0], current_color[1], current_color[2]};
  colorCharacteristic->setValue(color_value, 3);

  // power
  powerCharacteristic = pService->createCharacteristic(
                                         POWER_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  powerCharacteristic->setCallbacks(new MyCallbacks());
  uint8_t power_value[1] = {current_power};
  powerCharacteristic->setValue(power_value, 1);

  pService->start();
  
  setBleData();

  pinMode(SWITCH_PIN, INPUT);
}

void loop() {
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    storeData();
  }

  switchState = digitalRead(SWITCH_PIN);
  if (switchState != prevSwitchState) {
    if (switchState == HIGH) {
      current_power = 0x01;
      uint8_t power_value[1] = {current_power};
      powerCharacteristic->setValue(power_value, 1);
      setLedColor();
      setBleData();
    } else {
      current_power = 0x00;
      uint8_t power_value[1] = {current_power};
      powerCharacteristic->setValue(power_value, 1);
      setLedOff();
      setBleData();
    }
  }
}