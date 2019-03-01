#ifndef DATA_H
#define DATA_H

#include <vector>
#include <iostream>
#include <stdio.h>
#include <fstream>

using namespace std;

struct vehicle {
  int cap;
  int type;
  float fixed;
  float variable;
};

struct tw {
  int start;
  int end;
};

class Data {
public:
  int n, v, m, maxCap;
  vector<vehicle> vehiclesTypes;
  vector<vector<int> > vehiclesInDepot;
  vector<int> customersDemand;
  vector<int> clientType;
  vector<vector<double> > matrixDist;
  vector<vector<double> > matrixTime;
  vector<tw> timeWindow;
  vector<vector<int> > route;

  vector< vector< vector< vector<bool>>>> arcsX;
  vector< vector<bool>> arcsY;

  Data(const char* path);
  void print();
  void calcArcs();
  void printRoute();
};

#endif
