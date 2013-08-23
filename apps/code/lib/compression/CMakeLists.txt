ENABLE_QT()

# minizip
#INCLUDE(FindZLIB)
#INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
#LINK_DIRECTORIES(${ZLIB_LIBRARY_DIRS})

SET(MINIZIP_HDRS
    minizip/crypt.h
    minizip/ioapi.h
    minizip/mztools.h
    minizip/unzip.h
    minizip/zip.h
)

SET(MINIZIP_SRCS
    minizip/ioapi.c
    minizip/mztools.c
    minizip/unzip.c
    minizip/zip.c
)

SET(MINIZIP_WIN_HDRS
    minizip/iowin32.h
)
SET(MINIZIP_WIN_SRCS
    minizip/iowin32.c
    minizip/miniunz.c
    minizip/minizip.c
)

IF(WIN32)
    SET(MINIZIP_HDRS ${MINIZIP_WIN_HDRS} ${MINIZIP_HDRS})
    SET(MINIZIP_SRCS ${MINIZIP_WIN_SRCS} ${MINIZIP_SRCS})
ENDIF(WIN32)

# minizip source
SET(MINIZIP_SRCS ${MINIZIP_HDRS} ${MINIZIP_SRCS})


# unrar
ADD_DEFINITIONS(-DRARDLL)
ADD_DEFINITIONS(-DSFX_MODULE)
ADD_DEFINITIONS(-DSILENT)

SET(UNRAR_HDRS
    unrar/archive.hpp
    unrar/array.hpp
    unrar/cmddata.hpp
    unrar/coder.hpp
    unrar/compress.hpp
    unrar/consio.hpp
    unrar/crc.hpp
    unrar/crypt.hpp
    unrar/dll.hpp
    unrar/encname.hpp
    unrar/errhnd.hpp
    unrar/extinfo.hpp
    unrar/extract.hpp
    unrar/filcreat.hpp
    unrar/filefn.hpp
    unrar/file.hpp
    unrar/filestr.hpp
    unrar/find.hpp
    unrar/getbits.hpp
    unrar/global.hpp
    unrar/headers.hpp
    unrar/int64.hpp
    unrar/isnt.hpp
    unrar/list.hpp
    unrar/loclang.hpp
    unrar/log.hpp
    unrar/match.hpp
    unrar/model.hpp
    unrar/options.hpp
    unrar/os.hpp
    unrar/pathfn.hpp
    unrar/rardefs.hpp
    unrar/rarfn.hpp
    unrar/rar.hpp
    unrar/rarlang.hpp
    unrar/raros.hpp
    unrar/rartypes.hpp
    unrar/rarvm.hpp
    unrar/rawread.hpp
    unrar/rdwrfn.hpp
    unrar/recvol.hpp
    unrar/resource.hpp
    unrar/rijndael.hpp
    unrar/rs.hpp
    unrar/savepos.hpp
    unrar/scantree.hpp
    unrar/sha1.hpp
    unrar/smallfn.hpp
    unrar/strfn.hpp
    unrar/strlist.hpp
    unrar/suballoc.hpp
    unrar/system.hpp
    unrar/timefn.hpp
    unrar/ulinks.hpp
    unrar/unicode.hpp
    unrar/unpack.hpp
    unrar/version.hpp
    unrar/volume.hpp
)

SET(UNRAR_SRCS
    unrar/archive.cpp
    unrar/arcread.cpp
    unrar/cmddata.cpp
    unrar/consio.cpp
    unrar/crc.cpp
    unrar/crypt.cpp
    unrar/dll.cpp
    unrar/encname.cpp
    unrar/errhnd.cpp
    unrar/extinfo.cpp
    unrar/extract.cpp
    unrar/filcreat.cpp
    unrar/file.cpp
    unrar/filefn.cpp
    unrar/filestr.cpp
    unrar/find.cpp
    unrar/getbits.cpp
    unrar/global.cpp
    unrar/int64.cpp
    unrar/isnt.cpp
    unrar/list.cpp
    unrar/log.cpp
    unrar/match.cpp
    unrar/options.cpp
    unrar/pathfn.cpp
    unrar/rar.cpp
    unrar/rarvm.cpp
    unrar/rawread.cpp
    unrar/rdwrfn.cpp
    unrar/recvol.cpp
    unrar/resource.cpp
    unrar/rijndael.cpp
    unrar/rs.cpp
    unrar/savepos.cpp
    unrar/scantree.cpp
    unrar/sha1.cpp
    unrar/smallfn.cpp
    unrar/strfn.cpp
    unrar/strlist.cpp
    unrar/system.cpp
    unrar/timefn.cpp
    unrar/ulinks.cpp
    unrar/unicode.cpp
    unrar/unpack20.cpp
    unrar/unpack.cpp
    unrar/volume.cpp
)

SET(UNRAR_WIN_SRCS
    unrar/arccmt.cpp
    unrar/beosea.cpp
    unrar/coder.cpp
    unrar/model.cpp
    unrar/os2ea.cpp
    unrar/rarvmtbl.cpp
    unrar/suballoc.cpp
    unrar/unios2.cpp
    unrar/unpack15.cpp
    unrar/uowners.cpp
    unrar/win32acl.cpp
    unrar/win32stm.cpp
)

# Jim: should be improved
IF(NOT UNIX)
   SET(UNRAR_SRCS ${UNRAR_WIN_SRCS} ${UNRAR_SRCS})
ENDIF(NOT UNIX)

# unrar source
SET(UNRAR_SRCS ${UNRAR_HDRS} ${UNRAR_SRCS})


# 7z
# add this definition because of original makefile of 7z has -D_SZ_ONE_DIRECTORY
# in CFLAGS
ADD_DEFINITIONS(-D_SZ_ONE_DIRECTORY)

SET(7Z_HDRS
    7z/7zAlloc.h
    7z/7zBuffer.h
    7z/7zCrc.h
    7z/7zDecode.h
    7z/7zExtract.h
    7z/7zHeader.h
    7z/7zIn.h
    7z/7zItem.h
    7z/7zMethodID.h
    7z/Alloc.h
    7z/BranchTypes.h
    7z/BranchX86_2.h
    7z/BranchX86.h
    7z/LzmaDecode.h
    7z/LzmaTypes.h
    7z/Types.h
)

SET(7Z_SRCS
    7z/7zAlloc.c
    7z/7zBuffer.c
    7z/7zCrc.c
    7z/7zDecode.c
    7z/7zExtract.c
    7z/7zHeader.c
    7z/7zIn.c
    7z/7zItem.c
    7z/7zMain.c
    7z/7zMethodID.c
    7z/Alloc.c
    7z/BranchX86_2.c
    7z/BranchX86.c
    7z/LzmaDecode.c
)

# 7z source
SET(7Z_SRCS ${7Z_HDRS} ${7Z_SRCS})


# compression library
ADD_LIBRARY(compression ${MINIZIP_SRCS} ${UNRAR_SRCS} ${7Z_SRCS})
TARGET_LINK_LIBRARIES(compression
    z
)
