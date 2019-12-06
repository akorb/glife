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
void para_range(int, int, int, int, int*, int*);

// 
class GameOfLifeGrid {

public:
	GameOfLifeGrid(int cols, int rows, int gen);
	
	void next();
	void next(const int from, const int to);
	
	int isLive(int cols, int rows) {
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
int nprocs;

struct range {
	int from;
	int to;
};

// Entry point
int main(int argc, char* argv[])
{
	int cols, rows, gen;
	ifstream inputFile;
	int x, y;
	struct timeval start_time, end_time, result_time;
	pthread_t* threadID;
	int status;

	if (argc != 6) {
		cout << "Usage: " << argv[0] << " <input file> <nprocs> <# of generations> <width> <height>" << endl;
		return 1;
	}

	inputFile.open(argv[1], ifstream::in);

	if (inputFile.is_open() == false) {
		cout << "The "<< argv[1] << " file can not be opened" << endl;
		return 1;
	}

	//
	nprocs = atoi(argv[2]);
	gen = atoi(argv[3]);
	cols = atoi(argv[4]);
	rows = atoi(argv[5]);

	g_GameOfLifeGrid = new GameOfLifeGrid(cols, rows, gen);

	while (inputFile.good()) {
		inputFile >> x >> y;
		g_GameOfLifeGrid->setCell(x, y);
	}
	inputFile.close();
	
	g_GameOfLifeGrid->dump();

	while (g_GameOfLifeGrid->getGens() > 0) {
		g_GameOfLifeGrid->next();
		g_GameOfLifeGrid->dump();
	}

	gettimeofday(&start_time, NULL);

    // HINT: YOU MAY NEED TO WRITE PTHREAD INVOKING CODES HERE
	
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &result_time);
	
	cout << "Execution Time: " << result_time.tv_sec <<  "s" << endl;
	//g_GameOfLifeGrid->dumpCoordinate();

	cout << "Program end... " << endl;
	return 0;
}

// HINT: YOU MAY NEED TO FILL OUT BELOW FUNCTIONS
void* workerThread(void *arg)
{
	range r = *(range*)arg;
	g_GameOfLifeGrid->next(r.from, r.to);
}

// to = exclusive
void GameOfLifeGrid::next(const int from, const int to)
{
	for (int row = from; row < to; row++) {
		for (int col = 0; col < this->getCols(); col++) {
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

	int** xchg = m_Temp;
	m_Temp = m_Grid;
	m_Grid = xchg;

	this->decGen();
}

void GameOfLifeGrid::next()
{
	this->next(0, this->getRows() - 1);
}

int GameOfLifeGrid::getNumOfLivingNeighbors(int col, int row)
{
	int countLiving = 0;

	countLiving += this->isLive(col - 1, row - 1);
	countLiving += this->isLive(col + 0, row - 1);
	countLiving += this->isLive(col + 1, row - 1);

	countLiving += this->isLive(col - 1, row + 0);
	countLiving += this->isLive(col + 1, row + 0);

	countLiving += this->isLive(col - 1, row + 1);
	countLiving += this->isLive(col + 0, row + 1);
	countLiving += this->isLive(col + 1, row + 1);

	return countLiving;
}

// HINT: YOU CAN MODIFY BELOW CODES IF YOU WANT
void GameOfLifeGrid::dump() 
{
	cout << "===============================" << endl;

	for (int i=0; i < m_Cols; i++) {
		
		cout << "[" << i << "] ";
		
		for (int j=0; j < m_Rows; j++) {
			cout << m_Grid[i][j];
		}
		
		cout << endl;
	}
	
	cout << "===============================\n" << endl;
}

void GameOfLifeGrid::dumpCoordinate()
{
	cout << ":: Dump X-Y coordinate" << endl;

	for (int i=0; i < m_Cols; i++) {

		for (int j=0; j < m_Rows; j++) {

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


	m_Grid[0] = (int*)malloc(sizeof(int) * (cols*rows));
	if (m_Grid[0] == NULL) 
		cout << "3 Memory allocation error " << endl;

	m_Temp[0] = (int*)malloc(sizeof(int) * (cols*rows));	
	if (m_Temp[0] == NULL) 
		cout << "4 Memory allocation error " << endl;


	for (int i=1; i< cols; i++) {
		m_Grid[i] = m_Grid[i-1] + rows;
		m_Temp[i] = m_Temp[i-1] + rows;
	}

	for (int i=0; i < cols; i++) {
		for (int j=0; j < rows; j++) {
			m_Grid[i][j] = m_Temp[i][j] = 0;
		}
	}

}

