#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <errno.h>
#include <string.h>

using namespace std;

struct SharedData
{
    int btypes[100];
    double bprices[100];
    int buffPointer;
    double priceHistory[11][4];
    int historyIndex[11];
    int flag[11];
    double avgHistory[11];
};

union SemaphoreUnion
{
    int value;
    struct semid_ds *buffer;
    unsigned short *array;
};

class Consumer
{
private:
    int bufferSize;

    int sharedMemoryID;

    int mutexSemaphoreID;
    int emptySemaphoreID;
    int fullSempahoreID;

    SharedData *sharedData;

    void semaphoreWait(int semId)
    {
        struct sembuf semaphoreOperationBuffer = {0, -1, 0};
        semop(semId, &semaphoreOperationBuffer, 1);
    }

    void semaphoreSignal(int semId)
    {
        struct sembuf semaphoreOperationBuffer = {0, 1, 0};
        semop(semId, &semaphoreOperationBuffer, 1);
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

public:
    Consumer(int size)
    {

        // cerr << size;

        bufferSize = size;

        sharedMemoryID = shmget(12345, sizeof(SharedData), IPC_CREAT | 0666);
        if (sharedMemoryID == -1)
        {
            cerr << "Error in intialising shared memory " << strerror(errno) << endl;
            exit(1);
        }

        sharedData = (SharedData *)shmat(sharedMemoryID, NULL, 0);
        if (sharedData == (void *)-1)
        {
            cerr << "Error while consumer is attaching to shared memory" << endl;
            exit(1);
        }

        sharedData->buffPointer = 0;
        for (int i = 0; i < 11; i++)
        {
            sharedData->historyIndex[i] = 0;
            sharedData->avgHistory[i] = 0.00;
            for (int j = 0; j < 4; j++)
            {
                sharedData->priceHistory[i][j] = 0;
            }
            sharedData->flag[i] = 0;
        }

        mutexSemaphoreID = semget(5678, 1, IPC_CREAT | 0666);
        if (mutexSemaphoreID == -1)
        {
            cerr << "Error in creating semaphore " << strerror(errno) << endl;
            exit(1);
        }

        union SemaphoreUnion mutexSemUnion;
        mutexSemUnion.value = 1;
        int semaphoreSetValue = semctl(mutexSemaphoreID, 0, SETVAL, mutexSemUnion);
        if (semaphoreSetValue == -1)
        {
            cerr << "Error in initialising semaphore " << strerror(errno) << endl;
            exit(1);
        }

        emptySemaphoreID = semget(1567, 1, IPC_CREAT | 0666);
        if (emptySemaphoreID == -1)
        {
            cerr << "Error in creating semaphore " << strerror(errno) << endl;
            exit(1);
        }

        union SemaphoreUnion emptySemUnion;
        emptySemUnion.value = size;
        semaphoreSetValue = semctl(emptySemaphoreID, 0, SETVAL, emptySemUnion);
        if (semaphoreSetValue == -1)
        {
            cerr << "Error in initialising semaphore " << strerror(errno) << endl;
            exit(1);
        }

        fullSempahoreID = semget(2567, 1, IPC_CREAT | 0666);
        if (fullSempahoreID == -1)
        {
            cerr << "Error in creating semaphore " << strerror(errno) << endl;
            exit(1);
        }

        union SemaphoreUnion fullSemUnion;
        fullSemUnion.value = 0;
        semaphoreSetValue = semctl(fullSempahoreID, 0, SETVAL, fullSemUnion);
        if (semaphoreSetValue == -1)
        {
            cerr << "Error in initialising semaphore " << strerror(errno) << endl;
            exit(1);
        }
    }

    void printTable()
    {

        semaphoreWait(fullSempahoreID);
        semaphoreWait(mutexSemaphoreID);

        double itemValue = sharedData->bprices[sharedData->buffPointer - 1];
        sharedData->buffPointer -= 1;

        int bufferIdx;
        system("clear");

        // cout << "\e[1;1H\e[2J";
        // cout << "\033[1;1H";

        cout.flush();

        cout << "+---------------+-----------+-----------+" << endl;
        cout << "| Currency      | Price     | AvgPrice  |" << endl;
        cout << "+---------------+-----------+-----------+" << endl;

        if (sharedData->flag[0] == 0)
        {
            cout << "| ALUMINIUM     |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[0][(sharedData->historyIndex[0] - 2 + 4) % 4] < sharedData->priceHistory[0][(sharedData->historyIndex[0] - 1 + 4) % 4]) // last price < current price
                cout << "| ALUMINIUM     |" << setw(7) << "\033[;32m" << sharedData->priceHistory[0][(sharedData->historyIndex[0] - 1 + 4) % 4] << "↑\033[0m" << "     |";

            else if(sharedData->priceHistory[0][(sharedData->historyIndex[0] - 2 + 4) % 4] > sharedData->priceHistory[0][(sharedData->historyIndex[0] - 1 + 4) % 4])
                cout << "| ALUMINIUM     |" << setw(7) << "\033[;31m" << sharedData->priceHistory[0][(sharedData->historyIndex[0] - 1 + 4) % 4] << "↓\033[0m" << "     |";
                
            else
                cout << "| ALUMINIUM     |" << setw(7) << sharedData->priceHistory[0][(sharedData->historyIndex[0] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(0);
            if (sharedData->avgHistory[0] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;
        }

        if (!sharedData->flag[1])
        {
            cout << "| COPPER        |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[1][(sharedData->historyIndex[1] - 2 + 4) % 4] < sharedData->priceHistory[1][(sharedData->historyIndex[1] - 1 + 4) % 4]) // last price < current price
                cout << "| COPPER        |" << setw(7) << "\033[;32m" << sharedData->priceHistory[1][(sharedData->historyIndex[1] - 1 + 4) % 4] << "↑\033[0m" << "     |";

            else if(sharedData->priceHistory[1][(sharedData->historyIndex[1] - 2 + 4) % 4] > sharedData->priceHistory[1][(sharedData->historyIndex[1] - 1 + 4) % 4])
                cout << "| COPPER        |" << setw(7) << "\033[;31m" << sharedData->priceHistory[1][(sharedData->historyIndex[1] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| COPPER        |" << setw(7) << sharedData->priceHistory[1][(sharedData->historyIndex[1] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(1);
            if (sharedData->avgHistory[1] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[2])
        {
            cout << "| COTTON        |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[2][(sharedData->historyIndex[2] - 2 + 4) % 4] < sharedData->priceHistory[2][(sharedData->historyIndex[2] - 1 + 4) % 4]) // last price < current price
                cout << "| COTTON        |" << setw(7) << "\033[;32m" << sharedData->priceHistory[2][(sharedData->historyIndex[2] - 1 + 4) % 4] << "↑\033[0m" << "     |";
           
            else if(sharedData->priceHistory[1][(sharedData->historyIndex[2] - 2 + 4) % 4] > sharedData->priceHistory[2][(sharedData->historyIndex[2] - 1 + 4) % 4])
                cout << "| COTTON        |" << setw(7) << "\033[;31m" << sharedData->priceHistory[2][(sharedData->historyIndex[2] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| COTTON        |" << setw(7) << sharedData->priceHistory[2][(sharedData->historyIndex[2] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(2);
            if (sharedData->avgHistory[2] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[3])
        {
            cout << "| CRUDEOIL      |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[3][(sharedData->historyIndex[3] - 2 + 4) % 4] < sharedData->priceHistory[3][(sharedData->historyIndex[3] - 1 + 4) % 4]) // last price < current price
                cout << "| CRUDEOIL      |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[3][(sharedData->historyIndex[3] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[3][(sharedData->historyIndex[3] - 2 + 4) % 4] > sharedData->priceHistory[3][(sharedData->historyIndex[3] - 1 + 4) % 4])
                cout << "| CRUDEOIL      |"<< setw(7) << "\033[;31m" << sharedData->priceHistory[3][(sharedData->historyIndex[3] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| CRUDEOIL      |"<< setw(7) << sharedData->priceHistory[3][(sharedData->historyIndex[3] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(3);
            if (sharedData->avgHistory[3] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[4])
        {
            cout << "| GOLD          |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[4][(sharedData->historyIndex[4] - 2 + 4) % 4] < sharedData->priceHistory[4][(sharedData->historyIndex[4] - 1 + 4) % 4]) // last price < current price
                cout << "| GOLD          |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[4][(sharedData->historyIndex[4] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[4][(sharedData->historyIndex[4] - 2 + 4) % 4] > sharedData->priceHistory[4][(sharedData->historyIndex[4] - 1 + 4) % 4])
                cout << "| GOLD          |" << setw(7) << "\033[;31m" << sharedData->priceHistory[4][(sharedData->historyIndex[4] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| GOLD          |" << setw(7) << sharedData->priceHistory[4][(sharedData->historyIndex[4] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(4);
            if (sharedData->avgHistory[4] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[5])
        {
            cout << "| LEAD          |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[5][(sharedData->historyIndex[5] - 2 + 4) % 4] < sharedData->priceHistory[5][(sharedData->historyIndex[5] - 1 + 4) % 4]) // last price < current price
                cout << "| LEAD          |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[5][(sharedData->historyIndex[5] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[5][(sharedData->historyIndex[5] - 2 + 4) % 4] > sharedData->priceHistory[5][(sharedData->historyIndex[5] - 1 + 4) % 4])
                cout << "| LEAD          |" << setw(7) << "\033[;31m" << sharedData->priceHistory[5][(sharedData->historyIndex[5] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| LEAD          |" << setw(7) << sharedData->priceHistory[5][(sharedData->historyIndex[5] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(5);
            if (sharedData->avgHistory[5] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[6])
        {
            cout << "| MENTHAOIL     |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[6][(sharedData->historyIndex[6] - 2 + 4) % 4] < sharedData->priceHistory[6][(sharedData->historyIndex[6] - 1 + 4) % 4]) // last price < current price
                cout << "| MENTHAOIL     |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[6][(sharedData->historyIndex[6] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[6][(sharedData->historyIndex[6] - 2 + 4) % 4] > sharedData->priceHistory[6][(sharedData->historyIndex[6] - 1 + 4) % 4])
                cout << "| MENTHAOIL     |" << setw(7) << "\033[;31m" << sharedData->priceHistory[6][(sharedData->historyIndex[6] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| MENTHAOIL     |" << setw(7) << sharedData->priceHistory[6][(sharedData->historyIndex[6] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(6);
            if (sharedData->avgHistory[6] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[7])
        {
            cout << "| NATURALGAS    |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[7][(sharedData->historyIndex[7] - 2 + 4) % 4] < sharedData->priceHistory[7][(sharedData->historyIndex[7] - 1 + 4) % 4]) // last price < current price
                cout << "| NATURALGAS    |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[7][(sharedData->historyIndex[7] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[7][(sharedData->historyIndex[7] - 2 + 4) % 4] > sharedData->priceHistory[7][(sharedData->historyIndex[7] - 1 + 4) % 4])
                cout << "| NATURALGAS    |" << setw(7) << "\033[;31m" << sharedData->priceHistory[7][(sharedData->historyIndex[7] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| NATURALGAS    |" << setw(7) << sharedData->priceHistory[7][(sharedData->historyIndex[7] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(7);
            if (sharedData->avgHistory[7] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[8])
        {
            cout << "| NICKEL        |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[8][(sharedData->historyIndex[8] - 2 + 4) % 4] < sharedData->priceHistory[8][(sharedData->historyIndex[8] - 1 + 4) % 4]) // last price < current price
                cout << "| NICKEL        |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[8][(sharedData->historyIndex[8] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[8][(sharedData->historyIndex[8] - 2 + 4) % 4] > sharedData->priceHistory[8][(sharedData->historyIndex[8] - 1 + 4) % 4])
                cout << "| NICKEL        |" << setw(7) << "\033[;31m" << sharedData->priceHistory[8][(sharedData->historyIndex[8] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| NICKEL        |" << setw(7) << sharedData->priceHistory[8][(sharedData->historyIndex[8] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(8);
            if (sharedData->avgHistory[8] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[9])
        {
            cout << "| SILVER        |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[9][(sharedData->historyIndex[9] - 2 + 4) % 4] < sharedData->priceHistory[9][(sharedData->historyIndex[9] - 1 + 4) % 4]) // last price < current price
                cout << "| SILVER        |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[9][(sharedData->historyIndex[9] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[9][(sharedData->historyIndex[9] - 2 + 4) % 4] > sharedData->priceHistory[9][(sharedData->historyIndex[9] - 1 + 4) % 4])
                cout << "| SILVER        |" << setw(7) << "\033[;31m" << sharedData->priceHistory[9][(sharedData->historyIndex[9] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| SILVER        |" << setw(7) << sharedData->priceHistory[9][(sharedData->historyIndex[9] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(9);
            if (sharedData->avgHistory[9] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }

        if (!sharedData->flag[10])
        {
            cout << "| ZINC          |" << setw(7) << "0.00" << "    | " << setw(7) << "0.00" << "   |" << endl;
        }
        else
        {
            if (sharedData->priceHistory[10][(sharedData->historyIndex[10] - 2 + 4) % 4] < sharedData->priceHistory[10][(sharedData->historyIndex[10] - 1 + 4) % 4]) // last price < current price
                cout << "| ZINC          |"  << setw(7) << "\033[;32m" << sharedData->priceHistory[10][(sharedData->historyIndex[10] - 1 + 4) % 4] << "↑\033[0m" << "     |";
            
            else if(sharedData->priceHistory[10][(sharedData->historyIndex[10] - 2 + 4) % 4] > sharedData->priceHistory[10][(sharedData->historyIndex[10] - 1 + 4) % 4])
                cout << "| ZINC          |" << setw(7) << "\033[;31m" << sharedData->priceHistory[10][(sharedData->historyIndex[10] - 1 + 4) % 4] << "↓\033[0m" << "     |";

            else
                cout << "| ZINC          |" << setw(7) << sharedData->priceHistory[10][(sharedData->historyIndex[10] - 1 + 4) % 4] << "      |";

            double avg = calculateAverage(10);
            if (sharedData->avgHistory[10] < avg)
                cout << setw(7) << "\033[;32m" << avg << "↑\033[0m\t|" << endl;

            else
                cout << setw(7) << "\033[;31m" << avg << "↓\033[0m\t|" << endl;

        }
        cout << "+---------------+-----------+-----------+" << endl;

        cout.flush();

        semaphoreSignal(emptySemaphoreID);
        semaphoreSignal(mutexSemaphoreID);
    }
};

int main(int argc, char *argv[])
{
    Consumer consumer = Consumer(atoi(argv[1]));

    while (true)
    {
        consumer.printTable();
    }

    return 0;
}