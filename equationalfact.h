#ifndef EQUATIONALFACT_H__
#define EQUATIONALFACT_H__

#include "fact.h"
#include "frame.h"

struct EquationalFact : public Fact
{
  Term *U;
  Term *V;

  virtual void apply(Substitution &, Substitution &);
  virtual vector<Variable *> vars();
  virtual void combineInto(KnowledgeBase &);

  virtual string toString();

  virtual void solve(KnowledgeBase &);

  bool holdsIn(Frame *phi, RewriteSystem &rewrite);

  EquationalFact rename(string, string);

  virtual ostream &output(ostream &);
};

ostream &operator<<(ostream &os, EquationalFact &deductionFact);

#endif
