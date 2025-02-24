#ifndef INDEX_H
#define INDEX_H

const char index_html[] = 
"<!DOCTYPE html>
"
"<html lang="en">
"
"<head>
"
"    <meta charset="UTF-8">
"
"    <meta name="viewport" content="width=device-width, initial-scale=1.0">
"
"    <title>Robot Control</title>
"
"    <style>
"
"        body {
"
"            font-family: monospace;
"
"            margin: 0;
"
"            padding: 0;
"
"            background-color: #343434;
"
"            color: #ffffff;
"
"        }
"
"        h1 {
"
"            text-align: center;
"
"            color: #66fcf1;
"
"        }
"
"        .controls {
"
"            display: flex;
"
"            justify-content: space-between;
"
"            margin: 20px;
"
"        }
"
"        .command-mode, .arrow-mode {
"
"            width: 45%;
"
"            border: 1px solid #ccc;
"
"            padding: 10px;
"
"            text-align: center;
"
"            display: flex;
"
"            flex-direction: column;
"
"            justify-content: center;
"
"            align-items: center;
"
"        }
"
"        textarea {
"
"            font-family: monospace;
"
"            width: 90%;
"
"            border-radius: 5px;
"
"            height: 100px;
"
"            font-size: 16px;
"
"            color: #ffffff;
"
"            background-color: #36454f;
"
"            border: none;
"
"            padding: 10px;
"
"            box-shadow: 0 0 10px #000;
"
"            margin: 10px 0;
"
"        }
"
"        button {
"
"            width: 200px;
"
"            padding: 10px;
"
"            font-size: 18px;
"
"            background-color: #45a29e;
"
"            color: #0b0c10;
"
"            border: 2px solid #1f2833;
"
"            cursor: pointer;
"
"            transition-duration: 0.4s;
"
"            border-radius: 5px;
"
"            margin: 10px 0;
"
"        }
"
"        button:hover {
"
"            background-color: #1f2833;
"
"            color: #c5c6c7;
"
"        }
"
"        .arrow-buttons {
"
"            display: grid;
"
"            grid-template-columns: repeat(3, 60px);
"
"            grid-template-rows: repeat(3, 60px);
"
"            gap: 5px;
"
"            justify-items: center;
"
"            align-items: center;
"
"            height: 200px;
"
"        }
"
"        .arrow-buttons button {
"
"            font-size: 24px;
"
"            background-color: #45a29e;
"
"            border: none;
"
"            color: #0b0c10;
"
"            cursor: pointer;
"
"            border-radius: 5px;
"
"            width: 60px;
"
"            height: 60px;
"
"            display: flex;
"
"            justify-content: center;
"
"            align-items: center;
"
"        }
"
"        .arrow-buttons button:hover {
"
"            background-color: #1f2833;
"
"            color: #c5c6c7;
"
"        }
"
"        .arrow-buttons .up { grid-column: 2; grid-row: 1; }
"
"        .arrow-buttons .left { grid-column: 1; grid-row: 2; }
"
"        .arrow-buttons .right { grid-column: 3; grid-row: 2; }
"
"        .arrow-buttons .down { grid-column: 2; grid-row: 2; }
"
"        @media (max-width: 768px) {
"
"            .controls {
"
"                flex-direction: column;
"
"                align-items: center;
"
"            }
"
"            .command-mode, .arrow-mode {
"
"                width: 90%;
"
"                margin-bottom: 20px;
"
"            }
"
"            .arrow-buttons button {
"
"                font-size: 18px;
"
"                width: 50px;
"
"                height: 50px;
"
"            }
"
"        }
"
"    </style>
"
"</head>
"
"<body>
"
"    <h1>Robot Control</h1>
"
"    <div class="controls">
"
"        <div class="command-mode">
"
"            <h2>Command Mode</h2>
"
"            <textarea id="commands" spellcheck="false" placeholder="Enter commands here..."></textarea>
"
"            <button onclick="sendCommands()">Send Commands</button>
"
"            <div id="response"></div>
"
"        </div>
"
"        <div class="arrow-mode">
"
"            <h2>Arrow Control</h2>
"
"            <div class="arrow-buttons">
"
"                <button class="up" onmousedown="startMove('fwd')" onmouseup="stopMove()">W</button>
"
"                <button class="left" onmousedown="startMove('left')" onmouseup="stopMove()">A</button>
"
"                <button class="right" onmousedown="startMove('right')" onmouseup="stopMove()">D</button>
"
"                <button class="down" onmousedown="startMove('back')" onmouseup="stopMove()">S</button>
"
"            </div>
"
"        </div>
"
"        <div class="command-mode">
"
"            <h2>Sensor Data</h2>
"
"            <p>Temperature: <span id="temp">Loading...</span></p>
"
"            <p>Humidity: <span id="humidity">Loading...</span></p>
"
"            <p>Light Intensity: <span id="lightIntensity">Loading...</span></p>
"
"        </div>
"
"    </div>
"
"    <script>
"
"        function sendCommands() {
"
"            var xhr = new XMLHttpRequest();
"
"            var commands = document.getElementById('commands').value;
"
"            xhr.open('POST', '/control', true);
"
"            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
"
"            xhr.onreadystatechange = function() {
"
"                if (xhr.readyState == 4 && xhr.status == 200) {
"
"                    document.getElementById('response').innerHTML = xhr.responseText;
"
"                }
"
"            };
"
"            xhr.send('commands=' + encodeURIComponent(commands));
"
"        }
"
"        function updateSensorData() {
"
"            var xhr = new XMLHttpRequest();
"
"            xhr.open('GET', '/sensor_data', true);
"
"            xhr.onreadystatechange = function () {
"
"                if (xhr.readyState == 4 && xhr.status == 200) {
"
"                    var data = JSON.parse(xhr.responseText);
"
"                    document.getElementById('temp').textContent = data.temperature + ' °C';
"
"                    document.getElementById('humidity').textContent = data.humidity + ' %';
"
"                    document.getElementById('lightIntensity').textContent = data.lightIntensity;
"
"                }
"
"            };
"
"            xhr.send();
"
"        }
"
"        setInterval(updateSensorData, 2000);
"
"        function startMove(direction) {
"
"            var xhr = new XMLHttpRequest();
"
"            xhr.open('POST', '/move_' + direction, true);
"
"            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
"
"            xhr.send();
"
"        }
"
"        function stopMove() {
"
"            var xhr = new XMLHttpRequest();
"
"            xhr.open('POST', '/stop', true);
"
"            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
"
"            xhr.send();
"
"        }
"
"    </script>
"
"</body>
"
"</html>
"
"";

#endif // INDEX_H
