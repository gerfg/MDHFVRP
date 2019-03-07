#include "data.h"

void Data::readBS(const char* path){
  ifstream f(path);

  f >> n;
  f >> v;
  f >> m;

  vehiclesTypes.resize(v+1);
  vehiclesInDepot.resize(n+m+1, vector<int>(v+1));
  customersDemand.resize(n+m+1);
  clientType.resize(n+m+1);
  matrixDist.resize(n+m+1, vector<double>(n+m+1));
  matrixTime.resize(n+m+1, vector<double>(n+m+1));
  timeWindow.resize(n+m+1);
  route.resize(n+m+1, vector<int>(3));

  arcsX.resize(n+m+1, vector<vector<vector<bool>>>(n+m+1, vector<vector<bool>>(v+1, vector<bool>(m+1, false))));
  arcsY.resize(n+m+1, vector<bool>(n+m+1, false));

  for (size_t i = 1; i <= v; i++) {
    f >> vehiclesTypes[i].cap;
    f >> vehiclesTypes[i].fixed;
    f >> vehiclesTypes[i].variable;
    f >> vehiclesTypes[i].type;
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
    f >> clientType[i];
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

void Data::readVSP(const char* path) {
  ifstream f(path);

  f >> m;
  f >> n;
  f >> v;
  f >> h;

  vehiclesTypes.resize(v+1);
  vehiclesInDepot.resize(n+m+1, vector<int>(v+1));
  customersDemand.resize(n+m+1);
  clientType.resize(n+m+1);
  timeWindow.resize(n+m+1);
  route.resize(n+m+1, vector<int>(3));

  coord.resize(n+m+1);
  matrixDist.resize(n+m+1, vector<double>(n+m+1));
  matrixTime.resize(n+m+1, vector<double>(n+m+1));

  arcsX.resize(n+m+1, vector<vector<vector<bool>>>(n+m+1, vector<vector<bool>>(v+1, vector<bool>(m+1, false))));
  arcsY.resize(n+m+1, vector<bool>(n+m+1, false));

  for (size_t i = n+1; i <= n+m; i++) {
    f >> coord[i].x;
    f >> coord[i].y;
    for (size_t j = 1; j <= v; j++) {
      f >> vehiclesInDepot[i][j];
    }
  }

  for (size_t i = 1; i <= n+m; i++) {
    f >> coord[i].x;
    f >> coord[i].y;
    f >> clientType[i];
    f >> timeWindow[i].serviceTime;
    f >> timeWindow[i].start;
    f >> timeWindow[i].end;
  }
  return ;

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

void Data::calcArcs() {
  for(int i = 1; i <= n+m; i++) {
    for(int j = 1; j <= n+m; j++) {
      
      if (i == j) {
        continue;
      }
      
      for(int k = 1; k <= v; k++) {
        for(int d = n+1; d <= n+m; d++) {
          
          if (vehiclesInDepot[d][k] == 0) {
            continue;
          }
          
          if ((i <= n && j <= n)
              && (clientType[i] == vehiclesTypes[k].type)
              && (clientType[j] == vehiclesTypes[k].type)
          ) {
            arcsX[i][j][k][d] = true;
          }

          if ((i > n && j <= n)
              && (clientType[j] == vehiclesTypes[k].type)
              && (i == d)
          ) {
            arcsX[i][j][k][d] = true;
          }

          if ((i <= n && j > n)
              && (clientType[i] == vehiclesTypes[k].type)
              && (j == d)
          ) {
            arcsX[i][j][k][d] = true;
          }
          
        }
      }
    }
  }

  for(int i = 1; i <= n+m; i++) {
    for(int j = 1; j <= n+m; j++) {
      if (i == j) {
        continue;
      }

      if (i <= n && j <= n) {
        arcsY[i][j] = true;
      }
      if (i > n && j <= n) {
        arcsY[i][j] = true;
      }
    }
  }

  for(int i = 1; i <= n+m; i++) {
    for(int j = 1; j <= n+m; j++) {
      for(int k = 1; k <= v; k++) {
        for(int d = n+1; d <= n+m; d++) {
          if (arcsX[i][j][k][d]) {
            cout << "arcsX[" << i << "][" << j << "][" << k << "][" << d << "]\n";
          }
        }
      }
    }
  }
  std::cout << "\n";
  for(int i = 1; i <= n+m; i++) {
    for(int j = 1; j <= n+m; j++) {
      if (arcsY[i][j]) {
        cout << "arcsY[" << i << "][" << j << "]\n";
      }
    }
  }
}

void Data::print(){
  cout << "Qtd Depots: " << m << "\n"
  << "Qtd Clients: " << n << "\n"
  << "Types Vehicles: " << v << "\n"
  << "Horizon(days): " << h << "\n"
  
  << ">carTypes: " << endl;
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
  // for (size_t i = 1; i <= n+m; i++) {
  //   cout << "Demand: " << customersDemand[i] << " timeWindow ";
  //   cout << timeWindow[i].start << " - ";
  //   cout << timeWindow[i].end << "\n";
  // }
  // cout << endl;

  // cout << "matrixDist:" << endl;
  // for (size_t i = 1; i <= n+m; i++) {
  //   for (size_t j = 1; j <= n+m; j++) {
  //     cout << matrixDist[i][j] << " ";
  //   }
  //   cout << endl;
  // }
  // cout << endl;
  // cout << "matrixTime:" << endl;
  // for (size_t i = 1; i <= n+m; i++) {
  //   for (size_t j = 1; j <= n+m; j++) {
  //     cout << matrixTime[i][j] << " ";
  //   }
  //   cout << endl;
  // }
  // cout << "\nMaxCap: " << maxCap << "\n\n";
}

void Data::printRoute(){
  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= 2; j++) {
      // cout << route[i][j] << " ";
    }
    // cout << '\n';
  }

  int position = n+1;
  for (size_t i = 1; i <= n+m; i++) {
    cout << route[position][1] << " (B: ";
    position = route[position][2];
  }

}
