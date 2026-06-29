#include <WiFi.h>
#include <WebServer.h>

// ===================== WiFi配置区（修改为你的2.4G WiFi） =====================
const char* WIFI_SSID = "WiFi";
const char* WIFI_PWD  = "123456789";

// 硬件引脚定义
#define ALARM_LED_PIN   2
#define TOUCH_SENSE_PIN 4
#define TOUCH_THRESHOLD 20
#define DEBOUNCE_TIME   300

// Web服务器
WebServer server(80);

// 系统全局状态变量
bool isArm = false;     // false=撤防  true=布防
bool isAlarm = false;   // false=无报警 true=报警锁定
unsigned long lastTouchCheck = 0;

// ===================== 网页HTML页面（双按钮+状态显示） =====================
const String htmlWebPage = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 安防报警主机</title>
    <style>
        *{box-sizing: border-box; margin:0; padding:0; font-family: "微软雅黑";}
        body{background:#121212; color:#fff; text-align:center; padding-top:60px;}
        .box{width:90%; max-width:550px; margin:0 auto; padding:30px; border:1px solid #444; border-radius:12px;}
        h1{margin-bottom:30px; color:#0cf;}
        .state{font-size:22px; margin:20px 0; padding:15px; border-radius:8px;}
        .disarm-state{background:#222; border:1px solid #666;}
        .arm-state{background:#003322; border:1px solid #0c8;}
        .alarm-state{background:#440000; border:1px solid #f33;}
        button{width:42%; height:60px; font-size:20px; margin:10px 2%; border:none; border-radius:8px; cursor:pointer;}
        #armBtn{background:#0a6; color:#fff;}
        #disarmBtn{background:#d22; color:#fff;}
    </style>
</head>
<body>
    <div class="box">
        <h1>物联网安防报警主机</h1>
        <div class="state" id="statusBox">加载中...</div>
        <br>
        <button id="armBtn">布防 ARM</button>
        <button id="disarmBtn">撤防 DISARM</button>
    </div>

    <script>
        const statusBox = document.getElementById("statusBox");
        const armBtn = document.getElementById("armBtn");
        const disarmBtn = document.getElementById("disarmBtn");

        // 更新页面状态文字与样式
        function refreshStatus(){
            fetch("/getstate")
            .then(res=>res.text())
            .then(data=>{
                let arr = data.split("|");
                let armStat = arr[0];
                let alarmStat = arr[1];
                if(alarmStat === "1"){
                    statusBox.className = "state alarm-state";
                    statusBox.innerText = "⚠️ 警报触发！设备锁定报警，请撤防复位";
                }else if(armStat === "1"){
                    statusBox.className = "state arm-state";
                    statusBox.innerText = "✅ 系统已布防，触碰传感器将触发报警";
                }else{
                    statusBox.className = "state disarm-state";
                    statusBox.innerText = "🔒 系统已撤防，触摸无报警";
                }
            })
        }

        // 布防按钮
        armBtn.onclick = ()=>{
            fetch("/arm");
            setTimeout(refreshStatus,100);
        }
        // 撤防按钮（同时清除报警）
        disarmBtn.onclick = ()=>{
            fetch("/disarm");
            setTimeout(refreshStatus,100);
        }
        // 页面加载自动刷新状态
        refreshStatus();
        setInterval(refreshStatus,1000);
    </script>
</body>
</html>
)HTML";

// ===================== 网页路由处理函数 =====================
// 首页：展示控制面板
void handleRoot(){
  server.send(200, "text/html", htmlWebPage);
}

// 布防接口
void handleArm(){
  isArm = true;
  server.send(200, "text/plain", "armed");
}

// 撤防接口：清除布防+清除报警+熄灭LED
void handleDisarm(){
  isArm = false;
  isAlarm = false;
  digitalWrite(ALARM_LED_PIN, LOW);
  server.send(200, "text/plain", "disarmed");
}

// 获取当前系统状态（网页实时刷新）
void handleGetState(){
  String ret = String(isArm ? "1" : "0") + "|" + String(isAlarm ? "1" : "0");
  server.send(200, "text/plain", ret);
}

// ===================== WiFi连接初始化 =====================
void initWiFi(){
  Serial.print("连接WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功！");
  Serial.print("安防主机网页地址：");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  // IO初始化报警LED
  pinMode(ALARM_LED_PIN, OUTPUT);
  digitalWrite(ALARM_LED_PIN, LOW);

  initWiFi();

  // 绑定Web路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/getstate", handleGetState);
  server.begin();
  Serial.println("Web安防服务器启动完成");
}

void loop() {
  server.handleClient(); // 持续处理网页访问请求

  unsigned long now = millis();
  int touchValue = touchRead(TOUCH_SENSE_PIN);
  bool touchDetect = (touchValue < TOUCH_THRESHOLD);

  // 仅【已布防且未报警】时，触摸才会触发报警锁定
  if(isArm && !isAlarm && touchDetect){
    if(now - lastTouchCheck > DEBOUNCE_TIME){
      lastTouchCheck = now;
      isAlarm = true;
      Serial.println("=== 入侵触发，进入报警锁定 ===");
    }
  }

  // 报警锁定状态：LED高频快速闪烁
  if(isAlarm){
    digitalWrite(ALARM_LED_PIN, HIGH);
    delay(80);
    digitalWrite(ALARM_LED_PIN, LOW);
    delay(80);
  }
}