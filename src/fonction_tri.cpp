#include <iostream>
#include <vector>
#include <cmath>
#include "operators.hpp"

using namespace std; 


// on lui donne vec_a_trier (le vecteur inst.uf)
// il les trie dans l'ordre décroissant sans le modifier et ressortira 
// un vecteur<paires<int,int>> new_vec où new_vec[i].first : capacité max de l'entrepot new_vec[i].second
// et evidement, new_vec[i] <= new_vec[i+1] pour tout i 
// nb_a_conserver doit etre set à inst.p

// en fait, on peut aussi l'utiliser pour trier les demandes des clients (par ordre décroissant aussi)
// il suffit de set nb_a_conserver à inst.C
vector<pair<int,int>> trie_p_plus_grands(int nb_a_conserver, const vector<int>& vec_a_trier) {
    
    cout << "trie_p_plus_grands" << endl;

    vector<int> copie = vec_a_trier; 
    vector<int> indexs; 
    for(int i = 0; i < (int)copie.size(); ++i) indexs.push_back(i); 
    vector<pair<int,int>> new_vec(nb_a_conserver); // new_vec[i].second = indice d'origine de la valeur new_vec.first

    // trier copie par ordre décroissant
    for(int i = 0; i < (int)copie.size()-1; ++i) {
        if(copie[i] < copie[i+1]) {

            int temp = copie[i]; 
            copie[i] = copie[i+1];  // repositionnement copie 
            copie[i+1] = temp; 

            int temp_index = indexs[i]; 
            indexs[i] = indexs[i+1]; 
            indexs[i+1] = temp_index; 

        }
        for(int j = 0; j < i; ++j) {
            if(copie[j] < copie[i]) {

                int temp = copie[j]; 
                copie[j] = copie[i]; 
                copie[i] = temp; 
                    
                int temp_index = indexs[j]; 
                indexs[j] = indexs[i]; 
                indexs[i] = temp_index; 

            }
        }
    }

    for(int i = 0; i < nb_a_conserver; ++i) {
        new_vec[i] = {copie[i], indexs[i]}; 
    }

    return new_vec; 
}

vector<vector<int>> create_colonne_set(int p, int nb_client, vector<pair<int,int>> demandes_tries, vector<pair<int,int>> cap_tries) {

    cout << "entree create_colonne_set" << endl;

    vector<vector<int>> ens_col; 
    for(int j = 0; j < p; ++j) {
        vector<int> col(nb_client+1, 0); // créer la colonne
        col[0] = cap_tries[j].second; // lui attribuer l'entrepot correct
        ens_col.push_back(col); // l'ajouter à l'ensemble
    }

    for(int i = 0; i < nb_client; ++i) {
        int index_client = demandes_tries[i].second+1; 
        for(int j = 0; j < p; ++j) {

            if(cap_tries[j].first >= demandes_tries[i].first) { // si y'a la place pour assigner index_client à index_facility
                ens_col[j][index_client] = 1; 
                cap_tries[j].first -= demandes_tries[i].first; // on maj 
                break; // et on sort
            }
        }
    }

    return ens_col; 
}


vector<vector<int>> get_first_col(const Instance& inst) {

    cout << "entree get_first_col" << endl;

    vector<pair<int,int>> u_tries = trie_p_plus_grands(inst.p, inst.uf); 
    vector<pair<int,int>> d_tries = trie_p_plus_grands(inst.C, inst.dc); 

    vector<vector<int>> first_col; 
    first_col = create_colonne_set(inst.p, inst.C, d_tries, u_tries); 

    return first_col; 
}


int main(int argc, char* argv[]) {

    Instance inst; 
    ifstream file(argv[1]); 
    if(file.is_open()) file >> inst; 
    else cerr << "error opening file" << endl;

    cout << inst; 

    vector<vector<int>> cols_naive;  
    cols_naive = get_first_col(inst); 

    cout << "affichage first_col : " << endl;
    for(int i = 0; i < (int)cols_naive.size(); ++i) {
        cout << "["; 
        for(int j = 0; j < (int)cols_naive[i].size(); ++j) {
            
            cout << cols_naive[i][j] << " "; 

        }
        cout << "]" << endl;
    }



    // VERIFIE QUE LES CLIENTS SONT TOUS ASSIGNES 
    vector<int> verif_ok(inst.C,-1); 
    for(int i = 0; i < (int)cols_naive.size(); ++i) {
        for(int j = 0; j < (int)cols_naive[i].size(); ++j) {
            
            if(j != 0 && cols_naive[i][j]) verif_ok[j-1] = cols_naive[i][0]; 

        }
    }   

    cout << "verif_ok : " << endl;
    for(int& i : verif_ok) cout << i << " "; 
    cout << endl;
    
    return 0; 
}


// int main() {

//     vector<int> vec_a_trier = {100,87,17,213,2872,2,193,92,827,9,10,9281}; 
//     int nb_a_conserver = ceil(vec_a_trier.size()); 
//     // avant 
    
//     cout << "p : " << nb_a_conserver << endl;
//     cout << "avant : " << endl;
//     cout << "V : "; 
//     for(int & i : vec_a_trier) cout << i << " "; 
//     cout << endl << "I : "; 
//     for(int i = 0; i < (int)vec_a_trier.size(); ++i) cout << i << " "; 
//     cout << endl;

//     vector<pair<int,int>> recup_sol = trie_p_plus_grands(nb_a_conserver, vec_a_trier); 

//     return 0; 
// }