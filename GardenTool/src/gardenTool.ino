/*
 * Project gardenTool
 * Description:
 * Author: Hyunrae Kim and Joel Ki
 * Date:
 */

// SOIL MOISTURE READINGS:
// Dry: 3650
// Wet:

#include <Adafruit_SSD1306.h>

int pirPin = D0;
int soundPin = D1;
int moisturePin = A0;
int lightPin = D3;

int soundTime = 1000;
int moistureUpdateTime = 5000;
int motionUpdateTime = 500;

bool motionDetected = false;
int soilMoisture = 0;
int temperature = 0;

int servoAngle = 0;

int naturalUpdateTime = 5000;

String topic = "cse222Garden/thisGardenTool";

Servo servo;

Timer soundTimer(soundTime, stopSound);

Timer moistureUpdateTimer(moistureUpdateTime, updateMoisture);

Timer motionTimer(motionUpdateTime, moveServo);

Timer activeMotionTimer(10000, [](){ activateSound(""); }, 1);

Timer naturalUpdateTimer(naturalUpdateTime, [](){ publishState(""); });

#define OLED_DC     D3
#define OLED_CS     D4
#define OLED_RESET  D5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
int  x, minX;

void moveServo() {
  if (servoAngle == 0) {
    servoAngle = 180;
    servo.write(180);
  } else {
    servoAngle = 0;
    servo.write(0);
  }
}

void stopSound() {
  digitalWrite(soundPin, LOW);
}

int activateSound(String arg) {
  digitalWrite(soundPin, HIGH);
  soundTimer.reset();
}

void updateMoisture() {
  soilMoisture = analogRead(moisturePin);
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  pinMode(pirPin, INPUT);
  pinMode(soundPin, OUTPUT);
  pinMode(moisturePin, INPUT);
  pinMode(lightPin, OUTPUT);

  servo.attach(D2);

  Particle.variable("motionDetected", motionDetected);
  Particle.variable("soilMoisture", soilMoisture);

  Particle.function("activateSound", activateSound);
  Particle.function("publishState", publishState);

  moistureUpdateTimer.start();

  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(7);
  display.setTextColor(WHITE);
  display.setTextWrap(false); // turn off text wrapping so we can do scrolling
  x = display.width(); // set scrolling frame to display width
  minX = -1200;

  naturalUpdateTimer.start();
}

void loop() {

  display.clearDisplay();
  display.setCursor(x/2, 7);
  display.print("SM: ");
  display.print(String(soilMoisture));
  display.display();
  if(--x < minX) x = display.width()*2;

  if (digitalRead(pirPin) == 0) {
    if (motionDetected == false) {
      motionTimer.reset();
      activeMotionTimer.reset();
    }
    motionDetected = true;
  } else {
    motionDetected = false;
    motionTimer.stop();
    activeMotionTimer.stop();
  }
}


int publishState(String arg) {
  String data = "{";

  if(motionDetected) {
    data += "\"motion\":true";
  } else {
    data += "\"motion\":false";
  }
  data += ", ";
  data += "\"moisture\":";
  data += soilMoisture;
  data += ", ";
  data += "\"temp\":";
  data += temperature;
  data += "}";

  Serial.println("Publishing:");
  Serial.println(data);

  Particle.publish(topic, data, 60, PRIVATE);
  return 0;
}
