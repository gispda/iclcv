#ifdef HAVE_XCF

#include <ICLIO/XCFUtils.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/StringUtils.h>

using memory::interface::Attachments;

using xmltio::TIODocument;
using xmltio::Location;
using xmltio::XPath;

namespace icl{

  

  const std::string &XCFUtils::createEmptyXMLDoc(){
    // {{{ open

    static const std::string s("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                               "<IMAGE uri=\"\">"
                               "<TIMESTAMPS>"
                               "<CREATED timestamp=\"\"/>"
                               "</TIMESTAMPS>"
                               "<PROPERTIES width=\"\" height=\"\" depth=\"\" channels=\"\" format=\"\"/>"
                               "<ROI offsetX=\"\" offsetY=\"\" width=\"\" height=\"\" />"
                               "</IMAGE>");
    return s;
  }

  // }}}
  
  xmltio::Location XCFUtils::createXMLDoc(const ImgBase* poImg, const std::string& uri, const std::string& bayerPattern){
    // {{{ open

    xmltio::Location l(createEmptyXMLDoc(), "/IMAGE");
    l["uri"] = uri;
    
    xmltio::Location p(l, "PROPERTIES");
    p["width"]	  = poImg->getWidth();
    p["height"]	  = poImg->getHeight();
    p["depth"]	  = str(poImg->getDepth());
    p["channels"] = poImg->getChannels();
    p["format"]	  = str(poImg->getFormat());
    if (bayerPattern != "") p["bayerPattern"] = bayerPattern;
    
    xmltio::Location r(l, "ROI");
    r["offsetX"] = poImg->getROIXOffset();
    r["offsetY"] = poImg->getROIYOffset();
    r["width"]	 = poImg->getROIWidth();
    r["height"]	 = poImg->getROIHeight();
    
    l[xmltio::XPath("TIMESTAMPS/CREATED/@timestamp")] = poImg->getTime().toMicroSeconds();
    return l;
  }

  // }}}

  void XCFUtils::getImage(memory::interface::MemoryPtr &mem, 
                          const std::string &xmlDoc,
                          ImgBase **dst, 
                          Attachments *reusableAttachment,
                          const std::string &xpath){
    // {{{ open

    Attachments *attToUse = reusableAttachment ? reusableAttachment : new Attachments;
    
    mem->getAttachments(xmlDoc, *attToUse);
    TIODocument doc(xmlDoc);
    Location docRootLoc = doc.getRootLocation();
    
    ImageDescription d = getImageDescription(docRootLoc[XPath(xpath)]);
    
    unserialize((*attToUse)[d.uri],d, dst);
    
    if(!reusableAttachment){
      delete attToUse;
    }
  }

  // }}}

  void XCFUtils::attachImage(memory::interface::MemoryPtr &mem, 
                             xmltio::Location &anchor,
                             const std::string &imageURI,
                             const ImgBase *image,
                             memory::interface::Attachments *reusableAttachment,
                             bool insertInsteadOfReplace){
    ICLASSERT_RETURN(mem);
    ICLASSERT_RETURN(image);
    
    Attachments *attToUse = reusableAttachment ? reusableAttachment : new Attachments;
    
    xmltio::Location imageLoc = XCFUtils::createXMLDoc(image,imageURI,"");
    XCFUtils::serialize(image,(*attToUse)[imageURI]);

    anchor.add(imageLoc);

    try{
       if(insertInsteadOfReplace){
#if 1 
          // to workaround the replace-bug of the C++ memory
          // we add the current id as dlgid
          // this is needed for the (old) ERBI robot setup
          anchor["dlgid"] = anchor.getDocument().getID();
#endif
          mem->insert(anchor.getDocumentText(), attToUse);
       }else{
         
#if 0 // replace igores attachments currently
          mem->replace(anchor.getDocumentText(), attToUse);
#else
          std::ostringstream oss;
          oss << "/*[@dbxml:id='" << anchor.getDocument().getID() << "']";
          mem->replaceByXPath(oss.str(), anchor.getDocumentText(), attToUse);
#endif
       }
    }catch(const std::exception &ex){
       ERROR_LOG("Caught std::exception: \"" << ex.what() << "\""); throw;
    }
    
    if(!reusableAttachment){
      delete attToUse;
    }
  }


  namespace{
    template <class XCF_T, typename ICE_T>
    static void CTUtoImage_Template (ImgBase* poImg, XCF::Binary::TransportUnitPtr btu) {
      // {{{ open

      XCF_T* pv = dynamic_cast<XCF_T*>(btu.get());
      if(!pv){
        throw ICLException(str(__FUNCTION__) + " unexpected type " + typeid(btu.get()).name() + 
                           "(Expected type " + typeid(XCF_T*).name());
      }
      const std::vector<ICE_T> &v = pv->value;

      unsigned int channeldim = poImg->getDim()*getSizeOf(poImg->getDepth());
      ICLASSERT_RETURN( v.size()*sizeof(ICE_T) == channeldim*poImg->getChannels() );
      

      const unsigned char *vData = reinterpret_cast<const unsigned char*>(v.data());

      for(int i=0;i<poImg->getChannels();++i){
        memcpy(poImg->getDataPtr(i),vData+i*channeldim, channeldim);
      }
    }

    // }}}

    template <class XCF_T, typename ICE_T>
    XCF::Binary::TransportUnitPtr ImageToCTU_Template(const ImgBase* poImg, XCF::Binary::TransportUnitPtr btu) {
      // {{{ open
      XCF_T *pTypedBTU = dynamic_cast<XCF_T*>(btu.get());
      // on type mismatch, create new instance
      if (!pTypedBTU) pTypedBTU = new XCF_T;
      // get reference to data content
      std::vector<ICE_T> &vecImage = pTypedBTU->value;
      vecImage.resize (poImg->getChannels() * poImg->getDim());
      // copy data
      int imgSize = poImg->getDim();
      int imgByteSize = imgSize * getSizeOf(poImg->getDepth());
      for (int i=0, nChannels=poImg->getChannels(); i < nChannels; i++) {
        memcpy(&vecImage[i*imgSize], poImg->getDataPtr(i), imgByteSize);
      }
      return pTypedBTU;
    }

    // }}}
  }

  XCFUtils::ImageDescription XCFUtils::getImageDescription(const xmltio::Location &l){
    ImageDescription d;
    d.uri = xmltio::extract<std::string>(l["uri"]);
    
    xmltio::Location  p(l, "PROPERTIES");
    d.size.width = xmltio::extract<int>(p["width"]);
    d.size.height  = xmltio::extract<int>(p["height"]);
    d.imagedepth = parse<depth>(xmltio::extract<std::string>(p["depth"]));
    d.channels   = xmltio::extract<int>(p["channels"]);
    try{
      d.imageformat = parse<format>(xmltio::extract<std::string>(p["format"]));
    }catch(const InvalidFormatException &ex){
      d.imageformat = formatMatrix;
    }
    
    xmltio::LocationPtr  r = xmltio::find (l, "ROI");
    if (r) {
       d.roi = Rect((int) xmltio::extract<int>((*r)["offsetX"]), 
                    (int) xmltio::extract<int>((*r)["offsetY"]),
                    (int) xmltio::extract<int>((*r)["width"]),
                    (int) xmltio::extract<int>((*r)["height"]));
    }
    d.time = Time::microSeconds(xmltio::extract<Time::value_type>(l[xmltio::XPath("TIMESTAMPS/CREATED/@timestamp")]));

    return d;
  }
  
  void XCFUtils::createOutputImage(const xmltio::Location& l, 
                                   ImgBase *poSrc, 
                                   ImgBase *poOutput, 
                                   ImgBase **poBayerBuffer, 
                                   BayerConverter *poBC, 
                                   Converter *poConv) {
    xmltio::LocationPtr locBayer = xmltio::find (l, "PROPERTIES/@bayerPattern");
    if (locBayer) {
      const std::string& bayerPattern =  xmltio::extract<std::string>(*locBayer);
      ImgParams p = poSrc->getParams();
      p.setFormat (formatRGB);
      *poBayerBuffer = icl::ensureCompatible (poBayerBuffer, poSrc->getDepth(), p);
         
      poBC->setBayerImgSize(poSrc->getSize());
      //poBC->setConverterMethod(BayerConverter::nearestNeighbor);
      poBC->setBayerPattern(BayerConverter::translateBayerPattern(bayerPattern));
      
      poBC->apply(poSrc->asImg<icl8u>(), poBayerBuffer);
      poConv->apply (*poBayerBuffer, poOutput);
    } else {
      poConv->apply (poSrc, poOutput);
    }
  }

  void XCFUtils::ImageDescription::show(){
    std::cout << "URI:" << uri << " Size:" << size << " Depth:" << imagedepth << 
    " Channels:" << channels << " Format:" << imageformat << " ROI:" << roi << "Time:" <<
    time.toMicroSeconds() << std::endl;
  }
      
  void XCFUtils::CTUtoImage (const XCF::CTUPtr ctu, const xmltio::Location& l, ImgBase** ppoImg){
    // {{{ open
    ImageDescription d = getImageDescription(l);

    if(d.imageformat != formatMatrix && d.channels != getChannelsOfFormat(d.imageformat)){
      bool first = true;
      if(first){
        first = false;
        ERROR_LOG("format " << str(d.imageformat) 
                  << " and channel count " 
                  << d.channels << " are incompatible\n"
                  "using minimal channel count");
      }
      d.channels = iclMin(d.channels,getChannelsOfFormat(d.imageformat));
      if(d.channels != getChannelsOfFormat(d.imageformat)){
        d.imageformat = formatMatrix;
      }
    }

    *ppoImg = ensureCompatible (ppoImg, d.imagedepth, d.size,d.channels,d.imageformat,d.roi);
    
    (*ppoImg)->setTime (d.time);

    XCF::Binary::TransportUnitPtr btu = ctu->getBinary (d.uri);

    switch ((*ppoImg)->getDepth()) {
      case depth8u:
        CTUtoImage_Template<XCF::Binary::TransportVecByte, Ice::Byte> (*ppoImg, btu);
        break;
      case depth32s:
        CTUtoImage_Template<XCF::Binary::TransportVecInt, int> (*ppoImg, btu);
        break;
      case depth32f:
        CTUtoImage_Template<XCF::Binary::TransportVecFloat, float> (*ppoImg, btu);
        break;
      case depth64f:
        CTUtoImage_Template<XCF::Binary::TransportVecDouble, double> (*ppoImg, btu);
        break;
      default:
        return;
        break;
    }
  }

  // }}}
  

  XCF::Binary::TransportUnitPtr XCFUtils::ImageToCTU(const ImgBase* poImg, XCF::Binary::TransportUnitPtr btu){
    // {{{ open

    switch (poImg->getDepth()){
      case depth8u:
        return ImageToCTU_Template<XCF::Binary::TransportVecByte, Ice::Byte> (poImg, btu);
      case depth32s:
        return ImageToCTU_Template<XCF::Binary::TransportVecInt, int> (poImg, btu);
      case depth32f:
        return ImageToCTU_Template<XCF::Binary::TransportVecFloat, float> (poImg, btu);
      case depth64f:
        return ImageToCTU_Template<XCF::Binary::TransportVecDouble, double> (poImg, btu);
      default:
        return 0;
        break;
    }
  }

  // }}}

  void XCFUtils::serialize(const ImgBase *image, std::vector<unsigned char> &dst){
    // {{{ open

    
    ICLASSERT_RETURN(image);
    unsigned int channeldim = image->getDim()*icl::getSizeOf(image->getDepth());
    dst.resize(channeldim*image->getChannels());
    
    for(int i=0;i<image->getChannels();++i){
      memcpy(dst.data()+i*channeldim,image->getDataPtr(i),channeldim);
    }
    
  }

  // }}}

  void XCFUtils::unserialize(const std::vector<unsigned char> &src, const XCFUtils::ImageDescription &d, ImgBase **dst){
    // {{{ open

    unsigned int channeldim = d.size.width*d.size.height*icl::getSizeOf(d.imagedepth);
    if(src.size() != channeldim*d.channels){
      if(src.size() < channeldim*d.channels){
        ERROR_LOG("Unable to unserialize, dimension mismatch: src-vector.size="<<src.size()<<" channeldim*d.channels="<<channeldim*d.channels);
        return;
      }else{
        ERROR_LOG("WARNING dimension mismatch: src-vector.size="<<src.size()<<" channeldim*d.channels="<<channeldim*d.channels);
      }
    }

    *dst = ensureCompatible (dst, d.imagedepth, d.size,d.channels,d.imageformat,d.roi);
    ImgBase *image = *dst;
    image->setTime(d.time);
    
    for(int i=0;i<d.channels;++i){
      memcpy(image->getDataPtr(i),src.data()+i*channeldim,channeldim);
    }
  }

  // }}}

}


#endif // HAVE_XCF