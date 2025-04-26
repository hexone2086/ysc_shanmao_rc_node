#ifndef CONTROL_H
#define CONTROL_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <tinyxml2.h>
#include <string>

#define SERVER_IP "10.21.41.1"
#define PORT 30000
#define BUFFER_SIZE 1024
#define RECONNECT_ATTEMPTS 5
#define RECONNECT_DELAY 5000  // 毫秒
#define SEND_TIMEOUT 3000     // 毫秒

struct tcpMessage {
  unsigned char header[16];
  unsigned char data[BUFFER_SIZE];
};

class ControlClient {
private:
  int client_fd;
  struct sockaddr_in server_addr;
  std::thread hbThread;
  std::atomic<bool> running{false};
  std::mutex socketMutex;

  char *getCurrentTimestamp();
  std::string generateHeartbeatXml();
  std::string generateStandModeXml(int mode);
  std::string generateMotionModeXml(int mode);
  std::string generateMotionDataXml(float x, float y, float yaw);
  bool sendTcpMessage(const std::string &xmlData);
  void heartbeatThreadFunc();
  bool connectToServer();
  void reconnect();

public:
  ControlClient();
  ~ControlClient();
  bool start();
  bool standUp(int mode = 1);
  bool switchMotionMode(int mode);
  bool sendMotionData(float x, float y, float yaw);
};

#endif // CONTROL_H
