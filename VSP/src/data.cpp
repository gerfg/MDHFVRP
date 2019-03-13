#include "data.h"

void Data::readInstance(const char* path) {
  ifstream f(path);

  f >> m;
  f >> n;
  f >> v;
  f >> h;

  vehiclesTypes.resize(v+1);
  for(int i = 1; i <= v; i++) {
    vehiclesTypes[i].cap = 9999999;
    vehiclesTypes[i].type = i;
    vehiclesTypes[i].fixed = 0;
    vehiclesTypes[i].variable = 0;
  }
  
  vehiclesInDepot.resize(n+m+1, vector<int>(v+1));

  coord.resize(n+m+1);
  timeWindow.resize(n+m+1);
  clientType.resize(n+1);
  customersDemand.resize(n+1);
  daysToAttend.resize(n+1, vector<bool>(h+1, false));
  vehiclesToAttend.resize(n+1, vector<bool>(v+1, false));

  matrixDist.resize(n+m+1, vector<double>(n+m+1));
  matrixTime.resize(n+m+1, vector<double>(n+m+1));

  arcsX.resize(n+m+1, vector<vector<vector<vector<bool>>>>(n+m+1, vector<vector<vector<bool>>>(v+1, vector<vector<bool>>(n+m+1, vector<bool>(h+1, false)))));
  arcsY.resize(n+m+1, vector<bool>(n+m+1, false));
  route.resize(n+m+1, vector<int>(3));

  for (size_t i = n+1; i <= n+m; i++) {
    f >> coord[i].x;
    f >> coord[i].y;
    customersDemand[i] = 0;
    for (size_t j = 1; j <= v; j++) {
      f >> vehiclesInDepot[i][j];
    }
  }

  int aux;
  for (size_t i = 1; i <= n; i++) {
    f >> coord[i].x;
    f >> coord[i].y;
    f >> clientType[i];
    f >> timeWindow[i].serviceTime;
    f >> timeWindow[i].start;
    f >> timeWindow[i].end;
    customersDemand[i] = 1;

    f >> aux;
    int iter = 1;
    boost::dynamic_bitset<> days(h, aux);
    for(int j = 0; j < days.size(); ++j) {
      if (days[j] == 0) {
        daysToAttend[i][iter] = false;
      } else { 
        daysToAttend[i][iter] = true;
      }
      iter++;
    }

    f >> aux;
    iter = 1;
    boost::dynamic_bitset<> vehicles(v, aux);
    for(int j = 0; j < vehicles.size(); ++j) {
      if (vehicles[j] == 0) {
        vehiclesToAttend[i][iter] = false;
      } else { 
        vehiclesToAttend[i][iter] = true;
      }
      iter++;
    }
  }

  for (size_t i = 1; i <= n+m; i++) {
    for (size_t j = 1; j <= n+m; j++) {
      matrixDist[i][j] = calcDistEucl(coord[i], coord[j]);
    }
  }
}

double Data::calcDistEucl(crd pt1, crd pt2) {
  double x = pt1.x - pt2.x;
  double y = pt1.y - pt2.y;
  
  return sqrt(pow(x,2) + pow(y,2));
}

void Data::calcArcs() {
  cout << "Calculando Arcos X e Y\n";
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
          
          for(int day = 1; day <= h; day++) {

            if (i < n && j < n) {
              if (   (daysToAttend[i][day] == true)
                  && (daysToAttend[j][day] == true)
                  && (vehiclesToAttend[i][k] == true)
                  && (vehiclesToAttend[j][k] == true)
              ){
                arcsX[i][j][k][d][day] = true;
                // cout << "X[" << i << "][" << j << "][" << k << "][" << d << "][" << day << "]\n";
              }
            } else {

              if ((i > n && j <= n)
                  && (vehiclesToAttend[j][k] == 1)
                  && (daysToAttend[j][day] == 1)
                  && (i == d)
              ) {
                arcsX[i][j][k][d][day] = true;
                // cout << "X[" << i << "][" << j << "][" << k << "][" << d << "][" << day << "]\n";
              }

              if ((i <= n && j > n)
                  && (vehiclesToAttend[i][k] == 1)
                  && (daysToAttend[i][day] == 1)
                  && (j == d)
              ) {
                arcsX[i][j][k][d][day] = true;
                // cout << "X[" << i << "][" << j << "][" << k << "][" << d << "][" << day << "]\n";
              }

            }
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
          for(int day = 1; day <= h; day++) {
            if (arcsX[i][j][k][d][day]) {
              // cout << "arcsX[" << i << "][" << j << "][" << k << "][" << d << "][" << day << "]\n";
            }
          }
        }
      }
    }
  }
  // std::cout << "\n";
  for(int i = 1; i <= n+m; i++) {
    for(int j = 1; j <= n+m; j++) {
      if (arcsY[i][j]) {
        // cout << "arcsY[" << i << "][" << j << "]\n";
      }
    }
  }

  cout << "Arcos Calculados.\n\n";
}

void Data::print(){
  cout << "Qtd Depots: " << m << "\n"
  << "Qtd Clients: " << n << "\n"
  << "Types Vehicles: " << v << "\n"
  << "Horizon(days): " << h << "\n";
  
  for (size_t i = n+1; i <= n+m; i++) {
    cout << "Depot-" << i << " " << coord[i].x << " " << coord[i].y << " ";
    for (size_t j = 1; j <= v; j++) {
      cout << "v" << j << ": " << vehiclesInDepot[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
  cout << ">Customers Demand: \n";
  for (size_t i = 1; i <= n; i++) {
    cout << coord[i].x << " - " << coord[i].y << " "
    << "ClientType: " << clientType[i] << " "
    << "ServiceTime: " << timeWindow[i].serviceTime << " "
    << " timeWindow " << timeWindow[i].start << " - "
    << timeWindow[i].end << " - "
    << "Demand: " << customersDemand[i] << " "
    << "DaysToAttend: ";
    for(int j = 1; j < daysToAttend[i].size(); j++) {
      cout << daysToAttend[i][j] << " ";
    }
    cout << "  -  "
    << "VehiclesToAttend: ";
    for(int j = 1; j < vehiclesToAttend[i].size(); j++) {
      cout << vehiclesToAttend[i][j] << " ";
    }
    cout << "\n";
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
