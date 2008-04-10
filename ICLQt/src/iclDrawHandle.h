#ifndef ICL_DRAW_HANDLE
#define ICL_DRAW_HANDLE

#include <iclGUIHandle.h>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class DrawHandle : public GUIHandle<ICLDrawWidget>{
    public:
    /// create a new ImageHandel
    DrawHandle(ICLDrawWidget *w=0):GUIHandle<ICLDrawWidget>(w){}
    
    /// make the wrapped ICLWidget show a given image
    void setImage(const ImgBase *image);
    
    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase *image) { setImage(image); }

    /// make the wrapped ICLWidget show a given image (as set Image)
    void operator=(const ImgBase &image) { setImage(&image); }
    
    /// calles updated internally
    void update();
  };
  
}

#endif
