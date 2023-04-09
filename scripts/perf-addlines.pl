#!/usr/bin/env perl

# Author: Stefan Talpalaru
# Usage: 
# perf record --call-graph dwarf,16000 ...
# perf script -F +srcline | ./filter-perf-script.pl > out.perf
# ./stackcollapse-perf.pl out.perf > perf_collapsed.txt
# ./flamegraph.pl perf_collapsed.txt > perf.svg

use strict;
use warnings;

my $prev_line = "";
my $line_info;

while(<>) {
    if(/^  \S+/ and $prev_line =~ /\+0x/) {
        $line_info = $_;
        $line_info =~ s/^\s+//;
        $line_info =~ s/\s+$//;
        $line_info =~ s/^(\S+)/|$1|/;

        $prev_line =~ s/\+0x/$line_info+0x/;
    }
    if(not $prev_line =~ /^  \S+/) {
        print($prev_line);
    }
    $prev_line = $_;
}
print($prev_line);

