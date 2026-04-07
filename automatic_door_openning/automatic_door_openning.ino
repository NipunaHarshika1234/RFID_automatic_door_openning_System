#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h> 

// === Pin Configurations ===
#define SS_PIN     5    // RFID SDA
#define RST_PIN   27    // RFID RST
#define BUZZER_PIN 14   // Buzzer
#define LED_PIN    12   // LED bulb
#define SERVO_PIN  13   // Servo motor

// === Objects ===
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD at 0x27, 16x2
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

// === Authorized RFID Card UID ===
// Replace these values with your card’s UID (use Serial Monitor first)
byte authorizedUID[4] = {0xC, 0xA8, 0xD4, 0x5};

void setup() {
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Door is closed");
  lcd.setCursor(0,1);
  lcd.print("Tap your card!");

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // Servo
  doorServo.attach(SERVO_PIN);
  doorServo.write(0); // closed position

  // Buzzer + LED
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.println("System ready. Waiting for card...");
}

void loop() {
  // Look for card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Print UID for debugging
  Serial.print("Card UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (checkUID(rfid.uid.uidByte)) {
    grantAccess();
  } else {
    denyAccess();
  }

  // Stop reading same card repeatedly
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool checkUID(byte *uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != authorizedUID[i]) {
      return false;
    }
  }
  return true;
}

void grantAccess() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access Granted");
  lcd.setCursor(0,1);
  lcd.print("Door Opening...");

  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, 200); // short beep
  doorServo.write(90); // open door
  delay(5000);         // keep open 5 sec

  // Close again
  doorServo.write(0);
  digitalWrite(LED_PIN, LOW);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Door is closed");
  lcd.setCursor(0,1);
  lcd.print("Tap your card!");
}

void denyAccess() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Access Denied!");
  lcd.setCursor(0,1);
  lcd.print("Tap valid card!");

  // buzzer 3 times
  for (int i=0; i<3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Door is closed");
  lcd.setCursor(0,1);
  lcd.print("Tap your card!");
}
