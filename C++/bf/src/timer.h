#ifndef TIMER_H_
#define TIMER_H_

#include <chrono>

class Timer {
public:
  void reset();
  double elapsed() const;

private:
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

#endif
