#ifndef DATA_H
#define DATA_H

#include <vector>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <math.h>
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
  int serviceTime;
};

struct crd {
  float x;
  float y;
};

class Data {
public:
  int n, v, m, h;
  vector<vehicle> vehiclesTypes;
  vector<vector<int> > vehiclesInDepot;
  
  vector<crd> coord;
  vector<tw> timeWindow;
  vector<int> clientType;
  vector<int> customersDemand;
  vector<vector<bool>> daysToAttend;
  vector<vector<bool>> vehiclesToAttend;

  vector<vector<double> > matrixDist;
  vector<vector<double> > matrixTime;

  vector< vector< vector< vector<vector<bool>>>>> arcsX;
  vector< vector<bool>> arcsY;
  vector<vector<int> > route;

  Data() = default;
  void readInstance(const char* path);
  double calcDistEucl(crd pt1, crd pt2);
  void print();
  void calcArcs();
  void printRoute();
};

#endif
