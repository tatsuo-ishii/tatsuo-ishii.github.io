#! /bin/sh
#
# usage: gen.sh index
#
# "index" ファイルのフォーマットは、
#  f1 f2 f3 (各フィールドは@で区切る)
#  f1: データファイル名
#  f2: プラットフォーム名(グラフ及びhtmlにコメントとして表れる)
#  f3: 元記事のURLなど(bench.htmlで使用)
#
# gnuplot 用のデータファイルを作る。
# ちなみに、"list" は、サンプル (query9)のデータを index の第1 フィー
# ルドの前に追加したもので、後で sort する時に使う。
#
cat $1|while read i
do
	data=`echo $i|awk -F@ '{print $1}`
	cp /dev/null $data.data
	awk '{printf "%d %s\n", NR-1,$3}' ../data/$data >> $data.data
	sed -n -e '10p' ../data/$data |awk '{printf "%s@", $3}'
	echo $i
done > list
#
# gnuplot のコマンドファイルを作る (#1)
#
sort -t@ -n list|tee list.bak|
awk '
BEGIN{
FS = "@"
#print "set logscale y"
print "set yrange [0:60]"
print "set ylabel \"sec\""
print "set data style linespoints"
print "set terminal pbm color"
printf "plot "
}
{
	if (NR == 1) {
		printf " \"%s.data\" title \"%s\" ", $2, $3
	} else {
		printf ", \"%s.data\" title \"%s\" ", $2, $3
	}
}
END{
printf "\n"
}'> gnuplot.cmd
/bin/cp gnuplot.cmd aaa
echo 'load "gnuplot.cmd"'|
/usr/mgr/t-ishii/src/gnuplot/gnuplot|ppmtogif > bench1.gif
#
# gnuplot のコマンドファイルを作る (Best 10)
#
sort -t@ -n list|head -10|
awk '
BEGIN{
FS = "@"
#print "set logscale y"
print "set yrange [0:6]"
print "set ylabel \"sec\""
print "set data style linespoints"
print "set terminal pbm color"
printf "plot "
}
{
	printf " \"%s.data\" title \"%s\", ", $2, $3
}
END{
printf "\n"
}'|sed 's/, $//'> gnuplot.cmd
cat gnuplot.cmd|/usr/mgr/t-ishii/src/gnuplot/gnuplot|ppmtogif > bench2.gif
#
# bench.html を作る
#
sort -t@ -n list|
awk '
BEGIN{
FS = "@"
print "<html>"
print "<head>"
print "<title>The PostgreSQL Wisconsin Benchmark</title>"
print "</head>"
print "<body bgcolor=\"#ffffff\">"
print "<h1>PostgreSQL の Wisconsin Benchmark</h1>"
print "<h2>ベンチマーク結果</h2>"
print "<img src=\"bench1.gif\">"
print "<br clear=all>"
print "<h2>ベンチマーク結果(Best 10)</h2>"
print "<img src=\"bench2.gif\">"
print "<br clear=all>"
print "<p>"
print "<ol>"
}
{
	printf "<li><a href=\"../data/%s\">%s</a>%s\n", $2, $3, $4
}
END{
print "</ol>"
printf "<hr>"
print "<img src=\"../../../gif/return06.gif\" border=0>"
print "<a href=\"../bench.html\">"
print "[return to the benchmark top page]</a>"
print ""
print "</body>"
print "</html>"
}' > bench.html
