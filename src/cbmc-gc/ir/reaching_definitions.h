#pragma once

#include <unordered_map>
#include <vector>
#include <ostream>
#include <limits>


// Follows "Field-Sensitive Program Dependence Analysis" by Litvak et. al.


class boolbv_widtht;

namespace ir {

class Decl;
class LoadInstr;
class StoreInstr;
class Function;
class InstrNameMap;
class PointsToMap;


struct Region
{
	constexpr Region() :
		first{0},
		last{0} {}

	constexpr Region(ptrdiff_t first, ptrdiff_t last) :
		first{first},
		last{last} {}

	ptrdiff_t first;
	ptrdiff_t last;
};

constexpr Region MaxRegion{
	std::numeric_limits<ptrdiff_t>::min(),
	std::numeric_limits<ptrdiff_t>::max()
};

inline Region operator + (Region const &r, ptrdiff_t offset)
{
	return Region{r.first + offset, r.last + offset};
}

inline bool operator < (Region const &a, Region const &b)
{
	return std::tie(a.first, a.last) < std::tie(b.first, b.last);
}

inline bool operator == (Region const &a, Region const &b)
{
	return a.first == b.first && a.last == b.last;
}

inline bool overlap(Region const &a, Region const &b)
{
	return (a.first >= b.first && a.first <= b.last) || (b.first >= a.first && b.first <= a.last);
}

inline bool contains(Region const &parent, Region const &child)
{
	return parent.first <= child.first && parent.last >= child.last;
}

inline Region intersection(Region const &a, Region const &b)
{
	Region res{std::max(a.first, b.first), std::min(a.last, b.last)};
	if(res.first > res.last)
		res.first = res.last = 0;

	return res;
}

inline bool empty(Region const &r)
{
	return r.first > r.last;
}

inline std::ostream& operator << (std::ostream &os, Region const &r)
{
	return os << '[' << r.first << ':' << r.last << ']';
}


// Represents a dependency on (some part of) a variable definition (= assignment) .
struct RDDependency
{
	// The variable that we depend on.
	Decl *variable;

	// The instruction that defines the variable.
	StoreInstr const *defined_at;

	// The intersection between the region defined by `defined_at` and the region of `variable` that
	// we depend on.
	Region dep_region;
};

inline bool operator < (RDDependency const &a, RDDependency const &b)
{
	return std::tie(a.variable, a.defined_at, a.dep_region) <
		std::tie(b.variable, b.defined_at, b.dep_region);
}

inline bool operator == (RDDependency const &a, RDDependency const &b)
{
	return std::tie(a.variable, a.defined_at, a.dep_region) ==
		std::tie(a.variable, b.defined_at, b.dep_region);
}

void print(RDDependency const &def, InstrNameMap &names, std::ostream &os);


// Stores for each LoadInstr the definitions it depends on
using ReachingDefinitions = std::unordered_map<LoadInstr const*, std::vector<RDDependency>>;

ReachingDefinitions reaching_definitions(
	Function const *func,
	PointsToMap const &pt,
	boolbv_widtht const &boolbv_width);

}
