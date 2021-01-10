/*************************************
* Lab 4
* Name:
* Student No:
* Lab Group:
*************************************/

#include <stddef.h>

/*
You may define helper structs here,
as long as the functions satisfy the
requirements in the lab document.  If you declare additional names (helper structs or helper functions), they should be prefixed with "mmf_" to avoid potential name clashes.
*/

/*
These functions form the public API of your library.
*/

void *mmf_create_or_open(const char *name, size_t sz);
void mmf_close(void *ptr, size_t sz);
