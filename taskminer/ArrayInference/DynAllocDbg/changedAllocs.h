#include <stdlib.h>
#include <string.h>

void *mallocDbg (size_t  size) {
  void *tmp = malloc(size + sizeof(size_t));
  if (tmp == NULL)
    return NULL;
  size_t *dbgInfo = (size_t*) tmp;
  dbgInfo[0] = size;
  return (void*) (dbgInfo + sizeof(size_t));
}

void *callocDbg (size_t nitems, size_t  size) {
  void *tmp = calloc(nitems,size + sizeof(size_t));
  if (tmp == NULL)
    return NULL;
  memset(tmp, 0, (size + sizeof(size_t)));
  size_t *dbgInfo = (size_t*) tmp;
  dbgInfo[0] = size;
  return (void*) (dbgInfo + sizeof(size_t));
}

void *reallocDbg (void* p, size_t size) {
  void *tmp = realloc(p, size);
  if (tmp == NULL)
    return NULL;
  size_t *dbgInfo = (size_t*) tmp;
  dbgInfo[0] = size;
  return (void*) (dbgInfo + sizeof(size_t));
}

void freeDbg (void *p) {
  void *tmpPointer = (void*) (&p - sizeof(size_t));
  free(tmpPointer);
}

