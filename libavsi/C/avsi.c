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

double readStr(char* str, double n) {
    return scanf("%s", str);
}
