#include <iostream>
using namespace std;

#include "gurobi_c++.h"

struct Point2D {
    double x, y;
};

// test 
int main(int argc, char **argv)
{
    GRBEnv env(true);
    env.set(GRB_IntParam_LogToConsole, 0); // Comment this line to enable Gurobi output
    env.start();

    GRBModel model(env);

    GRBVar x = model.addVar(0.0, 10.0, 3.0, GRB_CONTINUOUS, "x");
    GRBVar y = model.addVar(0.0, 10.0, 5.0, GRB_CONTINUOUS, "y");
    GRBVar z = model.addVar(0.0, 10.0, 4.0, GRB_CONTINUOUS, "z");

    GRBConstr c0 = model.addConstr(x + y + z >= 10);
    GRBConstr c1 = model.addConstr(x + y >= 5);
    GRBConstr c2 = model.addConstr(y + z >= 7);

    model.optimize();

    cout << "Optimal objective: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

    cout << "Values:" << endl;
    cout << "x: " << x.get(GRB_DoubleAttr_X) << endl
            << "y: " << y.get(GRB_DoubleAttr_X) << endl
            << "z: " << z.get(GRB_DoubleAttr_X) << endl;
    return 0;
}