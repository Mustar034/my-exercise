// 定义LED引脚（ESP32通常板载LED为GPIO2）
const int ledPin = 2;

// ====== 摩尔斯时间单位（可调整）======
int dotTime = 200;        // 点（短闪）
int dashTime = dotTime * 3;  // 划（长闪）
int symbolGap = dotTime;     // 字符内间隔
int letterGap = dotTime * 3; // 字母间隔
int wordGap = dotTime * 7;   // 单词间隔

// ====== 点 ======
void dot() {
  digitalWrite(ledPin, HIGH);
  delay(dotTime);
  digitalWrite(ledPin, LOW);
  delay(symbolGap);
}

// ====== 划 ======
void dash() {
  digitalWrite(ledPin, HIGH);
  delay(dashTime);
  digitalWrite(ledPin, LOW);
  delay(symbolGap);
}

// ====== S ====== (···)
void sendS() {
  dot(); dot(); dot();
}

// ====== O ====== (———)
void sendO() {
  dash(); dash(); dash();
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("SOS Morse LED Started");
}

void loop() {

  // S
  sendS();
  delay(letterGap);

  // O
  sendO();
  delay(letterGap);

  // S
  sendS();

  // 单词间隔（SOS 完整一组结束）
  delay(wordGap);

  Serial.println("Sent SOS");
}