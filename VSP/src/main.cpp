#include <ilcplex/ilocplex.h>
#include <iostream>
#include "data.h"

void solve(Data& data);

int main(int argc, char **argv){
  Data data;
  data.readInstance(argv[1]);
  data.print();
  data.calcArcs();

  solve(data);
  return 0;
}

void solve(Data& data){
  IloEnv env;
	IloModel model(env, "MDHFVSPTW");

  int limit = data.n+data.m;
  long long int bigM = 99999999999;

  // Decision variable
  IloArray <IloArray < IloArray< IloArray < IloBoolVarArray > > > > X (env, limit+1);
  for (int i = 1; i <= limit; i++) {
    X[i] = IloArray <IloArray< IloArray<IloBoolVarArray>> > (env, limit+1);
    for (int j = 1; j <= limit; j++) {
      X[i][j] = IloArray<IloArray<IloBoolVarArray>> (env, data.v+1);
      for (int k = 1; k <= data.v; k++) {
        X[i][j][k] = IloArray<IloBoolVarArray> (env, limit+1);
        for (int d = data.n+1; d <= limit; d++) {
          X[i][j][k][d] = IloBoolVarArray(env, data.h+1);
          for(int day = 1; day <= data.h; day++) {
            if (data.arcsX[i][j][k][d][day]) {
              char name[20];
              sprintf(name, "X(%d,%d,%d,%d,%d)", i, j, k, d, day);
              X[i][j][k][d][day].setName(name);
              model.add(X[i][j][k][d][day]);
            }
          }
        }
      }
    }
  }

  //Time variable

  IloNumVarArray b(env, limit+1, 0, 99999);
	for (int i = 1; i <= limit; ++i){
		char name[20];
		sprintf(name, "b(%d)", i);
		b[i].setName(name);
		model.add(b[i]);
	}

  // Flow variable
	IloArray <IloNumVarArray>  f(env, limit+1);
	for(int i = 1; i <= limit; i++){
		f[i] = IloNumVarArray(env, limit+1, 0, 99999);
		for(int j = 1; j <= limit; j++){
      if (data.arcsY[i][j]) {
        char name[20];
  			sprintf(name, "f(%d,%d)", i, j);
  			f[i][j].setName(name);
  			model.add(f[i][j]);
      }
		}
	}

  // FO
  IloExpr OBJ(env);
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= limit; j++) {
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          for(size_t l = 1; l <= data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              OBJ += data.matrixDist[i][j] * X[i][j][k][d][l];
            }
          }
        }
      }
    }
  }
  model.add(IloMinimize(env, OBJ));

  // (2) cada cliente é visitado apenas uma vez
  for (int j = 1; j <= data.n; j++) {
    IloExpr expr1(env);
    for (int i = 1; i <= limit; i++) {
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          for(size_t l = 1; l <= data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              expr1 += X[i][j][k][d][l];
            }
          }
        }
      }
    }
    IloRange r = (expr1 == 1);
		char c[100];
		sprintf(c, "c2_%d", j);
		r.setName(c);
		model.add(r);
  }

  // (3)
  for (int i = 1; i <= data.n; i++) {
    IloExpr expr1(env);
    for (int j = 1; j <= limit; j++) {
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          for(size_t l = 1; l <= data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              expr1 += X[i][j][k][d][l];
            }
          }
        }
      }
    }
    IloRange r = (expr1 == 1);
		char c[100];
		sprintf(c, "c3_%d", i);
		r.setName(c);
		model.add(r);
  }
