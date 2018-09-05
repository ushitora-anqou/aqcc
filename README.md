# AnQou C Compiler: aqcc

## Usage

Try `make selfself_test`, which tells you all the things.

---------

AnQou C Compiler の使い方

## aqcc をコンパイル

* `make aqcc` gcc を用い `aqcc` をコンパイル
* `make aqcc_self` `aqcc` を用い `aqcc_self` をコンパイル (2度目のコンパイル)
* `make aqcc_selfself` `aqcc_self` を用い `aqcc` をコンパイル (3度目のコンパイル)

なお、何かしらの理由で生成物を皆削除したいときは、 `make clean` などとしてください。

## aqcc の挙動をテスト

* `make test` `aqcc` をテスト
* `make self_test` `aqcc_self` をテスト
* `make selfself_test` `aqcc_selfself` をテスト
- `aqcc_self` と `aqcc_selfself` がそれぞれ出力するオブジェクトファイルに違いがないことも確認されます.

## サンプルプログラム

1段から8段までのQueen 問題を解くサンプルが `examples/nqueen` 以下にあります。
トップディレクトリで `make examples` を実行すると、 `examples/nqueen/nqueen` が `aqcc` を用い生成されます。
`examples/nqueen/nqueen` とすると、1段から8段までの結果が表示されます。

## 一般のCファイルをコンパイル

`./aqcc infile outfile`

拡張子を見てオブジェクトファイルを出力するべきかアセンブリを出力するべきか判断します。

`program.c` を以下のようにしてコンパイルし、実行できます。
`aqcc` や `program.c` などは適宜読みかえてください。

```
$ ./aqcc program.c program.o
$ gcc program.o -o program
$ ./program
```

なお、`#include <stdio.h>` などとはできません。
`program.c` の中にこのような構文が含まれている場合は、取り除いて下さい。
その代わりに、自前で `puts()` 関数などの プロトタイプ宣言を `program.c` の冒頭に加えてください。
なお、カレントディレクトリ内のファイルはインクルードできますので、 `#include "aqcc.h"` などとインクルードして、
`aqcc.h` に記されているプロトタイプ宣言を流用できます。
