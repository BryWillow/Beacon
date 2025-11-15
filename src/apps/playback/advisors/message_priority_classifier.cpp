#include <IClassifyMessagePriority.h>
#include "../playback_state.h"

namespace playback::advisors {

// Classifies messages as CRITICAL if price move exceeds threshold, else NORMAL
class MaxPriceMoveMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxPriceMoveMessagePriorityClassifier(double threshold)
    : _thresholdMovePrice(threshold), _lastPrice(0.0), _isFirstPrice(true) {}

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    double price = *reinterpret_cast<const double*>(message + 8);
    MessagePriority priority = MessagePriority::NORMAL;

    if (_isFirstPrice) {
      _lastPrice = price;
      _isFirstPrice = false;
      return priority;
    }

    double priceMove = std::abs(price - _lastPrice);
    if (priceMove > _thresholdMovePrice) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrice = price;
    return priority;
  }

private:
  double _thresholdMovePrice;
  double _lastPrice;
  bool _isFirstPrice;
};

} // namespace playback::advisors
