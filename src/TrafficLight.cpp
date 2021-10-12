#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  
  std::unique_lock<std::mutex> lock(_mutex);
  // wait for message
  _condition.wait(lock);
  // pull message from queue
  T recieved = std::move(_queue.back());
  _queue.pop_back();
  return recieved;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  
  std::lock_guard<std::mutex> lock(_mutex);
  _queue.clear();  // Source: https://knowledge.udacity.com/questions/98313
  _queue.push_back(msg);
  _condition.notify_one();
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
      if(_queue.receive() == TrafficLightPhase::green) {
        return;
      }
      // sleep for 1ms to reduce load on the processor
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  return;
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  TrafficObject::threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
  
  
  // Cycle duration should be random value between 4 and 6 seconds. 
  // create a random set of values from which to choose the cycle durations
  
  // Source: cppreference.com - https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(4, 6); 
  double cycleDur = distrib(gen);
  
  
  // Set timers
  std::chrono::time_point<std::chrono::system_clock> time_1;
  std::chrono::time_point<std::chrono::system_clock> time_2;
  
  //Initialize the timer 1
  time_1 = std::chrono::system_clock::now();
  
  //Infinite loop 
  	while(true) {
      // sleep for 1ms to reduce load on the processor
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
      //Initialize timer 2
      time_2 = std::chrono::system_clock::now();
      
      // Toggle lights if time duration is greater than above defined cycle duration
      if ((std::chrono::duration_cast<std::chrono::seconds>(time_1 - time_2).count()) >= cycleDur) {
        if (_currentPhase == red) {
          _currentPhase = green;}
          else {
              _currentPhase = red;
          }
        time_1 = std::chrono::system_clock::now();
        // push each new TrafficLightPhase into Messagequeue using move semantics in conjunction with send
        _queue.send(std::move(_currentPhase));
        

      }
    }
}

