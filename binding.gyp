{
  "targets": [
    {
      "target_name": "mknod",
      "sources": [ "rabin-bindings.cc", "src/msb.c", "src/rabinpoly.c", "src/sha1.c" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
      ],
    }
  ]
}