//
// Created by saleh on 11/30/24.
//

#include <iostream>
#include "bench01/bench01.h"

int main() {
    bench01 b(16, 1024*2, 5);
    b.Run();

    return 0;
}