#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#include <map>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  

public:
  bool install_custom_classes(Classes);
  bool get_parent_classes_and_check_inheritance();
  bool check_main_class();
  bool is_subclass(Symbol, Symbol);
  void register_class_and_its_methods();
  Symbol get_lca(Symbol, Symbol);
  void type_check();
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  ostream& semant_error(tree_node *t);
  std::map<Symbol, Class_> class_map;
  std::map<Symbol, Symbol> parent_map;
};


#endif

