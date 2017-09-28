#ifndef ECE454_LAB2_UTILITIES_H
#define ECE454_LAB2_UTILITIES_H

/***********************************************************************************************************************
 * Warning: DO NOT MODIFY or SUBMIT this file
 **********************************************************************************************************************/

struct kv {
    char *key;
    int value;
};

void printBMP(unsigned width, unsigned height, unsigned char *frame_buffer);
unsigned char* allocateFrame(unsigned width, unsigned height);
void deallocateFrame(unsigned char *frame);
unsigned char* copyFrame(unsigned char* src, unsigned char* dst, unsigned width, unsigned height);
void recordFrame(unsigned char *frame_buffer, unsigned int width, unsigned int height, bool grading_mode);
void verifyFrame(unsigned char *frame_buffer, unsigned int width, unsigned int height, bool grading_mode);
void verifiedAllFrames();
void verifiedAllFramesGrading();
#endif //ECE454_LAB2_UTILITIES_H
