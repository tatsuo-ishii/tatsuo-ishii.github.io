#!/sbin/sh
SYBASE=/disk2/usr/sybase
export SYBASE
DSQUERY=SGISYB
export DSQUERY
echo 'Content-Type: text/html'
echo
echo '<TITLE>���ʥǡ����١���</TITLE>'
echo '<P>'
echo '<H1>'
echo '<CENTER>'
echo '���ʥǡ����١���'
echo '</CENTER>'
echo '</H1>'
echo '<P>'
echo '<HR>'
echo '<CENTER>'
export a1;a1=`echo $QUERY_STRING | sed s/time=//g`
export a2;a2=`echo $a1 | sed s/\&/\ /g `
set ${a2}
echo '<P>'
if [ $1 = '4d' ] ;
then
  echo '<IMG Align=bottom SRC="/4d1.GIF">'
  echo '<IMG Align=bottom SRC="/4d2.GIF"><P>'
fi 
if [ $1 = 'vs' ] ;
then
  echo '<IMG Align=bottom SRC="/vs1.GIF">'
  echo '<IMG Align=bottom SRC="/vs2.GIF"><P>'
fi 

SrchWord=$1
/disk2/usr/sybase/bin/isql -Udemowww -Pdemowww -Jeucjis -w256 << _END | sed '/REC/!d' | \
awk '{printf "���ʥ����ɡ�%s<BR>����̾�Ρ�%s<BR>�߸ˡ�%s<BR>���ʡ�%s<BR>", $2, $3, $4, $5}'
select
"REC", pid, pname, stock, price
from product where pname_srch like "%$SrchWord%"
go
_END

echo '</CENTER>'
if [ $1 = '4d' ] ;
then
  echo '<CENTER>'
  echo '<TABLE BORDER>'
  echo '<TR>'
  echo '<TH ROWSPAN="2" CLOSEPAN="2"></TH>'
  echo '        <TH CLOSPAN="3"></TH>   </TR>'
  echo '<TR>    <TH>�����ֹ�</TH><TH>�߸�</TH><TH>����</TH>  </TR>'
  echo '<TR>    <TH ROWSPAN="2">4thDimension</TH><TH>4th Dimension���ܸ���</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">196,000</TD>     </TR>'
  echo '<TR>    <TH>4D Runtime3.2 ���ܸ���</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">38,000</TD></TR>'
  echo '<TR>    <TH ROWSPAN="2">4D Server</TH><TH>4D Server1.2���ܸ���</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">244,000</TD>     </TR>'
  echo '<TR>    <TH>4D Server���ܸ����ɲåѥå�</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">57,000</TD>     </TR>'
  echo '<TR>    <TH ROWSPAN="1">������</TH>'
  echo '        <TH>4DFirst</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">50,000</TD></TR>'
  echo '</TABLE>'
  echo '</CENTER>'
  echo '<P>'
fi 

if [ $1 = 'vs' ] ;
then
  echo '<CENTER>'
  echo '<TABLE BORDER>'
  echo '<TR>'
  echo '<TH ROWSPAN="2" CLOSEPAN="2"></TH>'
  echo '        <TH CLOSPAN="3"></TH>   </TR>'
  echo '<TR>    <TH>�����ֹ�</TH><TH>�߸�</TH><TH>����</TH>  </TR>'
  echo '<TR>    <TH ROWSPAN="1">VisualSmalltalk</TH>'
  echo '        <TH>XXXXXXX</TH>'
  echo '        <TD>�߸ˤ���</TD><TD ALIGN="right">XX,000</TD></TR>'
  echo '</TABLE>'
  echo '</CENTER>'
  echo '<P>'
fi 
echo '<HR>'
echo '<H4>'
echo '<A HREF = "/detabase.html">�ǡ����١��������</A><BR>'
echo '<A HREF = "/InternetGW_j.html">Takenoko�����</A><BR>'
echo '<A HREF = "/index.html">�ۡ���ڡ��������</A></H4>'
