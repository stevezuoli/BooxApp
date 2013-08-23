
ENABLE_QT()
ADD_DEFINITIONS( -Wall )
SET(LANGDEFINES
-DALIEN_EN_GB_FULL
-DALIEN_EN_GB_SUBSET
-DALIEN_JA_JP
-DALIEN_KO_KR
-DALIEN_ZH_CN
-DALIEN_ZH_TW
-DALIEN_AR_AR
-DALIEN_TH_TH
-DALIEN_BN_IN
-DALIEN_GU_IN
-DALIEN_DE_VA
-DALIEN_KN_IN
-DALIEN_PA_IN
-DALIEN_TA_IN
-DALIEN_TE_IN
-DALIEN_VI_VN
)

ADD_DEFINITIONS(${LANGDEFINES})

# add this definition because of original makefile of 7z has -D_SZ_ONE_DIRECTORY
# in CFLAGS
ADD_DEFINITIONS(-D_SZ_ONE_DIRECTORY)

SET(HDRS
    archive_file_item.h
    cache_files.h
    archive_file_stream.h
    comic_main_window.h
    comic_application.h
    comic_model.h
    comic_view.h
    comic_message_dialog.h
    comic_utils.h
)
SET(SRCS
    archive_file_item.cpp
    cache_files.cpp
    archive_file_stream.cpp
    main.cpp
    comic_main_window.cpp
    comic_application.cpp
    comic_model.cpp
    comic_view.cpp
    comic_message_dialog.cpp
)

QT4_WRAP_CPP(MOC_SRCS ${HDRS})

ADD_EXECUTABLE(comic_reader ${HDRS} ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES(comic_reader
    onyx_base
    onyx_data
    onyx_screen
    onyx_ui
    onyx_sys
    onyx_cms
    ${QT_LIBRARIES}
    compression
)
