#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;

// IR sensor pins (for 2 parking spots)
#define IR1 33
#define IR2 32

// WiFi + ThingSpeak details
const char* ssid = "Redmi";
const char* password = "Hrutika@02";
String apiKey = "ST6Y11ATG35ECUAA";
const char* server = "https://api.thingspeak.com/update";

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  gateServo.attach(25);

  // --- WiFi connection ---
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("WiFi Connected");
  } else {
    lcd.print("WiFi Failed");
  }
  delay(1500);
  lcd.clear();
}

void loop() {
  int ir1 = digitalRead(IR1);
  int ir2 = digitalRead(IR2);

  // Calculate available spots
  int availableSpots = 0;
  if (ir1 == HIGH) availableSpots++;
  if (ir2 == HIGH) availableSpots++;

  Serial.print("IR1: "); Serial.print(ir1);
  Serial.print(" | IR2: "); Serial.print(ir2);
  Serial.print(" | Free Spots: "); Serial.println(availableSpots);

  // --- Display and Servo Logic ---
  if (availableSpots == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No Parking Space");
    lcd.setCursor(0, 1);
    lcd.print("Gate Closed     ");
    gateServo.write(0); // Close gate
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(availableSpots) + " Spot Available");
    lcd.setCursor(0, 1);
    lcd.print("Gate Opening...");
    gateServo.write(90); // Open gate
    delay(2000);
    gateServo.write(0);  // Close after few seconds
  }

  // --- Upload data to ThingSpeak ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(availableSpots) +
                 "&field2=" + String(ir1 == LOW ? 0 : 1) +
                 "&field3=" + String(ir2 == LOW ? 0 : 1);
    http.begin(url);
    int httpCode = http.GET();
    http.end();
    Serial.println("Data sent to ThingSpeak");
  }

  delay(15000); // ThingSpeak needs 15s gap between uploads
}
