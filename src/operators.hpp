#pragma once 

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <limits>
#include <cmath>

using namespace std; 

struct Point2D {
    double x, y;
};

using Solution = vector<Point2D>; 

struct Instance {

    int C, F, p, U; 
    vector<Point2D> loc_client;  
    vector<int> dc; 
    vector<Point2D> loc_f; 
    vector<int> uf; 

    double checker(const Solution& sol) const; // const car pas de mofif de inst
    // et const solution car passage par ref pour mémoire mais pas de modif autorisé
    void dessine_inst(); 
    void dessine_sol(const Solution& sol) const; 

};

ostream& operator<<(ostream& out, const Instance& inst);

istream& operator>>(istream &in, Instance& inst);

ostream& operator<<(ostream& out, const Solution& sol);

istream& operator>>(istream& in, Solution& sol);

double dist(const Instance &inst, int i, int j); 

