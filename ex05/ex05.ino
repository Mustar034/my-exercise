// ========== 硬件与参数定义 ==========
#define TOUCH_PIN     4     // 触摸引脚 T0(GPIO4)
#define LED_PIN       2     // ESP32板载LED
#define TOUCH_THRESH  500    // 触摸阈值，可微调
#define DEBOUNCE_MS   300   // 触摸防抖时间，防止手抖误触

// PWM呼吸灯基础参数
const int pwmFreq = 5000;
const int pwmRes  = 8;      // 8位分辨率 亮度0~255

// 档位间隔：数值越小呼吸速度越快（对应3档：慢/中/快）
const int speedIntervals[3] = {15, 7, 3};
int speedLevel = 1;         // 初始档位1（慢速）
const int maxLevel = 3;     // 总共有3个速度档位
unsigned long lastTouchTime = 0; // 触摸防抖计时

// 呼吸灯非阻塞变量
int dutyCycle = 0;
int direction = 1;          // 1增亮 -1变暗
unsigned long previousMillis = 0;

// ========== 触摸中断回调函数 ==========
void touchTrigger() {
  unsigned long now = millis();
  // 防抖判断：间隔大于300ms才判定有效触摸
  if (now - lastTouchTime > DEBOUNCE_MS) {
    lastTouchTime = now;
    // 档位循环切换 1→2→3→1
    speedLevel += 1;
    if (speedLevel > maxLevel) {
      speedLevel = 1;
    }
    // 中断内禁止大量串口输出，标记标志放loop打印
  }
}

// ========== 初始化 ==========
void setup() {
  Serial.begin(115200);
  // PWM绑定LED引脚
  ledcAttach(LED_PIN, pwmFreq, pwmRes);
  ledcWrite(LED_PIN, 0);
  // 绑定触摸中断，低于阈值触发中断
  touchAttachInterrupt(TOUCH_PIN, touchTrigger, TOUCH_THRESH);
  
  Serial.println("===== 多档位触摸调速呼吸灯初始化完成 =====");
  Serial.println("档位1=慢速 | 档位2=中速 | 档位3=快速");
  Serial.print("当前档位：");
  Serial.println(speedLevel);
}

// ========== 主循环：非阻塞持续呼吸渐变 ==========
void loop() {
  // 读取当前档位对应的刷新间隔
  int interval = speedIntervals[speedLevel - 1];
  unsigned long currentMillis = millis();

  // 非阻塞PWM亮度更新
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    dutyCycle += direction;

    // 亮度边界翻转渐变方向
    if (dutyCycle >= 255) {
      dutyCycle = 255;
      direction = -1;
    } else if (dutyCycle <= 0) {
      dutyCycle = 0;
      direction = 1;
    }
    ledcWrite(LED_PIN, dutyCycle);
  }

  // 检测档位是否变更，打印串口信息
  static int lastPrintLevel = speedLevel;
  if (speedLevel != lastPrintLevel) {
    lastPrintLevel = speedLevel;
    Serial.print("触摸触发，切换至档位：");
    Serial.println(speedLevel);
  }
}