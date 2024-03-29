
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef double (*cos_t)(double);

int main(void) {
  cos_t cosine;
  char *error;
  void* handle = dlopen("libm.so.6", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  dlerror();
  cosine = (cos_t) dlsym(handle, "cos");
  error = dlerror();
  if (error != NULL) {
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
  }
  printf("%f\n", (*cosine)(2.0));
  dlclose(handle);
  return 0;
}