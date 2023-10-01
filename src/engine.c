#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad.h>
#include <GLFW/glfw3.h>
#include <cglm/struct.h>

// ################################################################
// defines
// ################################################################

#define NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_X (NUMBER_OF_CELLS_ON_AXIS_X * PIXELS_PER_CELL + (NUMBER_OF_CELLS_ON_AXIS_X - 1) * PIXEL_GAP_BETWEEN_CELLS)
#define NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_Y (NUMBER_OF_CELLS_ON_AXIS_Y * PIXELS_PER_CELL + (NUMBER_OF_CELLS_ON_AXIS_Y - 1) * PIXEL_GAP_BETWEEN_CELLS)

#define TITLE "C-Engine"
#define INITIAL_WIDTH  (NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_X + 30)
#define INITIAL_HEIGHT (NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_Y + 30)

#define NUMBER_OF_CELLS   (NUMBER_OF_CELLS_ON_AXIS_X * NUMBER_OF_CELLS_ON_AXIS_Y)
#define QUAD_INDEX_COUNT  6
#define INDEX_COUNT       (NUMBER_OF_CELLS * QUAD_INDEX_COUNT)
#define QUAD_VERTEX_COUNT 4
#define VERTEX_COUNT      (NUMBER_OF_CELLS * QUAD_VERTEX_COUNT)

const char* SHADER_VERTEX = \
    "#version 410 core\n"
    "layout (location = 0) in vec2 in_pos;\n"
    "layout (location = 1) in vec3 in_color;\n"
    "uniform mat4 u_proj_view;\n"
    "out vec3 pass_color;\n"
    "void main() {\n"
    "   gl_Position = u_proj_view * vec4(in_pos, 0.0, 1.0);\n"
    "   pass_color = in_color;\n"
    "}\n"
;
const char* SHADER_FRAGMENT = \
    "#version 410 core\n"
    "in vec3 pass_color;\n"
    "out vec4 out_color;\n"
    "void main() {\n"
    "   out_color = vec4(pass_color, 1.0);\n"
    "}\n"
;

#define FAIL(format, ...) \
	{ \
		char __buffer[1024]; \
		int result = snprintf(__buffer, sizeof(__buffer), format, ##__VA_ARGS__); \
        printf("{%s : %i} %s", __FILE__, __LINE__, __buffer); \
		exit(EXIT_FAILURE); \
	}
#define ASSERT(x, format, ...) { if (!(x)) FAIL(format, ##__VA_ARGS__); }

#define FROM_2D(x, y, w) ((y) * (w) + (x))

// ################################################################
// function declarations (static)
// ################################################################

static void engine_glfw_error_callback(int error, const char* description);
static void engine_glfw_framebuffer_size_callback(GLFWwindow*, int width, int height);
static void engine_check_compile_errors(GLuint id, const char* type);

static vec3s engine_from_color(color_t);
static color_t engine_to_color(vec3s);

static bool engine_get_input_state(bool* last_frame, bool* current_frame, int id);

static bool engine_is_in_bounds(i32 min, i32 max, i32 val);

// ################################################################
// global variables
// ################################################################

static GLFWwindow* s_window;

static GLuint s_pid;
static GLuint s_vao;
static GLuint s_ibo;
static GLuint s_vbo_positions;
static GLuint s_vbo_colors;

static vec2s s_offset;
static mat4s s_proj_view;

static vec3s s_vertex_colors[VERTEX_COUNT];
static bool s_vertex_colors_modified;

static bool s_key_down_last_frame[KEY_MAX];
static bool s_key_down_current_frame[KEY_MAX];

static bool s_button_down_last_frame[KEY_MAX];
static bool s_button_down_current_frame[KEY_MAX];

// ################################################################
// function definitions
// ################################################################

void engine_init() {
	// glfw
	glfwSetErrorCallback(engine_glfw_error_callback);
	int glfw_init_success = glfwInit();
	ASSERT(glfw_init_success, "GLFW: could not initialize");

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	s_window = glfwCreateWindow((int)INITIAL_WIDTH, (int)INITIAL_HEIGHT, TITLE, NULL, NULL);
	ASSERT(s_window, "GLFW: could not create window");

	glfwMakeContextCurrent(s_window);
	glfwSwapInterval(VSYNC);
    glfwSetFramebufferSizeCallback(s_window, engine_glfw_framebuffer_size_callback);

	int glad_load_success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	ASSERT(glad_load_success, "Could not load GL");

	// opengl
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
	engine_set_background(COLOR_BACKGROUND_DEFAULT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// - shader	
	GLuint vid = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vid, 1, &SHADER_VERTEX, NULL);
    glCompileShader(vid);
    engine_check_compile_errors(vid, "VERTEX");
    GLuint fid = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fid, 1, &SHADER_FRAGMENT, NULL);
    glCompileShader(fid);
    engine_check_compile_errors(fid, "FRAGMENT");
    s_pid = glCreateProgram();
    glAttachShader(s_pid, vid);
    glAttachShader(s_pid, fid);
    glLinkProgram(s_pid);
    engine_check_compile_errors(s_pid, "PROGRAM");
    glDeleteShader(vid);
    glDeleteShader(fid);
    glUseProgram(s_pid);

	// - vao
    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);
	// - ibo
	u32 indices[INDEX_COUNT];
    for (size_t i = 0; i < NUMBER_OF_CELLS; i++) {
        u32 offset = i * QUAD_VERTEX_COUNT;
        size_t base = i * QUAD_INDEX_COUNT;
        indices[base + 0] = offset + 0;
        indices[base + 1] = offset + 1;
        indices[base + 2] = offset + 2;
        indices[base + 3] = offset + 2;
        indices[base + 4] = offset + 3;
        indices[base + 5] = offset + 0;
    }
	glGenBuffers(1, &s_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // - vbo (positions)
	vec2s vertex_positions[VERTEX_COUNT];
    for (size_t i = 0; i < NUMBER_OF_CELLS; i++) {
        size_t x = i % NUMBER_OF_CELLS_ON_AXIS_X;
        size_t y = i / NUMBER_OF_CELLS_ON_AXIS_X;
        
        float x_min = (float)(x * (PIXELS_PER_CELL + PIXEL_GAP_BETWEEN_CELLS));
        float x_max = (float)(x_min + PIXELS_PER_CELL);
        float y_min = (float)(y * (PIXELS_PER_CELL + PIXEL_GAP_BETWEEN_CELLS));
        float y_max = (float)(y_min + PIXELS_PER_CELL);

        size_t base = i * QUAD_VERTEX_COUNT;
        vertex_positions[base + 0] = (vec2s){ x_max, y_min };
        vertex_positions[base + 1] = (vec2s){ x_min, y_min };
        vertex_positions[base + 2] = (vec2s){ x_min, y_max };
        vertex_positions[base + 3] = (vec2s){ x_max, y_max };
    }
    glGenBuffers(1, &s_vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2s), NULL);
    glEnableVertexAttribArray(0);
    // - vbo (colors)
    vec3s color = engine_from_color(COLOR_CELL_DEFAULT);
    for (size_t i = 0; i < NUMBER_OF_CELLS; i++) {
        size_t base = i * QUAD_VERTEX_COUNT;
        s_vertex_colors[base + 0] = color;
        s_vertex_colors[base + 1] = color;
        s_vertex_colors[base + 2] = color;
        s_vertex_colors[base + 3] = color;
    }
    glGenBuffers(1, &s_vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_colors), s_vertex_colors, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3s), NULL);
    glEnableVertexAttribArray(1);
}

void engine_deinit() {
	// opengl
    glDeleteBuffers(1, &s_vbo_colors);
    glDeleteBuffers(1, &s_vbo_positions);
    glDeleteBuffers(1, &s_ibo);
	glDeleteVertexArrays(1, &s_vao);
    glDeleteProgram(s_pid);

	// glfw
	glfwDestroyWindow(s_window);
	glfwTerminate();
}

bool engine_is_running() {
	return !glfwWindowShouldClose(s_window);
}

void engine_frame_begin() {
	glfwPollEvents();

	glClear(GL_COLOR_BUFFER_BIT);

    s_vertex_colors_modified = false;

    for (size_t i = KEY_MIN; i < KEY_MAX; i++) {
        s_key_down_current_frame[i] = glfwGetKey(s_window, i) == GLFW_PRESS;
    }
    for (size_t i = BUTTON_MIN; i < BUTTON_MAX; i++) {
        s_button_down_current_frame[i] = glfwGetMouseButton(s_window, i) == GLFW_PRESS;
    }
}

void engine_frame_end() {
    glUseProgram(s_pid);
    glUniformMatrix4fv(glGetUniformLocation(s_pid, "u_proj_view"), 1, GL_FALSE, (GLfloat*)s_proj_view.raw);

    glBindVertexArray(s_vao);
    if (s_vertex_colors_modified) {
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo_colors);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertex_colors), s_vertex_colors, GL_DYNAMIC_DRAW);
    }

    glDrawElements(GL_TRIANGLES, INDEX_COUNT, GL_UNSIGNED_INT, NULL);
    
	glfwSwapBuffers(s_window);

    memcpy(s_key_down_last_frame, s_key_down_current_frame, KEY_MAX);
    memcpy(s_button_down_last_frame, s_button_down_current_frame, KEY_MAX);
}

void engine_set_background(color_t c) {
	vec3s color = engine_from_color(c);
	glClearColor(color.r, color.g, color.b, 1.0);
}

void engine_set_cell(i32 x, i32 y, color_t c) {
    if (!engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_X, x)) FAIL("ERROR: coordinate 'x' out of bounds: %i", x);
    if (!engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_Y, y)) FAIL("ERROR: coordinate 'y' out of bounds: %i", y);

	vec3s color = engine_from_color(c);

    size_t base = FROM_2D(x, y, NUMBER_OF_CELLS_ON_AXIS_X) * QUAD_VERTEX_COUNT;
    vec3s color_old = s_vertex_colors[base];

    s_vertex_colors[base + 0] = color;
    s_vertex_colors[base + 1] = color;
    s_vertex_colors[base + 2] = color;
    s_vertex_colors[base + 3] = color;

    if (!glm_vec2_eqv_eps(color_old.raw, color.raw)) {
        s_vertex_colors_modified = true;
    }
}

color_t engine_get_cell(i32 x, i32 y) {
    if (!engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_X, x)) FAIL("ERROR: coordinate 'x' out of bounds: %i", x);
    if (!engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_Y, y)) FAIL("ERROR: coordinate 'y' out of bounds: %i", y);

    size_t base = FROM_2D(x, y, NUMBER_OF_CELLS_ON_AXIS_X) * QUAD_VERTEX_COUNT;
    return engine_to_color(s_vertex_colors[base]);
}

input_state_t engine_get_key_state(key_id_t id) {
    return engine_get_input_state(s_key_down_last_frame, s_key_down_current_frame, id);
}

input_state_t engine_get_button_state(button_id_t id) {
    return engine_get_input_state(s_button_down_last_frame, s_button_down_current_frame, id);
}

bool engine_get_hovered_cell(i32* x, i32* y) {
    double xpos, ypos;
    glfwGetCursorPos(s_window, &xpos, &ypos);
    i32 pos_x = (i32)xpos;
    i32 pos_y = (i32)ypos;

    pos_x = pos_x - s_offset.x;
    pos_y = pos_y - s_offset.y;

    *x = pos_x / (PIXELS_PER_CELL + PIXEL_GAP_BETWEEN_CELLS);
    *y = pos_y / (PIXELS_PER_CELL + PIXEL_GAP_BETWEEN_CELLS);

    return engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_X, *x) && engine_is_in_bounds(0, NUMBER_OF_CELLS_ON_AXIS_Y, *y);
}

// ################################################################
// function definitions (static)
// ################################################################

static void engine_glfw_error_callback(int error, const char* description) {
	FAIL("GLFW error (%i): %s", error, description);
}

static void engine_glfw_framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);

	float bottom = (float)height;
	float top    = 0.0f;
	float left   = 0.0f;
	float right  = (float)width;
	mat4s proj = glms_ortho(left, right, bottom, top, 0.0f, 1.0f);

    s_offset.x = (width - NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_X) / 2.0;
    s_offset.y = (height - NUMBER_OF_PIXELS_IN_GRID_ON_AXIS_Y) / 2.0;
    vec3s offset = (vec3s){ s_offset.x, s_offset.y, 0.0 };
    mat4s view = glms_mat4_identity();
    glm_translate(view.raw, offset.raw);

	s_proj_view; glm_mat4_mul(proj.raw, view.raw, s_proj_view.raw);
}

static void engine_check_compile_errors(GLuint id, const char* type) {
    GLint success;
	GLchar info_log[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            glGetShaderInfoLog(id, sizeof(info_log), NULL, info_log);
            FAIL("GL: SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n", type, info_log);
        }
    } else {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            glGetProgramInfoLog(id, sizeof(info_log), NULL, info_log);
            FAIL("GL: PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- -- \n", type, info_log);
        }
    }
}

static vec3s engine_from_color(color_t c) {
	vec3s color;
	color.r = ((c >> 16) & 0xFF) / (float)255;
	color.g = ((c >>  8) & 0xFF) / (float)255;
	color.b = ((c >>  0) & 0xFF) / (float)255;
	return color;
}

static color_t engine_to_color(vec3s c) {
    u32 r = (u32)(c.r * 255);
    u32 g = (u32)(c.g * 255);
    u32 b = (u32)(c.b * 255);
	return (r << 16) | (g << 8) | (b << 0);
}

static bool engine_get_input_state(bool* last_frame, bool* current_frame, int id) {
    if (current_frame[id] && last_frame[id])
        return INPUT_STATE_DOWN;
    if (!current_frame[id] && !last_frame[id])
        return INPUT_STATE_UP;
    if (current_frame[id] && !last_frame[id])
        return INPUT_STATE_PRESSED;
    if (!current_frame[id] && last_frame[id])
        return INPUT_STATE_RELEASED;
    // unreachable
}

static bool engine_is_in_bounds(i32 min, i32 max, i32 val) {
    return val >= min && val < max;
}