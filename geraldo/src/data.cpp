#include "data.h"

using namespace std;

Data::Data(const char* path){
  std::ifstream f(path);

  f >> n;
  f >> v;
  f >> m;

  vehiclesTypes.resize(v);
  vehiclesInDepot.resize(n+m, std::vector<int>(v));
  customersDemand.resize(n);
  matrixDist.resize(n+m, std::vector<int>(n+m));
  matrixTime.resize(n+m, std::vector<int>(n+m));

  for (size_t i = 0; i < v; i++) {
    f >> vehiclesTypes[i].cap;
    f >> vehiclesTypes[i].fixed;
    f >> vehiclesTypes[i].variable;
  }

  for (size_t i = n; i < n+m; i++) {
    for (size_t j = 0; j < v; j++) {
      f >> vehiclesInDepot[i][j];
    }
  }

  for (size_t i = 0; i < n+m; i++) {
    f >> customersDemand[i];
  }

  for (size_t i = 0; i < n+m; i++) {
    for (size_t j = 0; j < n+m; j++) {
      f >> matrixDist[i][j];
    }
  }

  for (size_t i = 0; i < n+m; i++) {
    for (size_t j = 0; j < n+m; j++) {
      f >> matrixTime[i][j];
    }
  }

  maxCap = 0;
  for (size_t i = 0; i < v; i++) {
    if (maxCap < vehiclesTypes[i].cap) {
      maxCap = vehiclesTypes[i].cap;
    }
  }

}

void Data::print(){
  cout << ">carTypes: " << endl;
  for (size_t i = 0; i < v; i++) {
    cout << "car" << i << "> ";
    cout << "cap: " << vehiclesTypes[i].cap << " ";
    cout << "FixedCost: " << vehiclesTypes[i].fixed << " ";
    cout << "VariableCost: " << vehiclesTypes[i].variable << endl;
  }

  for (size_t i = n; i < n+m; i++) {
    cout << "Depot-" << i << " ";
    for (size_t j = 0; j < v; j++) {
      cout << "v" << j << ": " << vehiclesInDepot[i][j] << " ";
    }
    cout << endl;
  }

  cout << ">Customers Demand: ";
  for (size_t i = 0; i < n+m; i++) {
    cout << customersDemand[i] << " ";
  }
  cout << endl;

  cout << "matrixDist:" << endl;
  for (size_t i = 0; i < n+m; i++) {
    for (size_t j = 0; j < n+m; j++) {
      cout << matrixDist[i][j] << " ";
    }
    cout << endl;
  }

  cout << "matrixTime:" << endl;
  for (size_t i = 0; i < n+m; i++) {
    for (size_t j = 0; j < n+m; j++) {
      cout << matrixTime[i][j] << " ";
    }
    cout << endl;
  }
}
