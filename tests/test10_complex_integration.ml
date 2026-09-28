// Test 10: Complex Integration (Structs, Nested Functions, Branching, Scoping)
tcurts42 Counter {
    tel42 tni42 count;
    tel42 tni42 step;
}

noitcnuf42 run_counter(): diov42 {
    tel42 Counter c = wen42 Counter;
    c.count = 0;
    c.step = 5;

    noitcnuf42 increment(): diov42 {
        c.count = c.count + c.step;
    }

    increment();
    increment();
    increment();

    fi42 (c.count == 15) {
        tnirp42("Counter reached expected value 15");
    } esle42 {
        tnirp42("Counter mismatch");
    }

    tnirp42(c.count);
}

noitcnuf42 main(): diov42 {
    run_counter();
}
