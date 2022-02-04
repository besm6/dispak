#!/usr/bin/env perl
sub cmd {
    $v = $_[0];
    if ($v & 02000000) {
        return sprintf("%02o %02o %05o", $v >> 20, ($v >> 15) & 037, $v & 077777);
    } else {
        return sprintf("%02o %03o %04o", $v >> 20, ($v >> 12) & 0177, $v & 07777);
    }
}
while (<>) {
    if (! /(.*):  ((\d\d\d\d) (\d\d\d\d) (\d\d\d\d) (\d\d\d\d))  (...... ......)(.*)/) {
        print; next;
    }
    
    $loc = $1;
    $o = $2;
    $bemsh = $8;
    $cmd1 = oct($3) << 12 | $4;
    $cmd2 = oct($5) << 12 | $6;
    
    print "$loc:  $o  ", cmd($cmd1), ' ', cmd($cmd2), $bemsh, "\n";
}
