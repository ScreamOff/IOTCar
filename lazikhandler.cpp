#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "index.h"
const char* wifi_ssid = "ssid";
const char* wifi_password = "haslo";

//Piny do czujników dht
#define DHTPIN 32
#define DHTTYPE DHT11
//Inicjalizacja czujnika
DHT dht(DHTPIN, DHTTYPE);
//Piny do mierzenia natężenia światła
#define PHOTORESISTOR_PIN 34
// L298N motor driver pins
#define FRONT_IN1 15
#define FRONT_IN2 17
#define FRONT_IN3 4
#define FRONT_IN4 16
#define REAR_IN1 27
#define REAR_IN2 14
#define REAR_IN3 12
#define REAR_IN4 13
//Utworzenie serwera na porcie 80
WebServer server(80);
//Obsługa czujników
String getLightIntensity() {
  int lightValue = analogRead(PHOTORESISTOR_PIN);
  if (lightValue < 1000) {
    return "Weak";
  } else if (lightValue < 3000) {
    return "Moderate";
  } else {
    return "Strong";
  }
}
// Endpoint: dane z DHT11
void handleSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  String lightIntensity = getLightIntensity();

  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read from DHT sensor!\"}");
    return;
  }

  // Tworzymy odpowiedź w formacie JSON
  String jsonResponse = "{";
  jsonResponse += "\"temperature\": " + String(temperature) + ",";
  jsonResponse += "\"humidity\": " + String(humidity) + ",";
  jsonResponse += "\"lightIntensity\": \"" + lightIntensity + "\"";
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}

// Endpoint: dane z fotorezystora
void handlePhotocell() {
  String lightIntensity = getLightIntensity();
  server.send(200, "text/plain", "Light Intensity: " + lightIntensity);
}


//Silniki
bool moving = false;
String currentDirection = "";

// Ustawienie pinów
void setPins(int pin1, int pin2, int pin3, int pin4, bool in1, bool in2, bool in3, bool in4) {
  digitalWrite(pin1, in1);
  digitalWrite(pin2, in2);
  digitalWrite(pin3, in3);
  digitalWrite(pin4, in4);
}

// Funkcja do ruchu do przodu
void forward() {
  // Ustawienie pinów dla ruchu do przodu
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, true, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, false, true);
}

// Funkcja do ruchu do tyłu
void back() {
  // Ustawienie pinów dla ruchu do tyłu
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, false, true);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, true, false);
}

// Funkcja do skrętu w lewo
void left() {
  // Ustawienie pinów dla skrętu w lewo
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, true, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, true, false);
}

// Funkcja do skrętu w prawo
void right() {
  // Ustawienie pinów dla skrętu w prawo
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, false, true);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, false, true);
}

// Funkcja do zatrzymania silników
void stopMotors() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, false, false, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, false, false, false);
}


void executeCommand(String command) {
  if (command.startsWith("fwd")) {
    int duration = command.substring(3).toInt();
    setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, true, false);
    setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, false, true);
    delay(duration);
  } else if (command.startsWith("back")) {
    int duration = command.substring(4).toInt();
    setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, false, true);
    setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, true, false);
    delay(duration);
  } else if (command.startsWith("left")) {
    int duration = command.substring(4).toInt();
    setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, true, false);
    setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, true, false);
    delay(duration);
  } else if (command.startsWith("right")) {
    int duration = command.substring(5).toInt();
    setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, false, true);
    setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, false, true);
    delay(duration);
  }
  stopMotors();
}

// Ustawienie pinów
void setup() {
  //inicjalizacja czuników
  dht.begin();
  // Set pin modes as OUTPUT
  pinMode(FRONT_IN1, OUTPUT);
  pinMode(FRONT_IN2, OUTPUT);
  pinMode(FRONT_IN3, OUTPUT);
  pinMode(FRONT_IN4, OUTPUT);
  pinMode(REAR_IN1, OUTPUT);
  pinMode(REAR_IN2, OUTPUT);
  pinMode(REAR_IN3, OUTPUT);
  pinMode(REAR_IN4, OUTPUT);

  // Initialize motor pins to LOW
  stopMotors();

  // Connect to WiFi
  Serial.begin(115200);
  WiFi.begin(wifi_ssid, wifi_password);

  Serial.println("\nConnecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.println("SSID: " + String(wifi_ssid));
  Serial.println("IP: " + WiFi.localIP().toString());

  // Handle control page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });


  //Obsluga endpointów
  // Move control routes
  server.on("/move_fwd", HTTP_POST, []() {
    forward();
    server.send(200, "text/plain", "Moving Forward");
  });

  server.on("/move_back", HTTP_POST, []() {
    back();
    server.send(200, "text/plain", "Moving Backward");
  });

  server.on("/move_left", HTTP_POST, []() {
    left();
    server.send(200, "text/plain", "Turning Left");
  });

  server.on("/move_right", HTTP_POST, []() {
    right();
    server.send(200, "text/plain", "Turning Right");
  });

  // Stop motors
  server.on("/stop", HTTP_POST, []() {
    stopMotors();
    server.send(200, "text/plain", "Motors Stopped");
  });
  server.on("/status", HTTP_GET, []() {
    String status = moving ? "<p>Robot is moving " + currentDirection + "</p>" : "<p>Robot is stopped</p>";
    server.send(200, "text/html", status);
  });
  // Handle control data
  server.on("/control", HTTP_POST, []() {
    if (server.hasArg("commands")) {
      String commands = server.arg("commands");
      Serial.println("Received commands: " + commands);

      String translation = "<p>Command translation:</p><ul>";
      // Split commands by ';'
      int start = 0;
      int index = commands.indexOf(';', start);
      while (index != -1) {
        String command = commands.substring(start, index);
        command.trim();
        if (command.length() > 0) {
          translation += "<li>" + command + "</li>";
          executeCommand(command);
        }
        start = index + 1;
        index = commands.indexOf(';', start);
      }
      translation += "</ul>";

      server.send(200, "text/html", translation);
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing commands\"}");
    }
  });
  server.on("/sensor_data", HTTP_GET, handleSensorData);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
