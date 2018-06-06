#include <ilcplex/ilocplex.h>
#include <iostream>
#include "data.h"

void solve(Data& data);

int main(int argc, char **argv){
  Data data(argv[1]);
  // data.print();
  solve(data);
  return 0;
}

// incluir 12 e 13

void solve(Data& data){
  IloEnv env;
	IloModel model(env, "MDHFVRP");

  int limit = data.n+data.m;

  // Decision variable
  IloArray <IloArray < IloArray< IloBoolVarArray > > > X (env, limit);
  for (size_t i = 0; i < limit; i++) {
    X[i] = IloArray <IloArray<IloBoolVarArray> > (env, limit);
    for (size_t j = 0; j < limit; j++) {
      X[i][j] = IloArray<IloBoolVarArray> (env, data.v);
      for (size_t k = 0; k < data.v; k++) {
        X[i][j][k] = IloBoolVarArray(env, limit);
        for (size_t d = data.n; d < limit; d++) {

          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
              char name[20];
              sprintf(name, "X(%d,%d,%d,%d)", i, j, k, d);
              X[i][j][k][d].setName(name);
              model.add(X[i][j][k][d]);
          }

        }
      }

    }
  }

  // Flow variable
	IloArray <IloNumVarArray>  f(env, limit);
	for(int i = 0; i < limit; i++){
		f[i] = IloNumVarArray(env, limit, 0, data.maxCap);
		for(int j = 0; j < limit; j++){
      if (i != j) {
        char name[20];
  			sprintf(name, "f(%d,%d)", i, j);
  			f[i][j].setName(name);
  			model.add(f[i][j]);
      }
		}
	}

  // FO
  IloExpr OBJ(env);
  for (size_t i = data.n; i < limit; i++) {
    for (size_t j = 0; j < data.n; j++) {
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            OBJ += data.vehiclesTypes[k].fixed * X[i][j][k][d];
          }
        }
      }
    }
  }

  for (size_t i = 0; i < limit; i++) {
    for (size_t j = 0; j < limit; j++) {
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            OBJ += data.matrixDist[i][j] * X[i][j][k][d];
          }
        }
      }
    }
  }
  model.add(IloMinimize(env, OBJ));

  // (2) cada cliente é visitado apenas uma vez
  for (size_t j = 0; j < data.n; j++) {
    IloExpr expr1(env);
    for (size_t i = 0; i < limit; i++) {
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr1 += X[i][j][k][d];
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
  for (size_t i = 0; i < data.n; i++) {
    IloExpr expr1(env);
    for (size_t j = 0; j < limit; j++) {
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr1 += X[i][j][k][d];
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

  // (4) um tipo de veiculo deve cobrir o arco i,j
  for (size_t k = 0; k < data.v; k++) {
    for (size_t j = 0; j < limit; j++) {
      for (size_t d = data.n; d < limit; d++) {
        IloExpr expr1(env);
        IloExpr expr2(env);

        for (size_t i = 0; i < limit; i++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr1 += X[i][j][k][d];
          }
        }

        for (size_t i = 0; i < limit; i++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr2 += X[j][i][k][d];
          }
        }

        IloRange r = ((expr1 - expr2) == 0);
    		char c[100];
    		sprintf(c, "c4_%d_%d_%d", k, j, d);
    		r.setName(c);
    		model.add(r);
      }
    }
  }

  // (5) total de carga dos veiculos que sairam dos depots é igual
  //     a demanda total dos clientes
  // não tem forall
  IloExpr expr1(env);
  IloExpr expr2(env);
  for (size_t i = data.n; i < limit; i++) {
    for (size_t j = 0; j < data.n; j++) {
      expr1 += f[i][j];
      expr2 += data.customersDemand[j];
    }
  }
  IloRange r = ((expr1 - expr2) == 0);
  char c[100];
  sprintf(c, "c5");
  r.setName(c);
  model.add(r);

  // (6) carga no carro = carga ants de passar no cliente + demanda
  for (size_t j = 0; j < data.n; j++) {
    IloExpr expr1(env);
    IloExpr expr2(env);
    for (size_t i = 0; i < limit; i++) {
      expr1 += f[i][j];
      expr2 += f[j][i];
    }
    IloRange r = ((expr1 - expr2) == data.customersDemand[j]);
    char c[100];
    sprintf(c, "c6_%d", j);
    r.setName(c);
    model.add(r);
  }

  // (7) não violação da carga do veiculo
  for (size_t i = 0; i < limit; i++) {
    for (size_t j = 0; j < data.n; j++) {
      IloExpr expr1(env);
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr1 += data.vehiclesTypes[k].cap * X[i][j][k][d];
          }
        }
      }
      if (i != j) {
        IloRange r = (expr1 - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c7_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (8)
  for (size_t i = 0; i < data.n; i++) {
    for (size_t k = 0; k < data.v; k++) {
      for (size_t d1 = data.n; d1 < limit; d1++) {
        for (size_t d2 = 0; d2 < limit; d2++) {
          IloExpr expr1(env);
          if (d1 != d2) {
            if ((data.vehiclesInDepot[k][d2] > 0) && (d1 != i) && ((d1 < data.n) || (i < data.n))) {
              expr1 += X[d1][i][k][d2];
            }
            IloRange r = (expr1 == 0);
            char c[100];
            sprintf(c, "c8_%d_%d_%d_%d", d1, i, k, d2);
            r.setName(c);
            model.add(r);
          }
        }
      }
    }
  }

  // (9)
  for (size_t i = 0; i < data.n; i++) {
    for (size_t k = 0; k < data.v; k++) {
      for (size_t d1 = data.n; d1 < limit; d1++) {
        for (size_t d2 = 0; d2 < limit; d2++) {
          IloExpr expr1(env);
          if (d1 != d2) {
            if ((data.vehiclesInDepot[k][d2] > 0) && (i != d1) && ((i < data.n) || (d1 < data.n))) {
              expr1 += X[i][d1][k][d2];
            }
            IloRange r = (expr1 == 0);
            char c[100];
            sprintf(c, "c9_%d_%d_%d_%d", i, d1, k, d2);
            r.setName(c);
            model.add(r);
          }
        }
      }
    }
  }

  // (11)
  for (size_t i = 0; i < limit; i++) {
    for (size_t j = 0; j < limit; j++) {
      if (i != j) {
        IloExpr expr1(env);
        expr1 += f[i][j];

        IloRange r = (expr1 >= 0);
        char c[100];
        sprintf(c, "c11_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (12)
  for (size_t i = 0; i < limit; i++) {
    for (size_t j = 0; j < data.n; j++) {
      IloExpr expr(env);
      for (size_t d = data.n; d < limit; d++) {
        for (size_t k = 0; k < data.v; k++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr += (data.vehiclesTypes[k].cap - data.customersDemand[i]) * X[i][j][k][d];
          }
        }
      }
      if (i != j) {
        IloRange r = (expr - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c12_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (13)
  for (size_t i = 0; i < data.n; i++) {
    for (size_t j = 0; j < data.n; j++) {
      IloExpr expr(env);
      for (size_t k = 0; k < data.v; k++) {
        for (size_t d = data.n; d < limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
            expr += data.customersDemand[j] * X[i][j][k][d];
          }
        }
      }
      if (i != j) {
        IloRange r = (expr - f[i][j] <= 0);
        char c[100];
        sprintf(c, "c13_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  IloCplex mdhfvrp(model);
	mdhfvrp.exportModel("mdhfvrp.lp");
  mdhfvrp.setParam(IloCplex::Threads, 1);

  mdhfvrp.solve();

  // double value = mdhfvrp.getObjValue();

  for(int k = 0; k < data.v; ++k){
    // std::cout << "Vehicle number " << k << '\n';
		for(int d = data.n; d < limit; ++d){
      // std::cout << "Depot number " << d << '\n';
			for(int i = 0; i < limit; ++i){
				for(int j = 0; j < limit; ++j){
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && ((i < data.n) || (j < data.n))) {
						// printf("%d to %d,", i, j);
						// printf("time: %.2lf, vehicle capacity: %.2lf\n", mdhdarp.getValue(b[j][k][d]), mdhdarp.getValue(f[i][j]));
            if (mdhfvrp.getValue(X[i][j][k][d]) > 0) {
              std::cout << i << " - " << j << " Veiculo: " << k << " do depot: " << d << '\n';
              std::cout << "f-> " << mdhfvrp.getValue(f[i][j]) << '\n';
              // std::cout << "X[" << i << "][" << j << "]: " << mdhfvrp.getValue(X[i][j][k][d]) << '\n';
            }
          }
				}
			}
		}
	}

  std::cout << "\nOBJ " << mdhfvrp.getObjValue() << std::endl;

}

/*
o processo é o seguinte
gera-se uma instância de brinquedo
no arquivo

aí implementa-se o leitor

joga tudo num struct
ou classe
Data

aí cria a função pra gerar o lp
(e talvez rodar)

aí passa lá o data pra função

define as variaveis e cria a FO

exporta o .lp

checa
se tem nexo

depois joga o 1o grupo de restrições

checa o .lp

e assim por diante
vai fazendo incrementalmente
*/
