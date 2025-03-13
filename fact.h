#ifndef FACT_H__
#define FACT_H__

#include <vector>
#include <ostream>
#include "sidecondition.h"

using namespace std;

struct Substitution;
struct KnowledgeBase;

struct Fact
{
  vector<SideCondition> sideConditions;

  virtual string toString() = 0;

  virtual void apply(Substitution &, Substitution &) = 0;
  virtual vector<Variable *> vars() = 0;
  virtual void combineInto(KnowledgeBase &) = 0;
  //  virtual void solve(KnowledgeBase &) = 0;

  virtual bool isSolved();

  virtual int firstUnsolved();

  virtual ostream &output(ostream &) = 0;

  virtual ~Fact() {}
};

ostream &operator<<(ostream &os, Fact &fact);

#endif
