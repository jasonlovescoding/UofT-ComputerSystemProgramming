#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

// GLOBAL VARIABLES
/***********************************************************************************************************************/
// global temporary image buffer
unsigned char *rendered_frame;
// width and height of this image, globally visible
unsigned int g_width, g_height;
/**********************************************************************************************************************/

// FUNCTION DECLARATIONS
/***********************************************************************************************************************/
unsigned char *processMoveUp(unsigned char *buffer_frame, int offset);
unsigned char *processMoveDown(unsigned char *buffer_frame, int offset);
unsigned char *processMoveLeft(unsigned char *buffer_frame, int offset);
unsigned char *processMoveRight(unsigned char *buffer_frame, int offset);
unsigned char *processRotateCCW(unsigned char *buffer_frame, int rotate_iteration);
unsigned char *processRotateCW(unsigned char *buffer_frame, int rotate_iteration);
unsigned char *processMirrorX(unsigned char *buffer_frame, int _unused);
unsigned char *processMirrorY(unsigned char *buffer_frame, int _unused);
// helper functions
//----------------------------------------------------------------------------------------------------------------------
unsigned char *rotateCW90(unsigned char *buffer_frame);
unsigned char *rotateCCW90(unsigned char *buffer_frame);
unsigned char *rotateCW180(unsigned char *buffer_frame);
//----------------------------------------------------------------------------------------------------------------------
/***********************************************************************************************************************/


/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveUp(unsigned char *buffer_frame, int offset) {
	// return processMoveUpReference(buffer_frame, width, height, offset);

    // handle negative offsets
    if (offset < 0){
        return processMoveDown(buffer_frame, -offset);
    }  

    // store shifted pixels to temporary buffer
    for (int row = 0; row < (g_height - offset); row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            int position_buffer_frame = (row + offset) * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // fill left over pixels with white pixels
    for (int row = (g_height - offset); row < g_height; row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = 255;
            rendered_frame[position_rendered_frame + 1] = 255;
            rendered_frame[position_rendered_frame + 2] = 255;
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image left
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveRight(unsigned char *buffer_frame, int offset) {
    // return processMoveRightReference(buffer_frame, width, height, offset);

	// handle negative offsets
    if (offset < 0){
        return processMoveLeft(buffer_frame, -offset);
    }

    // store shifted pixels to temporary buffer
    for (int row = 0; row < g_height; row++) {
        for (int column = offset; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            int position_buffer_frame = row * g_width * 3 + (column - offset) * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // fill left over pixels with white pixels
    for (int row = 0; row < g_height; row++) {
        for (int column = 0; column < offset; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = 255;
            rendered_frame[position_rendered_frame + 1] = 255;
            rendered_frame[position_rendered_frame + 2] = 255;
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image up
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveDown(unsigned char *buffer_frame, int offset) {
    // return processMoveDownReference(buffer_frame, width, height, offset);

	// handle negative offsets
    if (offset < 0){
        return processMoveUp(buffer_frame, -offset);
    }

    // store shifted pixels to temporary buffer
    for (int row = offset; row < g_height; row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            int position_buffer_frame = (row - offset) * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // fill left over pixels with white pixels
    for (int row = 0; row < offset; row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = 255;
            rendered_frame[position_rendered_frame + 1] = 255;
            rendered_frame[position_rendered_frame + 2] = 255;
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param offset - number of pixels to shift the object in bitmap image right
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note1: White pixels RGB(255,255,255) are treated as background. Object in the image refers to non-white pixels.
 * Note2: You can assume the object will never be moved off the screen
 **********************************************************************************************************************/
unsigned char *processMoveLeft(unsigned char *buffer_frame, int offset) {
    // handle negative offsets
    if (offset < 0){
        return processMoveRight(buffer_frame, -offset);
    }

    // store shifted pixels to temporary buffer
    for (int row = 0; row < g_height; row++) {
        for (int column = 0; column < (g_width - offset); column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            int position_buffer_frame = row * g_width * 3 + (column + offset) * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // fill left over pixels with white pixels
    for (int row = 0; row < g_height; row++) {
        for (int column = g_width - offset; column < g_width; column++) {
            int position_rendered_frame = row * g_width * 3 + column * 3;
            rendered_frame[position_rendered_frame] = 255;
            rendered_frame[position_rendered_frame + 1] = 255;
            rendered_frame[position_rendered_frame + 2] = 255;
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

unsigned char *rotateCW90(unsigned char *buffer_frame) {
    // store shifted pixels to temporary buffer
    int render_column = g_width - 1;
    int render_row = 0;
    for (int row = 0; row < g_width; row++) {
        for (int column = 0; column < g_height; column++) {
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
            render_row += 1;
        }
        render_row = 0;
        render_column -= 1;
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

unsigned char *rotateCCW90(unsigned char *buffer_frame) {
	// TODO: to be verified
	// store shifted pixels to temporary buffer
    int render_column = 0;
    int render_row = g_height - 1;
    for (int row = 0; row < g_width; row++) {
        for (int column = 0; column < g_height; column++) {
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
            render_row -= 1;
        }
        render_row = g_height - 1;
        render_column += 1;
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

unsigned char *rotateCW180(unsigned char *buffer_frame) {
	// TODO: to be verified
	int render_column = g_width - 1;
    int render_row = g_height - 1;
    for (int row = 0; row < g_width; row++) {
        for (int column = 0; column < g_height; column++) {
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
            render_column -= 1;
        }
        render_row -= 1;
        render_column = g_width - 1;
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
unsigned char *processRotateCW(unsigned char *buffer_frame, 
                               int rotate_iteration) {
	// return processRotateCWReference(buffer_frame, width, height, rotate_iteration);

    if (rotate_iteration < 0){ // handle negative offsets
        return processRotateCCW(buffer_frame, -rotate_iteration);
    } else if (rotate_iteration % 4 == 0) {
		return buffer_frame;	
	} else if (rotate_iteration % 4 == 1) {
		return rotateCW90(buffer_frame);
	} else if (rotate_iteration % 4 == 2) {
		return rotateCW180(buffer_frame);
	} else {
		return rotateCCW90(buffer_frame);		
	}
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer counter clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
unsigned char *processRotateCCW(unsigned char *buffer_frame, 
                                int rotate_iteration) {
    // return processRotateCCWReference(buffer_frame, width, height, rotate_iteration);

    if (rotate_iteration < 0){ // handle negative offsets
        return processRotateCW(buffer_frame, -rotate_iteration);
    } else if (rotate_iteration % 4 == 0) {
		return buffer_frame;	
	} else if (rotate_iteration % 4 == 1) {
		return rotateCCW90(buffer_frame);
	} else if (rotate_iteration % 4 == 2) {
		return rotateCW180(buffer_frame);
	} else {
		return rotateCW90(buffer_frame);		
	}
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorX(unsigned char *buffer_frame, int _unused) {
    // return processMirrorXReference(buffer_frame, width, height, _unused);

    // store shifted pixels to temporary buffer
    for (int row = 0; row < g_height; row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_height * 3 + column * 3;
            int position_buffer_frame = (g_height - row - 1) * g_height * 3 + column * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
unsigned char *processMirrorY(unsigned char *buffer_frame, int _unused) {
    // return processMirrorYReference(buffer_frame, width, height, _unused);

    // store shifted pixels to temporary buffer
    for (int row = 0; row < g_height; row++) {
        for (int column = 0; column < g_width; column++) {
            int position_rendered_frame = row * g_height * 3 + column * 3;
            int position_buffer_frame = row * g_height * 3 + (g_width - column - 1) * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }

    // copy the temporary buffer back to original frame buffer
    buffer_frame = copyFrame(rendered_frame, buffer_frame, g_width, g_height);

    // return a pointer to the updated image buffer
    return buffer_frame;
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "C99";

    // Please fill in your information
    char student1_first_name[] = "ZHANG";
    char student1_last_name[] = "Qianhao";
    char student1_student_number[] = "1004654377";

    // Please fill in your partner's information
    // If yon't have partner, do not modify this
    char student2_first_name[] = "joe";
    char student2_last_name[] = "doe";
    char student2_student_number[] = "0000000001";

    // Printing out team information
    printf("*******************************************************************************************************\n");
    printf("Team Information:\n");
    printf("\tteam_name: %s\n", team_name);
    printf("\tstudent1_first_name: %s\n", student1_first_name);
    printf("\tstudent1_last_name: %s\n", student1_last_name);
    printf("\tstudent1_student_number: %s\n", student1_student_number);
    printf("\tstudent2_first_name: %s\n", student2_first_name);
    printf("\tstudent2_last_name: %s\n", student2_last_name);
    printf("\tstudent2_student_number: %s\n", student2_student_number);
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          You can modify anything else in this file
 ***********************************************************************************************************************
 * @param sensor_values - structure stores parsed key value pairs of program instructions
 * @param sensor_values_count - number of valid sensor values parsed from sensor log file or commandline console
 * @param frame_buffer - pointer pointing to a buffer storing the imported  24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param grading_mode - turns off verification and turn on instrumentation
 ***********************************************************************************************************************
 *
 **********************************************************************************************************************/
void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
    int processed_frames = 0;
	// assign global variables
	g_width = width;
	g_height = height;

    // allocate memory for temporary image buffer
    rendered_frame = allocateFrame(width, height);

    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
//        printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
//               sensor_values[sensorValueIdx].value);
        if (!strcmp(sensor_values[sensorValueIdx].key, "W")) {
            frame_buffer = processMoveUp(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "A")) {
            frame_buffer = processMoveLeft(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "S")) {
            frame_buffer = processMoveDown(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "D")) {
            frame_buffer = processMoveRight(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CW")) {
            frame_buffer = processRotateCW(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "CCW")) {
            frame_buffer = processRotateCCW(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MX")) {
            frame_buffer = processMirrorX(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        } else if (!strcmp(sensor_values[sensorValueIdx].key, "MY")) {
            frame_buffer = processMirrorY(frame_buffer, sensor_values[sensorValueIdx].value);
//            printBMP(width, height, frame_buffer);
        }
        processed_frames += 1;
        //if (processed_frames % 25 == 0) {
		if (processed_frames % 5 == 0) {
            verifyFrame(frame_buffer, width, height, grading_mode);
        }
    }
    // free temporary image buffer
    deallocateFrame(rendered_frame);
    return;
}
