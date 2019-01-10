---
layout: default
---

---

## リリース情報とお知らせ

<ul>
  {% for post in site.posts %}
    <li>
      {% capture ymd %}{{ post.date | date: '%Y/%m/%d' }}{% endcapture %}
      <a href="{{ post.url }}">{{ post.title }} ({{ ymd }})</a>
    </li>
  {% endfor %}
</ul>
---

## その他お知らせとリンク

- [PostgreSQLオフィシャルサイト](https://www.postgresql.org)
- [Pgpoolオフィシャルサイト](https://pgpool.net)
- [PostgreSQLに関する技術情報](https://www.sraoss.co.jp/tech-blog/)
- [古いニュース記事](index-old.html)
