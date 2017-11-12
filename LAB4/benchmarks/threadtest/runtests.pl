#!/usr/bin/perl

use strict;

# Check for correct usage
if (@ARGV != 2) {
  print "usage: runtests.pl <dir> <iters>\n";
  print "    where <dir> is the directory containing the test executable and\n";
  print "    Results subdirectory, and <iters> is the number of trials to perform.\n";
  die;
}

my $dir = $ARGV[0];
my $iters = $ARGV[1];

#Ensure existence of $dir/Results
if (!-e "$dir/Results") {
    mkdir "$dir/Results", 0755
	or die "Cannot make $dir/Results: $!";
}

# Initialize list of allocators to test.
#my @namelist = ("libc", "kheap", "a2alloc");
my @namelist = ("libc", "alloc");
# replace this one with just "libc" and "kheap" if you wish to run those allocators
#my @namelist = ("a2alloc");
my $name;

foreach $name (@namelist) {
    print "allocator name = $name\n";
    # Create subdirectory for current allocator results
    if (!-e "$dir/Results/$name") {
	mkdir "$dir/Results/$name", 0755
	    or die "Cannot make $dir/Results/$name: $!";
    }

    # Run tests for 1 to 8 threads
    for (my $i = 1; $i <= 8; $i++) {
	print "Thread $i\n";
	my $cmd1 = "echo \"\" > $dir/Results/$name/threadtest-$i";
	system "$cmd1";
	for (my $j = 1; $j <= $iters; $j++) {
	    print "Iteration $j\n";
	    my $cmd = "$dir/threadtest-$name $i 1000 420 10 >> $dir/Results/$name/threadtest-$i 2>&1";
	    print "$cmd\n";
	    system "$cmd";
	}
    }
}


