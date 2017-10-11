#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utilities.h"  // DO NOT REMOVE this line
#include "implementation_reference.h"   // DO NOT REMOVE this line

// GLOBAL VARIABLES
/***********************************************************************************************************************/
typedef struct tagBlob {
    int start;
    int end;
    int upperright;
    int lowerleft;
    struct tagBlob *next;
} Blob;

// width and height of this image, globally visible
unsigned int g_width, g_height;
// global index for double-buffering
int g_rendered = 0;
// global tracker of blobs
Blob *g_blobs;
// tiling blocksize
const int T = 16;
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
    int rowbyte = g_width * 3;
    int gap = offset * rowbyte;
    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memcpy(rendered_frame + row * rowbyte + (b_start % rowbyte) - gap, 
                   buffer_frame + row * rowbyte + (b_start % rowbyte), 
                   (b_end - b_start) % rowbyte + 3);
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte),
                   BLANK, 
                   (b_end - b_start) % rowbyte + 3);
        }
        b_start -= gap;
        b_end -= gap;
        b_upperright -= gap;
        b_lowerleft -= gap;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    g_rendered = !g_rendered;
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
    //printf("RIGHT\n");
    int gap = offset * 3;
    int rowbyte = g_width * 3;

    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {	
            memcpy(rendered_frame + row * rowbyte + (b_start % rowbyte) + gap, 
                   buffer_frame + row * rowbyte + (b_start % rowbyte),
                   (b_end - b_start) % rowbyte + 3);
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        b_start += gap;
        b_end += gap;
        b_upperright += gap;
        b_lowerleft += gap;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    g_rendered = !g_rendered;
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
    int rowbyte = g_width * 3;
    int gap = offset * rowbyte;
    //printf("DOWN %d\n", offset);
    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;
        /*printf("start = (%d, %d), end = (%d, %d), upperright = (%d, %d), lowerleft = (%d, %d)\n", 
        b_start / rowbyte, (b_start % rowbyte) / 3, 
        b_end / rowbyte, (b_end % rowbyte) / 3,
        b_upperright / rowbyte, (b_upperright % rowbyte) / 3,
        b_lowerleft / rowbyte, (b_lowerleft % rowbyte) / 3);*/

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {          
            memcpy(rendered_frame + row * rowbyte + (b_start % rowbyte) + gap, 
                   buffer_frame + row * rowbyte + (b_start % rowbyte), 
                   (b_end - b_start) % rowbyte + 3);
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        b_start += gap;
        b_end += gap;
        b_upperright += gap;
        b_lowerleft += gap;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    g_rendered = !g_rendered;
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
    // handle negative offsets
    if (offset < 0){
        processMoveRight(buffer_frame, rendered_frame, -offset);
        return;
    }
    //printf("LEFT\n");
    int gap = offset * 3;
    int rowbyte = g_width * 3;

    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memcpy(rendered_frame + row * rowbyte + (b_start % rowbyte) - gap,
                    buffer_frame + row * rowbyte + (b_start % rowbyte),
                    (b_end - b_start) % rowbyte + 3);
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        b_start -= gap;
        b_end -= gap;
        b_upperright -= gap;
        b_lowerleft -= gap;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    g_rendered = !g_rendered;
}

int indexCW90(int idx) {
    int x, y;
    int rowbyte = g_width * 3;
    x = idx / rowbyte;
    y = (idx % rowbyte) / 3;
    return y * rowbyte + (g_width - x - 1) * 3;
}

void rotateCW90(unsigned char *buffer_frame, unsigned char *rendered_frame) {
    int render_column, render_row;
    int rowbyte = 3 * g_width;
    //printf("CW90\n");
    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row+=T) {
            int tmp0 = (b_end % rowbyte) / 3;
            for (int column = (b_start % rowbyte) / 3; column <= tmp0; column+=T) {
                int tmp1 = MIN(row + T - 1, b_end / rowbyte);
                for (int r = row; r <= tmp1; r++) {
                    render_column = g_width - r - 1;
                    int tmp2 = MIN(column + T - 1, (b_end % rowbyte) / 3);
                    for (int c = column; c <= tmp2; c++) {
                        render_row = c;
                        int position_buffer_frame = r * g_width * 3 + c * 3;
                        int position_rendered_frame = render_row * rowbyte + render_column * 3;
                        rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
                        rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
                        rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
                    }
                }
            }
        }
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }

        b_start = indexCW90(b_start);
        b_end = indexCW90(b_end);
        b_upperright = indexCW90(b_upperright);
        b_lowerleft = indexCW90(b_lowerleft);
        
        int tmp = b_start;
        b_start = b_lowerleft;
        b_lowerleft = b_end;
        b_end = b_upperright;
        b_upperright = tmp;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }

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
    int render_column, render_row;
    int rowbyte = g_width * 3;
    //printf("CCW90\n");
    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row+=T) {
            int tmp0 = (b_end % rowbyte) / 3;
            for (int column = (b_start % rowbyte) / 3; column <= tmp0; column+=T) {
                int tmp1 = MIN(row + T - 1, b_end / rowbyte);
                for (int r = row; r <= tmp1; r++) {
                    render_column = r;
                    int tmp2 = MIN(column + T - 1, (b_end % rowbyte) / 3);
                    for (int c = column; c <= tmp2; c++) {
                        render_row = g_width - c - 1;
                        int position_buffer_frame = r * g_width * 3 + c * 3;
                        int position_rendered_frame = render_row * rowbyte + render_column * 3;
                        rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
                        rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
                        rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
                    }
                }
            }
        }
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        
        b_start = indexCCW90(b_start);
        b_end = indexCCW90(b_end);
        b_upperright = indexCCW90(b_upperright);
        b_lowerleft = indexCCW90(b_lowerleft);
        
        int tmp = b_start;
        b_start = b_upperright;
        b_upperright = b_end;
        b_end = b_lowerleft;
        b_lowerleft = tmp;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    
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
    Blob *blob = g_blobs;
    //printf("CW180\n");
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row+=T) {
            int tmp0 = (b_end % rowbyte) / 3;
            for (int column = (b_start % rowbyte) / 3; column <= tmp0; column+=T) {
                int tmp1 = MIN(row + T - 1, b_end / rowbyte);
                for (int r = row; r <= tmp1; r++) {
                    render_row = g_height - r - 1;
                    int tmp2 = MIN(column + T - 1, (b_end % rowbyte) / 3);
                    for (int c = column; c <= tmp2; c++) {
                        render_column = g_width - c - 1;
                        int position_buffer_frame = r * g_width * 3 + c * 3;
                        int position_rendered_frame = render_row * rowbyte + render_column * 3;
                        rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
                        rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
                        rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
                    }
                }
            }
        }
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }

        b_start = indexCW180(b_start);
        b_end = indexCW180(b_end);
        b_upperright = indexCW180(b_upperright);
        b_lowerleft = indexCW180(b_lowerleft);

        int tmp = b_start;
        b_start = b_end;
        b_end = tmp;
        tmp = b_lowerleft;
        b_lowerleft = b_upperright;
        b_upperright = tmp;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }

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
    //printf("MX\n");
    Blob *blob = g_blobs;
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memcpy(rendered_frame + (g_height - row - 1) * rowbyte + (b_start % rowbyte), 
                    buffer_frame + row * rowbyte + (b_start % rowbyte),
                    (b_end - b_start) % rowbyte + 3);
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        
        b_start = indexMX(b_start);
        b_end = indexMX(b_end);
        b_upperright = indexMX(b_upperright);
        b_lowerleft = indexMX(b_lowerleft);
        
        int tmp = b_start;
        b_start = b_lowerleft;
        b_lowerleft = tmp;
        tmp = b_end;
        b_end = b_upperright;
        b_upperright = tmp;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }
    
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
    int rowbyte = g_width * 3;
    Blob *blob = g_blobs;
    //printf("MY\n");
    int b_start, b_end, b_upperright, b_lowerleft;
    while (blob) {
        b_start = blob->start;
        b_end = blob->end;
        b_upperright = blob->upperright;
        b_lowerleft = blob->lowerleft;

        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row+=T) {
            for (int column = (b_start % rowbyte) / 3; column <= (b_end % rowbyte) / 3; column+=T) {
                int tmp1 = MIN(row + T - 1, b_end / rowbyte);
                for (int r = row; r <= tmp1; r++) {
                    int tmp2 = MIN(column + T - 1, (b_end % rowbyte) / 3);
                    for (int c = column; c <= tmp2; c++) {
                        int position_buffer_frame = r * g_height * 3 + c * 3;
                        int position_rendered_frame = r * g_height * 3 + (g_width - c - 1) * 3;
                        rendered_frame[position_rendered_frame] = buffer_frame[position_buffer_frame];
                        rendered_frame[position_rendered_frame + 1] = buffer_frame[position_buffer_frame + 1];
                        rendered_frame[position_rendered_frame + 2] = buffer_frame[position_buffer_frame + 2];
                    }
                }
            }
        }
        for (int row = b_start / rowbyte; row <= b_end / rowbyte; row++) {
            memset(buffer_frame + row * rowbyte + (b_start % rowbyte), 
                    BLANK, 
                    (b_end - b_start) % rowbyte + 3);
        }
        
        b_start = indexMY(b_start);
        b_end = indexMY(b_end);
        b_upperright = indexMY(b_upperright);
        b_lowerleft = indexMY(b_lowerleft);
        
        int tmp = b_start;
        b_start = b_upperright;
        b_upperright = tmp;
        tmp = b_end;
        b_end = b_lowerleft;
        b_lowerleft = tmp;

        blob->start = b_start;
        blob->end = b_end;
        blob->upperright = b_upperright;
        blob->lowerleft = b_lowerleft;

        blob = blob->next;
    }

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
    INIT=0, SHIFT=1, ROTATE=2, MIRROR=3
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
        case SHIFT: {
            int ws = iter->W - iter->S;
            if (ws > 0) {
                if (g_rendered) {
                    processMoveUp(frame_rendered, frame_buffer, ws);
                } else {
                    processMoveUp(frame_buffer, frame_rendered, ws);
                }
            } else if (ws < 0) {
                if (g_rendered) {
                    processMoveDown(frame_rendered, frame_buffer, -ws);
                } else {
                    processMoveDown(frame_buffer, frame_rendered, -ws);
                }
            }
            iter->W = 0;
            iter->S = 0;

            int ad = iter->A - iter->D;
            if (ad > 0) {
                if (g_rendered) {
                    processMoveLeft(frame_rendered, frame_buffer, ad);
                } else {
                    processMoveLeft(frame_buffer, frame_rendered, ad);
                }
            } else if (ad < 0) {
                if (g_rendered) {
                    processMoveRight(frame_rendered, frame_buffer, -ad);
                } else {
                    processMoveRight(frame_buffer, frame_rendered, -ad);
                }
            }
            iter->A = 0;
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

Blob* allocateBlob() {
    Blob *blob = (Blob *)malloc(sizeof(Blob));
    blob->next = NULL;
    return blob;
}

void deallocateBlobs() {
    Blob *p = g_blobs;
    Blob *q;
    while (p) {
        q = p->next;
        free(p);
        p = q;
    }
}

void detectBlobs(unsigned char *frame_buffer) {
    int tolerance;
    int b_start, b_end, b_upperright, b_lowerleft, leftmost, rightmost;
    int cursor = 0;
    int rowbyte = g_width * 3;
    int framebyte = rowbyte * g_height;
    Blob *lastblob, *tail;
    while (cursor < framebyte) {
        while (frame_buffer[cursor] == BLANK && frame_buffer[cursor + 1] == BLANK && frame_buffer[cursor + 2] == BLANK) {
            cursor+=3;
            if (cursor >= framebyte) break;
        }
        if (cursor >= framebyte) break;
        lastblob = allocateBlob();
        b_start = cursor;
        rightmost = (cursor % rowbyte) / 3;
        leftmost = (cursor % rowbyte) / 3;

        tolerance = g_width;
        for (;;) {
            cursor += 3;
            if (frame_buffer[cursor] == BLANK && frame_buffer[cursor + 1] == BLANK && frame_buffer[cursor + 2] == BLANK) {
                tolerance--;
                if (tolerance == 0 || cursor == framebyte - 3) break; 
            } else {
                tolerance = g_width + 2; // what the f***
                b_end = cursor;   
                if (cursor == framebyte - 3) break;   
                leftmost = MIN(leftmost, (cursor % rowbyte) / 3);
                rightmost = MAX(rightmost, (cursor % rowbyte) / 3); 
            }
        }

        b_start = (b_start / (3 * g_width)) * g_width * 3 + leftmost * 3;
        b_end = (b_end / (3 * g_width)) * g_width * 3 + rightmost * 3;
        b_upperright = (b_start / (3 * g_width)) * g_width * 3 + rightmost * 3;
        b_lowerleft = (b_end / (3 * g_width)) * g_width * 3 + leftmost * 3;
        
        /*printf("start = (%d, %d), end = (%d, %d), upperright = (%d, %d), lowerleft = (%d, %d)\n", 
        b_start / rowbyte, (b_start % rowbyte) / 3, 
        b_end / rowbyte, (b_end % rowbyte) / 3,
        b_upperright / rowbyte, (b_upperright % rowbyte) / 3,
        b_lowerleft / rowbyte, (b_lowerleft % rowbyte) / 3);
        */
        lastblob->start = b_start;
        lastblob->end = b_end;
        lastblob->upperright = b_upperright;
        lastblob->lowerleft = b_lowerleft;
        
        if (g_blobs == NULL) {
            g_blobs = lastblob;
            tail = lastblob;
        } else {
            tail->next = lastblob;
            tail = lastblob;
        }
    }
}


void implementation_driver(struct kv *sensor_values, int sensor_values_count, unsigned char *frame_buffer,
                           unsigned int width, unsigned int height, bool grading_mode) {
	// assign global variables
	g_width = width;
    g_height = height;
    g_blobs = NULL;
    
    detectBlobs(frame_buffer);

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
    //printBMP(width, height, frame_buffer);
    // free temporary image buffer
    deallocateFrame(frame_rendered);
    deallocateBlobs();
    return;
}

