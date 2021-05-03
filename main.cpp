/*
 * This is an example of how a pool of threads can be used to
 * execute arbitrary funciton calls for a general purpose job
 * system.
 *
 * Author: Zach Thompson
 */
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdlib.h>


//Some global variables to keep track of state.
//These would be contained in a job-system class.


//This variable keeps the worker threads alive
bool running = true;

//This is the pool of worker threads
std::vector<std::thread> threads;

//These locks help orchestrate the multi-threaded
//operations.
std::mutex queue_lock;
std::mutex output_lock;


//The work queue that the worker threads pull
//their work units from
std::vector<work_unit*> work_queue;




//To allow for fully anonymous function passing, this
//generic class is always passed to a function that should
//be called in a worker thread. The function passed in the
//work_unit should know how to cast the generic to get
//the data it needs.
class generic{
    public:
    virtual ~generic(){}
};

//An example of deriving a class from generic to allow a
//string to be passed to the job system.
class hello_string: public generic{
    public:
    std::string data;
    ~hello_string(){}
};


//The work_unit is the structure that the worker threads
//operate on. It contains the function that the worker
//thread should execute and the generic to pass to the
//function.
class work_unit{
    public:
    void(*func)(generic*);
    generic* g;
    ~work_unit(){
        if(g != nullptr){ delete g; }
    }
};


//The function that all worker threads will run. The thread
//will run in a loop while the global variable running is true.
//It will try to acquire the queue_lock and collect a work_unit
//from the queue. If there are no work_units in the queue, it
//will release the lock and go to sleep to allow other processes
//to gain the lock. If it does acquire a work_unit from the
//queue it will release the lock and then run the function in
//the work_unit.
void thread_work(int thread_id){
    while(running){
        queue_lock.lock();
        work_unit* w;
        if(work_queue.size() > 0){
            w = work_queue.back();
            work_queue.pop_back();
            queue_lock.unlock();
            w->func(w->g);
            delete w;
        }
        else{
            queue_lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    }
}




////
//// func_1 through func_4 all basically do the same thing, but
//// represent potentially different functions that may need to
//// be run in a real application.
////
void func_1(generic* gen){
    output_lock.lock();
    std::cout << "Function 1 doing work" << std::endl;

    if(gen != nullptr){
        hello_string* string_data = (hello_string*)gen;
        std::cout << string_data->data << std::endl;
    }
    output_lock.unlock();
}


void func_2(generic* gen){
    output_lock.lock();
    std::cout << "Function 2 doing work" << std::endl;
    if(gen != nullptr){
        hello_string* string_data = (hello_string*)gen;
        std::cout << string_data->data << std::endl;
    }
    output_lock.unlock();
}


void func_3(generic* gen){
    output_lock.lock();
    std::cout << "Function 3 doing work" << std::endl;
    if(gen != nullptr){
        hello_string* string_data = (hello_string*)gen;
        std::cout << string_data->data << std::endl;
    }
    output_lock.unlock();
}

void func_4(generic* gen){
    output_lock.lock();
    std::cout << "Function 4 doing work" << std::endl;
    if(gen != nullptr){
        hello_string* string_data = (hello_string*)gen;
        std::cout << string_data->data << std::endl;
    }
    output_lock.unlock();
}



//This is an example of passing a function signature directly to
//another function. Originally this system was just to test passing
//functions somewhat anonymously to be called by another function.
//The next logical step was to create a multi-threaded job system
//based on this original test.
//void call_anonymous_function(void (*v)(generic*), generic* g){
//    std::cout << "Invoking function: " << std::endl;
//    v(g);
//}



int main(int argc, char** argv){


    //Create the pool of worker threads
    for(int i = 0; i < 10; ++i){
        threads.push_back(std::thread(thread_work, i));
    }



    //Lock the work queue so we can work on it
    queue_lock.lock();


    //Fill the work queue with some random data
    //Randomize which functions are put onto the queue
    //to simulate some different work
    for(int i = 0; i < 50; ++i){
        std::string message = "Hello " + std::to_string(i);
        work_unit* u = new work_unit;
        hello_string* hs = new hello_string;
        hs->data = message;
        int func = rand() % 4 + 1;


        u->g = hs;//nullptr;
        switch(func){
            case 1: u->func = func_1;
                    break;
            case 2: u->func = func_2;
                    break;
            case 3: u->func = func_3;
                    break;
            case 4: u->func = func_4;
                    break;
        }
        //u->func = func_1;
        work_queue.push_back(u);

    }

    //As soon as the lock is open, the worker threads will
    //start to process the data
    queue_lock.unlock();


    //Put the main thread to sleep so the workers can process the queue
    std::cout << "Putting main thread to sleep for 2 seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));


    //Then stop the workers
    running = false;


    //And join all the threads to clean up
    for (std::thread &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    return 0;

}
