# hello-wasm

A game written in C for the [WASM-4](https://wasm4.org) fantasy console.

## Building

Set the path to the WASI SDK, and optional to wasm-opt:

```shell
export WASI_SDK_PATH=/PATH/TO/WASI/SDK
export WASM_OPT_PATH=/PATH/TO/WASM/OPT # e.g "/binaryen/bin/"
```

Build the cart by running:

```shell
make
```

Then run it with:

```shell
w4 run build/cart.wasm
```

For more info about setting up WASM-4, see the [quickstart guide](https://wasm4.org/docs/getting-started/setup?code-lang=c#quickstart).

## Links

### WASM

- [GitHub](https://github.com/HerrKamrat/wwww): Source code
- [wasm4](https://wasm4.org/docs): Learn more about WASM-4.
