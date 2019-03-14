#include "circuit_creator_default.h"

#include <unordered_set>


//==================================================================================================
namespace {

std::vector<simple_circuitt::gatet*> literals_to_input_gates(
  boolbv_mapt::literal_mapt const &literals,
  std::unordered_map<int, simple_circuitt::gatet*> &literal_to_gate,
  int &counter,
  simple_circuitt &circuit)
{
  std::vector<simple_circuitt::gatet*> gates;
  for(auto lit: literals)
  {
    if(!lit.is_set)
      throw std::runtime_error{"Literal of INPUT varialble is not set"};

    auto gate = circuit.create_input_gate(std::to_string(counter++));
    literal_to_gate[lit.l.dimacs()] = gate;

    gates.push_back(gate);
  }

  return gates;
}

std::vector<simple_circuitt::gatet*> literals_to_output_gates(
  boolbv_mapt::literal_mapt const &literals,
  std::unordered_map<int, simple_circuitt::gatet*> &literal_to_gate,
  circuit_creator_defaultt &creator,
  simple_circuitt &circuit)
{
  std::vector<simple_circuitt::gatet*> gates;
  for(auto maybe_lit: literals)
  {
    if(!maybe_lit.is_set)
      throw std::runtime_error{"Literal of OUTPUT varialble is not set"};

    auto gate = circuit.create_output_gate(std::to_string(maybe_lit.l.dimacs()));
    gate->add_fanin({convert_node(maybe_lit.l, literal_to_gate, creator, circuit), 0}, 0);
    gates.push_back(gate);
  }

  return gates;
}

}

void circuit_creator_defaultt::create_circuit(
  std::vector<mpc_variable_infot> const &vars,
  std::vector<bool_function_callt> const &func_calls,
  simple_circuitt &circuit)
{
  std::unordered_map<int, simple_circuitt::gatet*> literal_to_gate;

  // Convert input variables
  int counter = 1;
  for(auto &&info: vars)
  {
    if(info.io_type != io_variable_typet::output)
    {
      std::vector<simple_circuitt::gatet*> gates = literals_to_input_gates(info.literals, literal_to_gate, counter, circuit);
      variable_ownert owner = variable_ownert::input_alice;
      if(info.io_type == io_variable_typet::input_b)
        owner = variable_ownert::input_bob;

      circuit.add_variable(str(info.var.unqualified_name), owner, from_cbmc(info.type), std::move(gates));
    }
  }

  // Convert inputs/outputs for function calls
  for(bool_function_callt const &call: func_calls)
  {
    simple_circuitt::function_callt circ_call;
    circ_call.name = as_string(call.name);
    circ_call.call_id = call.call_id;

    // The function's returns are our inputs
    for(auto const &var: call.returns)
    {
      std::vector<simple_circuitt::gatet*> gates = literals_to_input_gates(var.literals, literal_to_gate, counter, circuit);
      circ_call.returns.push_back({str(var.var.unqualified_name), from_cbmc(var.type), std::move(gates)});
    }

    // The function's arguments are our outputs
    for(auto const &var: call.args)
    {
      std::vector<simple_circuitt::gatet*> gates = literals_to_output_gates(var.literals, literal_to_gate, *this, circuit);
      circ_call.args.push_back({str(var.var.unqualified_name), from_cbmc(var.type), std::move(gates)});
    }

    circuit.add_function_call(std::move(circ_call));
  }

  // Convert circuit outputs
  for(auto &&info: vars)
  {
    if(info.io_type == io_variable_typet::output)
    {
      std::vector<simple_circuitt::gatet*> outs = literals_to_output_gates(info.literals, literal_to_gate, *this, circuit);
      circuit.add_variable(str(info.var.unqualified_name), variable_ownert::output, from_cbmc(info.type), std::move(outs));
    }
  }
}


//==================================================================================================
namespace {

char const* cstr(tmp_node_kindt kind)
{
  switch(kind)
  {
    case tmp_node_kindt::l_and: return "AND";
    case tmp_node_kindt::l_or: return "OR";
    case tmp_node_kindt::l_xor: return "XOR";
    case tmp_node_kindt::input: return "INPUT";
  }
}

void node_wires_to_dot(std::ostream &os, tmp_nodet const &node)
{
  switch(node.kind)
  {
    case tmp_node_kindt::l_and:
    case tmp_node_kindt::l_or:
    case tmp_node_kindt::l_xor:
      os << "\t\t" << node.input_left.dimacs() << " -> " << node.me.dimacs() << ";\n";
      os << "\t\t" << node.input_right.dimacs() << " -> " << node.me.dimacs() << ";\n";
      break;

    case tmp_node_kindt::input:
      // Nothing to do
      break;
  }
}

void scope_to_dot(
  std::ostream &os,
  circuit_creator_defaultt::scopet *scope,
  std::unordered_map<int, tmp_nodet> const& graph,
  std::unordered_set<int> &nodes_written)
{
  os << "\tsubgraph cluster_" << scope << " {\n";
  os << "\t\tlabel = \"" << scope->name << "\";\n";

  for(auto lit: scope->literals)
  {
    tmp_nodet const &node = graph.at(lit.dimacs());

    if(nodes_written.insert(lit.dimacs()).second)
    {
      // If the literal is negated we need to output a NOT gate first
      if(lit.sign())
      {
        literalt unsigned_lit = !lit;
        if(nodes_written.insert(unsigned_lit.dimacs()).second)
        {
          os << "\t\t" << unsigned_lit.dimacs() << " [label=\"" << cstr(node.kind) << "\"];\n";
          node_wires_to_dot(os, node);

          os << "\t\t" << lit.dimacs() << " [label=\"NOT\"];\n";
          os << "\t\t" << unsigned_lit.dimacs() << " -> " << lit.dimacs() << ";\n";
        }
      }
      else
      {
        os << "\t\t" << lit.dimacs() << " [label=\"" << cstr(node.kind) << "\"];\n";
        node_wires_to_dot(os, node);
      }
    }
  }

  for(auto const &child: scope->children)
    scope_to_dot(os, child.get(), graph, nodes_written);

  os << "\t}\n";
}

}

void circuit_creator_defaultt::scopes_to_dot(std::ostream &os)
{
  os << "digraph {\n";

  std::unordered_set<int> nodes_written;
  scope_to_dot(os, &m_root_scope, m_literal_to_node, nodes_written);

  os << "}\n";
}
