#include <iostream>
#include <cstdio>
#include <cstdint>
#include <map>

namespace {

[[noreturn]] [[maybe_unused]] void crash() {
  asm volatile("int3");
  asm volatile("ud2");
  __builtin_unreachable();
}

// allocated[key] = value means that the memory at the address `key` has been allocated with the size `value`.
std::map<uintptr_t, size_t> allocated;
// The cookie value to check the integrity of the memory allocation.
constexpr uint64_t kCookie = 0xdeadbeefdeadbeef;

} // namespace

extern "C" {

// Called on `malloc()`.
void* _diysan_malloc(size_t n) {
  void* p = malloc(n + sizeof(kCookie));

  // Set the cookie value.
  *reinterpret_cast<uintptr_t*>(reinterpret_cast<char*>(p) + n) = kCookie;

  uintptr_t address = reinterpret_cast<uintptr_t>(p);
  std::cerr << "malloc(" << n << ") = " << std::hex << address << std::endl;

  allocated[address] = n;
  
  return p;
}

// Called on `free()`.
void _diysan_free(void* p) {
  uintptr_t address = reinterpret_cast<uintptr_t>(p);
  std::cerr << "free(" << std::hex << address << ")" << std::endl;

  if (p == nullptr) {
    return;
  }

  if (allocated.find(address) == allocated.end()) {
    std::cerr << "free() is called on an invalid address" << std::endl;
    crash();
  }

  // Check the cookie value.
  if (*reinterpret_cast<uintptr_t*>(reinterpret_cast<char*>(p) + allocated[address]) != kCookie) {
    std::cerr << "free() is called on corrupted memory" << std::endl;
    crash();
  }

  allocated.erase(address);

  free(p);
}

// Called before storing a value into `p`.
void _diysan_store(void* p) {
  const uintptr_t address = reinterpret_cast<uintptr_t>(p);
  for (const auto& pair : allocated) {
    const uintptr_t end = pair.first + pair.second;
    if (end <= address && address < end + sizeof(kCookie)) {
      std::cerr << "Storing a value into Out-of-Bounds region" << std::endl;
      crash();
    }
  }
}

// Called on `exit()`.
void _diysan_exit(int status) {
  std::cerr << "exit(" << status << ")" << std::endl;

  if (!allocated.empty()) {
    std::cerr << "Memory leak detected" << std::endl;
    crash();
  }

  exit(status);
}

// Called on `exit()`.
void _diysan_exit(int status) {
  std::cerr << "exit(" << status << ")" << std::endl;

  exit(status);
}

}
