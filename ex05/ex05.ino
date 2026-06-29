// ========== 硬件与参数定义 ==========
#define TOUCH_PIN     4     // 触摸引脚 T4
#define LED_PIN       2     // ESP32板载LED
#define TOUCH_THRESH  20    // 触摸阈值，可微调
#define DEBOUNCE_MS   300   // 触摸防抖时间，防止手抖误触

// PWM呼吸灯基础参数（来自实验3）
const int pwmFreq = 5000;
const int pwmRes  = 8; // 0~255亮度范围

// 档位控制变量
int speedLevel = 1;    // 初始档位1（慢速）
const int maxLevel = 3;// 总共有3个速度档位
unsigned long lastTouchTime = 0; // 触摸防抖计时

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
    Serial.print("切换至档位：");
    Serial.println(speedLevel);
  }
}

// ========== 初始化 ==========
void setup() {
  Serial.begin(115200);
  // PWM绑定LED引脚（实验3标准用法）
  ledcAttach(LED_PIN, pwmFreq, pwmRes);
  // 绑定触摸中断
  touchAttachInterrupt(TOUCH_PIN, touchTrigger, TOUCH_THRESH);
  Serial.println("多档位触摸调速呼吸灯初始化完成");
  Serial.println("档位1=慢速 | 档位2=中速 | 档位3=快速");
}

// ========== 主循环：持续呼吸渐变 ==========
void loop() {
  // 根据当前档位设置渐变步长，步长越大呼吸越快
  int step;
  switch(speedLevel) {
    case 1: step = 1; break; // 慢
    case 2: step = 3; break; // 中
    case 3: step = 6; break; // 快
    default: step = 1;
  }

  // 渐变变亮
  for(int duty = 0; duty <= 255; duty += step) {
    ledcWrite(LED_PIN, duty);
    delay(10);
  }
  // 渐变变暗
  for(int duty = 255; duty >= 0; duty -= step) {
    ledcWrite(LED_PIN, duty);
    delay(10);
  }
}