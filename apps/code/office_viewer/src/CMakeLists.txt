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
SET(MOC_HDRS  main_widget.h  onyx_alien.h  onyx_office.h onyx_office_view.h  )
QT4_WRAP_CPP(MOC_SRCS ${MOC_HDRS})

FILE(GLOB picsel picsel/alien-common/*.c picsel/linux-alien-common/*.c )

add_library(onyx_office
            ${picsel}
            ${MOC_SRCS}
            onyx_alien.cpp
            main_widget.cpp
            onyx_office.cpp
            onyx_office_view.cpp)

TARGET_LINK_LIBRARIES(onyx_office
${QT_LIBRARIES}
${staticlib}
m pthread
onyxapp
onyx_data
onyx_screen
onyx_ui
${ADD_LIB})

ADD_EXECUTABLE(office_viewer main.cpp)
TARGET_LINK_LIBRARIES(office_viewer onyx_office)
