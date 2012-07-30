/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PointCloudCreator.cpp                      **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLGeom/PointCloudCreator.h>
#include <ICLCore/Img.h>

namespace icl{
  struct PointCloudCreator::Data{
    typedef FixedColVector<float,3> Vec3;
    
    SmartPtr<Mat>rgbdMapping;
    SmartPtr<Camera> depthCamera, colorCamera; // memorized for easy copying
    Size imageSize;
    Vec viewRayOffset;
    Array2D<Vec3> viewRayDirections;
    PointCloudCreator::DepthImageMode mode;    // memorized for easy copying

    static inline float compute_depth_norm(const Vec &dir, const Vec &centerDir){
      return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
    }

    void init(Camera *depthCam, Camera *colorCam, PointCloudCreator::DepthImageMode mode){
      this->mode = mode;
      depthCamera = depthCam;
      colorCamera = colorCam;
      imageSize = depthCam->getRenderParams().chipSize;
      
      if(colorCam){
        this->rgbdMapping = new Mat(colorCam->getProjectionMatrix()*colorCam->getCSTransformationMatrix());
      }
      
      Array2D<ViewRay> viewRays = depthCam->getAllViewRays();
      viewRayOffset = viewRays(0,0).offset;
      viewRayDirections = Array2D<Vec3>(imageSize);
      const Vec centerViewRayDir = viewRays(imageSize.width/2-1, imageSize.height/2-1).direction;
    
      for(int y=0;y<imageSize.height;++y){
        for(int x=0;x<imageSize.width;++x){
          const int idx = x + imageSize.width * y;
          const Vec &d = viewRays[idx].direction;
          if(mode == PointCloudCreator::DistanceToCamPlane){
            const float corr = 1.0/compute_depth_norm(d,centerViewRayDir);
            viewRayDirections[idx] = Vec3(d[0]*corr, d[1]*corr, d[2]*corr);
          }else{
            viewRayDirections[idx] = Vec3(d[0],d[1],d[2]);
          }
        }
      }
    }
    
  };
  
  PointCloudCreator::PointCloudCreator():m_data(new Data){
  }

  PointCloudCreator::PointCloudCreator(const Camera &depthCam, PointCloudCreator::DepthImageMode mode):m_data(new Data){
    m_data->init(new Camera(depthCam),0,mode);
  }
  
  PointCloudCreator::PointCloudCreator(const Camera &depthCam, const Camera &colorCam, PointCloudCreator::DepthImageMode mode){
    m_data->init(new Camera(depthCam), new Camera(colorCam),mode);
  }
  
  void PointCloudCreator::init(const Camera &depthCam,  PointCloudCreator::DepthImageMode mode){
    delete m_data;
    m_data = new Data;
    m_data->init(new Camera(depthCam),0,mode);
  }
  
  void PointCloudCreator::init(const Camera &depthCam, const Camera &colorCam,  PointCloudCreator::DepthImageMode mode){
    delete m_data;
    m_data = new Data;
    m_data->init(new Camera(depthCam), new Camera(colorCam), mode);
  }
  
  PointCloudCreator::PointCloudCreator(const PointCloudCreator &other):m_data(new Data){
    *this = other;
  }
  
  PointCloudCreator &PointCloudCreator::operator=(const PointCloudCreator &other){
    delete m_data;
    m_data = new Data;
    if(other.m_data->colorCamera){
      m_data->init(new Camera(*other.m_data->depthCamera), new Camera(*other.m_data->colorCamera), other.m_data->mode);
    }else{
      m_data->init(new Camera(*other.m_data->depthCamera), 0, other.m_data->mode);
    }
    return *this;
  }
  


  inline Point map_rgbd(const Mat &M, const Vec3 &v){
    const float phInv = 1.0/ (M(0,3) * v[0] + M(1,3) * v[1] + M(2,3) * v[2] + M(3,3));
    const int px = phInv * ( M(0,0) * v[0] + M(1,0) * v[1] + M(2,0) * v[2] + M(3,0) );
    const int py = phInv * ( M(0,1) * v[0] + M(1,1) * v[1] + M(2,1) * v[2] + M(3,1) );
    return Point(px,py);
  }


  template<class RGBAType>
  inline void assign_rgba(RGBAType &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
  }

  /// specialization for floats: scale range to [0,255]
  template<>
  inline void assign_rgba(Vec &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
    rgba[0] = r * 0.0039216f; // 1./255
    rgba[1] = g * 0.0039216f;
    rgba[2] = b * 0.0039216f;
    rgba[3] = a * 0.0039216f;
  }

  /// specialization for 3D rgb: no alpha
  template<>
  inline void assign_rgba(FixedColVector<icl8u,3> &rgb, icl8u r, icl8u g, icl8u b, icl8u a){
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
    (void)a;
  }

  /// specialization for icl32s: reinterpert as FixedColVector<icl8u,4>
  template<>
  inline void assign_rgba(icl32s &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
    //    assign_rgba(*reinterpret_cast<FixedColVector<icl8u,4>*>(&rgba), r,g,b,a);
    assign_rgba(reinterpret_cast<FixedColVector<icl8u,4>&>(rgba), r,g,b,a);
  }




  template<bool HAVE_RGBD_MAPPING, class RGBA_DATA_SEGMENT_TYPE>
  static void point_loop(const icl32f *depthValues, const Mat M, 
                         const Vec O, const unsigned int W, const unsigned int H, const int DIM, 
                         DataSegment<float,3> xyz, 
                         RGBA_DATA_SEGMENT_TYPE rgba,
                         const Channel8u rgb[3],
                         const Array2D<Vec3> &dirs){
    for(int i=0;i<DIM;++i){
      const Vec3 &dir = dirs[i];
      const float d = depthValues[i];
      
      FixedColVector<float,3> &dstXYZ = xyz[i];
      
      dstXYZ[0] = O[0] + d * dir[0]; // avoid 3-float temporary 
      dstXYZ[1] = O[1] + d * dir[1];
      dstXYZ[2] = O[2] + d * dir[2];
      
      if(HAVE_RGBD_MAPPING){ // optimized as template parameter
        Point p = map_rgbd(M,dstXYZ);
        if( ((unsigned int)p.x) < W && ((unsigned int)p.y) < H){ 
          const int idx = p.x + W * p.y;
          assign_rgba(rgba[i], rgb[0][idx], rgb[1][idx], rgb[2][idx], 255);
        }else{
          assign_rgba(rgba[i], 0,0,0,0);
        }
      }
    }
  }

  void PointCloudCreator::create(const Img32f &depthImageMM, PointCloudObjectBase &destination, const Img8u *rgbImage){
    if(depthImageMM.getSize() != m_data->imageSize){
      throw ICLException("PointCloudCreator::create: depthImage's size is not equal to the camera size");
    }
    if(!destination.supports(PointCloudObjectBase::XYZ)){
      throw ICLException("PointCloudCreator:create: destination point cloud object does not support XYZ data");
    }

    DataSegment<float,3> xyz = destination.selectXYZ();

    if(depthImageMM.getSize() != xyz.getSize()){
      if(xyz.getSize() == Size::null){
        throw ICLException("PointCloudCreator::create: given point cloud's size is not 2D-ordered");
      }else{
        throw ICLException("PointCloudCreator::create: depthImage's size is not equal to the point-cloud size");
      }
    }
    if(rgbImage && !m_data->rgbdMapping){
      throw ICLException("PointCloudCreator::create rgbImage to map was given, but no color camera calibration data is available");
    } 
 
    /// precaching variables ...
    const icl32f *dv = depthImageMM.begin(0);
    const Array2D<Data::Vec3> &dirs = m_data->viewRayDirections;
    const bool X = m_data->rgbdMapping;
    const Mat M = m_data->rgbdMapping ? *m_data->rgbdMapping : Mat(0.0f);
    const Vec O = m_data->viewRayOffset;
    const int W = m_data->imageSize.width;
    const int H = m_data->imageSize.height;
    const int DIM = W*H;
    

    const Channel8u rgb[3];
    for(int i=0;rgbImage && i<3;++i) rgb[i] = (*rgbImage)[i];

    Time t = Time::now();
    
    if(destination.supports(PointCloudObjectBase::RGBA32f)){
      if(X) point_loop<true>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs);
      else point_loop<false>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs);
    }else if(destination.supports(PointCloudObjectBase::BGRA)){
      if(X) point_loop<true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs);
      else point_loop<false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs);
    }else if(destination.supports(PointCloudObjectBase::BGR)){
      if(X) point_loop<true>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs);
      else point_loop<false>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs);
    }else if(destination.supports(PointCloudObjectBase::BGRA32s)){
      if(X) point_loop<true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs);
      else point_loop<false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs);
    }else{
      throw ICLException("unable to apply RGBD-Mapping given destination PointCloud type does not support rgb information");
    }
    
    SHOW(t.age().toMilliSecondsDouble());
  }

  const Camera &PointCloudCreator::getDepthCamera() const{
    return *m_data->depthCamera;
  }
    
  const Camera &PointCloudCreator::getColorCamera() const throw (ICLException){
    if(!hasColorCamera()) throw ICLException("PointCloudCreator::getColorCamera(): no color camera available");
    return *m_data->colorCamera;
  }
    
  bool PointCloudCreator::hasColorCamera() const{
    return m_data->colorCamera;
  }

}

