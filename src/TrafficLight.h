#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "TrafficObject.h"
#include <condition_variable>
#include <deque>
#include <mutex>

// forward declarations to avoid include cycle
class Vehicle;

template <class T> class MessageQueue {
public:
  T receive();
  void send(T &&msg);

private:
  std::mutex _mutex;
  std::condition_variable _condition;
  std::deque<T> _queue;
};

enum TrafficLightPhase { red, green };

class TrafficLight : TrafficObject {
public:
  // constructor / destructor
  TrafficLight();

  // getters / setters
  TrafficLightPhase getCurrentPhase();

  // behaviour methods
  void waitForGreen();
  void simulate();

private:
  // behaviour methods
  void cycleThroughPhases();

  MessageQueue<TrafficLightPhase> _queue;
  std::condition_variable _condition;
  std::mutex _mutex;
  TrafficLightPhase _currentPhase;
};

#endif