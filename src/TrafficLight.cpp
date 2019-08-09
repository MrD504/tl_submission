#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function
	std::unique_lock<std::mutex> lck( _mutex );
	_condition.wait( lck, [this] { return !_queue.empty(); } );
	T queueItem = std::move( _queue.back() );
	_queue.pop_front();

	return queueItem;
}

template <typename T>
void MessageQueue<T>::send( T &&msg )
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	std::lock_guard<std::mutex> lck( _mutex );
	_queue.push_back( std::move( msg ) );
	_condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{
	std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
	while(true) {
		if(_messages.receive() == TrafficLightPhase::green ) {
			return;
		};
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class
  	threads.emplace_back( std::thread( &TrafficLight::cycleThroughPhases, this ) );
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

	std::srand(std::time(0));
	
	int max_value = 6000, min_value = 4000, range;
	range = max_value - min_value + 1;
	
	int randomDuration = rand() % max_value + min_value;
  	std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  
  	// initiate stop watch
	lastUpdate = std::chrono::system_clock::now();

  	//infinite loop
    while (true) {

      	// wait between cycles
    	std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - lastUpdate ).count();
      
	    // wait for random amount of time between 4 and 6 seconds
		// std::cout << "time since update: " << timeSinceLastUpdate << ", random duration" << randomDuration << std::endl;
		if( timeSinceLastUpdate >= randomDuration ) {
			lastUpdate = std::chrono::system_clock::now();
			randomDuration = rand() % max_value + min_value;
			if( _currentPhase == TrafficLightPhase::red ) {
				_currentPhase = TrafficLightPhase::green;
			} else {
				_currentPhase = TrafficLightPhase::red;
			}
    		// send an update method to the message queue using move semantics
			_messages.send( std::move( _currentPhase ) );
		}
    }
}
