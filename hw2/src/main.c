#include <stdlib.h>
#include <getopt.h>

extern int original_main(int argc, const char *const *argv);

int main(int argc, const char *const *argv) {
    original_main(argc, argv);
}
