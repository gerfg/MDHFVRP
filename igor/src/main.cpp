#include <ilcplex/ilocplex.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <cmath>
#include "Data.h"
#include "MyLazyCallback.h"
#include "MyCutCallback.h"
#include "MyBranchCallback.h"

using namespace std;

void solve(Data& data){
	int BigM = 99999;

	IloEnv env;
	IloModel model(env, "MDHDARP");

	// **** VARIABLES ****

	//Decision variable
	IloArray <IloArray < IloArray< IloBoolVarArray > > > X (env, data.V.size()); //Vertices

	for(int i = 0; i < data.V.size(); ++i){
		X[i] = IloArray <IloArray<IloBoolVarArray> > (env, data.V.size()); //Vertices
		for(int j = 0; j < data.V.size(); ++j){

			if( data.A_[i][j]) continue; // If arc i to j is invalid

			X[i][j] = IloArray<IloBoolVarArray> (env, data.v); //Types of cars
			for(int k = 0; k < data.v; ++k){
				X[i][j][k] = IloBoolVarArray(env, data.m); //Depots
				for(int d = 0; d < data.m; ++d){
					char name[20];
					sprintf(name, "X(%d,%d,%d,%d)", i, j, k, d);
					X[i][j][k][d].setName(name);
					model.add(X[i][j][k][d]);
				}
			}
		}
	}

	//Flow variable
	IloArray <IloNumVarArray>  f(env, data.V.size());
	for(int i = 0; i < data.V.size(); ++i){
		f[i] = IloNumVarArray(env, data.V.size(), 0, IloInfinity);
		for(int j = 0; j < data.V.size(); ++j){
			char name[20];
			sprintf(name, "f(%d,%d)", i, j);
			f[i][j].setName(name);
			model.add(f[i][j]);
		}
	}

	//Time variable
	IloArray <IloArray <IloNumVarArray> > b(env, data.V.size());
	for (int i = 0; i < data.V.size(); ++i){
		b[i] = IloArray<IloNumVarArray>(env, data.v);
		for (int k = 0; k < data.v; ++k){
			b[i][k] = IloNumVarArray(env, data.m, 0, IloInfinity);
			for(int d = 0; d < data.m; ++d){
				char name[20];
				sprintf(name, "b(%d,%d,%d)", i, k, d);
				b[i][k][d].setName(name);
				model.add(b[i][k][d]);
			}
		}
	}

	//Ride time
	IloArray <IloArray <IloNumVarArray> > l(env, data.V.size());
	for (int i = 0; i < data.n; ++i){
		l[i] = IloArray<IloNumVarArray>(env, data.v);
		for (int k = 0; k < data.v; ++k){
			l[i][k] = IloNumVarArray(env, data.m, 0, IloInfinity);
			for(int d = 0; d < data.m; ++d){
				char name[20];
				sprintf(name, "l(%d,%d,%d)", i, k, d);
				l[i][k][d].setName(name);
				model.add(l[i][k][d]);
			}
		}
	}

	// **** OBJECTIVE FUNCTION ****
	IloExpr OBJ(env);
	for (int a = 0; a < data.A.size(); ++a){
		for (int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				int i = data.A[a].first;
				int j = data.A[a].second;
				OBJ += data.c[i][j] * X[i][j][k][d];
			}
		}
	}
	model.add(IloMinimize(env, OBJ));

	// ***** CONSTRAINTS *****
	//Every request is served once (2)
	for (int i = 0; i < data.n; ++i){
		IloExpr expr(env);
		for(int a = 0; a < data.A.size(); ++a){
			int u = data.A[a].first;
			int v = data.A[a].second;

			if(v != i) continue; //It is necessary only arcs reaching i

			for(int k = 0; k < data.v; ++k){
				for(int d = 0; d < data.m; ++d){
					expr += X[u][v][k][d];
				}
			}
		}

		IloRange r = (expr == 1);
		char c[100];
		sprintf(c, "c1_%d", i);
		r.setName(c);
		model.add(r);
	}


	//Same vehicle serves the pickup and delivery (3)
	for(int i = 0; i < data.n; ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				IloExpr expr1(env);
				IloExpr expr2(env);
				for(int a = 0; a < data.Apd.size(); ++a){

					int u = data.Apd[a].first;
					int v = data.Apd[a].second;

					if(v == i){ //if an arc reaches i
						expr1 += X[u][v][k][d];
					}
					if(v == (data.n+i)){ //if an arc reaches n+i
						expr2 += X[u][v][k][d];
					}
				}

				expr1 += X[2*data.n+d][i][k][d]; //Node from delivery to depot
				//expr2 += X[(data.n+i)][2*data.n+d][k][d]; //Node from delivery to depot

				IloRange r = ( (expr1-expr2) == 0);
				char c[100];
				sprintf(c, "c2_%d,%d,%d", i, k, d);
				r.setName(c);
				model.add(r);
			}
		}
	}
	
	//The same vehicle that enters a node leaves the node (4)
	for(int i = 0; i < (2*data.n); ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				IloExpr expr1(env);
				IloExpr expr2(env);

				for(int a = 0; a < data.Apd.size(); ++a){
					int u = data.Apd[a].first;
					int v = data.Apd[a].second;

					if(u == i){ //if an arc leaves i
						expr1 += X[u][v][k][d];
					}
					if(v == i){ //if an arc enters i
						expr2 += X[u][v][k][d];
					}
				}

				if(i >= data.n){	
					expr1 += X[i][2*data.n+d][k][d]; //arc from delivery to depot
				}

				if(i < data.n){
					expr2 += X[2*data.n+d][i][k][d]; //arc drom depot to pickup
				}

				IloRange r = ( (expr1-expr2) == 0);
				char c[100];
				sprintf(c, "c3_%d,%d,%d", i, k, d);
				r.setName(c);
				model.add(r);
			}
		}
	}

	//The number of vehicles of type k that leaves depot d is the same that ends in depot d and is at most Ukd (5)
	for(int k = 0; k < data.v; ++k){
		for(int d = 0; d < data.m; ++d){
			IloExpr expr1(env);
			IloExpr expr2(env);
			for(int a = 0; a < data.Adm.size(); ++a){
				int u = data.Adm[a].first;
				int v = data.Adm[a].second;
				expr1 += X[u][v][k][d];
			}
			for(int a = 0; a < data.Amp.size(); ++a){
				int u = data.Amp[a].first;
				int v = data.Amp[a].second;
				expr2 += X[u][v][k][d];
			}

			IloRange r1 = ( (expr1 - expr2) == 0);
			IloRange r2 = (expr1 <= data.U[k][d]);

			char c1[100];
			sprintf(c1, "c4_%d,%d", k, d);
			r1.setName(c1);

			char c2[100];
			sprintf(c2, "c5_%d,%d", k, d);
			r2.setName(c2);


			model.add(r1);
			model.add(r2);
		}
	}

	//Remove subtour size 2 (6)
	for(int i = 0; i < (2*data.n); ++i){
		for(int j = i+1; j < (2*data.n); ++j){
			if(!data.A_[i][j] && !data.A_[j][i]){
				IloExpr expr1(env);
				for(int k = 0; k < data.v; ++k){
					for(int d = 0; d < data.m; ++d){
						expr1 += X[i][j][k][d] + X[j][i][k][d];						
					}
				}
				IloRange r = ( expr1 <= 1);
				char c[100];
				sprintf(c, "c6_%d,%d", i, (i+data.n));
				r.setName(c);
				model.add(r);
			}				
		}		
	}

	
	//Setting and checking visit time (7) (8)
	for(int i = 0; i < (2*data.n); ++i){
		for(int j = 0; j < data.V.size(); ++j){
			if(!data.A_[i][j]){
				for(int k = 0; k < data.v; ++k){
					for(int d = 0; d < data.m; ++d){
						IloExpr expr(env);
						expr = b[i][k][d] + data.t[i][j] - BigM * (1 - X[i][j][k][d]); //(7)

						IloRange r = ( (b[j][k][d] - expr) >= 0);
						char c[100];
						sprintf(c, "c7_%d,%d,%d,%d", i, j, k, d);
						r.setName(c);
						model.add(r);
					}
				}
			}			
		}
	}
	//(8)
	for(int i = 0; i < data.n; ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				IloExpr expr(env);
				expr = b[(data.n+i)][k][d];

				IloRange r = (expr <= data.T[i]);
				char c[100];
				sprintf(c, "c8_%d,%d,%d", i, k, d);
				r.setName(c);
				model.add(r);
			}
		}
	}

	//Setting and checking ride time: (9) (10)
	for (int i = 0; i < data.n; ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				IloExpr expr1(env);
				IloExpr expr2(env);
				expr1 = b[(data.n+i)][k][d] - b[i][k][d];
				expr2 = l[i][k][d];

				IloRange r1 = ( (expr2 - expr1) == 0); //9
				char c1[100];
				sprintf(c1, "c9_%d,%d,%d", i, k, d);
				r1.setName(c1);
				model.add(r1);

				IloRange r2 = (expr2 <= data.L[i]); //10
				char c2[100];
				sprintf(c2, "c10_%d,%d,%d", i, k, d);
				r2.setName(c2);
				model.add(r2);

			}
		}
	}

	//The amount of flow inside in node less the amount that comes out must be equals to the demand (10)	
	for(int i = 0; i < data.V.size(); ++i){
		IloExpr FC1(env);
		IloExpr FC2(env);
		for(int j = 0; j < data.V.size(); ++j){
			if(data.A_[i][j]) continue;
			FC1 += f[i][j];
			FC2 += f[j][i];
		}

		IloRange r;

		if      ( i >= (2*data.n) ) r = ( (FC1-FC2) == 0 ); // 0 for deposit
		else if (i < data.n) 		r = ( (FC1-FC2) == data.q[i] );
		else 	  					r = ( (FC1-FC2) == -(data.q[i - data.n]) ); //Negative capacity for delivery

		char c[100];
		sprintf(c, "c11_%d", i);
		r.setName(c);
		model.add(r);
	}

	//The max flow in route must be less than capacity in X[i][j][k][d]
	for(int a = 0; a < data.A.size(); ++a){
		IloExpr FC3(env);
		int u = data.A[a].first;
		int v = data.A[a].second;
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				FC3 += data.Q[k][d] * X[u][v][k][d];
			}
		}

		IloRange r = ( (f[u][v] - FC3) <= 0);
		char c[100];
		sprintf(c, "c12_%d,%d", u, v);
		r.setName(c);
		model.add(r);

	}
	
	IloCplex mdhdarp(model);
	mdhdarp.exportModel("mdhdarp.lp");
	mdhdarp.setParam(IloCplex::Threads, 1);

	const IloArray<IloArray<IloArray<IloBoolVarArray>> >& x_ref = X;

	// ***** CALLBACKS *****
	MyLazyCallback* lazyCbk = new (env) MyLazyCallback(env, x_ref, &data); 
    mdhdarp.use(lazyCbk);
    MyCutCallback* cutCbk = new (env) MyCutCallback(env, x_ref, &data); 
    mdhdarp.use(cutCbk);
    MyBranchCallback* branchCbk = new (env) MyBranchCallback(env);
    mdhdarp.use(branchCbk);

	mdhdarp.solve();
	double value = mdhdarp.getObjValue();

	for(int k = 0; k < data.v; ++k){
		printf("\nVehicle number %d\n", k);
		for(int d = 0; d < data.m; ++d){
			printf("Depot number %d\n", d);
			for(int i = 0; i < data.V.size(); ++i){
				for(int j = 0; j < data.V.size(); ++j){
					if( !data.A_[i][j] ){
						if(mdhdarp.getValue(X[i][j][k][d]) > 0.0001){
							//printf("%d to %d,", i, j);
							//printf("time: %.2lf, vehicle capacity: %.2lf\n", mdhdarp.getValue(b[j][k][d]), mdhdarp.getValue(f[i][j]));
							printf("X[%d][%d]: %.2lf ", i,j,mdhdarp.getValue(X[i][j][k][d]));
						}
							
					} 
				}
				printf("\n");
			}
		}
	}

	cout << "\n\n\n";
	for(int k = 0; k < data.v; ++k){
		printf("\nVehicle number %d\n", k);
		for(int d = 0; d < data.m; ++d){
			printf("Depot number %d\n", d);
			for (int i = 0; i < data.V.size(); ++i){
				for(int j = 0; j < data.V.size(); ++j){	
					if(!data.A_[i][j]){
						if(mdhdarp.getValue(X[i][j][k][d]) > 0.0001) 
							printf("f[%d][%d]: %.2lf ", i,j, mdhdarp.getValue(f[i][j]));
					}			
				}
				printf("\n");
			}
			printf("\n\n\n");
		}
	}
	/*
	for (int i = 0; i < data.V.size(); ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				printf("\nb[%d][%d][%d]: %lf ", i,k,d,mdhdarp.getValue(b[i][k][d]));
			}
			printf("\n");
		}
		printf("\n");
	}

	for (int i = 0; i < data.n; ++i){
		for(int k = 0; k < data.v; ++k){
			for(int d = 0; d < data.m; ++d){
				printf("\nl[%d][%d][%d]: %lf ", i,k,d,mdhdarp.getValue(l[i][k][d]));
			}
			printf("\n");
		}
		printf("\n");
	}*/

	cout << "\n\nOBJ " << value << endl;
}

int main( int argc, char **argv ) {
	Data data(argv[1]);
	data.print();
	solve(data);
}