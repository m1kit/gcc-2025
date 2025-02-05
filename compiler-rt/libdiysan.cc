#include <iostream>
#include <cstdio>
#include <cstdint>

namespace {

[[noreturn]] [[maybe_unused]] void crash() {
  asm volatile("int3");
  asm volatile("ud2");
  __builtin_unreachable();
}

} // namespace

extern "C" {

// Called on `malloc()`.
void* _diysan_malloc(size_t n) {
  void* p = malloc(n);

  uintptr_t address = reinterpret_cast<uintptr_t>(p);
  std::cout << "malloc(" << n << ") = " << std::hex << address << std::endl;
  
  return p;
}

// Called on `free()`.
void _diysan_free(void* p) {
  uintptr_t address = reinterpret_cast<uintptr_t>(p);
  std::cout << "free(" << std::hex << address << ")" << std::endl;

  free(p);
}

// Called before storing a value into `p`.
void _diysan_store(void* p) {
}

}
