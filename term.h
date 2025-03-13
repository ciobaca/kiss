#ifndef TERM_H__
#define TERM_H__

#include <string>
#include <vector>

#include "name.h"
#include "variable.h"
#include "function.h"
#include "rewrite.h"
#include "substitution.h"
#include "helper.h"
#include "frame.h"

using namespace std;

struct Substitution;
struct FunTerm;
struct VarTerm;
struct NamTerm;
struct KnowledgeBase;

struct Term
{
  virtual string toString() = 0;

  bool computedVars;
  vector<Variable *> allVars;

  Term() {
    computedVars = false;
  }

  virtual vector<Variable *> vars();
  virtual vector<Name *> names() = 0;

  virtual Term *substitute(Substitution &);
  virtual Term *computeSubstitution(Substitution &, map<Term *, Term *> &) = 0;

  virtual vector<Variable *> computeVars() = 0;

  virtual bool isNormalized(RewriteSystem &rewriteSystem);
  virtual Term *normalize(RewriteSystem &rewriteSystem);

  virtual bool unifyWith(Term *, Substitution &) = 0;
  virtual bool unifyWithFunTerm(FunTerm *, Substitution &) = 0;
  virtual bool unifyWithVarTerm(VarTerm *, Substitution &) = 0;
  virtual bool unifyWithNamTerm(NamTerm *, Substitution &) = 0;

  virtual vector<pair<Term *, Term *> > split() = 0;

  virtual Term *generatedBy(KnowledgeBase &, map<Term *, Term *> &) = 0;

  virtual bool isInstanceOf(Term *, Substitution &);

  virtual bool isVariable() = 0;

  virtual Term *applyFrame(Frame *phi);

  virtual ~Term() {}

  virtual bool computeIsNormalized(RewriteSystem &, map<Term *, bool> &) = 0;
  virtual Term *computeNormalize(RewriteSystem &, map<Term *, Term *> &) = 0;

  virtual bool computeIsInstanceOf(Term *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;
  virtual bool computeIsGeneralizationOf(NamTerm *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;
  virtual bool computeIsGeneralizationOf(VarTerm *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;
  virtual bool computeIsGeneralizationOf(FunTerm *, Substitution &, map<pair<Term *, Term *>, bool> &) = 0;

  virtual int dagSize();
  virtual int computeDagSize(map<Term *, int> &) = 0;

  virtual Term *computeApplyFrame(Frame *, map<Term *, Term *> &) = 0;
};

void logmgu(string, Term *, Term *, Substitution &);

#endif
