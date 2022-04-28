#include <stdio.h>

void _ZN3std8printStr(char* s) {
    printf("%s", s);
}

void _ZN3std9printReal(double num) {
    printf("%f", num);
}

double _ZN3std7readNum() {
    double num;
    scanf("%lf", &num);
    return num;
}

double _ZN3std7readStr(char* str, double n) {
    return scanf("%s", str);
}
