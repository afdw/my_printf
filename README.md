my_printf
===

This is a library that consists of:

* `printf` (and related functions, using custom `output_stream` instead of `FILE *`) implementation
* Arbitrary-precision integer implementation
* Floating point with arbitrary precision implementation

To my knowledge, everything in the `printf` functions is implemented, except for obscure locale-dependant features such as apostrophe (`'`) flag and wide characters and strings (`%lc` and `%ls`).

Additional length modifiers are available: `Z` for printing `struct ap` and `F` for printing `struct fp`.

Note that all of the public functions (including `printf`) that accept `struct ap` or `struct fp` consume these arguments (will call the corresponding destroy functions on them). The only exception
from this rule are the copying functions (`ap_copy` and `fp_copy`) that should be used to pass to a function a `struct ap`
or `struct fp` that will be used afterwards.

## Usage example

See [`example/main.c`](example/main.c) for some examples of using the features mentioned above.

## Testing

To run the [PFT](https://github.com/gavinfielder/pft) test suite, execute the following commands in the repository root (after checking out Git submodules):

```shell
my_printf$ mkdir cmake-build-debug
my_printf/cmake-build-debug$ cd cmake-build-debug
my_printf/cmake-build-debug$ cmake ..
my_printf/cmake-build-debug$ cd ../tests/pft
my_printf/tests/pft$ ./enable-test -q
my_printf/tests/pft$ ./disable-test -q '*_throwswarning'
my_printf/tests/pft$ ./disable-test -q '*_D_*'
my_printf/tests/pft$ ./disable-test -q '*_widechar_*'
my_printf/tests/pft$ ./disable-test -q '*_widestr_*'
my_printf/tests/pft$ make && ./test -X
```

## License

This project is licensed under either of

* The Unlicense ([LICENSE-UNLICENSE](LICENSE-UNLICENSE) or
  https://unlicense.org/)
* MIT license ([LICENSE-MIT](LICENSE-MIT) or
  http://opensource.org/licenses/MIT)

at your option.
