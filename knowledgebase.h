#ifndef KNOWLEDGEBASE_H__
#define KNOWLEDGEBASE_H__

#include "deductionfact.h"
#include "equationalfact.h"
#include "substitution.h"

struct KnowledgeBase
{
  vector<DeductionFact> deductionFacts;
  vector<EquationalFact> equationalFacts;
  bool changed;

  map<Term *, Term *> generated;

  Term *generates(Term *);
  void saturate(RewriteSystem &rewriteSystem);

  void narrow(DeductionFact &fact, RewriteSystem &rewriteSystem, int n);

  void addContextFact(Function *);
  void addClosedFact(Term *, Term *);
};

ostream &operator<<(ostream &, KnowledgeBase &);

Substitution clashAvoider(vector<Variable *>, vector<Variable *>);

#endif
