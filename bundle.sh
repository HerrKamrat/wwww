export WASI_SDK_PATH=../wasi-sdk-21.0
export WASM_OPT_PATH=/usr/local/Cellar/binaryen/116/bin/

../w4 bundle \
  --html-template template.html \
  --html release/index.html \
  --title WWWW \
  --description "Just some testing" \
  --icon-file "icon.png" \
  build/cart.wasm
