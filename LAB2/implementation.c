#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

typedef struct tagPixel {
    int row;
    int column;
    unsigned char R, G, B;
} Pixel;

typedef enum tagState {
    INIT=0, SHIFT=1, ROTATE=2, MIRROR=3
} State;

typedef struct tagIteration {
    unsigned int W, A, S, D, CW, CCW, MX, MY;
} Iteration;

// GLOBAL VARIABLES
/***********************************************************************************************************************/
// width and height of this image, globally visible
unsigned int g_width, g_height;
// buffer of colored pixels
Pixel *pixels;
// number of colored pixels
int numpixels;
// bytes in a row
int rowbyte;
// bytes in a frame
int framebyte;
// BLANK color value
#define BLANK 0xFF
/**********************************************************************************************************************/

// helper functions
//----------------------------------------------------------------------------------------------------------------------
void rotateCW90();
void rotateCCW90();
void rotateCW180();
//----------------------------------------------------------------------------------------------------------------------
/***********************************************************************************************************************/

void processShift(Iteration *iter) {
    int sw = iter->S - iter->W;
    int da = iter->D - iter->A;
    iter->W = 0;
    iter->A = 0;
    iter->S = 0;
    iter->D = 0;
    for (int i = 0; i < numpixels; i++) {
        pixels[i].row += sw;
        pixels[i].column += da;
    }
}

void processRotate(Iteration *iter) {
    int cycle = iter->CW - iter->CCW;
    iter->CW = 0;
    iter->CCW = 0;

    int r;
    if (cycle > 0) {
        r = cycle % 4;
        if (r == 0) {
            return;
        } else if (r == 1) {
            rotateCW90();
        } else if (r == 2) {
            rotateCW180();
        } else if (r == 3) {
            rotateCCW90();
        }
    } else if (cycle < 0) {
        cycle = - cycle;
        r = cycle % 4;
        if (r == 0) {
            return;
        } else if (r == 1) {
            rotateCCW90();
        } else if (r == 2) {
            rotateCW180();
        } else if (r == 3) {
            rotateCW90();
        }
    }
}

void rotateCW90() {
    int tmp;
    for (int i = 0; i < numpixels; i++) {
        tmp = g_width - pixels[i].row - 1;
        pixels[i].row = pixels[i].column;
        pixels[i].column = tmp;
    }
}

void rotateCCW90() {
    int tmp;
    for (int i = 0; i < numpixels; i++) {
        tmp = g_height - pixels[i].column - 1;
        pixels[i].column = pixels[i].row;
        pixels[i].row = tmp;
    }
}

void rotateCW180() {
    for (int i = 0; i < numpixels; i++) {
        pixels[i].row = g_height - pixels[i].row - 1;
        pixels[i].column = g_width - pixels[i].column - 1;
    }
}

void processMirror(Iteration *iter) {
    bool X = ((iter->MX % 2) == 1);
    bool Y = ((iter->MY % 2) == 1);
    iter->MX = 0;
    iter->MY = 0;

    if (X && Y) {
        for (int i = 0; i < numpixels; i++) {
            pixels[i].row = g_height - pixels[i].row - 1;
            pixels[i].column = g_width - pixels[i].column - 1;
        }
    } else if (X) {
        for (int i = 0; i < numpixels; i++) {
            pixels[i].row = g_height - pixels[i].row - 1;
        }
    } else if (Y) {
        for (int i = 0; i < numpixels; i++) {
            pixels[i].column = g_width - pixels[i].column - 1;
        }
    }
}

void process(Iteration *iter, State state)
{
    switch (state) {
        case INIT: {
            break;
        }
        case SHIFT: {
            processShift(iter);
            break;
        }
        case ROTATE: {
            processRotate(iter);
            break;
        }
        case MIRROR: {
            processMirror(iter);
            break;
        }
        default: fprintf(stderr, "processing unknown state: %d\n", state);
    }
}

State state_transfer(const char *key, const int value, Iteration *iter) 
{
    if (!strcmp(key, "W")) {
        if (value > 0) {
            iter->W += value;
            return SHIFT;
        } else if (value < 0) {
            iter->S += -value;
            return SHIFT;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "A")) {
        if (value > 0) {
            iter->A += value;
            return SHIFT;
        } else if (value < 0) {
            iter->D += -value;
            return SHIFT;
        } else {
            return INIT;
        }
    }  else if (!strcmp(key, "S")) {
        if (value > 0) {
            iter->S += value;
            return SHIFT;
        } else if (value < 0) {
            iter->W += -value;
            return SHIFT;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "D")) {
        if (value > 0) {
            iter->D += value;
            return SHIFT;
        } else if (value < 0) {
            iter->A += -value;
            return SHIFT;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "CW")) {
        if (value > 0) {
            iter->CW += value;
            return ROTATE;
        } else if (value < 0) {
            iter->CCW += -value;
            return ROTATE;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "CCW")) {
        if (value > 0) {
            iter->CCW += value;
            return ROTATE;
        } else if (value < 0) { 
            iter->CW += -value;
            return ROTATE;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "MX")) {
        iter->MX += 1;
        return MIRROR;
    } else if (!strcmp(key, "MY")) {
        iter->MY += 1;
        return MIRROR;
    } else {
        printf("Invalid key %s\n", key);
        return -1;
    }
}

void detectPixels(unsigned char *frame_buffer) {
    int idx = 0;
    unsigned char R, G, B;
    while (idx < framebyte) {
        R = frame_buffer[idx];
        G = frame_buffer[idx + 1];
        B = frame_buffer[idx + 2];
        if (R != BLANK || G != BLANK || B != BLANK) {
            pixels[numpixels].R = R;
            pixels[numpixels].G = G;
            pixels[numpixels].B = B;
            pixels[numpixels].row = idx / rowbyte;
            pixels[numpixels].column = (idx % rowbyte) / 3;
            numpixels++;  
        }
        idx += 3;
    }
}

void dumpImage(unsigned char *frame_buffer) {
    int idx, row, column;
    for (int i = 0; i < numpixels; i++) {
        row = pixels[i].row;
        column = pixels[i].column;
        idx = row * rowbyte + column * 3;
        frame_buffer[idx] = pixels[i].R;
        frame_buffer[idx + 1] = pixels[i].G;
        frame_buffer[idx + 2] = pixels[i].B;
    }
}

void clearImage(unsigned char *frame_buffer) {
    int idx, row, column;
    for (int i = 0; i < numpixels; i++) {
        row = pixels[i].row;
        column = pixels[i].column;
        idx = row * rowbyte + column * 3;
        frame_buffer[idx] = BLANK;
        frame_buffer[idx + 1] = BLANK;
        frame_buffer[idx + 2] = BLANK;
    }
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "C99";

    // Please fill in your information
    char student1_first_name[] = "Qianhao";
    char student1_last_name[] = "Zhang";
    char student1_student_number[] = "1004654377";

    // Please fill in your partner's information
    // If yon't have partner, do not modify this
    char student2_first_name[] = "Jingfeng";
    char student2_last_name[] = "Chen";
    char student2_student_number[] = "1000411262";

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
	// assign global variables
	g_width = width;
    g_height = height;
    rowbyte = 3 * width;
    framebyte = height * rowbyte;
    pixels = (Pixel *)malloc(width * height * sizeof(Pixel));
    numpixels = 0;

    detectPixels(frame_buffer);
    clearImage(frame_buffer);

    int processed_frames = 0;
	State state = INIT;
	Iteration iter;
    memset(&iter, 0, sizeof(Iteration));	
    char *key;
    int value;
    State next_state = INIT;
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
        processed_frames += 1;
        key = sensor_values[sensorValueIdx].key;
        value = sensor_values[sensorValueIdx].value;
        next_state = state_transfer(key, value, &iter);
        if (state != INIT && state != next_state) {
            process(&iter, state);
        }
        state = next_state;
        if (processed_frames % 25 == 0) {
            process(&iter, state);
            dumpImage(frame_buffer);
            verifyFrame(frame_buffer, width, height, grading_mode);
            if (sensorValueIdx + 1 != sensor_values_count) {
                clearImage(frame_buffer);
            }
        }
    }
    if (processed_frames % 25) {
        process(&iter, state);
        dumpImage(frame_buffer);
    }
    free(pixels);
    //printBMP(width, height, frame_buffer);
    return;
}
