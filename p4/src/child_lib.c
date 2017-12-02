#include "child_lib.h"

off_t getFileSize(const char *fileName) {
    struct stat st;
    /* Get file stats */
    if (stat(filename, &st) == 0)
        return st.st_size;
    perror("Cannot get file stats");
    return -1;
}
