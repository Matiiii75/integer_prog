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





