#include <iostream>

int divide(int x, int y) {
  return x / y;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Two integer arguments required\n");
    return -1;
  }

  int x = std::stoi(argv[1]);
  int y = std::stoi(argv[2]);
  
  int z = divide(x, y);

  printf("%d / %d = %d\n", x, y, z);
}
