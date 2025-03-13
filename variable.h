#ifndef VARIABLE_H__
#define VARIABLE_H__

#include <string>

using namespace std;

struct Substitution;

struct Variable
{
  string name;
  Variable *rename(Substitution &);

private:
  Variable(string name)
  {
    this->name = name;
  }

  friend void createVariable(string);
};


#endif
