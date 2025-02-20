#include"test.h"


int main() {
    try {
        test();
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}