#include "../src/library.h"

int main(int argc, char *argv[]) {
    int precision = 2, hour = 12, min = 34, sec = 56;
    my_printf("%1$d:%2$.*3$d:%4$.*3$d\n", hour, min, precision, sec);
    return 0;
}
