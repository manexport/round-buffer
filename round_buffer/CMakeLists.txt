# Make sure the compiler can find include files from our round_buffer library.
INCLUDE_DIRECTORIES(
    ${ROUND_BUFFER_SOURCE_DIR}/round_buffer
    ${ROUND_BUFFER_SOURCE_DIR}/3rd/pthreadVC2/include/pthread
)
# Make sure the linker can find the round_buffer library once it is built.
LINK_DIRECTORIES(
    ${ROUND_BUFFER_BINARY_DIR}/round_buffer
    ${ROUND_BUFFER_BINARY_DIR}/3rd/pthreadVC2/libs
)
ADD_DEFINITIONS(-D ROUND_BUFFER_EXPORTS)
# Create a library called "round_buffer" which includes the source file "round_buffer.cpp".
# The extension is already found.  Any number of sources could be listed here.

ADD_LIBRARY (round_buffer SHARED round_buffer.c)
TARGET_LINK_LIBRARIES (round_buffer pthreadVC2)

MESSAGE(STATUS "Project source directory:" ${ROUND_BUFFER_SOURCE_DIR}) 
MESSAGE(STATUS "Project libs directory:" ${ROUND_BUFFER_BINARY_DIR}) 