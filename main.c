// Sokol libraries
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "cimgui.h"
#include "sokol_imgui.h"

// Helper library for general math operations
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SIMD
#include "HandmadeMath.h"

// Shader code - generated using sokol-shdc (see build.sh)
#include "terrain_shader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG

typedef enum {
    SOLID = 0,
    WIREFRAME,
    NUM_DRAW_MODES
} draw_mode;

typedef struct {
    HMM_Vec3 position;
    float* vertices;
    uint16_t* indices;
    int num_vertices;
    int num_indices;
    draw_mode current_draw_mode;
} plane_t;

static struct {
    sg_pipeline pip[NUM_DRAW_MODES];
    sg_pass_action pass_action;
    sg_bindings bind;
    plane_t plane;
    vs_params_t vs_params;
    float ry;
    bool wireframe;
} state;

void destroy_plane(plane_t plane) {
    free(plane.vertices);
    free(plane.indices);
}

plane_t make_plane(HMM_Vec3 position, int div, float width, draw_mode mode) {
    plane_t plane = {
        .position = position,
        .current_draw_mode = mode
    };
    // Implemented with reference to: https://www.youtube.com/watch?v=FKLbihqDLsg&ab_channel=VictorGordan

    // Construct vertices
    plane.num_vertices = (div + 1) * (div + 1);
    plane.vertices = malloc(sizeof(float) * plane.num_vertices * 3);

    float triangle_side = width / div;
    float center = width * 0.5f;
    int count = 0;
    for (int row = 0; row < div + 1; row++) {
        for (int col = 0; col < div + 1; col++) {
            // Arrange vertices around the center point of the plane
            float x = col * triangle_side;
            float y = 0.0;
            float z = row * triangle_side;
            plane.vertices[count + 0] = x - center;
            plane.vertices[count + 1] = y;
            plane.vertices[count + 2] = z - center;

            count += 3;
        }
    }

#ifdef DEBUG
    /*
    for (int i = 0; i < plane.num_vertices;  i++) {
        float x = plane.vertices[3 * i + 0];
        float y = plane.vertices[3 * i + 1] ;
        float z = plane.vertices[3 * i + 2];
        printf("Vertex %i -> (%f, %f, %f)\n", i, x, y, z);
    }
    */
#endif

    // Construct indices
    plane.num_indices = div * div * 2 * 3;
    plane.indices = malloc(sizeof(uint16_t) * plane.num_indices);

    count = 0;
    for (int row = 0; row < div; row++) {
        for (int col = 0; col < div; col++) {
            int index = (row * (div + 1) + col);
            // Top triangle
            //   ---
            //   | /
            //   |/ <- index
            plane.indices[count + 0] = index;
            plane.indices[count + 1] = (index + (div + 1) + 1);
            plane.indices[count + 2] = (index + (div + 1));

            // Bottom triangle
            //            /|
            //           / | 
            // index -> /__|
            plane.indices[count + 3] = index;
            plane.indices[count + 4] = index + 1; 
            plane.indices[count + 5] = (index + (div + 1) + 1);
            count += 6;
        }
    }

#ifdef DEBUG
    /*
    for (int i = 0; i < plane.num_indices;  i++) {
        printf("Indices %i\n", plane.indices[i]);
    }
    */
#endif

    return plane;
}

void init(void) {
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func
    });

    simgui_setup(&(simgui_desc_t){ 0 });

    // Create plane
    int plane_division = 1; 
    float plane_size = 1.0f;
    state.plane = make_plane(HMM_V3(0.0f, 0.0f, 0.0f), plane_division, plane_size, SOLID);

#ifdef DEBUG
    printf("Num Vertices: %i\n", state.plane.num_vertices);
    printf("Num Indices: %i\n", state.plane.num_indices);
#endif

    // Setup vertex and index buffers
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc) {
        .data = (sg_range) {
            .ptr = state.plane.vertices,
            .size = sizeof(float) * state.plane.num_vertices * 3
        },
        .label = "plane-vertices"
    });
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc) {
        .usage.index_buffer = true,
        .data = (sg_range) {
            .ptr = state.plane.indices,
            .size = sizeof(uint16_t) * state.plane.num_indices
        },
        .label = "plane-indices"
    });

    // Load shader
    sg_shader shd = sg_make_shader(terrain_shader_desc(sg_query_backend()));


    // Create pipelines
    state.pip[WIREFRAME] = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .primitive_type = SG_PRIMITIVETYPE_LINE_STRIP,
        .layout = {
            .attrs = {
                [ATTR_terrain_position].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .label = "terrain-wireframe-pipeline"
    });
    state.pip[SOLID] = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .layout = {
            .attrs = {
                [ATTR_terrain_position].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .label = "terrain-solid-pipeline"
    });

    state.pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0x28 / 255.0f, 0x29 / 255.0f, 0x23 / 255.0f, 1.0f}
        }
    };
}


void event(const sapp_event* e) {
    simgui_handle_event(e);
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (e->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
        if (e->key_code == SAPP_KEYCODE_W) {
            state.plane.current_draw_mode = WIREFRAME;
        }
    }
}

void frame(void) {
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale()
    });

    // Render imgui
    igSetNextWindowPos((ImVec2){ 10, 10}, ImGuiCond_Once);
    igSetNextWindowSize((ImVec2){ 400, 100}, ImGuiCond_Once);
    igBegin("Sokol Dirt Jam", 0, ImGuiWindowFlags_None);
        igCheckbox("Wireframe", &state.wireframe);
    igEnd();

    state.plane.current_draw_mode = state.wireframe ? WIREFRAME : SOLID;

    // Calculate view projection matrix
    HMM_Mat4 proj = HMM_Perspective_LH_ZO(HMM_AngleDeg(60.0f), sapp_widthf() / sapp_heightf(), 0.01f, 10.0f);
    HMM_Mat4 view = HMM_LookAt_LH(HMM_V3(0.0f, 2.0f, -2.0f), HMM_V3(0.0f, 0.0f, 0.0f), HMM_V3(0.0f, 1.0f, 0.0f));
    HMM_Mat4 view_proj = HMM_MulM4(proj, view);

    // Model rotation matrix
    const float t = (float)(sapp_frame_duration() * 60.0f); 
    state.ry += 0.5f * t;
    HMM_Mat4 rym = HMM_Rotate_LH(HMM_AngleDeg(state.ry), HMM_V3(0.0f, 1.0f, 0.0f));

    sg_begin_pass(&(sg_pass) { .action = state.pass_action, .swapchain = sglue_swapchain() });
        sg_apply_pipeline(state.pip[state.plane.current_draw_mode]);
        sg_apply_bindings(&state.bind);

        // Apply model view projection matrix
        HMM_Mat4 model = HMM_MulM4(HMM_Translate(state.plane.position), rym);
        state.vs_params.mvp = HMM_MulM4(view_proj, model);
        sg_apply_uniforms(UB_vs_params, &SG_RANGE(state.vs_params));
        sg_draw(0, state.plane.num_indices, 1);
        simgui_render();
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
    destroy_plane(state.plane);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .width = 1280,
        .height = 720,
        .window_title = "Sokol Dirt Jam",
        .icon.sokol_default = true,
        // .sample_count = 4,
        .logger.func = slog_func
    };
}
