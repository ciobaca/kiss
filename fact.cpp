#include "fact.h"
#include "knowledgebase.h"
#include <cassert>

using namespace std;

bool Fact::isSolved()
{
  for (int i = 0; i < len(sideConditions); ++i) {
    if (!sideConditions[i].isSolved()) {
      return false;
    }
  }
  return true;
}

int Fact::firstUnsolved()
{
  assert(!isSolved());
  int i;
  for (i = 0; i < len(sideConditions); ++i) {
    if (!sideConditions[i].isSolved()) {
      break;
    }
  }
  return i;
}

ostream &operator<<(ostream &os, Fact &fact)
{
  return fact.output(os);
}

string Fact::toString()
{
  return "impossible fact";
}
