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

    // timer 
    chrono::steady_clock::time_point start; 

    // colonnes 
    int taille_col; 

    modele(const Instance& inst_, const vector<vector<int>>& cols, const vector<bool>& clients_places);

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

    /**
     * Reconstruit la solution de prog_dyn_sac à partir du tableau
     * liaisons est un vecteur permettant de se rappeler quel client est situé à quel 
     * index dans le tableau. 
     */
    vector<int> reconstruit_solution(int j, const vector<int>& liaisons, const vector<vector<pair<double,int>>>& tab); 

    /**
     * Résoud le pricing grâce à un sac à dos dynamique où :
     * _ poids : demande des clients
     * _ profits : coûts réduits des clients
     * _ taille du sac : capacité de l'entrepôt j
     * _ Renvoie un vecteur binaire indiquant quel client est affecté à j
     */
    vector<int> prog_dyn_sac(int j, const Duales& donnees_duales); 

    /* ---------------- PARTIE GENERATION COL ---------------- */

    /**
     * Lance la génération de colonnes en utilisant l'implémentation gurobi
     * du pricing. 
     */
    void gen_col();

    /**
     * Lance la génération de colonnes en utilisant l'algorithme de programmation dynamique 
     * pour le pricing 
     */
    void gen_col_DP(); 

    /* ---------------- PARTIE GENERATION STABILISATION ---------------- */

    void update_sep(Duales& sep, const Duales& in, const Duales& out, double alpha); 

    /**
     * Lance une génération de colonnes avec stabilisation in-out et utilisation 
     * de programmation dynamique pour résoudre pricing
     */
    void gen_col_stabilization(int alpha); 

    ~modele();

    // fonctions temps 
    void lance_timer(); 
    double stop_timer(); 
    
}; 

