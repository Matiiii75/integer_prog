#include "operators.hpp"

using namespace std; 
using Solution = vector<Point2D>; 

// question 4

double Instance::checker(const Solution& sol) const{

    // contiendra les capacite consomme pour chaque entrepot
    vector<int> capacite_conso(F, 0.0);
    int obj = 0; 

    for(size_t i = 0; i < sol.size(); ++i) { // pr chaque coord de sol
        bool isin = false; 
        for(size_t j = 0; j < loc_f.size(); ++j) { // pr chaque coord d'entrepots
            
            if(sol[i].x == loc_f[j].x && sol[i].y == loc_f[j].y) { // verif correspondance
                
                isin = true; 
                capacite_conso[j] += dc[i]; 
                if(capacite_conso[j] > uf[j]) { // dépassement capacité
                    return numeric_limits<double>::infinity(); 
                }
                
                // ajout valeur à l'objectif 
                double x, y; 
                // debug 
                cout << "loc_c x :" << loc_client[i].x << ", loc_c y :" << loc_client[i].y << endl;
                cout << "loc_f x :" << sol[i].x << ", loc_f y :" << sol[i].y << endl;
                x = pow(loc_client[i].x - sol[i].x, 2); 
                y = pow(loc_client[i].y - sol[i].y, 2); 
                cout << "x : " << x << ", y : " << y << ", dist : " << pow(x+y, 0.5) << endl;
                obj += pow(x+y, 0.5); 

                break; 
            }
        }
        if(isin == true) { // debug
            cout << "(" << sol[i].x << "," << sol[i].y << ") found !" << endl;
            cout << endl;
        }
        else { // si pas de correspondance à 1 moment 
            cout << "(" << sol[i].x << "," << sol[i].y << ") not found !" << endl;
            return numeric_limits<double>::infinity(); // renvoie inf 
        }
    }

    return obj; 
}


// question 5 visualiseur d'instance 

void Instance::dessine_inst() {
    
    ofstream dessin("../dessins/dessin_inst.svg"); 
    // création encadré pour le dessin 1000*1000 (j'utilise R"" car les guillemets cassent le string)
    dessin << R"(<svg width="1050" height="1050" xmlns="http://www.w3.org/2000/svg">)" << endl; 
    dessin << R"(<rect x="0" y="0" width="1050" height="1050" fill="lightgray" />)" << endl;
    
    // dessiner un point pour chaque client 
    for(int i = 0; i < C; ++i) {
        dessin << "<circle cx=\"" << loc_client[i].x*1000 << 
        "\" cy=\"" << loc_client[i].y*1000 << "\" r=\"5\" fill=\"red\"/>" << endl;
    }

    // pareil pr chaque entrepot
    for(int i = 0; i < F; ++i) {
        dessin << "<circle cx=\"" << loc_f[i].x*1000 << 
        "\" cy=\"" << loc_f[i].y*1000 << "\" r=\"5\" fill=\"blue\"/>" << endl;
    }

    dessin << "</svg>"; 
    dessin.close(); 

}

// question 5 visualiseur de solution

void Instance::dessine_sol(const Solution& sol) const {

    ofstream dessin("../dessins/dessin_sol.svg"); 

    dessin << R"(<svg width="1050" height="1050" xmlns="http://www.w3.org/2000/svg">)" << endl; 
    dessin << R"(<rect x="0" y="0" width="1050" height="1050" fill="lightgray" />)" << endl;

    // dessiner un point pour chaque client 
    for(int i = 0; i < C; ++i) {
        dessin << "<circle cx=\"" << loc_client[i].x*1000 << 
        "\" cy=\"" << loc_client[i].y*1000 << "\" r=\"5\" fill=\"red\"/>" << endl;
    }

    // pareil pr chaque entrepot
    for(int i = 0; i < F; ++i) {
        dessin << "<circle cx=\"" << loc_f[i].x*1000 << 
        "\" cy=\"" << loc_f[i].y*1000 << "\" r=\"5\" fill=\"blue\"/>" << endl;
    }

    // puis, pour chaque paire client entrepôt dans sol, les relier par une ligne
    for(int i = 0; i < C; ++i) { // j'itère sur C en suppoosant que la validité de la solution a été vérifiée par checker
        dessin << "<line x1=\"" << loc_client[i].x*1000 << "\" y1=\"" <<
        loc_client[i].y*1000 << "\" x2=\"" << sol[i].x*1000 << "\" y2=\"" << 
        sol[i].y*1000 << "\" stroke=\"black\" stroke-width=\"3\"/>"; 
    }

    dessin << "</svg>"; 
    dessin.close(); 

}


ostream& operator<<(ostream& out, const Instance& inst) {
     
    out << "C:" << inst.C << "| F:" << inst.F << "| p:" << 
    inst.p <<  "| U:" << inst.U << endl;
    
    for(int i = 0; i < inst.C; ++i) { // Affichage clients
        out << "client i:" << i << "-> (" << inst.loc_client[i].x << "," << inst.loc_client[i].y << ")"; 
        out << "-> demande:" << inst.dc[i]; 
        out << endl;
    }

    for(int i = 0; i < inst.F; ++i) { // Affichage entrepots
        out << "entr f:" << i << "-> (" << inst.loc_f[i].x << "," << inst.loc_f[i].y << ")"; 
        out << "-> capacity:" << inst.uf[i]; 
        out << endl;
    }

    return out; 
}


istream& operator>>(istream &in, Instance& inst) {
    
    in >> inst.C >> inst.F >> inst.p >> inst.U; 
    
    /* redimmensionnement des vecteurs pour l'ajout */

    inst.loc_client.resize(inst.C); 
    inst.dc.resize(inst.C); 
    inst.loc_f.resize(inst.F); 
    inst.uf.resize(inst.F); 

    for(int i = 0; i < inst.C; ++i) {

        in >> inst.loc_client[i].x;
        in >> inst.loc_client[i].y; 
        in >> inst.dc[i]; 

    }

    for(int i = 0; i < inst.F; ++i) {

        in >> inst.loc_f[i].x; 
        in >> inst.loc_f[i].y;
        in >> inst.uf[i]; 
        
    }

    return in;
}


ostream& operator<<(ostream& out, const Solution& sol) {

    for(size_t i = 0; i < sol.size(); ++i) {
        out << sol[i].x << " " << sol[i].y; 
        out << endl;
    }

    return out; 
}


istream& operator>>(istream& in, Solution& sol) {
    
    double x, y; 

    while(true) {
        in >> x >> y; 
        if(in.fail()) break;
        Point2D p; 
        p.x = x;
        p.y = y; 
        // On ajoute (x,y) a la sol courante
        sol.push_back(p); 
    }

    return in; 
}


// étant donné une instance et un entrepot, cette fonction calcule la distance les séparant 
double dist(const Instance &inst, int i, int j) {

    if(i<0 || i >= inst.C) cout << "erreur dans dist, client impossible" << endl;
    if(j<0 || i >= inst.F) cout << "erreur dans dist, entrepot impossible" << endl;

    double x = inst.loc_client[i].x - inst.loc_f[j].x; 
    double y = inst.loc_client[i].y - inst.loc_f[j].y; 

    return sqrt(x*x + y*y);
} 


// fonction qui permet d'isoler le numéro de l'instance "uniform_{num}.inst"
int get_num_instance(const string& nom) {
    
    size_t pos = nom.find("uniform_"); 
    if(pos == string::npos) {
        cerr << "pb dans get_num_instance, impossible de trouver le numéro pour : " << nom << endl;
        return -1; 
    }

    pos += 8; // longueur de uniform_
    size_t endpos = pos; 

    while(endpos < nom.length() && isdigit(nom[endpos])) { // tant que le char a la position courante est un numéro : avancer
        endpos ++ ; 
    }
    if(endpos > pos) {
        return stoi(nom.substr(pos, endpos - pos)); 
    }

    return -1; 
}


// fonction d'écriture des données pour la gen col dans la forme suivante : 
// numéro d'instance -> choix(0,1,2) pour pricing & stab -> temps -> valeur -> alpha
// si on est pas dans le cadre de la stabilisation, le alpha vaudra -1
void ecrire_data(const string& path, char choix, double temps, double val, double alpha) {

    ofstream fichier_ou_ecrire; 
    fichier_ou_ecrire.open("../solutions/solutions_gen_col2.sol", ios::app); 
    
    if(!fichier_ou_ecrire.is_open()) {
        cerr << "Erreur ouverture fichier" << endl;
    }

    int num_instance = get_num_instance(path); 

    fichier_ou_ecrire << num_instance << " " << choix << " " << temps << " " << val << " " << alpha << endl; 
    fichier_ou_ecrire.close(); 

}


// fonction d'écriture des données pour formulation compacte entière de la forme suivante : 
// num instance -> (1 si LP, 0 sinon MIP) -> temps -> valeur -> gap (que si relaxation = 0)
void ecrire_data_compact(int num_instance, double temps, double val, double gap, bool relaxation) {

    ofstream fichier_ou_ecrire; 
    fichier_ou_ecrire.open("../solutions/solutions_compact.sol", ios::app);

    if(!fichier_ou_ecrire.is_open()) {
        cerr << "Erreur ouverture fichier" << endl;
    }

    if(relaxation) fichier_ou_ecrire << num_instance << " " << relaxation << " " << temps << " " << val << " " << endl; // si c une relaxation -> pas de gap
    else 
        fichier_ou_ecrire << num_instance << " " << relaxation << " " << temps << " " << val << " " << gap << endl;

    fichier_ou_ecrire.close(); 

}