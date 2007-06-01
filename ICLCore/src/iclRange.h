#ifndef RANGE_H
#define RANGE_H

#include "iclCore.h"

namespace icl{
  /// class representing a range defined by min and max value
  template<class Type> 
  struct Range{
    virtual ~Range(){}
    /// create an empty range (min = max = 0)
    Range():minVal(Type(0)),maxVal(Type(0)){}
    
    /// create a special Range
    Range(Type minVal, Type maxVal): minVal(minVal), maxVal(maxVal){}

    /// minimum value of this range
    Type minVal;
    
    /// maximum value of this range
    Type maxVal;
    
    /// return max-min
    Type getLength() const { return maxVal - minVal; } 
    
    /// casts this range into another depth
    template<class dstType>
    const Range<dstType> castTo() const{
      return Range<dstType>(Cast<Type,dstType>::cast(minVal),Cast<Type,dstType>::cast(maxVal));
    }
    
    /// tests whether a given value is inside of this range
    virtual bool in(Type value) const { return value >= minVal && value <= maxVal; }
  };
}
#endif
