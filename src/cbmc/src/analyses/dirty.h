/*******************************************************************\

Module: Variables whose address is taken

Author: Daniel Kroening

Date: March 2013

\*******************************************************************/

/// \file
/// Variables whose address is taken

#ifndef CPROVER_ANALYSES_DIRTY_H
#define CPROVER_ANALYSES_DIRTY_H

#include <unordered_set>

#include <util/std_expr.h>
#include <goto-programs/goto_functions.h>

class dirtyt
{
public:
  typedef std::unordered_set<irep_idt, irep_id_hash> id_sett;
  typedef goto_functionst::goto_functiont goto_functiont;

  dirtyt()
  {
  }

  explicit dirtyt(const goto_functiont &goto_function)
  {
    build(goto_function);
  }

  explicit dirtyt(const goto_functionst &goto_functions)
  {
    forall_goto_functions(it, goto_functions)
      build(it->second);
  }

  void output(std::ostream &out) const;

  bool operator()(const irep_idt &id) const
  {
    return dirty.find(id)!=dirty.end();
  }

  bool operator()(const symbol_exprt &expr) const
  {
    return operator()(expr.get_identifier());
  }

  const id_sett &get_dirty_ids() const
  {
    return dirty;
  }

  void add_function(const goto_functiont &goto_function)
  {
    build(goto_function);
  }

protected:
  void build(const goto_functiont &goto_function);

  // variables whose address is taken
  id_sett dirty;

  void find_dirty(const exprt &expr);
  void find_dirty_address_of(const exprt &expr);
};

inline std::ostream &operator<<(
  std::ostream &out,
  const dirtyt &dirty)
{
  dirty.output(out);
  return out;
}

/// Wrapper for dirtyt that permits incremental population, ensuring each
/// function is analysed exactly once.
class incremental_dirtyt
{
public:
  void populate_dirty_for_function(
    const irep_idt &id, const goto_functionst::goto_functiont &function);

  bool operator()(const irep_idt &id) const
  {
    return dirty(id);
  }

  bool operator()(const symbol_exprt &expr) const
  {
    return dirty(expr);
  }

private:
  dirtyt dirty;
  std::unordered_set<irep_idt, irep_id_hash> dirty_processed_functions;
};

#endif // CPROVER_ANALYSES_DIRTY_H
