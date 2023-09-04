set(CMAKE_SYSTEM_NAME               Windows)

# Some default GCC settings
set(TOOLCHAIN_PREFIX                i686-w64-mingw32-)
set(CMAKE_C_COMPILER                gcc)
set(CMAKE_CXX_COMPILER              g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)