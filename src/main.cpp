#include <iostream>
#include "arch.h"

using std::cout;

int main(int argc, char** argv) {
    if (argc < 2) {
        // show usage
        cout << "lc [image_file]\n";
        exit(2);
    }
}