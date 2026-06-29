// 硬件引脚定义
const int ledA = 2;   // LED A引脚
const int ledB = 4;   // LED B引脚

// PWM基础参数（和实验3保持一致）
const int pwmFreq = 5000;
const int pwmRes = 8; // 8位分辨率 0~255

void setup() {
  Serial.begin(115200);
  // 双通道分别绑定PWM，相互独立
  ledcAttach(ledA, pwmFreq, pwmRes);
  ledcAttach(ledB, pwmFreq, pwmRes);
  Serial.println("双通道反相渐变警车灯初始化完成");
}

void loop() {
  // 阶段1：A渐亮(0→255)，B同步渐暗(255→0)
  for(int dutyA = 0; dutyA <= 255; dutyA++){
    int dutyB = 255 - dutyA; // 反相关系
    ledcWrite(ledA, dutyA);
    ledcWrite(ledB, dutyB);
    delay(10);
  }

  // 阶段2：A渐暗(255→0)，B同步渐亮(0→255)
  for(int dutyA = 255; dutyA >= 0; dutyA--){
    int dutyB = 255 - dutyA;
    ledcWrite(ledA, dutyA);
    ledcWrite(ledB, dutyB);
    delay(10);
  }
  Serial.println("一组交替渐变完成");
}