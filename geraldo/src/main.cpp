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
  IloArray <IloArray < IloArray< IloBoolVarArray > > > X (env, limit+1);
  for (size_t i = 1; i <= limit; i++) {
    X[i] = IloArray <IloArray<IloBoolVarArray> > (env, limit+1);
    for (size_t j = 1; j <= limit; j++) {
      X[i][j] = IloArray<IloBoolVarArray> (env, data.v+1);
      for (size_t k = 1; k <= data.v; k++) {
        X[i][j][k] = IloBoolVarArray(env, limit+1);
        for (size_t d = data.n+1; d <= limit; d++) {
          // std::cout << i << " " << j << " " << k << " " << d << '\n';
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
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
	IloArray <IloNumVarArray>  f(env, limit+1);
	for(int i = 1; i <= limit; i++){
		f[i] = IloNumVarArray(env, limit+1, 0, data.maxCap);
		for(int j = 1; j <= limit; j++){
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        // std::cout << "f["<< i << "][" << j << "]" << '\n';
        char name[20];
  			sprintf(name, "f(%d,%d)", i, j);
  			f[i][j].setName(name);
  			model.add(f[i][j]);
      }
		}
	}

  // FO
  IloExpr OBJ(env);
  for (size_t i = data.n+1; i <= limit; i++) {
    for (size_t j = 1; j <= data.n; j++) {
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            OBJ += data.vehiclesTypes[k].fixed * X[i][j][k][d];
          }
        }
      }
    }
  }

  for (size_t i = 1; i <= limit; i++) {
    for (size_t j = 1; j <= limit; j++) {
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            OBJ += data.matrixDist[i][j] * X[i][j][k][d];
          }
        }
      }
    }
  }
  model.add(IloMinimize(env, OBJ));

  // (2) cada cliente é visitado apenas uma vez
  for (size_t j = 1; j <= data.n; j++) {
    IloExpr expr1(env);
    for (size_t i = 1; i <= limit; i++) {
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
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
  for (size_t i = 1; i <= data.n; i++) {
    IloExpr expr1(env);
    for (size_t j = 1; j <= limit; j++) {
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
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
  for (size_t k = 1; k <= data.v; k++) {
    for (size_t j = 1; j <= limit; j++) {
      for (size_t d = data.n+1; d <= limit; d++) {
        if (data.vehiclesInDepot[d][k] > 0) {
          IloExpr expr1(env);
          IloExpr expr2(env);
          for (size_t i = 1; i <= limit; i++) {
            if ((i != j) && (i < data.n+1 || j < data.n+1)) {
              expr1 += X[i][j][k][d];
            }
          }

          for (size_t i = 1; i <= limit; i++) {
            if ((i != j) && (i < data.n+1 || j < data.n+1)) {
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
  }

  // (5) total de carga dos veiculos que sairam dos depots é igual
  //     a demanda total dos clientes
  IloExpr expr1(env);
  IloExpr expr2(env);
  for (size_t i = data.n+1; i <= limit; i++) {
    for (size_t j = 1; j <= data.n; j++) {
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        expr1 += f[i][j];
        expr2 += data.customersDemand[j];
      }
    }
  }
  IloRange r = ((expr1 - expr2) == 0);
  char c[100];
  sprintf(c, "c5");
  r.setName(c);
  model.add(r);

  // (6) carga no carro = carga ants de passar no cliente + demanda
  //     conservação de fluxo
  for (size_t j = 1; j <= data.n; j++) {
    IloExpr expr1(env);
    IloExpr expr2(env);
    for (size_t i = 1; i <= limit; i++) {
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        expr1 += f[i][j];
        expr2 += f[j][i];
      }
    }

    IloRange r = ((expr1 - expr2) == data.customersDemand[j]);
    char c[100];
    sprintf(c, "c6_%d", j);
    r.setName(c);
    model.add(r);
  }

  // (7) não violação da carga do veiculo
  for (size_t i = 1; i <= limit; i++) {
    for (size_t j = 1; j <= data.n; j++) {
      IloExpr expr1(env);
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr1 += data.vehiclesTypes[k].cap * X[i][j][k][d];
          }
        }
      }
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        IloRange r = (expr1 - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c7_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (8)
  for (size_t i = 1; i <= data.n; i++) {
    for (size_t k = 1; k <= data.v; k++) {
      for (size_t d1 = data.n+1; d1 <= limit; d1++) {
        for (size_t d2 = data.n+1; d2 <= limit; d2++) {
          IloExpr expr1(env);
          if (d1 != d2) {
            if ((data.vehiclesInDepot[d2][k] > 0) && (d1 != i) && (d1 < data.n+1 || i < data.n+1)) {
              expr1 += X[d1][i][k][d2];
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
  }

  // (9)
  for (size_t i = 1; i <= data.n; i++) {
    for (size_t k = 1; k <= data.v; k++) {
      for (size_t d1 = data.n+1; d1 <= limit; d1++) {
        for (size_t d2 = data.n+1; d2 <= limit; d2++) {
          IloExpr expr1(env);
          if (d1 != d2) {
              if ((data.vehiclesInDepot[d2][k] > 0) && (i != d1) && (i < data.n+1 || d1 < data.n+1)) {
              expr1 += X[i][d1][k][d2];
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
  }

  // (11)
  for (size_t i = 1; i <= limit; i++) {
    for (size_t j = 1; j <= limit; j++) {
      IloExpr expr1(env);
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
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
  for (size_t i = 1; i <= limit; i++) {
    for (size_t j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      for (size_t d = data.n+1; d <= limit; d++) {
        for (size_t k = 1; k <= data.v; k++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr += (data.vehiclesTypes[k].cap - data.customersDemand[i]) * X[i][j][k][d];
          }
        }
      }
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
        IloRange r = (expr - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c12_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (13)
  for (size_t i = 1; i <= data.n; i++) {
    for (size_t j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      for (size_t k = 1; k <= data.v; k++) {
        for (size_t d = data.n+1; d <= limit; d++) {
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            expr += data.customersDemand[j] * X[i][j][k][d];
          }
        }
      }
      if ((i != j) && (i < data.n+1 || j < data.n+1)) {
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

  std::cout << '\n';
  for(int k = 1; k <= data.v; ++k){
    // std::cout << "Vehicle number " << k << '\n';
		for(int d = data.n+1; d <= limit; ++d){
      // std::cout << "Depot number " << d << '\n';
			for(int i = 1; i <= limit; ++i){
				for(int j = 1; j <= limit; ++j){
          if ((data.vehiclesInDepot[d][k] > 0) && (i != j) && (i < data.n+1 || j < data.n+1)) {
            if (mdhfvrp.getValue(X[i][j][k][d]) > 0) {
              std::cout << "X[" << i << "][" << j << "] Veiculo: " << k << " do depot: " << d << '\n';
              // std::cout << "X[" << i << "][" << j << "]: " << mdhfvrp.getValue(X[i][j][k][d]) << " f-> " << mdhfvrp.getValue(f[i][j]) << '\n';
            }
          }
				}
			}
		}
    std::cerr << '\n';
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
