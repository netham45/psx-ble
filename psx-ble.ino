#include <BleGamepad.h> // ESP32-BLE-Gamepad - https://github.com/lemmingDev/ESP32-BLE-Gamepad, NimBLE - https://github.com/h2zero/NimBLE-Arduino
#include <PsxControllerHwSpi.h> // PsxNewLib - https://github.com/SukkoPera/PsxNewLib, DigitalIO - https://github.com/greiman/DigitalIO
#include <Battery18650Stats.h> // Battery18650Stats - https://github.com/danilopinotti/Battery18650Stats
#define LED_PIN 2
#define TOUCH_PIN T0
#define TOUCH_THRESHOLD 15
#define SLEEP_SECONDS 600
const byte PIN_PS2_ATT = 5;

Battery18650Stats battery;
BleGamepad bleGamepad("BLE PS1/PS2 Adapter", "Netham45", 100);
BleGamepadConfiguration bleGamepadConfig;
PsxControllerHwSpi<PIN_PS2_ATT> psx;
uint32_t lastButtonPress = millis();
uint32_t lastTouch = 0;
bool touched = false;
uint8_t batteryLevel;
void callback() {}

void deepSleep() {
  for (int i=0;i<15;i++) // Flash the LED a few times
  {
    digitalWrite(LED_PIN, i%2);
    delay(100);
  }
  bleGamepad.end();
  esp_sleep_enable_touchpad_wakeup();
  touchAttachInterrupt(TOUCH_PIN, callback, 40);
  
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // Turn off everything that can be turned off
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start(); 
}

void handleTouch() {
  uint8_t touchValue = touchRead(TOUCH_PIN);
  if ( touchValue < TOUCH_THRESHOLD && !touched ) { // Handle Touch
    lastTouch = millis();
  }
  touched = touchValue < TOUCH_THRESHOLD;
  if ( touched && ( (millis() - lastTouch > 1000) ) ) {
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  batteryLevel = battery.getBatteryChargeLevel();
  Serial.printf("Battery level: %i\n",battery.getBatteryChargeLevel());
  Serial.printf("Battery level (converted): %i\n",battery.getBatteryChargeLevel(true));
  Serial.printf("Battery Voltage: %f\n",battery.getBatteryVolts());
  if (battery.getBatteryVolts() < 3)
  {
    Serial.printf("Battery dead!\n");
    for (int i=0;i<60;i++) // Flash the LED a bunch of times really fast to indicate dead battery
    {
      digitalWrite(LED_PIN, i%2);
      delay(20);
    }
    deepSleep(); // Go back to sleep
  }
  bleGamepadConfig.setWhichAxes(true, true, true, true, false, false, false, false);
  bleGamepadConfig.setHatSwitchCount(0);
  bleGamepadConfig.setAxesMax(255);
  bleGamepad.setBatteryLevel(batteryLevel);
  bleGamepad.begin(&bleGamepadConfig);
  pinMode(LED_PIN, OUTPUT);
  if (psx.begin()) { // Try to connect to a PSX controller
    if (psx.enterConfigMode()) {
      psx.enableAnalogSticks();
      psx.enableAnalogButtons();
      psx.exitConfigMode();
    }
  } else { // Sleep if the controller is not connected
    deepSleep();
  }
}

void loop() {
  if (!psx.read()) { // If the PSX controller is now disconnected go to sleep
   deepSleep();
  }
  
  byte lx, ly, rx, ry;
  uint16_t buttonMask = psx.getButtonWord();
  
  psx.getLeftAnalog(lx, ly);
  psx.getRightAnalog(rx, ry);
  bleGamepad.setAxes(rx, ry, lx, 0, ly, 0, 0);
  
  buttonMask = (buttonMask & 0x3FF9) | ((buttonMask & 0x0006) << 13) | ((buttonMask & 0xC000) >> 13); // Swap buttons 1,2 and 14,15 to work around a bug in BlueRetro
  
  if (buttonMask) {
    lastButtonPress = millis();
  }
  
  for (int i=0;i<16;i++) {
    if ((buttonMask >> i) & 1) {
      bleGamepad.press(i + 1);
    } else {
      bleGamepad.release(i + 1);
    }
  }
  
  if (millis() - lastButtonPress > (SLEEP_SECONDS * 1000)) { // If no controller button pressed for 600 seconds go to sleep
    deepSleep();
  }
  digitalWrite(LED_PIN, bleGamepad.isConnected() ? 1 : ((millis() % 500) > 250) );
  handleTouch();
  if (battery.getBatteryChargeLevel() != batteryLevel)
  {
    batteryLevel = battery.getBatteryChargeLevel();
    bleGamepad.setBatteryLevel(batteryLevel);  
  }
}
