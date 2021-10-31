#!/usr/bin/env perl

#Author: Stefan Talpalaru

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

