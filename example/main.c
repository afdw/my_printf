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

void example_hexadecimal_float() {
    my_printf("%a\n", 0);
    my_printf("%a\n", 0.3);
    my_printf("%a\n", 1231231.0 / 256.0);
    my_printf("%.100a\n", 1231231.0 / 256.0);
    my_printf("%a\n", -8000.0);
    my_printf("%a\n", -1.0 / 0.0);
}

void example_numbers_cache() {
    char numbers_cache[1024];
    size_t numbers_positions[11];
    my_snprintf(
        numbers_cache,
        1024,
        "%1$zn%12$02d%2$zn%13$02d%3$zn%14$02d%4$zn%15$02d%5$zn%16$02d%6$zn%17$02d%7$zn%18$02d%8$zn%19$02d%9$zn%20$02d%10$zn%21$02d%11$zn%22$02d",
        &numbers_positions[0],
        &numbers_positions[1],
        &numbers_positions[2],
        &numbers_positions[3],
        &numbers_positions[4],
        &numbers_positions[5],
        &numbers_positions[6],
        &numbers_positions[7],
        &numbers_positions[8],
        &numbers_positions[9],
        &numbers_positions[10],
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        10
    );
    my_printf("%.2s + %.2s = %.2s\n", &numbers_cache[numbers_positions[3]], &numbers_cache[numbers_positions[7]], &numbers_cache[numbers_positions[10]]);
}

int main(int argc, char *argv[]) {
    example_basic_numbers();
    example_factorial();
    example_huge_precision();
    example_hexadecimal_float();
    example_numbers_cache();
    return 0;
}
