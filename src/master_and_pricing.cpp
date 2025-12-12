#include "master_and_pricing.hpp"

using namespace std; 

modele::modele(const Instance& inst_, const vector<vector<int>>& cols) : inst(inst_) {

    taille_col = inst.C + inst.F; 
    calcul_distances(); // calcul préalable des distances 

    // environnement 
    env = new GRBEnv(true); 
    env->set(GRB_IntParam_LogToConsole, 0); // désactive les logs
    env->start(); 
    model = new GRBModel(*env); 

    // DECLARATION DES CONTRAINTES
    // contrainte pas plus de p entrepot
    max_facility = model->addConstr(0.0, GRB_LESS_EQUAL, inst.p);
    // contraintes client 
    tout_client_assigne.resize(inst.C);
    for(int i = 0; i < inst.C; ++i) {
        tout_client_assigne[i] = model->addConstr(0.0, GRB_EQUAL, 1.0);
    }

    if(cols.empty()) { // si on a pas généré de colonnes initiales -> BIG M
        // on les ajoutes pr chaque contrainte 
        for(int i = 0; i < inst.C; ++i) {
            GRBColumn col; 
            col.addTerm(1.0, tout_client_assigne[i]); 
            model->addVar(0.0, INFINITY, 1e9, GRB_CONTINUOUS, col);  
        }
    }
    else 
    {   // si on en a généré, on les ajoute
        for(auto& col : cols) ajoute_colonne(col); 
    }

    model->optimize(); 

}

// étant donné une colonne (entrepot_ouvert, c1, ... , c|C|), renvoie son cout dans l'obj
double modele::calcul_cout_colonne(const vector<int>& colonne) {

    double val_col = 0; 
    for(int i = 1; i < inst.C+1; ++i) {
        val_col += colonne[i]*matrice_distances[i-1][colonne[0]]; 
        // colonne[0] contient l'entrepot ouvert j auxquels sont associés les clients actifs
    }

    return val_col; 
}

// prend en entrée colonne_du_pricing
void modele::ajoute_colonne(vector<int> colonne_du_pricing) {

    double distance_totale = calcul_cout_colonne(colonne_du_pricing); 

    GRBColumn col; 
    // ajout a la contrainte sur le nb de facility
    col.addTerm(1, max_facility); 

    // ajoute la variable a la contrainte sur l'affectation de chaque client 
    for(int i = 1; i < inst.C+1; ++i) {
        col.addTerm(colonne_du_pricing[i], tout_client_assigne[i-1]); 
        // col.addTerm (X, Y[i]) est la commande qui dit : 
        // la nouvelle variable ajoutée aura un coefficient de X dans la contrainte Y[i]
    }

    model->addVar(0.0, INFINITY, distance_totale, GRB_CONTINUOUS, col); 
}

// récupérer val duale theta associée a ctrt <= p
double modele::theta() {
    return max_facility.get(GRB_DoubleAttr_Pi); 
}

void modele::calcul_distances() {

    matrice_distances.resize(inst.C, vector<double>(inst.F)); 
    for(int i = 0; i < inst.C; ++i) {
        for(int j = 0; j < inst.F; ++j) {
            matrice_distances[i][j] = dist(inst,i,j); 
        }
    }
}

// récupérer les Pi_i (val duale associées aux ctrt d'affectations pr chaque client)
vector<double> modele::duales_des_clients() {

    vector<double> duales; 

    for(auto& contrainte : tout_client_assigne) {
        duales.push_back(contrainte.get(GRB_DoubleAttr_Pi)); 
    }

    return duales; 
}

long long timeOptim = 0;

void modele::optimize() {
    model->set(GRB_IntParam_Method, 0); 
    timeOptim -= clock();
    model->optimize(); 
    timeOptim += clock();
}

// récupérer l'obj
double modele::obj() {
    return model->get(GRB_DoubleAttr_ObjVal); 
}

// résoud le pricing. Prends en entrée j : la facility considérée. Renvoie une colonne
vector<int> modele::pricing(int j, const Duales& donnees_duales) {

    GRBModel pricing_model(env); 
    GRBLinExpr contraintes;
    vector<GRBVar> x; 

    for(int i = 0; i < inst.C; ++i) {
        x.push_back(pricing_model.addVar(0.0, 1.0, matrice_distances[i][j] - donnees_duales.duales_des_clients[i], GRB_INTEGER));
        contraintes += x.back()*inst.dc[i]; 
    }

    pricing_model.addConstr(contraintes <= inst.uf[j]); 
    pricing_model.optimize(); 

    double pricing_obj = pricing_model.get(GRB_DoubleAttr_ObjVal); 

    // si l'obj de pricing >= 0, on renvoie vecteur vide; Permettra de détecter que ça a convergé 
    if(pricing_obj - donnees_duales.theta >= -1e-6) return {}; 

    vector<int> colonne; // on créer la colonne 
    colonne.push_back(j); 
    for(int i = 0; i < inst.C; ++i) {
        if(x[i].get(GRB_DoubleAttr_X) > 0.5) colonne.push_back(1); 
        else colonne.push_back(0); 
    }

    return colonne; 
}

// boucle pour la génération de colonne
void modele::gen_col() {

    auto start = std::chrono::high_resolution_clock::now();   
    Duales donnees_duales; // permettra de stocker les duales; 

    while(true) {   
        bool a_ajoute = false; 
        donnees_duales.duales_des_clients = duales_des_clients();
        donnees_duales.theta = theta(); 
        for(int j = 0; j < inst.F; ++j) {  
            auto col = pricing(j, donnees_duales); 
            if(col.empty()) continue; // si colonne vide, on l'ajoute pas 
            ajoute_colonne(col); // on l'ajoute
            a_ajoute = true; // on retient 
        }

        if(!a_ajoute) break; 
        optimize(); 
    }

    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start); 
    cout << duration.count() << endl;
}

// destructeur modele 
modele::~modele() {
    delete env; 
    delete model; 
}


/* ########### PARTIE PROGRAMMATION DYNAMIQUE ########### */

// fonction qui reconstruit la solution trouvée par le programme dynamique 
vector<int> modele::reconstruit_solution(int j, const vector<int>& liaisons, const vector<vector<pair<double,int>>>& tab) {

    vector<int> solution(inst.C+1, 0); 
    solution[0] = j; // on met la facility 

    // Soit on vient de g(i-1, d) soit on vient de G(i-1, d-di). 
    // suffit de vérifier que g(i-1,d) != g(i-1,d)
    int pred;
    int c_courant = tab[0].size()-1;  
 
    for(int i = (int)tab.size()-1; i > 0; --i) {

        pred = tab[i][c_courant].second; 
        if(tab[i][c_courant].first == tab[i-1][c_courant].first) { // alors on vient de G(i-1, c_courant)
            continue; // i pas dans sol
        }
        else {
            solution[liaisons[i-1]+1]++; // je mets plus 1 car sinon on risquerait de moidifier la facility !!
            c_courant = pred;  
        }
    }

    return solution;
}


vector<int> modele::prog_dyn_sac(int j, const Duales& donnees_duales) {

    // données du pb 
    int taille_sac = inst.uf[j];  
    vector<int> poids; 
    vector<double> profits;

    vector<int> liaisons; // liaisons est un vecteur qui permet de se souvenir des indices d'origine des clients (car je calle tout a gauche)
    for(int i = 0; i < inst.C; ++i) { // si cr < 0 pour client i, alors on va le considérer dans le sac à dos; 
        if(matrice_distances[i][j] - donnees_duales.duales_des_clients[i] < 0) {
            liaisons.push_back(i); // on retiens l'index du client i 
            profits.push_back(-matrice_distances[i][j] + donnees_duales.duales_des_clients[i]); // inversion signes car max du knapsack
            poids.push_back(inst.dc[i]); // on retiens le poids de i 
        }
    }
    
    int nb_obj = liaisons.size(); 
    vector<vector<pair<double,int>>> tableau(nb_obj+1, vector<pair<double,int>>(taille_sac+1)); // tableau prog dyn
    for(int d = 0; d < taille_sac; ++d) tableau[0][d] = {0,0}; 

    // ----------------------- RESOLUTION PROG DYN ----------------------- 
    
    for(int i = 1; i < nb_obj + 1; ++i) { 

        int poids_i = poids[i-1]; 
        double profit_i = profits[i-1];

        for(int d = 0; d < taille_sac + 1; ++d) {

            if(poids_i > d) tableau[i][d] = tableau[i-1][d];  // cas le poids de i excede la capacite 
            else 
            {   
                // cas G(i-1, d-di) > G(i-1, d) prendre i dans le sac 
                if(tableau[i-1][d-poids_i].first + profit_i > tableau[i-1][d].first) {

                    tableau[i][d].first = tableau[i-1][d-poids_i].first + profit_i; 
                    tableau[i][d].second = d-poids_i;

                }
                else { // cas G(i,d) = G(i-1, d) en gros, pas prendre i dans le sac 
                    tableau[i][d] = tableau[i-1][d]; 
                }
            }
        }
    }

    vector<int> solution; 
    if(-tableau[nb_obj][taille_sac].first - donnees_duales.theta < -1e-6) { // si l'objectif < 0 (a epsilon pret) renvoyer la solution reconstruite
        return reconstruit_solution(j, liaisons, tableau); 
    } 

    return {};  // sinon, pas de vecteur

}

// fonction qui genere des colonnes en utilisant l'algorithme DP pour résoudre pricing 
void modele::gen_col_DP() {

    auto start = std::chrono::high_resolution_clock::now();  
    Duales donnees_duales; 
    
    while(true) {   
        bool a_ajouter = false;  
        donnees_duales.duales_des_clients = duales_des_clients();
        donnees_duales.theta = theta(); 
        for(int j = 0; j < inst.F; ++j) {
            auto col = prog_dyn_sac(j, donnees_duales); 
            if(col.empty()) continue; // si colonne vide, on l'ajoute pas
            ajoute_colonne(col); 
            a_ajouter = true; 
        }
        
        if(!a_ajouter) break; // si on a aajouté aucune col 
        optimize(); 
    }

    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start); 
    cout << duration.count() << endl;

}


/* ########### IMPLE SEPARATION ########### */


void modele::update_sep(Duales& sep, const Duales& in, const Duales& out, double alpha) {

    sep.theta = alpha*in.theta + (1-alpha)*out.theta; 
    for(int i = 0; i < inst.C; ++i) {
        sep.duales_des_clients[i] = alpha*in.duales_des_clients[i] + (1-alpha)*out.duales_des_clients[i]; 
    }

}


// il va falloir implémenter un algorithme de stabilisation. Je vais utiliser le concept in-out. 
/*
Pseudo - code : 
solution duale réalisable initiale (in) = (0,...,0); 
out <-- résoudre le master avec mes colonnes initiales; 

TANT QUE : out n'est pas réalisable (ie. cas où les duales renvoyées par master ont permis a pricing de renvoyer une colonne)
    _sep = alpha*in + (1-alpha)out
    _si sep est réalisable : (ie. on a pas renvoyé de colonnes avec duales = sep)
        _ in <-- sep
    _sinon (cas où pricing renvoie une colonne)
        _ajoute la colonne
        _met a jour out (ie. out <-- duales obtenues par resolution master apres ajout)
*/
void modele::gen_col_stabilization() {

    auto start = std::chrono::high_resolution_clock::now();
    Duales in; 
    in.duales_des_clients.resize(inst.C); 
    Duales sep; 
    sep.duales_des_clients.resize(inst.C); 

    double alpha = 0.9; // alpha (que je réglerais moi meme)

    Duales out; 
    while(true) { // tant que out n'est pas réalisable 

        bool a_ajoute = false;  
        out.duales_des_clients = duales_des_clients();
        out.theta = theta();
        
        for(int j = 0; j < inst.F; ++j) { // résolution des j pricings 
            auto col = prog_dyn_sac(j, out); 
            if(col.empty()) continue; // si colonne vide, on l'ajoute pas
            ajoute_colonne(col); 
            a_ajoute = true; 
        }

        if(!a_ajoute) break; // si out est réalisable -> sol optimale 

        while(true) { // tant que sep est réalisable (ie on a pas renvoyé de colonne)
 
            update_sep(sep, in, out, alpha); 
        
            bool a_ajoute_sep = false; 
            for(int j = 0; j < inst.F; ++j) { // on regarde si sep est réalisable 
                auto col = prog_dyn_sac(j, sep);
                if(col.empty()) continue; // si colonne vide, on l'ajoute pas
                ajoute_colonne(col); 
                a_ajoute_sep = true;  
            }
            
            if(!a_ajoute_sep) { // si on a pas ajouté 
                in.duales_des_clients = sep.duales_des_clients; // in = sep
                in.theta = sep.theta;  
            } 
            else break; // si on a ajouté, on doit retourner calculer out
        }

        optimize(); 
    }

    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start); 
    cout << duration.count() << endl;

}



int main(int argc, char* argv[]) {

    Instance inst; 
    ifstream file(argv[1]); 
    if(file.is_open()) file >> inst; 
    else cerr << "erreur ouverture fichier" << endl;

    // on génère des colonnes initiales : ca fonction grv bien et ça améliore le temps de facteur 3 !!!
    vector<vector<int>> colonnes_initiales = get_first_col(inst); 

    modele m(inst, colonnes_initiales);
    char choix = argv[2][0];  
    if(choix=='1') {
        m.gen_col_DP();
    } 
    if(choix=='0') {
        m.gen_col(); 
    }  
    if(choix=='2') {
        m.gen_col_stabilization(); 
    }
    
    cout << "relaxation LP maitre : " << m.obj() << endl;
    cout << "temps gurobi : " << timeOptim / (double)CLOCKS_PER_SEC << endl; 

    return 0; 
}