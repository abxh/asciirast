#include "transform.h"

typedef struct {
    vec2_type c;
    vec2_type v;
    float t;
} line_segment; // f(t) = c + t * v, t in [0;1]

line_segment from_points_to_line_segment(vec2_type v[2]) {
    const vec2_type c = v[0];
    

}

bool check_intersection_between_line_segments(line_segment l0, line_segment l1, float* res_t);
