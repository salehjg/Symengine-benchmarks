//
// Created by saleh on 11/30/24.
//

#include <iostream>
#include "bench05/bench05.h"

int main() {
    bench05 b(1024, 4096, 15);
    b.Run();

    return 0;
}