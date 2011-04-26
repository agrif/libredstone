# From the Virtual Terrain Project
# <http://code.google.com/p/vtp/>

# - Find zlib
# Find the native ZLIB includes and library
#
#  ZLIB_INCLUDE_DIR - where to find zlib.h, etc.
#  ZLIB_LIBRARIES   - List of libraries when using zlib.
#  ZLIB_FOUND       - True if zlib found.


IF (ZLIB_INCLUDE_DIR)
  # Already in cache, be silent
  SET(ZLIB_FIND_QUIETLY TRUE)
ENDIF (ZLIB_INCLUDE_DIR)

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h)

SET(ZLIB_NAMES z zlib zdll zlib1)
FIND_LIBRARY(ZLIB_LIBRARY NAMES ${ZLIB_NAMES} )

#SET(ZLIB_NAMES_DEBUG zlib1d)
#FIND_LIBRARY(ZLIB_LIBRARY_DEBUG NAMES ${ZLIB_NAMES_DEBUG} )

# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if 
# all listed variables are TRUE
# TODO !!!!!! I do not know if I have to modiy this for the debug library stuff
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZLIB DEFAULT_MSG ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

IF(ZLIB_FOUND)
  SET( ZLIB_LIBRARIES optimized ${ZLIB_LIBRARY} )
ELSE(ZLIB_FOUND)
  SET( ZLIB_LIBRARIES )
ENDIF(ZLIB_FOUND)
