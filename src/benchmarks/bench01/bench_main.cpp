//
// Created by saleh on 11/30/24.
//

#include <iostream>
#include "bench01/bench01.h"

int main() {
    bench01 b(256, 1024*4, 25);
    b.Run();

    return 0;
}