#include "gauntlet.h"
#include "negamax.h"
#include "alpha_beta_search.h"
#include <iostream>

int main()  
{
    setup();
    Search* one = new AlphaBeta();
    Search* two = new Negamax();
    EvalParams params1 = {{100, 300, 300, 500, 900, 1000}};
    EvalParams params2 = {{100, 300, 300, 500, 900, 1000}};
    int wins = Gauntlet::fight(one, two, params1, params2, 100, 3);
    cout << wins << endl;

    free(one);
    free(two);

    return 0;
}