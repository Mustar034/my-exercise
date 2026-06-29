#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi配置区（修改为你的2.4G WiFi） ==========
const char* WIFI_SSID = "WiFi";
const char* WIFI_PASS = "123456789";

// 硬件定义
#define TOUCH_PIN 4    // T4触摸引脚
WebServer server(80);

// 存储实时触摸采样值
int touchRawVal = 0;

// ========== 仪表盘HTML页面（AJAX自动拉取数据） ==========
const String dashHtml = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 触摸传感器实时仪表盘</title>
    <style>
        * {margin: 0; padding: 0; box-sizing: border-box; font-family: "Microsoft YaHei", sans-serif;}
        body {
            background-color: #0f141c;
            color: #ffffff;
            text-align: center;
            padding-top: 100px;
        }
        .dashboard-box {
            width: 90%;
            max-width: 600px;
            margin: 0 auto;
            border: 2px solid #00ccff;
            border-radius: 16px;
            padding: 40px 20px;
            background: #151c29;
        }
        h1 {
            color: #00ccff;
            margin-bottom: 35px;
            font-size: 28px;
        }
        .data-label {
            font-size: 20px;
            color: #aaa;
            margin-bottom: 12px;
        }
        #sensorValue {
            font-size: 72px;
            font-weight: bold;
            color: #00ff99;
            letter-spacing: 4px;
        }
        .tip {
            margin-top: 40px;
            color: #888;
            font-size: 16px;
        }
    </style>
</head>
<body>
    <div class="dashboard-box">
        <h1>触摸传感器实时监测仪表盘</h1>
        <div class="data-label">当前触摸原始数值</div>
        <div id="sensorValue">--</div>
        <div class="tip">提示：手指靠近GPIO4(T4)，数值会实时减小；松开后数值回升</div>
    </div>

    <script>
        // 获取数字显示容器
        const valDom = document.getElementById("sensorValue");

        // 异步拉取传感器数据（AJAX fetch）
        function refreshSensorData() {
            fetch("/data")
                .then(res => res.text())
                .then(num => {
                    valDom.innerText = num;
                })
                .catch(err => {
                    valDom.innerText = "通信断开";
                })
        }

        // 每100毫秒自动拉取一次数据，实现实时刷新
        setInterval(refreshSensorData, 100);
        // 页面加载立刻执行一次
        refreshSensorData();
    </script>
</body>
</html>
)HTML";

// ========== Web路由处理函数 ==========
// 1. 首页：返回仪表盘页面
void handleIndex() {
  server.send(200, "text/html", dashHtml);
}

// 2. 数据接口：返回实时触摸数值（供前端AJAX拉取）
void handleGetData() {
  server.send(200, "text/plain", String(touchRawVal));
}

// ========== WiFi连接初始化 ==========
void initWiFi() {
  Serial.print("正在连接WiFi：");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功！");
  Serial.print("仪表盘访问地址：");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  // 绑定网页路由
  server.on("/", handleIndex);
  server.on("/data", handleGetData);
  server.begin();
  Serial.println("实时传感器Web仪表盘服务启动完成");
}

void loop() {
  // 持续处理网页请求
  server.handleClient();

  // 循环采集触摸原始模拟量
  touchRawVal = touchRead(TOUCH_PIN);
  delay(100);
}