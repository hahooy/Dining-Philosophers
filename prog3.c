//cse422 Operating Systems
//Dining Philosophers
//Team members:
//Yilong Hu (hu.yilong@wustl.edu)
//Yongzheng Huang (iamhuangyz@gmail.com)
//Junyuan Suo (jsuo.mail@gmail.com)
//File Purpose: prog3.c defines a series of functions to simulate 
//the dining philosophers without deadlock or starvation
//The program is able to handle n philosophers to eat, where n > 0

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef enum {THINKING, EATING, WAITING} Activity; // identify what the philosopher is doing
typedef enum {START, END} StartEnd; // does the philosopher start or end eating

pthread_mutex_t access_activity; // binary semaphore, needed for implementing the monitor
pthread_cond_t *forksReady; // array of conditions variables for eating
pthread_t *threads; // array of threads simulating the philosophers
Activity *activity; // an array containing the current activity of each philosopher
int N; // number of philosophers
int *philosopherID; // An array of IDs
int *eat_times; // the number of times each philosopher eats

// Displays the title for the state change table
void printTitle()
{
    printf("Eating Activity\n");
    int i;
    // print the first digit, print a space if it is 0
    for(i = 0; i < N; ++i) {
	if (i >= 10) {
	    printf("%d", i / 10);
	} else {
	    printf(" ");
	}
    }
    printf("\n");

    // print the second digit
    for(i = 0; i < N; ++i) {
	printf("%d", i % 10);
    }
    printf("\n");
}

// Displays each philosopher's state
void printActivity(int id, StartEnd a)
{
    int i;
    // print the state of each philosopher
    for(i = 0; i < N; ++i) {
	switch(activity[i]) {
	case THINKING:
	    printf(" ");
	    break;
	case WAITING:
	    printf(" ");
	    break;
	case EATING:
	    printf("*");
	    break;
	default:
	    printf("error");
	}
    }
 
    // print if the philosopher starts to eat or just finished eating
    switch(a){
    case START:
	printf("   %d starts eating", id);
	break;
    case END:
	printf("   %d ends eating", id);
	break;
    default:
	printf("error");
    }
    printf("\n");
    fflush(stdout);
}

// if a philosopher is waiting and its left and right neighbors are not eating,
// change it to eating state and signal it to eat. Otherwise keep it in its current state.
void test(int id)
{
    if(activity[id] == WAITING &&
       activity[(id + 1) % N] != EATING && 
       activity[(id == 0) ? N - 1 : id - 1] != EATING) {
	activity[id] = EATING;
	printActivity(id, START);
	pthread_cond_signal(&forksReady[id]);
    }
}

// A philosopher must acquire the semaphore(fork) before eating
void pickUpForks(int id)
{
    pthread_mutex_lock(&access_activity); // need a mutex to simulate a monitor, aqcuiring this mutex means we enter the monitor
    activity[id] = WAITING; // indicate I am waiting to eat
    // check if its neighbors are eating if left and right
    // neighbors are indeed eating, wait for them to finish
    test(id);
    if (activity[id] == WAITING) {
	pthread_cond_wait(&forksReady[id], &access_activity);
    } 
    pthread_mutex_unlock(&access_activity);
}

// A philosopher must release the semaphore(fork) after eating
void putDownForks(int id)
{
    pthread_mutex_lock(&access_activity); // enter the monitor
    activity[id] = THINKING; // indicate I am thinking
    printActivity(id, END);    
    test((id + 1) % N); // signal right neighbor can eat if it is waiting, move on otherwise
    test((id == 0) ? N - 1 : id - 1); // signal left neighbor can eat if it is waiting, move on otherwise
    pthread_mutex_unlock(&access_activity);
}

// a philosopher spends 2s to 10s thinking
void think()
{
    unsigned int seed = time(0);
    srand(seed);
    usleep(200000 + rand() % 800000);
}

// a philosopher spends 2s to 5s eating
void eat()
{
    unsigned int seed = time(0);
    srand(seed);
    usleep(200000 + rand() % 300000);
}

// check if every philosopher has done thinking 
// and eating at least 3 times
int isDone(void)
{
    int i;
    for(i = 0; i < N; ++i){
	if(eat_times[i] < 3){
	    return 0;
	}
    }
    return 1;
}

// run the philosopher until the program finishes
void *init_phil(void *id)
{
    int *philid = (int *) id;
    while(1){
	if(isDone()) {
	    /* for (int i = 0; i < N; ++i) { */
	    /* 	printf("%d ", eat_times[i]); */
	    /* } */
	    return NULL;
	}
	think();
	pickUpForks(*philid);
	eat();
	eat_times[*philid]++;
	putDownForks(*philid);
    }
}

// create and initiate every philosopher thread, 
// also make sure threads terminate properly
void initThreads()
{
    printTitle();
    int i;
    // create thread for each philosopher
    for(i = 0; i < N; ++i){
	pthread_create(&threads[i], NULL, init_phil, &philosopherID[i]);
    }
    //wait for all philosophers to complete eating
    for(i = 0; i < N; ++i){
	pthread_join(threads[i], NULL);
    }
}

// Allocate space for all arrays and initialize them
void initAllArrays()
{
    activity = (Activity *) malloc(N * sizeof(int));
    forksReady = (pthread_cond_t *) malloc(N * sizeof(pthread_cond_t));
    threads = (pthread_t *) malloc(N * sizeof(pthread_t));
    eat_times = (int *) malloc(N * sizeof(int));
    philosopherID = (int *) malloc(N * sizeof(int));

    int i;
    for(i = 0; i < N; ++i) {
	activity[i] = THINKING;
	pthread_cond_init(&forksReady[i], NULL);
	philosopherID[i] = i;
	eat_times[i] = 0;
    }    
}

// free allocated memory
void freeAllArrays()
{
    free(activity);
    free(forksReady);
    free(threads);
    free(eat_times);
    free(philosopherID);
}

// main function
int main(int argc, char *argv[])
{
    // command line args checking
    if(argc < 2){
	printf("Usage: prog3 n ");
	exit(1);
    }
    N = atoi(argv[1]);
    
    pthread_mutex_init(&access_activity, NULL);  // Initialize the semaphore access_activity
    initAllArrays();
    initThreads(); // initiate all threads
    freeAllArrays();

    return 0;
}
