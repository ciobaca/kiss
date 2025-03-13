#ifndef DEDUCTIONFACT_H__
#define DEDUCTIONFACT_H__

#include "fact.h"
#include "helper.h"
#include <vector>

using namespace std;

struct DeductionFact : public Fact
{
  Term *R;
  Term *T;

  virtual void apply(Substitution &, Substitution &);
  virtual vector<Variable *> vars();
  virtual void combineInto(KnowledgeBase &);

  virtual string toString();

  void applyClashAvoider(Substitution &);

  virtual bool isCanonical();
  DeductionFact canonicalize();

  virtual void solve(KnowledgeBase &, int n);

  DeductionFact rename(string, string);

  virtual ostream &output(ostream &);
};

ostream &operator<<(ostream &os, DeductionFact &deductionFact);

#endif
