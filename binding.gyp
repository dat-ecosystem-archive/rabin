{
  "targets": [
    {
      "target_name": "rabin",
      "sources": [ "rabin.cc", "src/rabin.c"],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
    },
  ]
}
