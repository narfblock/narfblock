#include <stdio.h>
#include "test.h"

int main() {
  printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
  return 0;
}
