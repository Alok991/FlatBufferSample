#include <client_generated.h>
#include <iostream>

int main(int argc, char const *argv[]) {
  fb::Gender g = fb::Gender::Gender_Female;
  std::cout << (int)g;
  return 0;
}
