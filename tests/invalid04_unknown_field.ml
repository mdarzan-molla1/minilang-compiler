// Invalid Test 04: Error 4 - Accessing Non-Existent Struct Field
tcurts42 Point {
    tel42 tni42 x;
    tel42 tni42 y;
}

noitcnuf42 main(): diov42 {
    tel42 Point pt = wen42 Point;
    pt.x = 5;
    // Field 'z' is not declared in struct Point
    pt.z = 10;
    tnirp42(pt.x);
}
