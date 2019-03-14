#include "program_dependence_graph.h"
#include "dominators.h"
#include "pointer_analysis.h"
#include "instr.h"


namespace ir {

namespace {

// If an element X occurs more than once in the sorted container, then all occurences of X are
// removed. This differs from unique(), which would remove all but one occurence.
// Example: [1, 2, 2, 2, 3, 4, 5, 5] -> [1, 3, 4].
template<typename Container, typename Comparer>
void keep_singletons(Container &cont, Comparer &&comp)
{
	auto cur = cont.begin();
	while(cur != cont.end())
	{
		auto last_equal = cur;
		auto lookahead = std::next(last_equal);
		while(lookahead != cont.end() && comp(*lookahead, *cur))
			++last_equal, ++lookahead;

		if(cur != last_equal)
			cur = cont.erase(cur, std::next(last_equal));
		else
			++cur;
	}
}

void create_immediate_control_dependencies(
	BasicBlock *branch_block,
	ProgramDependenceGraph &pdg,
	DominatorTree const &post_dom_tree)
{
	// Make sure `branch_block` actually branches.
	assert(branch_block->fanouts().size() > 1);

	PDGNode const *branch_node = pdg.node(branch_block->terminator());
	// `branch_post_idom` is the immediate post-dominator of `branch_block`. Thus, no block
	// following `branch_post_idom` can be control-dependent on `branch_block`.
	BasicBlock const *branch_post_idom = post_dom_tree.get_idom(branch_block);

	for(BlockEdge const &edge: branch_block->fanouts())
	{
		BasicBlock const *cur = edge.target();
		while(cur != branch_post_idom)
		{
			for(Instr const &instr: cur->instructions())
				pdg.add_control_dep(pdg.node(&instr), branch_node);

			cur = post_dom_tree.get_idom(cur);
		}
	}
}


using StoreOffsetMap = std::unordered_map<std::pair<StoreInstr const*, Decl*>, ptrdiff_t, PairHash>;

StoreInstr const* get_if_store_instr(Instr const *instr)
{
	if(instr->kind() == InstrKind::store)
		return static_cast<StoreInstr const*>(instr);

	return nullptr;
}

Region compute_transitive_dep_region(
	PDGEdge const &imm_edge,
	PDGEdge const &trans_edge,
	StoreOffsetMap const &store_offsets)
{
	Region result = trans_edge.dep_region;

	if(is_data(imm_edge) && is_data(trans_edge) && trans_edge.preserves_dep_region)
	{
		PDGNode const *parent = imm_edge.target;
		if(StoreInstr const *store = get_if_store_instr(parent->instr))
		{
			auto offset_it = store_offsets.find({store, trans_edge.variable});
			Region store_region = offset_it == store_offsets.end() ? MaxRegion : trans_edge.dep_region + offset_it->second;
			result = intersection(imm_edge.dep_region, store_region);
		}
		else
			result = intersection(imm_edge.dep_region, trans_edge.dep_region);
	}

	return result;
}

optional<PDGEdge> create_transitive_edge(
	PDGEdge const &imm_edge,
	PDGEdge const &trans_edge,
	StoreOffsetMap const &store_offsets)
{
	PDGEdge new_edge = trans_edge;
	new_edge.dep_region = compute_transitive_dep_region(imm_edge, trans_edge, store_offsets);
	new_edge.preserves_dep_region = imm_edge.preserves_dep_region && trans_edge.preserves_dep_region;

	if(empty(new_edge.dep_region))
		return emptyopt;

	return new_edge;
}

// Let INSTR be an instruction of kind `kind` and let INSTR[i] denote its ith operand. The function
// `instr_preserves_dep_regions` returns true iff INSTR would preserve the dependency region of a
// data dependency of INSTR[`op_idx`].
bool instr_preserves_dep_regions(InstrKind kind, int op_idx)
{
	switch(kind)
	{
		// TODO Casts that just reinterpret some memory would preserve dependency regions, but this
		//      is not true for, e.g., casting int<->float.
		//case InstrKind::cast:
			//assert(op_idx == 0);
			//return true;
		
		// Note that load instructions do not preserve the dependency region of their operand
		// because the result of a load is the value stored at the address denoted by the operand.
		// Thus, the operand only indirectly influences the result of the load.

		case InstrKind::store:
			assert(op_idx >= 0 && op_idx < 2);

			// Remember: the first operand of a store instruction denotes the destination address,
			// and the second denotes the value that we want to store at that address. Since a
			// dependency on a store refers to the value that is being stored, only those dependency
			// regions are preserved.
			return op_idx == 1;

		default:
			return false;
	};
}

bool create_transitive_dependencies(
	PDGNode const *node,
	ProgramDependenceGraph &pdg,
	StoreOffsetMap const &store_offsets)
{
	UniqueSortedVector<PDGEdge> new_deps;

	for(size_t op_idx = 0; op_idx < node->instr->operands().size(); ++op_idx)
	{
		Instr const *op = node->instr->operands()[op_idx];
		PDGNode const *parent = pdg.node(op);
		for(PDGEdge const &trans_edge: parent->fanins)
		{
			// We create a dummy edge for the data dependency between the instruction `node->instr`
			// and its operand `op` so we can use the create_transitive_edge() function.
			PDGEdge imm_edge{
				DependenceKind::data,
				node,
				nullptr,
				trans_edge.dep_region,
				instr_preserves_dep_regions(node->instr->kind(), op_idx)
			};

			if(optional<PDGEdge> new_edge = create_transitive_edge(imm_edge, trans_edge, store_offsets))
				new_deps.insert(*new_edge);
		}
	}

	for(PDGEdge const &imm_edge: node->fanins)
	{
		PDGNode const *parent = imm_edge.target;
		for(PDGEdge const &trans_edge: parent->fanins)
		{
			if(optional<PDGEdge> new_edge = create_transitive_edge(imm_edge, trans_edge, store_offsets))
				new_deps.insert(*new_edge);
		}
	}

	return pdg.add_deps(node, new_deps);
}

// 
UniqueSortedVector<std::pair<Decl*, ptrdiff_t>> get_definite_addresses(
	std::vector<PointsToMap::Entry> const &dests)
{
	UniqueSortedVector<std::pair<Decl*, ptrdiff_t>> offsets;
	for(PointsToMap::Entry const &entry: dests)
	{
		if(entry.target_locs.stride() == 0)
			offsets.insert({entry.target_obj, entry.target_locs.offset()});
	}

	keep_singletons(offsets, [](std::pair<Decl*, ptrdiff_t> a, std::pair<Decl*, ptrdiff_t> b)
	{
		return a.first == b.first;
	});

	return offsets;
}

void create_transitive_dependencies(ProgramDependenceGraph &pdg, PointsToMap const &pt)
{
	StoreOffsetMap store_offsets;
	for(auto const &pair: pdg.nodes())
	{
		if(pair.first->kind() == InstrKind::store)
		{
			StoreInstr const *store = static_cast<StoreInstr const*>(pair.first);
			std::vector<PointsToMap::Entry> dests = pt.get_addresses(store->op_at(0));
			for(std::pair<Decl*, ptrdiff_t> offset: get_definite_addresses(dests))
				store_offsets[{store, offset.first}] = offset.second;
		}
	}


	// TODO We can improve the efficiency of the algorithm if we sort the nodes based on the program
	//      order of their instruction.
	UniqueSortedVector<PDGNode const*> work_list;
	for(auto const &pair: pdg.nodes())
		work_list.insert(&pair.second);

	while(work_list.size())
	{
		PDGNode const *node = work_list.back();
		work_list.pop_back();

		if(create_transitive_dependencies(node, pdg, store_offsets))
		{
			for(PDGEdge const &edge: node->fanouts)
				work_list.insert(edge.target);

			for(Instr const *user: node->instr->users())
				work_list.insert(pdg.node(user));
		}
	}
}

}


//==================================================================================================
ProgramDependenceGraph create_pdg(
	Function const *func,
	ReachingDefinitions const &rd,
	PointsToMap const &pt)
{
	ProgramDependenceGraph pdg{func};

	// Add immediate (non-transitive) data dependencies.
	// This is exactly what ReachingDefinitions gives us.
	for(auto const &pair: rd)
	{
		LoadInstr const *load = pair.first;
		for(RDDependency const &dep: pair.second)
			pdg.add_data_dep(pdg.node(load), pdg.node(dep.defined_at), dep.variable, dep.dep_region, true);
	}

	// Compute immediate (non-transitive) control dependencies.
	// From "The Program Dependence Graph and Its Use in Optimization" by Ferrante, Ottenstein,
	// Warren, 1987:
	//
	// > Let G be a control flow graph. Let X and Y be nodes in G. Y is _control dependent_ on X iff
    // >
	// > 1. there exists a directed path P from X to Y with any Z in P (excluding X and Y)
	// >    post-dominated by Y and
    // > 2. X is not post-dominated by Y.
	//
	// The algorithm for marking immediate (i.e., non-transitive) control dependencies is as
	// follows:
	//
	//    For each basic block B such that there is an edge (A, B) in the CFG where basic block A
	//    terminates with a branch instruction
	//    1. mark all instructions in B as control dependent on the terminator of A
	//    2. set B to the immediate post-dominator of itself
	//    3. if B is equal to the immediate post-dominator of A, terminate, otherwise go to 1.
	DominatorTree post_dom_tree = compute_idoms(*func, PostDominatorFuncs{});
	for(auto const &bb: func->basic_blocks())
	{
		// Does `bb` terminates in a branch instruction, i.e., has multiple successors?
		if(bb->fanouts().size() > 1)
			create_immediate_control_dependencies(bb.get(), pdg, post_dom_tree);
	}

	// To compute transitive dependencies we need to make sure that every instruction has a node in
	// the dependence graph.
	for(auto const &bb: func->basic_blocks())
	{
		for(auto const &instr: bb->instructions())
			pdg.node(&instr);
	}

	// Now that we have immediate data and control dependencies we need to compute the transitive
	// closure.
	create_transitive_dependencies(pdg, pt);

	return pdg;
}


//==================================================================================================
void ProgramDependenceGraph::print(std::ostream &os, InstrNameMap &names) const
{
	for(auto const &bb: compute_post_order_of_reverse_cfg(*m_func))
	{
		os << std::setw(5) << std::left << (std::to_string(bb->id()) + ':') << '\n';
		for(auto &instr: bb->instructions())
		{
			PDGNode const *node = this->node(&instr);
			for(PDGEdge const &edge: node->fanins)
			{
				os << "    // " << cstr(edge.kind) << " dep: ";
				if(edge.kind == DependenceKind::data)
				{
					os << edge.variable->name() << "[" << edge.dep_region.first << ":" << edge.dep_region.last << "]";
					os << " defined by '"; edge.target->instr->print_inline(os, names);
					os << "', p: " << edge.preserves_dep_region << "\n";
				}
				else // edge.kind == DependenceKind::control
				{
					edge.target->instr->print_inline(os, names);
					os << "', p: " << edge.preserves_dep_region << "\n";
				}
			}

			os << "    "; instr.print_inline(os, names); os << "\n\n";
		}
	}
}

}
