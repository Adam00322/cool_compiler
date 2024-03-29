#include "cgen.h"
#include "cgen_gc.h"

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;
static EnvTable* envTable = nullptr;
static CgenClassTable *codegen_classtable = nullptr;
//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
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

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  envTable = new EnvTable();
  codegen_classtable = new CgenClassTable(classes,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}

static void emit_start_frame(ostream &s) {
    emit_addiu(SP, SP, -12, s);
    emit_store(FP, 3, SP, s);
    emit_store(SELF, 2, SP, s);
    emit_store(RA, 1, SP, s);
    emit_addiu(FP, SP, 4, s);
    emit_move(SELF, ACC, s);
}

static void emit_end_frame(ostream &s) {
    // emit_move(ACC, SELF, s);
    emit_load(FP, 3, SP, s);
    emit_load(SELF, 2, SP, s);
    emit_load(RA, 1, SP, s);
    emit_addiu(SP, SP, 12, s);
}

static void emit_abort(int lebal, int lineno, ostream &s) {
    emit_bne(ACC, ZERO, lebal, s);
    s << LA << ACC << " str_const0" << endl;
    emit_load_imm(T1, lineno, s);
    emit_jal("_dispatch_abort", s);
}

static void emit_gc_update(char *src, int offset, ostream &s) {
    if (cgen_Memmgr == GC_GENGC) {
        emit_addiu(A1, src, WORD_SIZE * offset, s);
        emit_gc_assign(s);
   }
}

static void emit_load_t1_t2(ostream &s, Expression e1, Expression e2) {
    e1->code(s);
    emit_push(ACC, s);
    e2->code(s);

    emit_load(T1, 1, SP, s);
    emit_move(T2, ACC, s);
    emit_addiu(SP, SP, 4, s);
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;

      s << STRINGNAME << DISPTAB_SUFFIX;  s << endl;          // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 

      s << INTNAME << DISPTAB_SUFFIX;  s << endl;         // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

      s << BOOLNAME << DISPTAB_SUFFIX;  s << endl;          // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL 
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL 
      << WORD << stringclasstag << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}


CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
   stringclasstag = 1;
   intclasstag =    2;
   boolclasstag =   3;

   codegen_classtable = this;
   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();
   install_name_to_cgen();
   install_classtags();
   install_attrs_and_methods();

   code();
   exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
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
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
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
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

void CgenClassTable::install_name_to_cgen(){
    for(List<CgenNode> *l = nds; l; l = l->tl()){
        name_to_cgen_map_[l->hd()->get_name()] = l->hd();
    }
}

void CgenClassTable::install_classtags(){
    int curr_tag = 0;

    std::function<void(CgenNodeP)> dfs_set_tags = [&](CgenNodeP curr_cgen) {
        curr_cgen->set_classtag(curr_tag);
        class_tag_map_[curr_tag] = curr_cgen->get_name();
        Symbol curr_name = curr_cgen->get_name();
        if (curr_name == Str) {
            stringclasstag = curr_tag;
        } else if (curr_name == Bool) {
            boolclasstag = curr_tag;
        } else if (curr_name == Int) {
            intclasstag = curr_tag;
        }
        curr_tag++;
        for (List<CgenNode> *l = curr_cgen->get_children(); l; l = l->tl()) { // 遍历子节点
            dfs_set_tags(l->hd());
        }
    };

    std::function<int(CgenNodeP)> set_des_cnt = [&](CgenNodeP curr_cgen) {
        int des_cnt = 1; // 包括自己
        for (List<CgenNode> *l = curr_cgen->get_children(); l; l = l->tl()) {
            des_cnt += set_des_cnt(l->hd());
        }
        curr_cgen->set_descendants_cnt(des_cnt - 1);
        return des_cnt;
    };

    dfs_set_tags(name_to_cgen_map_[Object]);
    set_des_cnt(name_to_cgen_map_[Object]);
}

void CgenClassTable::install_attrs_and_methods(){
    CgenNodeP curr_cgennode;
    Features curr_features;
    for (List<CgenNode> *l = nds; l; l = l->tl()) {
        curr_cgennode = l->hd();
        curr_features = curr_cgennode->get_features();
        Feature curr_feature;
        Symbol curr_cgen_name = curr_cgennode->get_name();
        for (int i = curr_features->first(); curr_features->more(i); i = curr_features->next(i)) {
            curr_feature = curr_features->nth(i);
            if (curr_feature->is_method()) {
                class_method_map_[curr_cgen_name].push_back(static_cast<method_class*>(curr_feature));
            } else {
                class_attr_map_[curr_cgennode->get_name()].push_back(static_cast<attr_class*>(curr_feature));
            }
        }
    }
   for (List<CgenNode> *l = nds; l; l = l->tl()) {
        curr_cgennode = l->hd();
        Symbol curr_name = curr_cgennode->get_name();
        CgenNodeP pa = curr_cgennode;
        std::vector<CgenNodeP> chain;
        while (true) {
            chain.push_back(pa);
            if (pa->get_name() == Object) {
                break;
            }
            pa = pa->get_parentnd();
        }
        std::reverse(chain.begin(), chain.end());
        for (auto parent : chain) {
            auto methods = class_method_map_[parent->get_name()];
            for (auto method : methods) {  // 获取其中的method
                Symbol meth_name = method->get_name();

                if (meth_offset_map_[curr_name].find(meth_name) == meth_offset_map_[curr_name].end()) {
                    dispatch_tab_map_[curr_name].push_back({parent->get_name(), meth_name});
                    meth_offset_map_[curr_name][meth_name] = dispatch_tab_map_[curr_name].size() - 1;
                } else {
                    auto meth_offset = meth_offset_map_[curr_name][meth_name];
                    dispatch_tab_map_[curr_name][meth_offset] = {parent->get_name(), meth_name};
                }
            }
        }
        curr_cgennode->set_chain_depth(chain.size());
        parent_chain_map_[curr_name] = std::move(chain);
   }
}


void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();
  if (cgen_debug) cout << "coding class name tab" << endl;
  code_class_nametabs();
  if (cgen_debug) cout << "coding class object table" << endl;
  code_class_objtabs();
  if (cgen_debug) cout << "coding object dispatabs" << endl;
  code_object_disptabs();
  if (cgen_debug) cout << "coding object prot" << endl;
  code_protobjs();
  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();
  if (cgen_debug) cout << "coding object init method" << endl;
  code_object_inits();
  if (cgen_debug) cout << "coding methods" << endl;
  code_methods();
}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}

///////////////////////////////////////////////////////////////////////
//
// CgenClassTable methods
//
///////////////////////////////////////////////////////////////////////
void CgenClassTable::code_class_nametabs() {
    str << CLASSNAMETAB << LABEL;
    int len = class_tag_map_.size();

    StringEntry* str_entry;
    for (int i = 0; i < len; i++) {
        Symbol name = class_tag_map_[i];
        str_entry = stringtable.lookup_string(name->get_string());
        str << WORD;
        str_entry->code_ref(str);
        str << endl;
    }
}

void CgenClassTable::code_class_objtabs() {
    str << CLASSOBJTAB << LABEL;
    int len = class_tag_map_.size();
    StringEntry* str_entry;
    for (int i = 0; i < len; i++) {
        Symbol name = class_tag_map_[i];
        str << WORD;
        emit_protobj_ref(name, str);
        str << endl;
        str << WORD;
        emit_init_ref(name, str);
        str << endl;
    }
}

bool CgenClassTable::get_attr_offset(Symbol cls, Symbol attr, int *offset) {
    if (attr_offset_map_.find(cls) == attr_offset_map_.end()) {
        return false;
    }
    if (attr_offset_map_[cls].find(attr) == attr_offset_map_[cls].end()) {
        return false;
    }
    *offset = attr_offset_map_[cls][attr];
    return true;
}

bool CgenClassTable::get_meth_offset(Symbol cls1, Symbol cls2, Symbol meth, int *offset) {
    if (dispatch_tab_map_.find(cls2) == dispatch_tab_map_.end()) {
        return false;
    }
    int curr_offset = 0;
    for (auto &t : dispatch_tab_map_[cls2]) {
        auto& class_name = t.first;
        auto& meth_name = t.second;
        if (class_name == cls2 && meth_name == meth) {
            *offset = curr_offset;
            return true;
        }
        curr_offset++;
    }
    return true;
}

bool CgenClassTable::get_meth_offset(Symbol cls, Symbol meth, int *offset) {
    if (meth_offset_map_.find(cls) == meth_offset_map_.end()) {
        return false;
    }
    auto find_meth = meth_offset_map_[cls].find(meth);
    if (find_meth != meth_offset_map_[cls].end()) {
        *offset = find_meth->second;
        return true;
    }
    return false;
}

void CgenClassTable::code_object_disptabs() {
    List<CgenNode> *nd_list = nds;
    CgenNode *head;

    for (auto &t : dispatch_tab_map_) {
        auto& class_name = t.first;
        auto& distab = t.second;
        emit_disptable_ref(class_name, str);
        str << LABEL;
        for (auto &p : distab) {
            str << WORD;
            emit_method_ref(p.first, p.second, str);
            str << endl;
        }
    }

}

void CgenClassTable::code_protobjs() {
    CgenNodeP curr_cgennode, curr_parent;
    for (List<CgenNode> *l = nds; l; l = l->tl()) {
        curr_cgennode = l->hd();
        Symbol curr_name = curr_cgennode->get_name();
        str << WORD << -1 << endl;
        emit_protobj_ref(curr_name, str);
        str << LABEL;
        str << WORD << curr_cgennode->get_classtag() << endl;
        int attr_cnt = class_attr_map_[curr_name].size();
        auto parents = parent_chain_map_[curr_name];
        for (auto parent : parents) {
            Symbol parent_name = parent->get_name();
            if (parent_name == curr_cgennode->get_name()) {
                continue;
            }
            attr_cnt += class_attr_map_[parent_name].size();
        }
        str << WORD << attr_cnt + ATTR_BASE_OFFSET << endl;
        str << WORD;
        emit_disptable_ref(curr_name, str);
        str << endl;
        int offset = ATTR_BASE_OFFSET; //
        for (auto parent : parents) {
            Symbol parent_name = parent->get_name();
            const auto& curr_attr_list = class_attr_map_[parent_name];
            for (auto attr : curr_attr_list) {
                Symbol attr_type = attr->get_type();
                str << WORD;
                attr_offset_map_[curr_name][attr->get_name()] = offset++;
                if (attr_type == Str) {
                    StringEntry *strentry = stringtable.lookup_string("");
                    strentry->code_ref(str);
                } else if (attr_type == Bool) {
                    falsebool.code_ref(str);
                } else if (attr_type == Int) {
                    IntEntry *intentry = inttable.lookup_string("0");
                    intentry->code_ref(str);
                } else {
                    str << 0;
                }
                str << endl;
            }
        }
    }
}



void CgenClassTable::code_object_inits() {
    CgenNodeP curr_cgen;
    for (List<CgenNode> *l = nds; l; l = l->tl()) {
        curr_cgen = l->hd();
        curr_cgenclass_ = curr_cgen;
        emit_init_ref(curr_cgen->get_name(), str);
        str << LABEL;
        emit_start_frame(str);
        envTable->enterframe();

        CgenNodeP parent = curr_cgen->get_parentnd();
        if (parent && parent->get_name() != No_class) {
            str << JAL;
            emit_init_ref(parent->get_name(), str);
            str << endl;
        }
        // 处理中间的attr,这一部分比较复杂, 只是处理本层的attr
        const auto& curr_attrs = class_attr_map_[curr_cgen->get_name()];
        for (auto attr : curr_attrs) {
            Expression init_expr = attr->get_init();
            Symbol attr_type = attr->get_type();
            if (init_expr->get_type() != NULL) {
                init_expr->code(str);
                int attr_off = attr_offset_map_[curr_cgen->get_name()][attr->get_name()];
                emit_store(ACC, attr_off, SELF, str);
                emit_gc_update(SELF, attr_off, str);
            }
        }
        emit_move(ACC, SELF, str);
        emit_end_frame(str);
        envTable->exitframe();
        emit_return(str);
    }
}

void CgenClassTable::code_methods() {
    CgenNodeP curr_cgennode;
    for (List<CgenNode> *l = nds; l; l = l->tl()) {
        curr_cgennode = l->hd();
        curr_cgenclass_ = curr_cgennode;
        Symbol cgenname = curr_cgenclass_->get_name();
        if (cgenname == Object || cgenname == Str || cgenname == Bool || cgenname == Int || cgenname == IO) { // basic中的method无需做出处理
            continue;
        }
        // 只处理本层的method
        const auto& methods = class_method_map_[cgenname];
        Formals curr_formals;
        std::list<Formal> formal_list;
        for (auto method : methods) {
            formal_list.clear();
            envTable->enterframe();
            curr_formals = method->formals;
            for (int i = curr_formals->first(); curr_formals->more(i); i = curr_formals->next(i)) {
                formal_list.push_front(curr_formals->nth(i));
            }
            for (auto formal : formal_list) {
                envTable->add_formal_id(formal->get_name());
            }
            emit_method_ref(curr_cgenclass_->get_name(), method->get_name(), str);
            str << LABEL;
            emit_start_frame(str);
            method->expr->code(str);
            emit_end_frame(str);
            emit_addiu(SP, SP, formal_list.size() * WORD_SIZE, str);
            emit_return(str);
            envTable->exitframe();
        }
    }
}

///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   class_tag_(0),
   basic_status(bstatus)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream &s) {
    expr->code(s);
    CgenNodeP curr_cgen = codegen_classtable->get_curr_class();
    int offset;
    if (envTable->lookup(name, &offset)) {
        emit_store(ACC, offset, FP, s);
        emit_gc_update(FP, offset, s);
        return;
    }
    if (codegen_classtable->get_attr_offset(curr_cgen->get_name(), name, &offset)) {
        emit_store(ACC, offset, SELF, s);
        emit_gc_update(SELF, offset, s);
    }
}

void static_dispatch_class::code(ostream &s) {
    Expression curr_expr;
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        curr_expr = actual->nth(i);
        curr_expr->code(s);
        emit_push(ACC, s);
    }
    expr->code(s);

    int lebalid = codegen_classtable->get_labelid_and_add();

    emit_abort(lebalid, get_line_number(), s);

    emit_label_def(lebalid, s);
    std::string suffix = DISPTAB_SUFFIX;
    std::string distab_addr = type_name->get_string() + suffix;
    emit_load_address(T1, const_cast<char*>(distab_addr.c_str()), s);
    int offset;
    codegen_classtable->get_meth_offset(type_name, name, &offset);

    emit_load(T1, offset, T1, s);
    emit_jalr(T1, s);
}

void dispatch_class::code(ostream &s) {
    // 首先将参数全部压栈
    Expression curr_expr;
    for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
        curr_expr = actual->nth(i);
        curr_expr->code(s);
        emit_push(ACC, s);
    }
    expr->code(s);
    int lebalid = codegen_classtable->get_labelid_and_add();
    emit_abort(lebalid, get_line_number(), s);
    emit_label_def(lebalid, s);
    emit_load(T1, DISPTABLE_OFFSET, ACC, s); // 将dispacth表加载到T1中
    int offset;
    Symbol expr_type = expr->get_type();
    if (expr_type == SELF_TYPE) {
        expr_type = codegen_classtable->get_curr_class()->get_name();
    }
    codegen_classtable->get_meth_offset(expr_type, name, &offset);
    emit_load(T1, offset, T1, s); // 获取该dispatch的地址

    emit_jalr(T1, s);
}

void cond_class::code(ostream &s) {
    pred->code(s);
    emit_load(T1, ATTR_BASE_OFFSET, ACC, s);
    emit_move(T2, ZERO, s);

    int out_lebal = codegen_classtable->get_labelid_and_add();
    int false_lebal = codegen_classtable->get_labelid_and_add();

    emit_beq(T1, T2, false_lebal, s);
    then_exp->code(s);
    emit_branch(out_lebal, s);

    emit_label_def(false_lebal, s);
    else_exp->code(s);
    emit_label_def(out_lebal, s);
}

void loop_class::code(ostream &s) {
    int start_lebal = codegen_classtable->get_labelid_and_add();
    int end_lebal = codegen_classtable->get_labelid_and_add();

    emit_label_def(start_lebal, s);
    pred->code(s);
    emit_load(T1, ATTR_BASE_OFFSET, ACC, s);
    emit_beq(T1, ZERO, end_lebal, s); // 条件等于0时结束

    body->code(s);
    emit_branch(start_lebal, s); // 跳转回去
    emit_label_def(end_lebal, s); // 结束循环对应的lebal

    emit_move(ACC, ZERO, s);
}

void typcase_class::code(ostream &s) {
    expr->code(s);
    int no_void_lebal = codegen_classtable->get_labelid_and_add();
    int out_lebal = codegen_classtable->get_labelid_and_add();
    int notmatch_lebal = codegen_classtable->get_labelid_and_add();
    emit_bne(ACC, ZERO, no_void_lebal, s);
    s << LA << ACC << " str_const0" << endl;
    emit_load_imm(T1, get_line_number(), s);
    emit_jal("_case_abort2", s);
    emit_label_def(no_void_lebal, s);
    emit_load(T1, TAG_OFFSET, ACC, s);
    std::vector<Case> sorted_cases;
    sorted_cases.reserve(cases->len());
    Case curr_case;
    for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
        curr_case = cases->nth(i);
        sorted_cases.push_back(curr_case);
    }
    std::function<bool(Case, Case)> sort_comp = [&](Case a, Case b)-> bool {
        return codegen_classtable->get_cgennode(a->get_type_decl())->get_chain_depth() >
            codegen_classtable->get_cgennode(b->get_type_decl())->get_chain_depth();
    };
    std::sort(sorted_cases.begin(), sorted_cases.end(), sort_comp);
    for (auto case_class : sorted_cases) {
        Symbol cgen_type = case_class->get_type_decl();
        CgenNodeP cgen = codegen_classtable->get_cgennode(cgen_type);
        int next_case_lebal = codegen_classtable->get_labelid_and_add();
        int start_tag = cgen->get_classtag();
        int end_tag = start_tag + cgen->get_descendants_cnt();
        emit_blti(T1, start_tag, next_case_lebal, s);
        emit_bgti(T1, end_tag, next_case_lebal, s);
        emit_push(ACC, s);
        envTable->enterscope();
        envTable->add_local_id(case_class->get_name());
        case_class->get_expr()->code(s);
        envTable->exitscope();
        emit_addiu(SP, SP, 4, s);
        emit_branch(out_lebal, s);
        emit_label_def(next_case_lebal, s);
    }
    emit_label_def(notmatch_lebal, s);
    emit_jal("_case_abort", s);
    emit_label_def(out_lebal, s);
}

void block_class::code(ostream &s) {
    Expressions exprs = body;
    Expression expr;
    for (int i = exprs->first(); exprs->more(i); i = exprs->next(i)) {
        expr = exprs->nth(i);
        expr->code(s);
    }
}

void let_class::code(ostream &s) {
    init->code(s);
    if (init->get_type() == NULL) { // 如果初始化表达式是空的，初始化为默认值
        if (type_decl == Int) {
            emit_load_int(ACC, inttable.lookup_string("0"), s);
        } else if (type_decl == Str) {
            emit_load_string(ACC, stringtable.lookup_string(""), s);
        } else if (type_decl == Bool) {
            emit_load_bool(ACC, falsebool, s);
        }
    }

    emit_push(ACC, s); // 入栈
    envTable->enterscope();
    envTable->add_local_id(identifier);  // 加入到环境表
    body->code(s);

    envTable->exitscope();
    emit_addiu(SP, SP, 4, s);
}

void plus_class::code(ostream &s) {
    e1->code(s);
    emit_push(ACC, s);
    e2->code(s);
    emit_jal("Object.copy", s);
    emit_load(T1, 1, SP, s);
    emit_load(T2, ATTR_BASE_OFFSET, T1, s);
    emit_load(T3, ATTR_BASE_OFFSET, ACC, s);
    emit_addiu(SP, SP, 4, s);
    emit_add(T3, T2, T3, s);
    emit_store(T3, ATTR_BASE_OFFSET, ACC, s);
}

void sub_class::code(ostream &s) {
    e1->code(s);
    emit_push(ACC, s);
    e2->code(s);
    emit_jal("Object.copy", s);
    emit_load(T1, 1, SP, s);
    emit_load(T2, ATTR_BASE_OFFSET, T1, s);
    emit_load(T3, ATTR_BASE_OFFSET, ACC, s);
    emit_addiu(SP, SP, 4, s);
    emit_sub(T3, T2, T3, s);
    emit_store(T3, ATTR_BASE_OFFSET, ACC, s);
}

void mul_class::code(ostream &s) {
    e1->code(s);
    emit_push(ACC, s);
    e2->code(s);
    emit_jal("Object.copy", s);
    emit_load(T1, 1, SP, s);
    emit_load(T2, ATTR_BASE_OFFSET, T1, s);
    emit_load(T3, ATTR_BASE_OFFSET, ACC, s);
    emit_addiu(SP, SP, 4, s);
    emit_mul(T3, T2, T3, s);
    emit_store(T3, ATTR_BASE_OFFSET, ACC, s);
}

void divide_class::code(ostream &s) {
    e1->code(s);
    emit_push(ACC, s);
    e2->code(s);
    emit_jal("Object.copy", s);
    emit_load(T1, 1, SP, s);
    emit_load(T2, ATTR_BASE_OFFSET, T1, s);
    emit_load(T3, ATTR_BASE_OFFSET, ACC, s);
    emit_addiu(SP, SP, 4, s);
    emit_div(T3, T2, T3, s);
    emit_store(T3, ATTR_BASE_OFFSET, ACC, s);
}

void neg_class::code(ostream &s) {
    e1->code(s);
    emit_jal("Object.copy", s);
    emit_load(T1, ATTR_BASE_OFFSET, ACC, s);
    emit_neg(T1, T1, s);
    emit_store(T1, ATTR_BASE_OFFSET, ACC, s);
}

void lt_class::code(ostream &s) {
    emit_load_t1_t2(s, e1, e2);
    emit_load(T1, ATTR_BASE_OFFSET, T1, s);
    emit_load(T2, ATTR_BASE_OFFSET, T2, s);
    int out_lebal = codegen_classtable->get_labelid_and_add();
    emit_load_bool(ACC, truebool, s);
    emit_blt(T1, T2, out_lebal, s);
    emit_load_bool(ACC, falsebool, s);
    emit_label_def(out_lebal, s);
}

void eq_class::code(ostream &s) {
    emit_load_t1_t2(s, e1, e2);
    Symbol e1type = e1->get_type();
    Symbol e2type = e2->get_type();
    if ((e1type == Int || e1type == Bool || e1type == Str)
        && (e2type == Int || e2type == Bool || e2type == Str)) {
        emit_load_bool(ACC, truebool, s);
        emit_load_bool(A1, falsebool, s);
        emit_jal("equality_test", s);
        return;
    }
    int lebalid = codegen_classtable->get_labelid_and_add();

    emit_load_bool(ACC, truebool, s);
    emit_beq(T1, T2, lebalid, s);
    emit_load_bool(ACC, falsebool, s);
    emit_label_def(lebalid, s);
}

void leq_class::code(ostream &s) {
    emit_load_t1_t2(s, e1, e2);
    emit_load(T1, ATTR_BASE_OFFSET, T1, s);
    emit_load(T2, ATTR_BASE_OFFSET, T2, s);

    int lebalid = codegen_classtable->get_labelid_and_add();
    emit_load_bool(ACC, truebool, s);
    emit_bleq(T1, T2, lebalid, s);
    emit_load_bool(ACC, falsebool, s);
    emit_label_def(lebalid, s);
}

void comp_class::code(ostream &s) {
    e1->code(s);
    emit_load(T1, ATTR_BASE_OFFSET, ACC, s);

    int lebalid = codegen_classtable->get_labelid_and_add();
    emit_load_bool(ACC, truebool, s);
    emit_beq(T1, ZERO, lebalid, s);
    emit_load_bool(ACC, falsebool, s);
    emit_label_def(lebalid, s);
}

void int_const_class::code(ostream& s) {
  IntEntry *int_entry = inttable.lookup_string(token->get_string());
  emit_load_int(ACC, int_entry, s);
}

void string_const_class::code(ostream& s) {
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream& s) {
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s) {
    std::string object_name = type_name->get_string();
    if (type_name == SELF_TYPE) {
        emit_load(T1, 0, ACC, s);
        emit_load_address(T2, CLASSOBJTAB, s);
        emit_sll(T1, T1, 3, s);
        emit_add(T2, T1, T2, s);
        emit_load(ACC, 0, T2, s);
        emit_jal("Object.copy", s);

        emit_load(T1, 0, ACC, s);
        emit_load_address(T2, CLASSOBJTAB, s);
        emit_sll(T1, T1, 3, s);
        emit_add(T2, T1, T2, s);
        emit_load(A1, 1, T2, s);
        emit_jalr(A1, s);
    } else {
        std::string protobj_object = object_name + PROTOBJ_SUFFIX;
        emit_load_address(ACC, const_cast<char *>(protobj_object.c_str()), s);
        emit_jal("Object.copy", s);
        std::string init_object = object_name + CLASSINIT_SUFFIX;
        emit_jal(const_cast<char *>(init_object.c_str()), s);
    }
}

void isvoid_class::code(ostream &s) {
    e1->code(s);
    emit_move(T1, ACC, s);

    int lebal_id = codegen_classtable->get_labelid_and_add();

    emit_load_bool(ACC, truebool, s);
    emit_beq(T1, ZERO, lebal_id, s);
    emit_load_bool(ACC, falsebool, s);
    emit_label_def(lebal_id, s);
}

void no_expr_class::code(ostream &s) {
    emit_move(ACC, ZERO, s);
}

void object_class::code(ostream &s) {
    if (name == self) {
        emit_move(ACC, SELF, s);
        return;
    }
    CgenNodeP curr_cgen = codegen_classtable->get_curr_class();
    int offset;
    if (envTable->lookup(name, &offset)) {
        emit_load(ACC, offset, FP, s);
        emit_gc_update(FP, offset, s);
        return;
    }
    if (codegen_classtable->get_attr_offset(curr_cgen->get_name(), name, &offset)) {
        emit_load(ACC, offset, SELF, s);
        emit_gc_update(SELF, offset, s);
    }
}


