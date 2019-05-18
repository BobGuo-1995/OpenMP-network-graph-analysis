#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <omp.h>

using namespace std;

template<typename T>
class CMatrix {
	size_t _rows = 0, _columns = 0;
	vector<T> _m;
public:
	CMatrix() {};
	CMatrix(size_t rows, size_t columns) {
		Resize(rows, columns);
	}
	void Resize(size_t rows, size_t columns) {
		_m.resize(rows*columns);
		_rows = rows; _columns = columns;
	}
	T& operator() (int row, int column)
	{
		return _m[row*_columns + column];
	}
	size_t Rows() { return _rows; }
	size_t Columns() { return _columns; }
	//*************************
	//return begin iterator of the row 
	auto RowBegin(int row) {
		return _m.begin() + row * _columns;
	}
	//return end iterator of the row 
	auto RowEnd(int row) {
		return _m.begin() + (row + 1) * _columns;
	}
};


//I replaced the old definition these functions
chrono::time_point<std::chrono::high_resolution_clock> start, stop;
void inline tic() {
	start = chrono::high_resolution_clock::now();
}
void inline toc() {
	stop = std::chrono::high_resolution_clock::now();
}
double inline etime() {
	return chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000000.0;
}

//adjacency matrix
CMatrix<int> mat;
//degree for each node
vector<int> vDegree;
//cluster coefficient for each node
vector<double> vCluster;

//For debug
void PrintMatrix()
{
	int N = mat.Rows();
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
			cout << mat(i, j) << " ";
		cout << endl;
	}
}
int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "Usage: ./network input_file output_file number_of_threads" << endl;
		return -1;
	}
	string fnameInput(argv[1]);
	string fnameOutput(argv[2]);
	int nThreads = stoi(argv[3]);
	
	//Read data from the file
	ifstream f1(fnameInput);
	if (!f1.is_open()) { cout << "Can't open file " << fnameInput << endl; exit(-1); }
	cout << "reading file ..." << endl;
	int N, E;
	f1 >> N >> E;
	mat.Resize(N, N);
	while (!f1.eof()) {
		int v1, v2;
		f1 >> v1 >> v2;
		mat(v1, v2) = 1;
		mat(v2, v1) = 1;
	}
	f1.close();
	cout << "done" << endl;
	//PrintMatrix();

	vDegree.resize(N); vCluster.resize(N);
	//********
	//Main part

	cout << "starting main part" << endl;
	cout << "part 1" << endl;
	tic();
	//Part 1 (compute degree)
	int i;
//	private variable With respect to a given set of task regions that bind to the same parallel
//	region, a variable whose name provides access to a different block of storage for each task region.

//	shared variable With respect to a given set of task regions that bind to the same parallel
//	region, a variable whose name provides access to the same block of storage
//	for each task region.
#pragma omp parallel num_threads(nThreads) shared(mat,vDegree) private(i) 
	{
//	schedule(static) is specified, iterations are divided
//	into chunks of size chunk_size, and the chunks are assigned to the threads in
//	the team in a round - robin fashion in the order of the thread number.
#pragma omp for schedule(static)
		for (i = 0; i < N; ++i)
			vDegree[i] = count(mat.RowBegin(i), mat.RowEnd(i), 1);
	}
	cout << "part 2" << endl;
	//Part 2 (cluster)
	int m;
#pragma omp parallel num_threads(nThreads) shared(mat,vDegree,vCluster) private(m) 
	{

//	schedule(dynamic) is specified, the iterations are
//	distributed to threads in the team in chunks as the threads request them.Each
//	thread executes a chunk of iterations, then requests another chunk, until no
//	chunks remain to be distributed.
#pragma omp for schedule(dynamic)
		for (m = 0; m < N; ++m)
		{
			double sum = 0;
			auto row_m = mat.RowBegin(m);
			if (vDegree[m] > 1) {
				for (int n = 0; n < N; ++n)
				{
					if (n == m)continue;
					auto row_n = mat.RowBegin(n);
					if (*(row_m + n) == 1)
						for (int p = n + 1; p < N; ++p)
							if (*(row_n + p) == 1 && *(row_m + p) == 1)
								++sum;
				}
				sum = 2*sum/(vDegree[m] * (vDegree[m] - 1));
			}
			else sum = -1;
			vCluster[m] = sum;
		}
	}
	toc();
	//End main part
	cout << "done" << endl;
	cout << "time=" << etime() << endl;
	//Write result to the file
	ofstream f2(fnameOutput);
	f2 << N << endl;
	for (int i = 0; i < N; ++i)
		f2 << i << "," << vDegree[i] << "," << vCluster[i] << endl;
	f2.close();

	return 0;
}
