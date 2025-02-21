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
#define CORE_CUBE 0

#define TO_REVERSE 1

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define PI 3.14159265359
#define RT_2 1.41421356237
#define RT_3 1.73205080757
#define FLT_MAX 3.402823466e+38F

#define SCALE 1.1
#define GAP 0.6

#define BG_COLOR BLACK

// type as in what axis to rotate on
// It will be clockwise when looking at cube from said direction

#define NO_TYPE -1
#define POS_X 0
#define POS_Y 1
#define POS_Z 2
#define NEG_X 3
#define NEG_Y 4
#define NEG_Z 5

#define POS_X_POS_Y 6
#define POS_X_POS_Z 7
#define POS_X_NEG_Y 8
#define POS_X_NEG_Z 9
#define POS_Y_POS_Z 10
#define NEG_X_POS_Y 11
#define POS_Y_NEG_Z 12
#define NEG_X_POS_Z 13
#define NEG_Y_POS_Z 14
#define NEG_X_NEG_Y 15
#define NEG_X_NEG_Z 16
#define NEG_Y_NEG_Z 17

#define NEG_X_NEG_Y_NEG_Z 18
#define NEG_X_NEG_Y_POS_Z 19
#define NEG_X_POS_Y_NEG_Z 20
#define NEG_X_POS_Y_POS_Z 21
#define POS_X_NEG_Y_NEG_Z 22
#define POS_X_NEG_Y_POS_Z 23
#define POS_X_POS_Y_NEG_Z 24
#define POS_X_POS_Y_POS_Z 25

#define MOVE_IN 26

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

void compute_hidden(cube *cubes, int moving_in);


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

float quaternion_x(float i, float j, float k, const double q[4]) {
    return i * (1 - 2 * (q[2] * q[2] + q[3] * q[3])) +
            j * (2 * (q[1] * q[2] - q[3] * q[0])) +
            k * (2 * (q[1] * q[3] + q[2] * q[0]));
}

float quaternion_y(float i, float j, float k, const double q[4]) {
    return i * (2 * (q[1] * q[2] + q[3] * q[0])) +
            j * (1 - 2 * (q[1] * q[1] + q[3] * q[3])) +
            k * (2 * (q[2] * q[3] - q[1] * q[0]));
}

float quaternion_z(float i, float j, float k, const double q[4]) {
    return i * (2 * (q[1] * q[3] - q[2] * q[0])) +
            j * (2 * (q[2] * q[3] + q[1] * q[0])) +
            k * (1 - 2 * (q[1] * q[1] + q[2] * q[2]));
}

void populate_q(double q[4], double sin_value, Point_3D vector) {
    q[1] = sin_value * vector.x;
    q[2] = sin_value * vector.y;
    q[3] = sin_value * vector.z;
}

/*
 *  Function:    generateCorners
 *  ----------------------------
 *  Given the magnitude, camera distance, offsets in the x, y, and z axis,
 *      this function places the correct corner coordinates into the cube
 *      pointer that is passed in. By default, the cube will's center will
 *      be at (0,0,0).
 *
 *  Runtime Complexity: O(1)
 *
 *  Input params:
 *      struct cube *cube:      cube object where the corners will be set
 *      float magnitude:        length of cube size
 *      float camera_distance:  distance of camera from cube
 *      float x_offset:         amount of offset in the x-direction
 *      float y_offset:         amount of offset in the y-direction
 *      float z_offset:         amount of offset in the z-direction
 */
void generateCorners(cube *cubes, float magnitude, float camera_distance, float x_offset, float y_offset, float z_offset,
                     int angle_percent, int type, int select, int curr_cube, int move_in_cube,
                     float dx, float dy, float dz, float separation) {
    float x_factor = -1;
    float y_factor = -1;
    float z_factor = -1;

    for(int i=0; i<NUM_CORNERS; i++) {
        Point_3D newPoint;

        x_factor *= -1;
        if((i % 2) == 0) y_factor *= -1;
        if((i % 4) == 0) z_factor *= -1;

        if(type != MOVE_IN) {
            newPoint.x = x_factor * magnitude + x_offset;
            newPoint.y = y_factor * magnitude + y_offset;
            newPoint.z = z_factor * magnitude + z_offset + camera_distance;
        }

        Point_3D rotatedNewPoint;
        double rad = (angle_percent / 100.0) * (PI / 2.0); // only do 90deg for now
        double angle_of_turn;
        double q[4];

        double sin_value;

        float one_over_rt_2 = (float) (1.0 / RT_2);
        float one_over_rt_3 = (float) (1.0 / RT_3);

        Point_3D vector_pos_x = {1, 0, 0};
        Point_3D vector_neg_x = {-1, 0, 0};
        Point_3D vector_pos_y = {0, 1, 0};
        Point_3D vector_neg_y = {0, -1, 0};
        Point_3D vector_pos_z = {0, 0, 1};
        Point_3D vector_neg_z = {0, 0, -1};

        Point_3D vector_pos_x_pos_y = {one_over_rt_2, one_over_rt_2, 0};
        Point_3D vector_pos_x_pos_z = {one_over_rt_2, 0, one_over_rt_2};
        Point_3D vector_pos_x_neg_y = {one_over_rt_2, -one_over_rt_2, 0};
        Point_3D vector_pos_x_neg_z = {one_over_rt_2, 0, -one_over_rt_2};
        Point_3D vector_pos_y_pos_z = {0, one_over_rt_2, one_over_rt_2};
        Point_3D vector_neg_x_pos_y = {-one_over_rt_2, one_over_rt_2, 0};
        Point_3D vector_pos_y_neg_z = {0, one_over_rt_2, -one_over_rt_2};
        Point_3D vector_neg_x_pos_z = {-one_over_rt_2, 0, one_over_rt_2};
        Point_3D vector_neg_y_pos_z = {0, -one_over_rt_2, one_over_rt_2};
        Point_3D vector_neg_x_neg_y = {-one_over_rt_2, -one_over_rt_2, 0};
        Point_3D vector_neg_x_neg_z = {-one_over_rt_2, 0, -one_over_rt_2};
        Point_3D vector_neg_y_neg_z = {0, -one_over_rt_2, -one_over_rt_2};

        Point_3D vector_neg_x_neg_y_neg_z = {-one_over_rt_3, -one_over_rt_3, -one_over_rt_3};
        Point_3D vector_neg_x_neg_y_pos_z = {-one_over_rt_3, -one_over_rt_3, one_over_rt_3};
        Point_3D vector_neg_x_pos_y_neg_z = {-one_over_rt_3, one_over_rt_3, -one_over_rt_3};
        Point_3D vector_neg_x_pos_y_pos_z = {-one_over_rt_3, one_over_rt_3, one_over_rt_3};
        Point_3D vector_pos_x_neg_y_neg_z = {one_over_rt_3, -one_over_rt_3, -one_over_rt_3};
        Point_3D vector_pos_x_neg_y_pos_z = {one_over_rt_3, -one_over_rt_3, one_over_rt_3};
        Point_3D vector_pos_x_pos_y_neg_z = {one_over_rt_3, one_over_rt_3, -one_over_rt_3};
        Point_3D vector_pos_x_pos_y_pos_z = {one_over_rt_3, one_over_rt_3, one_over_rt_3};

        if(type != MOVE_IN && (angle_percent == 0 || type == NO_TYPE)) {
            cubes->points[i] = newPoint;
        }
        else if(type == MOVE_IN) {

            newPoint.x = x_factor * magnitude + x_offset + dx;
            newPoint.y = y_factor * magnitude + y_offset + dy;
            newPoint.z = z_factor * magnitude + z_offset + camera_distance + dz;

            cubes->points[i] = newPoint;

            rad = (angle_percent / 100.0) * (PI / 2.0);
            sin_value = sin(rad / 2.0);
            q[0] = cos(rad / 2.0);

            switch (move_in_cube) {
                // move-in: 1
                case 1:
                    // rotato potato
                    switch(curr_cube) {
                        case 2:
                            populate_q(q, sin_value, vector_pos_x);
                            break;

                        case 3:
                            populate_q(q, sin_value, vector_neg_z);
                            break;

                        case 4:
                            populate_q(q, sin_value, vector_neg_x);
                            break;

                        case 5:
                            populate_q(q, sin_value, vector_pos_z);
                            break;
                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            // slide pattern
                            newPoint.y = y_factor * magnitude + y_offset + dy + (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                    // move-in: 2
                case 2:
                    switch(curr_cube) {

                        case 1:
                            populate_q(q, sin_value, vector_neg_x);
                            break;

                        case 3:
                            populate_q(q, sin_value, vector_neg_y);
                            break;

                        case 5:
                            populate_q(q, sin_value, vector_pos_y);
                            break;

                        case 6:
                            populate_q(q, sin_value, vector_pos_x);
                            break;

                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            newPoint.z = z_factor * magnitude + z_offset + dz + camera_distance - (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                    // move-in: 3
                case 3:
                    switch(curr_cube) {
                        case 1:
                            populate_q(q, sin_value, vector_pos_z);
                            break;

                        case 2:
                            populate_q(q, sin_value, vector_pos_y);
                            break;

                        case 4:
                            populate_q(q, sin_value, vector_neg_y);
                            break;

                        case 6:
                            populate_q(q, sin_value, vector_neg_z);
                            break;

                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            newPoint.x = x_factor * magnitude + x_offset + dx - (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                    // move-in: 4
                case 4:
                    switch(curr_cube) {
                        case 1:
                            populate_q(q, sin_value, vector_pos_x);
                            break;

                        case 3:
                            populate_q(q, sin_value, vector_pos_y);
                            break;

                        case 5:
                            populate_q(q, sin_value, vector_neg_y);
                            break;

                        case 6:
                            populate_q(q, sin_value, vector_neg_x);
                            break;
                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            newPoint.z = z_factor * magnitude + z_offset + dz + camera_distance + (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                // move-in: 5
                case 5:
                    switch(curr_cube) {
                        case 1:
                            populate_q(q, sin_value, vector_neg_z);
                            break;

                        case 2:
                            populate_q(q, sin_value, vector_neg_y);
                            break;

                        case 4:
                            populate_q(q, sin_value, vector_pos_y);
                            break;

                        case 6:
                            populate_q(q, sin_value, vector_pos_z);
                            break;

                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            newPoint.x = x_factor * magnitude + x_offset + dx + (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                // move-in: 6
                case 6:
                    switch(curr_cube) {
                        case 2:
                            populate_q(q, sin_value, vector_neg_x);
                            break;

                        case 3:
                            populate_q(q, sin_value, vector_pos_z);
                            break;

                        case 4:
                            populate_q(q, sin_value, vector_pos_x);
                            break;

                        case 5:
                            populate_q(q, sin_value, vector_neg_z);
                            break;

                        default:
                            // no quaternion rotation
                            q[0] = 0;
                            q[1] = 0;
                            q[2] = 0;
                            q[3] = 0;

                            newPoint.y = y_factor * magnitude + y_offset + dy - (float) (angle_percent / 100.0 * separation);

                            break;
                    }
                    break;

                default:
                    break;

            }

            rotatedNewPoint.x = quaternion_x(newPoint.x - dx, newPoint.y - dy, newPoint.z - camera_distance - dz, q) + dx;
            rotatedNewPoint.y = quaternion_y(newPoint.x - dx, newPoint.y - dy, newPoint.z - camera_distance - dz, q) + dy;
            rotatedNewPoint.z = quaternion_z(newPoint.x - dx, newPoint.y - dy, newPoint.z - camera_distance - dz, q) + camera_distance + dz;
            cubes->points[i] = rotatedNewPoint;

        }

        else {
            // edge
            if(select % 2 == 1) {
                angle_of_turn = PI;
            }

            // center
            else if(select == 4 || select == 10 || select == 12 || select == 14 || select == 16 || select == 22) {
                angle_of_turn = PI / 2.0;
            }

            // corner
            else {
                angle_of_turn = PI / 1.5;
            }

            rad = (angle_percent / 100.0) * (angle_of_turn);


            sin_value = sin(rad/2.0);
            q[0] = cos(rad/2.0);

            switch(type) {
                // center
                case POS_X:
                    populate_q(q, sin_value, vector_pos_x);
                    break;

                case NEG_X:
                    populate_q(q, sin_value, vector_neg_x);
                    break;

                case POS_Y:
                    populate_q(q, sin_value, vector_pos_y);
                    break;

                case NEG_Y:
                    populate_q(q, sin_value, vector_neg_y);
                    break;

                case POS_Z:
                    populate_q(q, sin_value, vector_pos_z);
                    break;

                case NEG_Z:
                    populate_q(q, sin_value, vector_neg_z);
                    break;

                // edge
                case POS_X_POS_Y:
                    populate_q(q, sin_value, vector_pos_x_pos_y);
                    break;
                case POS_X_POS_Z:
                    populate_q(q, sin_value, vector_pos_x_pos_z);
                    break;
                case POS_X_NEG_Y:
                    populate_q(q, sin_value, vector_pos_x_neg_y);
                    break;
                case POS_X_NEG_Z:
                    populate_q(q, sin_value, vector_pos_x_neg_z);
                    break;
                case POS_Y_POS_Z:
                    populate_q(q, sin_value, vector_pos_y_pos_z);
                    break;
                case NEG_X_POS_Y:
                    populate_q(q, sin_value, vector_neg_x_pos_y);
                    break;
                case POS_Y_NEG_Z:
                    populate_q(q, sin_value, vector_pos_y_neg_z);
                    break;
                case NEG_X_POS_Z:
                    populate_q(q, sin_value, vector_neg_x_pos_z);
                    break;
                case NEG_Y_POS_Z:
                    populate_q(q, sin_value, vector_neg_y_pos_z);
                    break;
                case NEG_X_NEG_Y:
                    populate_q(q, sin_value, vector_neg_x_neg_y);
                    break;
                case NEG_X_NEG_Z:
                    populate_q(q, sin_value, vector_neg_x_neg_z);
                    break;
                case NEG_Y_NEG_Z:
                    populate_q(q, sin_value, vector_neg_y_neg_z);
                    break;

                case NEG_X_NEG_Y_NEG_Z:
                    populate_q(q, sin_value, vector_neg_x_neg_y_neg_z);
                    break;
                case NEG_X_NEG_Y_POS_Z:
                    populate_q(q, sin_value, vector_neg_x_neg_y_pos_z);
                    break;
                case NEG_X_POS_Y_NEG_Z:
                    populate_q(q, sin_value, vector_neg_x_pos_y_neg_z);
                    break;
                case NEG_X_POS_Y_POS_Z:
                    populate_q(q, sin_value, vector_neg_x_pos_y_pos_z);
                    break;
                case POS_X_NEG_Y_NEG_Z:
                    populate_q(q, sin_value, vector_pos_x_neg_y_neg_z);
                    break;
                case POS_X_NEG_Y_POS_Z:
                    populate_q(q, sin_value, vector_pos_x_neg_y_pos_z);
                    break;
                case POS_X_POS_Y_NEG_Z:
                    populate_q(q, sin_value, vector_pos_x_pos_y_neg_z);
                    break;
                case POS_X_POS_Y_POS_Z:
                    populate_q(q, sin_value, vector_pos_x_pos_y_pos_z);
                    break;

                default:
                    break;

            }

            rotatedNewPoint.x = quaternion_x(newPoint.x, newPoint.y, newPoint.z - camera_distance, q);
            rotatedNewPoint.y = quaternion_y(newPoint.x, newPoint.y, newPoint.z - camera_distance, q);
            rotatedNewPoint.z = quaternion_z(newPoint.x, newPoint.y, newPoint.z - camera_distance, q) + camera_distance;
            cubes->points[i] = rotatedNewPoint;
        }
    }
}

/*
 *  Function:    resetFaces
 *  -----------------------
 *  Sets all cubes to default, solved state
 *
 *  Input params:
 *      struct cube *cubes:     pointer to every cube that makes the
 *      int num_cubes:
 *
 *  Return:
 *      uint32_t *pixels: a 2D array containing the pixels to be drawn
 *          onto the screen.
 */
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

/*
 *  Function:    copy_cube
 *  ----------------------
 *  To copy cubes to another array
 *
 *  Input params:
 *      struct cube *dest:      pointer to destination cube (paste)
 *      struct cube *src:       pointer to source cube (copy)
 */
void copy_cube(cube *dest, cube *src) {
    for(int i = 0; i < NUM_CORNERS; i++) {
        dest->points[i] = src->points[i];
    }

    dest->selected = src->selected;
    dest->id = src->id;

}

/*
 *  Function:    generateCubes
 *  --------------------------
 *  given the number of cubes, magnitude, and camera distance, this function generates
 *      the correct corner coordinates for each cube, finds the active planes (i.e.
 *      corner cube has 3 active ones, edges have 2, and faces have 1). Lastly, it
 *      sorts the cubes based on the average z coordinates of the corners so that it
 *      is painted in the right order.
 *
 *  Input params:
 *      struct cube* cubes:     array of cubes that the generated cubes will be stored in
 *      int num_cubes:          number of cubes in the Rubik's Cube
 *      float magnitude:        how long the dimensions of each cube is
 *      float camera_distance:  how far away the camera is from the cube
 *
 *  Return:
 *      Returns a translated array of cubes that takes into account the orientation and
 *          distance the cube with respect to the camera.
 */
cube *generateCubes(cube *cubes, cube *translated_cubes, int num_cubes, float magnitude, float camera_distance,
                    uint32_t *colors, int angle_percent, int type, int select, int move_in_cube) {
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

    // 15
    float separation = 15;

    const int numVectors = 8;

    Point_3D offsetVector0 = {0, 0, 0};
    Point_3D offsetVector1 = {0, -separation, 0};
    Point_3D offsetVector2 = {0, 0, +separation};
    Point_3D offsetVector3 = {+separation, 0, 0};
    Point_3D offsetVector4 = {0, 0, -separation};
    Point_3D offsetVector5 = {-separation, 0, 0};
    Point_3D offsetVector6 = {0, +separation, 0};
    Point_3D offsetVector7 = {0, 0, 0};

    if(type == MOVE_IN) {

        compute_hidden(cubes, move_in_cube);

        switch(move_in_cube) {
            case 1:
                offsetVector7.y = -separation * 2;
                break;
            case 2:
                offsetVector7.z = separation * 2;
                break;
            case 3:
                offsetVector7.x = separation * 2;
                break;
            case 4:
                offsetVector7.z = -separation * 2;
                break;
            case 5:
                offsetVector7.x = -separation * 2;
                break;
            case 6:
                offsetVector7.y = separation * 2;
                break;

            default:
                break;

        }
    }


    Point_3D offsetVectors[8] = {offsetVector0, offsetVector1, offsetVector2, offsetVector3,
                                 offsetVector4, offsetVector5, offsetVector6, offsetVector7};


    for(int vector = 0; vector < numVectors; vector++) {
        for (i = 0; i < DIMENSION; i++) {
            for (j = 0; j < DIMENSION; j++) {
                for (k = 0; k < DIMENSION; k++) {
                    if(type == MOVE_IN) {
                        generateCorners(&translated_cubes[count], magnitude, camera_distance,
                                        x_offset + (spacing * (float) i),
                                        y_offset + (spacing * (float) j),
                                        z_offset + (spacing * (float) k),
                                        angle_percent, type, select, vector, move_in_cube,
                                        offsetVectors[vector].x, offsetVectors[vector].y,
                                        offsetVectors[vector].z, separation);
                    }
                    // rotate core and the 9 closest to it
                    else if(vector == 0 || (vector == 1 && j == DIMENSION - 1) || (vector == 2 && k == 0) ||
                            (vector == 3 && i == 0) || (vector == 4 && k == DIMENSION - 1) ||
                            (vector == 5 && i == DIMENSION -1 || vector == 6 && j == 0)) {
                        generateCorners(&translated_cubes[count], magnitude, camera_distance,
                                        x_offset + (spacing * (float) i) + offsetVectors[vector].x,
                                        y_offset + (spacing * (float) j) + offsetVectors[vector].y,
                                        z_offset + (spacing * (float) k) + offsetVectors[vector].z,
                                        angle_percent, type, select, vector, move_in_cube,
                                        0, 0, 0, separation);
                    }

                    else {
                        generateCorners(&translated_cubes[count], magnitude, camera_distance,
                                        x_offset + (spacing * (float) i) + offsetVectors[vector].x,
                                        y_offset + (spacing * (float) j) + offsetVectors[vector].y,
                                        z_offset + (spacing * (float) k) + offsetVectors[vector].z,
                                        0, NO_TYPE, select, vector, move_in_cube,
                                        0, 0, 0, separation);
                    }
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
            for(j = 0; j < NUM_CORNERS; j++) {
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

/*
 *  Function:   convert_3D_to_2D
 *  ----------------------------
 *  Given the dimensions of the canvas and the set of 3D points, relative to the camera,
 *      it converts the 3D coordinates and places its equivalent 2D points into the canvas
 *      array.
 *
 *  Input params:
 *      struct Point_2D* point2D:   pointer to store the converted 2D points
 *      int width:                  width of canvas
 *      int height:                 height of canvas
 *      struct Point_3D* points:    pointer to 3D points to be converted
 *      int num_corners:            number of corners
 *      struct Point_3D camera:
 *
 */
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

/*
 *  Function:   draw_planes
 *  -----------------------
 *  Given an array of planes, it will draw the planes (via 2 triangles) on the canvas 
 *      in the correct order of back to front by sorting the planes by its z value.
 *
 *  Input params:
 *      uint32_t *canvas:       array of pixels on screen
 *      int width:              width of canvas
 *      int height:             height of canvas
 *      struct plane *p:        array of planes to be drawn
 *      int count:              number of planes (size of struct plane *p)
 */
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
    green = (100.0 + percent_change)/100.0 * (double)green;

    if(red > 255) red = 255;
    if(blue > 255) blue = 255;
    if(green > 255) green = 255;

    uint32_t new_color = (uint32_t)red |
                         ((uint32_t)green << 8) |
                         ((uint32_t)blue << 16) |
                         ((uint32_t)alpha << 24);

    return new_color;
}

Point_3D get_normal_vector_of_plane(plane input) {
    Point_3D point_1 = input.pointA;
    Point_3D point_2 = input.pointB;
    Point_3D point_3 = input.pointC;

    Point_3D u = {point_2.x - point_1.x, point_2.y - point_1.y, point_2.z - point_1.z};
    Point_3D v = {point_3.x - point_1.x, point_3.y - point_1.y, point_3.z - point_1.z};

    Point_3D normal_vector;

    normal_vector.x = u.y * v.z - v.y * u.z;
    normal_vector.y = u.z * v.x - v.z * u.x;
    normal_vector.z = u.x * v.y - v.x * u.y;

    return normal_vector;
}

double dot_product(Point_3D vectorA, Point_3D vectorB) {
    return vectorA.x * vectorB.x + vectorA.y * vectorB.y + vectorA.z * vectorB.z;
}

double d_max(double a, double b) {
    if(a > b) {
        return a;
    }
    return b;
}

void draw_cube(uint32_t *canvas, int width, int height, cube *curr_cube, Point_3D *points, uint32_t color,
               Point_3D camera, int mouseX, int mouseY, int cube_index, int num_cubes, int type, int angle_percent) {
    if((type != MOVE_IN || angle_percent == 0) && curr_cube->id >= num_cubes - CUBES) {
        return;
    }

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

    // shade based on angle with y-axis (light will be at top of -y-axis)
    // take cross product then find angle

    Point_3D camera_vector = {(float) 0, (float) -1, (float) 0};
    Point_3D camera_vector_2 = {(float) 0, (float) 1, (float) 0};

    for (int i = 0; i < NUM_PLANES; i++) {
        Point_3D normal_vector = get_normal_vector_of_plane(planes[i]);
        double dot_prod = d_max(dot_product(camera_vector, normal_vector), dot_product(camera_vector_2, normal_vector));

        planes[i].color = ((curr_cube->color & 0x00EEEEEE) >> 1) | 0xFF000000;

        if(dot_prod < 0.40) {
            dot_prod = 0.40;
        }
        int percent_change = (int) float_abs((float) ((dot_prod * dot_prod) * 200.0));



        planes[i].color = shade(planes[i].color, percent_change);



//        if (i % 3 == 0) planes[i].color = curr_cube->color;
//        if (i % 3 == 1) planes[i].color = ((curr_cube->color & 0x00EEEEEE) >> 1) | 0xFF000000;
//        if (i % 3 == 2) planes[i].color = ((curr_cube->color & 0x009E9E9E) >> 1) | 0xFF000000;

    }
    if(curr_cube->selected) {
        for(int i=0; i<NUM_PLANES; i++) {
            planes[i].color = shade(planes[i].color, 100);
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





void rotate_center(cube *cubes, int select, int order, int **anchors) {

    // order is 4 digit: 3210
    int side_1 = (order / 1000) % 10;
    int side_2 = (order / 100) % 10;
    int side_3 = (order / 10) % 10;
    int side_4 = order % 10;

    int i, j, count;
    cube temp_cubes[DIMENSION * DIMENSION];

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

void flip(int *to_reverse) {
    // swap indexes 0 and 3; 1 and 2
    int temp;

    temp = to_reverse[0];
    to_reverse[0] = to_reverse[3];
    to_reverse[3] = temp;

    temp = to_reverse[1];
    to_reverse[1] = to_reverse[2];
    to_reverse[2] = temp;
}

void rotate_self(cube *cubes, int cube_num, int corner_anchors[4], int edge_anchors[4], int increment, int reverse) {
    // rotate corners
    int i, j, count;
    cube temp_cubes[DIMENSION];

    int anchor_corners[5] = {cube_num * CUBES + corner_anchors[0], cube_num * CUBES + corner_anchors[1],
                             cube_num * CUBES + corner_anchors[2], cube_num * CUBES + corner_anchors[3],
                             increment};

    int anchor_edges[5] = {cube_num * CUBES + edge_anchors[0], cube_num * CUBES + edge_anchors[1],
                           cube_num * CUBES + edge_anchors[2], cube_num * CUBES + edge_anchors[3],
                           increment};

    if(reverse) {
        flip(anchor_corners);
        flip(anchor_edges);
    }

    count = 0;
    for(i = 0; i < DIMENSION; i++) {
        move_color(&temp_cubes[count], &cubes[anchor_corners[0] + anchor_corners[4] * i]);
        count++;
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_corners[0] + anchor_corners[4] * i], &cubes[anchor_corners[1] + anchor_corners[4] * i]);
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_corners[1] + anchor_corners[4] * i], &cubes[anchor_corners[2] + anchor_corners[4] * i]);
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_corners[2] + anchor_corners[4] * i], &cubes[anchor_corners[3] + anchor_corners[4] * i]);
    }
    count = 0;
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_corners[3] + anchor_corners[4] * i], &temp_cubes[count]);
        count++;
    }


    // rotate edges
    count = 0;
    for(i = 0; i < DIMENSION; i++) {
        move_color(&temp_cubes[count], &cubes[anchor_edges[0] + anchor_edges[4] * i]);
        count++;
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_edges[0] + anchor_edges[4] * i], &cubes[anchor_edges[1] + anchor_edges[4] * i]);
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_edges[1] + anchor_edges[4] * i], &cubes[anchor_edges[2] + anchor_edges[4] * i]);
    }
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_edges[2] + anchor_edges[4] * i], &cubes[anchor_edges[3] + anchor_edges[4] * i]);
    }
    count = 0;
    for(i = 0; i < DIMENSION; i++) {
        move_color(&cubes[anchor_edges[3] + anchor_edges[4] * i], &temp_cubes[count]);
        count++;
    }

}

int last_move_in_axis = NO_TYPE;

void compute_hidden(cube *cubes, int moving_in) {

    if(last_move_in_axis == NO_TYPE) {
        return;
    }

    int reverse;

    int corner_anchors_x[4] = {0, 6, 8, 2};
    int edge_anchors_x[4] = {3, 7, 5, 1};
    int increment_x = 9;

    int corner_anchors_y[4] = {0, 2, 20, 18};
    int edge_anchors_y[4] = {1, 11, 19, 9};
    int increment_y = 3;

    int corner_anchors_z[4] = {18, 24, 6, 0};
    int edge_anchors_z[4] = {9, 21, 15, 3};
    int increment_z = 1;

    if(moving_in == 1 || moving_in == 6) {
        reverse = (moving_in == 1);

        if (Y_AXIS != last_move_in_axis) {
            if (last_move_in_axis == X_AXIS) {
                rotate_self(cubes, 7, corner_anchors_z, edge_anchors_z, increment_z, reverse);
                rotate_self(cubes, 7, corner_anchors_z, edge_anchors_z, increment_z, reverse);
            } else if (last_move_in_axis == Z_AXIS) {
                rotate_self(cubes, 7, corner_anchors_x, edge_anchors_x, increment_x, reverse);
                rotate_self(cubes, 7, corner_anchors_x, edge_anchors_x, increment_x, reverse);
            }
            last_move_in_axis = Y_AXIS;
        }
    }

    if(moving_in == 2 || moving_in == 4) {
        reverse = (moving_in == 2);

        if (Z_AXIS != last_move_in_axis) {
            if(last_move_in_axis == X_AXIS) {
                rotate_self(cubes, 7, corner_anchors_y, edge_anchors_y, increment_y, reverse);
                rotate_self(cubes, 7, corner_anchors_y, edge_anchors_y, increment_y, reverse);
            } else if(last_move_in_axis == Y_AXIS) {
                rotate_self(cubes, 7, corner_anchors_x, edge_anchors_x, increment_x, reverse);
                rotate_self(cubes, 7, corner_anchors_x, edge_anchors_x, increment_x, reverse);
            }
            last_move_in_axis = Z_AXIS;
        }
    }

    if(moving_in == 3 || moving_in == 5) {
        reverse = (moving_in == 3);

        if (X_AXIS != last_move_in_axis) {
            if(last_move_in_axis == Y_AXIS) {
                rotate_self(cubes, 7, corner_anchors_z, edge_anchors_z, increment_z, reverse);
                rotate_self(cubes, 7, corner_anchors_z, edge_anchors_z, increment_z, reverse);
            } else if(last_move_in_axis == Z_AXIS) {
                rotate_self(cubes, 7, corner_anchors_y, edge_anchors_y, increment_y, reverse);
                rotate_self(cubes, 7, corner_anchors_y, edge_anchors_y, increment_y, reverse);
            }
            last_move_in_axis = X_AXIS;
        }
    }



}

void change_center(cube *cubes, const int *cube_order) {

    int moving_in = cube_order[0];

    int corner_anchors_x[4] = {0, 6, 8, 2};
    int edge_anchors_x[4] = {3, 7, 5, 1};
    int increment_x = 9;

    int corner_anchors_y[4] = {0, 2, 20, 18};
    int edge_anchors_y[4] = {1, 11, 19, 9};
    int increment_y = 3;

    int corner_anchors_z[4] = {18, 24, 6, 0};
    int edge_anchors_z[4] = {9, 21, 15, 3};
    int increment_z = 1;

    int reverse;


    // i think the 7th cube also rotates here.. need to figure out what axis and direction
    if(moving_in == 1 || moving_in == 6) {
        if(last_move_in_axis == NO_TYPE) {
            last_move_in_axis = Y_AXIS;
        }

        reverse = (moving_in == 1);

        rotate_self(cubes, 2, corner_anchors_x, edge_anchors_x, increment_x, reverse);
        rotate_self(cubes, 4, corner_anchors_x, edge_anchors_x, increment_x, !reverse);

        rotate_self(cubes, 3, corner_anchors_z, edge_anchors_z, increment_z, !reverse);
        rotate_self(cubes, 5, corner_anchors_z, edge_anchors_z, increment_z, reverse);
    }

    if(moving_in == 2 || moving_in == 4) {
        if(last_move_in_axis == NO_TYPE) {
            last_move_in_axis = Z_AXIS;
        }

        reverse = (moving_in == 2);

        rotate_self(cubes, 1, corner_anchors_x, edge_anchors_x, increment_x, !reverse);
        rotate_self(cubes, 6, corner_anchors_x, edge_anchors_x, increment_x, reverse);

        rotate_self(cubes, 3, corner_anchors_y, edge_anchors_y, increment_y, !reverse);
        rotate_self(cubes, 5, corner_anchors_y, edge_anchors_y, increment_y, reverse);
    }

    if(moving_in == 3 || moving_in == 5) {
        if(last_move_in_axis == NO_TYPE) {
            last_move_in_axis = X_AXIS;
        }


        reverse = (moving_in == 3);

        rotate_self(cubes, 1, corner_anchors_z, edge_anchors_z, increment_z, reverse);
        rotate_self(cubes, 6, corner_anchors_z, edge_anchors_z, increment_z, !reverse);

        rotate_self(cubes, 2, corner_anchors_y, edge_anchors_y, increment_y, reverse);
        rotate_self(cubes, 4, corner_anchors_y, edge_anchors_y, increment_y, !reverse);
    }

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

void rotate_sandwich(cube *cubes, int bread, int anchor_1[8], int anchor_2[8], int reversed) {
    if(reversed) {
        flip(anchor_1);
        flip(&anchor_1[4]);
        flip(anchor_2);
        flip(&anchor_2[4]);
    }

    cube temp_cube;

    move_color(&temp_cube, &cubes[anchor_1[0]]);
    move_color(&cubes[anchor_1[0]], &cubes[anchor_1[1]]);
    move_color(&cubes[anchor_1[1]], &cubes[anchor_1[2]]);
    move_color(&cubes[anchor_1[2]], &cubes[anchor_1[3]]);
    move_color(&cubes[anchor_1[3]], &temp_cube);

    move_color(&temp_cube, &cubes[anchor_1[4]]);
    move_color(&cubes[anchor_1[4]], &cubes[anchor_1[5]]);
    move_color(&cubes[anchor_1[5]], &cubes[anchor_1[6]]);
    move_color(&cubes[anchor_1[6]], &cubes[anchor_1[7]]);
    move_color(&cubes[anchor_1[7]], &temp_cube);


    move_color(&temp_cube, &cubes[anchor_2[0]]);
    move_color(&cubes[anchor_2[0]], &cubes[anchor_2[1]]);
    move_color(&cubes[anchor_2[1]], &cubes[anchor_2[2]]);
    move_color(&cubes[anchor_2[2]], &cubes[anchor_2[3]]);
    move_color(&cubes[anchor_2[3]], &temp_cube);

    move_color(&temp_cube, &cubes[anchor_2[4]]);
    move_color(&cubes[anchor_2[4]], &cubes[anchor_2[5]]);
    move_color(&cubes[anchor_2[5]], &cubes[anchor_2[6]]);
    move_color(&cubes[anchor_2[6]], &cubes[anchor_2[7]]);
    move_color(&cubes[anchor_2[7]], &temp_cube);
}

void rotate_edge(cube *cubes, int select, const int anchor_a[4], const int anchor_b[4]) {
    
    int i, j;
    cube temp_cube;

    for(i=0; i < DIMENSION; i++) {
        for(j=0; j < DIMENSION; j++) {
            move_color(&temp_cube, &cubes[anchor_a[i] + anchor_a[3] * j]);
            move_color(&cubes[anchor_a[i] + anchor_a[3] * j], &cubes[anchor_b[i] + anchor_b[3] * j]);
            move_color(&cubes[anchor_b[i] + anchor_b[3] * j], &temp_cube);
        }
    }

}

void rotate_center_piece(cube *cubes, int select) {
    int order;
    int bread;

    // for rotate self
    int corner_anchors_x[4] = {0, 6, 8, 2};
    int edge_anchors_x[4] = {3, 7, 5, 1};
    int increment_x = 9;

    int corner_anchors_y[4] = {0, 2, 20, 18};
    int edge_anchors_y[4] = {1, 11, 19, 9};
    int increment_y = 3;

    int corner_anchors_z[4] = {18, 24, 6, 0};
    int edge_anchors_z[4] = {9, 21, 15, 3};
    int increment_z = 1;

    // for sandwich turns
    int sandwich_1[8] = {1 * CUBES + 6, 1 * CUBES + 8, 1 * CUBES + 26, 1 * CUBES + 24, 1 * CUBES + 7, 1 * CUBES + 17, 1 * CUBES + 25, 1 * CUBES + 15};
    int sandwich_2[8] = {2 * CUBES + 6, 2 * CUBES + 0, 2 * CUBES + 18, 2 * CUBES + 24, 2 * CUBES + 3, 2 * CUBES + 9, 2 * CUBES + 21, 2 * CUBES + 15};
    int sandwich_3[8] = {3 * CUBES + 2, 3 * CUBES + 0, 3 * CUBES + 6, 3 * CUBES + 8, 3 * CUBES + 5, 3 * CUBES + 1, 3 * CUBES + 3, 3 * CUBES + 7};
    int sandwich_4[8] = {4 * CUBES + 2, 4 * CUBES + 20, 4 * CUBES + 26, 4 * CUBES + 8, 4 * CUBES + 5, 4 * CUBES + 11, 4 * CUBES + 23, 4 * CUBES + 17};
    int sandwich_5[8] = {5 * CUBES + 20, 5 * CUBES + 18, 5 * CUBES + 24, 5 * CUBES + 26, 5 * CUBES + 23, 5 * CUBES + 19, 5 * CUBES + 21, 5 * CUBES + 25};
    int sandwich_6[8] = {6 * CUBES + 0, 6 * CUBES + 2, 6 * CUBES + 20, 6 * CUBES + 18, 6 * CUBES + 1, 6 * CUBES + 11, 6 * CUBES + 19, 6 * CUBES + 9};



    switch(select) {
        case 4:
            // about x
            order = 1462;
            bread = 35;

            int anchor_1_x[4] = {1 * CUBES + 8, 1 * CUBES + 7, 1 * CUBES + 6, 9};
            int anchor_4_x[4] = {4 * CUBES + 2, 4 * CUBES + 5, 4 * CUBES + 8, 9};
            int anchor_6_x[4] = {6 * CUBES + 0, 6 * CUBES + 1, 6 * CUBES + 2, 9};
            int anchor_2_x[4] = {2 * CUBES + 6, 2 * CUBES + 3, 2 * CUBES + 0, 9};
            int *anchors_x[7] = {NULL, anchor_1_x, anchor_2_x, NULL, anchor_4_x, NULL, anchor_6_x};

            rotate_center(cubes, select, order, anchors_x);
            rotate_sandwich(cubes, bread, sandwich_3, sandwich_5, !TO_REVERSE);
            rotate_self(cubes, 0, corner_anchors_x, edge_anchors_x, increment_x, !TO_REVERSE); // correct

            break;
        case 10:
            // about y
            order = 5234;
            bread = 35;

            int anchor_5_y[4] = {5 * CUBES + 18, 5 * CUBES + 19, 5 * CUBES + 20, 3};
            int anchor_2_y[4] = {2 * CUBES + 0, 2 * CUBES + 9, 2 * CUBES + 18, 3};
            int anchor_3_y[4] = {3 * CUBES + 2, 3 * CUBES + 1, 3 * CUBES + 0, 3};
            int anchor_4_y[4] = {4 * CUBES + 20, 4 * CUBES + 11, 4 * CUBES + 2, 3};
            int *anchors_y[7] = {NULL, NULL, anchor_2_y, anchor_3_y, anchor_4_y, anchor_5_y, NULL};

            rotate_center(cubes, select, order, anchors_y);
            rotate_sandwich(cubes, bread, sandwich_1, sandwich_6, !TO_REVERSE);
            rotate_self(cubes, 0, corner_anchors_y, edge_anchors_y, increment_y, !TO_REVERSE); // correct

            break;
        case 12:
            // about z
            order = 5136;
            bread = 0;

            int anchor_5_z[4] = {5 * CUBES + 26, 5 * CUBES + 23, 5 * CUBES + 20, -1};
            int anchor_1_z[4] = {1 * CUBES + 8, 1 * CUBES + 17, 1 * CUBES + 26, -1};
            int anchor_3_z[4] = {3 * CUBES + 2, 3 * CUBES + 5, 3 * CUBES + 8, -1};
            int anchor_6_z[4] = {6 * CUBES + 20, 6 * CUBES + 11, 6 * CUBES + 2, -1};
            int *anchors_z[7] = {NULL, anchor_1_z, NULL, anchor_3_z, NULL, anchor_5_z, anchor_6_z};

            rotate_center(cubes, select, order, anchors_z);

            rotate_sandwich(cubes, bread, sandwich_2, sandwich_4, !TO_REVERSE);

            rotate_self(cubes, 0, corner_anchors_z, edge_anchors_z, increment_z, !TO_REVERSE); // correct
            break;
        case 14:
            // about z
            order = 6315;

            int anchor_5_z_[4] = {5 * CUBES + 26, 5 * CUBES + 23, 5 * CUBES + 20, -1};
            int anchor_1_z_[4] = {1 * CUBES + 8, 1 * CUBES + 17, 1 * CUBES + 26, -1};
            int anchor_3_z_[4] = {3 * CUBES + 2, 3 * CUBES + 5, 3 * CUBES + 8, -1};
            int anchor_6_z_[4] = {6 * CUBES + 20, 6 * CUBES + 11, 6 * CUBES + 2, -1};
            int *anchors_z_[7] = {NULL, anchor_1_z_, NULL, anchor_3_z_, NULL, anchor_5_z_, anchor_6_z_};

            rotate_center(cubes, select, order, anchors_z_);

            rotate_sandwich(cubes, bread, sandwich_2, sandwich_4, TO_REVERSE);

            rotate_self(cubes, 0, corner_anchors_z, edge_anchors_z, increment_z, TO_REVERSE); // reverse
            break;
        case 16:
            // about y
            order = 4325;

            int anchor_5_y_[4] = {5 * CUBES + 18, 5 * CUBES + 19, 5 * CUBES + 20, 3};
            int anchor_2_y_[4] = {2 * CUBES + 0, 2 * CUBES + 9, 2 * CUBES + 18, 3};
            int anchor_3_y_[4] = {3 * CUBES + 2, 3 * CUBES + 1, 3 * CUBES + 0, 3};
            int anchor_4_y_[4] = {4 * CUBES + 20, 4 * CUBES + 11, 4 * CUBES + 2, 3};
            int *anchors_y_[7] = {NULL, NULL, anchor_2_y_, anchor_3_y_, anchor_4_y_, anchor_5_y_, NULL};

            rotate_center(cubes, select, order, anchors_y_);

            rotate_sandwich(cubes, bread, sandwich_1, sandwich_6, TO_REVERSE);

            rotate_self(cubes, 0, corner_anchors_y, edge_anchors_y, increment_y, TO_REVERSE); // reverse
            break;
        case 22:
            // about x
            order = 2641;

            int anchor_1_x_[4] = {1 * CUBES + 8, 1 * CUBES + 7, 1 * CUBES + 6, 9};
            int anchor_4_x_[4] = {4 * CUBES + 2, 4 * CUBES + 5, 4 * CUBES + 8, 9};
            int anchor_6_x_[4] = {6 * CUBES + 0, 6 * CUBES + 1, 6 * CUBES + 2, 9};
            int anchor_2_x_[4] = {2 * CUBES + 6, 2 * CUBES + 3, 2 * CUBES + 0, 9};
            int *anchors_x_[7] = {NULL, anchor_1_x_, anchor_2_x_, NULL, anchor_4_x_, NULL, anchor_6_x_};

            rotate_center(cubes, select, order, anchors_x_);

            rotate_sandwich(cubes, bread, sandwich_3, sandwich_5, TO_REVERSE);

            rotate_self(cubes, 0, corner_anchors_x, edge_anchors_x, increment_x, TO_REVERSE); // reverse
            break;
        default:
            order = 0;
    }
}



void rotate(cube *cubes, int select) {
    // 3 types: corner, edge, center. and core

    // for rotate self
    int corner_anchors_x[4] = {0, 6, 8, 2};
    int edge_anchors_x[4] = {3, 7, 5, 1};
    int increment_x = 9;

    int corner_anchors_y[4] = {0, 2, 20, 18};
    int edge_anchors_y[4] = {1, 11, 19, 9};
    int increment_y = 3;

    int corner_anchors_z[4] = {18, 24, 6, 0};
    int edge_anchors_z[4] = {9, 21, 15, 3};
    int increment_z = 1;


    // core
    if(select == 13) {
        return;
    }
    int reverse = 1;
    // edge
    if(select % 2 == 1) {
        switch(select) {
            case 3:
            case 23:
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 12);
                break;

            case 5:
            case 21:
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 4);
                rotate_center_piece(cubes, 4);
                break;

            case 9:
            case 17:
                rotate_center_piece(cubes, 4);
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 10);
                break;

            case 11:
            case 15:
                rotate_center_piece(cubes, 4);
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 12);
                break;

            case 1:
            case 25:
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 4);
                rotate_center_piece(cubes, 4);

                break;

            case 7:
            case 19:
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 10);

                break;

            default:
                break;
        }
    }
    // center
    else if(select == 4 || select == 10 || select == 12 || select == 14 || select == 16 || select == 22) {
        rotate_center_piece(cubes, select);
    }
    else {
        switch(select) {
            case 0:
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 4);

                break;

            case 2:
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 4);

                break;

            case 6:
                rotate_center_piece(cubes, 12);
                rotate_center_piece(cubes, 16);

                break;

            case 8:
                rotate_center_piece(cubes, 16);
                rotate_center_piece(cubes, 14);
                break;

            case 18:
                rotate_center_piece(cubes, 10);
                rotate_center_piece(cubes, 22);
                break;

            case 20:
                rotate_center_piece(cubes, 14);
                rotate_center_piece(cubes, 22);
                break;

            case 24:
                rotate_center_piece(cubes, 22);
                rotate_center_piece(cubes, 16);
                break;

            case 26:
                rotate_center_piece(cubes, 22);
                rotate_center_piece(cubes, 14);
                break;

            default:
                break;
        }
    }

}

int select_current_type(int select) {
    switch(select) {
        case 0:
            // Code for case 0
            return NEG_X_NEG_Y_NEG_Z;
        case 1:
            // Code for case 1
            return NEG_X_NEG_Y;
        case 2:
            // Code for case 2
            return NEG_X_NEG_Y_POS_Z;
        case 3:
            // Code for case 3
            return NEG_X_NEG_Z;
        case 4:
            // Code for case 4
            return NEG_X;
        case 5:
            // Code for case 5
            return NEG_X_POS_Z;
        case 6:
            // Code for case 6
            return NEG_X_POS_Y_NEG_Z;
        case 7:
            // Code for case 7
            return NEG_X_POS_Y;
        case 8:
            // Code for case 8
            return NEG_X_POS_Y_POS_Z;
        case 9:
            // Code for case 9
            return NEG_Y_NEG_Z;
        case 10:
            // Code for case 10
            return NEG_Y;
        case 11:
            // Code for case 11
            return NEG_Y_POS_Z;
        case 12:
            // Code for case 12
            return NEG_Z;
        case 13:
            // Code for case 13
            return NO_TYPE;
        case 14:
            // Code for case 14
            return POS_Z;
        case 15:
            // Code for case 15
            return POS_Y_NEG_Z;
        case 16:
            // Code for case 16
            return POS_Y;
        case 17:
            // Code for case 17
            return POS_Y_POS_Z;
        case 18:
            // Code for case 18
            return POS_X_NEG_Y_NEG_Z;
        case 19:
            // Code for case 19
            return POS_X_NEG_Y;
        case 20:
            // Code for case 20
            return POS_X_NEG_Y_POS_Z;
        case 21:
            // Code for case 21
            return POS_X_NEG_Z;
        case 22:
            // Code for case 22
            return POS_X;
        case 23:
            // Code for case 23
            return POS_X_POS_Z;
        case 24:
            // Code for case 24
            return POS_X_POS_Y_NEG_Z;
        case 25:
            // Code for case 25
            return POS_X_POS_Y;
        case 26:
            // Code for case 26
            return POS_X_POS_Y_POS_Z;
        default:
            // Code for default case
            return -1;
    }

}



int move_in_cube;

uint32_t *render(int dt, int keyboard_input, float a, float b, float c, int x, int y, int select, int to_rotate,
                 int angle_percent, int type) {

//    process_key(keyboard_input);
//    found_hovered = 0;
    A = a;
    B = b;
    C = c;

    f_modulo(&A, 2 * (float) PI);
    f_modulo(&B, 2 * (float) PI);
    f_modulo(&C, 2 * (float) PI);

    int num_cubes = CUBES * SIDES;
    cube cubes[num_cubes];
    cube translated_cubes[num_cubes];

    int current_type;

    uint32_t colors[SIDES] = {PURPLE, WHITE, CADMIUM_ORANGE, BLUE, RED, GREEN, YELLOW, PINK};

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
    else {
        if(select >= CUBES) {
            current_type = MOVE_IN;
            move_in_cube = select / CUBES;
        }
        else {
            current_type = select_current_type(select);
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
    generateCubes(cubes, translated_cubes, num_cubes, magnitude, camera_distance, colors, angle_percent, current_type, select, move_in_cube);



    for(int i = 0; i < num_cubes; i++) {
        draw_cube(pixels, WIDTH, HEIGHT, &translated_cubes[i], translated_cubes[i].points, translated_cubes[i].color, camera,
                  x, y, i, num_cubes, current_type, angle_percent);
    }


    for(int i=0; i< num_cubes; i++) {
        if(translated_cubes[i].selected == 1){
            Point_2D c_2D[NUM_CORNERS];
            convert_3D_to_2D(c_2D, WIDTH, HEIGHT, translated_cubes[i].points, camera);

            draw_outline(c_2D);
            break;
        }
    }

    return pixels;
}




int main(void) {
//    printf("hello world");
    render(0, 0, 1, 0, 0, 397, 301, 4, 0, 30, 0);
}

void *memcpy(void *dest, const void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char *) dest)[i] = ((char *) src)[i];
    }
    return NULL;
}
