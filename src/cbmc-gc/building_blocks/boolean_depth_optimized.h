#ifndef CBMC_GC_BOOLEAN_DEPTH_OPTIMIZED_BUILDING_BLOCKS_H
#define CBMC_GC_BOOLEAN_DEPTH_OPTIMIZED_BUILDING_BLOCKS_H

#include "common.h"


std::pair<bvt, literalt> adder_lowdepth(propt &prop, bvt const &a, bvt const &b);
std::pair<bvt, literalt> subtractor_lowdepth(propt &prop, bvt const &a, bvt const &b);
bvt multiplier_lowdepth(propt &prop, bvt const &a, bvt const &b);
bvt negator_lowdepth(propt &prop, bvt const &a);
bvt multiplier_adder_lowdepth(propt &prop, std::vector<std::pair<bool, bvt>> &&summands, std::vector<std::pair<bool, std::vector<bvt>>> &&mults);
std::pair<bvt, bvt> signed_divider_lowdepth(propt &prop, bvt const &a, bvt const &b);
bvt multiplexer_lowdepth(propt &prop, const bvt &selector, std::vector<bvt> const &columns);
bvt shifter_lowdepth(propt &prop, bvt const &in, const bv_utilst::shiftt shift, const bvt &dist);
literalt equal_lowdepth(propt &prop, bvt const& bv0, const bvt& bv1);
literalt less_than_lowdepth(propt &prop, const bvt &bv0, const bvt &bv1, bool is_signed);

#endif
