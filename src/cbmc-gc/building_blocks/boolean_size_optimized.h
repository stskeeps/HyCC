#ifndef CBMC_GC_BOOLEAN_SIZE_OPTIMIZED_BUILDING_BLOCKS_H
#define CBMC_GC_BOOLEAN_SIZE_OPTIMIZED_BUILDING_BLOCKS_H

#include "common.h"


std::pair<bvt, literalt> adder(propt &prop, bvt const &a, bvt const &b);
std::pair<bvt, literalt> subtractor(propt &prop, bvt const &a, bvt const &b);
bvt multiplier(propt &prop, bvt const &a, bvt const &b);
bvt negator(propt &prop, bvt const &a);
std::pair<bvt, bvt> signed_divider(propt &prop, bvt const &a, bvt const &b);
std::pair<bvt, bvt> unsigned_divider(propt &prop, bvt const &a, bvt const &b);
bvt shifter(propt &prop, bvt const &in, bv_utilst::shiftt shift, const bvt &dist);
bvt multiplexer(propt &prop, bvt const &selector_terms, std::vector<bvt> const &columns);
literalt less_than(propt &prop, const bvt &bv0, const bvt &bv1, bool is_signed);

std::pair<bvt, bvt> basic_signed_divider(propt &prop, CondNegateFunc cond_negate_func, const bvt &op0, const bvt &op1);

#endif
