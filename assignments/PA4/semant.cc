

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"

#include <queue>
#include <set>


extern int semant_debug;
extern char *curr_filename;
ClassTable *class_table;
SymbolTable<Symbol,Symbol> *objects_table;

Class_ current_class;

std::map<Symbol, std::map<Symbol, method_class*>> class_methods;
std::map<Symbol, std::map<Symbol, attr_class*>> class_attrs;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}



ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* Fill this in */
    install_basic_classes();
    install_custom_classes(classes);
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);
    
    class_map[Object] = Object_class;
    class_map[IO] = IO_class;
    class_map[Int] = Int_class;
    class_map[Bool] = Bool_class;
    class_map[Str] = Str_class;
}

bool ClassTable::install_custom_classes(Classes classes) {
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        Class_ c = classes->nth(i);
        Symbol name = c->get_name();
        if(name == SELF_TYPE || name == Str || name == Int || name == Bool || name == Object) {
            semant_error(c) << "Redefinition of basic class " << name << "is not allowed." << endl;
            return false;
        }
        if(class_map.find(name) != class_map.end()) {
            semant_error(c) << "Class " << name << " was previously defined." << endl;
            return false;
        }
        class_map[name] = c;
    }
    return true;
}

bool ClassTable::get_parent_classes_and_check_inheritance() {
    std::map<Symbol, int> in;
    for(auto it = class_map.begin(); it != class_map.end(); it++) {
        auto name = it->first;
        if(name == Object) {
            continue;
        }
        Class_ c = it->second;
        Symbol parent = c->get_parent_name();
        if(parent == Int || parent == Bool || parent == Str || parent == SELF_TYPE) {
            semant_error(c) << "Class " << name << " cannot inherit class " << parent << "." << endl;
            return false;
        }
        if(class_map.find(parent) == class_map.end()) {
            semant_error(c) << "Class " << name << " inherits from an undefined class " << parent << "." << endl;
            return false;
        }
        in[parent]++;
        parent_map[c->get_name()] = parent;
    }
    int sz = class_map.size();
    std::queue<Symbol> q;
    for(auto it = class_map.begin(); it != class_map.end(); it++) {
        auto name = it->first;
        if(in[name] == 0) {
            q.push(name);
        }
    }
    while(!q.empty()) { // topological sort
        Symbol name = q.front();
        q.pop();
        sz--;
        Symbol parent = parent_map[name];
        if(parent != No_class) {
            in[parent]--;
            if(in[parent] == 0) {
                q.push(parent);
            }
        }
    }
    if(sz > 0){
        semant_error() << "Class inheritance graph has a cycle." << endl;
    }
    return true;
}

bool ClassTable::check_main_class() {
    if(class_map.find(Main) == class_map.end()) {
        semant_error() << "Class Main is not defined." << endl;
        return false;
    }

    Class_ main_class = class_map[Main];
    Features features = main_class->get_features();

    bool find_main = false;
    for (int i = features->first(); features->more(i); i = features->next(i))
        if (features->nth(i)->is_method() && static_cast<method_class*>(features->nth(i))->get_name() == main_meth)
            find_main = true;
    
    if (!find_main)
        semant_error(main_class) << "No 'main' method in class Main.\n";
    return true;
}

bool ClassTable::is_subclass(Symbol type1, Symbol type2) {
    if (type1 == No_type)
        return true;

    if (type1 == SELF_TYPE)
        type1 = current_class->get_name();

    if (type2 == SELF_TYPE)
        type2 = current_class->get_name();

    while (type1 != Object && type1 != type2)
        type1 = parent_map[type1];

    return type1 == type2;
}

Symbol ClassTable::get_lca(Symbol type1, Symbol type2) {
    std::set<Symbol> ancestors;
    Symbol lca = Object;
    if(type1 == type2) return type1;

    if (type1 == SELF_TYPE)
        type1 = current_class->get_name();

    if (type2 == SELF_TYPE)
        type2 = current_class->get_name();

    while (type1 != Object) {
        ancestors.insert(type1);
        type1 = parent_map[type1];
    }

    while (type2 != Object) {
        if (ancestors.find(type2) != ancestors.end()) {
            lca = type2;
            break;
        }
        type2 = parent_map[type2];
    }

    return lca;
}

/*
    CLASS UTILITIES
*/

std::map<Symbol, method_class*> get_class_methods(Class_ class_definition) {
    std::map<Symbol, method_class*> class_methods;
    Symbol class_name = class_definition->get_name();
    Features class_features = class_definition->get_features();

    for (int i = class_features->first(); class_features->more(i); i = class_features->next(i)) 
    {
        Feature feature = class_features->nth(i);

        if (!feature->is_method())
            continue;

        method_class* method = static_cast<method_class*>(feature);
        Symbol method_name = method->get_name();
        
        if (class_methods.find(method_name) != class_methods.end())
        {
            ostream& error_stream = class_table->semant_error(class_definition);
            error_stream << "The method :";
            method_name->print(error_stream);
            error_stream << " has already been defined!\n";
        }
        else
        {
            class_methods[method_name] = method;
        }
    }
    return class_methods;
}

method_class* get_class_method(Symbol class_name, Symbol method_name) {
    std::map<Symbol, method_class*> methods = class_methods[class_name];

    if (methods.find(method_name) == methods.end())
        return nullptr;

    return methods[method_name];
}


attr_class* get_class_attr(Symbol class_name, Symbol attr_name) {
    std::map<Symbol, attr_class*> attrs = class_attrs[class_name];

    if (attrs.find(attr_name) == attrs.end())
        return nullptr;

    return attrs[attr_name];
}

std::map<Symbol, attr_class*> get_class_attributes(Class_ class_definition) {
    std::map<Symbol, attr_class*> class_attrs;
    Symbol class_name = class_definition->get_name();
    Features class_features = class_definition->get_features();

    for (int i = class_features->first(); class_features->more(i); i = class_features->next(i)) 
    {
        Feature feature = class_features->nth(i);

        if (!feature->is_attr())
            continue;

        attr_class* attr = static_cast<attr_class*>(feature);
        Symbol attr_name = attr->get_name();
        if (class_attrs.find(attr_name) != class_attrs.end())
        {
            class_table->semant_error(class_definition)
                << "The attribute :"
                << attr_name
                << " has already been defined!\n";
        }
        class_attrs[attr_name] = attr;
    }

    return class_attrs;
}


void build_attribute_scopes(Class_ current_class) {
    const auto& attrs = class_attrs[current_class->get_name()];
    for(const auto &x : attrs) {
        attr_class* attr_definition = x.second;
        if(attr_definition->get_name() == self){
            class_table->semant_error(attr_definition) 
                << "Attribute " 
                << attr_definition->get_name()
                << " cannot be named self.\n";
                continue;
        }
        if(attr_definition->get_type() != SELF_TYPE && class_table->class_map.find(attr_definition->get_type()) == class_table->class_map.end()){
            class_table->semant_error(attr_definition) 
                << "Class "
                << attr_definition->get_type()
                << " of attribute "
                << attr_definition->get_name()
                << " is undefined.\n";
                continue;
        }

        objects_table->addid(
            attr_definition->get_name(), 
            new Symbol(attr_definition->get_type())
        );
    }

    if(current_class->get_name() == Object)
        return;

    Class_ parent_definition = class_table->class_map[class_table->parent_map[current_class->get_name()]];
    build_attribute_scopes(parent_definition);
}

void process_attr(Class_ origin_class, Class_ current_class, attr_class* attr) {
    if (get_class_attr(current_class->get_name(), attr->get_name()) != nullptr)
    {
        class_table->semant_error(origin_class) 
            << "Attribute " 
            << attr->get_name()
            << " is an attribute of an inherited class.\n";
        cerr << "Compilation halted due to static semantic errors." << endl;
    }

    if(current_class->get_name() == Object) {
        return;
    }

    Symbol parent_type_name = current_class->get_parent_name();
    Class_ parent_definition = class_table->class_map[parent_type_name];
    process_attr(origin_class, parent_definition, attr);
}

void process_method(Class_ current_class, method_class* original_method, method_class* parent_method) {
    if (parent_method == nullptr)
        return;

    Formals original_method_args = original_method->get_formals();
    Formals parent_method_args = parent_method->get_formals();
    
    int original_formal_ix = 0;
    int parent_formal_ix = 0;
    
    if(original_method->get_return_type() != parent_method->get_return_type()) {
        class_table->semant_error(current_class) 
            << "In redefined method " 
            << original_method->get_name() 
            << ", the return type " 
            << original_method->get_return_type() 
            << " differs from the ancestor method return type "
            << parent_method->get_return_type() 
            << ".\n";
    }

    int original_methods_formals = 0;
    int parent_method_formals = 0;

    while (original_method_args->more(original_methods_formals))
        original_methods_formals = original_method_args->next(original_methods_formals);

    while (parent_method_args->more(parent_method_formals))
        parent_method_formals = parent_method_args->next(parent_method_formals);

    if (original_methods_formals != parent_method_formals) {
        class_table->semant_error(current_class) 
            << "In redefined method " 
            << original_method->get_name() 
            << ", the number of arguments " 
            << "(" << original_methods_formals << ")"
            << " differs from the ancestor method's "
            << "number of arguments "
            << "(" << parent_method_formals << ")"
            << ".\n";
    }

    while (
        original_method_args->more(original_formal_ix) &&
        parent_method_args->more(parent_formal_ix)
    )
    {
        Formal original_formal = original_method_args->nth(original_formal_ix);
        Formal parent_formal = parent_method_args->nth(parent_formal_ix);

        if (original_formal->get_type() != parent_formal->get_type()) {
            class_table->semant_error(current_class) 
                << "In redefined method " 
                << original_method->get_name() 
                << ", the return type of argument " 
                << original_formal->get_type() 
                << " differs from the corresponding ancestor method argument return type "
                << parent_formal->get_type() 
                << ".\n";
        }

        original_formal_ix = original_method_args->next(original_formal_ix);
        parent_formal_ix = parent_method_args->next(parent_formal_ix);
    }

    Symbol parent_type_name = class_table->parent_map[current_class->get_name()];

    if (parent_type_name == No_type)
        return;

    Class_ parent_class_definition = class_table->class_map[parent_type_name];

    process_method(
        parent_class_definition, 
        original_method,
        get_class_method(
            parent_type_name, 
            original_method->get_name()
        )
    );
}

void ClassTable::register_class_and_its_methods() {
    for(auto const& x : class_map) {
        Class_ class_definition = x.second;
        class_methods[class_definition->get_name()] = get_class_methods(class_definition);
        class_attrs[class_definition->get_name()] = get_class_attributes(class_definition);
    }
}

/*
    TYPECHECKING
*/

void ClassTable::type_check(Classes classes) {
    for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
        current_class = classes->nth(i);

        const auto& current_class_methods = class_methods[current_class->get_name()];
        const auto& current_class_attrs = class_attrs[current_class->get_name()];

        objects_table->enterscope();
        objects_table->addid(self, new Symbol(current_class->get_name()));

        build_attribute_scopes(current_class);
        
        for (const auto &x : current_class_methods) {
            process_method(current_class, x.second, x.second);
        }

        for (const auto &x : current_class_attrs) {
            Class_ parent_definition = class_table->class_map[current_class->get_parent_name()];
            process_attr(current_class, parent_definition, x.second);
        }

        for (const auto &x : current_class_attrs) {
            x.second->type_check();
        }

        for (const auto &x : current_class_methods) {
            x.second->type_check();
        }

        objects_table->exitscope();
    }

}

Symbol object_class::type_check() {
    if (name == self) {
        this->set_type(SELF_TYPE);
        return SELF_TYPE;
    }

    Symbol* object_type = objects_table->lookup(name);
    if (object_type == nullptr) {
        class_table->semant_error(this) 
            << "Undeclared identifier " 
            << name 
            << ".\n";
        object_type = &Object;
    }
    this->set_type(*object_type);
    return *object_type;
}

Symbol no_expr_class::type_check() {
    this->set_type(No_type);
    return No_type;
}

Symbol isvoid_class::type_check() {
    e1->type_check();
    this->set_type(Bool);
    return Bool;
}

Symbol new__class::type_check() {
    if (type_name != SELF_TYPE && class_table->class_map.find(type_name) == class_table->class_map.end()) {
        class_table->semant_error(this) 
            << "'new' used with undefined class " 
            << type_name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    this->set_type(type_name);
    return type_name;
}

Symbol comp_class::type_check() {
    Symbol e1_type = e1->type_check();
    if (e1_type != Bool) {
        class_table->semant_error(this) 
            << "Argument of 'not' has type " 
            << e1_type 
            << " instead of Bool.\n";
        e1_type = Object;
    }
    this->set_type(e1_type);
    return e1_type;
}

Symbol leq_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " <= " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Bool);
    return Bool;
}

Symbol eq_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type == Int || e1_type == Str || e1_type == Bool || e2_type == Int || e2_type == Str || e2_type == Bool) {
        if (e1_type != e2_type) {
            class_table->semant_error(this) 
                << "Illegal comparison with a basic type.\n";
            e1_type = Object;
        }
    }
    this->set_type(Bool);
    return Bool;
}

Symbol lt_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " < " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Bool);
    return Bool;
}

Symbol neg_class::type_check() {
    Symbol e1_type = e1->type_check();
    if (e1_type != Int) {
        class_table->semant_error(this) 
            << "Argument of '~' has type " 
            << e1_type 
            << " instead of Int.\n";
        e1_type = Object;
    }
    this->set_type(Int);
    return Int;
}

Symbol divide_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " / " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Int);
    return Int;
}

Symbol mul_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " * " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Int);
    return Int;
}

Symbol sub_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " - " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Int);
    return Int;
}

Symbol plus_class::type_check() {
    Symbol e1_type = e1->type_check();
    Symbol e2_type = e2->type_check();
    if (e1_type != Int || e2_type != Int) {
        class_table->semant_error(this) 
            << "non-Int arguments: " 
            << e1_type 
            << " + " 
            << e2_type 
            << ".\n";
        e1_type = Object;
    }
    this->set_type(Int);
    return Int;
}

Symbol let_class::type_check() {
    objects_table->enterscope();
    if (identifier == self) {
        class_table->semant_error(this) 
            << "'self' cannot be bound in a 'let' expression.\n";
    }

    Symbol init_type = init->type_check();

    if (type_decl != SELF_TYPE && class_table->class_map.find(type_decl) == class_table->class_map.end()) {
        class_table->semant_error(this) 
            << "Class " 
            << type_decl 
            << " of let-bound identifier " 
            << identifier 
            << " is undefined.\n";
    }else if (init_type != No_type && !class_table->is_subclass(init_type, type_decl)) {
        class_table->semant_error(this) 
            << "Inferred type " 
            << init_type 
            << " of initialization of " 
            << identifier 
            << " does not conform to identifier's declared type " 
            << type_decl 
            << ".\n";
    }

    objects_table->addid(identifier, new Symbol(type_decl));
    this->set_type(body->type_check());
    objects_table->exitscope();
    return type;
}

Symbol block_class::type_check() {
    Symbol type = Object;
    for(auto i = body->first(); body->more(i); i = body->next(i)) {
        type = body->nth(i)->type_check();
    }
    this->set_type(type);
    return type;
}

Symbol branch_class::type_check() {
    if(name == self) {
        class_table->semant_error(this) 
            << "'self' bound in 'case'.\n";
    }
    objects_table->enterscope();
    objects_table->addid(name, new Symbol(type_decl));
    Symbol expr_type = expr->type_check();
    objects_table->exitscope();
    return expr_type;
}

Symbol typcase_class::type_check() {
    Symbol expr_type = expr->type_check();
    Symbol type = Object;

    std::set<Symbol> branch_types;
    for (auto i = cases->first(); cases->more(i); i = cases->next(i)) {
        auto branch = static_cast<branch_class*>(cases->nth(i));
        if (branch_types.find(branch->get_type_decl()) != branch_types.end()) {
            class_table->semant_error(this) 
                << "Duplicate branch " 
                << branch->get_type_decl() 
                << " in case statement.\n";
        } else {
            branch_types.insert(branch->get_type_decl());
        }

        if (i == cases->first()){
            type = branch->type_check();
        } else {
            type = class_table->get_lca(type, branch->type_check());
        }
    }
    this->set_type(type);
    return type;
}

Symbol loop_class::type_check() {
    if (pred->type_check() != Bool) {
        class_table->semant_error(this) 
            << "Loop condition does not have type Bool.\n";
    }
    body->type_check();
    this->set_type(Object);
    return Object;
}

Symbol cond_class::type_check() {
    if (pred->type_check() != Bool) {
        class_table->semant_error(this) 
            << "Predicate of 'if' does not have type Bool.\n";
    }
    Symbol then_type = then_exp->type_check();
    Symbol else_type = else_exp->type_check();
    Symbol type = class_table->get_lca(then_type, else_type);
    this->set_type(type);
    return type;
}

Symbol dispatch_class::type_check() {
    Symbol expr_type = expr->type_check();
    if (expr_type == SELF_TYPE) {
        expr_type = current_class->get_name();
    }
    if (class_table->class_map.find(expr_type) == class_table->class_map.end()) {
        class_table->semant_error(this) 
            << "Dispatch on undefined class " 
            << expr_type 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    auto t = expr_type;
    method_class* method = nullptr;
    while (t != No_class) {
        auto methods = class_methods[t];
        if (methods.find(name) != methods.end()) {
            method = methods[name];
            break;
        }
        t = class_table->class_map[t]->get_parent_name();
    }

    if (method == nullptr) {
        class_table->semant_error(this) 
            << "Dispatch to undefined method " 
            << name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    if (actual->len() != method->get_formals()->len()) {
        class_table->semant_error(this) 
            << "Method " 
            << name 
            << " called with wrong number of arguments.\n";
        this->set_type(Object);
        return Object;
    }

    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        auto actual_type = actual->nth(i)->type_check();
        auto formal_type = method->get_formals()->nth(i)->get_type();
        if (!class_table->is_subclass(actual_type, formal_type)) {
            class_table->semant_error(this) 
                << "In call of method " 
                << name 
                << ", type " 
                << actual_type 
                << " of parameter " 
                << method->get_formals()->nth(i)->get_name() 
                << " does not conform to declared type " 
                << formal_type 
                << ".\n";
            this->set_type(Object);
            return Object;
        }
    }

    this->set_type(method->get_return_type());
    if (type == SELF_TYPE && current_class->get_name() != expr_type) {
        type = expr_type;
    }
    return type;
}

Symbol static_dispatch_class::type_check() {
    Symbol expr_type = expr->type_check();
    if (expr_type == SELF_TYPE) {
        expr_type = current_class->get_name();
    }
    if (class_table->class_map.find(expr_type) == class_table->class_map.end()) {
        class_table->semant_error(this) 
            << "Static dispatch to undefined class " 
            << expr_type 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    if (!class_table->is_subclass(expr_type, type_name)) {
        class_table->semant_error(this) 
            << "Expression type " 
            << expr_type 
            << " does not conform to declared static dispatch type " 
            << type_name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    if (class_methods[type_name].find(name) == class_methods[type_name].end()) {
        class_table->semant_error(this) 
            << "Static dispatch to undefined method " 
            << name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    method_class* method = class_methods[type_name][name];

    if (actual->len() != method->get_formals()->len()) {
        class_table->semant_error(this) 
            << "Method " 
            << name 
            << " called with wrong number of arguments.\n";
        this->set_type(Object);
        return Object;
    }

    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        auto actual_type = actual->nth(i)->type_check();
        auto formal_type = method->get_formals()->nth(i)->get_type();
        if (actual_type == SELF_TYPE) {
            actual_type = expr_type;
        }
        if (!class_table->is_subclass(actual_type, formal_type)) {
            class_table->semant_error(this) 
                << "In call of method " 
                << name 
                << ", type " 
                << actual_type 
                << " of parameter " 
                << method->get_formals()->nth(i)->get_name() 
                << " does not conform to declared type " 
                << formal_type 
                << ".\n";
            this->set_type(Object);
            return Object;
        }
    }

    this->set_type(method->get_return_type());
    if (type == SELF_TYPE) {
        type = expr_type;
    }
    return type;
}

Symbol assign_class::type_check() {
    if (name == self) {
        class_table->semant_error(this) 
            << "Cannot assign to 'self'.\n";
        this->set_type(Object);
        return Object;
    }

    Symbol* id_type = objects_table->lookup(name);
    if (id_type == nullptr) {
        class_table->semant_error(this) 
            << "Assignment to undeclared variable " 
            << name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    Symbol expr_type = expr->type_check();

    if (!class_table->is_subclass(expr_type, *id_type)) {
        class_table->semant_error(this) 
            << "Type " 
            << expr_type 
            << " of assigned expression does not conform to declared type " 
            << *id_type 
            << " of identifier " 
            << name 
            << ".\n";
        this->set_type(Object);
        return Object;
    }

    this->set_type(expr_type);
    return expr_type;
}

Symbol method_class::type_check() {
    objects_table->enterscope();
    std::set<Symbol> defined_args;

    for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
        auto formal = formals->nth(i);
        if (formal->get_name() == self) {
            class_table->semant_error(this) 
                << "'self' cannot be the name of a formal parameter.\n";
        } else {
            if (defined_args.find(formal->get_name()) != defined_args.end()) {
                class_table->semant_error(this) 
                    << "Formal parameter " 
                    << formal->get_name() 
                    << " is multiply defined.\n";
            } else {
                defined_args.insert(formal->get_name());
                objects_table->addid(formal->get_name(), new Symbol(formal->get_type()));
            }
        }

        if (formal->get_type() == SELF_TYPE) {
            class_table->semant_error(this) 
                << "Formal parameter " 
                << formal->get_name() 
                << " cannot have type SELF_TYPE.\n";
        } else {
            if (class_table->class_map.find(formal->get_type()) == class_table->class_map.end()) {
                class_table->semant_error(this) 
                    << "Class " 
                    << formal->get_type() 
                    << " of formal parameter " 
                    << formal->get_name() 
                    << " is undefined.\n";
            }
        }
    }

    Symbol expr_type = expr->type_check();
    if (!class_table->is_subclass(expr_type, return_type)) {
        class_table->semant_error(this) 
            << "Inferred return type " 
            << expr_type 
            << " of method " 
            << name 
            << " does not conform to declared return type " 
            << return_type 
            << ".\n";
    }

    objects_table->exitscope();
    return return_type;
}

Symbol attr_class::type_check() {
    auto init_type = init->type_check();
    if (init_type == SELF_TYPE) {
        init_type = current_class->get_name();
    }

    if (init_type != No_type && !class_table->is_subclass(init_type, type_decl)) {
        class_table->semant_error(this) 
            << "Inferred type " 
            << init_type 
            << " of initialization of attribute " 
            << name 
            << " does not conform to declared type " 
            << type_decl 
            << ".\n";
    }

    return type_decl;
}

Symbol int_const_class::type_check() {
    this->set_type(Int);
    return Int;
}

Symbol bool_const_class::type_check() {
    this->set_type(Bool);
    return Bool;
}

Symbol string_const_class::type_check() {
    this->set_type(Str);
    return Str;
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(tree_node *t)
{
    return semant_error(current_class->get_filename(), t);
}

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 



/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    class_table = new ClassTable(classes);
    if (class_table->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }

    /* some semantic analysis code may go here */
    class_table->get_parent_classes_and_check_inheritance();

    if (class_table->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }

    objects_table = new SymbolTable<Symbol, Symbol>();
    class_table->check_main_class();
    class_table->register_class_and_its_methods();
    class_table->type_check(classes);

    if (class_table->errors()) {
        cerr << "Compilation halted due to static semantic errors." << endl;
        exit(1);
    }
}


