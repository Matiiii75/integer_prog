#pragma once 

#include "operators.hpp"
#include "glouton_col.hpp"
#include "gurobi_c++.h"

#include <sstream> 
#include <chrono> 

using namespace std; 

struct Duales {
    vector<double> duales_des_clients; 
    double theta; 
}; 

struct modele {

    GRBEnv* env; 
    GRBModel* model; 

    Instance inst; 
    vector<vector<double>> matrice_distances; 

    // contraintes 
    GRBConstr max_facility; 
    vector<GRBConstr> tout_client_assigne; 

    // colonnes 
    int taille_col; 

    modele(const Instance& inst_, const vector<vector<int>>& cols);

    /* ---------------- PARTIE UTILITAIRES ---------------- */

    double calcul_cout_colonne(const vector<int>& colonne);

    void ajoute_colonne(vector<int> colonne_du_pricing);

    double theta();

    void calcul_distances(); 

    vector<double> duales_des_clients(); 

    void optimize();

    double obj();

    vector<int> pricing(int j, const Duales& donnees_duales);

    /* ---------------- PARTIE PROGRAMMATION DYNAMIQUE ---------------- */

    vector<int> reconstruit_solution(int j, const vector<int>& liaisons, const vector<vector<pair<double,int>>>& tab); 

    vector<int> prog_dyn_sac(int j, const Duales& donnees_duales); 

    /* ---------------- PARTIE GENERATION COL ---------------- */

    void gen_col();

    void gen_col_DP(); 

    /* ---------------- PARTIE GENERATION STABILISATION ---------------- */

    void update_sep(Duales& sep, const Duales& in, const Duales& out, double alpha); 

    void gen_col_stabilization(); 

    ~modele();

}; 

