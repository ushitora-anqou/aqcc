# AnQou C Compiler: aqcc

aqcc is yet another tiny self-hosted C compiler with an also tiny assembler,
linker and standard C library.

## Usage

`./aqcc [options] file...`

`options` are:

- `-S`: output an assembly file.
- `-c`: output an object file.
- `-o`: set the output file name.

To find the detail, try `make selfself_test`, which tells you all the things.

## Note

- aqcc is a product of [Security Camp 2018](https://www.ipa.go.jp/jinzai/camp/2018/zenkoku2018_index.html).
Special thanks to @rui314 san and @hikalium san.
- Many features, functions and so on have not yet been implemented in aqcc
that a C compiler generally has.
Feel free to implement missing features and to send pull requests :)


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

1段から8段までのN-Queen問題を解くサンプルが `examples/nqueen` 以下にあります。
トップディレクトリで `make examples` を実行すると、 `examples/nqueen/nqueen` が `aqcc` を用い生成されます。
`examples/nqueen/nqueen` とすると、1段から8段までの結果が表示されます。

## 一般のCファイルをコンパイル

`./aqcc [options] file...`

`options` には以下のようなものを使用できます。

- `-S`
    アセンブリファイルを出力します。
- `-c`
    オブジェクトファイルを出力します。
- `-o out`
    出力ファイル名を指定できます。

`program.c` を以下のようにしてコンパイルし、実行できます。
`aqcc` や `program.c` などは適宜読みかえてください。

```
$ ./aqcc program.c -o program
$ ./program
```

なお、`#include <stdio.h>` などとはできません。
`program.c` の中にこのような構文が含まれている場合は、取り除いて下さい。
その代わりに、自前で `puts()` 関数などの プロトタイプ宣言を `program.c` の冒頭に加えてください。
なお、カレントディレクトリ内のファイルはインクルードできますので、 `#include "aqcc.h"` などとインクルードして、
`aqcc.h` に記されているプロトタイプ宣言を流用できます。

また、標準ライブラリのうち提供されている機能はごく僅かです。
`stdlib.c` を参照して下さい。
