#include <stdio.h>

double println(double x) {
    printf("%f\n",x);
    return x;
}

double print(double x) {
    printf("%f",x);
    return x;
}

double put(double c) {
    printf("%c", (char)c);
    return c;
}

double input() {
    double x;
    scanf("%lf", &x);
    return x;
}
