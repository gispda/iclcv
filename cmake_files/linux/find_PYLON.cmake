if(ICL_64BIT)
  icl_check_external_package(PYLON "pylon/PylonIncludes.h;pylon/TransportLayer.h" "pylonbase;pylonutility;pylongigesupp" lib64 include HAVE_PYLON_COND TRUE)
else() 
  icl_check_external_package(PYLON "pylon/PylonIncludes.h" "pylonbase;pylonutility;pylongigesupp" lib include HAVE_PYLON_COND TRUE)
endif()
