#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------------- MIC ----------------
#define MIC_PIN 34
#define MIC_THRESHOLD 1500

// ---------------- WIFI & API ----------------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
String GEMINI_API_KEY = "AIzaSyXXXXXXXXXXXX";

// ---------------- UTIL ----------------
void show(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}

// ---------------- GEMINI ----------------
String askGemini(String prompt) {
  HTTPClient http;

  String url =
    "https://generativelanguage.googleapis.com/v1/models/gemini-pro:generateContent?key=" +
    GEMINI_API_KEY;

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  prompt.replace("\"", "\\\"");

  String body =
    "{ \"contents\": [ { \"parts\": [ { \"text\": \"" + prompt + "\" } ] } ] }";

  int code = http.POST(body);
  Serial.print("HTTP CODE: ");
  Serial.println(code);

  if (code != 200) {
    http.end();
    return "API ERROR";
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<2048> doc;
  if (deserializeJson(doc, payload)) {
    return "JSON ERROR";
  }

  return doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  show("Connecting WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    show("WiFi Failed");
    return;
  }

  Serial.println("\nWiFi Connected");
  show("WiFi Connected");
  delay(1000);

  pinMode(MIC_PIN, INPUT);
  show("Speak Loud");
}

// ---------------- LOOP ----------------
void loop() {
  static unsigned long last = 0;
  int mic = analogRead(MIC_PIN);
  Serial.println(mic);

  if (mic > MIC_THRESHOLD && millis() - last > 6000) {
    last = millis();
    show("Detecting...");

    String reply = askGemini("Who are you?");
    reply = reply.substring(0, 120); // OLED limit
    show(reply);
  }

  delay(100);
}
