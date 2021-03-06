#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define LIVE 1
#define DEAD 0

# define timersub(a, b, result)                 \
	do {                        \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;           \
		(result)->tv_usec = (a)->tv_usec - (b)->tv_usec;            \
		if ((result)->tv_usec < 0) {                \
			--(result)->tv_sec;                 \
			(result)->tv_usec += 1000000;               \
		}                       \
	} while (0)

using namespace std;

// function prototype
void* workerThread(void *);

// 
class GameOfLifeGrid {

public:
	GameOfLifeGrid(int cols, int rows, int gen);
	
	void next();
	void next(const int from, const int to);
	void update();
	
	int isLive(int cols, int rows) {
		// Took me some time but I found a non-branch way
		// to calculate "over the edges":
		// But the requirements linked to a example implementation
		// that does not use "over the edge" calculating
		// So commented out but left  for reference

		//rows = (m_Rows + rows) % m_Rows;
		//cols = (m_Cols + cols) % m_Cols;

		// No "over the edges"
		// We consider all cells outside of the grid as dead
		if (cols >= m_Cols || cols < 0 || rows >= m_Rows || rows < 0)
			return DEAD;

		return m_Grid[cols][rows]; 
	}
	int getNumOfLivingNeighbors(int col, int row);
	
	void dead(int cols, int rows) { m_Temp[cols][rows] = DEAD; }
	void live(int cols, int rows) { m_Temp[cols][rows] = LIVE; }

	int decGen() { return m_Generations--; }
	void setGen(int n) { m_Generations = n; }
	void setCell(int cols, int rows) { m_Grid[cols][rows] = true; }
	
	void dump();
	void dumpCoordinate();
	int* getColAddr(int col) { return m_Grid[col]; }

	int getCols() { return m_Cols; }
	int getRows() { return m_Rows; }
	int getGens() { return m_Generations; }

private:
	int** m_Grid;
	int** m_Temp;
	int m_Rows;
	int m_Cols;
	int m_Generations;

};

GameOfLifeGrid* g_GameOfLifeGrid;
pthread_barrier_t g_barrier;
int nprocs;

// Read-only! Will be only set in main and read once from each thread to init their own counter
int ro_gen;

struct thread_info {
	int from;
	int to;
	bool updater;
};


thread_info* ranges(int width, int nprocs, int* count) {
	if (width < nprocs)
		nprocs = width;

	// With this approach threads will do maximum one more column than the other threads
	int range = width / nprocs;
	int rest = width % nprocs;
	int offset = 0;
	struct thread_info* ret = (struct thread_info*)malloc(sizeof(struct thread_info) * nprocs);
	for (int i = 0; i < nprocs; i++) {
		ret[i].from = range * i + offset;
		ret[i].to = ret[i].from + range;
		if (offset < rest) {
			offset++;
			ret[i].to++;
		}
		ret[i].updater = false;
	}

	*count = nprocs;
	return ret;
}

// Entry point
int main(int argc, char* argv[])
{
	int cols, rows, gen;
	ifstream inputFile;
	int x, y;
	struct timeval start_time, end_time, result_time;

	if (argc != 6) {
		cout << "Usage: " << argv[0] << " <input file> <nprocs> <# of generations> <width> <height>" << endl;
		return 1;
	}

	inputFile.open(argv[1], ifstream::in);

	if (!inputFile.is_open()) {
		cout << "The "<< argv[1] << " file can not be opened" << endl;
		return 1;
	}

	//
	nprocs = atoi(argv[2]);
	gen = atoi(argv[3]);
	cols = atoi(argv[4]);
	rows = atoi(argv[5]);

	ro_gen = gen;
	g_GameOfLifeGrid = new GameOfLifeGrid(cols, rows, gen);

	while (inputFile.good()) {
		inputFile >> x >> y;
		g_GameOfLifeGrid->setCell(x, y);
	}
	inputFile.close();
	
	gettimeofday(&start_time, NULL);

	int thread_count;
	thread_info* rgs = ranges(cols, nprocs, &thread_count);

	pthread_barrier_init(&g_barrier, NULL, thread_count);

	// -1 because we will use the main thread too
	pthread_t threads[thread_count - 1];

	for (int i = 0; i < thread_count - 1; i++) {
		int res = pthread_create(&threads[i], NULL, &workerThread, &rgs[i]);
		if (res != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	// Only main thread uses the last range struct instance. So it will act as the updater too.
	rgs[thread_count - 1].updater = true;
	workerThread(&rgs[thread_count - 1]);

	// Everything is finished now
	//
	// So join threads
	for (int i = 0; i < thread_count - 1; i++) {
		pthread_join(threads[i], NULL);
	}
	//
	// and destroy barrier for cleanup
	pthread_barrier_destroy(&g_barrier);
	
	// I think cleanup should be included in benchmarking too because it wouldn't be required for non-parallelized program
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &result_time);

	#ifdef BENCHMARK
	cout << result_time.tv_sec << " ";
	#else
	g_GameOfLifeGrid->dumpCoordinate();
	#endif
	return EXIT_SUCCESS;
}

void* workerThread(void *arg) {
	thread_info r = *(thread_info*)arg;
	for (int gen = ro_gen; gen > 0; gen--) {
		g_GameOfLifeGrid->next(r.from, r.to);
		pthread_barrier_wait(&g_barrier);
		if (r.updater) {
			g_GameOfLifeGrid->update();
		}
		pthread_barrier_wait(&g_barrier);
	}
	return NULL;
}

// to = exclusive
void GameOfLifeGrid::next(const int from, const int to)
{
	for (int col = from; col < to; col++) {
		for (int row = 0; row < m_Rows; row++) {
			int countLiving = getNumOfLivingNeighbors(col, row);
			countLiving += this->isLive(col, row);
			if (countLiving == 4) {
				// retain current state
				m_Temp[col][row] = m_Grid[col][row];
			}
			else {
				m_Temp[col][row] = countLiving == 3;
			}
		}
	}
}

void GameOfLifeGrid::next()
{
	this->next(0, this->getRows() - 1);
}

void GameOfLifeGrid::update()
{
	int** xchg = m_Temp;
	m_Temp = m_Grid;
	m_Grid = xchg;
}


int GameOfLifeGrid::getNumOfLivingNeighbors(int col, int row)
{
	int countLiving = 0;

	// Keep as long as possible in same column for better caching
	countLiving += this->isLive(col - 1, row - 1);
	countLiving += this->isLive(col - 1, row + 0);
	countLiving += this->isLive(col - 1, row + 1);

	countLiving += this->isLive(col + 0, row - 1);
	countLiving += this->isLive(col + 0, row + 1);

	countLiving += this->isLive(col + 1, row - 1);
	countLiving += this->isLive(col + 1, row + 0);
	countLiving += this->isLive(col + 1, row + 1);

	return countLiving;
}

// HINT: YOU CAN MODIFY BELOW CODES IF YOU WANT
void GameOfLifeGrid::dump() 
{
	cout << "===============================" << endl;
	
	// This has terrible caching behavior,
	// but performance doesn't matter here anymore
	// so that's okay for this project
	for (int j = 0; j < m_Rows; j++) {
		cout << "[" << j << "] ";
		
		for (int i = 0; i < m_Cols; i++) {
			cout << m_Grid[i][j];
		}
		
		cout << endl;
	}
	
	cout << "===============================\n" << endl;
}

void GameOfLifeGrid::dumpCoordinate()
{
	cout << ":: Dump X-Y coordinate" << endl;

	for (int i = 0; i < m_Cols; i++) {

		for (int j = 0; j < m_Rows; j++) {

			if (m_Grid[i][j])
				cout << i << " " << j << endl;
		}
	}
}


GameOfLifeGrid::GameOfLifeGrid(int cols, int rows, int gen)
{
	m_Generations = gen;
	m_Cols = cols;
	m_Rows = rows;

	m_Grid = (int**)malloc(sizeof(int*) * cols);

	if (m_Grid == NULL) 
		cout << "1 Memory allocation error " << endl;

	m_Temp = (int**)malloc(sizeof(int*) * cols);
	if (m_Temp == NULL) 
		cout << "2 Memory allocation error " << endl;


	m_Grid[0] = (int*)calloc(cols * rows, sizeof(int));
	if (m_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	m_Temp[0] = (int*)calloc(cols * rows, sizeof(int));	
	if (m_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i = 1; i < cols; i++) {
		m_Grid[i] = m_Grid[i - 1] + rows;
		m_Temp[i] = m_Temp[i - 1] + rows;
	}
}
