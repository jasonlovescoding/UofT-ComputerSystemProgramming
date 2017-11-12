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
#my @namelist = ("libc", "kheap", "alloc");
my @namelist = ("alloc");
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
	my $cmd1 = "echo \"\" > $dir/Results/$name/larson-$i";
	system "$cmd1";
	for (my $j = 1; $j <= $iters; $j++) {
	    my $killed = 0;
	    my $pid;
	    my $maxtime = `head -1 $dir/Input/small-$i-threads`;
	    $maxtime *= 2;
	    print "Iteration $j, maxtime $maxtime\n";
	    my $cmd = "$dir/larson-$name < $dir/Input/small-$i-threads >> $dir/Results/$name/larson-$i 2>&1";
	    alarm $maxtime;
	    if (!($pid = fork)) {
		#child		
		print "$cmd\n";		
		exec($cmd);
	    }
	    # on alarm, ignore SIGHUP locally so we don't kill daemon,
	    # and send SIGHUP to all child processes in same process group
	    $SIG{ALRM} = sub { 
		local $SIG{HUP} = 'IGNORE'; 
		kill("HUP", -$$); 
		$killed = 1; 
	    };
	    waitpid $pid, 0;
	    alarm 0;
	    if ($killed) { 
		print "KILLED "; 
		wait;
	    }
#	    system "$cmd";
	}
    }
}


