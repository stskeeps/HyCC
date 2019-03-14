#include "boolean_size_optimized.h"
#include "common.h"

#include <libcircuit/utils.h>


std::pair<bvt, literalt> basic_adder(propt &prop, bvt const &a, bvt const &b, literalt carry_in)
{
  assert(a.size() == b.size());
  
  bvt sum(a.size());
  for (unsigned i = 0; i < a.size(); i++)
    std::tie(sum[i], carry_in) = full_adder(prop, a[i], b[i], carry_in);

  return {sum, carry_in};
}

std::pair<bvt, literalt> adder(propt &prop, bvt const &a, bvt const &b)
{
  return basic_adder(prop, a, b, const_literal(false));
}

std::pair<bvt, literalt> subtractor(propt &prop, bvt const &a, bvt const &b)
{
  return basic_adder(prop, a, inverter(b), const_literal(true));
}

bvt negator(propt &prop, bvt const &a)
{
  bv_utilst utils{prop};
  return utils.negate(a);
}


//------------------------------------------------------------------------------
bvt multiplier(propt &prop, bvt const &_op0, const bvt &_op1)
{
  bvt op0=_op0, op1=_op1;

  if(is_constant(op1))
    std::swap(op0, op1);

  bvt product;
  product.resize(op0.size());

  for(std::size_t i=0; i<product.size(); i++)
    product[i]=const_literal(false);

  for(std::size_t sum=0; sum<op0.size(); sum++)
  {
    if(op0[sum]!=const_literal(false))
    {
      bvt tmpop;

      tmpop.reserve(op0.size());

      for(std::size_t idx=0; idx<sum; idx++)
        tmpop.push_back(const_literal(false));

      for(std::size_t idx=sum; idx<op0.size(); idx++)
        tmpop.push_back(prop.land(op1[idx-sum], op0[sum]));

      product=adder(prop, product, tmpop).first;
    }
  }

  return product;
}


// Divider
//------------------------------------------------------------------------------
static bvt cond_negate(propt &prop, const bvt &bv, const literalt cond)
{
  bvt neg_bv=negator(prop, bv);

  bvt result;
  result.resize(bv.size());

  for(std::size_t i=0; i<bv.size(); i++)
    result[i]=prop.lselect(cond, neg_bv[i], bv[i]);

  return result;
}

std::pair<bvt, bvt> basic_unsigned_divider(propt &prop, const bvt &op0, const bvt &op1)
{
  assert(op0.size() == op1.size());

  bvt res, rem;
  
  int width=op0.size();
  res.resize(width);
  rem.resize(width);

  /* P := N
     D := D << n              -- P and D need twice the word width of N and Q
     for i = n−1..0 do        -- for example 31..0 for 32 bits
     P := 2P − D            -- trial subtraction from shifted value
     if P >= 0 then
     q(i) := 1            -- result-bit 1
     else
     q(i) := 0            -- result-bit 0
     P := P + D           -- new partial remainder is (restored) shifted value
     end
     end
  */


  for(int i=0; i<width; i++)
  {
    res[i]=const_literal(false);
    rem[i]=const_literal(false);
  }
  
  
  bvt bits_set(op1);
  for(int i = width - 2; i >= 0; i--) {
    bits_set[i]=prop.lor(bits_set[i+1], bits_set[i]);
  }
  
  for(int i = width - 1; i >= 0; i--) {
    int sub_bits = width - i; // Bitwidth for the subtract
    
    // Left shift of remainder
    if(i < width - 1) { // shift not needed in first round
      for(int j = (width)-1 ; j > 0; j--) {
        rem[j]=rem[j-1];
      }
    }
    rem[0] = op0[i];
    
    /*
      if R >= D then
        R := R − D
        Q(i) := 1
      end
     */
    
    // Resize, such that only necessary bits are used
    bvt remtemp(rem);
    bvt div(op1);
    remtemp.resize(sub_bits);
    div.resize(sub_bits);
    
    // Note that it seems like lselect and select_bv return a[0] on s=true and a[1] on s=false
    
    // overflow in unsigned subtract A-B indicates that B>A
    literalt carry_out;
    std::tie(remtemp, carry_out) = basic_adder(prop, remtemp, inverter(div), const_literal(true));
    // is overflow, or is divider larger than remainder
    if(i>0) {
      res[i] = !prop.lor(!carry_out, bits_set[sub_bits]);
      for(int j = 0; j < sub_bits; j++) {
        rem[j] = prop.lselect(res[i], remtemp[j], rem[j]);
      }
    } else {
      res[i] = carry_out;
      rem = select_bv(prop, res[i], remtemp, rem);
    }
    //rem = select_bv(res[i], remtemp, rem);
  }
  
  return {res, rem};
}

std::pair<bvt, bvt> unsigned_divider(propt &prop, const bvt &a, const bvt &b)
{
  return basic_unsigned_divider(prop, a, b);
}

std::pair<bvt, bvt> basic_signed_divider(propt &prop, CondNegateFunc cond_negate_func, const bvt &op0, const bvt &op1)
{
  if (op0.size() == 0 || op1.size() == 0)
    return {};

  literalt sign_op0 = op0[op0.size() - 1];
  literalt sign_op1 = op1[op1.size() - 1];

  bvt op0_inv = cond_negate_func(prop, op0, sign_op0);
  bvt op1_inv = cond_negate_func(prop, op1, sign_op1);

  bvt res, rem;
  std::tie(res, rem) = unsigned_divider(prop, op0_inv, op1_inv);

  literalt result_sign = prop.lxor(sign_op0, sign_op1);
  res = cond_negate_func(prop, res, result_sign);
  rem = cond_negate_func(prop, rem, sign_op0);

  return {res, rem};
}

std::pair<bvt, bvt> signed_divider(propt &prop, const bvt &op0, const bvt &op1)
{
  return basic_signed_divider(prop, cond_negate, op0, op1);
}


//------------------------------------------------------------------------------
bvt shifter(propt &prop, bvt const &in, bv_utilst::shiftt shift, const bvt &dist)
{
  bvt op = in;
  unsigned d = 1, width = op.size();

  for (unsigned stage = 0; stage < dist.size(); stage++)
  {
    if (d >= width)
      break;

    if (dist[stage] != const_literal(false))
    {
      bvt tmp(op);

      for (unsigned i = 0; i < width; i++)
      {
        literalt other;

        if (shift == bv_utilst::shiftt::LEFT)
          other = (d <= i ? tmp[i - d] : const_literal(false));
        else if (shift == bv_utilst::shiftt::ARIGHT)
          other = (i + d < width ? tmp[i + d] : tmp[width - 1]); // sign bit
        else if (shift == bv_utilst::shiftt::LRIGHT)
          other = (i + d < width ? tmp[i + d] : const_literal(false));

        op[i] = prop.lselect(dist[stage], other, tmp[i]);
      }
    }
    d = d << 1;
  }

  return op;
}


// Multiplexer
//------------------------------------------------------------------------------
// Creates a multiplexer for each input vector in `columns`.
//
// Circuit size/depth of a single multiplexer with n input bits:
// NonXORSize(n) = n - 1
// NonXORDepth(n) = log_2(n)
bvt multiplexer(propt &prop, bvt const &selector, std::vector<bvt> const &columns)
{
  assert(columns.size());

  bvt outputs;
  outputs.reserve(columns.size());

  for(size_t i = 0; i < columns.size(); ++i)
  {
    // Build a tree of 2-1 multiplexers
    auto inputs = columns[i];
    literalt selected = build_tree(inputs, [&](literalt a, literalt b, int level)
    {
      return prop.lselect(selector.at(level), b, a);
    });

    outputs.push_back(selected);
  }

  return outputs;
}


// Less than comparer
//------------------------------------------------------------------------------
literalt less_than(propt &prop, const bvt &bv0, const bvt &bv1, bool is_signed)
{
  return basic_less_than(subtractor, prop, bv0, bv1, is_signed);
}
