CLANG="/home/brenocfg/Work/llvm-3.7/test/bin/clang"
PLUGIN="/home/brenocfg/Work/llvm-3.7/test/lib/scope-finder.so"
INPUT="$1"

$CLANG -DLARGE_DATASET -Xclang -load -Xclang $PLUGIN -Xclang -add-plugin -Xclang -find-scope -g -O0 -fsyntax-only $INPUT
