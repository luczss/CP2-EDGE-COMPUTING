// ============================================================
//  Vinheria Agnello – Checkpoint 02
//  FIAP – Edge Computing & Computer Systems
//  Sensores: DHT22, LDR | Display: LCD I2C 16x2
//  RTC DS1307, EEPROM, LEDs, Buzzer, Botões
// ============================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <EEPROM.h>
#include <RTClib.h>

// ── Pinos ────────────────────────────────────────────────────
#define DHT_PIN       2
#define DHT_TYPE      DHT22
#define LDR_PIN       A0

#define LED_GREEN     5
#define LED_YELLOW    6
#define LED_RED       7
#define BUZZER_PIN    8

#define BTN_MENU      9
#define BTN_NEXT      10
#define BTN_SELECT    11

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);
RTC_DS1307 rtc;

void setup() {
  Serial.begin(115200);
}

void loop() {
}
