/*
  Skin.h

  Written by: Michael G�tting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef Skin_H
#define Skin_H

#include "Filter.h"

namespace icl {

/// This class implements a Skin color detection algorithm
/**
\section class
The skin class provides access to a skin color segmentation algorithm.
Therefor the class provide a training procedure and a skin detection algorithm.
In a first step the skin parabola parameter have to be trained or if the parameter set is allready known the can be set directly. After this the detection algorithm is well prepared for detection the skin color.
author Michael G�tting (mgoettin@techfak.uni-bielefeld.de)
**/

class Skin : public Filter
{
 public:
  Skin () : m_poChromaApply(0), m_poChromaTrain(0) {}
  Skin (std::vector<float> skinParams) : m_poChromaApply(0), 
    m_poChromaTrain(0),
    m_vecSkinParams(skinParams) {}
  ~Skin () {};
  
  ///Start the detection of skin color in the given image.
  /**
     @param poSrc The src image
     @param poDst The final skin color mask (binarized)
  **/
  void apply(icl::ImgI *poSrc, icl::ImgI **ppoDst);

  ///Start the training procedure for the skin parabla parameter
  /**
     param The tarining image. Usually a small skin color patttern
  **/
  void train(icl::ImgI *poSrc);

  ///Set the skin parabola parameter directly 
  /**
     @params The skin color parabola parameter
  **/
  void setSkinParams(std::vector<float> params) {
    m_vecSkinParams = params;}
  
  ///Get the current skin parabola parameter
  /**
     @retrun A vector with the skin parabola parameter
  **/
  std::vector<float> getSkinParams() { return m_vecSkinParams; }
  
 private:
  ImgI *m_poChromaApply;
  ImgI *m_poChromaTrain;
  std::vector<float> m_vecSkinParams;
  
}; //class Skin

} //namespace icl

#endif //Skin_H
