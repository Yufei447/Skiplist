/* 
 * Test the number of requests the project can handle per second under random read/write 
*/
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "../skiplist.h"

#define NUM_THREADS 4
#define TEST_COUNT 1000000
SkipList<int, std::string> skipList(20);

void * insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    
    // Compute the number of elements inserted per thread
    int tmp = TEST_COUNT/NUM_THREADS; 

	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
        // Randomly generate key and insert value with "a"
		skipList.insert_element(rand() % TEST_COUNT, "a"); 
	}
    pthread_exit(NULL);
}

void * getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    std::cout << tid << std::endl;  
    
    int tmp = TEST_COUNT/NUM_THREADS; 
	for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

int main() {
    // Initialize random number generator
    srand (time(NULL));  
    // {

    //     pthread_t threads[NUM_THREADS];
    //     // Return value for creating thread
    //     int rc;
    //     int i;
    //     // Start timing
    //     auto start = std::chrono::high_resolution_clock::now();

    //     for(i = 0; i < NUM_THREADS; i++) {
    //         std::cout << "main() : creating thread, " << i << std::endl;
    //         rc = pthread_create(&threads[i], NULL, insertElement, (void *)i);

    //         if (rc) {
    //             std::cout << "Error: unable to create thread," << rc << std::endl;
    //             exit(-1);
    //         }
    //     }

    //     void *ret;
    //     // Wait for all threads complete
    //     for(i = 0; i < NUM_THREADS; i++) {
    //         if (pthread_join(threads[i], &ret) != 0) {
    //             perror("Error: unable to join"); 
    //             exit(3);
    //         }
    //     }
    //     // Stop timing
    //     auto finish = std::chrono::high_resolution_clock::now(); 
    //     // Calculate time spent
    //     std::chrono::duration<double> elapsed = finish - start;
    //     std::cout << "Insert elapsed:" << elapsed.count() << std::endl;
    // }
    
    // skipList.displayList();

    {
        pthread_t threads[NUM_THREADS];
        int rc;
        int i;
        auto start = std::chrono::high_resolution_clock::now();

        for(i = 0; i < NUM_THREADS; i++) {
            std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

            if (rc) {
                std::cout << "Error: unable to create thread," << rc << std::endl;
                exit(-1);
             }
        }

        void *ret;
        for(i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "get elapsed:" << elapsed.count() << std::endl;
    }

	pthread_exit(NULL);
    return 0;

}
