//
// Created by Jacob Lin on 6/19/24.
//

//#include <stdio.h>
#include "graphi.c"
#include "math.c"

#define WIDTH 800
#define HEIGHT 600

#define CUBES 27
#define SIDES 8
#define NUM_CORNERS 8
#define DIMENSION 3
#define NUM_PLANES 6
#define PLANE_CORNERS 4

#define PI 3.14159265359
#define FLT_MAX 3.402823466e+38F

#define SCALE 1.1
#define GAP 0.6

#define BG_COLOR BLACK

static uint32_t pixels[WIDTH * HEIGHT];

float A, B, C;

typedef struct {
    float x, y, z;
} Point_3D;

typedef struct {
    float x, y;
} Point_2D;

typedef struct {
    Point_3D points[NUM_CORNERS];
    uint32_t color;
    int selected;
    int id;
} cube;

typedef struct {
    Point_3D pointA;
    Point_3D pointB;
    Point_3D pointC;
    Point_3D pointD;

    Point_2D s_pointA;
    Point_2D s_pointB;
    Point_2D s_pointC;
    Point_2D s_pointD;

    uint32_t color;
} plane;

float calculateX(float i, float j, float k) {
    return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) +
           j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}
float calculateY(float i, float j, float k) {
    return j * cos(A) * cos(C) + k * sin(A) * cos(C) -
           j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) -
           i * cos(B) * sin(C);
}
float calculateZ(float i, float j, float k) {
    return k * cos(A) * cos(B) - j * sin(A) * cos(B) + i * sin(B);
}

float get_plane_z_sum(plane input) {
    return input.pointA.z + input.pointB.z + input.pointC.z + input.pointD.z;
}

float get_cube_z_sum(Point_3D *input) {
    float sum = 0;
    for(int i = 0; i < NUM_CORNERS; i++) {
        sum += input[i].z;
    }
    return sum;
}

void generateCorners(cube *cubes, float magnitude, float camera_distance, float x_offset, float y_offset, float z_offset) {
    float x_factor = -1;
    float y_factor = -1;
    float z_factor = -1;

    for(int i=0; i<NUM_CORNERS; i++) {
        Point_3D newPoint;

        x_factor *= -1;
        if((i % 2) == 0) y_factor *= -1;
        if((i % 4) == 0) z_factor *= -1;

        newPoint.x = x_factor * magnitude + x_offset;
        newPoint.y = y_factor * magnitude + y_offset;
        newPoint.z = z_factor * magnitude + z_offset + camera_distance;

        cubes->points[i] = newPoint;
    }
}

void resetFaces(cube *cubes, int num_cubes, const uint32_t *colors) {
    int i;
    int curr_color = -1;
    for(i=0; i<num_cubes; i++) {
        if(i % 27 == 0) {
            ++curr_color;
        }

        cubes[i].color = colors[curr_color];
        cubes[i].selected = 0;
        cubes[i].id = i;

    }
}

void move_color(cube *dest, cube *src) {
    dest->color = src->color;
}

void copy_cube(cube *dest, cube *src) {
    for(int i = 0; i < NUM_CORNERS; i++) {
        dest->points[i] = src->points[i];
    }

    dest->selected = src->selected;
    dest->id = src->id;

}

cube *generateCubes(cube *cubes, cube *translated_cubes, int num_cubes, float magnitude, float camera_distance, uint32_t *colors) {
    int i, j, k;

    // copy
    for(i=0; i < num_cubes; i++) {
        for(j=0; j < NUM_CORNERS; j++) {
            translated_cubes[i].points[j] = cubes[i].points[j];
        }
        translated_cubes[i].color = cubes[i].color;
        translated_cubes[i].selected = cubes[i].selected;
        translated_cubes[i].id = cubes[i].id;
    }

    float spacing = (float) (SCALE * 2);

    float x_offset = -spacing * ((float)(DIMENSION - 1) / (float) 2);
    float y_offset = -spacing * ((float)(DIMENSION - 1) / (float) 2);
    float z_offset = -spacing * ((float)(DIMENSION - 1) / (float) 2);

    int count = 0;

    float separation = 15;

    const int numVectors = 8;

    Point_3D offsetVector0 = {0, 0, 0};
    Point_3D offsetVector1 = {0, -separation, 0};
    Point_3D offsetVector2 = {0, 0, +separation};
    Point_3D offsetVector3 = {+separation, 0, 0};
    Point_3D offsetVector4 = {0, 0, -separation};
    Point_3D offsetVector5 = {-separation, 0, 0};
    Point_3D offsetVector6 = {0, +separation, 0};
    Point_3D offsetVector7 = {+separation * 10, 0, 0};


    Point_3D offsetVectors[8] = {offsetVector0, offsetVector1, offsetVector2, offsetVector3,
                                 offsetVector4, offsetVector5, offsetVector6, offsetVector7};

    for(int vector = 0; vector < numVectors; vector++) {
        for (i = 0; i < DIMENSION; i++) {
            for (j = 0; j < DIMENSION; j++) {
                for (k = 0; k < DIMENSION; k++) {
                    generateCorners(&translated_cubes[count], magnitude, camera_distance,
                                    x_offset + (spacing * (float) i) + offsetVectors[vector].x,
                                    y_offset + (spacing * (float) j) + offsetVectors[vector].y,
                                    z_offset + (spacing * (float) k) + offsetVectors[vector].z);
//                    translated_cubes[count].color = colors[vector];
                    count++;
                }
            }
        }
    }

    for(i = 0; i < num_cubes; i++) {
        copy_cube(&cubes[i], &translated_cubes[i]);
    }

    for(i = 0; i < num_cubes; i++) {
        for(j = 0; j < NUM_CORNERS; j++) {
            Point_3D new_point = {calculateX(translated_cubes[i].points[j].x, translated_cubes[i].points[j].y, translated_cubes[i].points[j].z - camera_distance),
                                  calculateY(translated_cubes[i].points[j].x, translated_cubes[i].points[j].y, translated_cubes[i].points[j].z - camera_distance),
                                  calculateZ(translated_cubes[i].points[j].x, translated_cubes[i].points[j].y, translated_cubes[i].points[j].z - camera_distance) + camera_distance};
            translated_cubes[i].points[j] = new_point;
        }
    }

    // selection sort
    int min;
    for(i = 0; i < num_cubes-1; i++) {
        min = i;
        for(j = i + 1; j < num_cubes; j++) {
            if(get_cube_z_sum(translated_cubes[j].points) > get_cube_z_sum(translated_cubes[min].points)) {
                min = j;
            }
        }

        Point_3D temp;
        uint32_t temp_color;
        int temp_selected;
        int temp_id;
        if(min != i) {
            for(j = 0; j< NUM_CORNERS; j++) {
                temp = translated_cubes[min].points[j];
                translated_cubes[min].points[j] = translated_cubes[i].points[j];
                translated_cubes[i].points[j] = temp;
            }

            temp_color = translated_cubes[min].color;
            translated_cubes[min].color = translated_cubes[i].color;
            translated_cubes[i].color = temp_color;

            temp_selected = translated_cubes[min].selected;
            translated_cubes[min].selected = translated_cubes[i].selected;
            translated_cubes[i].selected = temp_selected;

            temp_id = translated_cubes[min].id;
            translated_cubes[min].id = translated_cubes[i].id;
            translated_cubes[i].id = temp_id;
        }
    }

    return translated_cubes;
}

void convert_3D_to_2D(Point_2D *point2D, int width, int height, Point_3D *points, Point_3D camera) {
    for(int i = 0; i < NUM_CORNERS; i++) {
        Point_3D curr = points[i];
        float u, v;
        float x, y, z;
        x = curr.x - camera.x;
        y = curr.y - camera.y;
        z = curr.z - camera.z;

        if(z == 0) {
            return;
        }

        u = (x / z) * 3000.0 + width/2.0;
        v = (y / z) * 3000.0 + height/2.0;

        Point_2D p = {u, v};
        point2D[i] = p;
    }
}

void draw_planes(uint32_t *canvas, int width, int height, plane *p, int count) {
    int i, j;

    // selection sort
    int min;
    for(i = 0; i < NUM_PLANES-1; i++) {
        min = i;
        for(j = i + 1; j < NUM_PLANES; j++) {
            if(get_plane_z_sum(p[j]) > get_plane_z_sum(p[min])) {
                min = j;
            }
        }

        if(min != i) {
            plane temp = p[min];
            p[min] = p[i];
            p[i] = temp;
        }
    }


    for(i = count/2; i < count; i++) {
        fill_triangle(canvas, width, height,
                      (int) (p[i].s_pointA.x), (int) (p[i].s_pointA.y),
                      (int) (p[i].s_pointB.x), (int) (p[i].s_pointB.y),
                      (int) (p[i].s_pointC.x), (int) (p[i].s_pointC.y),
                      p[i].color);
        fill_triangle(canvas, width, height,
                      (int) (p[i].s_pointD.x), (int) (p[i].s_pointD.y),
                      (int) (p[i].s_pointB.x), (int) (p[i].s_pointB.y),
                      (int) (p[i].s_pointC.x), (int) (p[i].s_pointC.y),
                      p[i].color);
    }
}

int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
    int i, j, c = 0;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((verty[i]>=testy) != (verty[j]>=testy)) &&
             (testx <= (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
    }
    return c;
}

#define HOVER_COLOR 0xFF808080
int found_hovered = 0;

int set_hovered(cube *cube, plane *p, int x, int y) {
    // organize by y value
    float vertx[PLANE_CORNERS];
    float verty[PLANE_CORNERS];

    for(int i=0; i<NUM_PLANES; i++) {
        vertx[0] = p[i].s_pointA.x;
        vertx[1] = p[i].s_pointB.x;
        vertx[2] = p[i].s_pointC.x;
        vertx[3] = p[i].s_pointD.x;

        verty[0] = p[i].s_pointA.y;
        verty[1] = p[i].s_pointB.y;
        verty[2] = p[i].s_pointC.y;
        verty[3] = p[i].s_pointD.y;

        if(pnpoly(PLANE_CORNERS, vertx, verty, (float) x, (float) y)) {
            cube->color = HOVER_COLOR;
            return 1;
        }
    }
    return 0;
}

void set_front_hover(cube *cubes, int num_cubes) {
    cube *curr_front = 0;
    float min_z = FLT_MAX;
    for(int i=0; i<num_cubes; i++) {

    }
}

void draw_outline(Point_2D *points) {
    int i;

    // 01 04 02 57 76 73 15 54 46

    draw_line(pixels, WIDTH, HEIGHT, (int) points[0].x, (int) points[0].y, (int) points[1].x, (int) points[1].y, HOTPINK); // 01
    draw_line(pixels, WIDTH, HEIGHT, (int) points[0].x, (int) points[0].y, (int) points[4].x, (int) points[4].y, HOTPINK); // 04
    draw_line(pixels, WIDTH, HEIGHT, (int) points[0].x, (int) points[0].y, (int) points[2].x, (int) points[2].y, HOTPINK); // 02
    draw_line(pixels, WIDTH, HEIGHT, (int) points[5].x, (int) points[5].y, (int) points[7].x, (int) points[7].y, HOTPINK); // 57
    draw_line(pixels, WIDTH, HEIGHT, (int) points[7].x, (int) points[7].y, (int) points[6].x, (int) points[6].y, HOTPINK); // 76
    draw_line(pixels, WIDTH, HEIGHT, (int) points[7].x, (int) points[7].y, (int) points[3].x, (int) points[3].y, HOTPINK); // 73
    draw_line(pixels, WIDTH, HEIGHT, (int) points[1].x, (int) points[1].y, (int) points[5].x, (int) points[5].y, HOTPINK); // 15
    draw_line(pixels, WIDTH, HEIGHT, (int) points[5].x, (int) points[5].y, (int) points[4].x, (int) points[4].y, HOTPINK); // 54
    draw_line(pixels, WIDTH, HEIGHT, (int) points[4].x, (int) points[4].y, (int) points[6].x, (int) points[6].y, HOTPINK); // 46
    draw_line(pixels, WIDTH, HEIGHT, (int) points[2].x, (int) points[2].y, (int) points[6].x, (int) points[6].y, HOTPINK); // 26
    draw_line(pixels, WIDTH, HEIGHT, (int) points[3].x, (int) points[3].y, (int) points[2].x, (int) points[2].y, HOTPINK); // 34
    draw_line(pixels, WIDTH, HEIGHT, (int) points[1].x, (int) points[1].y, (int) points[3].x, (int) points[3].y, HOTPINK); // 13
}

uint32_t shade(uint32_t color, int percent_change) {
    // AABBGGRR
    int red = (uint8_t) color;
    int green = (uint8_t) (color >> 8);
    int blue = (uint8_t) (color >> 16);
    int alpha = (uint8_t) (color >> 24);

    red = (100.0 + percent_change)/100.0 * (double)red;
    blue = (100.0 + percent_change)/100.0 * (double)blue;
    green = (100.0+percent_change)/100.0 * (double)green;

    if(red > 255) red = 255;
    if(blue > 255) blue = 255;
    if(green > 255) green = 255;

    uint32_t new_color = (uint8_t)red + ((uint8_t)green << 8) + ((uint8_t)blue << 16) + ((uint8_t)alpha << 24);

    return new_color;
}


void draw_cube(uint32_t *canvas, int width, int height, cube *curr_cube, Point_3D *points, uint32_t color,
               Point_3D camera, int mouseX, int mouseY, int cube_index) {
    Point_2D c_2D[NUM_CORNERS];

    convert_3D_to_2D(c_2D, width, height, points, camera);


    plane p0123 = {points[0], points[1], points[2], points[3],
                          c_2D[0], c_2D[1], c_2D[2], c_2D[3], BG_COLOR};
    plane p5140 = {points[5], points[1], points[4], points[0],
                          c_2D[5], c_2D[1], c_2D[4], c_2D[0], BG_COLOR};
    plane p4062 = {points[4], points[0], points[6], points[2],
                          c_2D[4], c_2D[0], c_2D[6], c_2D[2],BG_COLOR};
    plane p5476 = {points[5], points[4], points[7], points[6],
                          c_2D[5], c_2D[4], c_2D[7], c_2D[6],BG_COLOR};
    plane p5173 = {points[5], points[1], points[7], points[3],
                          c_2D[5], c_2D[1], c_2D[7], c_2D[3],BG_COLOR};
    plane p7362 = {points[7], points[3], points[6], points[2],
                          c_2D[7], c_2D[3], c_2D[6], c_2D[2],BG_COLOR};

    plane planes[NUM_PLANES] = {p0123, p5140, p4062, p5476, p5173, p7362};

//    if(mouseX != -1 && mouseY != -1) set_hovered(curr_cube, planes, mouseX, mouseY);


    for (int i = 0; i < NUM_PLANES; i++) {
        if (i % 3 == 0) planes[i].color = curr_cube->color;
        if (i % 3 == 1) planes[i].color = ((curr_cube->color & 0x00EEEEEE) >> 1) | 0xFF000000;
        if (i % 3 == 2) planes[i].color = ((curr_cube->color & 0x009E9E9E) >> 1) | 0xFF000000;
    }
    if(curr_cube->selected) {
//        curr_cube->color = HOVER_COLOR;
        for(int i=0; i<NUM_PLANES; i++) {
            planes[i].color = shade(planes[i].color, 1000);
        }
    }




    draw_planes(canvas, width, height, planes, NUM_PLANES);
}

void f_modulo(float *x, float y) {
    *x = *x - y * (float) (int) (*x/y);
}

void process_key(int input) {
    float magnitude = (float) 0.01;

    switch(input) {
        // w
        case 1: A += magnitude;
        // a
        case 2: B -= magnitude;
        // s
        case 3: A -= magnitude;
        // d
        case 4: B += magnitude;

        // j
        case 5: C += magnitude;

        case 6: C -= magnitude;
    }

    f_modulo(&A, 2 * (float) PI);
    f_modulo(&B, 2 * (float) PI);
    f_modulo(&C, 2 * (float) PI);


}

void change_center(cube *cubes, int *cube_order) {
    // brute force first one, and try to optimize for rest
    int i;
    cube temp_cubes[CUBES];
    int count = 0;


    for(i = cube_order[0] * CUBES; i < (cube_order[0] + 1) * CUBES; i++) {
        move_color(&temp_cubes[count], &cubes[i]);
        count++;
    }

    count = cube_order[0] * CUBES;
    for(i = cube_order[1] * CUBES; i < (cube_order[1] + 1) * CUBES; i++) {
        move_color(&cubes[count], &cubes[i]);
        count++;
    }

    count = cube_order[1] * CUBES;
    for(i = cube_order[2] * CUBES; i < (cube_order[2] + 1) * CUBES; i++) {
        move_color(&cubes[count], &cubes[i]);
        count++;
    }

    count = cube_order[2] * CUBES;
    for(i = cube_order[3] * CUBES; i < (cube_order[3] + 1) * CUBES; i++) {
        move_color(&cubes[count], &cubes[i]);
        count++;
    }

    count = 0;
    for(i = cube_order[3] * CUBES; i < (cube_order[3] + 1) * CUBES; i++) {
        move_color(&cubes[i], &temp_cubes[count]);
        count++;
    }

}

void rotate_edge(cube *cubes, int select) {

}

void rotate_center(cube *cubes, int select, int order) {

    // order is 4 digit: 3210

    int side_1 = 1;
    int side_2 = 4;
    int side_3 = 6;
    int side_4 = 2;

    int i, j, count;
    cube temp_cubes[9];

    int anchor_1[4] = {1 * CUBES + 8, 1 * CUBES + 7, 1 * CUBES + 6, 9};
    int anchor_2[4] = {2 * CUBES + 6, 2 * CUBES + 3, 2 * CUBES + 0, 9};

//    int anchor_3;
    int anchor_3[4] = {3 * CUBES + 8, 3 * CUBES + 7, 3 * CUBES + 6, -3};


    int anchor_4[4] = {4 * CUBES + 2, 4 * CUBES + 5, 4 * CUBES + 8, 9};

    // int anchor_5;
    int anchor_5[4] = {5 * CUBES + 24, 5 * CUBES + 25, 5 * CUBES + 26, -3};

    int anchor_6[4] = {6 * CUBES + 0, 6 * CUBES + 1, 6 * CUBES + 2, 9};

    int *anchors[7] = {NULL, anchor_1, anchor_2, anchor_3, anchor_4, anchor_5, anchor_6};



    count = 0;

    for(i = 0; i < DIMENSION; i++) {
        for(j = 0; j < DIMENSION; j++) {
            move_color(&temp_cubes[count], &cubes[anchors[side_1][i] + anchors[side_1][3] * j]);
            count++;
        }
    }


    for(i = 0; i < DIMENSION; i++) {
        for(j = 0; j < DIMENSION; j++) {
            move_color(&cubes[anchors[side_1][i] + anchors[side_1][3] * j], &cubes[anchors[side_2][i] + anchors[side_2][3] * j]);
        }
    }


    for(i = 0; i < DIMENSION; i++) {
        for(j = 0; j < DIMENSION; j++) {
            move_color(&cubes[anchors[side_2][i] + anchors[side_2][3] * j], &cubes[anchors[side_3][i] + anchors[side_3][3] * j]);
        }
    }

    for(i = 0; i < DIMENSION; i++) {
        for(j = 0; j < DIMENSION; j++) {
            move_color(&cubes[anchors[side_3][i] + anchors[side_3][3] * j], &cubes[anchors[side_4][i] + anchors[side_4][3] * j]);
        }
    }

    count = 0;
    for(i = 0; i < DIMENSION; i++) {
        for(j = 0; j < DIMENSION; j++) {
            move_color(&cubes[anchors[side_4][i] + anchors[side_4][3] * j], &temp_cubes[count]);
            count++;
        }
    }
}

void rotate_corner(cube *cubes, int select) {

}



void rotate(cube *cubes, int select) {
    // 3 types: corner, edge, center. and core

    // core
    if(select == 13) {
        return;
    }

    if(select % 2 == 1) {
        rotate_edge(cubes, select);
    }
    else if(select == 4 || select == 10 || select == 12 || select == 14 || select == 16 || select == 22) {
        int order;
        switch(select) {
            case 4:
                order = 1462;
            case 10:
                order = 4325;
            case 12:
                order = 6315;
            case 14:
                order = 5136;
            case 16:
                order = 5234;
            case 22:
                order = 2641;
            default:
                order = 0;
        }
        rotate_center(cubes, select, order);
    }
    else {
        rotate_corner(cubes, select);
    }
}




uint32_t *render(int dt, int keyboard_input, float a, float b, float c, int x, int y, int select, int to_rotate) {

//    process_key(keyboard_input);
    found_hovered = 0;
    A = a;
    B = b;
    C = c;

    f_modulo(&A, 2 * (float) PI);
    f_modulo(&B, 2 * (float) PI);
    f_modulo(&C, 2 * (float) PI);

    int num_cubes = CUBES * SIDES;
    cube cubes[num_cubes];
    cube translated_cubes[num_cubes];



    uint32_t colors[SIDES] = {PURPLE, WHITE, ORANGE, BLUE, RED, GREEN, YELLOW, PINK};

    if(dt == 0) {
        resetFaces(cubes, num_cubes, colors);
    }

    if(to_rotate) {
        if(select >= CUBES) {
            int cube_order[4];
            switch(select/CUBES) {
                case 1:
                    cube_order[0] = 1;
                    cube_order[1] = 7;
                    cube_order[2] = 6;
                    cube_order[3] = 0;
                    break;
                case 2:
                    cube_order[0] = 2;
                    cube_order[1] = 7;
                    cube_order[2] = 4;
                    cube_order[3] = 0;
                    break;
                case 3:
                    cube_order[0] = 3;
                    cube_order[1] = 7;
                    cube_order[2] = 5;
                    cube_order[3] = 0;
                    break;
                case 4:
                    cube_order[0] = 4;
                    cube_order[1] = 7;
                    cube_order[2] = 2;
                    cube_order[3] = 0;
                    break;
                case 5:
                    cube_order[0] = 5;
                    cube_order[1] = 7;
                    cube_order[2] = 3;
                    cube_order[3] = 0;
                    break;
                case 6:
                    cube_order[0] = 6;
                    cube_order[1] = 7;
                    cube_order[2] = 1;
                    cube_order[3] = 0;
                    break;
                default:
                    break;
            }
            change_center(cubes, cube_order);
        }
        else {
            rotate(cubes, select);

        }
    }

    if(select >= 0 && select < num_cubes) {
        for(int i=0; i<num_cubes; i++) {
            if(cubes[i].id == select) {
                cubes[select].selected = 1;
            }
            else {
                cubes[i].selected = 0;
            }
        }
    }


    fill_screen(pixels, WIDTH, HEIGHT, BG_COLOR);

    float magnitude = SCALE - GAP;
    float camera_distance = 200;

    Point_3D camera = {0, 0, 0};
    generateCubes(cubes, translated_cubes, num_cubes, magnitude, camera_distance, colors);



    for(int i = 0; i < num_cubes; i++) {
        draw_cube(pixels, WIDTH, HEIGHT, &translated_cubes[i], translated_cubes[i].points, translated_cubes[i].color, camera, x, y, i);
    }


    for(int i=0; i< num_cubes; i++) {
        if(translated_cubes[i].selected == 1){
            Point_2D c_2D[NUM_CORNERS];
            convert_3D_to_2D(c_2D, WIDTH, HEIGHT, translated_cubes[i].points, camera);

            draw_outline(c_2D);
        }
    }

    return pixels;
}




int main(void) {
//    printf("hello world");
    render(0, 0, 1, 0, 0, 397, 301, 4, 1);
}

void *memcpy(void *dest, const void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char *) dest)[i] = ((char *) src)[i];
    }
    return NULL;
}