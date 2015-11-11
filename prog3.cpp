//cse422 Operating Systems
//Dining Philosophers
//Team members:
//Yilong Hu (hu.yilong@wustl.edu)
//Yongzheng Huang (iamhuangyz@gmail.com)
//Junyuan Suo (jsuo.mail@gmail.com)
//File Purpose: prog3.cpp defines a series of functions to simulate 
//the dining philosophers without deadlock or starvation
//The program is able to handle n philosophers to eat, where 0 < n <= 15

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

using namespace std;

// philosopher states
#define THINKING 0
#define HUNGRY 1
#define EATING 2


int num_phils; // number of philosophers
int *phil_state; // an array containing the current state of each philosopher
pthread_mutex_t access_activity; // semaphore lock
pthread_cond_t *safe_to_eat; // array of conditions for eating
pthread_t *phil_threads; // array of threads simulating the philosophers
int *philosopherID; // An array of IDs
static int *eat_times; // the number of times each philosopher eats

// Displays the title for the state change table
void display_title(){
    printf("Eating Activity\n");

    for(int phil = 0; phil < num_phils; phil++) {
	if(phil >= 10) cout << "1";
	else cout << " ";
    }
    cout << endl;
    for(int phil = 0; phil < num_phils; phil++) {
	if(phil >= 10) cout << phil-10;
	else cout << phil;
    }
    cout << endl;
}

// Gets left philosopher's ID
int get_left_neighbor_id(int phil){
    return phil == num_phils - 1 ? 0 : phil + 1;
}

// Gets right philosopher's ID
int get_right_neighbor_id( int phil ){
    return phil == 0 ? num_phils - 1 : phil - 1;
}

// Displays each philosopher's state
void display_states(int phil, int event){
    for(int phil = 0; phil < num_phils; phil++){
	cout << left;
	switch(phil_state[phil]) {
	case THINKING: cout << " "; break;
	case HUNGRY: cout << " "; break;
	case EATING: cout << "*"; break;
	default: cout << "CONFUSED";
	}
    }
    switch(event){
    case -1:
	cout << "   " << phil <<  " starts eating";
	break;
    case -2:
	cout << "   " << phil << " ends eating";
	break;
    default:;
    }
    cout << endl;
    cout.flush( );
}

// return true if philosopher is HUNGARY and its 
// left and right neighbors are not eating
bool test(int phil){
    if(phil_state[phil] == 1){
	if(phil_state[get_left_neighbor_id(phil)] != 2 && 
	   phil_state[get_right_neighbor_id(phil)] != 2){
	    phil_state[phil] = EATING;
	    display_states(phil, -1);
	    return true;
	}else{
	    return false;
	}
    }else{
	return false;
    }
}

// A philosopher must acquire the semaphore(fork) before eating
void grab_forks(int phil){
    pthread_mutex_lock(&access_activity);
    phil_state[phil] = HUNGRY;
    // check if its neighbors are eating
    // if left and right neighbors are indeed eating, wait 
    // for them to finish
    if (!test(phil)){
	pthread_cond_wait(&safe_to_eat[phil], &access_activity);
    } 
    pthread_mutex_unlock(&access_activity);
}

// A philosopher must release the semaphore(fork) after eating
void release_forks(int phil){
    pthread_mutex_lock(&access_activity);
    phil_state[phil] = THINKING;
    display_states(phil, -2);
    // signal left neighbor can eat
    if (test(get_left_neighbor_id(phil))){
	pthread_cond_signal(&safe_to_eat[get_left_neighbor_id(phil)]);
    }
    // signal right neighbor can eat
    if (test(get_right_neighbor_id(phil))){
	pthread_cond_signal(&safe_to_eat[get_right_neighbor_id(phil)]);
    }
    pthread_mutex_unlock(&access_activity);
}

// a philosopher spends 2s to 10s thinking
void think(){
    unsigned int seed = time(0);
    srand(seed);
    usleep(200000 + rand() % 800000);
}

// a philosopher spends 2s to 5s eating
void eat(){
    unsigned int seed = time(0);
    srand(seed);
    usleep(200000 + rand() % 300000);
}

// check if every philosopher has done thinking 
// and eating at least 3 times
bool isDone(void){
    for(int i=0; i<num_phils; i++){
	if(eat_times[i] < 3){
	    return false;
	}
    }
    return true;
}

// stimulate each philosopher's behavior
void *init_phil(void *id){
    int *philid = (int *) id;
    while(1){
	if(isDone()) return NULL;
	think();
	grab_forks(*philid);
	eat();
	eat_times[*philid]++;
	release_forks(*philid);
    }
}

// create and initiate every philosopher thread, 
// also make sure threads terminate properly
void run(){
    display_title();
    // create thread for each philosopher
    for(int i = 0; i < num_phils; i++){
	pthread_create(&phil_threads[i], NULL, init_phil, &philosopherID[i]);
    }
    //wait for all philosophers to complete eating
    for(int j = 0; j < num_phils; j++){
	pthread_join(phil_threads[j], NULL);
    }
}

// main function
int main(int argc, char *argv[]){
    // command line args checking
    if(argc < 2){
	cout << "Usage: prog3 num_phils " << endl;
	exit(1);
    }
    num_phils = atoi(argv[1]);

    // Initialize the semaphore access_activity
    pthread_mutex_init(&access_activity, NULL);
  
    // Create all of the arrays
    phil_state = new int[num_phils];
    safe_to_eat = new pthread_cond_t[num_phils];
    phil_threads = new pthread_t[num_phils];
    eat_times = new int[num_phils];
    philosopherID = new int[num_phils];

    // Initialize all the arrays
    for(int phil = 0; phil < num_phils; phil++){
	phil_state[phil] = THINKING;
	pthread_cond_init(&safe_to_eat[phil], NULL);
	philosopherID[phil] = phil;
	eat_times[phil] = 0;
    }
    run();
    return 0;
}
