#pragma once

#include <vector>
#include <unordered_map>

#include "reaching_definitions.h"
#include <libcircuit/sorted_vector.h>


// Basic approach follows "Field-Sensitive Program Dependence Analysis" by Litvak et. al.


namespace ir {


//==================================================================================================
struct PDGNode;

enum class DependenceKind
{
	data,
	control,
};

inline char const* cstr(DependenceKind kind)
{
	switch(kind)
	{
		case DependenceKind::data: return "data";
		case DependenceKind::control: return "control";
	}
}


// An edge represents a data or control dependence between to nodes in the program dependence graph.
struct PDGEdge
{
	DependenceKind kind;
	
	// The target node of this edge. The source node is not explicitly stored here: whichever
	// PDGNode stores this edge is the source node.
	PDGNode const *target;

	// The following fields are only applicably if `kind == DependenceKind::data`. In this case,
	// this edge specifies a dependency on the byte-range `dep_region` of variable `variable` that
	// is defined by the StoreInstr in `target`.
	//
	// Let LHS denote the value computed by the instruction of `target`. Further, let LHS[rgn]
	// denote those bytes of LHS that are in the region rgn. If `preserves_dep_region`
	// is true, then only LHS[`dep_region`] depends on `variable`, i.e., all other parts of LHS that
	// are not covered by `dep_region` are independent of `variable`. Doing this increases the
	// precision of the dependence analysis, but has no influence on its soundness.
	// 
	// For example, consider the statement `LHS = RHS`, where, e.g., RHS[0:3] depends on A and
	// RHS[4:7] depends on B. After the assignment, we get that LHS[0:3] depends on A and LHS[4:7]
	// depends on B. Thus, assignments preserve dependency regions. On the other hand, if we have
	// `LHS = X * Y`, where, e.g., X[0:3] depends on A and X[4:7] depends on B, then *all* of LHS
	// may depend on both A and B. Thus, multiplication does not preserve dependency regions.
	//
	// At the moment, we assume that only store instructions preserve dependency regions.
	// Reinterpret-style casts would also qualify.
	Decl *variable;
	Region dep_region;
	bool preserves_dep_region;
};


inline bool is_data(PDGEdge const &edge)
{
	return edge.kind == DependenceKind::data;
}

inline bool operator < (PDGEdge const &a, PDGEdge const &b)
{
	return std::tie(a.kind, a.target, a.variable, a.dep_region, a.preserves_dep_region) <
		std::tie(b.kind, b.target, b.variable, b.dep_region, b.preserves_dep_region);
}

inline bool operator == (PDGEdge const &a, PDGEdge const &b)
{
	return a.kind == b.kind && a.target == b.target && a.variable == b.variable &&
		a.dep_region == b.dep_region && a.preserves_dep_region == b.preserves_dep_region;
}

inline PDGEdge data_edge(PDGNode const *target, Decl *variable, Region dep_region, bool preserves_dep_region)
{
	return PDGEdge{DependenceKind::data, target, variable, dep_region, preserves_dep_region};
}

inline PDGEdge control_edge(PDGNode const *target)
{
	return PDGEdge{DependenceKind::control, target, nullptr, {}, false};
}


//==================================================================================================
class Function;
class Instr;

template<typename T>
using UniqueSortedVector = sorted_vector<T, true>;

struct PDGNode
{
	PDGNode() :
		instr{nullptr} {}

	explicit PDGNode(Instr const *instr) :
		instr{instr} {}

	Instr const *instr;

	UniqueSortedVector<PDGEdge> fanins;
	UniqueSortedVector<PDGEdge> fanouts;
};


//==================================================================================================
class ProgramDependenceGraph
{
public:
	explicit ProgramDependenceGraph(Function const *func) :
		m_func{func} {}

	PDGNode const* node(Instr const *instr)
	{
		auto res = m_nodes.insert({instr, PDGNode{instr}});
		return &res.first->second;
	}

	PDGNode const* node(Instr const *instr) const
	{
		return &m_nodes.at(instr);
	}

	void add_data_dep(PDGNode const *node, PDGNode const *dep, Decl *variable, Region dep_region, bool preserves_dep_region)
	{
		// We assume that both `node` and `dep` were obtained via a call to `node()`, so the
		// const-casts should be okay.
		const_cast<PDGNode*>(node)->fanins.insert(data_edge(dep, variable, dep_region, preserves_dep_region));
		const_cast<PDGNode*>(dep)->fanouts.insert(data_edge(node, variable, dep_region, preserves_dep_region));
	}

	void add_control_dep(PDGNode const *node, PDGNode const *dep)
	{
		const_cast<PDGNode*>(node)->fanins.insert(control_edge(dep));
		const_cast<PDGNode*>(dep)->fanouts.insert(control_edge(node));
	}

	bool add_deps(PDGNode const *node, UniqueSortedVector<PDGEdge> const &new_deps)
	{
		size_t old_fanin_count = node->fanins.size();
		const_cast<PDGNode*>(node)->fanins.insert(new_deps.begin(), new_deps.end());
		return old_fanin_count != node->fanins.size();
	}

	std::unordered_map<Instr const*, PDGNode> const& nodes() const { return m_nodes; }

	void print(std::ostream &os, InstrNameMap &names) const;

private:
	std::unordered_map<Instr const*, PDGNode> m_nodes;
	Function const *m_func;
};


ProgramDependenceGraph create_pdg(
	Function const *func,
	ReachingDefinitions const &rd,
	PointsToMap const &pt);

}
