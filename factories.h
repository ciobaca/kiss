#ifndef FACTORIES_H__
#define FACTORIES_H__

#include <string>
#include "term.h"

struct Function;
struct Variable;
struct Name;
struct Term;

using namespace std;

Variable *getVariable(string name);
void createVariable(string name);

Variable *getInternalVariable(string name);

Function *getFunction(string name);
void createFunction(string name, int arity);

Name *getName(string name);
void createName(string name);

Term *getFunTerm(Function *f, vector<Term *> arguments);
Term *getVarTerm(Variable *v);
Term *getNamTerm(Name *n);

#endif
