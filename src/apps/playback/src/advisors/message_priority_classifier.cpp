#include <IClassifyMessagePriority.h>
#include "../playback_state.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace playback::advisors {

constexpr double DEFAULT_PRICE_THRESHOLD = 1000.00;
constexpr const char* PRICE_MOVE_THRESHOLD_KEY = "price_move_threshold";
constexpr const char* SYMBOL_THRESHOLDS_KEY = "symbol_price_move_thresholds";
constexpr size_t SYMBOL_OFFSET = 0; // adjust as needed
constexpr size_t SYMBOL_LENGTH = 8; // adjust as needed

/// @brief   Maintains all symbols' last prices. If a specific symbol moves
///          by more than the configured price, then this message is classified
///          as CRITICAL.
/// @example The last price received for MSFT was $100.
///          You receive a new price for MSFT if $100.26.
///          You configured a threhsold of $0.25.
///          The new price > threshold so the message will be CRITICAL.
/// @details Classifiers frequently work in tandem with Advisors.
///          For example, imagine that a rate limit rule is in place.
///          Just because this message is CRITICAL does not mean it will be sent.
///          Classifying messages is simply putting a label on them. Clearly,
///          you can't just allow this single message through - what if there
///          were other messages already being rate limited, but not critical?
///          The client would end up with out of order messages.
///          When this classifier is used with an advisor, you have more options.
///          (1) An advisor could override the rate limiting logic, flush all
///              all queued messages (even for a specific symbol).
///          (2) An advisor could halt a specific symbol for a brief period.
///          (3) An advisor could temporarily raise the rate limit.
///          
///          The bottom line is that we can have numerous combinations of
///          classifiers and actions, to have utmost flexibility.
class PriceBasedMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    // Example: Assume message contains price at offset 8 as double
    double price = *reinterpret_cast<const double*>(message + 8);
    if (price > 1000.0) {
      return MessagePriority::CRITICAL;
    }
    return MessagePriority::NORMAL;
  }
};

class MaxPriceMoveMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxPriceMoveMessagePriorityClassifier(const std::string& configPath)
    : _thresholdMovePct(DEFAULT_PRICE_THRESHOLD), _lastPrice(0.0), _isFirstPrice(true) {
    try {
      std::ifstream f(configPath);
      nlohmann::json j;
      f >> j;
      if (j.contains(PRICE_MOVE_THRESHOLD_KEY)) {
        _thresholdMovePct = j[PRICE_MOVE_THRESHOLD_KEY].get<double>();
      }
    } 
    catch (const std::exception& e) {
      std::cerr << "[MaxPriceMoveMessagePriorityClassifier] Failed to load config: " << e.what()
                << ". Using default threshold: " << _thresholdMovePct << std::endl;
    }
  }

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    double price = *reinterpret_cast<const double*>(message + 8);
    MessagePriority priority = MessagePriority::NORMAL;

    if (_isFirstPrice) {
      _lastPrice = price;
      _isFirstPrice = false;
      return priority;
    }

    // Calculate percentage move: (new - old) / old * 100
    double pctMove = (_lastPrice == 0.0) ? 0.0 : (std::abs(price - _lastPrice) / std::abs(_lastPrice)) * 100.0;
    if (pctMove > _thresholdMovePct) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrice = price;
    return priority;
  }

private:
  double _thresholdMovePct; // percentage threshold
  double _lastPrice;
  bool _isFirstPrice;
};

class MaxPriceMoveBySymbolMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxPriceMoveBySymbolMessagePriorityClassifier(const std::string& configPath)
    : _defaultThreshold(DEFAULT_PRICE_THRESHOLD) {
    try {
      std::ifstream f(configPath);
      nlohmann::json j;
      f >> j;
      if (j.contains(PRICE_MOVE_THRESHOLD_KEY)) {
        _defaultThreshold = j[PRICE_MOVE_THRESHOLD_KEY].get<double>();
      }
      if (j.contains(SYMBOL_THRESHOLDS_KEY)) {
        for (auto& [symbol, threshold] : j[SYMBOL_THRESHOLDS_KEY].items()) {
          _symbolThresholds[symbol] = threshold.get<double>();
        }
      }
    } catch (const std::exception& e) {
      std::cerr << "[MaxPriceMoveBySymbolMessagePriorityClassifier] Failed to load config: " << e.what()
                << ". Using default threshold: " << _defaultThreshold << std::endl;
    }
  }

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    std::string symbol(message + SYMBOL_OFFSET, SYMBOL_LENGTH);
    double price = *reinterpret_cast<const double*>(message + 8);

    double thresholdPct = _defaultThreshold;
    if (_symbolThresholds.count(symbol)) {
      thresholdPct = _symbolThresholds[symbol];
    }

    MessagePriority priority = MessagePriority::NORMAL;

    if (_lastPrices.find(symbol) == _lastPrices.end()) {
      _lastPrices[symbol] = price;
      return priority;
    }

    double lastPrice = _lastPrices[symbol];
    // Calculate percentage move: (new - old) / old * 100
    double pctMove = (lastPrice == 0.0) ? 0.0 : (std::abs(price - lastPrice) / std::abs(lastPrice)) * 100.0;
    if (pctMove > thresholdPct) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrices[symbol] = price;
    return priority;
  }

private:
  double _defaultThreshold; // percentage threshold
  std::unordered_map<std::string, double> _symbolThresholds; // symbol -> percentage threshold
  std::unordered_map<std::string, double> _lastPrices;       // symbol -> last price
};

// Classifier for raw price moves (absolute value)
class MaxRawPriceMoveMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxRawPriceMoveMessagePriorityClassifier(const std::string& configPath)
    : _thresholdMoveRaw(DEFAULT_PRICE_THRESHOLD), _lastPrice(0.0), _isFirstPrice(true) {
    try {
      std::ifstream f(configPath);
      nlohmann::json j;
      f >> j;
      if (j.contains(PRICE_MOVE_THRESHOLD_KEY)) {
        _thresholdMoveRaw = j[PRICE_MOVE_THRESHOLD_KEY].get<double>();
      }
    } 
    catch (const std::exception& e) {
      std::cerr << "[MaxRawPriceMoveMessagePriorityClassifier] Failed to load config: " << e.what()
                << ". Using default threshold: " << _thresholdMoveRaw << std::endl;
    }
  }

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    double price = *reinterpret_cast<const double*>(message + 8);
    MessagePriority priority = MessagePriority::NORMAL;

    if (_isFirstPrice) {
      _lastPrice = price;
      _isFirstPrice = false;
      return priority;
    }

    double priceMove = std::abs(price - _lastPrice);
    if (priceMove > _thresholdMoveRaw) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrice = price;
    return priority;
  }

private:
  double _thresholdMoveRaw; // raw price threshold
  double _lastPrice;
  bool _isFirstPrice;
};

// Classifier for percentage price moves
class MaxPctPriceMoveMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxPctPriceMoveMessagePriorityClassifier(const std::string& configPath)
    : _thresholdMovePct(DEFAULT_PRICE_THRESHOLD), _lastPrice(0.0), _isFirstPrice(true) {
    try {
      std::ifstream f(configPath);
      nlohmann::json j;
      f >> j;
      if (j.contains(PRICE_MOVE_THRESHOLD_KEY)) {
        _thresholdMovePct = j[PRICE_MOVE_THRESHOLD_KEY].get<double>();
      }
    } 
    catch (const std::exception& e) {
      std::cerr << "[MaxPctPriceMoveMessagePriorityClassifier] Failed to load config: " << e.what()
                << ". Using default threshold: " << _thresholdMovePct << std::endl;
    }
  }

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    double price = *reinterpret_cast<const double*>(message + 8);
    MessagePriority priority = MessagePriority::NORMAL;

    if (_isFirstPrice) {
      _lastPrice = price;
      _isFirstPrice = false;
      return priority;
    }

    double pctMove = (_lastPrice == 0.0) ? 0.0 : (std::abs(price - _lastPrice) / std::abs(_lastPrice)) * 100.0;
    if (pctMove > _thresholdMovePct) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrice = price;
    return priority;
  }

private:
  double _thresholdMovePct; // percentage threshold
  double _lastPrice;
  bool _isFirstPrice;
};

} // namespace playback::advisors
