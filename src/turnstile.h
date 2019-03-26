#ifndef SRC_TURNSTILE_H_
#define SRC_TURNSTILE_H_

#include <condition_variable>
#include <mutex>

struct Turnstile {
  std::mutex mtx;
  std::condition_variable cv;
  bool open{false};
  Turnstile *free{nullptr};
};

class Mutex {
  Turnstile *ts{nullptr};

 public:
  Mutex() = default;
  Mutex(const Mutex &) = delete;

  void lock();    // NOLINT
  void unlock();  // NOLINT
};

#endif  // SRC_TURNSTILE_H_
