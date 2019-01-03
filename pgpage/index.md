---
layout: default
---

---

## リリース情報とお知らせ

<ul>
  {% for post in site.posts %}
    <li>
      {% capture year %}{{ post.date | date: '%Y' }}{% endcapture %}
      {% capture month %}{{ post.date | date: '%m' }}{% endcapture %}
      {% capture day %}{{ post.date | date: '%d' }}{% endcapture %}
      <a href="{{ post.url }}">{{ post.title }} ({{ year }}/{{ month }}/{{ day }})</a>
    </li>
  {% endfor %}
</ul>
---

## その他お知らせとリンク

- [PostgreSQLオフィシャルサイト](https://www.postgresql.org)
- [Pgpoolオフィシャルサイト](https://pgpool.net)
- [PostgreSQLに関する技術情報](https://www.sraoss.co.jp/technology/postgresql/)
- [古いニュース記事](index-old.html)
