# ================================
#   Common
# ================================
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  carla_warning (
    "CARLA is set to be built in Debug mode. This may cause issues when building CarlaUnrealEditor."
  )
endif ()

if (LINUX)
  check_linker_flag (CXX -lpthread HAS_PTHREAD)
  if (HAS_PTHREAD)
    add_link_options (-lpthread)
  endif ()
endif ()

if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CARLA_DEBUG_AFFIX d)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
  else ()
    set (CARLA_DEBUG_AFFIX )
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif ()
endif ()

if (WIN32)
  set (EXE_EXT .exe)
  set (LIB_EXT .lib)
  set (DLL_EXT .dll)
elseif (LINUX)
  set (EXE_EXT)
  set (LIB_EXT .a)
  set (DLL_EXT .so)
elseif (APPLE)
  set (EXE_EXT)
  set (LIB_EXT .a)
  set (DLL_EXT .so)
else ()
  carla_error ("Unknown target system.")
endif ()

# ================================
#   Common Definitions
# ================================

if (WIN32)
  add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
endif ()

set (CARLA_COMMON_DEFINITIONS)

foreach (FORMAT ${LIBCARLA_IMAGE_SUPPORTED_FORMATS})
  carla_message ("Enabling CARLA image support for \"${FORMAT}\".")
  string (TOUPPER "${FORMAT}" FORMAT_UPPERCASE)
  list (APPEND CARLA_COMMON_DEFINITIONS LIBCARLA_IMAGE_SUPPORT_${FORMAT_UPPERCASE}=1)
endforeach ()

if (WIN32)
  # Documentation: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # <- Windows 10
  list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
  list (APPEND CARLA_COMMON_DEFINITIONS _USE_MATH_DEFINES)
endif ()

# ================================
#   Exception Definitions
# ================================

set (CARLA_EXCEPTION_OPTIONS)
set (CARLA_EXCEPTION_DEFINITIONS)

if (ENABLE_EXCEPTIONS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-fexceptions HAS_fexceptions)
      if (HAS_fexceptions)
        list (APPEND CARLA_EXCEPTION_OPTIONS -fexceptions)
      endif ()
    else ()
      check_cxx_compiler_flag(/EHa HAS_EHa)
      if (HAS_EHa)
        list (APPEND CARLA_EXCEPTION_OPTIONS /EHa)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-fexceptions HAS_fexceptions)
    if (HAS_fexceptions)
      list (APPEND CARLA_EXCEPTION_OPTIONS -fexceptions)
    endif ()
  endif ()
else ()
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-fno-exceptions HAS_fnoexceptions)
      if (HAS_fnoexceptions)
        list (APPEND CARLA_EXCEPTION_OPTIONS -fno-exceptions)
      endif ()
    else ()
      check_cxx_compiler_flag(/EHsc HAS_EHsc)
      if (HAS_EHsc)
        list (APPEND CARLA_EXCEPTION_OPTIONS /EHsc)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-fno-exceptions HAS_fnoexceptions)
    if (HAS_fnoexceptions)
      list (APPEND CARLA_EXCEPTION_OPTIONS -fno-exceptions)
    endif ()
  endif ()

  list (
    APPEND
    CARLA_EXCEPTION_DEFINITIONS
  )

  list (
    APPEND
    CARLA_EXCEPTION_DEFINITIONS

    ASIO_NO_EXCEPTIONS
    BOOST_NO_EXCEPTIONS
    BOOST_EXCEPTION_DISABLE
    PUGIXML_NO_EXCEPTIONS
    LIBCARLA_NO_EXCEPTIONS
  )
endif ()

# ================================
#   RTTI Definitions
# ================================

set (CARLA_RTTI_DEFINITIONS)
set (CARLA_RTTI_OPTIONS)

if (ENABLE_RTTI)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-frtti HAS_frtti)
      if (HAS_frtti)
        list (APPEND CARLA_RTTI_OPTIONS -frtti)
      endif ()
    else ()
      check_cxx_compiler_flag(/GR HAS_GR)
      if (HAS_GR)
        list (APPEND CARLA_RTTI_OPTIONS /GR)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-frtti HAS_frtti)
    if (HAS_frtti)
      list (APPEND CARLA_RTTI_OPTIONS -frtti)
    endif ()
  endif ()
else ()
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-fno-rtti HAS_fnortti)
      if (HAS_fnortti)
        list (APPEND CARLA_RTTI_OPTIONS -fno-rtti)
      endif ()
    else ()
      check_cxx_compiler_flag(/GR HAS_GRm)
      if (HAS_GRm)
        list (APPEND CARLA_RTTI_OPTIONS /GR-)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-fno-rtti HAS_fnortti)
    if (HAS_fnortti)
      list (APPEND CARLA_RTTI_OPTIONS -fno-rtti)
    endif ()
  endif ()

  list (
    APPEND
    CARLA_RTTI_DEFINITIONS
    BOOST_NO_RTTI
    BOOST_NO_TYPEID
    BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY
  )
endif ()

# ================================
#   WAll Config
# ================================

if (ENABLE_ALL_WARNINGS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-Wall HAS_WALL)
      if (HAS_WALL)
        add_compile_options (-Wall)
      endif ()
    else ()
      check_cxx_compiler_flag(/Wall HAS_WALL)
      if (HAS_WALL)
        add_compile_options (/Wall)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-Wall HAS_WALL)
    if (HAS_WALL)
      add_compile_options (-Wall)
    endif ()
  endif ()
endif ()

if (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  set (SUPPRESS_WARNING_DIRECTIVE_PREFIX -Wno-)
elseif (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set (SUPPRESS_WARNING_DIRECTIVE_PREFIX /wd)
endif ()

macro (carla_try_suppress_cxx_warning NAME FLAG)
  check_cxx_compiler_flag (
    ${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}
    HAS_${NAME}
  )
  if (HAS_${NAME})
    add_compile_options (
      $<$<COMPILE_LANGUAGE:CXX>:${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}>)
  endif ()
endmacro ()

macro (carla_try_suppress_c_warning NAME FLAG)
  check_c_compiler_flag (
    ${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}
    HAS_${NAME}
  )
  if (HAS_${NAME})
    add_compile_options (
      $<$<COMPILE_LANGUAGE:C>:${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}>)
  endif ()
endmacro ()

set (
  CARLA_C_SUPRESSED_WARNING_LIST
  macro-redefined 4005
  incompatible-pointer-types
)

set (
  CARLA_CXX_SUPRESSED_WARNING_LIST
  macro-redefined 4005
)

foreach (WARNING ${CARLA_C_SUPRESSED_WARNING_LIST})
  string (MAKE_C_IDENTIFIER "${WARNING}" WARNING_NAME)
  carla_try_suppress_c_warning (
    ${WARNING_NAME}
    ${WARNING}
  )
endforeach ()

foreach (WARNING ${CARLA_CXX_SUPRESSED_WARNING_LIST})
  string (MAKE_C_IDENTIFIER "${WARNING}" WARNING_NAME)
  carla_try_suppress_cxx_warning (
    ${WARNING_NAME}
    ${WARNING}
  )
endforeach ()

# ================================
#   WError Config
# ================================

if (ENABLE_WARNINGS_TO_ERRORS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-Werror HAS_WALL)
      if (HAS_WALL)
        add_compile_options (-Werror)
      endif ()
    else ()
      check_cxx_compiler_flag(/WX HAS_WALL)
      if (HAS_WALL)
        add_compile_options (/WX)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-Werror HAS_WALL)
    if (HAS_WALL)
      add_compile_options (-Werror)
    endif ()
  endif ()
endif ()
