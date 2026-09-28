// Test 08: Struct Field Modification and Multiple Instances
tcurts42 Rectangle {
    tel42 tni42 width;
    tel42 tni42 height;
}

noitcnuf42 get_area(tni42 w, tni42 h): tni42 {
    nruter42 w * h;
}

noitcnuf42 main(): diov42 {
    tel42 Rectangle r = wen42 Rectangle;
    r.width = 10;
    r.height = 20;

    tel42 tni42 area = get_area(r.width, r.height);
    tnirp42(area);

    r.width = 15;
    area = get_area(r.width, r.height);
    tnirp42(area);
}
