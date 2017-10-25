#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

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
int *ROW;
int *COLUMN;
unsigned char *R;
unsigned char *G;
unsigned char *B;
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

    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        ROW[i] += sw;
        ROW[i + 1] += sw;
        ROW[i + 2] += sw;
        ROW[i + 3] += sw;

        COLUMN[i] += da;  
        COLUMN[i + 1] += da; 
        COLUMN[i + 2] += da; 
        COLUMN[i + 3] += da; 
    }

    for (; i < numpixels; i++) {
        ROW[i] += sw;
        COLUMN[i] += da;  
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
    int tmp0, tmp1, tmp2, tmp3;
    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        tmp0 = g_width - ROW[i] - 1;
        tmp1 = g_width - ROW[i + 1] - 1;
        tmp2 = g_width - ROW[i + 2] - 1;
        tmp3 = g_width - ROW[i + 3] - 1;

        ROW[i] = COLUMN[i];
        ROW[i + 1] = COLUMN[i + 1];
        ROW[i + 2] = COLUMN[i + 2];
        ROW[i + 3] = COLUMN[i + 3];

        COLUMN[i] = tmp0;
        COLUMN[i + 1] = tmp1;
        COLUMN[i + 2] = tmp2;
        COLUMN[i + 3] = tmp3;
    }

    for (; i < numpixels; i++) {
        tmp0 = g_width - ROW[i] - 1;
        ROW[i] = COLUMN[i];
        COLUMN[i] = tmp0;
    }
}

void rotateCCW90() {
    int tmp0, tmp1, tmp2, tmp3;
    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        tmp0 = g_height - COLUMN[i] - 1;
        tmp1 = g_height - COLUMN[i + 1] - 1;
        tmp2 = g_height - COLUMN[i + 2] - 1;
        tmp3 = g_height - COLUMN[i + 3] - 1;

        COLUMN[i] = ROW[i];
        COLUMN[i + 1] = ROW[i + 1];
        COLUMN[i + 2] = ROW[i + 2];
        COLUMN[i + 3] = ROW[i + 3];

        ROW[i] = tmp0;
        ROW[i + 1] = tmp1;
        ROW[i + 2] = tmp2;
        ROW[i + 3] = tmp3;
    }

    for (; i < numpixels; i++) {
        tmp0 = g_height - COLUMN[i] - 1;
        COLUMN[i] = ROW[i];
        ROW[i] = tmp0;
    }
}

void rotateCW180() {
    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        ROW[i] = g_height - ROW[i] - 1;
        ROW[i + 1] = g_height - ROW[i + 1] - 1;
        ROW[i + 2] = g_height - ROW[i + 2] - 1;
        ROW[i + 3] = g_height - ROW[i + 3] - 1;

        COLUMN[i] = g_width - COLUMN[i] - 1;
        COLUMN[i + 1] = g_width - COLUMN[i + 1] - 1;
        COLUMN[i + 2] = g_width - COLUMN[i + 2] - 1;
        COLUMN[i + 3] = g_width - COLUMN[i + 3] - 1;
    }

    for (; i < numpixels; i++) {
        ROW[i] = g_height - ROW[i] - 1;
        COLUMN[i] = g_width - COLUMN[i] - 1;
    }
}

void processMirror(Iteration *iter) {
    bool X = ((iter->MX % 2) == 1);
    bool Y = ((iter->MY % 2) == 1);
    iter->MX = 0;
    iter->MY = 0;

    int i = 0;
    if (X && Y) {
        for (; i + 4 < numpixels; i+=4) {
            ROW[i] = g_height - ROW[i] - 1;
            ROW[i + 1] = g_height - ROW[i + 1] - 1;
            ROW[i + 2] = g_height - ROW[i + 2] - 1;
            ROW[i + 3] = g_height - ROW[i + 3] - 1;
            
            COLUMN[i] = g_width - COLUMN[i] - 1;
            COLUMN[i + 1] = g_width - COLUMN[i + 1] - 1;
            COLUMN[i + 2] = g_width - COLUMN[i + 2] - 1;
            COLUMN[i + 3] = g_width - COLUMN[i + 3] - 1;
        }
        for (; i < numpixels; i++) {
            ROW[i] = g_height - ROW[i] - 1;
            COLUMN[i] = g_width - COLUMN[i] - 1;
        }
    } else if (X) {
        for (; i + 4 < numpixels; i+=4) {
            ROW[i] = g_height - ROW[i] - 1;
            ROW[i + 1] = g_height - ROW[i + 1] - 1;
            ROW[i + 2] = g_height - ROW[i + 2] - 1;
            ROW[i + 3] = g_height - ROW[i + 3] - 1;
        }
        for (; i < numpixels; i++) {
            ROW[i] = g_height - ROW[i] - 1;
        }
    } else if (Y) {
        for (; i + 4 < numpixels; i+=4) {
            COLUMN[i] = g_width - COLUMN[i] - 1;
            COLUMN[i + 1] = g_width - COLUMN[i + 1] - 1;
            COLUMN[i + 2] = g_width - COLUMN[i + 2] - 1;
            COLUMN[i + 3] = g_width - COLUMN[i + 3] - 1;
        }
        for (; i < numpixels; i++) {
            COLUMN[i] = g_width - COLUMN[i] - 1;
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
    unsigned char r, g, b;
    int idx = 0;
    int scanbyte = 2 * rowbyte;
    unsigned char *BLANK_FRAME = (unsigned char *) malloc(scanbyte);
    memset(BLANK_FRAME, BLANK, scanbyte);
    while (idx < framebyte) {
        while (idx + scanbyte < framebyte && memcmp(frame_buffer + idx, BLANK_FRAME, scanbyte) == 0) {
            idx += scanbyte;
        }
        int bound = (idx + scanbyte) < framebyte ? (idx + scanbyte) : framebyte;
        for (; idx < bound; idx+=3) {
            r = frame_buffer[idx];
            g = frame_buffer[idx + 1];
            b = frame_buffer[idx + 2];
            if (r != BLANK || g != BLANK || b != BLANK) {
                R[numpixels] = r;
                G[numpixels] = g;
                B[numpixels] = b;
                ROW[numpixels] = idx / rowbyte;
                COLUMN[numpixels] = (idx % rowbyte) / 3;
                numpixels++;  
            }
        }
    }
    free(BLANK_FRAME);
}

void dumpImage(unsigned char *frame_buffer) {
    int idx0, idx1, idx2, idx3;
    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        idx0 = ROW[i] * rowbyte + COLUMN[i] * 3;
        idx1 = ROW[i + 1] * rowbyte + COLUMN[i + 1] * 3;
        idx2 = ROW[i + 2] * rowbyte + COLUMN[i + 2] * 3;
        idx3 = ROW[i + 3] * rowbyte + COLUMN[i + 3] * 3;

        frame_buffer[idx0] = R[i];
        frame_buffer[idx1] = R[i + 1];
        frame_buffer[idx2] = R[i + 2];
        frame_buffer[idx3] = R[i + 3];
 
        frame_buffer[idx0 + 1] = G[i];
        frame_buffer[idx1 + 1] = G[i + 1];
        frame_buffer[idx2 + 1] = G[i + 2];
        frame_buffer[idx3 + 1] = G[i + 3];
 
        frame_buffer[idx0 + 2] = B[i];
        frame_buffer[idx1 + 2] = B[i + 1];
        frame_buffer[idx2 + 2] = B[i + 2];
        frame_buffer[idx3 + 2] = B[i + 3];
    }

    for (; i < numpixels; i++) {
        idx0 = ROW[i] * rowbyte + COLUMN[i] * 3;
        frame_buffer[idx0] = R[i];
        frame_buffer[idx0 + 1] = G[i];
        frame_buffer[idx0 + 2] = B[i];
    }
}

void clearImage(unsigned char *frame_buffer) {
    int idx0, idx1, idx2, idx3;
    int i = 0;
    for (; i + 4 < numpixels; i+=4) {
        idx0 = ROW[i] * rowbyte + COLUMN[i] * 3;
        idx1 = ROW[i + 1] * rowbyte + COLUMN[i + 1] * 3;
        idx2 = ROW[i + 2] * rowbyte + COLUMN[i + 2] * 3;
        idx3 = ROW[i + 3] * rowbyte + COLUMN[i + 3] * 3;

        frame_buffer[idx0] = BLANK;
        frame_buffer[idx1] = BLANK;
        frame_buffer[idx2] = BLANK;
        frame_buffer[idx3] = BLANK;
 
        frame_buffer[idx0 + 1] = BLANK;
        frame_buffer[idx1 + 1] = BLANK;
        frame_buffer[idx2 + 1] = BLANK;
        frame_buffer[idx3 + 1] = BLANK;
 
        frame_buffer[idx0 + 2] = BLANK;
        frame_buffer[idx1 + 2] = BLANK;
        frame_buffer[idx2 + 2] = BLANK;
        frame_buffer[idx3 + 2] = BLANK;
    }

    for (; i < numpixels; i++) {
        idx0 = ROW[i] * rowbyte + COLUMN[i] * 3;
        frame_buffer[idx0] = BLANK;
        frame_buffer[idx0 + 1] = BLANK;
        frame_buffer[idx0 + 2] = BLANK;
    }
}

/***********************************************************************************************************************
 * WARNING: Do not modify the implementation_driver and team info prototype (name, parameter, return value) !!!
 *          Do not forget to modify the team_name and team member information !!!
 **********************************************************************************************************************/
void print_team_info(){
    // Please modify this field with something interesting
    char team_name[] = "###";

    // Please fill in your information
    char student1_first_name[] = "";
    char student1_last_name[] = "";
    char student1_student_number[] = "";

    // Please fill in your partner's information
    // If yon't have partner, do not modify this
    char student2_first_name[] = "";
    char student2_last_name[] = "";
    char student2_student_number[] = "";

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
    ROW = (int *)malloc(width * height * sizeof(int));
    COLUMN = (int *)malloc(width * height * sizeof(int));
    R = (unsigned char *)malloc(width * height);
    G = (unsigned char *)malloc(width * height);
    B = (unsigned char *)malloc(width * height);
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
    free(ROW);
    free(COLUMN);
    free(R);
    free(G);
    free(B);
    //printBMP(width, height, frame_buffer);
    return;
}
