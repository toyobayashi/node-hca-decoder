{
  "variables": {
    "module_name": "hca",
    "module_path": "./dist"
  },
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [
        "./src/index.cpp",
        # "./src/hca.cpp",
        "./src/clHCA.cpp",
        "./src/HCAInfo.cpp",
        "./src/HCAFileStream.cpp"
      ],
      'includes': [
        './common.gypi'
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
