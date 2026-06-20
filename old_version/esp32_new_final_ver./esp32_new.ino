#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "PaintingBot";
const char* ap_password = "12345678";

WebServer server(80);

// 目前狀態
String currentStatus = "待機中";
int currentStep = 0;
int totalSteps =8;
String currentPos = "0,0";

// 網頁 HTML
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>PaintingBot</title>
  <style>
    body { font-family: sans-serif; max-width: 420px; margin: 30px auto; padding: 20px; }
    h3 { margin-bottom: 8px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 16px; }
    .btn-color {
      padding: 18px 0; font-size: 16px; border-radius: 8px; border: 3px solid transparent;
      cursor: pointer; color: white; transition: all 0.2s; font-weight: bold;
    }
    .btn-color.disabled { opacity: 0.3; cursor: not-allowed; }
    .btn-color.selected { border-color: #333; box-shadow: 0 0 0 2px #333 inset; }
    #c00, #d00, #e00 { background: #FF6FA5; } /* 粉色 */
    #c01, #d01, #e01 { background: #FFD93D; color:#333; } /* 黃色 */
    #c10, #d10, #e10 { background: #E63946; } /* 紅色 */
    #c11, #d11, #e11 { background: #3B6FE0; } /* 藍色 */

    .start { width:100%; padding:15px; font-size:18px; border-radius:8px; border:none;
             cursor:pointer; background:#4CAF50; color:white; margin-top:8px; }
    .stop  { width:100%; padding:15px; font-size:18px; border-radius:8px; border:none;
             cursor:pointer; background:#f44336; color:white; margin-top:8px; }
    .status-box { border:1px solid #ccc; border-radius:8px; padding:15px; margin:15px 0; }
    .progress { background:#eee; border-radius:4px; height:20px; margin-top:8px; }
    .progress-bar { background:#4CAF50; height:20px; border-radius:4px; transition:width 0.5s; }

    .map-container {
      position: relative;
      width: 280px;
      height: 280px;
      border: 2px solid #333;
      margin: 15px auto;
      background: #f5f5f5;
    }
    .brush-dot {
      position: absolute;
      font-size: 22px;
      transform: translate(-50%, -50%);
      transition: left 0.3s, top 0.3s;
      z-index: 10;
    }
    .map-zone {
      position: absolute;
      border: 1px solid #888;
      border-radius: 4px;
      font-size: 9px;
      display: flex;
      align-items: center;
      justify-content: center;
      color: #555;
      text-align: center;
    }
  </style>
</head>
<body>
  <h2>PaintingBot</h2>

  <div class="status-box">
    <b>狀態：</b><span id="status">讀取中...</span>
    <div class="progress">
      <div class="progress-bar" id="bar" style="width:0%"></div>
    </div>
    <small id="stepText"></small>
  </div>

  <h3>即時位置</h3>
  <div class="map-container" id="mapArea">
    <div class="map-zone" style="left:1%; top:9%; width:53%; height:80%; background:#fff;">紙張</div>
    <div class="map-zone" style="left:55%; top:5%; width:8%; height:42%; background:#FFD8E8;">粉色</div>
    <div class="map-zone" style="left:65%; top:5%; width:8%; height:42%; background:#FFF3B0;">黃色</div>
    <div class="map-zone" style="left:55%; top:49%; width:8%; height:42%; background:#F7B7BB;">紅色</div>
    <div class="map-zone" style="left:65%; top:49%; width:8%; height:42%; background:#BBD0F0;">藍色</div>
    <div class="map-zone" style="left:74%; top:15%; width:11%; height:70%; background:#cfe8ff;">抹布</div>
    <div class="map-zone" style="left:86%; top:10%; width:13%; height:40%; background:#cfe8ff;">水槽1</div>
    <div class="map-zone" style="left:86%; top:50%; width:13%; height:40%; background:#cfe8ff;">水槽2</div>
    <div class="brush-dot" id="brushDot">🖌️</div>
  </div>

  <h3>第一次塗色</h3>
  <div class="grid">
    <button class="btn-color" id="c00" onclick="selectColor(1,'COLOR_00')">粉色</button>
    <button class="btn-color" id="c01" onclick="selectColor(1,'COLOR_01')">黃色</button>
    <button class="btn-color" id="c10" onclick="selectColor(1,'COLOR_10')">紅色</button>
    <button class="btn-color" id="c11" onclick="selectColor(1,'COLOR_11')">藍色</button>
  </div>

  <h3>第二次塗色</h3>
  <div class="grid">
    <button class="btn-color" id="d00" onclick="selectColor(2,'COLOR_00')">粉色</button>
    <button class="btn-color" id="d01" onclick="selectColor(2,'COLOR_01')">黃色</button>
    <button class="btn-color" id="d10" onclick="selectColor(2,'COLOR_10')">紅色</button>
    <button class="btn-color" id="d11" onclick="selectColor(2,'COLOR_11')">藍色</button>
  </div>

  <h3>第三次塗色</h3>
  <div class="grid">
    <button class="btn-color" id="e00" onclick="selectColor(3,'COLOR_00')">粉色</button>
    <button class="btn-color" id="e01" onclick="selectColor(3,'COLOR_01')">黃色</button>
    <button class="btn-color" id="e10" onclick="selectColor(3,'COLOR_10')">紅色</button>
    <button class="btn-color" id="e11" onclick="selectColor(3,'COLOR_11')">藍色</button>
  </div>

  <button class="start" onclick="startPaint()">▶ 開始</button>
  <button class="stop"  onclick="sendCmd('STOP')">⏹ 緊急停止</button>

  <script>
    let picks = { 1: null, 2: null, 3: null };
    const prefixMap = { 1: 'c', 2: 'd', 3: 'e' };
    const colors = ['COLOR_00','COLOR_01','COLOR_10','COLOR_11'];

    function refreshButtons() {
      [1,2,3].forEach(round => {
        const prefix = prefixMap[round];
        colors.forEach(col => {
          const id = prefix + col.split('_')[1];
          const btn = document.getElementById(id);
          btn.classList.remove('selected', 'disabled');

          const takenByOther = Object.entries(picks).some(([r, v]) => v === col && Number(r) !== round);
          if (takenByOther) btn.classList.add('disabled');

          if (picks[round] === col) btn.classList.add('selected');
        });
      });
    }

    function selectColor(round, val) {
      const takenByOther = Object.entries(picks).some(([r, v]) => v === val && Number(r) !== round);
      if (takenByOther) return;

      if (picks[round] === val) {
        picks[round] = null;
      } else {
        picks[round] = val;
      }
      refreshButtons();
    }

    function startPaint() {
      if (!picks[1] || !picks[2] || !picks[3]) {
        alert('請完成三次顏色選擇');
        return;
      }
      sendCmd('COLOR1:' + picks[1]);
      setTimeout(() => sendCmd('COLOR2:' + picks[2]), 200);
      setTimeout(() => sendCmd('COLOR3:' + picks[3]), 400);
      setTimeout(() => sendCmd('START'), 600);
    }

    function sendCmd(cmd) {
      fetch('/cmd?val=' + cmd).then(r => r.text());
    }

    const X_MAX = 1950;
    const Y_MAX = 1200;
    const MAP_SIZE = 280;

    function updateBrushPosition(posStr) {
      const parts = posStr.split(',').map(Number);
      if (parts.length !== 2) return;
      const [x, y] = parts;
      const left = (x / X_MAX) * MAP_SIZE;
      const top  = (y / Y_MAX) * MAP_SIZE;
      const dot = document.getElementById('brushDot');
      dot.style.left = left + 'px';
      dot.style.top  = top + 'px';
    }

    setInterval(() => {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('status').textContent = data.status;
          let pct = Math.round(data.step / data.total * 100);
          document.getElementById('bar').style.width = pct + '%';
          document.getElementById('stepText').textContent = data.step + ' / ' + data.total + ' 步驟';
          if (data.pos) updateBrushPosition(data.pos);
        });
    }, 1000);

    refreshButtons();
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
    while (Serial2.available()) {
      String msg = Serial2.readStringUntil('\n');
      msg.trim();
      if (msg.startsWith("STATUS:")) currentStatus = msg.substring(7);
      if (msg.startsWith("STEP:"))   currentStep = msg.substring(5).toInt();
      if (msg.startsWith("POS:"))    currentPos = msg.substring(4);
    }
    String json = "{\"status\":\"" + currentStatus + "\",\"step\":" + currentStep + 
                  ",\"total\":" + totalSteps + ",\"pos\":\"" + currentPos + "\"}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}