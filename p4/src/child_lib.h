#ifndef __CHILD_LIB_H__
#define __CHILD_LIB_H__

#include "protocol.h"
#include "common.h"

#include <dirent.h>

/*
 * Function: getFileSize()
 * Return file size in long int
 */
off_t getFileSize(const char *fileName);

#endif  /*__CHILD_LIB_H__*/
