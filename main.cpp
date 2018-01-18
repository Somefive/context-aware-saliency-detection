#include <iostream>

#include "SaliencyDetect.h"

using namespace std;

int main(int argc, const char** argv) {
    if (argc > 1) {
        int u = 4;
        char outFile[1000];
        strcpy(outFile, "target.png");
        for (int i = 2; i < argc - 1; ++i) {
            if (strcmp(argv[i], "-u") == 0) {
                u = atoi(argv[i+1]);
                ++i;
            } else if (strcmp(argv[i], "-o") == 0) {
                strcpy(outFile, argv[i+1]);
                ++i;
            }
        }
        printf("out=%s, u=%i\n", outFile, u);
        exec(argv[1], outFile, u);
    } else {
        cout << "Instruction: <filename> [-u region size] [-o output file name]" << endl;
    }
    return 0;
}