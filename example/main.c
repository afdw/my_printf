#include <float.h>
#include "../src/fp.h"
#include "../src/library.h"

void example_basic_numbers() {
    int precision = 2, hour = 12, min = 34, sec = 56;
    my_printf("%1$d:%2$.*3$d:%4$.*3$d\n", hour, min, precision, sec);
}

void example_factorial() {
    struct ap i = ap_from_uintmax_t(0);
    struct ap factorial = ap_from_uintmax_t(1);
    while (ap_compare(ap_copy(factorial), ap_power(ap_from_uintmax_t(10), ap_from_uintmax_t(70))) <= 0) {
        my_printf("%-2Zd %70Zd\n", ap_copy(i), ap_copy(factorial));
        i = ap_add(i, ap_from_uintmax_t(1));
        factorial = ap_multiply(factorial, ap_copy(i));
    }
    ap_destroy(i);
    ap_destroy(factorial);
}

void example_huge_precision() {
    my_printf("%.2000e\n", 623.28376510723481);
    my_printf("%.2000e\n", DBL_MIN);
    my_printf("%.2000Fe\n", fp_divide(fp_extend(fp_from_long_double(1.0).fp, 8000), fp_from_long_double(3.0).fp));
    my_printf("%f\n", DBL_MAX);
}

int main(int argc, char *argv[]) {
    example_basic_numbers();
    example_factorial();
    example_huge_precision();
    return 0;
}
