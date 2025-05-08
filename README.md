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
Usage: qr [OPTION]...
Where OPTION is one of the following:
    -t TEXT    Encode the given TEXT. Cannot be combined with -f.
    -f FILE    Encode the content of FILE. Cannot be combined with -t.
    -l LEVEL   Force error correction level, where VERSION is a number from 0 (Low) to 3 (High).
    -v VERSION Force QR version, where VERSION is a number from 1 to 40.
    -m MASK    Force mask pattern, where MASK is a number from 0 to 7.
    -o FORMAT  Output format, where FORMAT is one of: ANSI, ASCII, UTF8, UTF8Q. Defaults to UTF8.
    -d         Print debugging messages to STDERR.
If neither -t nor -f is specified, encodes the data read from STDIN.
```

### TODO:
- Support kenji
- Use SIMD
