#ifndef _DATA_H_
#define _DATA_H_

#include <vector>

#define NMAX 1000

typedef std::pair<int,int> arc;

class Data
{
public:

	const char* filePath;
	int n, v, m;
	double matG[NMAX][NMAX];
	std::vector<int> P, D, M, K, V, q;
	std::vector<double>L, T;
	std::vector<std::vector<int> > U, Q;
	std::vector<arc> Amp, Apd, Adm, A;
	std::vector<std::vector<arc> > Graph;
	std::vector<std::vector<double> > t, c;
	std::vector<std::vector<bool> > A_;

	Data(const char* filePath);

	void print();

};

#endif
