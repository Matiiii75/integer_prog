#pragma once 

#include <iostream>
#include <vector>
#include <cmath>
#include "operators.hpp"

using namespace std; 

vector<pair<int,int>> trie_p_plus_grands(int nb_a_conserver, const vector<int>& vec_a_trier); 

vector<vector<int>> create_colonne_set(int p, int nb_client, vector<pair<int,int>> demandes_tries, vector<pair<int,int>> cap_tries, vector<bool>& clients_places); 

vector<vector<int>> get_first_col(const Instance& inst, vector<bool>& clients_places); 

