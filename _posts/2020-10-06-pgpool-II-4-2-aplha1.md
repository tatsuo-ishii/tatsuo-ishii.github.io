---
layout: post
title: Pgpool-II 4.2 alpha1がリリースされました
categories: [blog]
tags: [Pgpool-II, Major-releases, 4.2.0, alpha]
---
Pgpool-II 4.2 alpla1がリリースされました。

[リリースアナウンス](https://www.pgpool.net/mediawiki/jp/index.php/%E3%83%A1%E3%82%A4%E3%83%B3%E3%83%9A%E3%83%BC%E3%82%B8#Pgpool-II_4.2_alpha1_.E3.83.AA.E3.83.AA.E3.83.BC.E3.82.B9_.282020.2F10.2F06.29)

Pgpool-II 4.2のリリースを控え、最初のAlpha1がリリースされました。

V4.2 の主な新機能や改良は以下の通りです(リリースノートより)。

- 設定ファイルpgpool.confの項目が大幅に改善され、設定と管理が容易になりました。
- logging_collectorが実装され、ログ管理が容易になりました。
- log_disconnectionsが実装され、接続終了のログが取得できるようになりました。
- pg_encコマンドとpg_md5コマンドでパスワード登録をファイルから一括して行えるようになりました。
- ヘルスチェックの統計情報をSHOW POOL_HEALTH_CHECK_STATSコマンドで、発行SQLの統計情報をSHOW POOL_BACKEND_STATSコマンド取得できるようになりました。
- 新しいPCPコマンドpcp_reload_configが追加されました。
- システムカタログの情報を参照することにより、write_function_listとread_only_function_listの記述を省略できるようになりました。
- 複数PostgreSQLの間で更新の一貫性だけでなく、読み取り一貫性を保証する新しいクラスタリングモードsnapshot_isolation_modeが追加されました。
- クライアントとPgpool-IIの間でLDAP認証がサポートされました。
- SSLの設定にssl_crl_fileとssl_passphrase_commandが追加されました。
- PostgreSQL 13のSQLパーサを取り込みました。
