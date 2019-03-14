#ifndef CBMC_GC_BUILDING_BLOCKS_COMMON_H
#define CBMC_GC_BUILDING_BLOCKS_COMMON_H

#include <solvers/prop/prop.h>
#include <solvers/flattening/bv_utils.h>


using AddSubConverterFunc = std::pair<bvt, literalt> (*)(propt&, bvt const&, bvt const&);
using UnaryConverterFunc = bvt (*)(propt&, bvt const&);
using BinaryConverterFunc = bvt (*)(propt&, bvt const&, bvt const&);
using ComparisonConverterFunc = literalt (*)(propt&, bvt const&, bvt const&);
using DivisionConverterFunc = std::pair<bvt, bvt> (*)(propt&, bvt const&, bvt const&);
using CondNegateFunc = bvt (*)(propt&, const bvt&, literalt);


bool is_constant(const bvt &bv);
bvt inverter(bvt const &bv);
std::pair<literalt, literalt> half_adder(propt &prop, literalt a, literalt b);
std::pair<literalt, literalt> full_adder(propt &prop, literalt a, literalt b, literalt c);
literalt basic_less_than(AddSubConverterFunc subtractor_func, propt &prop, const bvt &bv0, const bvt &bv1, bool is_signed);

// selects a vector out of two by the use of a condition literal
bvt select_bv(propt &prop, const literalt cond, const bvt &op1, const bvt &op2);


#endif
