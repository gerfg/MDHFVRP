#include "data.h"

using namespace std;

Data::Data(const char* path){
  std::ifstream f(path);

  f >> n;
  f >> v;
  f >> m;

  vehiclesTypes.resize(v+1);
  vehiclesInDepot.resize(n+m+1, std::vector<int>(v+1));
  customersDemand.resize(n+m+1);
  matrixDist.resize(n+m+1, std::vector<double>(n+m+1));
  matrixTime.resize(n+m+1, std::vector<double>(n+m+1));
  timeWindow.resize(n+m+1);
  route.resize(n+m+1, std::vector<int>(3) );

  for (size_t i = 1; i <= v; i++) {
    f >> vehiclesTypes[i].cap;
    f >> vehiclesTypes[i].fixed;
    f >> vehiclesTypes[i].variable;
  }

  for (size_t i = n+1; i <= n+m; i++) {
    for (size_t j = 1; j <= v; j++) {
      f >> vehiclesInDepot[i][j];
    }
  }

  for (size_t i = 1; i <= n+m; i++) {
    f >> customersDemand[i];
    f >> timeWindow[i].start;
    f >> timeWindow[i].end;
  }

  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= n+m; j++) {
      f >> matrixDist[i][j];
    }
  }

  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= n+m; j++) {
      f >> matrixTime[i][j];
    }
  }

  maxCap = 0;
  for (size_t i = 1; i <= v; i++) {
    if (maxCap < vehiclesTypes[i].cap) {
      maxCap = vehiclesTypes[i].cap;
    }
  }

}

void Data::print(){
  cout << ">carTypes: " << endl;
  for (size_t i = 1; i <= v; i++) {
    cout << "car" << i << "> ";
    cout << "cap: " << vehiclesTypes[i].cap << " ";
    cout << "FixedCost: " << vehiclesTypes[i].fixed << " ";
    cout << "VariableCost: " << vehiclesTypes[i].variable << endl;
  }
  cout << endl;
  for (size_t i = n+1; i <= n+m; i++) {
    cout << "Depot-" << i << " ";
    for (size_t j = 1; j <= v; j++) {
      cout << "v" << j << ": " << vehiclesInDepot[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
  cout << ">Customers Demand: \n";
  for (size_t i = 1; i <= n+m; i++) {
    cout << "Demand: " << customersDemand[i] << " timeWindow ";
    cout << timeWindow[i].start << " - ";
    cout << timeWindow[i].end << "\n";
  }
  cout << endl;

  cout << "matrixDist:" << endl;
  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= n+m; j++) {
      cout << matrixDist[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
  cout << "matrixTime:" << endl;
  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= n+m; j++) {
      cout << matrixTime[i][j] << " ";
    }
    cout << endl;
  }
  std::cout << "\nMaxCap: " << maxCap << "\n\n";
}

void Data::printRoute(){
  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= 2; j++) {
      // std::cout << route[i][j] << " ";
    }
    // std::cout << '\n';
  }

  int position = n+1;
  for (size_t i = 1; i <= n+m; i++) {
    std::cout << route[position][1] << " (B: ";
    position = route[position][2];
  }

}
