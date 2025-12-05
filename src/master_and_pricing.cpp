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



/* implémentation modele */

struct modele {

    GRBEnv* env; 
    GRBModel* model; 

    Instance inst; 

    // contraintes 
    GRBConstr max_facility; 
    vector<GRBConstr> tout_client_assigne; 

    // colonnes 
    int taille_col; 
    vector<vector<int>> cols; 

    modele(const Instance& inst_) : inst(inst_) {

        taille_col = inst.C + inst.F; 
        
        // environnement 

        env = new GRBEnv(true); 
        env->set(GRB_IntParam_LogToConsole, 0); // désactive les logs
        env->start(); 
        model = new GRBModel(*env); 

        // ajout d'une colonne initiale où tous les clients sont selectionnés 
        // cette colonne aura un coefficient >grand pour s'assurer qu'on ne la selectionne 
        // plus ensuite. 

        GRBVar t; 
        model->addVar(0.0, INFINITY, 1e9, GRB_CONTINUOUS, "t");

        // contrainte pas plus de p entrepot
        max_facility = model->addConstr(0.0, GRB_LESS_EQUAL, inst.p); 

        // tout client servis 
        for(int i = 0; i < inst.C; ++i) {
            tout_client_assigne.push_back(model->addConstr(t == 1)); 
        }

        model->optimize(); 
    }

    // étant donné une colonne (entrepot_ouvert, c1, ... , c|C|), renvoie son cout dans l'obj
    double calcul_cout_colonne(const vector<int>& colonne) {

        double val_col = 0; 
        for(int i = 1; i < inst.C+1; ++i) {
            val_col += colonne[i]*dist(inst, i-1, colonne[0]); 
            // colonne[0] contient l'entrepot ouvert j auxquels sont associés les clients actifs
        }

        return val_col; 
    }


    // prend en entrée colonne_du_pricing
    void ajoute_colonne(vector<int> colonne_du_pricing) {

        double distance_totale = calcul_cout_colonne(colonne_du_pricing); 

        GRBColumn col; 
        // ajout a la contrainte sur le nb de facility
        col.addTerm(1, max_facility); 

        // ajoute la variable a la contrainte sur l'affectation de chaque client 
        for(int i = 1; i < inst.C+1; ++i) {
            col.addTerm(colonne_du_pricing[i], tout_client_assigne[i-1]); 
            /* 
            col.addTerm (X, Y[i]) est la commande qui dit : 
            la nouvelle variable ajoutée aura un coefficient de X dans la contrainte Y[i]
            */
        }

        model->addVar(0.0, INFINITY, distance_totale, GRB_CONTINUOUS, col); 
    }

    // récupérer val duale theta associée a ctrt <= p
    double theta() {
        return max_facility.get(GRB_DoubleAttr_Pi); 
    }

    // récupérer les Pi_i (val duale associées aux ctrt d'affectations pr chaque client)
    vector<double> duales_des_clients() {

        vector<double> duales; 

        for(auto& contrainte : tout_client_assigne) {
            duales.push_back(contrainte.get(GRB_DoubleAttr_Pi)); 
        }

        return duales; 
    }

    void optimize() {
        model->set(GRB_IntParam_Method, 0); 
        model->optimize(); 
    }

    // récupérer l'obj
    double obj() {
        return model->get(GRB_DoubleAttr_ObjVal); 
    }

    // résoud le pricing. Prends en entrée j : la facility considérée. Renvoie une colonne
    vector<int> pricing(int j) {

        GRBModel pricing_model(env); 

        GRBLinExpr contraintes;
        vector<GRBVar> x; 

        vector<double> duales = duales_des_clients(); // récupère les duales

        for(int i = 0; i < inst.C; ++i) {
            x.push_back(pricing_model.addVar(0.0, 1.0, dist(inst, i, j) - duales[i], GRB_INTEGER));
            contraintes += x.back()*inst.dc[i]; 
        }

        pricing_model.optimize(); 

        double pricing_obj = pricing_model.get(GRB_DoubleAttr_ObjVal); 
        // si l'obj de pricing >= 0, on renvoie vecteur vide; Permettra de détecter que ça a convergé 
        if(pricing_obj + theta() >= -1e-6) return {}; 

        // on créer la colonne 
        vector<int> colonne; 
        colonne.push_back(j); 
        for(int i = 0; i < inst.C; ++i) {
            if(x[i].get(GRB_DoubleAttr_X) > 0.5) colonne.push_back(1); 
            else colonne.push_back(0); 
        }

        return colonne; 
    }


    // boucle pour la génération de colonne

    void gen_col() {

        while(true) {   

            vector<vector<int>> col_a_ajouter; 
            for(int j = 0; j < inst.F; ++j) {
                auto col = pricing(j); 
                if(col.empty()) break;
                col_a_ajouter.push_back(col); 
            }

            for(auto& col : col_a_ajouter) 
                ajoute_colonne(col); 
            optimize(); 

        }
    }


    // destructeur modele 
    ~modele() {
        delete env; 
        delete model; 
    }
}; 


int main(int argc, char* argv[]) {

    Instance inst; 
    ifstream file(argv[1]); 
    if(file.is_open()) file >> inst; 
    else cerr << "erreur ouverture fichier" << endl;

    modele m(inst); 
    m.gen_col();
    
    cout << "relaxation LP maitre : " << m.obj() << endl;

    return 0; 
}
