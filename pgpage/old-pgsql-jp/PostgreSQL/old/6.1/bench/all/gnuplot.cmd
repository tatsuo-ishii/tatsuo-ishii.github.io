set yrange [0:6]
set ylabel "sec"
set data style linespoints
set terminal pbm color
plot  "dec1.data" title "Digital UNIX V4.0B/DEC AlphaServer 4000 5/400/Alpha 21164A/400MHz(512MB)",  "linux7.data" title "Linux 2.0.30/Pentium II 266MHz/64MB",  "dec3.data" title "DEC Alpha Server 8200(4CPU)/Digital Unix 4.0B cc",  "linux13.data" title "Linux 2.0.30(SMP kernel)/Pentium Pro 200MHz x 2/128MB(6.1.1)",  "mono-linux.data" title "Linux 2.1.24(mono-linux)/PowerMac7600/200(PPC604e/200MHz)/288MB+JP patch",  "ultra2_cc.data" title "Solaris2.5.1/Ultra 2(200MHz x 2CPU, 512M)/Sun Pro C+jp patch",  "freebsd12.data" title "FreeBSD 2.2.2R/Pentium pro 200MHz/64MB",  "freebsd6.data" title "FreeBSD 2.2.1-RELEASE/Pentium Pro 200MHz/128MB+JP patch",  "freebsd4.data" title "FreeBSD 3.0-CURRENT/Pentium Pro 200MHz x2/64MB",  "freebsd3.data" title "FreeBSD 3.0-CURRENT/Pentium Pro 200MHz/64MB+JP patch"
