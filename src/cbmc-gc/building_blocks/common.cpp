#include "common.h"


bool is_constant(const bvt &bv)
{
  forall_literals(it, bv)
    if(!it->is_constant())
      return false;

  return true;
}

bvt inverter(bvt const &bv)
{
  bvt inv; inv.reserve(bv.size());
  for(auto &l: bv)
    inv.push_back(!l);
  
  return inv;
}

std::pair<literalt, literalt> half_adder(propt &prop, literalt a, literalt b)
{
  return {prop.lxor(a, b), prop.land(a, b)};
}

std::pair<literalt, literalt> full_adder(propt &prop, literalt a, literalt b, literalt c)
{
  literalt xor1_out = prop.lxor(a, c);
  literalt xor2_out = prop.lxor(b, c);

  literalt and_out = prop.land(xor1_out, xor2_out);

  literalt sum = prop.lxor(a, xor2_out);
  literalt carry_out = prop.lxor(and_out, c);

  return {sum, carry_out};
}

literalt basic_less_than(AddSubConverterFunc subtractor_func, propt &prop, const bvt &bv0, const bvt &bv1, bool is_signed)
{
  bvt diff;
  literalt carry_out;
  std::tie(diff, carry_out) = subtractor_func(prop, bv0, bv1);

  literalt result;
  if(is_signed)
  {
    literalt top0 = bv0[bv0.size()-1];
    literalt top1 = bv1[bv1.size()-1];
    result = prop.lxor(prop.lequal(top0, top1), carry_out);
  }
  else
    result = !carry_out;

  return result;
}

// selects a vector out of two by the use of a condition literal
bvt select_bv(propt &prop, const literalt cond, const bvt &op1, const bvt &op2)
{
  assert(op1.size() == op2.size());

  bvt bv;
  bv.resize(op1.size());

  for (unsigned i = 0; i < bv.size(); i++)
    bv[i] = prop.lselect(cond, op1[i], op2[i]);

  return bv;
}

