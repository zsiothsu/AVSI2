#include <stdio.h>

void printStr(char* s) {
    printf("%s", s);
}

void printReal(double num) {
    printf("%f", num);
}

double readNum() {
    double num;
    scanf("%lf", &num);
    return num;
}

int readStr(char* str) {
    return scanf("%s", str);
}
