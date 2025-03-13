#ifndef VARTERM_H__
#define VARTERM_H__

#include "term.h"

struct VarTerm : public Term
{
  Variable *variable;

  VarTerm(Variable *variable);

  virtual string toString();

  virtual vector<Variable *> computeVars();
  virtual vector<Name *> names();

  virtual Term *computeSubstitution(Substitution &, map<Term *, Term *> &);

  virtual bool unifyWith(Term *, Substitution &);
  virtual bool unifyWithFunTerm(FunTerm *, Substitution &);
  virtual bool unifyWithVarTerm(VarTerm *, Substitution &);
  virtual bool unifyWithNamTerm(NamTerm *, Substitution &);

  virtual bool isVariable();

  virtual Term *computeApplyFrame(Frame *phi, map<Term *, Term *> &);

  virtual vector<pair<Term *, Term *> > split();

  virtual Term *generatedBy(KnowledgeBase &, map<Term *, Term *> &);

  virtual bool computeIsNormalized(RewriteSystem &rewriteSystem, map<Term *, bool> &);
  virtual Term *computeNormalize(RewriteSystem &, map<Term *, Term *> &);

  virtual bool computeIsInstanceOf(Term *, Substitution &, map<pair<Term *, Term *>, bool> &);
  virtual bool computeIsGeneralizationOf(NamTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);
  virtual bool computeIsGeneralizationOf(VarTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);
  virtual bool computeIsGeneralizationOf(FunTerm *, Substitution &, map<pair<Term *, Term *>, bool> &);

  virtual int computeDagSize(map<Term *, int> &);
};

#endif
