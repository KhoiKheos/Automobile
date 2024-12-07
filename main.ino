#include <Servo.h>
#define PIN_DO 2

#define HOLES_DISC 20  // Số lỗ trên đĩa encoder
#define WHEEL_CIRCUMFERENCE 20.42  // Chu vi bánh xe (cm)
#define TARGET_DISTANCE 120  // Khoảng cách cần di chuyển (cm)
#define TARGET_PULSES ((TARGET_DISTANCE / 20) * 60)  // Số xung cần thiết 60.
Servo servo;

volatile unsigned int pulsesLeft = 0;   // Số xung của bánh trái
volatile unsigned int pulsesRight = 0;  // Số xung của bánh phải
float rpm;
unsigned long timeOld;

const int trigPin = 13;
const int echoPin = 12;
const int servoPin = 11;

const int enAPin = 6;
const int in1Pin = 7;
const int in2Pin = 5;

const int enBPin = 9;
const int in3Pin = 4;
const int in4Pin = 10;

const int encoderPinLeft = 3;
const int encoderPinRight = 2;

const int soundSensorPin = 8; // Cảm biến âm thanh

volatile long encoderCountLeft = 0;
volatile long encoderCountRight = 0;

const float wheelDiameter = 65.0;
const int pulsesPerRevolution = 20;
const float wheelCircumference = 3.1416 * wheelDiameter;

// Quãng đường cần đi trong chế độ Encoder Mode (10,000 mm)
//const float encoderModeDistance = 10000.0;
//long encoderModePulses = (encoderModeDistance / wheelCircumference) * pulsesPerRevolution;

#define NUM_ANGLES 7
unsigned char sensorAngle[NUM_ANGLES] = {50 ,60, 70, 80, 90, 100, 110};
unsigned int distance[NUM_ANGLES];

enum Motor { LEFT, RIGHT };

bool encoderMode = false; // Trạng thái chế độ Encoder Mode

void go(Motor m, int speed) {
    digitalWrite(m == LEFT ? in1Pin : in3Pin, speed > 0 ? HIGH : LOW);
    digitalWrite(m == LEFT ? in2Pin : in4Pin, speed <= 0 ? HIGH : LOW);
    analogWrite(m == LEFT ? enAPin : enBPin, abs(speed));
}

void goBoth(int speedLeft, int speedRight) {
    go(LEFT, speedLeft);
    go(RIGHT, speedRight);
}

void countEncoderLeft() {
    encoderCountLeft++;
    pulsesLeft++;
}
void countEncoderRight() {
    encoderCountRight++;
    pulsesRight++;
}

unsigned int readDistance() {
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    unsigned long duration = pulseIn(echoPin, HIGH);
    return duration * 343 / 2000;
}

void readNextDistance() {
    static unsigned char angleIndex = 0;
    static signed char step = 1;

    distance[angleIndex] = readDistance();
    angleIndex += step;
    if (angleIndex == NUM_ANGLES - 1) step = -1;
    else if (angleIndex == 0) step = 1;

    servo.write(sensorAngle[angleIndex]);
}

void toggleEncoderMode() {
    encoderMode = !encoderMode; // Chuyển đổi trạng thái
    if (encoderMode) {
        encoderCountLeft = 0;  // Reset encoder khi vào chế độ
        encoderCountRight = 0;
        Serial.println("Chuyển sang chế độ Encoder Mode.");
    } else {
        Serial.println("Thoát chế độ Encoder Mode.");
    }
}

void setup() {
    Serial.begin(9600);

    pinMode(encoderPinLeft, INPUT_PULLUP);
    pinMode(encoderPinRight, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderPinLeft), countEncoderLeft, RISING);
    attachInterrupt(digitalPinToInterrupt(encoderPinRight), countEncoderRight, RISING);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);

    pinMode(enAPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enBPin, OUTPUT);
    pinMode(in3Pin, OUTPUT);
    pinMode(in4Pin, OUTPUT);

    pinMode(soundSensorPin, INPUT);
    servo.attach(servoPin);servo.write(90);

    goBoth(0, 0);

    Serial.print("Brumm brumm !!");
    //Serial.println(encoderModePulses);

    pulsesLeft = 0;
    pulsesRight = 0;
}

unsigned long previousMillis = 0;
const long interval = 50;

void loop() {
    // Kiểm tra tín hiệu từ cảm biến âm thanh để chuyển đổi chế độ
    if (digitalRead(soundSensorPin) == HIGH) {
        delay(50); // Debounce
        if (digitalRead(soundSensorPin) == HIGH) {
            // Dừng xe trong 1 giây trước khi chuyển sang Encoder Mode
            goBoth(0, 0); // Dừng xe
            Serial.println("Dừng 1 giây trước khi chuyển sang Encoder Mode...");
            delay(1000); // Dừng 1 giây

            toggleEncoderMode(); // Chuyển sang Encoder Mode
            delay(500); // Ngăn kích hoạt liên tục
        }
    }

    if (encoderMode) {
        pulsesLeft = 0;
        pulsesRight = 0;
          Serial.println(pulsesLeft);
          Serial.println(pulsesRight);
        // Chế độ Encoder Mode
            if (pulsesLeft < TARGET_PULSES && pulsesRight < TARGET_PULSES) {
            // Cấu hình động cơ trái quay thuận
            digitalWrite(in1Pin, HIGH);
            digitalWrite(in2Pin, LOW);
            analogWrite(enAPin, 255);  // Động cơ trái quay với tốc độ 100%

            // Cấu hình động cơ phải quay thuận
            digitalWrite(in3Pin, HIGH);
            digitalWrite(in4Pin, LOW);
            analogWrite(enBPin, 255);  // Động cơ phải quay với tốc độ 100%
            } 
     
            else {
                // Dừng động cơ khi đạt đủ xung
                analogWrite(enAPin, 0);
                analogWrite(enBPin, 0);
                Serial.println("Đã di chuyển đủ khoảng cách !");
                Serial.println(encoderCountRight);
                Serial.println(encoderCountLeft);
                // while(1);  // Dừng chương trình sau khi đạt yêu cầu
                delay(50);
                goBoth(-170, -170); // Lùi
                // reverseSpeed = min(reverseSpeed + 10, 255); // Tăng tốc mỗi chu kỳ
                delay(50); // Cho phép tốc độ tăng dần mượt mà
                goBoth(0, 0); // Dừng lại sau khi lùi 200 xung
                Serial.println("Dừng và thoát chế độ Encoder");
                toggleEncoderMode(); // Thoát chế độ Encoder Mode
              
            }

    }
    else {
        // Chế độ chạy bình thường (tránh vật cản)
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            readNextDistance();

            bool tooClose = false;
            for (unsigned char i = 0; i < NUM_ANGLES; i++) {
                if (distance[i] < 300) {
                    tooClose = true;
                    break;
                }
            }

            if (tooClose) {
                goBoth(-170, -170); // Lùi lại một chút
                delay(50);
                
            } else {
                goBoth(255, 255); // Chạy bình thường
            }
        }
    }
}
