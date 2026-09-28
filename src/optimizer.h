#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "tac.h"

// Run Constant Folding optimization pass
int optimize_constant_folding(TACProgram *prog);

// Run Dead Code Elimination optimization pass
int optimize_dead_code_elimination(TACProgram *prog);

// Run all optimizations iteratively until fixpoint
void optimize_tac(TACProgram *prog);

#endif // OPTIMIZER_H
