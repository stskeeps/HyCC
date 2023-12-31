/*******************************************************************\

Module: Bounded Model Checking for ANSI-C + HDL

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Bounded Model Checking for ANSI-C + HDL

#ifndef CPROVER_CBMC_BMC_H
#define CPROVER_CBMC_BMC_H

#include <list>
#include <map>

#include <util/options.h>
#include <util/ui_message.h>

#include <solvers/prop/prop.h>
#include <solvers/prop/prop_conv.h>
#include <solvers/sat/cnf.h>
#include <solvers/sat/satcheck.h>
#include <solvers/smt1/smt1_dec.h>
#include <solvers/smt2/smt2_dec.h>

#include <goto-programs/goto_trace.h>

#include <goto-symex/symex_target_equation.h>
#include <goto-programs/safety_checker.h>
#include <goto-symex/memory_model.h>

#include "symex_bmc.h"

class bmct:public safety_checkert
{
public:
  bmct(
    const optionst &_options,
    const symbol_tablet &_symbol_table,
    message_handlert &_message_handler,
    prop_convt &_prop_conv)
    : safety_checkert(ns, _message_handler),
      options(_options),
      ns(_symbol_table, new_symbol_table),
      equation(ns),
      symex(_message_handler, ns, new_symbol_table, equation),
      prop_conv(_prop_conv),
      ui(ui_message_handlert::uit::PLAIN)
  {
    symex.constant_propagation=options.get_bool_option("propagation");
    symex.record_coverage=
      !options.get_option("symex-coverage-report").empty();
  }

  virtual resultt run(const goto_functionst &goto_functions);
  void setup();
  safety_checkert::resultt execute(const goto_functionst &);
  virtual ~bmct() { }

  // additional stuff
  expr_listt bmc_constraints;

  void set_ui(ui_message_handlert::uit _ui) { ui=_ui; }

  // the safety_checkert interface
  virtual resultt operator()(
    const goto_functionst &goto_functions)
  {
    return run(goto_functions);
  }

  void add_loop_unwind_handler(symex_bmct::loop_unwind_handlert handler)
  {
    symex.add_loop_unwind_handler(handler);
  }

  void add_unwind_recursion_handler(
    symex_bmct::recursion_unwind_handlert handler)
  {
    symex.add_recursion_unwind_handler(handler);
  }

protected:
  const optionst &options;
  symbol_tablet new_symbol_table;
  namespacet ns;
  symex_target_equationt equation;
  symex_bmct symex;
  prop_convt &prop_conv;
  std::unique_ptr<memory_model_baset> memory_model;
  // use gui format
  ui_message_handlert::uit ui;

  virtual decision_proceduret::resultt
    run_decision_procedure(prop_convt &prop_conv);

  virtual resultt decide(
    const goto_functionst &,
    prop_convt &);

  // unwinding
  virtual void setup_unwind();
  virtual void do_unwind_module();
  void do_conversion();

  virtual void freeze_program_variables();

  virtual void show_vcc();
  virtual void show_vcc_plain(std::ostream &out);
  virtual void show_vcc_json(std::ostream &out);

  trace_optionst trace_options()
  {
    return trace_optionst(options);
  }

  virtual resultt all_properties(
    const goto_functionst &goto_functions,
    prop_convt &solver);
  virtual resultt stop_on_fail(
    const goto_functionst &goto_functions,
    prop_convt &solver);
  virtual void show_program();
  virtual void report_success();
  virtual void report_failure();

  virtual void error_trace();
  void output_graphml(
    resultt result,
    const goto_functionst &goto_functions);

  void get_memory_model();
  void slice();
  void show(const goto_functionst &);

  bool cover(
    const goto_functionst &goto_functions,
    const optionst::value_listt &criteria);

  friend class bmc_all_propertiest;
  friend class bmc_covert;
  template <template <class goalt> class covert>
  friend class bmc_goal_covert;
  friend class fault_localizationt;
};

#endif // CPROVER_CBMC_BMC_H
