#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GRID_SIZE 20
#define NUM_THREADS 4
#define GENERATIONS 32

int grid[GRID_SIZE][GRID_SIZE];
pthread_barrier_t barrier;


void print_grid() {
    system("clear"); 
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                printf("# ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    usleep(500000); 
}

// Function to compute next generation of Game of Life
void* compute_next_gen(void* arg) {
  
    int row_start = *(int *) arg;
    int row_end = row_start + 5;

    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    int live_neighbors[5][20];

    for(int i = row_start ; i < row_end ; i++){

        for(int j = 0 ; j < 20 ; j++){
            
            live_neighbors[i-row_start][j] = 0;
            for(int k = 0 ; k < 8 ; k++){
                if(i + dx[k] > 19 || i + dx[k] < 0 || j + dy[k] > 19 || j + dy[k] < 0) //index out of bound check
                    continue;

                live_neighbors[i-row_start][j] += grid[i+dx[k]][j+dy[k]];
            }
        }
    }

    pthread_barrier_wait(&barrier);

    for(int i = row_start ; i < row_end ; i++){

        for(int j = 0 ; j < 20 ; j++){
            
            if(grid[i][j] == 0 && live_neighbors[i-row_start][j] == 3){ //dead cell has 3 live neighbors
                grid[i][j] = 1;
            }

            else if(grid[i][j] == 1 && (live_neighbors[i-row_start][j] < 2 || live_neighbors[i-row_start][j] > 3)){ //live cell has less than 2 or more than 3 live neighbors
                grid[i][j] = 0;
            }

        }
    }


    int ret = pthread_barrier_wait(&barrier);
    printf("ret: %d\n", ret);
    if(ret == -1){
        print_grid();
    }
}


void initialize_grid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;  // Set every cell to 0 (dead)
        }
    }
    }
void initialize_patterns(int grid[GRID_SIZE][GRID_SIZE]) {
    
    initialize_grid(grid);

    // Initialize Still Life (Square) at top-left
    grid[1][1] = 1;
    grid[1][2] = 1;
    grid[2][1] = 1;
    grid[2][2] = 1;

    // Initialize Oscillator (Blinker) in the middle
    grid[5][6] = 1;
    grid[6][6] = 1;
    grid[7][6] = 1;

    // Initialize Spaceship (Glider) in the bottom-right
    grid[10][10] = 1;
    grid[11][11] = 1;
    grid[12][9] = 1;
    grid[12][10] = 1;
    grid[12][11] = 1;
}

int main() {
    initialize_patterns(grid) ;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t threads[NUM_THREADS];

    int thread_row_start[] = {0, 5, 10, 15};

    for(int j = 0 ; j < GENERATIONS ; j++){
        
        for(int i = 0 ; i < NUM_THREADS ; i++){
            pthread_create(&threads[i], NULL, compute_next_gen, &thread_row_start[i]);
        }

        for(int i = 0 ; i < NUM_THREADS ; i++){
            pthread_join(threads[i], NULL);
        }
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}
