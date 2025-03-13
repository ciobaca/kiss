#include <map>
#include "funterm.h"
#include "varterm.h"
#include "namterm.h"
#include "factories.h"
#include "helper.h"
#include <sstream>
#include <cassert>

using namespace std;

map<string, Variable *> variables;
map<string, Function *> functions;
map<string, Name *> names;
map<pair<Function *, vector<Term *> >, Term *> funTerms;
map<Variable *, Term *> varTerms;
map<Name *, Term *> namTerms;

Variable *getVariable(string name)
{
  if (contains(variables, name)) {
    return variables[name];
  } else {
    return 0;
  }
}

void createVariable(string name)
{
#ifndef NDEBUG
  Variable *v = getVariable(name);
  assert(!v);
#endif

  variables[name] = new Variable(name);
}

Variable *getInternalVariable(string name)
{
  if (!getVariable(name)) {
    createVariable(name);
  }
  return getVariable(name);
}

Variable *createFreshVariable()
{
  static int number = 0;
  ostringstream oss;
  oss << "_" << number++;
  string freshName = oss.str();
  createVariable(freshName);
  return getVariable(freshName);
}

Function *getFunction(string name)
{
  if (contains(functions, name)) {
    return functions[name];
  } else {
    return 0;
  }
}

void createFunction(string name, int arity)
{
#ifndef NDEBUG
  Function *f = getFunction(name);
  assert(f == 0);
#endif

  functions[name] = new Function(name, arity);
}

Name *getName(string name)
{
  if (contains(names, name)) {
    return names[name];
  }
  return 0;
}

void createName(string name)
{
#ifndef NDEBUG
  Name *n = getName(name);
  assert(n == 0);
#endif

  names[name] = new Name(name);
}

Term *getFunTerm(Function *f, vector<Term *> arguments)
{
  pair<Function *, vector<Term *> > content = make_pair(f, arguments);
  if (contains(funTerms, content)) {
    return funTerms[content];
  } else {
    return funTerms[content] = new FunTerm(f, arguments);
  }
}

Term *getVarTerm(Variable *v)
{
  if (contains(varTerms, v)) {
    return varTerms[v];
  } else {
    return varTerms[v] = new VarTerm(v);
  }
}

Term *getNamTerm(Name *n)
{
  if (contains(namTerms, n)) {
    return namTerms[n];
  } else {
    return namTerms[n] = new NamTerm(n);
  }
}
