#include "operators.hpp"
#include <sstream> 
#include "gurobi_c++.h"

using namespace std; 

// Q7 : implémentation du modèle de Q6. 

void solve(const Instance& inst) {

    GRBEnv env = GRBEnv(false); // desactive ecritures
    env.start(); 
    env.set(GRB_IntParam_OutputFlag, 1); 
    GRBModel model = GRBModel(env); // creation modèle

    // déclaration des variables

    // x

    vector<vector<GRBVar>> x(inst.C,vector<GRBVar>(inst.F)); 
    for(int i = 0; i < inst.C; ++i) {
        for(int j = 0; j < inst.F; ++j) {

            stringstream ss; 
            ss << "x" << i << "," << j; 
            x[i][j] = model.addVar(0.0, 1.0,0.0, GRB_BINARY, ss.str()); 

        }
    }

    // y 

    vector<GRBVar> y(inst.F); 

    for(int j = 0; j < inst.F; ++j) {

        stringstream ss2;
        ss2 << "y" << j; 
        y[j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ss2.str()); 

    }

    // objectif 

    GRBLinExpr obj = 0; 
    for(int i = 0; i < inst.C; ++i) {
        for(int j = 0; j < inst.F; ++j) {

            obj += x[i][j] * pow( pow( inst.loc_client[i].x - inst.loc_f[j].x , 2) 
                + pow(inst.loc_client[i].y - inst.loc_f[j].y , 2), 0.5);

        }
    }
    model.setObjective(obj, GRB_MINIMIZE); 

    // contraintes 

    // chaque client a un entrepot assigné 

    for(int i = 0; i < inst.C; ++i) {
        GRBLinExpr c1 = 0; 
        for(int j = 0; j < inst.F; ++j) {
            c1 += x[i][j]; 
        }
        model.addConstr(c1 == 1); 
    }

    // la capacité de chaque entrepot respectée

    for(int j = 0; j < inst.F; ++j) {
        GRBLinExpr c2 = 0; 
        for(int i = 0; i < inst.C; ++i) {
            c2 += x[i][j]*inst.dc[i]; 
        }
        model.addConstr(c2 <= inst.uf[j]*y[j]); 
    }

    // p entrepots ouverts 

    GRBLinExpr c3 = 0; 
    for(int j = 0; j < inst.F; ++j) {
        c3 += y[j]; 
    }
    model.addConstr(c3 == inst.p); 


    /* CONFIGURATIONS DES RESULTATS */

    model.set(GRB_DoubleParam_TimeLimit, 600.0); 
    model.set(GRB_IntParam_Threads, 1);
    // model.write("model.lp"); // décommenter pr écriture du model dans un fichier

    // Résolution 
    model.optimize(); 

    // récuperation et affichage résultats

    int status = model.get(GRB_IntAttr_Status); 

    if(status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount)>0))
    {   
        cout << "solution trouvee en " << model.get(GRB_DoubleAttr_Runtime) << " seconds" << endl;
        cout << "valeur objective : " << model.get(GRB_DoubleAttr_ObjVal) << endl; 
        for(int i = 0; i < inst.C; ++i) {
            for(int j = 0; j < inst.F; ++j) {
                cout << "x" << i << "," << j << " : " << x[i][j].get(GRB_DoubleAttr_X) << endl;
            }
        }

        // récupérage de la solution : 

        Solution sol(inst.C); 
        for(int i = 0; i < inst.C; ++i) {
            for(int j = 0; j < inst.F; ++j) {
                if (x[i][j].get(GRB_DoubleAttr_X)) {
                    sol[i] = inst.loc_f[j]; 
                }
            }
        }
        cout << sol; 

        // verif sol 

        // double val;
        // val = inst.checker(sol); décommenter pour checker 
        
        // écris la solution dans un fichier 

        ofstream ecrit("../solutions/solution.sol"); 
        ecrit << sol; 
        ecrit.close(); 
        

        inst.dessine_sol(sol);  // dessine l'instance en svg dans le dossier dessins


    } else {
        cerr << "PROBLEME STATUS = " << status << endl;
    }

}

/*
int main(int argc, char* argv[]) {

    Instance inst; 
    ifstream file(argv[1]); 
    if(file.is_open()) {
        file >> inst; 
    }
    else cout << "erreur ouverture fichier" << endl;

    solve(inst); 

    return 0; 
}
*/
