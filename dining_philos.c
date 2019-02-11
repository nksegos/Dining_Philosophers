#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct phil                         //The backbone of the whole utilization; this struct contains the variables of critical importance to the threads and their mutexes.
{
  int indexer;                             // An integer variable that represents each philosophers index.
	pthread_t thread;                        // A POSIX thread in which a philosopher will be "housed".
	pthread_mutex_t *f_l, *f_r;              // The corresponding forks( POSIX mutexes) for each philosopher: one left and one right.
	int ful;                                 // An integer variable to track the progress of each philosopher's meal.
	double time_total;                       // A variable to track the time the philosopher(thread) has spent on hungry(READY) mode.
	int w8;                                  // An integer variable to track the times the philosopher(thread) has entered the hungry(READY) mode.
} philly;                                   // The struct variable;

int phillies;                              // Declaring the variable to hold the number of philosopher on a global scope.

const char * GetTime()                    // A custom function to get the current timestamp in string format.
{
time_t rawtime;                          // Declaring a time_t type variable.
rawtime = time(NULL);			//Getting current time.

return ctime(&rawtime);			//Returning current time formatted as a string.
}

int next_one(int i)			// A custom function to check the philosopher or fork index to the right of the caller.
{
	int x = (i+1)%phillies;
	if (x==0){x=phillies;}
	return x;
}

int prev_one(int i)			// A custom function to check the philosopher or fork index to the left of the caller.
{
	int x = (i-1)%phillies;
	if (x==0){x=phillies;}
	return x;
}

double rdy_avg(int i,int w,double t)	// A custom function to calculate the average time spent on READY mode.
{
	printf("Philosopher %d has been on READY(HUNGRY) mode for %f seconds on average. \n",i,(t/w) );
	return t/w;
}





void *PhilFunc(void *p)				// A custom pointer function that will serve as the routine function of the thread.
{
	philly *philo = (philly *)p;		// A pointer variable that gets initialized with a typecast of the thread routine argument and will act as the "delegate" between the function and the struct's contents .
	pthread_mutex_t *fork_left, *fork_right;// The 2 mutexes to represent the left and right fork.
	int lfork_grab_succ, rfork_grab_succ;	// 2 integer variables to represent if a mutex acquirement action of each fork has succeded or not.
	int fullness =0;			// An integer variable to track the total execution time.
	time_t start_rdy,end_rdy;		// 2 time variables to track the start and end of the READY(hungry) mode.
	while (fullness <= 20)			// A loop that get executed while a philosopher hasn't eaten for more than 20 seconds.
	{
		printf("Philosopher %d is thinking at %s \n", philo->indexer,GetTime());

		sleep(1+ rand()%6);		// The philosopher spends a random amount of time on the BLOCKED(thinking mode).
		fork_left = philo->f_l;		// Set the left mutex based on the struct content.
		fork_right = philo->f_r;	// Set the right mutex based on the struct content.
		printf("Philosopher %d is hungry at %s  \n", philo->indexer,GetTime());
		start_rdy = time(NULL);		// Gets the start time of the READY(hungry) mode.
		lfork_grab_succ = pthread_mutex_trylock(fork_left);					// A "soft" attempt to acquire the left mutex is made.
    if (lfork_grab_succ != 0)			// If the attempt to acquire the left mutex  is unsuccessful (meaning the success var is set to anything that's not the successful termination value(0)).
    {
	printf("Philosopher %d failed to take fork %d , cause philosopher %d  was using it at %s \n",philo->indexer,philo->indexer,prev_one(philo->indexer), GetTime());		//
      	pthread_mutex_lock(fork_left);		// A "Hard" attempt to acquire the mutes is made, so the thread blocks until the mutex is freed and the thread may attempt to acquire it again.
    }
    rfork_grab_succ = pthread_mutex_trylock(fork_right);	// A "soft" attempt to acquire the right mutex(once the left is secure) is made.
    if (rfork_grab_succ != 0)			// If the attempt to acquire the right mutex is unsuccessful (meaning the success var is set to anything that's not the successful termination value(0)).
    {
      printf("Philosopher %d failed to take fork %d , cause philosopher %d was using it at %s \n",philo->indexer,next_one(philo->indexer),next_one(philo->indexer),GetTime());
      pthread_mutex_lock(fork_right);		// A "Hard" attempt to acquire the mutes is made, so the thread blocks until the mutex is freed and the thread may attempt to acquire it again.
    }
						// The thread enters the critical region.
    		end_rdy = time(NULL);			// Once both mutexes are acquired the READY(hungry) mode is ended.
		philo->time_total += difftime(end_rdy,start_rdy);				// Waiting time is entered to the total waiting time into the struct.
		philo->w8++;			// The struct's times waited variable is incremented by one.
		printf("Philosopher %d is eating at %s  \n",philo->indexer,GetTime());
		sleep(philo->indexer);		// Each philosopher's "execution" time is determined by its time quantum(which is 1 second * N (where N is the philosopher's index) ) .
		fullness += philo->indexer;		// The exection time counter is incremented.
		pthread_mutex_unlock(fork_right);	// Unlock the right mutex since it's the last to be acquired.
		pthread_mutex_unlock(fork_left);	// Unlock the left mutex since it's the first to be acquired.
							// The thread exits the critical region.
 	}
	printf("Philosopher %d is full and is leaving the table  at %s \n",philo->indexer,GetTime());
	philo->ful = 1;				// One the execution has reached(or gone past) 20, the loop is exited and the philosopher's meal is considered over.

return NULL;
}


void master_controller(int N)			// A custom function that controls the programm by initializing mutexes and threads and destroying them when the time is right.
{
	pthread_mutex_t forks[N];		// Declaring an array of mutexes(forks) based on the number of given(by the user) philosophers.
	philly philosophers[N];			// Declaring an array of struct variables that stand as the philosophers based on the given(by the user) number of philosophers.
	philly *philo;				// A pointer variable to act as the "delegate" between the struct contents and the function.
	int i;					// A simple integer variable to be used for loops.


	for (i =0; i< N ; i++)			// Mutex initialization loop.
	{
		pthread_mutex_init(&forks[i],NULL);
	}
	for (i =0; i< N; i++)			// Philosopher struct initialization and thread creation loop.
	{
		philo = &philosophers[i];			// Sets for which philosopher will the var "delegate" for.
		philo->time_total=0.0;				// Variable initialized.
		philo->w8=0;					// Variable initialized.
		philo->ful = 0;					// Variable initialized.
		philo->indexer = i+1;				// Philosopher index set.
		philo->f_l = &forks[i];				// Left fork(mutes) assigned.
		philo->f_r = &forks[(i+1)%N];			// Right fork(mutex) assigned.
		pthread_create(&philo->thread,NULL,PhilFunc,philo);		// Philosopher thread created with the PhilFunc as the routine and the philo  "delegate" as the argument to it.
	}
	int running = 1;			// A variable to signal that the threads and mutexes  are created and running.
	while (running)				// While the threads and mutexes are running.
	{
		int g=0;			// A simple counting variable.
		for (i=0; i<N; i++)		//
		{
			philo = &philosophers[i];				// Set for which philosopher will the var "delegate" for.
			if(philo->ful){g +=1;}					// If the philosopher has ended his meal the counting var is incremented by 1 .
		}
		if (g == N)			// If all philosophers have ended their meals.
		{
			for (i=0; i<N; i++)					// Thread destruction loop
			{
				philo = &philosophers[i];
				pthread_join(philo->thread,NULL);
			}
			printf("All philosophers are full now \n");
			double all_phil_avg;					// The READY(hungry) mode average time for all philosophers.
			for (i=0; i<N; i++)					// Average time computation and mutex destruction.
			{
				pthread_mutex_destroy(&forks[i]);
				philo = &philosophers[i];
				all_phil_avg +=rdy_avg(philo->indexer,philo->w8,philo->time_total);		// Each philosopher's average time is computed and added to the total average.
			}
			printf("The average time spent on READY(HUNGRY) mode for all philosophers is %f seconds. \n",(all_phil_avg/N));			// The total average var is divided with the number of the philosophers and gets printed.

			running = 0;		// All threads and mutexes are destroyed.
		}
	}




}



int main()					// The main function.
{
	printf("___THE DINING PHILOSOPHERS___  \n");
	for(;;)						// Infinite loop.
	{
		printf("Enter the number of the philosophers(Warning! The number must be between 3 and 10 : ");
		scanf("%d",&phillies);			// Get the number of the philosophers from the user.
		if ((phillies <=10) && (phillies >=3)){break;}		// If the number criteria is met, the loop breaks.
	}

	master_controller(phillies);			// Call the thread and mutex controlling function.

	return 0;
}
