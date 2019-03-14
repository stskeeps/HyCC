#include "reaching_definitions.h"

#include "function.h"
#include "pointer_analysis.h"


namespace ir {

void print(RDDependency const &dep, InstrNameMap &names, std::ostream &os)
{
	os << dep.variable->name() << "[" << dep.dep_region.first << ":" << dep.dep_region.last << "] (defined by '";
	dep.defined_at->print_inline(os, names);
	os << "')";
}

namespace
{
	// Merges the elements of of the sorted sequence `src` into into the sorted sequence `dest`.
	// Elements that already exist in `dest` are ignored. Returns true iff at least one element has
	// been added to `dest`.
	template<typename Range>
	bool merge_into(Range &dest, Range const &src)
	{
		bool changed = false;

		auto dest_it = dest.begin();
		auto src_it = src.begin();
		while(dest_it != dest.end() && src_it != src.end())
		{
			if(*src_it < *dest_it)
			{
				dest_it = dest.insert(dest_it, *src_it);
				changed = true;
				++src_it;
				++dest_it;
			}
			else if(*src_it == *dest_it)
			{
				++src_it;
				++dest_it;
			}
			else
				++dest_it;
		}

		if(src_it != src.end())
			changed = true;
		
		dest.insert(dest.end(), src_it, src.end());

		return changed;
	}


	struct Definition
	{
		// The variable that is being defined.
		Decl *variable;
		// The region of the variable that is being defined.
		Region region;
		// The instruction that is defining the variable.
		StoreInstr const *defined_at;
	};

	bool operator < (Definition const &a, Definition const &b)
	{
		return std::tie(a.variable, a.defined_at, a.region) < std::tie(b.variable, b.defined_at, b.region);
	}

	bool operator == (Definition const &a, Definition const &b)
	{
		return std::tie(a.variable, a.defined_at, a.region) == std::tie(b.variable, b.defined_at, b.region);
	}


	enum class DefMode
	{
		kill,
		no_kill,
	};


	// During the analysis, an RDBlockState stores the state for a single basic block.
	class RDBlockState
	{
	public:
		// A RDBlockState only modifies the entries of those LoadInstrs that occur in its BasicBlock
		explicit RDBlockState(std::unordered_map<LoadInstr const*, std::vector<RDDependency>> *rd) :
			m_load_deps{rd} {}

		bool add_definition(Definition const &def, DefMode kind)
		{
			if(kind == DefMode::kill)
				return add_killing_definition(def);

			return merge_into(m_defs[def.variable], {def});
		}

		// `deps` must be sorted
		bool add_load_deps(LoadInstr const *load, std::vector<RDDependency> const &deps)
		{
			return merge_into((*m_load_deps)[load], deps);
		}

		std::vector<Definition> const& get_defs(Decl *decl) const
		{
			static std::vector<Definition> empty;

			auto it = m_defs.find(decl);
			if(it != m_defs.end())
				return it->second;

			return empty;
		}

		bool merge(RDBlockState const &other)
		{
			bool changed = false;
			for(auto const &pair: other.m_defs)
				changed |= merge_into(m_defs[pair.first], pair.second);

			return changed;
		}

		void print_defs(std::ostream &os, InstrNameMap &names) const
		{
			for(auto const &pair: m_defs)
			{
				for(Definition const &def: pair.second)
				{
					os << pair.first->name() << "[" << def.region.first << ":" << def.region.last << "] defined by '";
					def.defined_at->print_inline(os, names);
					os << "'\n";
				}
			}
		}

		std::unordered_map<Decl*, std::vector<Definition>> const& store_defs() const { return m_defs; }

	private:
		std::unordered_map<Decl*, std::vector<Definition>> m_defs;
		std::unordered_map<LoadInstr const*, std::vector<RDDependency>> *m_load_deps;

		bool add_killing_definition(Definition const &killing_def)
		{
			Region killing_region = killing_def.region;
			std::vector<Definition> &defs = m_defs[killing_def.variable];

			std::vector<Definition> new_defs = {killing_def};
			bool changed = false;
			auto it = defs.begin();
			while(it != defs.end())
			{
				Definition &cur_def = *it;
				assert(cur_def.variable == killing_def.variable);

				if(contains(killing_region, cur_def.region))
				{
					it = defs.erase(it);
					changed = true;
					continue;
				}

				if(contains(cur_def.region, killing_region))
				{
					new_defs.push_back(
						Definition{cur_def.variable, Region{cur_def.region.first, killing_region.first - 1}, cur_def.defined_at}
					);
					cur_def.region.first = killing_region.last + 1;
					changed = true;
				}
				else if(overlap(killing_region, cur_def.region))
				{
					if(cur_def.region.first < killing_region.first)
						cur_def.region.last = killing_region.first - 1;
					else
					{
						assert(cur_def.region.last > killing_region.last);
						cur_def.region.first = killing_region.last + 1;
					}

					changed = true;
				}

				++it;
			}

			std::sort(new_defs.begin(), new_defs.end());
			changed |= merge_into(defs, new_defs);

			remove_empty_defs(defs);

			return changed;
		}

		void remove_empty_defs(std::vector<Definition> &defs)
		{
			auto new_end = std::remove_if(defs.begin(), defs.end(), [](Definition const &def)
			{
				return empty(def.region);
			});
			defs.erase(new_end, defs.end());
		}
	};


	struct RDContext
	{
		PointsToMap const &pt;
		boolbv_widtht const &boolbv_width;
	};

	Region loc_set_to_region(LinearLocationSet locs, ptrdiff_t element_width, ptrdiff_t total_width)
	{
		if(locs.stride() == 0)
			return Region{locs.offset(), locs.offset() + element_width - 1};

		// TODO We need a more precise way to handle array accesses both in the reaching definition
		//      analysis and the pointer analysis. Currently, the pointer analysis assumes that an
		//      indexed write to an array may touch anything in a surrounding struct, which is only
		//      required if we want to support C programs that rely on undefined behavior.

		// Conservatively assume the whole object is affected
		return Region{0, total_width};
	}

	std::pair<Definition, DefMode> create_definition(
		PointsToMap::Entry const &dest,
		StoreInstr const *store_instr,
		RDContext &ctx)
	{
		Decl *dest_object = dest.target_obj;
		LinearLocationSet dest_locs = dest.target_locs;

		ptrdiff_t store_width = ctx.boolbv_width(store_instr->op_at(1)->type()) / config.ansi_c.char_width;
		ptrdiff_t total_width = ctx.boolbv_width(dest_object->type()) / config.ansi_c.char_width;
		Region reg = loc_set_to_region(dest_locs, store_width, total_width);
		if(dest_locs.stride() == 0)
			return {Definition{dest_object, reg, store_instr}, DefMode::kill};
		else
		{
			// If `dest_locs` has a stride that the computed region is an
			// over-approximation (see implementation of `loc_set_to_region()`), thus we
			// must not kill previous writes to this region.
			return {Definition{dest_object, reg, store_instr}, DefMode::no_kill};
		}
	}

	bool reaching_definitions(RDBlockState &state, BasicBlock const *block, RDContext &ctx)
	{
		bool changed = false;
		for(Instr const &instr: block->instructions())
		{
			if(instr.kind() == InstrKind::store)
			{
				StoreInstr const *store = static_cast<StoreInstr const*>(&instr);
				std::vector<PointsToMap::Entry> dests = ctx.pt.get_addresses(instr.op_at(0));
				// If the store target points to a single object then we can kill previous writes to
				// the same region. Otherwise, we need to keep old writes (i.e., no killing)
				if(dests.size() == 1)
				{
					// Even if we know exactly which object we are writing to, we still should avoid
					// killing previous writes *if* the region we are defining is an
					// over-approximation.
					std::pair<Definition, DefMode> def = create_definition(dests[0], store, ctx);
					changed |= state.add_definition(def.first, def.second);
				}
				else
				{
					for(auto const &dest: dests)
						changed |= state.add_definition(create_definition(dest, store, ctx).first, DefMode::no_kill);
				}
			}
			else if(instr.kind() == InstrKind::load)
			{
				LoadInstr const *load = static_cast<LoadInstr const*>(&instr);
				std::vector<PointsToMap::Entry> srcs = ctx.pt.get_addresses(instr.op_at(0));
				ptrdiff_t load_width = ctx.boolbv_width(instr.type()) / config.ansi_c.char_width;
				std::vector<RDDependency> deps;
				for(auto const &src: srcs)
				{
					Decl *src_object = src.target_obj;
					LinearLocationSet src_locs = src.target_locs;
					ptrdiff_t total_width = ctx.boolbv_width(src_object->type()) / config.ansi_c.char_width;
					Region reg = loc_set_to_region(src_locs, load_width, total_width);

					for(Definition const &def: state.get_defs(src_object))
					{
						if(overlap(reg, def.region))
						{
							deps.push_back(RDDependency{
								src_object,
								def.defined_at,
								intersection(reg, def.region)
							});
						}
					}
				}

				std::sort(deps.begin(), deps.end());
				changed |= state.add_load_deps(load, deps);
			}
		}

		return changed;
	}
}

ReachingDefinitions reaching_definitions(
	Function const *func,
	PointsToMap const &pt,
	boolbv_widtht const &boolbv_width)
{
	ReachingDefinitions rd;
	auto num_bbs = func->basic_blocks().size();
	std::vector<RDBlockState> bb_states(num_bbs, RDBlockState{&rd});

	RDContext ctx{pt, boolbv_width};
	BBWorkList work_list = create_work_list_rpo(func);
	for(auto const &bb: func->basic_blocks())
		work_list.insert(bb.get());

	while(work_list.size())
	{
		BasicBlock const *bb = *work_list.begin(); work_list.erase(work_list.begin());
		RDBlockState &state = bb_states[bb->id()];
		bool changed = false;

		for(BlockEdge const &pred: bb->fanins())
			changed |= state.merge(bb_states[pred.target()->id()]);

		changed |= reaching_definitions(state, bb, ctx);
		if(changed)
		{
			for(BlockEdge const &succ: bb->fanouts())
				work_list.insert(succ.target());
		}
	}

	return rd;
}

}
