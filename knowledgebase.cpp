#include "knowledgebase.h"
#include "factories.h"
#include <sstream>
#include <iostream>

using namespace std;

Term *KnowledgeBase::generates(Term *t)
{
  map<Term *, Term *> cache;
  Term *recipe = t->generatedBy(*this, cache);
  return recipe;
}

Term *fillContext(Term *context, Term *hole)
{
  Substitution subst;
  subst.add(getVariable("\\_"), hole);
  return context->substitute(subst);
}

void KnowledgeBase::narrow(DeductionFact &fact, RewriteSystem &rewriteSystem, int n)
{
  vector<pair<Term *, Term *> > splits = fact.T->split();

  for (int i = 0; i < len(rewriteSystem); ++i) {
    Term *l = rewriteSystem[i].first;
    Term *r = rewriteSystem[i].second;

    for (int j = 0; j < len(splits); ++j) {
      Substitution sigma;
      Term *T = splits[j].second;
      if (!T->isVariable() && T->unifyWith(l, sigma)) {
	DeductionFact newFact = fact;
	newFact.T = fillContext(splits[j].first, r);
	Substitution empty;
	newFact.apply(sigma, empty);

	DeductionFact clashfreeFact = newFact.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
	//	cout << "Narrow Solving " << clashfreeFact << endl;
	clashfreeFact.solve(*this, n);
	//	cout << "Done Narrow" << endl;
      }
    }
  }
}

void KnowledgeBase::saturate(RewriteSystem &rewriteSystem)
{
  RewriteSystem clashfreeRewriteSystem = rewriteSystem.rename(REWRITE);

  while (1) {
    int n = len(deductionFacts);
    for (int i = 0; i < n; ++i) {
      narrow(deductionFacts[i], clashfreeRewriteSystem, n);
    }
    if (len(deductionFacts) == n) {
      break;
    }
  }

  //  cout << "First saturation phase done" << endl;

  //  cout << (*this) << endl;

  //  cout << "Count: " << len(deductionFacts) << endl;

  for (int i = 0; i < len(deductionFacts) - 1; ++i) {
    for (int j = i + 1; j < len(deductionFacts); ++j) {
      //      cout << "unify " << i << " with " << j << endl;
      
      //      cout << "Unifying " << deductionFacts[i] << endl;
      //      cout << "with " << deductionFacts[j] << endl;

      DeductionFact f1 = deductionFacts[i];
      DeductionFact f2 = deductionFacts[j].rename(SMALL_TEMP, BIG_TEMP);
      //      cout << "rename " << f2 << endl; 
      Substitution sigma;
      if (f1.T->unifyWith(f2.T, sigma)) {
	EquationalFact e;
	e.U = f1.R;
	e.V = f2.R;
	for (int i = 0; i < len(f1.sideConditions); ++i) {
	  SideCondition side(
			     f1.sideConditions[i].X,
			     f1.sideConditions[i].t->substitute(sigma));
	  e.sideConditions.push_back(side);
	}
	for (int i = 0; i < len(f2.sideConditions); ++i) {
	  SideCondition side(
			     f2.sideConditions[i].X,
			     f2.sideConditions[i].t->substitute(sigma));
	  e.sideConditions.push_back(side);
	}
	//	cout << "Unifying " << f1 << endl;
	//	cout << "with " << f2 << endl;
	EquationalFact clashFree = e.rename(SMALL_UNSOLVED, BIG_UNSOLVED);
	//	cout << "Result is " << e << endl;
	clashFree.solve(*this);
      }
    }
  }
}

void KnowledgeBase::addContextFact(Function *f)
{
  vector<Variable *> big;
  vector<Variable *> small;

  int counter = 0;
  for (int i = 0; i < f->arity; ++i) {
    ostringstream osss;
    osss << SMALL_SOLVED << counter;
    small.push_back(getInternalVariable(osss.str()));

    ostringstream ossb;
    ossb << BIG_SOLVED << counter;
    big.push_back(getInternalVariable(ossb.str()));

    counter++;
  }

  DeductionFact fact;
  vector<Term *> bigArguments, smallArguments;
  for (int i = 0; i < f->arity; ++i) {
    bigArguments.push_back(getVarTerm(big[i]));
    smallArguments.push_back(getVarTerm(small[i]));
  }
  fact.R = getFunTerm(f, bigArguments);
  fact.T = getFunTerm(f, smallArguments);
  for (int i = 0; i < f->arity; ++i) {
    fact.sideConditions.push_back(SideCondition(big[i], getVarTerm(small[i])));
  }
  deductionFacts.push_back(fact.rename(SMALL_SOLVED, BIG_SOLVED));
}

void KnowledgeBase::addClosedFact(Term *R, Term *T)
{
  DeductionFact fact;

  fact.R = R;
  fact.T = T;
  deductionFacts.push_back(fact.rename(SMALL_SOLVED, BIG_SOLVED));
}

ostream &operator<<(ostream &os, KnowledgeBase &knowledgeBase)
{
  os << "Deduction facts: " << endl;
  for (int i = 0; i < len(knowledgeBase.deductionFacts); ++i) {
    os << "   " << knowledgeBase.deductionFacts[i] << endl;
    //    os << knowledgeBase.deductionFacts[i].R->dagSize() << " and " << knowledgeBase.deductionFacts[i].T->dagSize() << endl;
  }
  os << "Equational facts: " << endl;
  for (int i = 0; i < len(knowledgeBase.equationalFacts); ++i) {
    os << "   " << knowledgeBase.equationalFacts[i] << endl;
    //    os << knowledgeBase.equationalFacts[i].U->dagSize() << " and " << knowledgeBase.equationalFacts[i].V->dagSize() << endl;
  }
  return os;
}

