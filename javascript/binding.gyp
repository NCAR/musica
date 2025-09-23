{
  "targets": [
    {
      "target_name": "musica-addon",
      "cflags_cc": [
        "-std=c++20",
        "-fexceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "sources": [
        "src/musica_addon.cpp",
        "src/musica_wrapper.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "musica-deps/include",
        "musica-deps/build/include",
        "musica-deps/build/_deps/micm-src/include",
        "musica-deps/build/_deps/mechanism_configuration-src/include",
        "musica-deps/build/_deps/yaml-cpp-src/include"
      ],
      "defines": [
        "NAPI_VERSION=8",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        [
          "OS=='mac'",
          {
            "cflags_cc": [
              "-std=c++20",
              "-stdlib=libc++"
            ],
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
              "MACOSX_DEPLOYMENT_TARGET": "10.15",
              "OTHER_LDFLAGS": [
                "-framework", "Foundation",
                "-framework", "Metal"
              ]
            }
          }
        ],
        [
          "OS=='win'",
          {
            "defines": [
              "BUILDING_NODE_EXTENSION"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1
              }
            }
          }
        ]
      ]
    }
  ]
}