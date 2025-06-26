function(silence_warnings target)
    set(msvc_warnings
      /wd4190
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(${target} PRIVATE ${clang_gnu_warnings})
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      target_compile_options(${target} PRIVATE ${msvc_warnings})
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      target_compile_options(${target} PRIVATE ${intel_llvm_warnings})
    endif()
  endfunction()