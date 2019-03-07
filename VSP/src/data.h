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
  int serviceTime;
};

struct crd {
  float x;
  float y;
};

class Client {
public:
  int taskType;
  tw timeW;
  vector<bool> days;
  vector<bool> vehicles;
};

class Data {
public:
  int n, v, m, h, maxCap;
  vector<vehicle> vehiclesTypes;
  vector<vector<int> > vehiclesInDepot;
  vector<Client> clts;
  
  vector<vector<int> > route;

  vector<crd> coord;
  vector<vector<double> > matrixDist;
  vector<vector<double> > matrixTime;

  vector< vector< vector< vector<bool>>>> arcsX;
  vector< vector<bool>> arcsY;

  Data() = default;
  void readBS(const char* path);
  void readVSP(const char* path);
  void print();
  void calcArcs();
  void printRoute();
};

#endif
