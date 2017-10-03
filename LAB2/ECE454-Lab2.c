#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <stdbool.h>

// Bitmap Image Library
#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"

// Lab2 Common Libraries
#include "utilities.h"

// Lab2 Benchmark Library
#include "fcyc.h"

// Lab2 Reference Implementation
#include "implementation_reference.h"

// Lab2 Your Implementation
#include "implementation.h"

// save compiler switches to disable pointer casting compilation warnings
// this part of the code is only needed to maintain instrumentation compatibility
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

void implementation_driver_reference_wraper(void *argv[]){
    struct kv *sensor_values    = (struct kv *)     argv[0];
    int sensor_values_count     = (int)             argv[1];
    unsigned char *frame_buffer = (unsigned char *) argv[2];
    int width                   = (unsigned int)    argv[3];
    int height                  = (unsigned int)    argv[4];
    bool grading_mode           = (bool)            argv[4];

    // Provide fresh copy of input frame buffer
    unsigned char *frame_copy = allocateFrame(width, height);
    frame_copy = copyFrame(frame_buffer, frame_copy, width, height);

    implementation_driver_reference(sensor_values, sensor_values_count,
                                    frame_copy, width, height,
                                    grading_mode);

    deallocateFrame(frame_copy);
}

void implementation_driver_wraper(void *argv[]){
    struct kv *sensor_values    = (struct kv *)     argv[0];
    int sensor_values_count     = (int)             argv[1];
    unsigned char *frame_buffer = (unsigned char *) argv[2];
    int width                   = (unsigned int)    argv[3];
    int height                  = (unsigned int)    argv[4];
    bool grading_mode           = (bool)            argv[4];

    // Provide fresh copy of input frame buffer
    unsigned char *frame_copy = allocateFrame(width, height);
    frame_copy = copyFrame(frame_buffer, frame_copy, width, height);

    implementation_driver(sensor_values, sensor_values_count,
                          frame_copy, width, height,
                          grading_mode);

    deallocateFrame(frame_copy);
}
// restore compiler switches
#pragma GCC diagnostic pop

/***********************************************************************************************************************
 * Warning: DO NOT MODIFY or SUBMIT this file
 **********************************************************************************************************************/
int main(int argc, char **argv) {
    /*******************************************************************************************************************
     * Below are the five input parameters which you care about for your lab2 implementation:
     *******************************************************************************************************************
     * 1) frame_buffer: This pointer pointing to a buffer storing the imported  24-bit bitmap image
     *      - This is a 1D array representing 24bit bitmap pixels.
     *      - Each element of the array is 8-bit char* which contains the value(0-255) of a sub-pixel (Red, Green, Blue)
     *      - frame_buffer elements are organized as groups of sub-pixels: frame_buffer = [R][G][B][R][G][B][R][G][B]...
     * 2) width: width of the imported 24-bit bitmap image
     * 3) height: height of the imported 24-bit bitmap image
     * 4) sensor_values_count: number of valid sensor values parsed from sensor log file or commandline console
     * 4) sensor_values: this structure stores parsed key value pairs of program instructions
     *      a) <W,offset> - move object inside frame buffer up by <offset> pixels
     *      b) <A,offset> - move object inside frame buffer left by <offset> pixels
     *      c) <S,offset> - move object inside frame buffer down by <offset> pixels
     *      d) <D,offset> - move object inside frame buffer right by <offset> pixels
     *      e) <CW,iteration> - rotate object inside frame buffer clockwise by 90 degrees, <iteration> times
     *          - i.e. <CW,3> refers to rotating clockwise by 270 degrees or counter clockwise by 90 degrees
     *      f) <CCW, iteration> - rotate object inside frame buffer counter clockwise by 90 degrees, <iteration> times
     *      g) <MX,_unused> - mirror the frame buffer on the X axis
     *          - i.e. after rendering, first row of pixels becomes the last row of pixels
     *      h) <MY,_unused> - mirror the frame buffer on the Y axis
     *          - i.e. after rendering, first column of pixels becomes the last column of pixels
     * 5) grading_mode: passing this parameter in your lab2 will turn off the verification and turn on instrumentation
     ******************************************************************************************************************/
    unsigned char *frame_buffer = NULL;
    unsigned int width, height;
    int sensor_values_count = 0;
    struct kv sensor_values[10240];
    bool grading_mode = false;

    /*******************************************************************************************************************
     * Commandline options parsing script:
     *******************************************************************************************************************
     * Usage: ./ECE454-Lab2 [options]
     *
     * Mandatory Options:
     * -f [file path]   relative or absolute file path of the sensor value input csv file
     *                  - each line of the csv file is in the specific format: <key,value>\n
     *                  - note: you may also input csv values via console input using "-c [comma separate k & v]"
     *                      - you may only use either -f or -c flag, using both will result with an error
     *
     * -c [comma separate k & v]    console input for sensor value (used for quick testing purpose)
     *                              - note: you may also specify a sensor value input file, see -f flag
     *
     * -i [file path]   relative or absolute file path of the initial bitmap image file to load into frame buffer
     *                  - note: only use bitmap files with OS/2 bitmap headers
     *                      - on windows, use paint to create and save as 24bit bitmap image
     *                      - on linux, use GIMP and save as 24bit G8 R8 B8 format with color space information disabled
     *
     * Optional Options:
     * -g               grading mode
     *                  - Using this mode runs your lab2 implementation for 2 times.
     *                      1) first iteration runs your implementation and verifies implementation results
     *                      2) second iteration turns on the instrumentation and turns off the the verifcation
     *                  - Any attempt to tamper or bypass verification or instrumentation mechanism will result
     *                    a "0%" lab2 grade assigned by default if found. i.e. do not design your implementation
     *                    such that program takes two different paths when grading option is toggled.
     ******************************************************************************************************************/
    int fflag = 0, cflag = 0, iflag = 0, rflag = 0, option;

    while ((option = getopt(argc, argv, "c:f:i:rg")) != -1)
        switch (option) {
            case 'c':   // import sensor log from console argument input
                if (fflag == 1) {
                    fprintf(stderr, "Cannot import sensor log from file and console argument at the same time");
                    return 1;
                } else {
                    cflag = 1;
                    printf("Loading input sensor input from console argument: %s\n", optarg);

                    // obtain the first key value pair
                    sensor_values[sensor_values_count].key = strtok(optarg, ",\n");
                    sensor_values[sensor_values_count].value = atoi(strtok(NULL, ",\n"));

                    // obtain more kv pair if there is more
                    while (sensor_values[sensor_values_count].key != NULL) {
                        sensor_values_count += 1;
                        sensor_values[sensor_values_count].key = strtok(NULL, ",\n");
                        sensor_values[sensor_values_count].value = atoi(strtok(NULL, ",\n"));
                    }
                }
                break;
            case 'f':   // import sensor logs from file
                if (cflag == 1) {
                    fprintf(stderr, "Cannot import sensor log from file and console argument at the same time");
                    return 1;
                } else {
                    fflag = 1;
                    printf("Loading input sensor input from file: %s\n", optarg);
                    FILE *fp = fopen(optarg, "r");
                    if (fp != NULL) {
                        char line[128];     // each line of the sensor input file can only be 128 characters long
                        char *token = NULL;
                        while (fgets(line, sizeof line, fp) != NULL) {
                            token = strtok(line, ",\n");
                            sensor_values[sensor_values_count].key = (char *) malloc(
                                    (strlen(token) + 1) * sizeof(char));
                            strcpy(sensor_values[sensor_values_count].key, token);
                            token = strtok(NULL, ",\n");
                            sensor_values[sensor_values_count].value = atoi(token);
                            sensor_values_count += 1;
                        }
                        fclose(fp);
                    } else {
                        fprintf(stderr, "Error: Cannot open sensor input file: %s\n", optarg);
                        fclose(fp);
                        return 1;
                    }
                }
                break;
            case 'i':   // initial 2D object bmp image location
                iflag = 1;
                printf("Loading initial 2D object bmp image from file: %s\n", optarg);

                unsigned int err = loadbmp_decode_file(optarg, &frame_buffer, &width, &height, LOADBMP_RGB);
                if (err)
                    fprintf(stderr, "LoadBMP Load Error: %u\n", err);
                break;
            case 'r':   // run reference implementation
                rflag = 1;
                break;
            case 'g':   // grading mode
                grading_mode = true;
                break;
            case '?':
                if (optopt == 'f' || optopt == 'c' || optopt == 'i') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                    return (-1);
                } else if (isprint (optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }
    if (!(fflag || cflag)) {
        fprintf(stderr, "Must import sensor log data either from file or console argument\n");
    }
    if (!iflag) {
        fprintf(stderr, "Must import initial 2d object bitmap image from file via console argument\n");
    }

    // Interface to your lab 2 implementation
    print_team_info();


    printf("*******************************************************************************************************\n");
//    printf("initial bmp\n");
//    printBMP(width, height, frame_buffer);

    // Measure the performance of student's solution
    unsigned char *frame_copy = allocateFrame(width, height);
    if (grading_mode) {
        printf("Performance Results: \n");
        // note: due to the way the fcyc benchmark utility is designed, it only takes an array as parameter input.
        // We created a warper function which takes an array as input, then calls the appropriate method.
        // In this case, we do not need to modify the instrumentation library.

        // save compiler switches to disable pointer casting compilation warnings
        // this part of the code is only needed to maintain instrumentation compatibility
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

        // start instrumentation
        // Prepare array of parameter input
        void *arglist[6];
        bool boolVar = true;
        arglist[0] = (void *)sensor_values;
        arglist[1] = (void *)sensor_values_count;
        arglist[2] = (void *)frame_copy;
        arglist[3] = (void *)width;
        arglist[4] = (void *)height;
        arglist[5] = (void *)boolVar;

        // restore compiler switches
        #pragma GCC diagnostic pop

        // Provide fresh copy of input frame buffer
        copyFrame(frame_buffer, frame_copy, width, height);
        // Instrument implementation_driver_reference()
        set_fcyc_cache_size(1 << 23);   // 8MB cache size - Core i7 4970 L3 Cache Size
        set_fcyc_clear_cache(1);        // clear the cache before each measurement
        set_fcyc_compensate(1);         // try to compensate for timer overhead

        double num_cycles_reference = fcyc_v((test_funct_v)&implementation_driver_reference_wraper, arglist);
        printf("\tNumber of cpu cycles consumed by the reference implementation: %.0f\n", num_cycles_reference);

        // Provide fresh copy of input frame buffer
        copyFrame(frame_buffer, frame_copy, width, height);
        // Instrument implementation_driver()
        set_fcyc_cache_size(1 << 23);   // 8MB cache size - Core i7 4970 L3 Cache Size
        set_fcyc_clear_cache(1);        // clear the cache before each measurement
        set_fcyc_compensate(1);         // try to compensate for timer overhead
        double num_cycles_optimized = fcyc_v((test_funct_v)&implementation_driver_wraper, arglist);
        printf("\tNumber of cpu cycles consumed by your implementation: %.0f\n", num_cycles_optimized);

        // End instrumentation
        verifiedAllFramesGrading();

        // Compute simple speedup statistics - round speedup to the nearest integer
        printf("\tOptimization Speedup Ratio (nearest integer): %d\n", (int)((num_cycles_reference / num_cycles_optimized) + 0.5));
        printf("*******************************************************************************************************\n");
    }

    // Check the correctness of student's solution
    copyFrame(frame_buffer, frame_copy, width, height);
    implementation_driver_reference(sensor_values, sensor_values_count, frame_copy, width, height, false);
    copyFrame(frame_buffer, frame_copy, width, height);
    implementation_driver(sensor_values, sensor_values_count, frame_copy, width, height, false);
    verifiedAllFrames();

    return 0;
}
