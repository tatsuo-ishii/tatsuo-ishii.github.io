---
layout: post
title: PostgreSQL 12 beta1がリリースされました
categories: [blog]
tags: [PostgreSQL, Major-beta-releases, 12]
---

待望のPostgreSQL 12 beta1がリリースされました。

[本家のリリースアナウンス](https://www.postgresql.org/about/news/1943/)

以下、主なPostgreSQL 12の新機能、変更点です。

- B-treeインデックスの性能向上、容量削減
- 特定の場合のCTE (WITH)の最適化が可能に
- GiST, GIN, SP-GiSTの容量削減
- インデックスへの書き込みをブロックしないREINDEX
- パーティショニングの性能、機能向上
- Generated Columnsのサポート
- SQL:2016のJSON path queriesのサポート
- 最頻値(Most Common Value)拡張統計
- Pluggable Table Storage Interface
- initdbし直さなくてもpage checksumの有効/無効が可能に
- GSSAPI認証のサポート
- recovery.confがpostgresql.confに統合(recovery.confがあると、postmasterが起動しないので注意!)
- ユーザテーブルにおける行OIDがサポートされなくなりました
- JIT (Just In Time) Complilationがデフォルトで有効に

詳しい技術情報は[SRA OSSのtech blog](https://www.sraoss.co.jp/tech-blog/)をご覧ください。
