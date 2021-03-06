
#ifndef GEN_386_H
#define GEN_386_H

#include "Code_Gen.h"

//Represents the stack for the current function's frame
//every new scope starts off with the stack pointer being 16-byte aligned
//the earliest declared variable has the highest mem addr, the latest declared has the lowest addr
//padding always sits under the variables on the stack
struct StackMan_i386 : public StackMan {

   std::string load_var(Variable var, Scope *ts = nullptr, int total_adjust = 0) {

      if (ext_adj) {
         total_adjust = ext_adj;
         ext_adj = 0;
      }

      int stack_loc = 0;
      for (size_t i = 0; i < params.size(); ++i) {
         if (params[i].name.compare(var.name) == 0) {
            stack_loc = i * 4 + 8;// +8 accounts for push %ebp and 4 byte return address
            return std::to_string(stack_loc) + "(%ebp)";
         }
      }


      if (scope && !ts) {

         for (size_t i = 0; i < scope->variables.size(); ++i) {

            if (scope->variables[i].name.compare(var.name) == 0) {
               int padding = 16 - ((scope->variables.size() * 4) % 16);
               if (padding == 16) padding = 0;
               stack_loc = i * 4 + padding + total_adjust;
               return std::to_string(stack_loc) + "(%esp)";
            }
         }
      } else if (ts) {
         printf("TS\n");
         for (size_t i = 0; i < ts->variables.size(); ++i) {

            if (ts->variables[i].name.compare(var.name) == 0) {
               int padding = 16 - ((ts->variables.size() * 4) % 16);
               if (padding == 16) padding = 0;
               stack_loc = i * 4 + padding + total_adjust;
               return std::to_string(stack_loc) + "(%esp)";
            }
         }
      }
      Scope *sc = (ts ? ts : scope);
      int padding = 16 - ((sc->variables.size() * 4) % 16);
      if (padding == 16) padding = 0;
      int adj = padding + sc->variables.size() * 4;
      if (sc->variables.size() == 0) {
         adj = 0;
      }
      adj += total_adjust;
      if ((ts ? !ts->parent : !scope->parent)) {
         return "";
      }
      return (scope->is_function ? ""
         : (ts ? (ts->is_function ? "" : load_var(var, ts->parent, adj)) : load_var(var, scope->parent, adj)));
   }

};

struct Gen_386 : public Code_Gen {

   Gen_386(std::ostream &ost) : Code_Gen(ost) {
      stack_man = new StackMan_i386();
      stack_man->code_gen = this;
   }

   virtual std::string gen_var(Variable var);
   virtual void gen_stack_alignment(Scope &scope);
   virtual void gen_stack_unalignment(Scope &scope);
   virtual void gen_stack_pop_params(std::vector<Variable> &plist);
   virtual int gen_stack_unwind(Scope &scope);

   virtual void gen_rodata();

   virtual void emit_cmp(Variable src0, Variable src1);
   virtual void emit_inc(Variable dst);
   virtual void emit_push(Variable src);
   virtual void emit_pop(Variable dst);
   virtual void emit_mov(Variable src, Variable dst);
   virtual void emit_sub(Variable src, Variable dst);
   virtual void emit_add(Variable src, Variable dst);
   virtual void emit_or(Variable src, Variable dst);
   virtual void emit_call(std::string label);
   virtual void emit_jump(std::string label);
   virtual void emit_cond_jump(std::string label, Conditional::CType condition);
   virtual void emit_return();
   virtual void emit_function_header();
   virtual void emit_function_footer();
};



#endif
