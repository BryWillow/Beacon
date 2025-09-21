#pragma once
#include "spsc_ringbuffer.h"
#include <thread>
#include <atomic>

template <typename MsgType, typename Callback>
class SpScRingBufferConsumer {
public:
    SpScRingBufferConsumer(SpScRingBuffer<MsgType>& buffer, Callback callback)
    : _buffer(buffer), _callback(callback), _stopFlag(false) {}
    void start(int core = -1) { _thread = std::thread([this]{ consumeLoop(); }); }
    void stop() { _stopFlag.store(true); if (_thread.joinable()) _thread.join(); }
private:
    SpScRingBuffer<MsgType>& _buffer;
    Callback _callback;
    std::atomic<bool> _stopFlag;
    std::thread _thread;
    void consumeLoop() { MsgType msg; while(!_stopFlag.load()) { _buffer.tryPop(msg); _callback(msg); } }
};
