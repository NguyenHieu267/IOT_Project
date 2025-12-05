// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", function () {
  renderRelays();
});
window.addEventListener("load", onLoad);

let gaugeTemp, gaugeHumi;
function onLoad(event) {
  initWebSocket();
}

function onOpen(event) {
  console.log("Connection opened");
}

function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
  console.log("Trying to open a WebSocket connection…");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function Send_Data(data) {
  if (websocket && websocket.readyState === WebSocket.OPEN) {
    websocket.send(data);
    console.log("📤 Gửi:", data);
  } else {
    console.warn("⚠️ WebSocket is not ready!");
    alert("⚠️ WebSocket is not connected!");
  }
}

function onMessage(event) {
  console.log("📩 Receive:", event.data);
  try {
    var data = JSON.parse(event.data);

    // If sensor data present, update gauges
    if (data.temperature !== undefined && data.humidity !== undefined) {
      if (
        typeof gaugeTemp !== "undefined" &&
        typeof gaugeHumi !== "undefined"
      ) {
        gaugeTemp.refresh(Math.round(data.temperature));
        gaugeHumi.refresh(Math.round(data.humidity));
      }
    }

    // Handle device/state updates
    if (data.page === "device" && data.value) {
      const relay = relayList.find((r) => r.gpio == data.value.gpio);
      if (relay) {
        relay.state = data.value.status === "ON";
        renderRelays();
      }
    }
  } catch (e) {
    console.warn("This json file is not valid", event.data);
  }
}

// ==================== UI NAVIGATION ====================
let relayList = [
  { id: 1, name: "Onboard LED", gpio: 48, state: false },
  { id: 2, name: "NeoPixel", gpio: 45, state: false },
];
let deleteTarget = null;

function showSection(id, event) {
  document
    .querySelectorAll(".section")
    .forEach((sec) => (sec.style.display = "none"));
  document.getElementById(id).style.display =
    id === "settings" ? "flex" : "block";
  document
    .querySelectorAll(".nav-item")
    .forEach((i) => i.classList.remove("active"));
  event.currentTarget.classList.add("active");
}

// ==================== HOME GAUGES ====================
window.onload = function () {
  const style = this.document.createElement("style");
  style.innerHTML = `/* Path thứ 2 là thanh giá trị (màu vàng/xanh) */
        #gauge_temp path:nth-of-type(2),
        #gauge_humi path:nth-of-type(2) {
            stroke: #000 !important;      /* Màu viền đen */
            stroke-width: 3px !important; /* Độ dày viền */
            stroke-opacity: 10 !important; /* Hiển thị rõ */
        }`;
  document.head.appendChild(style);
  gaugeTemp = new JustGage({
    id: "gauge_temp",
    value: 26,
    min: -10,
    max: 50,
    donut: true,
    pointer: false,
    gaugeWidthScale: 0.25,
    gaugeColor: "transparent",
    levelColorsGradient: true,
    levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"],
  });

  gaugeHumi = new JustGage({
    id: "gauge_humi",
    value: 60,
    min: 0,
    max: 100,
    donut: true,
    pointer: false,
    gaugeWidthScale: 0.25,
    gaugeColor: "transparent",
    levelColorsGradient: true,
    levelColors: ["#42A5F5", "#00BCD4", "#0288D1"],
  });

  // Gauges will be updated by WebSocket messages from the device.
};

// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
  document.getElementById("addRelayDialog").style.display = "flex";
}
function closeAddRelayDialog() {
  document.getElementById("addRelayDialog").style.display = "none";
}
function saveRelay() {
  const name = document.getElementById("relayName").value.trim();
  const gpio = document.getElementById("relayGPIO").value.trim();
  if (!name || !gpio) return alert("⚠️ Please fill all fields!");
  relayList.push({ id: Date.now(), name, gpio, state: false });
  renderRelays();
  closeAddRelayDialog();
}
function renderRelays() {
  const container = document.getElementById("relayContainer");
  container.innerHTML = "";
  relayList.forEach((r) => {
    const card = document.createElement("div");
    card.className = "device-card";
    card.innerHTML = `
      <i class="fa-solid fa-bolt device-icon"></i>
      <h3>${r.name}</h3>
      <p>GPIO: ${r.gpio}</p>
      <button class="toggle-btn ${r.state ? "on" : ""}" onclick="toggleRelay(${
      r.id
    })">
        ${r.state ? "ON" : "OFF"}
      </button>
      <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${
        r.id
      })"></i>
    `;
    container.appendChild(card);
  });
}
function toggleRelay(id) {
  const relay = relayList.find((r) => r.id === id);
  if (relay) {
    relay.state = !relay.state;
    const relayJSON = JSON.stringify({
      page: "device",
      value: {
        name: relay.name,
        status: relay.state ? "ON" : "OFF",
        gpio: relay.gpio,
      },
    });
    Send_Data(relayJSON);
    renderRelays();
  }
}
function showDeleteDialog(id) {
  deleteTarget = id;
  document.getElementById("confirmDeleteDialog").style.display = "flex";
}
function closeConfirmDelete() {
  document.getElementById("confirmDeleteDialog").style.display = "none";
}
function confirmDelete() {
  relayList = relayList.filter((r) => r.id !== deleteTarget);
  renderRelays();
  closeConfirmDelete();
}

// ==================== SETTINGS FORM (BỔ SUNG) ====================
document
  .getElementById("settingsForm")
  .addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
      page: "setting",
      value: {
        ssid: ssid,
        password: password,
        token: token,
        server: server,
        port: port,
      },
    });

    Send_Data(settingsJSON);
    alert("✅ Configuration has been sent to the device!");
  });

// ==================== RESET WIFI ====================
function resetWiFi() {
  if (
    confirm(
      "⚠️ Are you sure you want to reset WiFi?\nThe device will return to AP mode!"
    )
  ) {
    const resetJSON = JSON.stringify({
      page: "reset",
      value: {
        action: "reset_wifi",
      },
    });
    Send_Data(resetJSON);
    alert("🔄 RESTING WIFI");
  }
}
