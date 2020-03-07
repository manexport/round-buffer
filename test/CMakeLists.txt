# Make sure the compiler can find include files from our round_buffer library.
INCLUDE_DIRECTORIES (${ROUND_BUFFER_SOURCE_DIR}/round_buffer)
INCLUDE_DIRECTORIES(
    ${ROUND_BUFFER_SOURCE_DIR}/round_buffer
    ${ROUND_BUFFER_SOURCE_DIR}/3rd/pthreadVC2/include/pthread
)
# Make sure the linker can find the round_buffer library once it is built.
LINK_DIRECTORIES(
    ${ROUND_BUFFER_BINARY_DIR}/round_buffer
    ${ROUND_BUFFER_BINARY_DIR}/3rd/pthreadVC2/libs
)
# Add executable called "test" that is built from the source files
# "test.cpp" and "test.cpp".  The extensions are automatically found.
ADD_EXECUTABLE (test test.cpp)

SET(LIBS 
    round_buffer
)
# Link the executable to the test library.
TARGET_LINK_LIBRARIES (test ${LIBS})