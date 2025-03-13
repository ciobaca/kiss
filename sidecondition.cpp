#include "sidecondition.h"
#include "helper.h"

bool SideCondition::isSolved()
{
  return t->isVariable();
}

vector<Variable *> SideCondition::vars()
{
  vector<Variable *> result;
  result.push_back(X);
  append(result, t->vars());
  return unique(result);
}
