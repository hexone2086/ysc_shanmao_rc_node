#include "ysc_control/ysc_control_api.h"

// 生成当前时间戳并返回 C 风格字符串
char *ControlClient::getCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

  char buffer[80];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                std::localtime(&currentTime));

  char *result = new char[strlen(buffer) + 1];
  std::strcpy(result, buffer);

  return result;
}

// 使用 tinyxml2 生成心跳包 XML 数据
std::string ControlClient::generateHeartbeatXml() {
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = doc.NewElement("PatrolDevice");
  doc.InsertFirstChild(root);

  root->InsertNewChildElement("Type")->SetText(100);
  root->InsertNewChildElement("Command")->SetText(100);
  root->InsertNewChildElement("Time")->SetText(getCurrentTimestamp());

  tinyxml2::XMLElement *items = doc.NewElement("Items");
  root->InsertEndChild(items);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  return printer.CStr();
}

// 使用 tinyxml2 生成起立指令 XML 数据
std::string ControlClient::generateStandModeXml(int mode) {
  if (mode < 1) mode = 1;
  if (mode > 6) mode = 6;

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = doc.NewElement("PatrolDevice");
  doc.InsertFirstChild(root);

  root->InsertNewChildElement("Type")->SetText(102);
  root->InsertNewChildElement("Command")->SetText(1);
  root->InsertNewChildElement("Time")->SetText(getCurrentTimestamp());

  tinyxml2::XMLElement *items = doc.NewElement("Items");
  root->InsertEndChild(items);

  tinyxml2::XMLElement *motionParam = doc.NewElement("MotionParam");
  motionParam->SetAttribute("number", "true");
  motionParam->SetText(mode);
  items->InsertEndChild(motionParam);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  return printer.CStr();
}

std::string ControlClient::generateMotionModeXml(int mode) {
  if (mode < 1) mode = 1;
  if (mode > 4) mode = 4;

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLElement *root = doc.NewElement("PatrolDevice");
  doc.InsertFirstChild(root);

  root->InsertNewChildElement("Type")->SetText(102);
  root->InsertNewChildElement("Command")->SetText(1);
  root->InsertNewChildElement("Time")->SetText(getCurrentTimestamp());

  tinyxml2::XMLElement *items = doc.NewElement("Items");
  root->InsertEndChild(items);

  tinyxml2::XMLElement *gaitParam = doc.NewElement("GaitParam");
  gaitParam->SetAttribute("number", "true");
  gaitParam->SetText(mode);
  items->InsertEndChild(gaitParam);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  return printer.CStr();
}

// 使用 tinyxml2 生成偏航指令 XML 数据
std::string ControlClient::generateMotionDataXml(float x, float y, float yaw) {
    if (x < -1.0) x = -1.0;
    if (x > 1.0) x = 1.0;
    if (y < -1.0) y = -1.0;
    if (y > 1.0) y = 1.0;
    if (yaw < -1.0) yaw = -1.0;
    if (yaw > 1.0) yaw = 1.0;
  
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *root = doc.NewElement("PatrolDevice");
    doc.InsertFirstChild(root);
  
    root->InsertNewChildElement("Type")->SetText(103);
    root->InsertNewChildElement("Command")->SetText(1);
    root->InsertNewChildElement("Time")->SetText(getCurrentTimestamp());
  
    tinyxml2::XMLElement *items = doc.NewElement("Items");
    root->InsertEndChild(items);
  
    tinyxml2::XMLElement* yElem = doc.NewElement("Y");
    yElem->SetText(y);
    items->InsertEndChild(yElem);
  
    tinyxml2::XMLElement* xElem = doc.NewElement("X");
    xElem->SetText(x);
    items->InsertEndChild(xElem);
  
    tinyxml2::XMLElement* yawElem = doc.NewElement("Yaw");
    yawElem->SetText(yaw);
    items->InsertEndChild(yawElem);
  
    tinyxml2::XMLElement* pitch = doc.NewElement("Pitch");
    pitch->SetText(0.0);
    items->InsertEndChild(pitch);
  
    tinyxml2::XMLElement* roll = doc.NewElement("Roll");
    roll->SetText(0.0);
    items->InsertEndChild(roll);
  
    tinyxml2::XMLElement* z = doc.NewElement("Z");
    z->SetText(0.0);
    items->InsertEndChild(z);
  
    tinyxml2::XMLPrinter printer;
    doc.Print(&printer);
    return printer.CStr();
  }

// 发送 TCP 消息的通用函数
bool ControlClient::sendTcpMessage(const std::string &xmlData) {
  std::lock_guard<std::mutex> lock(socketMutex);
  tcpMessage message;
  memset(&message, 0, sizeof(message));

  // 设置消息头
  message.header[0] = 0xeb;
  message.header[1] = 0x90;
  message.header[2] = 0xeb;
  message.header[3] = 0x90;

  unsigned short dataLength = xmlData.length();
  message.header[4] = dataLength & 0xFF;
  message.header[5] = (dataLength >> 8) & 0xFF;
  message.header[6] = 0x11;
  message.header[7] = 0x00;
  message.header[8] = 0x00;

  memcpy(message.data, xmlData.c_str(), dataLength);

  // 设置发送超时
  struct timeval timeout;
  timeout.tv_sec = SEND_TIMEOUT / 1000;
  timeout.tv_usec = (SEND_TIMEOUT % 1000) * 1000;
  setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  // 发送消息
  ssize_t send_len = send(client_fd, &message, dataLength + 16, 0);
  if (send_len < 0) {
    perror("send failed");
    return false;
  }

  return true;
}

// 心跳包发送线程
void ControlClient::heartbeatThreadFunc() {
  while (running) {
    std::string heartbeatXml = generateHeartbeatXml();
    if (!sendTcpMessage(heartbeatXml)) {
      std::cerr << "Failed to send heartbeat. Trying to reconnect..." << std::endl;
      reconnect();
    } else {
      std::cout << "Heartbeat sent successfully." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
}

bool ControlClient::connectToServer() {
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd < 0) {
    perror("socket creation failed");
    return false;
  }

  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("connect failed");
    close(client_fd);
    return false;
  }

  return true;
}

void ControlClient::reconnect() {
  for (int i = 0; i < RECONNECT_ATTEMPTS; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
    if (connectToServer()) {
      std::cout << "Reconnected to server successfully." << std::endl;
      return;
    }
  }
  std::cerr << "Failed to reconnect after multiple attempts." << std::endl;
  running = false;
}

ControlClient::ControlClient() {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
  }
}

ControlClient::~ControlClient() {
  running = false;
  if (hbThread.joinable()) {
    hbThread.join();
  }
  close(client_fd);
}

bool ControlClient::start() {
  if (!connectToServer()) {
    return false;
  }
  running = true;
  hbThread = std::thread(&ControlClient::heartbeatThreadFunc, this);
  return true;
}

bool ControlClient::standUp(int mode) {
  std::string commandXml = generateStandModeXml(mode);
  return sendTcpMessage(commandXml);
}

bool ControlClient::switchMotionMode(int mode) {
  std::string commandXml = generateMotionModeXml(mode);
  return sendTcpMessage(commandXml);
}

bool ControlClient::sendMotionData(float x, float y, float yaw) {
  std::string commandXml = generateMotionDataXml(x, y, yaw);
  return sendTcpMessage(commandXml);
}