#include <stdio.h>
#include <time.h>
const int g_height = 2048;
const int g_width = 2048;
unsigned char buffer_frame[3*2048*2048];
unsigned char rendered_frame[3*2048*2048];

unsigned char buffer_frame1[2048*2048];
unsigned char buffer_frame2[2048*2048];
unsigned char buffer_frame3[2048*2048];
unsigned char rendered_frame1[2048*2048];
unsigned char rendered_frame2[2048*2048];
unsigned char rendered_frame3[2048*2048];
const int T = 16;
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
/*
void rotate1(void) {
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
}

void rotate2(void) {
    for (int row = 0; row < g_width; row+=T) {
        int row_bound = min(row + T, g_width);
        for (int column = 0; column < g_height; column+=T) {
            int column_bound = min(column + T, g_height);
            for (int row_t = row; row_t < row_bound; row_t++) {
                for (int column_t = column; column_t < column_bound; column_t++) {
                    int position_frame_buffer = row_t * g_width + column_t;
                    int position_rendered_frame = column_t * g_width + row_t;
                    rendered_frame1[position_rendered_frame] = buffer_frame1[position_frame_buffer];
                    rendered_frame2[position_rendered_frame] = buffer_frame2[position_frame_buffer];
                    rendered_frame3[position_rendered_frame] = buffer_frame3[position_frame_buffer];
                }
            }
        }
    }
}
*/

char temp1[2048*2048];
char temp2[2048*2048];

void rotate1 (void)
{
  int x,y;
  for (y=0; y<2048; y++)
  for (x=0; x<2048; x++)
    temp2[2048*y+x] = temp1[2048*x+y];
}

void rotate2 (void)
{
  int x,y;
  int bx, by;

  for (by=0; by<2048; by+=16)
  for (bx=0; bx<2048; bx+=16)
  for (y=0; y<16; y++)
  for (x=0; x<16; x++)
    temp2[2048*(y+by)+x+bx] = temp1[2048*(x+bx)+y+by];
}


int main() {
	clock_t start, end;
	start = clock();
	for (int i = 0; i < 1; i++) {
		rotate1();	
	}
	end = clock();
	printf("rotate1: %d\t", end - start);

	start = clock();
	for (int i = 0; i < 1; i++) {
		rotate2();
	}
	end = clock();
	printf("rotate2: %d\t", end - start);

	return 0;
}