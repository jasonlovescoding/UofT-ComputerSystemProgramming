# This is the tester which is used to mark your ECE454-Lab4
import os
import subprocess
import os
os.chdir(os.path.dirname(os.path.realpath(__file__)))
scriptDir = os.path.dirname(os.path.realpath(__file__))

# Test
# print("STDOUTs")
# print("*********************************\n")
# print("Student's solution obtained 99.5% of the total performance portion score")
# print("\tSequential Performance Pts: 99.5%")
# print("\tMultiThreaded Performance Pts: 99.5%")
# print("\tFalse Sharing Pts: : 99.5%")
# exit()


# Run 'runall.pl' script to execute all of the benchmarks
# from subprocess import STDOUT, check_output
timeoutSeconds = 60*30
parameterList = ['./runall.pl', '.']
try:
    output = subprocess.check_output(parameterList, timeout=timeoutSeconds).decode('utf-8').strip().split("\n")  + ["*********************************\n"]
except subprocess.CalledProcessError as exc:
    output = exc.output.decode('utf-8').strip().split("\n") + ["*********************************\n"]
except subprocess.TimeoutExpired as exc:
    output = exc.output.decode('utf-8').strip().split("\n") + ["*********************************\n", "Killed after 30min\n"]


# load benchmark results into in memory data structure
import pathlib
score_cacheScratch = {}
score_cacheThrash = {}
score_larson = {}
score_threadTest = {}

try:
    path = pathlib.Path(scriptDir + '/cache-scratch/Results/alloc/scores')
    with path.open() as scoreFile:
        for line in scoreFile:
            try:
                kv = line.strip().split(" = ")
                score_cacheScratch[kv[0]] = float(kv[1])
            except:
                pass    # skips lines which does not contain scores delimited by " = "

    path = pathlib.Path(scriptDir + '/cache-thrash/Results/alloc/scores')
    with path.open() as scoreFile:
        for line in scoreFile:
            try:
                kv = line.strip().split(" = ")
                score_cacheThrash[kv[0]] = float(kv[1])
            except:
                pass  # skips lines which does not contain scores delimited by " = "

    path = pathlib.Path(scriptDir + '/larson/Results/alloc/scores')
    with path.open() as scoreFile:
        for line in scoreFile:
            try:
                kv = line.strip().split(" = ")
                score_larson[kv[0]] = float(kv[1])
            except:
                pass  # skips lines which does not contain scores delimited by " = "

    path = pathlib.Path(scriptDir + '/threadtest/Results/alloc/scores')
    with path.open() as scoreFile:
        for line in scoreFile:
            try:
                kv = line.strip().split(" = ")
                score_threadTest[kv[0]] = float(kv[1])
            except:
                pass  # skips lines which does not contain scores delimited by " = "
except OSError:
    print("No score file found due to benchmark failing to run on the student's implementation")


# Calculate Performance Portion Marks.
# Mark Breakdown:
# 15% - Single threaded performance with respect to libc malloc
# 65% - Multi-threaded performance with respect to libc malloc
#     - Scalability score of threadtest and larson gives good indication of multi-threaded performance
# 20% - False sharing avoidance performance
#     - Scalability score of cache-scratch and cache-thrash gives good indication of false sharing avoidance performance
# The marks are first calculated with respect to libc implementation of malloc.
# A threshold is then used to determine a cut off which we would award full mark

# Take the average of average sequential speed of all four benchmarks
sequentialSpeedAvg = sum([score_cacheScratch['sequential speed'], score_cacheThrash['sequential speed'],
                          score_larson['sequential speed'], score_threadTest['sequential speed']]) / 4
multiThreadedSpeedupAvg = sum([score_larson['scalability score'], score_threadTest['scalability score']]) / 2
falseSharingAvoidanceAvg = sum([score_cacheScratch['scalability score'], score_cacheThrash['scalability score']]) / 2

# Threshold derived from a good solution running on ug lab machine (4 core processor).
# This threshold is not used for marking, but only used to guage a rough performance ballpark compared to good solution
threshold_sequentialSpeedAvg = 1
threshold_multiThreadedSpeedupAvg = 1
threshold_falseSharingAvoidanceAvg = 1

# Threshold used on the 8 core server, these threshold are use for marking
# If your solution can achieve good performance using threshold derived from ug lab machine,
# you should expect good marks on the 8 core server.
# threshold_sequentialSpeedAvg = xxx
# threshold_multiThreadedSpeedupAvg = xxx
# threshold_falseSharingAvoidanceAvg = xxx

score_sequentialSpeed = sequentialSpeedAvg / threshold_sequentialSpeedAvg
score_multiThreadedSpeedup = multiThreadedSpeedupAvg / threshold_multiThreadedSpeedupAvg
score_falseSharingAvoidance = falseSharingAvoidanceAvg / threshold_falseSharingAvoidanceAvg

finalPerformancPortionMark = (min(score_sequentialSpeed, 1) * 0.15
                              + min(score_multiThreadedSpeedup, 1) * 0.65
                              + min(score_falseSharingAvoidance, 1) * 0.2) * 100

print("Student's solution obtained " + str(finalPerformancPortionMark) + "% of the total performance portion score")
print("\tSequential Performance Pts: " + str(min(score_multiThreadedSpeedup, 1)*100) + "%")
print("\tMultiThreaded Performance Pts: " + str(min(score_sequentialSpeed, 1)*100) + "%")
print("\tFalse Sharing Pts: " + str(min(score_falseSharingAvoidance, 1)*100) + "%")