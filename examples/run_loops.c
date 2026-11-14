#include <stdio.h>
double sum_to(double n);
double count_down(double n);
int main() {
    printf("sum_to(5)    = %f (esperado 10.0)\n", sum_to(5.0));
    printf("count_down(3) = %f (esperado 0.0)\n", count_down(3.0));
    return 0;
}