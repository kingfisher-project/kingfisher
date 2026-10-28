#include <iostream>

#include "utils/errors.h"

int main(int argc, char** argv) {
  try {
    return 0;
  } catch (const kingfisher::Error& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}
