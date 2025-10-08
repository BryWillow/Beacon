// ---------------------------------------------------------------------------
// File        : main_replayer.cpp
// Description : Market Data Replayer for NSDQ ITCH messages
// ---------------------------------------------------------------------------
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include "json.hpp"
#include "nsdq/md_itch_udp_replayer.h"
#include "constants.h"

using json = nlohmann::json;
std::atomic<bool> g_stopFlag{false};
void signalHandler(int signal){ if(signal==SIGINT){ g_stopFlag.store(true); std::cerr << "[Replayer] SIGINT received.\n"; } }

struct ReplayerConfig {
  std::string filePath;
  std::string destIp;
  uint16_t destPort;
  double replaySpeed;
  int cpuCore;
  bool stressTest;
  size_t numMessages;
};

ReplayerConfig loadReplayerConfig(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Cannot open config file: " + path);

  json configJson; file >> configJson;
  const auto &sharedJson = configJson["shared"];
  const auto &replayerJson = configJson["replayer"];

  ReplayerConfig cfg{};
  cfg.filePath = replayerJson.value("file_path","default.itch");
  cfg.destIp = replayerJson.value("dest_ip","127.0.0.1");
  cfg.destPort = sharedJson.value("udp_port",5555);
  cfg.replaySpeed = replayerJson.value("replay_speed",1.0);
  cfg.cpuCore = replayerJson.value("cpu_core",0);
  cfg.stressTest = replayerJson.value("stress_test",false);
  cfg.numMessages = sharedJson.value("num_messages_to_send",10000);
  return cfg;
}

int main(int argc, char* argv[])
{
  try {
    std::signal(SIGINT, signalHandler);
    std::filesystem::path projectRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    std::filesystem::path configPath = projectRoot / "config.json";

    if(!std::filesystem::exists(configPath)) {
      std::cerr << "[Replayer] config.json not found\n";
      return 1;
    }

    ReplayerConfig cfg = loadReplayerConfig(configPath.string());
    std::filesystem::path dataDir = projectRoot / "data";

    std::string inputFile = (argc>1)? argv[1]:"default.itch";
    std::filesystem::path filePath = dataDir / inputFile;
    if(!std::filesystem::exists(filePath)) {
      std::cerr << "[Replayer] File does not exist: " << filePath << "\n";
      return 1;
    }
    cfg.filePath = filePath.string();

    ItchMessageUdpReplayer replayer(cfg.filePath,cfg.destIp,cfg.destPort,cfg.replaySpeed,cfg.cpuCore);
    std::cout << "[Replayer] Loading messages...\n";
    replayer.loadAllMessages();
    std::cout << "[Replayer] Loaded " << cfg.numMessages << " messages.\n";
    replayer.start();

    while(!replayer.finished() && !g_stopFlag.load(std::memory_order_relaxed)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    replayer.stop();
    std::cout << "[Replayer] Replay complete.\n";
  }
  catch(const std::exception& e) {
    std::cerr << "[Replayer] Exception: " << e.what() << "\n"; return 1;
  }

  return 0;
}