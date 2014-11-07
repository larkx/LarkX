#!/usr/bin/perl -w

# Copyright (C) 2014, Peter Conrad <conrad@quisquis.de>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

use strict;

if ($#ARGV != 1) {
    print STDERR "Usage: $0 <seeder-db> <default-port>\n";
    exit 1;
}

my $DB_FILE = $ARGV[0];
my $PORT = $ARGV[1];

my $active_addrs = &parse_input($PORT);
my $known_addrs = &parse_db($DB_FILE);
&update_db($known_addrs, $active_addrs);
&save_db($DB_FILE, $known_addrs);

exit 0;

sub parse_input {
my $port = shift;
my %addrs = ();

    while ($_ = <STDIN>) {
	my $addr = &parse_input_line($_, $port);
	if ($addr) { $addrs{$addr} = 1; }
    }
    return \%addrs;
}

sub parse_input_line {
$_ = shift;
my $port = shift;

    if (! / connection_accepted_message_type /) { return 0; }
    if (! / from peer ([0-9a-f.:]*):(\d+)\s*$/s) {
	print STDERR "Warning: can't parse line $_\n";
	return 0;
    }
    if ($2 != $port) {
	print STDERR "Info: skipping non-default port $1:$2\n";
	return 0;
    }
    return $1;
}

sub parse_db {
my $filename = shift;
my %entries = ();

    if (!open(DB, "<", $filename)) {
	if (! -r $filename) { return \%entries; }
	die("Can't open $filename!?\n");
    }
    while ($_ = <DB>) {
	my ($addr, $seen) = &parse_db_line($_);
	if ($addr) { $entries{$addr} = $seen; }
    }
    close DB;

    return \%entries;
}

sub parse_db_line {
    $_ = shift;
    $_ =~ s/[\r\n]+$//;
    my ($addr, $weight, $history) = split(/\t/);
    return ($addr, $history);
}

sub update_db {
my $known = shift;
my $active = shift;

    for my $addr (keys %$known) {
	my $app = '-';
	if (exists($active->{$addr})) {
	    $app = '+';
	}
	$known->{$addr} = substr($known->{$addr}.$app, 1);
    }

    for my $addr (keys %$active) {
	if (!exists($known->{$addr})) {
	    $known->{$addr} = ('?' x 95).'+';
	}
    }
}

sub save_db {
my $filename = shift;
my $entries = shift;

    my $lines = &calc_weights($entries);
    open(DB, ">", $filename.'.tmp') or die("Can't create new DB file!?!");
    for my $line (&sort_db($lines)) {
	print DB &db_line_as_string($line)."\n";
    }
    close DB;
    rename $filename.'.tmp', $filename;
}

sub calc_weights {
my $entries = shift;
my @lines = ();

    for my $addr (keys %$entries) {
	my $history = $entries->{$addr};
	push @lines,
	     [ $addr, &calc_weight($addr, $entries->{$addr}), $history ];
    }
    return \@lines;
}

sub calc_weight {
my $addr = shift;
my $history = shift;

    my $i = length($history);
    my $weight = 12;
    my $sum = 0;
    while ($i-- > 0) {
	my $char = substr($history, $i, 1);
	if ($char eq '+') { $sum += $weight; }
	elsif ($char eq '-') { $sum -= .1; }
	if ($weight > 1) { $weight--; }
    }
    return $sum;
}

sub sort_db {
my $lines = shift;

    return sort { $b->[1] <=> $a->[1] } @$lines;
}

sub db_line_as_string {
my $line = shift;

    return join("\t", @$line);
}
