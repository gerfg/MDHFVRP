#include "data.h"

using namespace std;

Data::Data(const char* path){
  std::ifstream f(path);

  f >> n;
  f >> v;
  f >> m;

  capCars.resize(v);
  fixedCostCars.resize(v);
  depots.resize(m, std::vector<int>());
  customers.resize(n);
  matrixDist.resize(n, std::vector<int>(n));
  matrixTime.resize(n, std::vector<int>(n));

  for (size_t i = 0; i < v; i++) {
    f >> capCars[i];
    f >> fixedCostCars[i];
  }

  int countDepotCars, aux;
  for (size_t i = 0; i < m; i++) {
    f >> countDepotCars;
    depots[i].resize(countDepotCars);
    for (size_t j = 0; j < countDepotCars; j++) {
      f >> aux;
      depots[i][j] = capCars[aux];
    }
  }

  for (size_t i = 0; i < n; i++) {
    f >> customers[i];
  }

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      f >> matrixDist[i][j];
    }
  }

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      f >> matrixTime[i][j];
    }
  }
}

void Data::print(){
  cout << ">carTypes: " << endl;
  for (size_t i = 0; i < v; i++) {
    cout << "car" << i << ">  ";
    cout << "cap: " << capCars[i] << " FixedCost:";
    cout << fixedCostCars[i] << endl;
  }

  cout << ">depots:" << endl;
  for (size_t i = 0; i < m; i++) {
    cout << "depot-" << i << ">  ";
    for (size_t j = 0; j < depots[i].size(); j++) {
      cout << "veh" << j << "-Cap: " << depots[i][j] << " ";
    }
    cout << endl;
  }

  cout << ">Customers Demand: ";
  for (size_t i = 0; i < n; i++) {
    cout << customers[i] << " ";
  }
  cout << endl;

  cout << "matrixDist:" << endl;
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      cout << matrixDist[i][j] << " ";
    }
    cout << endl;
  }

  cout << "matrixTime:" << endl;
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      cout << matrixTime[i][j] << " ";
    }
    cout << endl;
  }
}
