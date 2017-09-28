#ifndef ECE454_LAB2_IMPLEMENTATION_REFERENCE_H
#define ECE454_LAB2_IMPLEMENTATION_REFERENCE_H

/***********************************************************************************************************************
 * Warning: DO NOT MODIFY or SUBMIT this file
 **********************************************************************************************************************/

void implementation_driver_reference(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                                     unsigned int width, unsigned int height, bool grading_mode);

unsigned char *processMoveUpReference(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processMoveRightReference(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processMoveDownReference(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processMoveLeftReference(unsigned char *buffer_frame, unsigned width, unsigned height, int offset);

unsigned char *processRotateCWReference(unsigned char *buffer_frame, unsigned width, unsigned height,
                                        int rotate_iteration);

unsigned char *processRotateCCWReference(unsigned char *buffer_frame, unsigned width, unsigned height,
                                         int rotate_iteration);

unsigned char *
processMirrorXReference(unsigned char *buffer_frame, unsigned int width, unsigned int height, int _unused);

unsigned char *processMirrorYReference(unsigned char *buffer_frame, unsigned width, unsigned height, int _unused);

#endif //ECE454_LAB2_IMPLEMENTATION_REFERENCE_H
