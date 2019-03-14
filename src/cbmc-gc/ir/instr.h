#pragma once

#include "intrusive_list.h"
#include "symbol_table.h"

#include <langapi/language_util.h>
#include <util/c_types.h>

#include <libcircuit/utils.h>

#include <unordered_set>


namespace ir {

//==================================================================================================
enum class InstrKind
{
	constant,
	output,

	// Arithmetic operations
	add,
	sub,
	mul,
	div,

	// Bitwise operations
	b_and,
	b_or,
	b_not,
	lshr, // Logical shift right

	// Comparison operations
	lt,
	gt,
	eq,

	call,

	// Takes the address of a symbol
	named_addr,
	compute_addr, // Like LLVM's getelementptr
	// Gets the value pointed to by an address.
	load,
	store,

	// Combine multiple values into one
	combine,

	cast,

	jump,
	branch,
	phi,

	nondet,
};

char const* cstr(InstrKind kind);

inline bool is_jump(InstrKind kind)
{
	return kind == InstrKind::jump || kind == InstrKind::branch;
}


//==================================================================================================
class BasicBlock;
class InstrNameMap;

class Instr : public IntrusiveListItem<Instr>
{
	friend BasicBlock;
	using ListItem = IntrusiveListItem<Instr>;

public:
	using Iterator = ListItem::Iterator;
	using ConstIterator = ListItem::ConstIterator;

	using UserRange = IteratorRange<std::unordered_set<Instr*>::const_iterator>;
	using OpRange = IteratorRange<std::vector<Instr*>::const_iterator>;


	Instr(InstrKind kind, typet const &type, BasicBlock *block = nullptr);
	virtual ~Instr() {}

	// Since Instrs keep pointers to each other copying/moving is not allowed
	Instr(Instr const&) = delete;
	Instr(Instr &&) = delete;

	InstrKind kind() const { return m_kind; }

	typet const& type() const { return m_type; }
	UserRange users() const { return UserRange(m_users.begin(), m_users.end()); }

	void add_operand(Instr *val)
	{
		val->m_users.insert(this);
		m_operands.push_back(val);
	}

	void set_operand(Instr *val, size_t i)
	{
		assert(i < m_operands.size());
		val->m_users.insert(this);
		m_operands[i] = val;
	}

	void set_operands(OpRange ops)
	{
		clear_operands();
		for(Instr *n: ops)
			add_operand(n);
	}

	void clear_operands()
	{
		for(Instr *n: m_operands)
			n->m_users.erase(this);

		m_operands.clear();
	}

	Instr* op_at(size_t i) const { return m_operands.at(i); }
	OpRange operands() const { return {m_operands.begin(), m_operands.end()}; }

	BasicBlock* block() { return m_block; }
	BasicBlock const* block() const { return m_block; }

	bool is_well_formed(struct ValidationContext &ctx) const;

	virtual void print_ref(std::ostream &os, InstrNameMap &names) const;
	virtual void print_inline(std::ostream &os, InstrNameMap &names) const;
	void dump(InstrNameMap &names) const;

	Iterator global_iterator() { return ListItem::iterator(); }
	ConstIterator global_const_iterator() const { return ListItem::const_iterator(); }

	void set_name_hint(std::string const &name) { m_name_hint = name; }
	std::string const& name_hint() const { return m_name_hint; }

private:
	void set_block(BasicBlock *block)
	{
		assert(!m_block && "Value already belongs to a block");
		m_block = block;
	}

private:
	typet m_type;
	InstrKind m_kind;
	BasicBlock *m_block;
	std::unordered_set<Instr*> m_users;
	std::vector<Instr*> m_operands;
	std::string m_name_hint;
};


//==================================================================================================
class Constant : public Instr
{
	friend class Function;

public:
	mp_integer const& value() const { return m_value; }

	virtual void print_ref(std::ostream &os, InstrNameMap &names) const override;
	virtual void print_inline(std::ostream &os, InstrNameMap &names) const override;

private:
	Constant(mp_integer const &value, typet const &type) :
		Instr{InstrKind::constant, type},
		m_value{value} {}

private:
	mp_integer m_value;
};


inline typet const& get_return_type_from_pointer(typet const &type)
{
	return to_code_type(to_pointer_type(type).subtype()).return_type();
}

class CallInstr : public Instr
{
public:
	explicit CallInstr(Instr *func_addr, BasicBlock *bb = nullptr) :
		Instr{InstrKind::call, get_return_type_from_pointer(func_addr->type()), bb}
	{
		add_operand(func_addr);
	}

	Instr* func_addr() const { return op_at(0); }
	OpRange args() const { return {++operands().b, operands().e}; };

	code_typet const& func_type() const { return to_code_type(to_pointer_type(func_addr()->type()).subtype()); }
};


class NamedAddrInstr : public Instr
{
public:
	explicit NamedAddrInstr(Decl *decl, BasicBlock *bb = nullptr) :
		Instr{InstrKind::named_addr, pointer_type(decl->type()), bb},
		m_decl{decl}
	{
		set_name_hint(m_decl->name());
	}

	Decl* decl() const { return m_decl; }

	virtual void print_inline(std::ostream &os, InstrNameMap &names) const;

private:
	Decl *m_decl;
};

class ComputeAddrInstr : public Instr
{
public:
	explicit ComputeAddrInstr(typet const &type, Instr *addr, BasicBlock *bb = nullptr) :
		Instr{InstrKind::compute_addr, type, bb}
	{
		add_operand(addr);
	}
};


class LoadInstr : public Instr
{
public:
	explicit LoadInstr(Instr *op, BasicBlock *bb = nullptr) :
		Instr{InstrKind::load, to_pointer_type(op->type()).subtype(), bb}
	{
		add_operand(op);
	}
};


class StoreInstr : public Instr
{
public:
	StoreInstr(Instr *dest, Instr *from, BasicBlock *bb = nullptr) :
		Instr{InstrKind::store, typet{}, bb}
	{
		add_operand(dest);
		add_operand(from);
	}
};


class JumpInstr : public Instr
{
public:
	explicit JumpInstr(BasicBlock *bb = nullptr) :
		Instr{InstrKind::jump, typet{}, bb} {}
};

class BranchInstr : public Instr
{
public:
	explicit BranchInstr(Instr *condition, BasicBlock *bb = nullptr) :
		Instr{InstrKind::branch, typet{}, bb}
	{
		add_operand(condition);
	}
};

bool type_check(Instr const *instr, ValidationContext &ctx);

}
