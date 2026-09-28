// Test 09: TAC Optimization Test (Constant Folding & Dead Code Elimination)
noitcnuf42 main(): diov42 {
    // Constant folding will fold 10 + 20 into 30, and 5 * 6 into 30
    tel42 tni42 a = 10 + 20;
    tel42 tni42 b = 5 * 6;
    tel42 tni42 c = a + b;
    tnirp42(c);

    // Dead code after return will be eliminated by DCE
    nruter42;
    tel42 tni42 dead_var = 999;
    tnirp42(dead_var);
}
