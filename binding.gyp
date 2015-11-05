{
  "targets": [
    {
      "target_name": "rabin",
      "sources": [ "rabin.cc", "src/msb.cc", "src/rabinpoly.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
    },
  ]
}
