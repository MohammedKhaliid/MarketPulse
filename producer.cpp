#include <random>
#include <string>
#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;


struct SharedData {
    int btypes[100];
    double bprices[100];
    int buffPointer;
    double priceHistory[11][4];
    int historyIndex[11];
    int flag[11];
    double avgHistory[11];
};

union SemaphoreUnion {
    int value;
    struct semid_ds *buffer;
    unsigned short *array;
};


class PricesProducer {
    private:
        std::string name;
        double mean;
        double deviation;
        int sleepInterval;
        int bufferSize;
        normal_distribution<double> normalDistribution;
        default_random_engine engine;

        int sharedMemoryID;
        int mutexSemaphoreID;
        int emptySemaphoreID;
        int fullSemaphoreID;

        SharedData* sharedData;
        int bufferIdx;

        void semaphoreWait(int semId){
            struct sembuf semaphoreOperationBuffer = {0, -1, 0};
            semop(semId, &semaphoreOperationBuffer, 1);
        }

        void semaphoreSignal(int semId){
            struct sembuf semaphoreOperationBuffer = {0, 1, 0};
            semop(semId, &semaphoreOperationBuffer, 1);
        }

        double getPrice(){
            return normalDistribution(engine);
        }

        double calculateAverage(int index)
        {
            double sum = 0;

            for (int i = 0; i < 4; i++)
            {
                sum += sharedData->priceHistory[index][i];
            }
            return sum / 4.0;
        }

        string getCurrentTime(){
            struct timespec time;

            clock_gettime(CLOCK_REALTIME, &time);

            struct tm* tm_info = localtime(&time.tv_sec);
    
            std::stringstream ss;
            ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
            
            return ss.str();
        }

    public:
        PricesProducer(std::string name, double mean, double deviation, int sleepInterval, int bufferSize){
            this->name = name;
            this->mean = mean;
            this->deviation = deviation;
            this->sleepInterval = sleepInterval;
            this->bufferSize = bufferSize;
            normalDistribution = normal_distribution<double>(mean,deviation);
            engine = default_random_engine(static_cast<unsigned int>(std::hash<std::string>()(getCurrentTime())));

            sharedMemoryID = shmget(12345, sizeof(SharedData), 0666);
            if(sharedMemoryID == -1){
                cerr << "Error while producer " << name << " getting shared memory" << endl;
                exit(1);
            }

            sharedData = (SharedData*)shmat(sharedMemoryID, NULL, 0);
            if(sharedData == (void*) -1){
                cerr << "Error while producer " << name << " getting shared memory" << endl;
                exit(1);
            }

            if(name == "ALUMINIUM"){
                bufferIdx = 0;
            }else if(name == "COPPER"){
                bufferIdx = 1;
            }else if(name == "COTTON"){
                bufferIdx = 2;
            }else if(name == "CRUDEOIL"){
                bufferIdx = 3;
            }else if(name == "GOLD"){
                bufferIdx = 4;
            }else if(name == "LEAD"){
                bufferIdx = 5;
            }else if(name == "MENTHAOIL"){
                bufferIdx = 6;
            }else if(name == "NATURALGAS"){
                bufferIdx = 7;
            }else if(name == "NICKEL"){
                bufferIdx = 8;
            }else if(name == "SILVER"){
                bufferIdx = 9;
            }else if(name == "ZINC"){
                bufferIdx = 10;
            }

            mutexSemaphoreID = semget(5678, 1, 0666);
            if(mutexSemaphoreID == -1){
                cerr << "Error while producer " << name << " getting semaphore" << endl;
                exit(1);
            }

            emptySemaphoreID = semget(1567, 1, 0666);
            if(emptySemaphoreID == -1){
                cerr << "Error while producer " << name << " getting semaphore" << endl;
                exit(1);
            }

            fullSemaphoreID = semget(2567, 1, 0666);
            if(fullSemaphoreID == -1){
                cerr << "Error while producer " << name << " getting semaphore" << endl;
                exit(1);
            }
        }

        void run(){
            int value = getPrice();

            cerr << "[" << getCurrentTime() << "] " << name << ": generating a new price " << value << endl;
            cerr << "[" << getCurrentTime()<< "] " << name << ": trying to get mutex on shared buffer" << endl;

            
            semaphoreWait(emptySemaphoreID);
            semaphoreWait(mutexSemaphoreID);

            cerr << "[" << getCurrentTime()<< "] " << name << ": placing " << value << " on shared buffer" << endl;

            sharedData->btypes[sharedData->buffPointer] = bufferIdx;
            sharedData->bprices[sharedData->buffPointer] = value;
            sharedData->buffPointer += 1;
            
            if(sharedData->flag[bufferIdx] == 1){
                sharedData->avgHistory[bufferIdx] = calculateAverage(bufferIdx);
            }

            int idx = sharedData->historyIndex[bufferIdx];
            sharedData->priceHistory[bufferIdx][idx] = value;
            sharedData->historyIndex[bufferIdx] = (idx + 1) % 4;

            idx = sharedData->historyIndex[bufferIdx];
            if(idx == 0 && sharedData->flag[bufferIdx] == 0){
                sharedData->flag[bufferIdx] = 1;
            }

            semaphoreSignal(fullSemaphoreID);          
            semaphoreSignal(mutexSemaphoreID);          

            cerr << "[" << getCurrentTime()<< "] " << name << ": sleeping for " << sleepInterval << " ms" << endl;  
            usleep(sleepInterval * 1000);
        }
        
};

int main(int argc, char* argv[]){
    PricesProducer producer = PricesProducer(argv[1], std::atof(argv[2]), std::atof(argv[3]), std::atoi(argv[4]), std::atoi(argv[5]));
    
    while(true){
        producer.run();
    }

    return 0;
}