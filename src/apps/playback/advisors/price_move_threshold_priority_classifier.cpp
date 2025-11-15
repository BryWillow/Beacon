#include <IClassifyMessagePriority.h>
#include "../playback_state.h"

namespace playback::advisors {

// Classifies messages as CRITICAL if price move exceeds threshold, else NORMAL
class MaxPriceMoveMessagePriorityClassifier : public IClassifyMessagePriority {
public:
  MaxPriceMoveMessagePriorityClassifier(double threshold)
    : _threshold(threshold), _lastPrice(0.0), _isFirst(true) {}

  MessagePriority classify(size_t messageIndex, const char* message, const PlaybackState& state) override {
    double price = *reinterpret_cast<const double*>(message + 8);
    MessagePriority priority = MessagePriority::NORMAL;

    if (_isFirst) {
      _lastPrice = price;
      _isFirst = false;
      return priority;
    }

    double priceMove = std::abs(price - _lastPrice);
    if (priceMove > _threshold) {
      priority = MessagePriority::CRITICAL;
    }

    _lastPrice = price;
    return priority;
  }

private:
  double _threshold;
  double _lastPrice;
  bool _isFirst;
};

} // namespace playback::advisors
