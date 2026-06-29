#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置区（自行修改WiFi名称密码） ====================
const char* ssid     = "WiFi";
const char* password = "123456789";

// PWM硬件参数（和实验3保持一致）
const int ledPin = 2;
const int pwmFreq = 5000;
const int pwmRes  = 8; // 0~255亮度区间

// Web服务器端口80
WebServer server(80);
int brightness = 0; // 全局存储LED亮度

// ==================== 网页HTML页面（内置滑块+JS） ====================
const String htmlPage = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <title>ESP32 网页无极调光</title>
    <style>
        body { font-family: Arial; text-align: center; margin-top: 80px; background:#1a1a1a; color:#fff; }
        .container { width: 80%; max-width: 600px; margin: 0 auto; }
        input[type="range"] { width: 100%; height: 25px; margin: 30px 0; }
        #valShow { font-size: 32px; color: #0cf; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1>LED 无极亮度调节</h1>
        <p>当前亮度值：<span id="valShow">0</span></p>
        <!-- 滑动条：范围0~255 -->
        <input type="range" id="slider" min="0" max="255" value="0">
    </div>

    <script>
        // 获取滑块与数值显示框
        const slider = document.getElementById("slider");
        const valText = document.getElementById("valShow");

        // 监听滑块拖动事件
        slider.addEventListener("input", function(){
            let val = this.value;
            valText.innerText = val;
            // 发送GET请求到ESP32，传递亮度参数
            fetch(`/?val=${val}`)
                .catch(err => console.log("通信异常:", err));
        });
    </script>
</body>
</html>
)HTML";

// ==================== 网页首页路由 ====================
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// ==================== 亮度控制路由：解析URL参数val ====================
void handleBrightness() {
  // 判断URL是否携带val参数
  if (server.hasArg("val")) {
    String valStr = server.arg("val");
    int newVal = valStr.toInt();
    // 限制数值区间0~255，防止越界
    if(newVal >= 0 && newVal <= 255){
      brightness = newVal;
      ledcWrite(ledPin, brightness); // 更新PWM亮度
    }
  }
  // 返回页面，保持滑块状态
  server.send(200, "text/html", htmlPage);
}

// ==================== WiFi连接函数 ====================
void connectWiFi() {
  Serial.print("正在连接WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功！");
  Serial.print("网页控制地址：");
  Serial.println(WiFi.localIP()); // 打印ESP32局域网IP
}

void setup() {
  Serial.begin(115200);
  // PWM初始化（实验3标准接口）
  ledcAttach(ledPin, pwmFreq, pwmRes);
  ledcWrite(ledPin, 0); // 上电默认熄灭

  connectWiFi();

  // 绑定Web路由
  server.on("/", handleBrightness); // 根路径统一处理页面+亮度参数
  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  server.handleClient(); // 持续监听浏览器访问
}