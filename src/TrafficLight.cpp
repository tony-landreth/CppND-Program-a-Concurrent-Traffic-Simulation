#include "TrafficLight.h"
#include <future>
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  std::unique_lock<std::mutex> uLock(_mutex);
  _condition.wait(uLock, [this] {
    return !_queue.empty();
  }); // pass unique lock to condition variable
  T msg = std::move(_queue.back());
  _queue.pop_back();

  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  std::lock_guard<std::mutex> uLock(_mutex);
  _queue.push_back(std::move(msg));
  _condition.notify_one(); // notifiy client after pushing new TrafficLightPhase
                           // into vector
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (_queue.receive() == TrafficLightPhase::green) {
      return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // Select a random value between 4 and 6 seconds:
  // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<int> distr(4000, 6000); // define the range
  double cycleDuration =
      distr(eng); // duration of a single simulation cycle in ms
  auto lastUpdate = std::chrono::system_clock::now();

  while (true) {
    // Wait for 1 ms to reduce CPU load
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Measure time for a function, see:
    // https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
    auto timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();

    // Toggle the traffic light on schedule
    if (timeSinceLastUpdate >= cycleDuration) {
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } else {
        _currentPhase = TrafficLightPhase::red;
      }
      auto ftrPhase =
          std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send,
                     &_queue, std::move(_currentPhase));
      ftrPhase.wait();

      // reset stop watch for next cycle
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}
