#!/usr/bin/perl

use strict;

# Check for correct usage
if (@ARGV != 1) {
    print "usage: runall.pl <dir>\n";
    print "    where <dir> is the directory containing the benchmark subdirectories\n";
    die;
}

my $dir=$ARGV[0];
my $name;
my $iters = 5;

foreach $name ("cache-scratch", "cache-thrash", "threadtest", "larson") {
  print "benchmark name = $name\n";
  print "running $dir/$name/runtests.pl\n";
  system "$dir/$name/runtests.pl $dir/$name $iters";
  system "$dir/$name/graphtests.pl $dir/$name"; 
}


