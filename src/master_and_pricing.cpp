#include "operators.hpp"
#include "gurobi_c++.h"
#include <sstream> 

using namespace std; 

/* Fonction qui implémente le master. Elle prend en entrée l'instance 
ainsi que l'ensemble L des colonnes. Je pense qu'une colonne p peut etre de la forme : 
p = (e1, ...., eF | c1, .... , cC) où ei = entrepot i et ci = client i. ei = 1 si entrepot i
est dans le point p et ci = 1 si client i dans point p. 
Pour accéder a l'information si client i est dans p, avec l'entrepot j, il faut faire : 
p_{F+i} et pour entrepot j : p_{j} 
*/

vector<double> solve_master(const Instance& inst, const vector<vector<int>>& L) {

    GRBEnv env = GRBEnv(false); // desactive ecritures
    env.start(); 
    env.set(GRB_IntParam_OutputFlag, 1); 
    GRBModel model = GRBModel(env); // creation modèle

    // variables lambda 

    int SL = L.size(); 

    vector<GRBVar> lambda(SL); 
    for(int i = 0; i < SL; ++i) {

        stringstream ss; 
        ss << "lambda" << i; 
        lambda[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, ss.str()); 

    }


    // fonction objectif 

    GRBLinExpr obj = 0; 
    for(int p = 0; p < SL; ++p) {
        for(int i = 0; i < inst.C; ++i) {
            for(int j = 0; j < inst.F; ++j) {

                obj += L[p][inst.F-1+i]*L[p][j]*lambda[p] * pow( pow( inst.loc_client[i].x - inst.loc_f[j].x , 2) 
                + pow(inst.loc_client[i].y - inst.loc_f[j].y , 2), 0.5);
                // L[p][inst.F-1+i] vaut 1 si le client i est dans p. O sinon. 
            }
        }
    }
    model.setObjective(obj, GRB_MINIMIZE); 


    // pour stocker les contraintes afin de pouvoir accéder aux duales + tard, 
    // je créer un array

    GRBConstr* contraintes = new GRBConstr[inst.C + 2]; 

    // contraintes : tout client est assigné exactement une fois

    for(int i = 0; i < inst.C; ++i) {
        GRBLinExpr c1 = 0; 
        for(int j = 0; j < inst.F; ++j) {
            for(int p = 0; p < SL; ++p) {
                c1 += L[p][inst.F-1+i]*L[p][j]*lambda[p]; 
            }
        }
        contraintes[i] = model.addConstr(c1 == 1); 
    }


    // p entrepots 

    GRBLinExpr c2 = 0; 
    for(int j = 0; j < inst.F; ++j) {
        for(int p = 0; p < SL; ++p) {
            c2 += L[p][j]*lambda[p]; 
        }
    }
    contraintes[inst.C] = model.addConstr(c2 == inst.p); // est ce quil faut mettre <= du coup ??????????????????


    // somme lambda == 1 ------> peut etre inutile par redondance avec c1 ??????????

    GRBLinExpr c3 = 0; 
    for(int p = 0; p < SL; ++p) {
        c3 += lambda[p]; 
    }
    contraintes[inst.C+1] = model.addConstr(c3 == 1);


    /* CONFIGURATIONS DES RESULTATS */

    model.set(GRB_DoubleParam_TimeLimit, 600.0); 
    model.set(GRB_IntParam_Threads, 1);
    // model.write("model.lp"); // décommenter pr écriture du model dans un fichier

    // Résolution 
    model.optimize(); 

    // récuperation et affichage résultats

    int status = model.get(GRB_IntAttr_Status);

    vector<double> duales; // il faudrait ajouter une sécurité au cas ou on rentre pas dans le if en dessous

    if(status == GRB_OPTIMAL || (status == GRB_TIME_LIMIT && model.get(GRB_IntAttr_SolCount)>0)){

        // si la résolution s'est correctement déroulée, on peut extraire les valeurs duales des contraintes 
        // pas sur de ce qui suit :  

        for(int i = 0; i < inst.C+2; ++i) {
            duales.push_back(contraintes[i].get(GRB_DoubleAttr_Pi)); 
        }
    }

    // renvoie les duales 
    return duales; 
} 

// doit renvoyer des valeurs duales pour les variables. 



/* Implémentation du pricing pb. 
Il prend en entrée l'ensemble des colonnes L, l'instance ainsi les duales trouvées par master
Il modifie L si il trouve une colonne a ajouter.
il retourne également un bool permettant de savoir si on a ou non ajouté une colonne. 
Si bool = false, alors on a convergé   
*/

bool pricing(const Instance& inst, const vector<double>& duales, vector<vector<int>>& L) {

    bool ajout_colonne = false; 

    GRBEnv env = GRBEnv(false); // desactive ecritures
    env.start(); 
    env.set(GRB_IntParam_OutputFlag, 1); 
    GRBModel model = GRBModel(env); // creation modèle

    // déclaration variables

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
                + pow(inst.loc_client[i].y - inst.loc_f[j].y , 2), 0.5) 
                - duales[i]*x[i][j]; 
                // duales[i]*x[i][j] : contribution duale
        }
    }
    for(int j =0; j < inst.F; ++j) {
        obj -= duales[inst.C]*y[j]; // duales[inst.C] correspond à la duale theta dans mon modele
    }
    obj -= duales[inst.C+1]; // et alpha 
    model.setObjective(obj, GRB_MINIMIZE); 


    // contrainte de capacité  

    for(int j = 0; j < inst.F; ++j) {
        GRBLinExpr c1 = 0; 
        for(int i = 0; i < inst.C; ++i) {
            c1 += x[i][j]*inst.dc[i]; 
        }
        c1 -= inst.uf[j]*y[j]; 
        model.addConstr(c1 <= 0); 
    }


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
        
        double val_opt = model.get(GRB_DoubleAttr_ObjVal); 
        // si val_opt < 0 : construire la nouvelle colonne a ajouter
        if(val_opt < 0) {
            vector<int> nouvelle_colonne(inst.F + inst.C); 
            for(int j = 0; j < inst.F; ++j) {
                // normalement, on doit avoir 1 seul entrepot a chaque fois 
                if(y[j].get(GRB_DoubleAttr_X) > 0.5) {
                    cout << "y" << j << " = " << 1 << endl;
                    nouvelle_colonne[j] = 1; 
                } else {
                    nouvelle_colonne[j] = 0; 
                }
            } 
            // récupérage x 
            for(int i = 0; i < inst.C; ++i) {
                // si on a bien un seul y valant 1, on pourra supprimer cette boucle
                for(int j = 0; j < inst.F; ++j) {

                    if(x[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                        nouvelle_colonne[inst.F-1+i] = 1; 
                    } else {
                        nouvelle_colonne[inst.F-1+i] = 0; 
                    }
                }
            }
            L.push_back(nouvelle_colonne); // ajout de la nouvelle col
            ajout_colonne = true; 
        }
    }
    
    return ajout_colonne; 
}



// fonction qui boucle sur le master et le pricing jusqu'à la convergence. 
// peut etre ajouter une limite de temps ou des securités si ça converge pas ? jpense pas 

void boucle_master_pricing(const Instance& inst) {

    bool ajout_colonne = true; 
    vector<double> duales; 
    vector<vector<int>> L; 

    // création vecteur plein de zéros pour premiere colonne nulle a chier mais ok
    vector<int> first_col; 
    for(int i = 0; i < inst.C + inst.F; ++i) {
        first_col.push_back(0); 
    }
    L.push_back(first_col); 

    while(ajout_colonne) {

        duales = solve_master(inst, L); 
        ajout_colonne = pricing(inst, duales, L); 

    }

    // affichage colonnes (L) : 
    for(int i = 0; i < L.size(); ++i) {
        cout << "col " << i << " : "; 
        for(int j = 0; j < L[i].size(); ++j) {
            cout << L[i][j] << ", "; 
        }
        cout << endl;
    }
}


int main(int argc, char* argv[]) {

    Instance inst; 
    ifstream file(argv[1]); 
    if(file.is_open()) file >> inst; 
    else cout << "erreur ouverture file" << endl;
    boucle_master_pricing(inst); 

    return 0; 
}



