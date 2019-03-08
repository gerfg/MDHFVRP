#include <ilcplex/ilocplex.h>
#include <iostream>
#include "data.h"

void solve(Data& data);

int main(int argc, char **argv){
  Data data;
  data.readInstance(argv[1]);
  // data.print();
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

  // (4)
  bool added = false;
  for (int k = 1; k <= data.v; k++) {
    for (int j = 1; j <= limit; j++) {
      for (int d = data.n+1; d <= limit; d++) {
        for(int l = 1; l <= data.h; l++) {
          added = false;
          IloExpr expr(env);
          for (int i = 1; i <= limit; i++) {
            if (data.arcsX[i][j][k][d][l]) {
              added = true;
              expr += X[i][j][k][d][l];
            }
          }

          for (int i = 1; i <= limit; i++) {
            if (data.arcsX[i][j][k][d][l]) {
              added = true;
              expr -= X[j][i][k][d][l];
            }
          }

          if (added) {
            IloRange r = (expr == 0);
            char c[100];
            sprintf(c, "c4_%d_%d_%d_%d", k, j, d, l);
            r.setName(c);
            model.add(r);
          }
        }
      }
    }
  }

  // (5) total de carga dos veiculos que sairam dos depots é igual
  //     a demanda total dos clientes
  IloExpr expr1(env);
  IloExpr expr2(env);
  for (int i = data.n+1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      if (data.arcsY[i][j]) {
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
  for (int j = 1; j <= data.n; j++) {
    IloExpr expr1(env);
    IloExpr expr2(env);
    added = false;
    for (int i = 1; i <= limit; i++) {
      if (data.arcsY[i][j]) {
        added = true;
        expr1 += f[i][j];
      }
      if (data.arcsY[j][i]) {
        added = true;
        expr2 += f[j][i];
      }
    }

    if (added) {
      IloRange r = ((expr1 - expr2) == data.customersDemand[j]);
      char c[100];
      sprintf(c, "c6_%d", j);
      r.setName(c);
      model.add(r);
    }
  }

  // (7) não violação da carga do veiculo
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      added = false;
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          for(int l = 1; l <= data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              expr += data.vehiclesTypes[k].cap * X[i][j][k][d][l];
              added = true;
            }
          }
        }
      }
      if (added) {
        IloRange r = (expr - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c7_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (11)
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= limit; j++) {
      if (data.arcsY[i][j]) {
        IloExpr expr1(env);
        expr1 = f[i][j];
        IloRange r = (expr1 >= 0);
        char c[100];
        sprintf(c, "c11_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // (12)
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      added = false;
      for (int d = data.n+1; d <= limit; d++) {
        for (int k = 1; k <= data.v; k++) {
          for(int l = 1; l <= data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              expr += (data.vehiclesTypes[k].cap - data.customersDemand[i]) * X[i][j][k][d][l];
              added = true;
            }
          }
        }
      }
      if (added) {
        IloRange r = (expr - f[i][j] >= 0);
        char c[100];
        sprintf(c, "c12_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }


  // (13)
  for (int i = 1; i <= data.n; i++) {
    for (int j = 1; j <= data.n; j++) {
      IloExpr expr(env);
      added = false;
      for (int k = 1; k <= data.v; k++) {
        for (int d = data.n+1; d <= limit; d++) {
          for(int l = 0; l < data.h; l++) {
            if (data.arcsX[i][j][k][d][l]) {
              expr += data.customersDemand[j] * X[i][j][k][d][l];
              added = true;
            }
          }
        }
      }
      if (added) {
        IloRange r = (f[i][j] - expr >= 0);
        char c[100];
        sprintf(c, "c13_%d_%d", i, j);
        r.setName(c);
        model.add(r);
      }
    }
  }

  // // // (14) -------------------------------------------------
  // // for (int i = 1; i <= data.n; i++) {
  // //   for (int j = data.n+1; j <= limit; j++) {
  // //     IloExpr expr(env);
  // //     if (data.arcsY[i][j]) {
  // //       expr = f[i][j];
  // //     }
  // //     IloRange r = (expr == 0);
  // //     char c[100];
  // //     sprintf(c, "c14_%d_%d", i, j);
  // //     r.setName(c);
  // //     model.add(r);
  // //   }
  // // }

  // // // (15)
  // // for (int i = data.n+1; i <= limit; i++) {
  // //   for (int j = data.n+1; j <= limit; j++) {
  // //     IloExpr expr(env);
  // //     if (data.arcsY[i][j]) {
  // //       expr += f[i][j];
  // //     }
  // //     IloRange r = (expr == 0);
  // //     char c[100];
  // //     sprintf(c, "c15_%d_%d", i, j);
  // //     r.setName(c);
  // //     model.add(r);
  // //   }
  // // }

  // // // (16)
  // // for (int i = 1; i <= data.n; i++) {
  // //   IloExpr expr(env);
  // //   if (data.arcsY[i][i]) {
  // //     expr += f[i][i];
  // //   }
  // //   IloRange r = (expr == 0);
  // //   char c[100];
  // //   sprintf(c, "c16_%d", i);
  // //   r.setName(c);
  // //   model.add(r);
  // // }

  // // // (17)
  // // for (int i = data.n+1; i <= limit; i++) {
  // //   for (int j = data.n+1; j <= limit; j++) {
  // //     for (int k = 1; k <= data.v; k++) {
  // //       for (int d = data.n+1; d <= limit; d++) {
  // //         IloExpr expr(env);
  // //         if (data.arcsX[i][j][k][d]) {
  // //           expr += X[i][j][k][d];
  // //         }
  // //         IloRange r = (expr == 0);
  // //         char c[100];
  // //         sprintf(c, "c17_%d_%d_%d_%d", i,j,k,d);
  // //         r.setName(c);
  // //         model.add(r);
  // //       }
  // //     }
  // //   }
  // // }

  // // // (18)
  // // for (int i = 1; i <= limit; i++) {
  // //   for (int k = 1; k <= data.v; k++) {
  // //     for (int d = data.n+1; d <= limit; d++) {
  // //       IloExpr expr(env);
  // //       if (data.arcsX[i][i][k][d]) {
  // //         expr += X[i][i][k][d];
  // //       }
  // //       IloRange r = (expr == 0);
  // //       char c[100];
  // //       sprintf(c, "c18_%d_%d_%d_%d", i,i,k,d);
  // //       r.setName(c);
  // //       model.add(r);
  // //     }
  // //   }
  // // }

  // (19) TW
  for (int i = 1; i <= limit; i++) {
    for (int j = 1; j <= data.n; j++) {
        IloExpr expr(env);
        added = false;

        for (int k = 1; k <= data.v; k++) {
          for (int d = data.n+1; d <= limit; d++) {
            for(int l = 1; l < data.h; l++) {
              if (data.arcsX[i][j][k][d][l]) {
                expr += X[i][j][k][d][l];
                added = true;
              }
            }
          }
        }

        if (added) {
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
	mdhfvrp.exportModel("vsp.lp");
  mdhfvrp.setParam(IloCplex::Threads, 1);

  mdhfvrp.solve();

  // std::cout << '\n';
  // for(int k = 1; k <= data.v; ++k){
	// 	for(int d = data.n+1; d <= limit; ++d){
  //     for(int l = 1; l <= data.h; l++) {
  //       for(int i = 1; i <= limit; ++i){
  //         for(int j = 1; j <= limit; ++j){
  //           if (mdhfvrp.getValue(X[i][j][k][d][l]) > 0.5) {
  //             // data.route[i][1] = i;
  //             // data.route[i][2] = j;
  //             // std::cout << "X[" << i << "][" << j << "] Veiculo: " << k << " do depot: " << d << " f-> " << mdhfvrp.getValue(f[i][j]) << "  B[" << j << "]->" << mdhfvrp.getValue(b[j]) << /*" - " << mdhfvrp.getValue(b[j]) <<*/ '\n';
  //             // std::cout << "X[" << i << "][" << j << "]: " << mdhfvrp.getValue(X[i][j][k][d][l]) << " f-> " << mdhfvrp.getValue(f[i][j]) << '\n';
  //           }
  //         }
	// 			}
	// 		}
	// 	}
	// }

  // std::cout << '\n';
  // // Print only Route
  // int position = data.n+1;
  // for (size_t i = 1; i <= limit; i++) {
  //   std::cout << data.route[position][1] << "  ";
  //   position = data.route[position][2];
  // }
  // std::cout << data.route[position][1] << '\n';

  // // Print Route and B
  // position = data.n+1;
  // for (size_t i = 1; i <= limit; i++) {
  //   // std::cout << data.route[position][1] << " (B(" << data.route[position][1] << "): " << mdhfvrp.getValue(b[data.route[position][2]]) << ")  ";
  //   position = data.route[position][2];
  // }
  // // std::cout << data.route[position][1] << " (B(" << data.route[position][1] << "): " << mdhfvrp.getValue(b[data.route[position][2]]) << ")  \n";

  // std::cout << '\n';
  // position = data.n+1;
  // double tme = 0;
  // int cl1, cl2;
  // for (size_t i = 1; i <= limit; i++) {
  //   cl1 = data.route[position][1];
  //   cl2 = data.route[position][2];
  //   std::cout << cl1 << "(" << data.timeWindow[cl1].start << " " << data.timeWindow[cl1].end << ")";
  //   std::cout << "[" << tme << "] - ";
  //   std::cout << cl2 << "(" << data.timeWindow[cl2].start << " " << data.timeWindow[cl2].end << ")";
  //   tme += data.matrixTime[cl1][cl2];
  //   std::cout << "[" << tme << "]" << '\n';
  //   position = data.route[position][2];
  // }


  // std::cout << '\n' << "B" << '\n';
  // for (size_t i = 1; i <= data.n; i++) {
  //   std::cout << i << " : " << mdhfvrp.getValue(b[i]) << "\n";
  // }

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
