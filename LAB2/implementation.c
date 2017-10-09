#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

// GLOBAL VARIABLES
/***********************************************************************************************************************/
// width and height of this image, globally visible
unsigned int g_width, g_height;
// global index for double-buffering
int g_rendered = 0;
// the first non-blank pixel of this image
int g_start;
// the last non-blank pixel of this image
int g_end;
// the upper-right corner of this image
int g_upperright;
// the lower-left corner of this image
int g_lowerleft;
#define BLANK 0xFF
#ifndef MAX
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif
#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif
/**********************************************************************************************************************/

// FUNCTION DECLARATIONS
/***********************************************************************************************************************/
void processMoveUp(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset);
void processMoveDown(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset);
void processMoveLeft(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset);
void processMoveRight(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset);
void processRotateCCW(unsigned char *buffer_frame, unsigned char *rendered_frame, int rotate_iteration);
void processRotateCW(unsigned char *buffer_frame, unsigned char *rendered_frame, int rotate_iteration);
void processMirrorX(unsigned char *buffer_frame, unsigned char *rendered_frame, int _unused);
void processMirrorY(unsigned char *buffer_frame, unsigned char *rendered_frame, int _unused);
// helper functions
//----------------------------------------------------------------------------------------------------------------------
void rotateCW90(unsigned char *buffer_frame, unsigned char *rendered_frame);
void rotateCCW90(unsigned char *buffer_frame, unsigned char *rendered_frame);
void rotateCW180(unsigned char *buffer_frame, unsigned char *rendered_frame);
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
void processMoveUp(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset) {
    // handle negative offsets
    if (offset < 0){
        processMoveDown(buffer_frame, rendered_frame, -offset);
        return;
    }  
    //printf("UP\n");
    int gap = offset * g_width * 3;
    int rowbyte = g_width * 3;
    /*memmove(buffer_frame + g_start - gap, buffer_frame + g_start, g_end - g_start);
    memset(buffer_frame + g_end - gap, BLANK, gap);
    */
    //printf("g_start= %d, g_end= %d, gap= %d, len= %d\n", g_start, g_end, gap, g_width * g_height * 3);
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        memmove(buffer_frame + row * rowbyte + (g_start % rowbyte) - gap, 
                buffer_frame + row * rowbyte + (g_start % rowbyte), 
                (g_end % rowbyte) - (g_start % rowbyte));
    }
    for (int row = (g_end - gap) / rowbyte + 1; row <= g_end / rowbyte; row++) {
        memset(buffer_frame + row * rowbyte + (g_start % rowbyte), BLANK, (g_end % rowbyte) - (g_start % rowbyte));
    }
    //memset(buffer_frame + g_end - gap, BLANK, gap);
    g_start -= gap;
    g_end -= gap;
    g_upperright -= gap;
    g_lowerleft -= gap;
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
void processMoveRight(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset) {
	// handle negative offsets
    if (offset < 0){
        processMoveLeft(buffer_frame, rendered_frame, -offset);
        return;
    }
    int gap = offset * 3;
    int rowbyte = g_width * 3;
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {	
        memmove(buffer_frame + row * rowbyte + (g_start % rowbyte) + gap, 
                buffer_frame + row * rowbyte + (g_start % rowbyte),
                (g_end % rowbyte) - (g_start % rowbyte));
        memset(buffer_frame + row * rowbyte + (g_start % rowbyte), BLANK, gap);
    }
    g_start += gap;
    g_end += gap;
    g_upperright += gap;
    g_lowerleft += gap;
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
void processMoveDown(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset) {
	// handle negative offsets
    if (offset < 0){
        processMoveUp(buffer_frame, rendered_frame, -offset);
        return;
    }
    //printf("DOWN\n");
    int gap = offset * g_width * 3;
    int rowbyte = g_width * 3;
    //memmove(buffer_frame + g_start + gap, buffer_frame + g_start, g_end - g_start);
    //memset(buffer_frame + g_start, BLANK, gap);
    for (int row = g_end / rowbyte; row >= g_start / rowbyte; row--) {
        memmove(buffer_frame + row * rowbyte + (g_start % rowbyte) + gap, 
                buffer_frame + row * rowbyte + (g_start % rowbyte), 
                (g_end % rowbyte) - (g_start % rowbyte));
    }
    for (int row = g_start / rowbyte; row < (g_start + gap) / rowbyte; row++) {
        memset(buffer_frame + row * rowbyte + (g_start % rowbyte), BLANK, (g_end % rowbyte) - (g_start % rowbyte));
    }
    g_start += gap;
    g_end += gap;
    g_upperright += gap;
    g_lowerleft += gap;
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
void processMoveLeft(unsigned char *buffer_frame, unsigned char *rendered_frame, int offset) {
    //printf("LEFT\n");
    // handle negative offsets
    if (offset < 0){
        processMoveRight(buffer_frame, rendered_frame, -offset);
        return;
    }

    int gap = offset * 3;
    int rowbyte = g_width * 3;
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        memmove(buffer_frame + row * rowbyte + (g_start % rowbyte) - gap,
                buffer_frame + row * rowbyte + (g_start % rowbyte),
                (g_end % rowbyte) - (g_start % rowbyte));
        memset(buffer_frame + row * rowbyte + (g_end % rowbyte) - gap, BLANK, gap);
    }
    
    g_start -= gap;
    g_end -= gap;
    g_upperright -= gap;
    g_lowerleft -= gap;
}

int indexCW90(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return y * rowbyte + (g_width - x - 1) * 3;
}

void rotateCW90(unsigned char *buffer_frame, unsigned char *rendered_frame) {
    //printf("CW90\n");
    int render_column, render_row;
    int rowbyte = 3 * g_width;

    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        render_column = g_width - row - 1;
        for (int column = (g_start % rowbyte) / 3; column <= (g_end % rowbyte) / 3; column++) {
            render_row = column;
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
        }
    }
    memset(buffer_frame + g_start, BLANK, g_end - g_start);
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d\n", g_start, g_end - 3, g_upperright, g_lowerleft);
    g_start = indexCW90(g_start);
    g_end = indexCW90(g_end - 3);
    g_upperright = indexCW90(g_upperright);
    g_lowerleft = indexCW90(g_lowerleft);
    
    int tmp = g_start;
    g_start = g_lowerleft;
    g_lowerleft = g_end;
    g_end = g_upperright;
    g_upperright = tmp;
    g_end += 3;
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d\n", g_start, g_end - 3, g_upperright, g_lowerleft);

    g_rendered = !g_rendered;
}

int indexCCW90(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return (g_height - y - 1) * rowbyte + x * 3;
}

void rotateCCW90(unsigned char *buffer_frame, unsigned char *rendered_frame) {
    // printf("CCW90\n");
    int render_column, render_row;
    int rowbyte = g_width * 3;
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        render_column = row;
        for (int column = (g_start % rowbyte) / 3; column <= (g_end % rowbyte) / 3; column++) {
            render_row = g_width - column - 1;
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
        }
    }
    memset(buffer_frame + g_start, BLANK, g_end - g_start);
    // printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d\n", g_start, g_end, g_upperright, g_lowerleft);
    g_start = indexCCW90(g_start);
    g_end = indexCCW90(g_end - 3);
    g_upperright = indexCCW90(g_upperright);
    g_lowerleft = indexCCW90(g_lowerleft);
    
    int tmp = g_start;
    g_start = g_upperright;
    g_upperright = g_end;
    g_end = g_lowerleft;
    g_lowerleft = tmp;
    g_end += 3;
    // printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d\n", g_start, g_end, g_upperright, g_lowerleft);
    g_rendered = !g_rendered;
}

int indexCW180(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return (g_height - x - 1) * rowbyte + (g_width - y - 1) * 3;
}

void rotateCW180(unsigned char *buffer_frame, unsigned char *rendered_frame) {
    int render_column, render_row;
    int rowbyte = g_width * 3;
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        render_row = g_height - row - 1;
        for (int column = (g_start % rowbyte) / 3; column <= (g_end % rowbyte) / 3; column++) {
            render_column = g_width - column - 1;
            int position_frame_buffer = row * g_width * 3 + column * 3;
            rendered_frame[render_row * g_width * 3 + render_column * 3] = buffer_frame[position_frame_buffer];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 1] = buffer_frame[position_frame_buffer + 1];
            rendered_frame[render_row * g_width * 3 + render_column * 3 + 2] = buffer_frame[position_frame_buffer + 2];
        }
    }
    memset(buffer_frame + g_start, BLANK, g_end - g_start);
    g_start = indexCW180(g_start);
    g_end = indexCW180(g_end - 3);
    g_upperright = indexCW180(g_upperright);
    g_lowerleft = indexCW180(g_lowerleft);

    int tmp = g_start;
    g_start = g_end;
    g_end = tmp;
    tmp = g_lowerleft;
    g_lowerleft = g_upperright;
    g_upperright = tmp;
    g_end += 3;

    g_rendered = !g_rendered;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param rotate_iteration - rotate object inside frame buffer clockwise by 90 degrees, <iteration> times
 * @return - pointer pointing a buffer storing a modified 24-bit bitmap image
 * Note: You can assume the frame will always be square and you will be rotating the entire image
 **********************************************************************************************************************/
void processRotateCW(unsigned char *buffer_frame, unsigned char *rendered_frame, int rotate_iteration) {
    if (rotate_iteration < 0){ // handle negative offsets
        processRotateCCW(buffer_frame, rendered_frame, -rotate_iteration);
        return;
    } else if (rotate_iteration % 4 == 0) {
        return;
	} else if (rotate_iteration % 4 == 1) {
        rotateCW90(buffer_frame, rendered_frame);
        return;
	} else if (rotate_iteration % 4 == 2) {
        rotateCW180(buffer_frame, rendered_frame);
        return;
	} else {
        rotateCCW90(buffer_frame, rendered_frame);		
        return;
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
void processRotateCCW(unsigned char *buffer_frame, unsigned char *rendered_frame, int rotate_iteration) {
    if (rotate_iteration < 0){ // handle negative offsets
        processRotateCW(buffer_frame, rendered_frame, -rotate_iteration);
        return;
    } else if (rotate_iteration % 4 == 0) {
		return;	
	} else if (rotate_iteration % 4 == 1) {
        rotateCCW90(buffer_frame, rendered_frame);
        return;
	} else if (rotate_iteration % 4 == 2) {
        rotateCW180(buffer_frame, rendered_frame);
        return;
	} else {
        rotateCW90(buffer_frame, rendered_frame);		
        return;
	}
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
int indexMX(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return (g_height - x - 1) * rowbyte + y * 3;
}

void processMirrorX(unsigned char *buffer_frame, unsigned char *rendered_frame, int _unused) {
    int rowbyte = g_width * 3;
    // store shifted pixels to temporary buffer
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        memmove(rendered_frame + (g_height - row - 1) * rowbyte + (g_start % rowbyte), 
                buffer_frame + row * rowbyte + (g_start % rowbyte),
                (g_end % rowbyte) - (g_start % rowbyte));
    }
    memset(buffer_frame + g_start, BLANK, g_end - g_start);
    //printf("MX\n");
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d, len= %d\n", g_start, g_end - 3, g_upperright, g_lowerleft, g_width*g_height*3);
    g_start = indexMX(g_start);
    g_end = indexMX(g_end - 3);
    g_upperright = indexMX(g_upperright);
    g_lowerleft = indexMX(g_lowerleft);
    
    int tmp = g_start;
    g_start = g_lowerleft;
    g_lowerleft = tmp;
    tmp = g_end;
    g_end = g_upperright;
    g_upperright = tmp;
    g_end += 3;
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d, len= %d\n", g_start, g_end - 3, g_upperright, g_lowerleft, g_width*g_height*3);
    g_rendered = !g_rendered;
}

/***********************************************************************************************************************
 * @param buffer_frame - pointer pointing to a buffer storing the imported 24-bit bitmap image
 * @param width - width of the imported 24-bit bitmap image
 * @param height - height of the imported 24-bit bitmap image
 * @param _unused - this field is unused
 * @return
 **********************************************************************************************************************/
int indexMY(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return x * rowbyte + (g_width - y - 1) * 3;
}

void processMirrorY(unsigned char *buffer_frame, unsigned char *rendered_frame, int _unused) {
    //printf("MY\n");
    int rowbyte = g_width * 3;
    // store shifted pixels to temporary buffer
    for (int row = g_start / rowbyte; row <= g_end / rowbyte; row++) {
        for (int column = (g_start % rowbyte) / 3; column <= (g_end % rowbyte) / 3; column++) {
            int position_buffer_frame = row * g_height * 3 + column * 3;
            int position_rendered_frame = row * g_height * 3 + (g_width - column - 1) * 3;
            rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
            rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
            rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
        }
    }
    memset(buffer_frame + g_start, BLANK, g_end - g_start);
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d, len= %d\n", g_start, g_end, g_upperright, g_lowerleft, g_width*g_height*3);
    g_start = indexMY(g_start);
    g_end = indexMY(g_end - 3);
    g_upperright = indexMY(g_upperright);
    g_lowerleft = indexMY(g_lowerleft);
    //printf("g_start= %d, g_end= %d, g_ur= %d, g_ll= %d\n", g_start, g_end, g_upperright, g_lowerleft);
    int tmp = g_start;
    g_start = g_upperright;
    g_upperright = tmp;
    tmp = g_end;
    g_end = g_lowerleft;
    g_lowerleft = tmp;
    g_end += 3;

    g_rendered = !g_rendered;
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
typedef enum tagState {
    INIT=0, W=1, A=2, S=3, D=4, ROTATE=5, MIRROR=6
} State;

typedef struct tagIteration {
    unsigned int W, A, S, D, CW, CCW, MX, MY;
} Iteration;

void process(unsigned char *frame_buffer, unsigned char *frame_rendered, Iteration *iter, State state)
{
    switch (state) {
        case INIT: {
            break;
        }
        case W: {
            if (g_rendered) {
                processMoveUp(frame_rendered, frame_buffer, iter->W);
            } else {
                processMoveUp(frame_buffer, frame_rendered, iter->W);
            }
            iter->W = 0;
            break;
        }
        case A: {
            if (g_rendered) {
                processMoveLeft(frame_rendered, frame_buffer, iter->A);
            } else {
                processMoveLeft(frame_buffer, frame_rendered, iter->A);
            }
            iter->A = 0;
            break;
        }
        case S: {
            if (g_rendered) {
                processMoveDown(frame_rendered, frame_buffer, iter->S);
            } else {
                processMoveDown(frame_buffer, frame_rendered, iter->S);
            }
            iter->S = 0;
            break;
        }
        case D: {
            if (g_rendered) {
                processMoveRight(frame_rendered, frame_buffer, iter->D);
            } else {
                processMoveRight(frame_buffer, frame_rendered, iter->D);
            }
            iter->D = 0;
            break;
        }
        case ROTATE: {
            int cw = iter->CW - iter->CCW;
            if (cw > 0) { 
                if (g_rendered) {   
                    processRotateCW(frame_rendered, frame_buffer, cw);
                } else {
                    processRotateCW(frame_buffer, frame_rendered, cw);
                }
            } else if (cw < 0) { 
                if (g_rendered) {   
                    processRotateCCW(frame_rendered, frame_buffer, -cw);
                } else {
                    processRotateCCW(frame_buffer, frame_rendered, -cw);
                }
            }
            iter->CW = 0;
            iter->CCW = 0;
            break;
        }
        case MIRROR: {
            if (iter->MX % 2) { 
                if (g_rendered) {   
                    processMirrorX(frame_rendered, frame_buffer, iter->MX);
                } else {
                    processMirrorX(frame_buffer, frame_rendered, iter->MX);
                }
            }
            if (iter->MY % 2) { 
                if (g_rendered) {   
                    processMirrorY(frame_rendered, frame_buffer, iter->MY);
                } else {
                    processMirrorY(frame_buffer, frame_rendered, iter->MY);
                }
            }
            iter->MX = 0;
            iter->MY = 0;
            break;
        }
        default: fprintf(stderr, "processing unknown state\n");
    }
}

State state_transfer(const char *key, const int value, Iteration *iter) 
{
    if (!strcmp(key, "W")) {
        if (value > 0) {
            iter->W += value;
            return W;
        } else if (value < 0) {
            iter->S += -value;
            return S;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "A")) {
        if (value > 0) {
            iter->A += value;
            return A;
        } else if (value < 0) {
            iter->D += -value;
            return D;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "S")) {
        if (value > 0) {
            iter->S += value;
            return S;
        } else if (value < 0) {
            iter->W += -value;
            return W;
        } else {
            return INIT;
        }
    } else if (!strcmp(key, "D")) {
        if (value > 0) {
            iter->D += value;
            return D;
        } else if (value < 0) {
            iter->A += -value;
            return A;
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
        printf("Invalid key %s", key);
        return -1;
    }
}

void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
	// assign global variables
	g_width = width;
    g_height = height;

    g_start = 0;
    while (frame_buffer[g_start] == BLANK && frame_buffer[g_start + 1] == BLANK && frame_buffer[g_start + 2] == BLANK) {
        g_start+=3;
    }
    g_end = height * width * 3;
    while (frame_buffer[g_end - 1] == BLANK && frame_buffer[g_end - 2] == BLANK && frame_buffer[g_end - 3] == BLANK) {
        g_end-=3;
    }
    int rightmost = 0;
    int leftmost = g_width;
    for (int row = g_start / (3 * width); row <= g_end / (3 * width); row++) {
        for (int column = 0; column < width; column++) {
            int idx = row * g_width * 3 + column * 3;
            if (frame_buffer[idx] == BLANK && frame_buffer[idx + 1] == BLANK && frame_buffer[idx + 2] == BLANK) {
                continue;
            } else {
                leftmost = MIN(leftmost, column);
                rightmost = MAX(rightmost, column);
            }
        }
    }
    g_start = (g_start / (3 * width)) * width * 3 + leftmost * 3;
    g_end = (g_end / (3 * width)) * width * 3 + rightmost * 3 + 3;
    g_upperright = (g_start / (3 * width)) * width * 3 + rightmost * 3;
    g_lowerleft = (g_end / (3 * width)) * width * 3 + leftmost * 3;

    // allocate memory for temporary image buffer
    unsigned char *frame_rendered = allocateFrame(width, height);
    memset(frame_rendered, BLANK, 3 * g_height * g_width);
	int processed_frames = 0;

	State state = INIT;
	Iteration iter;
    memset(&iter, 0, sizeof(Iteration));	
    
    char *key;
    int value;
    State next_state = INIT;
    for (int sensorValueIdx = 0; sensorValueIdx < sensor_values_count; sensorValueIdx++) {
    //      printf("Processing sensor value #%d: %s, %d\n", sensorValueIdx, sensor_values[sensorValueIdx].key,
    //               sensor_values[sensorValueIdx].value);
        key = sensor_values[sensorValueIdx].key;
        value = sensor_values[sensorValueIdx].value;
        next_state = state_transfer(key, value, &iter);
        if (state != INIT && state != next_state) {
            process(frame_buffer, frame_rendered, &iter, state);
        }
        state = next_state;
        
	    processed_frames += 1;
        if (processed_frames % 25 == 0) {
            process(frame_buffer, frame_rendered, &iter, state);
            if (g_rendered) {               
                verifyFrame(frame_rendered, width, height, grading_mode);
            } else {
                verifyFrame(frame_buffer, width, height, grading_mode);
            }
        }
    }
    process(frame_buffer, frame_rendered, &iter, state);
    if (g_rendered) {
        frame_buffer = copyFrame(frame_rendered, frame_buffer, g_width, g_height);
    }
    
    // free temporary image buffer
    deallocateFrame(frame_rendered);
    return;
}
