{
  "targets": [
    {
      "target_name": "rabin",
      "sources": [ "bindings.cc", "src/rabin.cc"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
    },
  ]
}
