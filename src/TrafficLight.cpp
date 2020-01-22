#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    //remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // perform queue modification under the lock

    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modication under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "  Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notifiy client after pushing new TrafficLightPhase into vector
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
		while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
			enum TrafficLightPhase phase = _queue->receive();
			if(phase == TrafficLightPhase::green) { break; }
		}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 


    // Select a random value between 4 and 6 seconds
    // see: https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(4, 6); // define the range
    double cycleDuration = distr(eng)*1000; // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    while(true) {
      // Wait for 1 ms to reduce CPU load
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      // Measure time for a function, see: https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
      long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

      // Toggle the traffic light on schedule
      if(timeSinceLastUpdate >= cycleDuration) {
        if(_currentPhase == TrafficLightPhase::red) {
          _currentPhase = TrafficLightPhase::green;
        } else {
          _currentPhase = TrafficLightPhase::red;
        }
      }
      // Send an update to the message queue, see FP.5a
			std::cout << "Spawning threads..." << std::endl;
			std::vector<std::future<void> > futures;
			futures.emplace_back(std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _queue, std::move(_currentPhase)));

      // reset stop watch for next cycle
      lastUpdate = std::chrono::system_clock::now();
    }
}
