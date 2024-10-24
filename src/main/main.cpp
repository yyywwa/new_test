#include "any.hpp"
#include "util.hpp"

using namespace mtd;
class T1 {
public:
  virtual void f() { println("T1"); }
  virtual void f2() { println("T1-2"); }

  ~T1() { println("deleted ", this); }
};

class T2 : public T1 {
public:
  virtual void f() { println("T2"); }
  virtual void f2() { println("T2-2"); }
};

class T3 : public T2 {
public:
  virtual void f() { println("T3"); }
  virtual void f2() { println("T3-2"); }
};

int main() {
  Any t = T1{};
  return 0;
}
