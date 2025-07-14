
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

//Replace with your network credentials
const char* ssid = "";
const char* password = "";
IPAddress local_IP(192, 168, 0, 80);  
IPAddress gateway(192, 168, 0, 1);     
IPAddress subnet(255, 255, 255, 0);   
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

WebServer server(80);

short switchStatus = 0;

#define LIGHT_PIN 4

void handleStream() {
  WiFiClient client = server.client();

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Błąd pobierania obrazu");
      continue;
    }

    client.printf("--frame\r\n");
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %d\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    // krótkie opóźnienie, żeby nie zabić ESP32
    delay(50);
  }
}
void switchLight(){
  if(!switchStatus){
    digitalWrite(LIGHT_PIN, HIGH);
    switchStatus = 1;
    Serial.println("Światło: WŁĄCZONE");
  }else{
    digitalWrite(LIGHT_PIN, LOW);
    switchStatus = 0;
    Serial.println("Światło: WYŁĄCZONE");
  }
}




void setup() {
 
 
  Serial.begin(115200);
  pinMode(LIGHT_PIN,OUTPUT);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
    config.frame_size = FRAMESIZE_QVGA; // 352x288

    config.jpeg_quality = 30;
    config.fb_count = 4;
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Błąd konfiguracji statycznego IP");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print(WiFi.localIP());
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/light", HTTP_GET, switchLight);
  server.begin();

}

void loop() {
  server.handleClient();
}
