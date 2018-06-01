#include "Data.h"
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include <climits>
#include <algorithm>
#include <exception>
#include <string>

using namespace std;

Data::Data(const char* filePath){
	FILE *input_file = fopen(filePath , "r" );

	fscanf( input_file, "%d %d %d", &n, &v, &m); //Requestes, type of cars, depots

	P.resize(n); //Request vertices
	D.resize(n); //Delivery vertices
	M.resize(m); //Depot vertices
	V.resize(2*n+m); //All vertices
	K.resize(v); //Type of vehicles
	L.resize(n); //Max ride time
	T.resize(n); //Latest time arrived
	q.resize(n); //Number of passengers per request

	U.resize(v, vector<int>(m)); //Number of type of vehicles from depot
	Q.resize(v, vector<int>(m)); //Capacity of type of vehicle from depot

	t.resize(V.size(), vector<double>(V.size()));
	c.resize(V.size(), vector<double>(V.size()));
	A_.resize(V.size(), vector<bool>(V.size()));

	double X[V.size()], Y[V.size()]; //Each point in map

	for(int i = 0; i < v; ++i){
		for(int j = 0; j < m; ++j){
			fscanf(input_file, "%d", &U[i][j]);
		}
	}

	for(int i = 0; i < v; ++i){
		for(int j = 0; j < m; ++j){
			fscanf(input_file, "%d", &Q[i][j]);
		}
	}
	for (int i = 0; i < n; ++i)
		fscanf(input_file, "%d", &q[i]);

	for (int i = 0; i < n; ++i)
		fscanf(input_file, "%lf", &L[i]);

	for (int i = 0; i < n; ++i)
		fscanf(input_file, "%lf", &T[i]);

	for (int i = 0; i < V.size(); ++i)
		fscanf(input_file, "%lf", &X[i]);

	for (int i = 0; i < V.size(); ++i)
		fscanf(input_file, "%lf", &Y[i]);

	for(int i = 0; i < V.size(); ++i){
		for(int j = 0; j< V.size(); ++j){
			t[i][j] = c[i][j] = hypot( (X[i]-X[j]), (Y[i]-Y[j]));
		}
	}

	//All arcs are first invalid
	for(int i = 0; i < V.size(); ++i){
		for(int j = 0; j < V.size(); ++j){
			A_[i][j] = true;
		}
	}

	for(int i = 0; i < n; ++i){
		for (int d = 0; d < m; ++d){
			Amp.push_back(make_pair(V.size()+d-1, i));
			A.push_back(make_pair(V.size()+d-1, i));
			A_[V.size()+d-1][i] = false;
		}
	}

	for(int i = 0; i < (2*n); ++i){
		for (int j = 0; j < (2*n); ++j){
			if ( (i == j) || (j == (i-n) ) ) continue; // Arcs where i == j or from delivery i to pickup i
			Apd.push_back(make_pair(i, j));
			A.push_back(make_pair(i, j));
			A_[i][j] = false;
		}
	}

	for(int i = 0; i < n; ++i){
		for (int d = 0; d < m; ++d){
			Adm.push_back(make_pair(i+n, V.size()+d-1));
			A.push_back(make_pair(i+n, V.size()+d-1));
			A_[i+n][V.size()+d-1] = false;
		}
	}
}

void Data::print(){
	printf("%d %d %d \n", n, v, m);
	for(int i = 0; i < v; ++i){
		printf("The vehicle type: %d \n", i);
		for(int j = 0; j < m; ++j){
			printf("In depot %d, we have %d copies and their capacity is %d: \n", j, U[i][j], Q[i][j]);
		}
		printf("\n");
	}

	for (int i = 0; i < n; ++i)
		printf("\nVertex %d request %d passengers, they have maximum ride time: %.2lf and must arrived before: %.2lf, \n", i, q[i], L[i], T[i]);


	printf("Cost Matrix\n");
	for(int i = 0; i < V.size(); ++i){
		for(int j = 0; j < V.size(); ++j)
			printf("%lf ", c[i][j]);
		printf("\n");
	}

	printf("\nArcs A\n");
	for (int i = 0; i < A.size(); ++i){
		printf("(%d, %d)", A[i].first, A[i].second);
	}

	printf("\nArcs Amp\n");
	for (int i = 0; i < Amp.size(); ++i){
		printf("(%d, %d)", Amp[i].first, Amp[i].second);
	}

	printf("\nArcs Apd\n");
	for (int i = 0; i < Apd.size(); ++i){
		printf("(%d, %d)", Apd[i].first, Apd[i].second);
	}

	printf("\nArcs Adm\n");
	for (int i = 0; i < Adm.size(); ++i){
		printf("(%d, %d)", Adm[i].first, Adm[i].second);
	}

	printf("\nArcs Adm\n");
	for (int i = 0; i < V.size(); ++i){
		for(int j = 0; j < V.size(); ++j)
			printf("A_[%d][%d]: %s ", i, j, A_[i][j] ? "true" : "false");
		printf("\n");
	}
	printf("\n");
}
