#pragma once 

#include "operators.hpp"
#include "gurobi_c++.h"
#include <sstream> 
#include <chrono> 

using namespace std; 

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

    modele(const Instance& inst_);

    double calcul_cout_colonne(const vector<int>& colonne);

    void ajoute_colonne(vector<int> colonne_du_pricing);

    double theta();

    vector<double> duales_des_clients(); 

    void optimize();

    double obj();

    vector<int> pricing(int j, const vector<double>& duales, const vector<vector<double>>& distances);

    void gen_col();

    vector<int> reconstruit_solution(int j, const vector<vector<pair<double,int>>>& tab); 

    vector<int> prog_dyn_sac(int j, const vector<double>& duales, const vector<vector<double>>& distances); 

    vector<double> couts_reduits_j(int j, const vector<double>& duales); 


    // DEBUT IMPLE TEST PROG DYN

    vector<int> reconstruit_solution_TEST(int j, const vector<int>& liaisons, const vector<vector<pair<double,int>>>& tab); 

    vector<int> prog_dyn_TEST(int j, const vector<double>& duales, const vector<vector<double>>& distances); 

    void gen_col_DP_TEST(); 

    // DEBUT IMPLE TEST PROG DYN

    void gen_col_DP(); 

    ~modele();

}; 

