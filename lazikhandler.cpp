#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ESP32Servo.h>
const char index_html_part1[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Robot Control</title>
    <style>
        body {
            font-family: monospace;
            margin: 0;
            padding: 0;
            background-color: #343434;
            color: #ffffff;
        }
        h1 {
            text-align: center;
            color: #66fcf1;
        }
        .controls {
            display: flex;
            justify-content: space-between;
            margin: 20px;
        }
        .camera,
        .command-mode,
        .arrow-mode {
            width: 45%;
            border: 1px solid #ccc;
            padding: 10px;
            text-align: center;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        textarea {
            font-family: monospace;
            width: 90%;
            border-radius: 5px;
            height: 100px;
            font-size: 16px;
            color: #ffffff;
            background-color: #36454f;
            border: none;
            padding: 10px;
            box-shadow: 0 0 10px #000;
            margin: 10px 0;
        }
        button {
            width: 200px;
            padding: 10px;
            font-size: 18px;
            background-color: #45a29e;
            color: #0b0c10;
            border: 2px solid #1f2833;
            cursor: pointer;
            transition-duration: 0.4s;
            border-radius: 5px;
            margin: 10px 0;
        }
        button:hover {
            background-color: #1f2833;
            color: #c5c6c7;
        }
        .arrow-buttons {
            display: grid;
            grid-template-columns: repeat(3, 60px);
            grid-template-rows: repeat(3, 60px);
            gap: 5px;
            justify-items: center;
            align-items: center;
            height: 200px;
        }
        .arrow-buttons button {
            font-size: 24px;
            background-color: #45a29e;
            border: none;
            color: #0b0c10;
            cursor: pointer;
            border-radius: 5px;
            width: 60px;
            height: 60px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .arrow-buttons button:hover {
            background-color: #1f2833;
            color: #c5c6c7;
        }
        .arrow-buttons .up {
            grid-column: 2;
            grid-row: 1;
        }
        .arrow-buttons .left {
            grid-column: 1;
            grid-row: 2;
        }
        .arrow-buttons .right {
            grid-column: 3;
            grid-row: 2;
        }
        .arrow-buttons .down {
            grid-column: 2;
            grid-row: 2;
        }
        @media (max-width: 768px) {
            .controls {
                flex-direction: column;
                align-items: center;
            }
            .command-mode,
            .arrow-mode {
                width: 90%;
                margin-bottom: 20px;
            }
            .arrow-buttons button {
                font-size: 18px;
                width: 50px;
                height: 50px;
            }
        }
    </style>
</head>
<body>
    <h1>Robot Control</h1>
    <div class="controls">
        <div class="camera">
            <h2>Camera</h2>
            <img id="cam" src="http://your-esp32-cam-ip:81/stream" style="width: 100%; max-width: 480px; border-radius: 10px" />
        </div>
        <div class="command-mode">
            <h2>Sensor Data</h2>
            <p>Temperature: <span id="temp">Loading...</span></p>
            <p>Humidity: <span id="humidity">Loading...</span></p>
        </div>
    </div>
)rawliteral";

const char index_html_part2[] PROGMEM = R"rawliteral(
    <div class="controls">
        <div class="command-mode">
            <h2>Command Mode</h2>
            <textarea id="commands" spellcheck="false" placeholder="Enter commands here..."></textarea>
            <button onclick="sendCommands()">Send Commands</button>
            <div id="response"></div>
        </div>
        <div class="arrow-mode">
            <h2>Wheels Control</h2>
            <div class="arrow-buttons">
                <button class="up" onmousedown="startMove('fwd')" onmouseup="stopMove()">W</button>
                <button class="left" onmousedown="startMove('left')" onmouseup="stopMove()">A</button>
                <button class="right" onmousedown="startMove('right')" onmouseup="stopMove()">D</button>
                <button class="down" onmousedown="startMove('back')" onmouseup="stopMove()">S</button>
            </div>
        </div>
        <div class="arrow-mode">
            <h2>Camera Controls</h2>
            <div class="arrow-buttons">
                <button class="up" onclick="moveServo('up')">↑</button>
                <button class="left" onclick="moveServo('left')">←</button>
                <button class="right" onclick="moveServo('right')">→</button>
                <button class="down" onclick="moveServo('down')">↓</button>
            </div>
        </div>
    </div>
)rawliteral";
const char index_html_part3[] PROGMEM = R"rawliteral(
    <script>
        function sendCommands() {
            var xhr = new XMLHttpRequest();
            var commands = document.getElementById("commands").value;
            xhr.open("POST", "/control", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    document.getElementById("response").innerHTML = xhr.responseText;
                }
            };
            xhr.send("commands=" + encodeURIComponent(commands));
        }
        
        function updateSensorData() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/sensor_data", true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4 && xhr.status == 200) {
                    var data = JSON.parse(xhr.responseText);
                    document.getElementById("temp").textContent = data.temperature + " °C";
                    document.getElementById("humidity").textContent = data.humidity + " %";
                }
            };
            xhr.send();
        }

        setInterval(updateSensorData, 2000);

        function startMove(direction) {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/move_" + direction, true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send();
        }

        function stopMove() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/stop", true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send();
        }

        function moveServo(dir) {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/servo_" + dir, true);
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.send();
        }

        let keys = { w: "fwd", a: "left", s: "back", d: "right" };
        let activeKey = null;

        document.addEventListener("keydown", function (e) {
            let key = e.key.toLowerCase();
            if (keys[key] && activeKey !== key) {
                activeKey = key;
                startMove(keys[key]);
            }
        });

        document.addEventListener("keyup", function (e) {
            let key = e.key.toLowerCase();
            if (keys[key]) {
                activeKey = null;
                stopMove();
            }
        });
    </script>
</body>
</html>
)rawliteral";




// Dane sieci WiFi
const char* wifi_ssid = "";
const char* wifi_password = "";
// Czujnik DHT
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


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


Servo servoX;
Servo servoY;
int posX = 90;
int posY = 90;
#define SERVO_X_PIN 18
#define SERVO_Y_PIN 33
// Obsługa czujników
void handleSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    server.send(500, "application/json", "{\"error\":\"Failed to read from DHT sensor!\"}");
    return;
  }


String jsonResponse = "{";
jsonResponse += "\"temperature\": " + String(temperature) + ",";
jsonResponse += "\"humidity\": " + String(humidity);
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
void moveCamera(String direction) {
  if (direction == "up") posY = max(0, posY - 10);
  else if (direction == "down") posY = min(180, posY + 10);
  else if (direction == "right") posX = max(0, posX - 10);
  else if (direction == "left") posX = min(180, posX + 10);
  Serial.println(direction);
  Serial.println(posX);
  Serial.println(posY);

  servoX.write(posX);
  servoY.write(posY);
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
servoX.attach(SERVO_X_PIN);
servoY.attach(SERVO_Y_PIN);
servoX.write(posX);
servoY.write(posY);

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
    String html = String((const char*)index_html_part1) +
                String((const char*)index_html_part2) +
                String((const char*)index_html_part3);
    server.send(200, "text/html", html);
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
  server.on("/servo_up", HTTP_POST, []() { moveCamera("up"); server.send(200); });
  server.on("/servo_down", HTTP_POST, []() { moveCamera("down"); server.send(200); });
  server.on("/servo_left", HTTP_POST, []() { moveCamera("left"); server.send(200); });
  server.on("/servo_right", HTTP_POST, []() { moveCamera("right"); server.send(200); });
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
