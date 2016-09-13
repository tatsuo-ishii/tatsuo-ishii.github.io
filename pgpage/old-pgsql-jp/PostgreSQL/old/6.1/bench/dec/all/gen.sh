#! /bin/sh
#
# usage: gen.sh index
#
# "index" �ե�����Υե����ޥåȤϡ�
#  f1 f2 f3 (�ƥե�����ɤ�@�Ƕ��ڤ�)
#  f1: �ǡ����ե�����̾
#  f2: �ץ�åȥե�����̾(����յڤ�html�˥����ȤȤ���ɽ���)
#  f3: ��������URL�ʤ�(bench.html�ǻ���)
#
# gnuplot �ѤΥǡ����ե�������롣
# ���ʤߤˡ�"list" �ϡ�����ץ� (query9)�Υǡ����� index ����1 �ե���
# ��ɤ������ɲä�����Τǡ���� sort ������˻Ȥ���
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
# gnuplot �Υ��ޥ�ɥե�������� (#1)
#
sort -t@ -n list|
awk '
BEGIN{
FS = "@"
#print "set logscale y"
print "set yrange [0:3]"
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
echo 'load "gnuplot.cmd"'|
/usr/mgr/t-ishii/src/gnuplot/gnuplot|ppmtogif > bench1.gif
#
# bench.html ����
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
print "<h1>PostgreSQL �� Wisconsin Benchmark</h1>"
print "<h2>�٥���ޡ������(DEC Alpha ������)</h2>"
print "<img src=\"bench1.gif\">"
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
print "<img src=\"../../../../gif/return06.gif\" border=0>"
print "<a href=\"../bench.html\">"
print "[return to the benchmark top page]</a>"
print ""
print "</body>"
print "</html>"
}' > bench.html
