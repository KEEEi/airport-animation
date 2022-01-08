#include "AirportAnimator.hpp"

#include <pthread.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

using namespace std;

//define plane and passengers
#define PLANES 8
#define PASSENGERS 12

//declare unsigned integer variables (not negative num)
static unsigned int passengersNum;//how many people are available for boarding
static unsigned int toursNum;//how many tour 
static unsigned int toursStarted;//starte tour
static unsigned int toursCompleted;//completed tour

//declare pthread_mutex_t variables
static pthread_mutex_t passengersNum_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t toursNum_mutex      = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t runway_mutex        = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t passengersNum_cond = PTHREAD_COND_INITIALIZER;


//Borarding passenger function
static void boardPassenger(unsigned int planeNum, unsigned int passenger){
	
	//lock mutex object
	pthread_mutex_lock(&passengersNum_mutex);

	//Just wait if there is no passenger
	while (passengersNum == 0){
		  pthread_cond_wait(&passengersNum_cond, &passengersNum_mutex);
	}

	passengersNum--;

	//unlock mutex object
	pthread_mutex_unlock(&passengersNum_mutex);
	
	//change #passengers on board plane [plane_num] to be [numpassengers] 
	//stop a process 
	AirportAnimator::updatePassengers(planeNum, passenger);
	sleep(rand() % 3);
}

//Deplane passenger function
static void deplanePassenger(unsigned int planeNum, unsigned int passenger){
	
	//lock mutex object
	pthread_mutex_lock(&passengersNum_mutex);
	passengersNum++;

	//unlock a thread's block
	pthread_cond_signal(&passengersNum_cond);
	
	//unlock mutex object
	pthread_mutex_unlock(&passengersNum_mutex);

	//change #passengers on board plane [plane_num] to be [numpassengers] 
	//stop a process 
	AirportAnimator::updatePassengers(planeNum, passenger);
	sleep(1);
}

//Tour function
static int shouldStartNewTour(void){
	int result = 1;
	
	//lock mutex object
	pthread_mutex_lock(&toursNum_mutex);
	//if number of tourStarted and completed tour are equal to tour number, result is zero, otherwise add 1 to a tourStarted
	if (toursStarted + toursCompleted == toursNum){
		result = 0;
	}else{
		toursStarted++;
	}

	//unlock mutex object
	pthread_mutex_unlock(&toursNum_mutex);
	
	return result;
}

//plane handler function 
static void* planeHandler(void* ptr){
	const int planeNum = reinterpret_cast<intptr_t>(ptr);
	int done = 0;
	
	//while a new tour should be started, board a passenger
	while (shouldStartNewTour()){
		for (int passenger = 1; passenger <= PASSENGERS; passenger++){
			boardPassenger(planeNum, passenger);
		}

		//change the status message for plane number [plane_num] to be [status]
		//draw plane number [plane_num] taxi-ing from the airport to the runway.
		AirportAnimator::updateStatus(planeNum, "TAXI");
		AirportAnimator::taxiOut(planeNum);
		
		//lock mutex object
		pthread_mutex_lock(&runway_mutex);

		//change the status message for plane number [plane_num] to be [status]
		//draw plane number [plane_num] taking off.
		AirportAnimator::updateStatus(planeNum, "TKOFF");
		AirportAnimator::takeoff(planeNum);
		
		//unlock mutex object
		pthread_mutex_unlock(&runway_mutex);

		//change the status message for plane number [plane_num] to be [status]
		//stop a process 
		AirportAnimator::updateStatus(planeNum,"TOUR");
		sleep(15 + rand() % 31);

		//change the status message for plane number [plane_num] to be [status]
		AirportAnimator::updateStatus(planeNum,"LNDRQ");
		
		//lock mutex object
		pthread_mutex_lock(&runway_mutex);

		//change the status message for plane number [plane_num] to be [status]
		//draw plane number [plane_num] landing.
		AirportAnimator::updateStatus(planeNum,"LAND");
		AirportAnimator::land(planeNum);
		
		//unlock mutex object
		pthread_mutex_unlock(&runway_mutex);

		//change the status message for plane number [plane_num] to be [status]
		//draw plane number [plane_num] taxiing from the runway to the airport. 
		AirportAnimator::updateStatus(planeNum, "TAXI");
		AirportAnimator::taxiIn(planeNum);

		//change the status message for plane number [plane_num] to be [status]
		AirportAnimator::updateStatus(planeNum, "DEPLN");
		
		//deplane passenger
		for(int passenger = 11; passenger >= 0; passenger--){
			deplanePassenger(planeNum, passenger);
		}

		//change the status message for plane number [plane_num] to be [status]
		AirportAnimator::updateStatus(planeNum, "DEPLN");

		//lock mutex object
		pthread_mutex_lock(&toursNum_mutex);
		--toursStarted;
		
		//print the total number of tours [num_tours] to the screen
		AirportAnimator::updateTours(++toursCompleted);
		
		//unlock mutex object
		pthread_mutex_unlock(&toursNum_mutex);
	}
	
	//finish thread
	pthread_exit(NULL);
}

//main function
int main(int argc, char *argv[]){
	//User input
	if (argc != 3){
		cerr << "Usage: " << argv[0] << " <passengers> <tours>" << endl;
		return EXIT_FAILURE;
	}

	//Declare passenger and tour numbers
	passengersNum = atoi(argv[1]);
	toursNum = atoi(argv[2]);

	//make a randum number
	srand(time(NULL));

	//initializes screen and draws airport, taxiways, and runway
	AirportAnimator::init();

	//Thread id
	pthread_t threads[PLANES];

	//create a threds
	for (int i = 0; i < PLANES; ++i){
		pthread_create(&threads[i], NULL, planeHandler, reinterpret_cast<void*>(i));
	}

	// wait for threads to finish
	for (int i = 0; i < PLANES; ++i){
		pthread_join(threads[i], NULL);
	}

	//finish airport animator
	AirportAnimator::end();
	
	//finish with success
	return EXIT_SUCCESS;
}
