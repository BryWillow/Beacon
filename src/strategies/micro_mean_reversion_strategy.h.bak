#pragma once
#include <deque>
#include <numeric>
#include <mutex>
#include <cstddef>

/**
 * @brief Computes short-term moving average and suggests buy/sell/hold
 */
template <typename PriceType>
class MicroMeanReversionStrategy {
public:
    explicit MicroMeanReversionStrategy(std::size_t window_size) : window_size_(window_size) {}
    int on_new_price(PriceType price) {
        std::lock_guard<std::mutex> lock(mutex_);
        prices_.push_back(price);
        if(prices_.size()>window_size_) prices_.pop_front();
        if(prices_.size()<window_size_) return 0;
        PriceType avg = std::accumulate(prices_.begin(), prices_.end(), PriceType(0))/PriceType(prices_.size());
        if(price<avg) return 1;
        if(price>avg) return -1;
        return 0;
    }
private:
    std::size_t window_size_;
    std::deque<PriceType> prices_;
    std::mutex mutex_;
};
