#ifndef DATA_H
#define DATA_H

#include <vector>
#include <iostream>
#include <stdio.h>
#include <fstream>

struct vehicle {
  int cap;
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
  std::vector<vehicle> vehiclesTypes;
  std::vector<std::vector<int> > vehiclesInDepot;
  std::vector<int> customersDemand;
  std::vector<std::vector<double> > matrixDist;
  std::vector<std::vector<double> > matrixTime;
  std::vector<tw> timeWindow;

  Data(const char* path);
  void print();
};

#endif
