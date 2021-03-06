#include <ilcplex/ilocplex.h>
#include <iostream>
#include "data.h"

void solve(Data& data);

int main(int argc, char **argv){
  Data data(argv[1]);
  data.print();
  solve(data);
  return 0;
}

void solve(Data& data){
  IloEnv env;
	IloModel model(env, "MDHFVRP");

  int limit = data.n+data.m;
  long long int bigM = 99999999999;

  // Decision variable
  IloArray <IloArray < IloArray< IloBoolVarArray > > > X (env, limit+1);
  for (int i = 1; i <= limit; i++) {
    X[i] = IloArray <IloArray<IloBoolVarArray> > (env, limit+1);
    for (int j = 1; j <= limit; j++) {
      X[i][j] = IloArray<IloBoolVarArray> (env, data.v+1);
      for (int k = 1; k <= data.v; k++) {
        X[i][j][k] = IloBoolVarArray(env, limit+1);
        for (int d = data.n+1; d <= limit; d++) {
          // std::cout << i << " " << j << " " << k << " " << d << '\n';
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
              char name[20];
              sprintf(name, "X(%d,%d,%d,%d)", i, j, k, d);
              X[i][j][k][d].setName(name);
              model.add(X[i][j][k][d]);
          // }

        }
      }

    }
  }

  //Time variable

	// IloArray <IloArray <IloNumVarArray> > b(env, limit+1);
	// for (int i = 1; i <= limit; ++i){
	// 	b[i] = IloArray<IloNumVarArray>(env, data.v+1);
	// 	for (int k = 1; k <= data.v; ++k){
	// 		b[i][k] = IloNumVarArray(env, limit+1, data.timeWindow[i].start, data.timeWindow[i].end);
	// 		for(int d = data.n+1; d <= limit; ++d){
	// 			char name[20];
	// 			sprintf(name, "b(%d,%d,%d)", i, k, d);
	// 			b[i][k][d].setName(name);
	// 			model.add(b[i][k][d]);
	// 		}
	// 	}
	// }

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
		f[i] = IloNumVarArray(env, limit+1, 0, data.maxCap);
		for(int j = 1; j <= limit; j++){
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        // std::cout << "f["<< i << "][" << j << "]" << '\n';
        char name[20];
  			sprintf(name, "f(%d,%d)", i, j);
  			f[i][j].setName(name);
  			model.add(f[i][j]);
      // }
		}
	}

  // FO
  IloExpr OBJ(env);
  for (size_t d = data.n+1; d <= limit; d++) {
    for (size_t k = 1; k <= data.v; k++) {
      IloExpr expr(env);
      for (size_t i = data.n+1; i <= limit; i++) {
        for (size_t j = 1; j <= data.n; j++) {
          expr += X[i][j][k][d];
        }
      }
      OBJ += data.vehiclesTypes[k].fixed * expr;
    }
  }

  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= limit; j++) {
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            OBJ += data.vehiclesTypes[k].variable * data.matrixDist[i][j] * X[i][j][k][d];
          // }
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
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr1 += X[i][j][k][d];
          // }
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
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr1 += X[i][j][k][d];
          // }
        }
      }
    }
    IloRange r = (expr1 == 1);
		char c[100];
		sprintf(c, "c3_%d", i);
		r.setName(c);
		model.add(r);
  }

  // (4)
  for (int k = 1; k <= data.v; k++) {
    for (int j = 1; j <= limit; j++) {
      for (int d = data.n+1; d <= limit; d++) {
        // if (data.vehiclesInDepot[d][k] > 0) {
          IloExpr expr1(env);
          IloExpr expr2(env);
          for (int i = 1; i <= limit; i++) {
            // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
              expr1 += X[i][j][k][d];
            // }
          }

          for (int i = 1; i <= limit; i++) {
            // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
              expr2 += X[j][i][k][d];
            // }
          }

          IloRange r = ((expr1 - expr2) == 0);
          char c[100];
          sprintf(c, "c4_%d_%d_%d", k, j, d);
          r.setName(c);
          model.add(r);
        // }
      }
    }
  }

  // (5) total de carga dos veiculos que sairam dos depots é igual
  //     a demanda total dos clientes
  IloExpr expr1(env);
  IloExpr expr2(env);
  for (int i = data.n+1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        expr1 += f[i][j];
        expr2 += data.customersDemand[j];
      // }
    }
  }

  IloRange r = ((expr1 - expr2) == 0);
  char c[100];
  sprintf(c, "c5");
  r.setName(c);
  model.add(r);

  // (6) carga no carro = carga ants de passar no cliente + demanda
  //     conservação de fluxo
  for (int j = 1; j <= data.n; j++) {
    IloExpr expr1(env);
    IloExpr expr2(env);
    for (int i = 1; i <= limit; i++) {
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        expr1 += f[i][j];
        expr2 += f[j][i];
      // }
    }

    IloRange r = ((expr1 - expr2) == data.customersDemand[j]);
    char c[100];
    sprintf(c, "c6_%d", j);
    r.setName(c);
    model.add(r);
  }

  // (7) não violação da carga do veiculo
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          expr += data.vehiclesTypes[k].cap * X[i][j][k][d];
        }
      }
      IloRange r = (expr - f[i][j] >= 0);
      char c[100];
      sprintf(c, "c7_%d_%d", i, j);
      r.setName(c);
      model.add(r);
    }
  }

  // (8)
  for (int i = 1; i <= data.n; i++) {
    for (int k = 1; k <= data.v; k++) {
      for (int d1 = data.n+1; d1 <= limit; d1++) {
        for (int d2 = data.n+1; d2 <= limit; d2++) {
          if (d1 != d2) {
            IloExpr expr1(env);
            // if ((data.vehiclesInDepot[d2][k] > 0) && (d1 != i) && (d1 < data.n+1 || i < data.n+1)) {
              expr1 += X[d1][i][k][d2];
              IloRange r = (expr1 == 0);
              char c[100];
              sprintf(c, "c8_%d_%d_%d_%d", d1, i, k, d2);
              r.setName(c);
              model.add(r);
            // }
          }
        }
      }
    }
  }

  // (9)
  for (int i = 1; i <= data.n; i++) {
    for (int k = 1; k <= data.v; k++) {
      for (int d1 = data.n+1; d1 <= limit; d1++) {
        for (int d2 = data.n+1; d2 <= limit; d2++) {
          if (d1 != d2) {
            IloExpr expr1(env);
              // if ((data.vehiclesInDepot[d2][k] > 0) && (i != d1) && (i < data.n+1 || d1 < data.n+1)) {
              expr1 += X[i][d1][k][d2];
              IloRange r = (expr1 == 0);
              char c[100];
              sprintf(c, "c9_%d_%d_%d_%d", i, d1, k, d2);
              r.setName(c);
              model.add(r);
            // }
          }
        }
      }
    }
  }

  // (11)
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= limit; j++) {
      IloExpr expr1(env);
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        expr1 += f[i][j];
        IloRange r = (expr1 >= 0);
        char c[100];
        sprintf(c, "c11_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      // }
    }
  }

  // (12)
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      for (int d = data.n+1; d <= limit; d++) {
        for (int k = 1; k <= data.v; k++) {
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr += (data.vehiclesTypes[k].cap - data.customersDemand[i]) * X[i][j][k][d];
          // }
        }
      }
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        IloRange r = (expr - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c12_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      // }
    }
  }

  // (13)
  for (int i = 1; i <= data.n; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          // if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr += data.customersDemand[j] * X[i][j][k][d];
          // }
        }
      }
      // if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        IloRange r = (f[i][j] - expr >= 0);
        char c[100];
        sprintf(c, "c13_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      // }
    }
  }

  // (14)
  for (int i = 1; i <= data.n; i++) {
    for (int j = data.n+1; j <= limit; j++) {
      IloExpr expr(env);
      expr += f[i][j];
      IloRange r = (expr == 0);
      char c[100];
      sprintf(c, "c14_%d_%d", i, j);
      r.setName(c);
      model.add(r);
    }
  }

  // (15)
  for (int i = data.n+1; i <= limit; i++) {
    for (int j = data.n+1; j <= limit; j++) {
      IloExpr expr(env);
      expr += f[i][j];
      IloRange r = (expr == 0);
      char c[100];
      sprintf(c, "c15_%d_%d", i, j);
      r.setName(c);
      model.add(r);
    }
  }

  // (16)
  for (int i = 1; i <= data.n; i++) {
    IloExpr expr(env);
    expr += f[i][i];
    IloRange r = (expr == 0);
    char c[100];
    sprintf(c, "c16_%d", i);
    r.setName(c);
    model.add(r);
  }

  // (17)
  for (int i = data.n+1; i <= limit; i++) {
    for (int j = data.n+1; j <= limit; j++) {
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          IloExpr expr(env);
          expr += X[i][j][k][d];
          IloRange r = (expr == 0);
          char c[100];
          sprintf(c, "c17_%d_%d_%d_%d", i,j,k,d);
          r.setName(c);
          model.add(r);
        }
      }
    }
  }

  // (18)
  for (int i = 1; i <= limit; i++) {
    for (int k = 1; k <= data.v; k++) {
      for (int d = data.n+1; d <= limit; d++) {
        IloExpr expr(env);
        expr += X[i][i][k][d];
        IloRange r = (expr == 0);
        char c[100];
        sprintf(c, "c18_%d_%d_%d_%d", i,i,k,d);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (19) TW
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      if (i != j) {
        IloExpr expr(env);

        for (int k = 1; k <= data.v; k++) {
          for (int d = data.n+1; d <= limit; d++) {
             expr += X[i][j][k][d];
          }
        }

        // IloRange r = ( (b[j] - (expr1 - expr2) ) >= 0 );
        IloRange r = ( (b[j] - ( b[i] + data.matrixTime[i][j] - bigM*(1-expr) )  ) >= 0 );
        char c[100];
        sprintf(c, "c19_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (20)
  for (int i = 1; i <= limit; i++) {
    IloRange r = (b[i] >= data.timeWindow[i].start);
    char c[100];
    sprintf(c, "c20_%d", i);
    r.setName(c);
    model.add(r);
  }
  // (21)
  for (int i = 1; i <= limit; i++) {
    IloRange r = (b[i] <= data.timeWindow[i].end);
    char c[100];
    sprintf(c, "c21_%d", i);
    r.setName(c);
    model.add(r);
  }


  IloCplex mdhfvrp(model);
	mdhfvrp.exportModel("mdhfvrp.lp");
  mdhfvrp.setParam(IloCplex::Threads, 1);

  mdhfvrp.solve();

  std::cout << '\n';
  for(int k = 1; k <= data.v; ++k){
		for(int d = data.n+1; d <= limit; ++d){
			for(int i = 1; i <= limit; ++i){
				for(int j = 1; j <= limit; ++j){
          if (mdhfvrp.getValue(X[i][j][k][d]) > 0) {
            data.route[i][1] = i;
            data.route[i][2] = j;
            std::cout << "X[" << i << "][" << j << "] Veiculo: " << k << " do depot: " << d << " f-> " << mdhfvrp.getValue(f[i][j]) << "  B[" << j << "]->" << mdhfvrp.getValue(b[j]) << /*" - " << mdhfvrp.getValue(b[j]) <<*/ '\n';
            // std::cout << "X[" << i << "][" << j << "]: " << mdhfvrp.getValue(X[i][j][k][d]) << " f-> " << mdhfvrp.getValue(f[i][j]) << '\n';
          }
				}
			}
		}
	}

  std::cout << '\n';
  // Print only Route
  int position = data.n+1;
  for (size_t i = 1; i <= limit; i++) {
    std::cout << data.route[position][1] << "  ";
    position = data.route[position][2];
  }
  std::cout << data.route[position][1] << '\n';

  // Print Route and B
  position = data.n+1;
  for (size_t i = 1; i <= limit; i++) {
    // std::cout << data.route[position][1] << " (B(" << data.route[position][1] << "): " << mdhfvrp.getValue(b[data.route[position][2]]) << ")  ";
    position = data.route[position][2];
  }
  // std::cout << data.route[position][1] << " (B(" << data.route[position][1] << "): " << mdhfvrp.getValue(b[data.route[position][2]]) << ")  \n";

  std::cout << '\n';
  position = data.n+1;
  double tme = 0;
  int cl1, cl2;
  for (size_t i = 1; i <= limit; i++) {
    cl1 = data.route[position][1];
    cl2 = data.route[position][2];
    std::cout << cl1 << "(" << data.timeWindow[cl1].start << " " << data.timeWindow[cl1].end << ")";
    std::cout << "[" << tme << "] - ";
    std::cout << cl2 << "(" << data.timeWindow[cl2].start << " " << data.timeWindow[cl2].end << ")";
    tme += data.matrixTime[cl1][cl2];
    std::cout << "[" << tme << "]" << '\n';
    position = data.route[position][2];
  }


  // std::cout << '\n' << "B" << '\n';
  // for (size_t i = 1; i <= data.n; i++) {
  //   std::cout << i << " : " << mdhfvrp.getValue(b[i]) << "\n";
  // }

  std::cout << "\nOBJ " << mdhfvrp.getObjValue() << std::endl;

}
