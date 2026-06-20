#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "PaintingBot";
const char* ap_password = "12345678";

WebServer server(80);

// 目前狀態
String currentStatus = "待機中";
int currentStep = 0;
int totalSteps = 6;

// 網頁 HTML
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>繪圖機控制</title>
  <style>
    body { font-family: sans-serif; max-width: 420px; margin: 40px auto; padding: 20px; }
    h3 { margin-bottom: 8px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 16px; }
    .btn-color {
      padding: 18px 0; font-size: 16px; border-radius: 8px; border: 3px solid transparent;
      cursor: pointer; background: #2196F3; color: white; transition: all 0.2s;
    }
    .btn-color.selected-1  { border-color: #FF5722; background: #FF7043; }
    .btn-color.selected-2a { border-color: #9C27B0; background: #AB47BC; }
    .btn-color.selected-2b { border-color: #009688; background: #26A69A; }
    .legend { font-size: 13px; color: #555; margin-bottom: 12px; }
    .legend span { margin-right: 12px; }
    .dot { display:inline-block; width:12px; height:12px; border-radius:50%; margin-right:4px; vertical-align:middle; }
    .start { width:100%; padding:15px; font-size:18px; border-radius:8px; border:none;
             cursor:pointer; background:#4CAF50; color:white; margin-top:8px; }
    .stop  { width:100%; padding:15px; font-size:18px; border-radius:8px; border:none;
             cursor:pointer; background:#f44336; color:white; margin-top:8px; }
    .status-box { border:1px solid #ccc; border-radius:8px; padding:15px; margin:15px 0; }
    .progress { background:#eee; border-radius:4px; height:20px; margin-top:8px; }
    .progress-bar { background:#4CAF50; height:20px; border-radius:4px; transition:width 0.5s; }
    .hint { font-size:13px; color:#888; margin-bottom:4px; }
  </style>
</head>
<body>
  <h2>🎨 繪圖機控制</h2>

  <div class="status-box">
    <b>狀態：</b><span id="status">讀取中...</span>
    <div class="progress">
      <div class="progress-bar" id="bar" style="width:0%"></div>
    </div>
    <small id="stepText"></small>
  </div>

  <!-- 第一次塗色 -->
  <h3>第一次塗色（選 1 個）</h3>
  <div class="legend">
    <span><span class="dot" style="background:#FF7043"></span>已選</span>
  </div>
  <div class="grid">
    <button class="btn-color" id="c00" onclick="selectColor1('COLOR_00')">顏色 00</button>
    <button class="btn-color" id="c01" onclick="selectColor1('COLOR_01')">顏色 01</button>
    <button class="btn-color" id="c10" onclick="selectColor1('COLOR_10')">顏色 10</button>
    <button class="btn-color" id="c11" onclick="selectColor1('COLOR_11')">顏色 11</button>
  </div>

  <!-- 第二次塗色 -->
  <h3>第二次塗色（選 2 個）</h3>
  <div class="legend">
    <span><span class="dot" style="background:#AB47BC"></span>顏色 2A</span>
    <span><span class="dot" style="background:#26A69A"></span>顏色 2B</span>
  </div>
  <div class="grid">
    <button class="btn-color" id="d00" onclick="selectColor2('COLOR_00')">顏色 00</button>
    <button class="btn-color" id="d01" onclick="selectColor2('COLOR_01')">顏色 01</button>
    <button class="btn-color" id="d10" onclick="selectColor2('COLOR_10')">顏色 10</button>
    <button class="btn-color" id="d11" onclick="selectColor2('COLOR_11')">顏色 11</button>
  </div>

  <button class="start" onclick="startPaint()">▶ 開始</button>
  <button class="stop"  onclick="sendCmd('STOP')">⏹ 緊急停止</button>

  <script>
    let color1  = null;
    let color2a = null;
    let color2b = null;

    const idMap = {
      COLOR_00: { top: 'c00', bot: 'd00' },
      COLOR_01: { top: 'c01', bot: 'd01' },
      COLOR_10: { top: 'c10', bot: 'd10' },
      COLOR_11: { top: 'c11', bot: 'd11' },
    };

    function clearClass(prefix, cls) {
      ['00','01','10','11'].forEach(k => {
        document.getElementById(prefix + k).classList.remove(cls);
      });
    }

    function selectColor1(val) {
      clearClass('c', 'selected-1');
      color1 = val;
      document.getElementById(idMap[val].top).classList.add('selected-1');
    }

    function selectColor2(val) {
      // 如果已經是 2a 或 2b，取消
      if (color2a === val) {
        color2a = null;
        document.getElementById(idMap[val].bot).classList.remove('selected-2a');
        return;
      }
      if (color2b === val) {
        color2b = null;
        document.getElementById(idMap[val].bot).classList.remove('selected-2b');
        return;
      }
      // 填入空位
      if (!color2a) {
        color2a = val;
        document.getElementById(idMap[val].bot).classList.add('selected-2a');
      } else if (!color2b) {
        color2b = val;
        document.getElementById(idMap[val].bot).classList.add('selected-2b');
      } else {
        alert('已選滿 2 個，請先取消一個');
      }
    }

    function startPaint() {
      if (!color1)          { alert('請選擇第一次塗色的顏色'); return; }
      if (!color2a || !color2b) { alert('請選擇第二次塗色的兩個顏色'); return; }
      sendCmd('COLOR1:' + color1);
      setTimeout(() => sendCmd('COLOR2A:' + color2a), 200);
      setTimeout(() => sendCmd('COLOR2B:' + color2b), 400);
      setTimeout(() => sendCmd('START'), 600);
    }

    function sendCmd(cmd) {
      fetch('/cmd?val=' + cmd).then(r => r.text());
    }

    setInterval(() => {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('status').textContent = data.status;
          let pct = Math.round(data.step / data.total * 100);
          document.getElementById('bar').style.width = pct + '%';
          document.getElementById('stepText').textContent = data.step + ' / ' + data.total + ' 步驟';
        });
    }, 1500);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);        // 用於除錯
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // 和 Mega 溝通

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("熱點已開啟");
  Serial.println("IP: " + WiFi.softAPIP().toString());

  // 根路由：回傳網頁
  server.on("/", [](){ server.send(200, "text/html", htmlPage); });

  // 指令路由
  server.on("/cmd", [](){
    String cmd = server.arg("val");
    Serial2.println(cmd);  // 傳給 Mega
    server.send(200, "text/plain", "OK");
  });

  // 狀態路由
  server.on("/status", [](){
    // 先問 Mega 有沒有新狀態
    if (Serial2.available()) {
      String msg = Serial2.readStringUntil('\n');
      msg.trim();
      if (msg.startsWith("STATUS:")) currentStatus = msg.substring(7);
      if (msg.startsWith("STEP:")) currentStep = msg.substring(5).toInt();
    }
    String json = "{\"status\":\"" + currentStatus + "\",\"step\":" + currentStep + ",\"total\":" + totalSteps + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}