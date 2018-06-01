#ifndef DATA_H
#define DATA_H

#include <vector>
#include <iostream>
#include <stdio.h>
#include <fstream>

class Data {
public:
  int n, v, m;
  std::vector<int> capCars;
  std::vector<float> fixedCostCars;
  std::vector<std::vector<int> > depots;
  std::vector<int> customers;
  std::vector<std::vector<int> > matrixDist;
  std::vector<std::vector<int> > matrixTime;

  Data(const char* path);
  void print();
};

#endif
