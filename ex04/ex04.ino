// 硬件引脚定义
#define TOUCH_PIN     4    // 触摸引脚T4
#define LED_PIN       2    // ESP32板载LED引脚
#define TOUCH_THRESH  400   // 触摸阈值，可根据开发板微调
#define DEBOUNCE_DELAY 300 // 软件防抖300ms，防止手抖多次触发

bool ledState = false;       // LED自锁状态变量
unsigned long lastTouchTime = 0; // 记录上一次有效触摸时间，用于防抖

// 触摸中断服务函数
void gotTouch() {
  unsigned long now = millis();
  // 防抖判断：距离上次有效触摸超过防抖间隔才执行
  if (now - lastTouchTime > DEBOUNCE_DELAY) {
    lastTouchTime = now;
    ledState = !ledState;    // 翻转LED状态
    digitalWrite(LED_PIN, ledState);
    Serial.print("LED状态切换: ");
    Serial.println(ledState ? "亮" : "灭");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 上电默认熄灭LED

  // 绑定触摸中断：触摸引脚、中断回调函数、触摸阈值
  touchAttachInterrupt(TOUCH_PIN, gotTouch, TOUCH_THRESH);
  Serial.println("触摸自锁开关初始化完成，触摸T4引脚切换LED");
}

void loop() {
  // 主循环无业务逻辑，仅延时喂狗，防止部分内核看门狗复位
  delay(100);
}