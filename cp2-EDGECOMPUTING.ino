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

#define BTN_MENU      9   // entra/sai do menu de setup
#define BTN_NEXT      10  // próxima opção
#define BTN_SELECT    11  // confirma seleção

// ── Objetos ──────────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);
RTC_DS3231 rtc;

// ── Configurações (default) ──────────────────────────────────
int  utcOffset   = -3;    // UTC-3 (Brasília)
bool useCelsius  = true;
bool usePtBr     = true;

// ── Calibração LDR ───────────────────────────────────────────
int ldrMin = 1023;
int ldrMax = 0;

// ── Acumuladores para média 10 s ─────────────────────────────
const int  SAMPLE_INTERVAL = 1000;  // ms entre amostras
const int  SAMPLES_COUNT   = 10;
float sumTemp = 0, sumHum = 0, sumLux = 0;
int   sampleIdx = 0;
unsigned long lastSample = 0;

float avgTemp = 0, avgHum = 0, avgLux = 0;
bool  firstAvg = false;

// ── Controle de telas ─────────────────────────────────────────
// 0 = logo animado, 1 = lux, 2 = temp, 3 = hum
int  displayMode   = 0;
unsigned long lastScreenChange = 0;
const unsigned long SCREEN_INTERVAL = 3000;

// ── Log na EEPROM ─────────────────────────────────────────────
// Cada entrada = 8 bytes: temp*10(2B), hum*10(2B), lux(2B), flags(1B), chk(1B)
#define EEPROM_SIZE    512
#define LOG_SIZE       8
#define MAX_LOGS       (EEPROM_SIZE / LOG_SIZE)
int logAddr = 0;

// ── Setup menu ────────────────────────────────────────────────
bool inMenu      = false;
int  menuItem    = 0;   // 0=UTC, 1=Unidade, 2=Idioma, 3=Sair
int  menuOption  = 0;

// ── Debounce botões ──────────────────────────────────────────
unsigned long lastBtnMenu = 0, lastBtnNext = 0, lastBtnSel = 0;
const unsigned long DEBOUNCE = 200;

// ── Caracteres customizados (logo vinho) ──────────────────────
byte charBottle[8] = {0b00100,0b01110,0b01110,0b11111,0b11111,0b11111,0b01110,0b00100};
byte charGlass[8]  = {0b11111,0b11111,0b01110,0b00100,0b00100,0b01110,0b11111,0b00000};
byte charHeart[8]  = {0b00000,0b01010,0b11111,0b11111,0b01110,0b00100,0b00000,0b00000};
byte charNote[8]   = {0b00100,0b00110,0b00101,0b00101,0b00100,0b11100,0b11100,0b00000};

// ── Protótipos ────────────────────────────────────────────────
void showLogo();
void showLux();
void showTemp();
void showHum();
void handleAlerts();
void saveLog();
void handleMenu();
void calibrateLDR();
bool btnPressed(int pin, unsigned long &last);

// ── SETUP ─────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, charBottle);
  lcd.createChar(1, charGlass);
  lcd.createChar(2, charHeart);
  lcd.createChar(3, charNote);

  dht.begin();

  if (!rtc.begin()) {
    lcd.setCursor(0, 0); lcd.print("RTC nao encontrado");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(LED_GREEN,  OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_MENU,   INPUT_PULLUP);
  pinMode(BTN_NEXT,   INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  // Calibração inicial do LDR (2 s)
  unsigned long t = millis();
  while (millis() - t < 2000) calibrateLDR();

  showLogo();
  delay(3000);
  displayMode = 1;
}

// ── LOOP ──────────────────────────────────────────────────────
void loop() {
  // Botão Menu
  if (btnPressed(BTN_MENU, lastBtnMenu)) {
    inMenu = !inMenu;
    menuItem = 0; menuOption = 0;
    if (inMenu) {
      lcd.clear();
      lcd.print(">> MENU SETUP <<");
    }
  }

  if (inMenu) {
    handleMenu();
    return;
  }

  // Amostragem
  calibrateLDR();
  unsigned long now = millis();
  if (now - lastSample >= SAMPLE_INTERVAL) {
    lastSample = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int   raw = analogRead(LDR_PIN);
    int   lux = map(raw, ldrMin, ldrMax, 0, 100);
    lux = constrain(lux, 100, 0);

    sumTemp += t;
    sumHum  += h;
    sumLux  += lux;
    sampleIdx++;

    if (sampleIdx >= SAMPLES_COUNT) {
      avgTemp = sumTemp / SAMPLES_COUNT;
      avgHum  = sumHum  / SAMPLES_COUNT;
      avgLux  = sumLux  / SAMPLES_COUNT;
      sumTemp = sumHum = sumLux = 0;
      sampleIdx = 0;
      firstAvg = true;
      saveLog();
    }
  }

  if (!firstAvg) return;   // aguarda primeira média

  // Troca automática de tela
  if (now - lastScreenChange >= SCREEN_INTERVAL) {
    lastScreenChange = now;
    displayMode++;
    if (displayMode > 3) displayMode = 1;
    lcd.clear();
  }

  handleAlerts();

  switch (displayMode) {
    case 1: showLux();  break;
    case 2: showTemp(); break;
    case 3: showHum();  break;
  }
}

// ── TELA: LOGO ANIMADO ────────────────────────────────────────
void showLogo() {
  lcd.clear();
  // Linha 0: animação slide-in
  String title = " VINHERIA AGNELLO";
  for (int i = 0; i < (int)title.length(); i++) {
    lcd.setCursor(i, 0);
    lcd.print(title[i]);
    delay(80);
  }
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); lcd.print("   MOOCA  ");
  lcd.write(byte(1));
  delay(1500);

  // Pisca ícones
  for (int k = 0; k < 3; k++) {
    lcd.setCursor(0, 1); lcd.write(byte(2));
    lcd.setCursor(15,1); lcd.write(byte(3));
    delay(400);
    lcd.setCursor(0, 1); lcd.print(" ");
    lcd.setCursor(15,1); lcd.print(" ");
    delay(400);
  }
}

// ── TELA: LUMINOSIDADE ───────────────────────────────────────
void showLux() {
  DateTime now = rtc.now();
  int h = now.hour() + utcOffset;
  if (h < 0) h += 24;
  if (h > 23) h -= 24;

  lcd.setCursor(0, 0);
  if (usePtBr) lcd.print("Luz:"); else lcd.print("Light:");
  lcd.setCursor(5, 0);
  lcd.print((int)avgLux);
  lcd.print("%   ");

  lcd.setCursor(0, 1);
  char buf[17];
  sprintf(buf, "%02d:%02d:%02d UTC%+d", h, now.minute(), now.second(), utcOffset);
  lcd.print(buf);
}

// ── TELA: TEMPERATURA ────────────────────────────────────────
void showTemp() {
  lcd.setCursor(0, 0);
  if (usePtBr) lcd.print("Temperatura:    "); else lcd.print("Temperature:    ");

  lcd.setCursor(0, 1);
  float t = useCelsius ? avgTemp : avgTemp * 9.0 / 5.0 + 32.0;
  char buf[17];
  if (useCelsius)
    sprintf(buf, "%.1f C  ", t);
  else
    sprintf(buf, "%.1f F  ", t);
  lcd.print(buf);

  // Ícone de estado
  lcd.setCursor(10, 1);
  if (avgTemp < 10 || avgTemp > 18) {
    lcd.print(usePtBr ? "CRITICO!" : "CRITICAL");
  } else if (avgTemp < 12 || avgTemp > 16) {
    lcd.print(usePtBr ? "ATENCAO " : "WARNING ");
  } else {
    lcd.print(usePtBr ? "  OK    " : "  OK    ");
  }
}

// ── TELA: UMIDADE ────────────────────────────────────────────
void showHum() {
  lcd.setCursor(0, 0);
  if (usePtBr) lcd.print("Umidade:        "); else lcd.print("Humidity:       ");

  lcd.setCursor(0, 1);
  char buf[17];
  sprintf(buf, "%.1f%%  ", avgHum);
  lcd.print(buf);

  lcd.setCursor(8, 1);
  if (avgHum < 50 || avgHum > 80) {
    lcd.print(usePtBr ? "CRITICO!" : "CRITICAL");
  } else if (avgHum < 60 || avgHum > 75) {
    lcd.print(usePtBr ? "ATENCAO " : "WARNING ");
  } else {
    lcd.print(usePtBr ? "  OK    " : "  OK    ");
  }
}

// ── ALERTAS (LEDs + Buzzer) ───────────────────────────────────
void handleAlerts() {
  bool luzCrit  = (avgLux < 20 || avgLux > 80);
  bool luzWarn  = !luzCrit && (avgLux < 30 || avgLux > 70);
  bool tempCrit = (avgTemp < 10 || avgTemp > 18);
  bool tempWarn = !tempCrit && (avgTemp < 12 || avgTemp > 16);
  bool humCrit  = (avgHum  < 50 || avgHum  > 80);
  bool humWarn  = !humCrit  && (avgHum  < 60 || avgHum  > 75);

  bool isCrit = luzCrit || tempCrit || humCrit;
  bool isWarn = !isCrit && (luzWarn || tempWarn || humWarn);

  digitalWrite(LED_GREEN,  (!isCrit && !isWarn) ? HIGH : LOW);
  digitalWrite(LED_YELLOW, isWarn ? HIGH : LOW);
  digitalWrite(LED_RED,    isCrit  ? HIGH : LOW);

  if (isCrit) {
    // beep rápido
    tone(BUZZER_PIN, 880, 100);
  } else if (isWarn) {
    // beep lento
    static unsigned long lastBeep = 0;
    if (millis() - lastBeep > 2000) {
      tone(BUZZER_PIN, 440, 200);
      lastBeep = millis();
    }
  } else {
    noTone(BUZZER_PIN);
  }
}

// ── LOG NA EEPROM + SERIAL MONITOR ───────────────────────────
void saveLog() {

  int16_t t16 = (int16_t)(avgTemp * 10);
  int16_t h16 = (int16_t)(avgHum  * 10);
  int16_t l16 = (int16_t)(avgLux);

  uint8_t flags = 0;

  // Flags de alerta
  if (avgTemp < 10 || avgTemp > 18) flags |= 0x01;
  if (avgHum  < 50 || avgHum  > 80) flags |= 0x02;
  if (avgLux  < 20 || avgLux  > 80) flags |= 0x04;

  // Checksum
  uint8_t chk = (uint8_t)(t16 ^ h16 ^ l16 ^ flags);

  // Salva na EEPROM
  EEPROM.put(logAddr,     t16);
  EEPROM.put(logAddr + 2, h16);
  EEPROM.put(logAddr + 4, l16);
  EEPROM.put(logAddr + 6, flags);
  EEPROM.put(logAddr + 7, chk);

  // Próximo endereço
  logAddr = (logAddr + LOG_SIZE) % (MAX_LOGS * LOG_SIZE);

  // ===== SERIAL MONITOR =====

  DateTime now = rtc.now();

  int hora = now.hour() + utcOffset;

  if (hora < 0)  hora += 24;
  if (hora > 23) hora -= 24;

  Serial.println("====================================");

  // Horário
  Serial.print("Horario: ");

  if (hora < 10) Serial.print("0");
  Serial.print(hora);
  Serial.print(":");

  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");

  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());

  // Temperatura
  Serial.print("Temperatura: ");

  if (useCelsius) {
    Serial.print(avgTemp, 1);
    Serial.println(" C");
  } else {
    float tempF = avgTemp * 9.0 / 5.0 + 32.0;
    Serial.print(tempF, 1);
    Serial.println(" F");
  }

  // Umidade
  Serial.print("Umidade: ");
  Serial.print(avgHum, 1);
  Serial.println(" %");

  // Luz
  Serial.print("Luminosidade: ");
  Serial.print(avgLux, 0);
  Serial.println(" %");

  // Status
  bool critico =
    (avgTemp < 10 || avgTemp > 18) ||
    (avgHum  < 50 || avgHum  > 80) ||
    (avgLux  < 20 || avgLux  > 80);

  bool alerta =
    !critico &&
    (
      (avgTemp < 12 || avgTemp > 16) ||
      (avgHum  < 60 || avgHum  > 75) ||
      (avgLux  < 30 || avgLux  > 70)
    );

  Serial.print("Status: ");

  if (critico) {
    Serial.println("CRITICO");
  }
  else if (alerta) {
    Serial.println("ATENCAO");
  }
  else {
    Serial.println("OK");
  }

  // Flags
  Serial.print("Flags: 0x");
  Serial.println(flags, HEX);

  // Checksum
  Serial.print("Checksum: ");
  Serial.println(chk);

  Serial.println("====================================");
  Serial.println();
}
// ── CALIBRAÇÃO AUTOMÁTICA LDR ─────────────────────────────────
void calibrateLDR() {
  int raw = analogRead(LDR_PIN);
  if (raw < ldrMin) ldrMin = raw;
  if (raw > ldrMax) ldrMax = raw;
  if (ldrMax == ldrMin) ldrMax = ldrMin + 1; // evita div/0
}

// ── MENU DE SETUP ────────────────────────────────────────────
void handleMenu() {
  // Navegação
  if (btnPressed(BTN_NEXT, lastBtnNext)) {
    menuOption++;
    lcd.clear();
  }

  switch (menuItem) {
    case 0: { // UTC
      int opts[] = {-5,-4,-3,-2,-1,0,1,2,3};
      int n = 9;
      menuOption = menuOption % n;
      lcd.setCursor(0,0); lcd.print("Fuso horario UTC");
      lcd.setCursor(0,1);
      char buf[17];
      sprintf(buf, "UTC%+d           ", opts[menuOption]);
      lcd.print(buf);
      if (btnPressed(BTN_SELECT, lastBtnSel)) {
        utcOffset = opts[menuOption];
        menuItem++; menuOption = 0; lcd.clear();
      }
      break;
    }
    case 1: { // Unidade
      menuOption = menuOption % 2;
      lcd.setCursor(0,0); lcd.print("Unidade Temp.   ");
      lcd.setCursor(0,1);
      lcd.print(menuOption == 0 ? "Celsius (C)     " : "Fahrenheit (F)  ");
      if (btnPressed(BTN_SELECT, lastBtnSel)) {
        useCelsius = (menuOption == 0);
        menuItem++; menuOption = 0; lcd.clear();
      }
      break;
    }
    case 2: { // Idioma
      menuOption = menuOption % 2;
      lcd.setCursor(0,0); lcd.print("Idioma / Lang   ");
      lcd.setCursor(0,1);
      lcd.print(menuOption == 0 ? "Portugues       " : "English         ");
      if (btnPressed(BTN_SELECT, lastBtnSel)) {
        usePtBr = (menuOption == 0);
        menuItem++; menuOption = 0; lcd.clear();
      }
      break;
    }
    default: { // Sair
      lcd.setCursor(0,0); lcd.print("Config. salvas! ");
      lcd.setCursor(0,1); lcd.print("Saindo do menu..");
      delay(1500);
      inMenu = false;
      displayMode = 1;
      lcd.clear();
      break;
    }
  }
}

// ── DEBOUNCE ─────────────────────────────────────────────────
bool btnPressed(int pin, unsigned long &last) {
  if (digitalRead(pin) == LOW && millis() - last > DEBOUNCE) {
    last = millis();
    return true;
  }
  return false;
}