---
layout: post
title: Pgpool-II 4.1 beta1がリリースされました
categories: [blog]
tags: [Pgpool-II, Major-releases, 4.1beta1]
---
待望のPgpool-II 4.1 beta1がリリースされました。

[リリースアナウンス](https://www.pgpool.net/mediawiki/jp/index.php/%E3%83%A1%E3%82%A4%E3%83%B3%E3%83%9A%E3%83%BC%E3%82%B8#Pgpool-II_4.1_beta1_.E3.83.AA.E3.83.AA.E3.83.BC.E3.82.B9_.282019.2F09.2F06.29)

Pgpool-II 4.1では、多くの新機能追加、性能改善が行われています。

- ステートメントレベルの負荷分散
- PostgreSQLスタンバイノードの自動復帰
- 共有メモリを利用した高速リレーションキャッシュの実装
- SQLパーサの取り込みを含むPostgreSQL 12への対応
- DML専用の簡易化されたSQLパーサの導入による高速化
- クライアントからの多すぎる接続要求を弾く機能
- COPY文の大幅な高速化
- 地理的に離れた場所にストリーミングレプリケーションのプライマリサーバとスタンバイサーバを設置するディザスタリカバリ構成への対応

詳しくは[リリースノート](https://www.pgpool.net/docs/41/ja/html/release-4-1.html)をどうぞ。

なお、Pgpool-II 4.1の正式リリースは、2019年10月を予定しています。
