#!/usr/bin/perl

use strict;

my $dir=$ARGV[0];
my $graphtitle = "threadtest - runtimes";

#my @namelist = ("libc", "kheap", "alloc");
my @namelist = ("alloc");
my %names;

# This allows you to give each series a name on the graph
# that is different from the file or directory names used
# to collect the data.  We happen to be using the same names.
$names{"libc"} = "libc";
$names{"kheap"} = "kheap";
$names{"alloc"} = "alloc";

my $name;
my %scalability;
my %fragmentation;
my %baseline;

my $reqd_mem = 4096; # Min amt of memory for test, for any no. of threads
my $reftime=0.135;   # Sequential libc speed - libc time for 1 thread
my $nthread = 8;

foreach $name (@namelist) {
    open G, "> $dir/Results/$name/data";
    $baseline{$name} = 0;
    $scalability{$name} = 0;
    for (my $i = 1; $i <= 8; $i++) {
	open F, "$dir/Results/$name/threadtest-$i";
	my $total = 0;
	my $count = 0;
	my $min = 1e30;
	my $max = -1e30;
	my $mem_min = 1e30;
	my $mem_max = -1e30;
	my $mem_total = 0;

	while (<F>) {
	    chop;
	    # Runtime results
	    if (/([0-9]+\.[0-9]+) seconds/) {
		#	 print "$i\t$1\n";
		my $current = $1;
		$total += $1;
		$count++;
		if ($current < $min) {
		    $min = $current;
		}
		if ($current > $max) {
		    $max = $current;
		}
	    }
	    # Memory usage results
	    if (/Memory used =\s+([0-9]+)\s+/) {
	      my $current = $1;
	      $mem_total += $1;
	      if ($current < $mem_min) {
		$mem_min = $current;
	      }
	      if ($current > $mem_max) {
		$mem_max = $current;
	      }
	    }

	}
	if ($count > 0) {
	    my $avg = $total / $count;
	    #	   print G "$i\t$avg\t$min\t$max\n";
	    print G "$i\t$min\n";
	    $avg = $mem_total / $count;
	    #Fragmentation score should be < 0 if used > minimum memory 
	    $fragmentation{$name} += $reqd_mem / $avg; 
	    if ($i == 1) {
		$baseline{$name} = $max; #Use worst single-thread as baseline
	    } elsif ($i < 8) {	# don't count 8 thread case
		my $speedup = $baseline{$name} / $min;
		$scalability{$name} += $speedup / $i;
		my $contrib = $speedup/$i;
		print "i=$i, speedup[i]=$speedup, contrib=$contrib, scal now $scalability{$name}\n";
	    }
	} else {
	    print "oops count is zero, $name, threadtest-$i\n";
	}

	close F;
    }
    close G;


}


open PLOT, "|gnuplot";
print PLOT "set term postscript\n";
print PLOT "set output \"$dir/threadtest.ps\"\n";
print PLOT "set title \"$graphtitle\"\n";
print PLOT "set ylabel \"Runtime (seconds)\"\n";
print PLOT "set xlabel \"Number of threads\"\n";
print PLOT "set xrange [0:9]\n";
print PLOT "set yrange [0:*]\n";
print PLOT "plot ";

foreach $name (@namelist) {

    open SCORE, "> $dir/Results/$name/scores";
    $scalability{$name} /= 6.0;
    printf "name = $name\n\tscalability score %.3f\n",$scalability{$name};
    $fragmentation{$name} /= $nthread;
    printf "\tfragmentation score = %.3f\n\n",$fragmentation{$name};
    printf SCORE "\tsequential speed = %.3f\n",$reftime/$baseline{$name};
    printf SCORE "\tscalability score = %.3f\n",$scalability{$name};
    printf SCORE"\tfragmentation score = %.3f\n\n",$fragmentation{$name};
    close SCORE;

    my $titlename = $names{$name};
    if ($name eq $namelist[-1]) {
	print PLOT "\"$dir/Results/$name/data\" title \"$titlename\" with linespoints\n";
    } else {
	print PLOT "\"$dir/Results/$name/data\" title \"$titlename\" with linespoints,";
    }
}
close PLOT;


