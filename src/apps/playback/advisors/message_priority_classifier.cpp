#include "../interfaces/IClassifyMessagePriority.h"
#include "../playback_state.h"

namespace playback::advisors {

// Example: Classifies messages as CRITICAL if price > 1000, else NORMAL
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

} // namespace playback::advisors
