#include "gauntlet.h"
#include "negamax.h"
#include <iostream>

int main()
{
    setup();
    Negamax* one = new Negamax();
    Negamax* two = new Negamax();
    EvalParams params1 = {{100, 300, 300, 0, 0, 1000}};
    EvalParams params2 = {{100, 300, 300, 500, 900, 1000}};
    int wins = Gauntlet::fight(one, two, params1, params2, 100, 3);
    cout << wins << endl;

    free(one);
    free(two);

    return 0;
}