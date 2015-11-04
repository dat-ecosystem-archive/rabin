{
  "targets": [
    {
      "target_name": "rabin",
      "sources": [ "rabin.cc", "src/msb.cc", "src/rabinpoly.cc", "src/sha1.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
    },
  ]
}
