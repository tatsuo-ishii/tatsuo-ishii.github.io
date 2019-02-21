---
layout: post
title: Pgpool-II 4.0.3などがリリースされました
categories: [PostgreSQL]
tags: [Pgpool-II, 4.0.3, 3.7.8. 3.6.15, 3.5.19, 3.4.22]
---

 [Pgpool-II](https://pgpool.net)のマイナーリリースがありました。
今回リリースされたのは、4.0.3, 3.7.8. 3.6.15, 3.5.19, 3.4.22です。
[リリースノートはこちら](http://www.pgpool.net/docs/latest/ja/html/release.html)

4.0.3の主な修正点は以下です。

- detach_false_primary = onのときに、pg_stat_wal_reciver()が返すwal_receiver_conninfo情報に"host"情報が含まれていると、誤って正しいバックエンドの状態を正しくないと誤検知することがある問題を修正しました

- 拡張問い合わせ処理中にSELECTなどが大量の検索結果を返す際にメモリが足りなくなる問題を修正しました(bug 462)

- 空文字列の問い合わせを処理中にセグメンテーションフォルトすることがあるのを修正しました(bug 458)

- watchdogがクエリモードで監視しているときにセグメンテーションフォルトするのを修正しました(bug 455)

- PostgreSQLが"terminating connection due to idle-in-transaction timeout"でエラーになった時に、Pgpool-IIがハングしたり、フェイルオーバーするの問題を修正しました(bug 448)。

- 特定の場合に拡張問い合わせクエリ処理中にハングアップする[問題](https://www.pgpool.net/pipermail/pgpool-hackers/2018-December/003164.html)を修正しました

- PAM認証が失敗するのを修正しました[pgpool-general: 6353]

- 特定のrace conditionでPgpool-IIがセグメンテーションフォルトするのを修正しました[pgpool-hackers: 3214]
