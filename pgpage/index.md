---
layout: default
---

# このページでは、PostgreSQLに関する情報をお届けします。
---

## リリース情報とお知らせ

<ul>
  {% for post in site.posts %}
    <li>
      <a href="{{ post.url }}">{{ post.title }}</a>
    </li>
  {% endfor %}
</ul>
---

## その他お知らせとリンク

- [PostgreSQLオフィシャルサイト](https://www.postgresql.org)
- [Pgpoolオフィシャルサイト](https://pgpool.net)
- [PostgreSQLに関する技術情報](https://www.sraoss.co.jp/technology/postgresql/)
- [古いニュース記事](index-old.html)
