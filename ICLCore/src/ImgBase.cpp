#include <ImgBase.h>
#include <Img.h>

using namespace std;

namespace icl {
  
  // {{{ constructor / destructor 
  
  ImgBase::ImgBase(depth d, const ImgParams &params):m_oParams(params),m_eDepth(d) {
    // {{{ open

    FUNCTION_LOG("ImgBase(" << getWidth()
                 << "," << getHeight()
                 << "," << translateFormat(getFormat()) 
                 << ", "<< translateDepth(getDepth()) 
                 << "," << getChannels() << ")  this:" << this); 
  }

  // }}}
  
  ImgBase::~ImgBase(){
    // {{{ open
    FUNCTION_LOG("");
  }

  // }}}
  
  // }}} 
  
  // {{{ utility functions

  void ImgBase::print(const string sTitle) const{
    // {{{ open

  FUNCTION_LOG(sTitle);
  printf(   " -----------------------------------------\n"
            "| image: %s\n"
            "| timestamp: %s\n"
            "| width: %d, height: %d, channels: %d\n",sTitle.c_str(), this->getTime().toString().c_str(),
            getSize().width,getSize().height,getChannels());
  printf(   "| depth: %s  format: %s\n",translateDepth(getDepth()).c_str(), translateFormat(getFormat()).c_str());
  printf(   "| ROI: x: %d, y: %d, w: %d, h: %d \n", getROI().x, getROI().y,getROI().width, getROI().height);

  switch (m_eDepth){
    case depth8u:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl8u>()->getMin(i),asImg<icl8u>()->getMax(i));
      }
      break;
    case depth16s:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl16s>()->getMin(i),asImg<icl16s>()->getMax(i));
      }
      break;
    case depth32s:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %d, max:%d \n",i,asImg<icl32s>()->getMin(i),asImg<icl32s>()->getMax(i));
      }
      break;
    case depth32f:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl32f>()->getMin(i),asImg<icl32f>()->getMax(i));
      }
      break;
    case depth64f:
      for(int i=0;i<getChannels();i++){
        printf("| channel: %d, min: %f, max:%f \n",i,asImg<icl64f>()->getMin(i),asImg<icl64f>()->getMax(i));
      }
      break;
    default: ICL_INVALID_DEPTH; break;
  }
  
  printf(" -----------------------------------------\n");
 

}

  // }}}

  // }}}
  
  // {{{ convert(depth)
      
  ImgBase *ImgBase::convert(depth d) const{
    FUNCTION_LOG("");
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convert((Img<icl##D>*)0); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}
  
  // {{{ convert(ImgBase *poDst)

  ImgBase *ImgBase::convert(ImgBase *poDst) const{
    FUNCTION_LOG("");
    if(!poDst) return deepCopy();
    switch(poDst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convert(poDst->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_FORMAT; break;
    }
  }

  // }}}
  
  // {{{ convert(Img<otherT>*)
  template<class otherT>
  Img<otherT> *ImgBase::convert(Img<otherT> *poDst) const{ 
    FUNCTION_LOG("ptr:"<<poDst);
    if(!poDst) poDst = new Img<otherT>(getParams());
    else poDst->setParams(getParams());
    
#define ICL_INSTANTIATE_DEPTH(D)                                            \
      case depth##D: for(int c=getChannels()-1;c>=0;--c){                   \
      icl::convert<icl##D,otherT>(asImg<icl##D>()->getData(c),              \
      asImg<icl##D>()->getData(c)+getDim(),poDst->getData(c));    \
      } break;
    switch(getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS;
    }
#undef ICL_INSTANTIATE_DEPTH
    return poDst;    
  }
  // }}}

  // {{{ convertROI(depth)
      
  ImgBase *ImgBase::convertROI(depth d) const{
    FUNCTION_LOG("");
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convertROI((Img<icl##D>*)0); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  // {{{ convertROI(ImgBase *)
      
  ImgBase *ImgBase::convertROI(ImgBase *poDst) const{
    FUNCTION_LOG("");
    if(!poDst) return deepCopy();
    switch(poDst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convertROI(poDst->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}

  // {{{ convertROI(Img<otherT>*)

  template<class otherT>
  Img<otherT> *ImgBase::convertROI(Img<otherT> *poDst) const{ 
    FUNCTION_LOG("ptr:"<<poDst);
    ImgParams p(getROISize(),getChannels(),getFormat());
    if(!poDst) poDst = new Img<otherT>(p);
    else poDst->setParams(p);
    
#define ICL_INSTANTIATE_DEPTH(D)                                         \
  case depth##D:                                                         \
    for(int c=getChannels()-1;c>=0;--c){                                 \
      convertChannelROI(asImg<icl##D>(),c,getROIOffset(),getROISize(),   \
                        poDst,c,Point::null,getROISize());               \
    }                                                                    \
    break;
    
    switch(getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS;
    }
#undef ICL_INSTANTIATE_DEPTH
    return poDst;    
  }
  // }}}

  // {{{ convertROIToROI(Img<otherT> *)

 template<class otherT>
 Img<otherT> *ImgBase::convertROIToROI(Img<otherT> *poDst) const{ 
   FUNCTION_LOG("ptr:"<<poDst);
   if(!poDst) return convertROI<otherT>();
   ICLASSERT( poDst->getROISize() == getROISize() );
   ICLASSERT( poDst->getChannels() == getChannels() );
    
#define ICL_INSTANTIATE_DEPTH(D)                                            \
  case depth##D:                                                            \
    for(int c=getChannels()-1;c>=0;--c){                                    \
      convertChannelROI(asImg<icl##D>(),c,getROIOffset(),getROISize(),      \
                        poDst,c,poDst->getROIOffset(),poDst->getROISize()); \
    }                                                                       \
    break;
    
    switch(getDepth()){
      ICL_INSTANTIATE_ALL_DEPTHS;
    }
#undef ICL_INSTANTIATE_DEPTH
    return poDst;    
  }
  // }}}

  // {{{ convertROIToROI(ImgBase *)
      
  ImgBase *ImgBase::convertROIToROI(ImgBase *poDst) const{
    FUNCTION_LOG("");
    if(!poDst) return deepCopyROI();
    switch(poDst->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return this->convertROIToROI(poDst->asImg<icl##D>()); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_FORMAT; break;
    }
  }
  // }}}  

  // {{{ setFormat
  void ImgBase::setFormat(format fmt){
    FUNCTION_LOG("");
    int newcc = getChannelsOfFormat(fmt);
    if(fmt != formatMatrix && newcc != getChannels()){
      setChannels(newcc);
    }
    m_oParams.setFormat(fmt);
  }

  // }}}

  // {{{ clear
  void ImgBase::clear(int iChannel, icl64f val, bool bROIOnly){
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)       \
      case depth##D:                   \
      asImg<icl##D>()->clear(iChannel, \
      Cast<icl64f,icl##D>::cast(val),  \
      bROIOnly); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
  }
  
  // }}}
    
  // {{{ normalizeXXX
 
  void ImgBase::normalizeAllChannels(const Range<icl64f> &dstRange){
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                              \
      case depth##D: asImg<icl##D>()->normalizeAllChannels(   \
      dstRange.castTo<icl##D>()); break;                    
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  
  void ImgBase::normalizeChannel(int iChannel,const Range<icl64f> &srcRange,const Range<icl64f> &dstRange){
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                 \
      case depth##D: asImg<icl##D>()->normalizeChannel(iChannel, \
      srcRange.castTo<icl##D>(),                                 \
      dstRange.castTo<icl##D>()); break;                       
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  
  void ImgBase::normalizeChannel(int iChannel,const Range<icl64f> &dstRange){
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                 \
      case depth##D: asImg<icl##D>()->normalizeChannel(iChannel, \
        dstRange.castTo<icl##D>()); break;                     
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  void ImgBase::normalizeImg(const Range<icl64f> &srcRange,const Range<icl64f> &dstRange){
    FUNCTION_LOG("");
     switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                    \
      case depth##D: asImg<icl##D>()->normalizeImg( \
      srcRange.castTo<icl##D>(),                    \
      dstRange.castTo<icl##D>()); break;          
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  
  void ImgBase::normalizeImg(const Range<icl64f> &dstRange){
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                     \
      case depth##D: asImg<icl##D>()->normalizeImg(  \
      dstRange.castTo<icl##D>()); break;           
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  
  // }}}
  
  // {{{ getMin,getMax, getMinMax

  icl64f ImgBase::getMax(int iChannel) const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMax(iChannel);
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
    return 0;
  }
  
  /// Returns min pixel value of channel iChannel within ROI
  /** @param iChannel Index of channel
  **/
  icl64f ImgBase::getMin(int iChannel) const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMin(iChannel);
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
    return 0;
  }


  /// return maximal pixel value over all channels (restricted to ROI)
  icl64f ImgBase::getMin() const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMin();
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
    return 0;
  }

  /// return minimal pixel value over all channels (restricted to ROI)
  icl64f ImgBase::getMax() const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMax();
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
    return 0;
  }

  /// Returns min and max pixel values of channel iChannel within ROI
  /** @param rtMin reference to store the min value 
      @param rtMax reference to store the max value
      @param iChannel Index of channel
  **/
  const Range<icl64f> ImgBase::getMinMax(int iChannel) const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMinMax(iChannel).castTo<icl64f>();
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
    return Range<icl64f>();
  }

  /// return minimal and maximal pixel values over all channels (restricted to ROI)
  const Range<icl64f> ImgBase::getMinMax() const{
    FUNCTION_LOG("");
    switch(getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: return asImg<icl##D>()->getMinMax().castTo<icl64f>();
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH   
    }
    return Range<icl64f>();
  }

  // }}}
  
  // {{{ setParams

  void ImgBase::setParams(const ImgParams &params){
    FUNCTION_LOG("");
    setChannels(params.getChannels());
    setSize(params.getSize());
    setFormat(params.getFormat());
    setROI(params.getROI());
  }
  
  // }}}


} //namespace icl
