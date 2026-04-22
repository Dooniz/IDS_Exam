// Load MQTT.js from CDN
const mqttUrl = "https://unpkg.com/mqtt/dist/mqtt.min.js";
const script = document.createElement("script");
script.src = mqttUrl;
document.head.appendChild(script);

let mqttClient;
let messageReceived = "No message yet.";
let inputBox, publishBtn, ledOnBtn, ledOffBtn;
let topic = "esp32IDS/vandkvalitet"; // Shared topic for send/receive

script.onload = function () {
  setupMQTT();
};

function setupMQTT() {
  let randomId = "p5js_client_" + Math.random().toString(36).substr(2, 5);
  let broker = "wss://public.cloud.shiftr.io:443"; // WSS for secure WebSocket

  mqttClient = mqtt.connect(broker, {
    clientId: randomId,
    username: "public",
    password: "public",
  });

  mqttClient.on("connect", onConnect);
  mqttClient.on("message", onMessageArrived);
  mqttClient.on("error", onFailure);
  mqttClient.on("close", onConnectionLost);
}

function setup() {
  createCanvas(500, 300);
  background(220);

  // Input field for publishing messages
  inputBox = createInput('');
  inputBox.position(20, 20);
  inputBox.size(300);

  // Publish button
  publishBtn = createButton("Publish");
  publishBtn.position(340, 20);
  publishBtn.mousePressed(publishMessage);

  // LED ON button
  ledOnBtn = createButton("LED ON");
  ledOnBtn.position(20, 60);
  ledOnBtn.style("background-color", "green");
  ledOnBtn.style("color", "white");
  ledOnBtn.mousePressed(() => sendLedCommand("LED_ON"));

  // LED OFF button
  ledOffBtn = createButton("LED OFF");
  ledOffBtn.position(120, 60);
  ledOffBtn.style("background-color", "red");
  ledOffBtn.style("color", "white");
  ledOffBtn.mousePressed(() => sendLedCommand("LED_OFF"));
}

function draw() {
  background(220);
  textAlign(CENTER, CENTER);
  textSize(16);
  fill(0);
  text("Last message: " + messageReceived, width / 2, height / 2);
}

function publishMessage() {
  let msg = inputBox.value().trim();
  if (msg && mqttClient && mqttClient.connected) {
    mqttClient.publish(topic, msg);
    console.log("Published:", msg);
    inputBox.value(""); // Clear input after sending
  } else {
    console.log("MQTT not connected or empty message");
  }
}

function sendLedCommand(cmd) {
  if (mqttClient && mqttClient.connected) {
    mqttClient.publish(topic, cmd);
    console.log("Published:", cmd);
  } else {
    console.log("MQTT not connected, cannot send LED command");
  }
}

function onConnect() {
  console.log("Connected to MQTT");
  mqttClient.subscribe(topic);
}

function onMessageArrived(topic, message) {
  let msg = message.toString();
  console.log("Message Arrived: " + msg);
  messageReceived = msg;

  if (msg === "Button Pressed") {
    drawRandomCircle();
  }
}

function drawRandomCircle() {
  let x = random(width);
  let y = random(height);
  let radius = random(20, 50);
  let r = random(255);
  let g = random(255);
  let b = random(255);

  fill(r, g, b);
  noStroke();
  ellipse(x, y, radius * 2);
}

function onFailure(error) {
  console.log("Failed to connect: " + error);
}

function onConnectionLost() {
  console.log("Connection Lost");
}
