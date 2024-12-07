#define PIN_DO 2
volatile unsigned int pulsesLeft = 0;   // Số xung của bánh trái
volatile unsigned int pulsesRight = 0;  // Số xung của bánh phải
float rpm;
unsigned long timeOld;
#define HOLES_DISC 20  // Số lỗ trên đĩa encoder
#define WHEEL_CIRCUMFERENCE 20.42  // Chu vi bánh xe (cm)
#define TARGET_DISTANCE 120  // Khoảng cách cần di chuyển (cm)
#define TARGET_PULSES ((TARGET_DISTANCE / 20) * 60)  // Số xung cần thiết 60.

const int enAPin = 6;    // Chân ENA để điều chỉnh tốc độ động cơ trái
const int in1Pin = 7;    // Chân IN1 để điều khiển hướng động cơ trái
const int in2Pin = 5;    // Chân IN2 để điều khiển hướng động cơ trái

const int enBPin = 9;    // Chân ENB để điều chỉnh tốc độ động cơ phải
const int in3Pin = 4;    // Chân IN3 để điều khiển hướng động cơ phải
const int in4Pin = 10;   // Chân IN4 để điều khiển hướng động cơ phải

const int encoderPinLeft = 3;  // Chân encoder động cơ trái
const int encoderPinRight = 2; // Chân encoder động cơ phải

void countEncoderLeft() {
    pulsesLeft++;  // Tăng giá trị mỗi khi có xung từ encoder trái
}

void countEncoderRight() {
    pulsesRight++; // Tăng giá trị mỗi khi có xung từ encoder phải
}

void setup() {
    // Thiết lập chân điều khiển động cơ
    pinMode(enAPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(enBPin, OUTPUT);
    pinMode(in3Pin, OUTPUT);
    pinMode(in4Pin, OUTPUT);

    // Thiết lập chân encoder
    pinMode(encoderPinLeft, INPUT_PULLUP);
    pinMode(encoderPinRight, INPUT_PULLUP);

    // Thiết lập interrupt để đếm xung từ encoder
    attachInterrupt(digitalPinToInterrupt(encoderPinLeft), countEncoderLeft, RISING);
    attachInterrupt(digitalPinToInterrupt(encoderPinRight), countEncoderRight, RISING);

    // Khởi động Serial Monitor
    Serial.begin(9600);
    Serial.println("Chuẩn bị chạy động cơ...");

    // Reset encoder
    pulsesLeft = 0;
    pulsesRight = 0;
}

void loop() {
    // Kiểm tra xem cả hai bánh xe đã di chuyển đủ khoảng cách chưa
    if (pulsesLeft < TARGET_PULSES && pulsesRight < TARGET_PULSES) {
        // Cấu hình động cơ trái quay thuận
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
        analogWrite(enAPin, 255);  // Động cơ trái quay với tốc độ 100%

        // Cấu hình động cơ phải quay thuận
        digitalWrite(in3Pin, HIGH);
        digitalWrite(in4Pin, LOW);
        analogWrite(enBPin, 255);  // Động cơ phải quay với tốc độ 100%
        
    } else {
        // Dừng động cơ khi đạt đủ xung
        analogWrite(enAPin, 0);
        analogWrite(enBPin, 0);
        Serial.println("Đã di chuyển đủ khoảng cách !");


        // while(1);  // Dừng chương trình sau khi đạt yêu cầu
    }
}
