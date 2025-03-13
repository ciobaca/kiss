#ifndef SIDECONDITION_H__
#define SIDECONDITION_H__

#include "term.h"
#include <vector>

using namespace std;

struct SideCondition
{
  Variable *X;
  Term *t;

  SideCondition(Variable *X, Term *t)
  {
    this->X = X;
    this->t = t;
  }

  bool isSolved();
  vector<Variable *> vars();
};

#endif
