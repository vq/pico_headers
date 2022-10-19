/*
    @file pico_unit.h
    @brief Separating Axis Test (SAT) written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------
*/

#ifndef PICO_SAT_H
#define PICO_SAT_H

#include <float.h>

#include "pico_math.h"

#ifndef PICO_SAT_MAX_POLY_VERTS
#define PICO_SAT_MAX_POLY_VERTS 8
#endif

typedef struct
{
    pm_v2  pos;
    pm_float radius;
} sat_circle_t;

typedef struct
{
    int   vertex_count;
    pm_v2 vertices[PICO_SAT_MAX_POLY_VERTS];
    pm_v2 normals[PICO_SAT_MAX_POLY_VERTS];
    pm_v2 edges[PICO_SAT_MAX_POLY_VERTS];
} sat_poly_t;

typedef struct
{
    pm_v2    normal;
    pm_float depth[2];
    pm_v2    contacts[2];
    int      count;
} sat_manifold_t;

//later
//pm_b2 sat_polygon_to_aabb(const sat_polygon_t* poly);
//pm_b2 sat_circle_to_aabb(const sat_circle_t* circle);

sat_circle_t sat_make_cicle(pm_v2 pos, pm_float radius);
sat_poly_t sat_make_polygon(int vertex_count, pm_v2 vertices[]);
sat_poly_t sat_aabb_to_poly(const pm_b2* aabb);

bool sat_test_poly_poly(const sat_poly_t* p1,
                        const sat_poly_t* p2,
                        sat_manifold_t* manifold);

bool sat_test_circle_poly(const sat_circle_t* c,
                          const sat_poly_t* p,
                          sat_manifold_t* manifold);

bool sat_test_poly_circle(const sat_poly_t* p,
                          const sat_circle_t* c,
                          sat_manifold_t* manifold);

bool sat_test_circle_circle(const sat_circle_t* c1,
                            const sat_circle_t* c2,
                            sat_manifold_t* manifold);

#endif // PICO_SAT_H

#ifdef PICO_SAT_IMPLEMENTATION

void sat_axis_range(const sat_poly_t* poly, pm_v2 normal, pm_float range[2])
{
    pm_float dot = pm_v2_dot(poly->vertices[0], normal);
    pm_float min = dot;
    pm_float max = dot;

    for (int i = 1; i < poly->vertex_count; i++)
    {
        dot = pm_v2_dot(poly->vertices[i], normal);

        if (dot < min)
            min = dot;

        if (dot > max)
            max = dot;
    }

    range[0] = min;
    range[1] = max;
}

pm_float sat_axis_overlap(const sat_poly_t* p1,
                          const sat_poly_t* p2,
                          pm_v2 axis)

{
    pm_float range1[2];
    pm_float range2[2];

    sat_axis_range(p1, axis, range1);
    sat_axis_range(p2, axis, range2);

    if (range1[1] < range2[0] || range2[1] < range1[0])
        return 0.0f;

    pm_float depth1 = range1[1] - range2[0];
    pm_float depth2 = range2[1] - range1[0];

    return (depth2 > depth1) ? depth1 : -depth2;
}

pm_float sat_


sat_circle_t sat_make_cicle(pm_v2 pos, pm_float radius)
{
    sat_circle_t circle;
    circle.pos = pos;
    circle.radius = radius;
    return circle;
}

sat_poly_t sat_make_poly(int vertex_count, pm_v2 vertices[])
{
    //assert(vertex_count <= PICO_SAT_MAX_POLY_VERTS);

    sat_poly_t poly;

    poly.vertex_count = vertex_count;

    for (int i = 0; i < vertex_count; i++)
    {
        poly.vertices[i] = vertices[i];
    }

    for (int i = 0; i < vertex_count - 1; i++)
    {
        pm_v2 v1 = vertices[i];
        pm_v2 v2 = (i < vertex_count - 1) ? vertices[i + 1] : vertices[0];
        poly.edges[i] = pm_v2_sub(v2, v1);
        poly.normals[i] = pm_v2_neg(pm_v2_perp(poly.edges[i]));
        poly.normals[i] = pm_v2_normalize(poly.normals[i]);
    }

    return poly;
}

sat_poly_t sat_aabb_to_poly(const pm_b2* aabb)
{
    pm_v2 pos = pm_b2_pos(aabb);
    pm_v2 size = pm_b2_size(aabb);

    pm_v2 vertices[] =
    {
        { pos.x, pos.y                   },
        { pos.x,          pos.y + size.y },
        { pos.x + size.x, pos.y + size.y },
        { pos.x + size.x, pos.y          }
    };

    return sat_make_poly(4, vertices);
}

/*void sat_update_normal(pm_float signed_depth,
                       pm_float* depth,
                       pm_v2 normal_in,
                       pm_v2* normal_out)
{
    pm_float abs_depth = pm_abs(signed_depth);

    if (abs_depth < *depth)
    {
        *depth = abs_depth;

        if (signed_depth < 0.0f)
            *normal_out = pm_v2_neg(normal_in);
        else
            *normal_out = normal_in;
    }
}*/

bool sat_test_poly_poly(const sat_poly_t* p1,
                        const sat_poly_t* p2,
                        sat_manifold_t* manifold)
{
    for (int i = 0; i < p1->vertex_count - 1; i++)
    {
        pm_float overlap = sat_axis_overlap(p1, p2, p1->normals[i]);

        if (overlap == 0.0f)
            return false;

    }

    for (int i = 0; i < p2->vertex_count - 1; i++)
    {
        pm_float overlap = sat_axis_overlap(p2, p1, p2->normals[i]);

        if (overlap == 0.0f)
            return false;
    }

    return true;
}

bool sat_test_poly_circle(const sat_poly_t* p,
                          const sat_circle_t* c,
                          sat_manifold_t* manifold)
{


}


#endif // PICO_SAT_IMPLEMENTATION

