ENABLE_QT()

# application
SET(run_shell_srcs run_shell.cpp)
ADD_EXECUTABLE(run_shell.oar ${run_shell_srcs})
TARGET_LINK_LIBRARIES(run_shell.oar
   ${QT_LIBRARIES}
   ${ADD_LIB}
)
# Install the script_manager to $BUILD/bin
INSTALL(TARGETS run_shell.oar RUNTIME DESTINATION bin)

# oar runner for user application
SET(oar_wrapper_hrds user_process.h)
SET(oar_wrapper_srcs user_process.cpp oar_wrapper.cpp)

# oar_wrapper
ADD_EXECUTABLE(oar_wrapper ${oar_wrapper_hrds} ${oar_wrapper_srcs})
TARGET_LINK_LIBRARIES(oar_wrapper
   ${QT_LIBRARIES}
   ${ADD_LIB}
)

# Install the oar_wrapper to $BUILD/bin
INSTALL(TARGETS oar_wrapper RUNTIME DESTINATION bin)
