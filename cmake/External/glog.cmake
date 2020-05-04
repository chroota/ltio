include("cmake/External/gflags.cmake")

if (NOT __GLOG_INCLUDED)
  set(__GLOG_INCLUDED TRUE)

  # try the system-wide glog first
  find_package(Glog)
  if (GLOG_FOUND)
      set(GLOG_EXTERNAL FALSE)
  else()
    # fetch and build glog from github

    # build directory
    set(glog_PREFIX ${CMAKE_BINARY_DIR}/external/glog-prefix)
    # install directory
    set(glog_INSTALL ${CMAKE_BINARY_DIR}/external/glog-install)

    # we build glog statically, but want to link it into the caffe shared library
    # this requires position-independent code
    if (UNIX)
      set(GLOG_EXTRA_COMPILER_FLAGS "-fPIC")
    endif()

    set(GLOG_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GLOG_EXTRA_COMPILER_FLAGS})
    set(GLOG_C_FLAGS ${CMAKE_C_FLAGS} ${GLOG_EXTRA_COMPILER_FLAGS})

    # depend on gflags if we're also building it
    if (GFLAGS_EXTERNAL)
      set(GLOG_DEPENDS gflags)
    endif()

    ExternalProject_Add(glog
      DEPENDS ${GLOG_DEPENDS}
      PREFIX ${glog_PREFIX}
      GIT_REPOSITORY "https://gitee.com/ltecho/glog.git"
      GIT_TAG "v0.3.5"
      UPDATE_COMMAND ""
      INSTALL_DIR ${gflags_INSTALL}
      PATCH_COMMAND autoreconf -i ${glog_PREFIX}/src/glog
      CONFIGURE_COMMAND env "CFLAGS=${GLOG_C_FLAGS}" "CXXFLAGS=${GLOG_CXX_FLAGS}"
        ${glog_PREFIX}/src/glog/configure
        --prefix=${glog_INSTALL}
        --enable-shared=yes --enable-static=yes
        --with-gflags=${GFLAGS_LIBRARY_DIRS}/..
      LOG_INSTALL 1
      LOG_DOWNLOAD 1
      LOG_CONFIGURE 1
      )

    set(GLOG_FOUND TRUE)
    set(GLOG_INCLUDE_DIRS ${glog_INSTALL}/include)
    set(GLOG_LIBRARY ${glog_INSTALL}/lib/libglog.so)
    set(GLOG_LIBRARIES ${GFLAGS_LIBRARIES} ${glog_INSTALL}/lib/libglog.so)
    set(GLOG_LIBRARY_DIRS ${glog_INSTALL}/lib)
    set(GLOG_EXTERNAL TRUE)

    list(APPEND external_project_dependencies glog)
  endif()
endif()

add_library(glog::glog SHARED IMPORTED GLOBAL)
set_property(TARGET glog::glog
    PROPERTY
    IMPORTED_LOCATION ${GLOG_LIBRARIES}
    )

#
#GLOG_LIBRARIESif(GLOG_FOUND AND NOT TARGET glog::glog)
#  add_library(glog::glog SHARED IMPORTED)
#  set_target_properties(glog::glog PROPERTIES IMPORTED_LOCATION ${GLOG_LIBRARY})
#  set_property(TARGET glog::glog PROPERTY INTERFACE_LINK_LIBRARIES ${GFLAGS_LIBRARIES})
#  set_property(TARGET glog::glog PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GLOG_INCLUDE_DIRS})
#endif()

