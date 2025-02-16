#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Servo.h>

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define FIREBASE_HOST "your_firebase_project_url"
#define FIREBASE_AUTH "your_firebase_auth_token"

#define IR_SENSOR_PIN D5   
#define SERVO_PIN D4       

Servo myServo;
bool rampUp = false;  
bool manualControl = false;  

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
    Serial.begin(9600);

   
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Povezivanje na WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nPovezano! IP Adresa: " + WiFi.localIP().toString());

    
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println(Firebase.ready() ? "🔥 Firebase povezan!" : "❌ Firebase GREŠKA!");

    
    pinMode(IR_SENSOR_PIN, INPUT);
    myServo.attach(SERVO_PIN);
    myServo.write(0);  

    Serial.println("✅ Sistem spreman...");
}

void loop() {
    int sensorState = digitalRead(IR_SENSOR_PIN);

    
    bool rampCommand = false;
    if (Firebase.getBool(firebaseData, "/rampa/status")) {
        rampCommand = firebaseData.boolData();
    }

    if (rampCommand && !rampUp) {
        Serial.println("🚗 Rampa podignuta manualno.");
        myServo.write(90);  
        rampUp = true;
        manualControl = true;  
    } 
    else if (!rampCommand && rampUp) {
        Serial.println("⬇ Rampa spuštena manualno.");
        myServo.write(0);  
        rampUp = false;
        manualControl = false;  
    }

    
    if (!manualControl) {
        if (sensorState == LOW && !rampUp) {  
            Serial.println("🚗 Vozilo detektovano - Podizanje rampe...");
            myServo.write(90);
            rampUp = true;
            Firebase.setBool(firebaseData, "/rampa/status", true);
        }

        if (sensorState == HIGH && rampUp) {  
            delay(2000);
            Serial.println("⬇ Vozilo prošlo - Spuštanje rampe...");
            myServo.write(0);
            rampUp = false;
            Firebase.setBool(firebaseData, "/rampa/status", false);
        }
    }

    delay(100);
}




