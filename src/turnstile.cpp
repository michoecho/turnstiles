#include "turnstile.h"
#include <memory>

static std::mutex &getInterlock(Mutex *ptr) {
  static constexpr size_t INTERLOCK_SIZE = 257;
  static std::mutex interlock[INTERLOCK_SIZE];
  return interlock[reinterpret_cast<uintptr_t>(ptr) % INTERLOCK_SIZE];
}

thread_local std::unique_ptr<Turnstile> local_ts(new Turnstile);

static int nonsense;
static Turnstile *const dummy = reinterpret_cast<Turnstile *>(&nonsense);

void Mutex::lock() {
  std::mutex &interlock = getInterlock(this);
  interlock.lock();

  if (ts == nullptr) {
    // The Mutex is released. We acquire it.
    ts = dummy;
    interlock.unlock();
    return;
  }

  if (ts == dummy) {
    // The Mutex is acquired, but it has no turnstile yet.
    // We will lend our thread_local turnstile to it.
    ts = local_ts.release();
  } else {
    // The Mutex is acquired, and has a turnstile. We will lock on it
    // and add our own turnstile to its free list.
    local_ts->free = ts->free;
    ts->free = local_ts.release();
  }

  interlock.unlock();
  {
    std::unique_lock<std::mutex> lock(ts->mtx);
    ts->cv.wait(lock, [this] { return ts->open; });
    ts->open = false;
  }
  interlock.lock();

  // We need to restock our turnstile.
  if (ts->free) {
    local_ts.reset(ts->free);
    ts->free = local_ts->free;
    local_ts->free = nullptr;
  } else {
    local_ts.reset(ts);
    ts = dummy;
  }

  interlock.unlock();
}

void Mutex::unlock() {
  std::unique_lock<std::mutex> lock(getInterlock(this));
  if (ts != dummy) {
    ts->mtx.lock();
    ts->open = true;
    ts->mtx.unlock();
    ts->cv.notify_one();
  } else {
    ts = nullptr;
  }
}
