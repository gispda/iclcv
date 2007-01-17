#ifndef CANNY_H
#define CANNY_H

#include <Filter.h>
#include <Img.h>
#include <Array.h>

namespace icl {
  
/// Class for the canny edge detector (IPP only!)
/**  
     This subsection describes a classic edge detector proposed by J.Canny. The
     detector uses a grayscale image as an input and outputs a black-and-white image, where non-zero
     pixels mark detected edges. The algorithm consists of three stages described below.
  
     Stage 1: Differentiation
     Assuming two-dimensional convolution, the image data are differentiated with respect to the
     directions x and y. The gradient of the surface of the convoluted image function in any direction is
     possible to compute from the known gradient in any two directions.
     From the computed x and y gradient values, the magnitude and angle of the slope can be calculated
     from the hypotenuse and arctangent.
  
     Note: Stage 1 is done by two sobel filters. The implementation can handle the output of the two sobel filters, 
     or can handle a normal image, applying the sobel filters by itself.
  
     Stage 2: Non-Maximum Suppression
     With the rate of intensity change found at each point in the image, edges must now be placed at the
     points of maximum, or rather non-maximum must be suppressed. A local maximum occurs at a
     peak in the gradient function, or alternatively where the derivative of the gradient function is set to
     zero. However, in this case it is preferable to suppress non-maximum perpendicular to the edge
     direction, rather than parallel to the edge direction, since the edge strength is expected to continue
     along an extended contour.
     The algorithm starts off by reducing the angle of gradient to one of 4 sectors shown in Figure 14-1.
     The algorithm passes 3�3 neighborhood across the magnitude array. At each point, the center
     element of neighborhood is compared with its two neighbors along line of the gradient given by
     the sector value.
     If the central value is non-maximum, i.e., not greater than the neighbors, it is suppressed.


  
     Stage 3: Edge Thresholding
  
     streaking by setting an upper and lower edge value limit. Considering a line segment, if a value lies
     above the upper threshold limit it is immediately accepted. If the value lies below the low
     threshold it is immediately rejected. Points which lie between the two limits are accepted if they
     are connected to pixels which exhibit strong response. The likelihood of streaking is reduced
     drastically since the line segment points must fluctuate above the upper limit and below the lower
     limit for streaking to occur.
     J.Canny recommends the ratio of high to low limit be in the range two or three to one, based on
     predicted signal-to-noise ratios.

*/
   class Canny : public Filter {
   public:
      Canny(const Img32f *src);
      virtual ~Canny();
      /// applys Canny algorithm for edge detection, using 2 src images containing the x- and y-derivation
      /** 
          @param poSrcDx    source image, x-derivation
          @param poSrcDy    source image, y-derivation
          @param ppoDst   pointer to destination image
          @param lowThresh    lower threshold for edges detection
          @param highThresh   upper threshold for edges detection
      */
      void apply (const ImgBase *poSrcDx, const ImgBase *poSrcDy, ImgBase **ppoDst, icl32f lowThresh, icl32f highThresh);
      /// applys Canny algorithm for edge detection
      /**
         @param poSrc    source image
         @param ppoDst   pointer to destination image
         @param lowThresh    lower threshold for edges detection
         @param highThresh   upper threshold for edges detection
      */
      void apply (const ImgBase *poSrc, ImgBase **ppoDst, icl32f lowThresh, icl32f highThresh);
      void apply (const Img32f *srcDx, const Img32f *srcDy, Img8u *dst, icl32f lowThresh, icl32f highThresh);

   private:
      /// buffer for ippiCanny
      icl8u* m_oBuffer8u;
      ImgBase *m_poSobelx; //sobel-x => y-derivation
      ImgBase *m_poSobely; //sobel-y => x-derivation
   };
} // namespace icl
#endif


