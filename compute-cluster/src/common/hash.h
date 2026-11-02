#ifndef COMMON_HASH_H
#define COMMON_HASH_H
#include "common/types.h"
/* Owner: C (Section 13). Content hash of a file, lowercase hex, NUL-terminated.
 * Write your own SHA-256 (no OpenSSL: the TA's machine may not have it). 0 ok, -1 error. */
int hash_file(const char *path, char out_hex[HASH_HEX_LEN + 1]);
#endif
