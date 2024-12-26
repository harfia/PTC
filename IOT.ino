#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>

// Konfigurasi Wi-Fi dan Firebase
#define SSID "Pioo" // Nama Wi-Fi
#define PASSWORD "12345678" // Password Wi-Fi
#define API_KEY "AIzaSyB4dI3QP-X8SF27M-tIb7lHSfQisg6WQto" // API Key Firebase
#define DATABASE_URL "https://latihan-kirim-data-esp-f8c9b-default-rtdb.asia-southeast1.firebasedatabase.app/" // URL Database Firebase

// Pin untuk sensor PIR, relay, dan DHT (Ruangan 1)
#define PIR1_SENSOR_PIN 18   // Pin untuk PIR sensor Ruangan 1
#define RELAY1_1_PIN 26    // Pin untuk kipas 1 Ruangan 1
#define RELAY1_2_PIN 25    // Pin untuk kipas 2 Ruangan 1
#define DHT1_PIN 15       // Pin untuk DHT sensor Ruangan 1
#define DHTTYPE DHT11      // Jenis sensor DHT (DHT11)

// Pin untuk sensor PIR, relay, dan DHT (Ruangan 2)
#define PIR2_SENSOR_PIN 19 // Pin untuk PIR sensor Ruangan 2
#define RELAY2_1_PIN 14    // Pin untuk kipas 1 Ruangan 2
#define RELAY2_2_PIN 27    // Pin untuk kipas 2 Ruangan 2
#define DHT2_PIN 4        // Pin untuk DHT sensor Ruangan 2
#define DHTTYPE DHT11      // Jenis sensor DHT (DHT11)

// Inisialisasi sensor DHT untuk masing-masing ruangan
DHT dht1(DHT1_PIN, DHTTYPE);
DHT dht2(DHT2_PIN, DHTTYPE);

// Inisialisasi objek Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool firebaseConnected = false;

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);

  // Konfigurasi pin untuk Ruangan 1
  pinMode(PIR1_SENSOR_PIN, INPUT);
  pinMode(RELAY1_1_PIN, OUTPUT);
  pinMode(RELAY1_2_PIN, OUTPUT);

  // Konfigurasi pin untuk Ruangan 2
  pinMode(PIR2_SENSOR_PIN, INPUT);
  pinMode(RELAY2_1_PIN, OUTPUT);
  pinMode(RELAY2_2_PIN, OUTPUT);

  // Awal kondisi relay mati
  digitalWrite(RELAY1_1_PIN, HIGH);
  digitalWrite(RELAY1_2_PIN, HIGH);
  digitalWrite(RELAY2_1_PIN, HIGH);
  digitalWrite(RELAY2_2_PIN, HIGH);

  // Inisialisasi sensor DHT
  dht1.begin();
  dht2.begin();

  // Koneksi Wi-Fi
  Serial.println("Memulai koneksi Wi-Fi...");
  WiFi.begin(SSID, PASSWORD);

  // Tunggu hingga terhubung ke Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWi-Fi terhubung. IP: " + WiFi.localIP().toString());

    // --- Konfigurasi Firebase ---
  Serial.println("Mengonfigurasi Firebase...");
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Serial.println("Menghubungkan ke Firebase...");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("SignUp berhasil.");
    firebaseConnected = true;
  } else {
    Serial.printf("Error SignUp: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Serial.println("Firebase berhasil dimulai.");
}

void loop() {
  // Membaca data Ruangan 1
  Serial.println("\n=== Data Ruangan 1 ===");
  processRoom(PIR1_SENSOR_PIN, RELAY1_1_PIN, RELAY1_2_PIN, dht1, "Ruangan 1");

  // Membaca data Ruangan 2
  Serial.println("\n=== Data Ruangan 2 ===");
  processRoom(PIR2_SENSOR_PIN, RELAY2_1_PIN, RELAY2_2_PIN, dht2, "Ruangan 2");

  delay(2000); // Delay untuk pembacaan berikutnya
}

// Fungsi untuk membaca sensor dan mengontrol kipas di masing-masing ruangan
void processRoom(int pirPin, int relay1Pin, int relay2Pin, DHT &dht, const char *roomName) {
  int pirStatus = digitalRead(pirPin);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor tidak valid.");
    return;
  }

  Serial.print("Gerakan: ");
  Serial.println(pirStatus == HIGH ? "Terdeteksi" : "Tidak terdeteksi");
  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Kelembaban: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Mengirim data ke Firebase
  if (firebaseConnected) {
    String path = "/" + String(roomName);
    Firebase.RTDB.setBool(&fbdo, path + "/PIR_Sensor", pirStatus == HIGH);
    Firebase.RTDB.setFloat(&fbdo, path + "/TemperatureC", temperature);
    Firebase.RTDB.setFloat(&fbdo, path + "/Humidity", humidity);

    if (fbdo.errorReason()) {
      Serial.print("Gagal mengirim data ke Firebase: ");
      Serial.println(fbdo.errorReason());
    } else {
      Serial.println("Data berhasil dikirim ke Firebase.");
    }
  }


  if (pirStatus == HIGH) {
    Serial.print("Gerakan terdeteksi di ");
    Serial.println(roomName);

    if (temperature < 26.0) {
      digitalWrite(relay1Pin, HIGH);
      digitalWrite(relay2Pin, LOW);
      Serial.println("Kipas 1: Mati");
      Serial.println("Kipas 2: Menyala");
    } else {
      digitalWrite(relay1Pin, LOW);
      digitalWrite(relay2Pin, LOW);
      Serial.println("Kipas 1: Menyala");
      Serial.println("Kipas 2: Menyala");
    }
  } else {
    Serial.print("Tidak ada gerakan di ");
    Serial.println(roomName);

    if (temperature < 26.0) {
      digitalWrite(relay1Pin, HIGH);
      digitalWrite(relay2Pin, HIGH);
      Serial.println("Kipas 1: Mati");
      Serial.println("Kipas 2: Mati");
    } else {
      digitalWrite(relay1Pin, LOW);
      digitalWrite(relay2Pin, HIGH);
      Serial.println("Kipas 1: Menyala");
      Serial.println("Kipas 2: Mati");
    }
  }
}

void processRoom2(int pirPin, int relay1Pin, int relay2Pin, DHT &dht, const char *roomName) {
  int pirStatus = digitalRead(pirPin);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor tidak valid.");
    return;
  }

  Serial.print("Gerakan: ");
  Serial.println(pirStatus == HIGH ? "Terdeteksi" : "Tidak terdeteksi");
  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Kelembaban: ");
  Serial.print(humidity);
  Serial.println(" %");

  if (pirStatus == HIGH) {
    Serial.print("Gerakan terdeteksi di ");
    Serial.println(roomName);

    digitalWrite(relay1Pin, LOW);
    digitalWrite(relay2Pin, LOW);
    Serial.println("Kipas 1: Menyala");
    Serial.println("Kipas 2: Menyala");
  } else if (temperature > 26.0) {
    Serial.print("Tidak ada gerakan di ");
    Serial.println(roomName);

    digitalWrite(relay1Pin, LOW);
    digitalWrite(relay2Pin, HIGH);
    Serial.println("Kipas 1: Menyala");
    Serial.println("Kipas 2: Mati");
  } else {
    Serial.print("Tidak ada gerakan dan suhu < 26 °C di ");
    Serial.println(roomName);

    digitalWrite(relay1Pin, HIGH);
    digitalWrite(relay2Pin, HIGH);
    Serial.println("Kipas 1: Mati");
    Serial.println("Kipas 2: Mati");
  }
}
