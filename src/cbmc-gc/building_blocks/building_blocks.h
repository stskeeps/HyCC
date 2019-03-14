#ifndef CBMC_GC_BUILDING_BLOCKS_H
#define CBMC_GC_BUILDING_BLOCKS_H

#include "boolean_size_optimized.h"
#include "boolean_depth_optimized.h"


struct building_blockst
{
  UnaryConverterFunc negator;
  AddSubConverterFunc adder;
  AddSubConverterFunc subtractor;
  // Since the width of the output is same as the width of the inputs signed/unsigned multiplication
  // is equivalent.
  BinaryConverterFunc multiplier;
  DivisionConverterFunc signed_divider;
  DivisionConverterFunc unsigned_divider;
  bvt (*shifter)(propt&, bvt const&, bv_utilst::shiftt, bvt const&);

  ComparisonConverterFunc equal;
  literalt (*less_than)(propt&, bvt const&, bvt const&, bool);

  bvt (*multiplexer)(propt&, bvt const&, std::vector<bvt> const&);
};


inline building_blockst get_default_building_blocks()
{
  building_blockst bb;
  bb.adder = adder;
  bb.negator = negator;
  bb.subtractor = subtractor;
  bb.multiplier = multiplier;
  bb.signed_divider = signed_divider;
  bb.unsigned_divider = unsigned_divider;
  bb.shifter = shifter;
  bb.multiplexer = multiplexer;
  bb.equal = equal_lowdepth;
  bb.less_than = less_than;

  return bb;
}

inline building_blockst get_lowdepth_building_blocks()
{
  building_blockst bb;
  bb.adder = adder_lowdepth;
  bb.negator = negator_lowdepth;
  bb.subtractor = subtractor_lowdepth;
  bb.multiplier = multiplier_lowdepth;
  bb.signed_divider = signed_divider_lowdepth;
  bb.unsigned_divider = unsigned_divider;
  bb.shifter = shifter_lowdepth;
  bb.multiplexer = multiplexer_lowdepth;
  bb.equal = equal_lowdepth;
  bb.less_than = less_than_lowdepth;

  return bb;
}


// Helpers
//------------------------------------------------------------------------------
inline bvt adder_subtractor(building_blockst const &conv, propt &prop, bvt const &a, bvt const &b, bool subtract)
{
  if(subtract)
    return conv.subtractor(prop, a, b).first;
  
  return conv.adder(prop, a, b).first;
}

inline literalt comparer(building_blockst const &bb, propt &prop, const bvt &bv0, irep_idt id, const bvt &bv1, bv_utilst::representationt rep)
{
  if(id==ID_equal)
    return bb.equal(prop, bv0, bv1);
  else if(id==ID_notequal)
    return !bb.equal(prop, bv0, bv1);
  else if(id==ID_le)
    return !bb.less_than(prop, bv1, bv0, rep == bv_utilst::representationt::SIGNED);
  else if(id==ID_lt)
    return bb.less_than(prop, bv0, bv1, rep == bv_utilst::representationt::SIGNED);
  else if(id==ID_ge)
    return !bb.less_than(prop, bv0, bv1, rep == bv_utilst::representationt::SIGNED);
  else if(id==ID_gt)
    return bb.less_than(prop, bv1, bv0, rep == bv_utilst::representationt::SIGNED);
  else
    assert(false);
}

#endif
