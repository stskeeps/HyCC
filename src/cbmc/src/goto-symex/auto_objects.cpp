/*******************************************************************\

Module: Symbolic Execution of ANSI-C

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Symbolic Execution of ANSI-C

#include "goto_symex.h"

#include <util/prefix.h>
#include <util/cprover_prefix.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>

exprt goto_symext::make_auto_object(const typet &type)
{
  dynamic_counter++;

  // produce auto-object symbol
  symbolt symbol;

  symbol.base_name="auto_object"+std::to_string(dynamic_counter);
  symbol.name="symex::"+id2string(symbol.base_name);
  symbol.is_lvalue=true;
  symbol.type=type;
  symbol.mode=ID_C;

  new_symbol_table.add(symbol);

  return symbol_exprt(symbol.name, symbol.type);
}

void goto_symext::initialize_auto_object(
  const exprt &expr,
  statet &state)
{
  const typet &type=ns.follow(expr.type());

  if(type.id()==ID_struct)
  {
    const struct_typet &struct_type=to_struct_type(type);

    for(const auto &comp : struct_type.components())
    {
      member_exprt member_expr;
      member_expr.struct_op()=expr;
      member_expr.set_component_name(comp.get_name());
      member_expr.type()=comp.type();

      initialize_auto_object(member_expr, state);
    }
  }
  else if(type.id()==ID_pointer)
  {
    const pointer_typet &pointer_type=to_pointer_type(type);
    const typet &subtype=ns.follow(type.subtype());

    // we don't like function pointers and
    // we don't like void *
    if(subtype.id()!=ID_code &&
       subtype.id()!=ID_empty)
    {
      // could be NULL nondeterministically

      address_of_exprt address_of_expr(
        make_auto_object(type.subtype()),
        pointer_type);

      if_exprt rhs(
        side_effect_expr_nondett(bool_typet()),
        null_pointer_exprt(pointer_type),
        address_of_expr);

      code_assignt assignment(expr, rhs);
      symex_assign(state, assignment);
    }
  }
}

void goto_symext::trigger_auto_object(
  const exprt &expr,
  statet &state)
{
  if(expr.id()==ID_symbol &&
     expr.get_bool(ID_C_SSA_symbol))
  {
    const ssa_exprt &ssa_expr=to_ssa_expr(expr);
    const irep_idt &obj_identifier=ssa_expr.get_object_name();

    if(obj_identifier!="goto_symex::\\guard")
    {
      const symbolt &symbol=
        ns.lookup(obj_identifier);

      if(has_prefix(id2string(symbol.base_name), "auto_object"))
      {
        // done already?
        if(state.level2.current_names.find(ssa_expr.get_identifier())==
           state.level2.current_names.end())
        {
          initialize_auto_object(expr, state);
        }
      }
    }
  }

  // recursive call
  forall_operands(it, expr)
    trigger_auto_object(*it, state);
}
