#include <math.h>

double _sin(double x) {
    return sin(x);
}
double _cos(double x) {
    return cos(x);
}
double _tan(double x) {
    return tan(x);
}
double _cot(double x) {
    return cos(x) / sin(x);
}
double _sec(double x) {
    return 1 / cos(x);
}
double _csc(double x) {
    return 1 / sin(x);
}
double _log(double x) {
    return log(x);
}
double _exp(double x) {
    return exp(x);
}
double _sqrt(double x) {
    return sqrt(x);
}
double _arcsin(double x) {
    return asin(x);
}
double _arccos(double x) {
    return acos(x);
}
double _arctan(double x) {
    return atan(x);
}
double _arccot(double x) {
    return 3.14159265358979323846/2.0 - asin(x);
}