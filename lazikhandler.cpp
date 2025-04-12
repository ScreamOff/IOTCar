#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// index_html jako string
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Control</title>
    <style>
        body { font-family: monospace; background-color: #343434; color: #fff; margin: 0; padding: 0; }
        h1 { text-align: center; color: #66fcf1; }
        .controls { display: flex; flex-wrap: wrap; justify-content: center; padding: 20px; gap: 20px; }
        .command-mode, .arrow-mode { background: #2d2d2d; padding: 20px; border-radius: 10px; width: 300px; }
        textarea { width: 100%; height: 100px; margin-top: 10px; background: #1f1f1f; color: #fff; border: none; padding: 10px; font-size: 16px; }
        button { margin-top: 10px; width: 100%; padding: 10px; background: #45a29e; border: none; font-size: 18px; cursor: pointer; color: #0b0c10; }
        button:hover { background: #1f2833; color: #c5c6c7; }
        .arrow-buttons { display: grid; grid-template: 60px 60px 60px / 60px 60px 60px; gap: 5px; justify-content: center; margin-top: 10px; }
        .arrow-buttons button { width: 60px; height: 60px; font-size: 24px; }
        .up { grid-column: 2; grid-row: 1; }
        .left { grid-column: 1; grid-row: 2; }
        .right { grid-column: 3; grid-row: 2; }
        .down { grid-column: 2; grid-row: 2; }
    </style>
</head>
<body>
    <h1>Robot Control</h1>
    <div class="controls">
        <div class="command-mode">
            <h2>Command Mode</h2>
            <textarea id="commands" placeholder="Enter commands here..."></textarea>
            <button onclick="sendCommands()">Send Commands</button>
            <div id="response"></div>
        </div>
        <div class="arrow-mode">
            <h2>Arrow Control</h2>
            <div class="arrow-buttons">
                <button class="up" onmousedown="startMove('fwd')" onmouseup="stopMove()">W</button>
                <button class="left" onmousedown="startMove('left')" onmouseup="stopMove()">A</button>
                <button class="right" onmousedown="startMove('right')" onmouseup="stopMove()">D</button>
                <button class="down" onmousedown="startMove('back')" onmouseup="stopMove()">S</button>
            </div>
        </div>
        <div class="command-mode">
            <h2>Sensor Data</h2>
            <p>Temperature: <span id="temp">Loading...</span></p>
            <p>Humidity: <span id="humidity">Loading...</span></p>
            <p><span id="gas">Loading...</span></p>
        </div>
    </div>
    <script>
        function sendCommands() {
            var xhr = new XMLHttpRequest();
            var commands = document.getElementById('commands').value;
            xhr.open('POST', '/control', true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.onreadystatechange = function() {
                if (xhr.readyState == 4 && xhr.status == 200)
                    document.getElementById('response').innerHTML = xhr.responseText;
            };
            xhr.send('commands=' + encodeURIComponent(commands));
        }

        function updateSensorData() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/sensor_data', true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    var data = JSON.parse(xhr.responseText);
                    document.getElementById('temp').textContent = data.temperature + ' °C';
                    document.getElementById('humidity').textContent = data.humidity + ' %';
                    document.getElementById('gas').textContent = data.gas;
                }
            };
            xhr.send();
        }
        setInterval(updateSensorData, 2000);

        function startMove(direction) {
            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/move_' + direction, true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.send();
        }

        function stopMove() {
            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/stop', true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.send();
        }
    </script>
</body>
</html>
)rawliteral";

// Dane sieci WiFi
const char* wifi_ssid = "SSIDWifi";
const char* wifi_password = "pass";

// Czujnik DHT
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Czujnik światła
#define MQT135_PIN 34

// Piny silników
#define FRONT_IN1 15
#define FRONT_IN2 4
#define FRONT_IN3 16
#define FRONT_IN4 17
#define REAR_IN1 27
#define REAR_IN2 14
#define REAR_IN3 12
#define REAR_IN4 26

WebServer server(80);
// Funkcja pobierająca poziom gazu z czujnika MQ-135
String getGasLevel() {
  int gasValue = analogRead(MQT135_PIN);  // Odczyt wartości z pin 34 (MQ-135)
  
  if (gasValue < 500) return "Very Low Gas Level";
  else if (gasValue < 1500) return "Low Gas Level";
  else if (gasValue < 2500) return "Moderate Gas Level";
  else if (gasValue < 3500) return "High Gas Level";
  else return "Very High Gas Level";
}
// Obsługa czujników
void handleSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  String gasLevel = getGasLevel();

  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read from DHT sensor!\"}");
    return;
  }


  String jsonResponse = "{";
  jsonResponse += "\"temperature\": " + String(temperature) + ",";
  jsonResponse += "\"humidity\": " + String(humidity) + ",";
  jsonResponse += "\"gas\": \"" + gasLevel + "\"";
  jsonResponse += "}";

  server.send(200, "application/json", jsonResponse);
}
bool moving = false;
String currentDirection = "";

void setPins(int pin1, int pin2, int pin3, int pin4, bool in1, bool in2, bool in3, bool in4) {
  digitalWrite(pin1, in1);
  digitalWrite(pin2, in2);
  digitalWrite(pin3, in3);
  digitalWrite(pin4, in4);
}

void forward() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, true, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, false, true);
}

void back() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, false, true);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, true, false);
}

void left() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, true, false, true, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, true, false, true, false);
}

void right() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, true, false, true);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, true, false, true);
}

void stopMotors() {
  setPins(FRONT_IN1, FRONT_IN2, FRONT_IN3, FRONT_IN4, false, false, false, false);
  setPins(REAR_IN1, REAR_IN2, REAR_IN3, REAR_IN4, false, false, false, false);
}

void executeCommand(String command) {
  if (command.startsWith("fwd")) {
    int duration = command.substring(3).toInt();
    forward();
    delay(duration);
  } else if (command.startsWith("back")) {
    int duration = command.substring(4).toInt();
    back();
    delay(duration);
  } else if (command.startsWith("left")) {
    int duration = command.substring(4).toInt();
    left();
    delay(duration);
  } else if (command.startsWith("right")) {
    int duration = command.substring(5).toInt();
    right();
    delay(duration);
  }
  stopMotors();
}

void setup() {
  dht.begin();
  pinMode(FRONT_IN1, OUTPUT);
  pinMode(FRONT_IN2, OUTPUT);
  pinMode(FRONT_IN3, OUTPUT);
  pinMode(FRONT_IN4, OUTPUT);
  pinMode(REAR_IN1, OUTPUT);
  pinMode(REAR_IN2, OUTPUT);
  pinMode(REAR_IN3, OUTPUT);
  pinMode(REAR_IN4, OUTPUT);

  stopMotors();

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

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

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

  server.on("/stop", HTTP_POST, []() {
    stopMotors();
    server.send(200, "text/plain", "Motors Stopped");
  });

  server.on("/status", HTTP_GET, []() {
    String status = moving ? "<p>Robot is moving " + currentDirection + "</p>" : "<p>Robot is stopped</p>";
    server.send(200, "text/html", status);
  });

  server.on("/control", HTTP_POST, []() {
    if (server.hasArg("commands")) {
      String commands = server.arg("commands");
      Serial.println("Received commands: " + commands);

      String translation = "<p>Command translation:</p><ul>";
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
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
