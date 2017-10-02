#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utilities.h"

/***********************************************************************************************************************
 * Warning: DO NOT MODIFY or SUBMIT this file
 **********************************************************************************************************************/

void printBMP(unsigned width, unsigned height, unsigned char *frame_buffer) {
    int row;
    int column;
    int location;

    printf("*******************************************************************************************************\n");
    for (row = 0; row < height; row++) {
        for (column = 0; column < width; column++) {
            location = ((width * 3) * row) + (column * 3);
            printf("[%03d,%03d,%03d]", frame_buffer[location], frame_buffer[location + 1], frame_buffer[location + 2]);
        }
        printf("\n");
    }
    printf("*******************************************************************************************************\n");
}

// Frame Utilities
unsigned char* allocateFrame(unsigned width, unsigned height){
    return (unsigned char*)malloc((width * height * 3) * sizeof(char));
}

void deallocateFrame(unsigned char *frame){
    free(frame);
    return;
}

// Copy content from one frame to another
unsigned char* copyFrame(unsigned char* src, unsigned char* dst, unsigned width, unsigned height) {
    // copy the rendered image back to original frame buffer (pixel)
    for(int row = 0; row < height; row++){
        for(int column = 0; column < width; column++){
            int position = row * width * 3 + column * 3;
            dst[position] = src[position];
            dst[position + 1] = src[position + 1];
            dst[position + 2] = src[position + 2];
        }
    }
    return dst;
}

// Variables used by recordFrame and verifyFrame
unsigned char *recorded_frames[1000];
unsigned int recorded_frames_count = 0;
unsigned int verified_frames_count = 0;

// Each time recordFrame is called, it makes a deep copy of the buffer passed in and store it in recorded_frames
void recordFrame(unsigned char *frame_buffer, unsigned int width, unsigned int height, bool grading_mode) {
    if (grading_mode) {
        recorded_frames_count++;
        return;
    } else {
        recorded_frames[recorded_frames_count] = allocateFrame(width, height);
        copyFrame(frame_buffer, recorded_frames[recorded_frames_count], width, height);
        recorded_frames_count++;
    }

}

// Each time verifyFrame is called, it verifies if the buffer passed in is the same as the corresponding one in recorded_frames
// If it is different, print out the frame number and exit immediately.
void verifyFrame(unsigned char *frame_buffer, unsigned int width, unsigned int height, bool grading_mode) {
    if (grading_mode) {
        verified_frames_count++;
        return;
    } else {
        // verify frame, memcmp returns 0 if buffer is the same, -1 or +1 when buffers are different
        if (memcmp(frame_buffer, recorded_frames[verified_frames_count], width * height * 3)){
            printf("ERROR: frame #%d is different compared to the reference implementation\n", verified_frames_count);
            exit (-1);
        } else {
            printf("SUCCESS: frame #%d is the same compared to the reference implementation\n", verified_frames_count);
        }
        verified_frames_count++;
        if (verified_frames_count > recorded_frames_count){
            printf("ERROR: frame #%d in your application does not exist in the reference solution.\n",
                   (verified_frames_count - 1));
            exit (-1);
        }
    }
}

// Verify that all frames has been verified
void verifiedAllFrames(){
    if (verified_frames_count < recorded_frames_count){
        printf("ERROR: your implementation only contains %u out of %u required frames.\n",
               verified_frames_count, recorded_frames_count);

    }
}

// This is slightly redundant, but nevertheless helpful to catch cheaters
void verifiedAllFramesGrading(){
    if (!recorded_frames_count && !verified_frames_count){
        printf("ERROR: your implementation did not output the same number of required frames as reference implementation");
        exit (-1);
    } else {
        // reset counters for verification use
        recorded_frames_count = 0;
        verified_frames_count = 0;
    }
}