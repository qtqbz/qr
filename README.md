# Terminal QR encoder

This is a simple terminal utility for generating QR codes.

![image](./qr_demo.png)

### Notes
- Written in C
- Low memory footprint
- NO external dependencies
- NO heap allocations

### Requirements
- GCC 14.2.1+
- Make

### Build
```
$ make build
```

### Usage
```
qr [OPTIONS]

OPTIONS:
    -t TEXT    Encode the given TEXT. Cannot be combined with -f.
    -f FILE    Encode the content of FILE. Cannot be combined with -t.
    -l LEVEL   Force error correction level from 0 (Low) to 3 (High).
    -v VERSION Force QR version from 1 to 40.
    -m MASK    Force mask pattern from 0 to 7.
    -d         Print debugging messages to STDERR.
If neither -t nor -f is specified, encodes the data read from STDIN.
```

### TODO:
- Support kenji 
- Add tests
- Use SIMD
