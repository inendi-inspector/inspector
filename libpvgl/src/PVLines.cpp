//! \file PVLines.cpp
//! $Id: PVLines.cpp 2977 2011-05-26 04:39:06Z dindinx $
//! Copyright (C) SÃ©bastien Tricaud 2009-2011
//! Copyright (C) Philippe Saade 2009-2011
//! Copyright (C) Picviz Labs 2011

#include <sstream>
#include <cmath>

#define GLEW_STATIC 1
#include <GL/glew.h>

#include <picviz/PVLinesProperties.h>
#include <picviz/PVView.h>
#include <picviz/PVSelection.h>

#include <pvgl/PVUtils.h>
#include <pvgl/views/PVParallel.h>
#include <pvgl/PVWidgetManager.h>
#include <pvgl/PVConfig.h>
#include <pvgl/PVMain.h>

#include <pvgl/PVLines.h>

// attribute list:
// 0: vec4 color (property)
// 1: vec4 : z (in the x component)
// 2-15: vec4 : ys
// nb_indices = max_number_of_lines_in_view
// size of the position array: sizeof(vec4)*((nb_axes+3)/4)*max_number_of_lines_in_view

const int NB_ATTRIBUTES_PER_BATCH = 14;
const int NB_AXES_PER_BATCH = NB_ATTRIBUTES_PER_BATCH * 4;

const int FBO_MAX_WIDTH = 6072;
const int FBO_MAX_HEIGHT= 2048;

const float fbo_width_factor  = 1.0f + 2.0f * PVGL_VIEW_OUTSIDE_HORIZONTAL_BORDER / 100.0;
const float fbo_height_factor = 1.0f + 2.0f * PVGL_VIEW_OUTSIDE_VERTICAL_BORDER / 100.0;

/******************************************************************************
 *
 * PVLines::PVLines
 *
 *****************************************************************************/
PVGL::PVLines::PVLines(PVView *view_) : view(view_)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	reset_offset();
	drawn_lines = 0;
	drawn_zombie_lines = 0;

	vbo_color = 0;
	vbo_zla = 0;
	tbo_selection = 0;
	tbo_selection_texture = 0;
	tbo_zombie = 0;
	tbo_zombie_texture = 0;
	fbo_vao = 0;
	main_fbo = 0;
	main_fbo_tex = 0;
	lines_fbo = 0;
	lines_fbo_tex = 0;
	zombie_fbo = 0;
	zombie_fbo_tex = 0;
}

/******************************************************************************
 *
 * PVLines::~PVLines
 *
 *****************************************************************************/
PVGL::PVLines::~PVLines()
{
	PVLOG_INFO("In PVLines destructor\n");
	free_buffers();
}

/******************************************************************************
 *
 * PVLines::free_buffers
 *
 *****************************************************************************/
void PVGL::PVLines::free_buffers()
{
	// Free OpenGL objects
	if (vbo_color != 0) {
		glDeleteBuffers(1, &vbo_color);
		vbo_color = 0;
	}
	if (vbo_zla != 0) {
		glDeleteBuffers(1, &vbo_zla);
		vbo_zla = 0;
	}
	if (tbo_selection != 0) {
		glDeleteBuffers(1, &tbo_selection);
		tbo_selection = 0;
	}
	if (tbo_selection_texture != 0) {
		glDeleteTextures(1, &tbo_selection_texture);
		tbo_selection_texture = 0;
	}
	if (tbo_zombie != 0) {
		glDeleteBuffers(1, &tbo_zombie);
		tbo_zombie = 0;
	}
	if (tbo_zombie_texture != 0) {
		glDeleteTextures(1, &tbo_zombie_texture);
		tbo_zombie_texture = 0;
	}
	if (fbo_vao != 0) {
		glDeleteVertexArrays(1, &fbo_vao); 
		fbo_vao = 0;
	}
	if (main_fbo != 0) {
		glDeleteFramebuffers(1, &main_fbo);
		main_fbo = 0;
	}
	if (main_fbo_tex != 0) {
		glDeleteTextures(1, &main_fbo_tex);
		main_fbo_tex = 0;
	}
	if (lines_fbo != 0) {
		glDeleteFramebuffers(1, &lines_fbo);
		lines_fbo = 0;
	}
	if (lines_fbo_tex != 0) {
		glDeleteTextures(1, &lines_fbo_tex);
		lines_fbo_tex = 0;
	}
	if (zombie_fbo != 0) {
		glDeleteFramebuffers(1, &zombie_fbo);
		zombie_fbo = 0;
	}
	if (zombie_fbo_tex != 0) {
		glDeleteTextures(1, &zombie_fbo_tex);
		zombie_fbo_tex = 0;
	}

	std::vector<Batch>::iterator it;
	for (it = batches.begin(); it != batches.end(); it++) {
		Batch& batch = *it;
		glDeleteVertexArrays(1, &batch.vao);
		glDeleteBuffers(1, &batch.vbo_position);
	}
	batches.clear();
}

/******************************************************************************
 *
 * PVLines::reset_offset
 *
 *****************************************************************************/
void PVGL::PVLines::reset_offset()
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	offset = vec2(0);
}

/******************************************************************************
 *
 * PVGL::PVLines::move_offset
 *
 *****************************************************************************/
void PVGL::PVLines::move_offset(const vec2 &delta)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	offset += vec2(delta.x, -delta.y);
}

/******************************************************************************
 *
 * PVGL::PVLines::move_offset
 *
 *****************************************************************************/
void PVGL::PVLines::set_size(int width, int height)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	fbo_width = width * fbo_width_factor;
	fbo_height= height* fbo_height_factor;

	set_main_fbo_dirty();
	set_lines_fbo_dirty();
	set_zombie_fbo_dirty();
	reset_offset();
}

/******************************************************************************
 *
 * PVGL::PVLines::create_batches
 *
 *****************************************************************************/
void PVGL::PVLines::create_batches()
{
	size_t lpr = view->get_max_lines_per_redraw();
	size_t nb_axes = picviz_view->get_axes_count();

	nb_batches = 1 + (nb_axes - 2) / (NB_AXES_PER_BATCH - 1);

	PVLOG_DEBUG("PVGL::PVLines::%s, nb_axes: %d\n", __FUNCTION__, nb_axes);
	PVLOG_DEBUG("PVGL::PVLines::%s, nb_batches: %d\n", __FUNCTION__, nb_batches);

	std::vector<Batch>::iterator it;
	for (it = batches.begin(); it != batches.end(); it++) {
		Batch& batch = *it;
		glDeleteVertexArrays(1, &batch.vao);
		glDeleteBuffers(1, &batch.vbo_position);
	}
	batches.clear();
	for (unsigned k = 0; k < nb_batches; k++) {
		std::vector<std::string> attributes;
		Batch batch;
		int nb_vec4;
		int nb_axes_in_batch;
		int array_size;

		if (k == nb_batches - 1) { // The last batch is special (incomplete)
			nb_axes_in_batch = nb_axes;
		} else {
			nb_axes_in_batch = NB_AXES_PER_BATCH;
		}
		nb_vec4 = (nb_axes_in_batch + 3) / 4;

		glGenVertexArrays(1, &batch.vao); PRINT_OPENGL_ERROR();
		glBindVertexArray(batch.vao); PRINT_OPENGL_ERROR();

		// The VBO holding the line colors.
		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glEnableVertexAttribArray(0); PRINT_OPENGL_ERROR();
		glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, BUFFER_OFFSET(0));
		attributes.push_back("color_v"); PRINT_OPENGL_ERROR();

		// The VBO holding the line depth.
		glBindBuffer(GL_ARRAY_BUFFER, vbo_zla);
		glEnableVertexAttribArray(1); PRINT_OPENGL_ERROR();
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		attributes.push_back("zla_v"); PRINT_OPENGL_ERROR();

		///////////////
		array_size = nb_vec4 * lpr;
		size_t salloc = array_size * sizeof(vec4);
		PVLOG_INFO("PVGL: for batch %d, allocate %u bytes\n", k, salloc);
		glGenBuffers(1, &batch.vbo_position); PRINT_OPENGL_ERROR();
		glBindBuffer(GL_ARRAY_BUFFER, batch.vbo_position); PRINT_OPENGL_ERROR();
		glBufferData(GL_ARRAY_BUFFER, array_size * sizeof(vec4), 0, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
		batch.vbo_pos_alloc_size = salloc;

		for (int i = 2; i < nb_vec4 + 2; i++) {
			std::stringstream pos;
			glEnableVertexAttribArray(i); PRINT_OPENGL_ERROR();
			glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, nb_vec4 * sizeof(vec4), BUFFER_OFFSET((i - 2) * sizeof(vec4))); PRINT_OPENGL_ERROR();
			pos << "position_v[" << i - 2 << "]";
			attributes.push_back(pos.str());
			PVLOG_DEBUG("PVGL::PVLines::%s, Attrib: %d enabled.\n", __FUNCTION__, i);
		}
		std::stringstream prefix_stream;
		prefix_stream << "#version 330\n\n#define coordinates " << nb_axes_in_batch << "\n" <<
		                 "#define tab_size " << nb_vec4 << "\n" <<
		                 "#define NB_AXES_PER_BATCH " << NB_AXES_PER_BATCH << ".0 \n";
		PVLOG_DEBUG("PVGL::PVLines::%s, Prefix: %s.\n", __FUNCTION__, prefix_stream.str().c_str());
		batch.program = read_shader("parallel/lines.vert", "parallel/lines.geom", "parallel/lines.frag",
		                            prefix_stream.str(), prefix_stream.str(), "", attributes);
		glUniform1f(get_uni_loc(batch.program, "index_axe"), k*55.0); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(batch.program, "selection_sampler"), 1); PRINT_OPENGL_ERROR();

		batch.zombie_program = read_shader("parallel/lines_zombie.vert", "parallel/lines_zombie.geom", "parallel/lines_zombie.frag",
		                                   prefix_stream.str(), prefix_stream.str(), "", attributes);
		glUniform1f(get_uni_loc(batch.zombie_program, "index_axe"), k * 55.0); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(batch.zombie_program, "zombie_sampler"), 2); PRINT_OPENGL_ERROR();

		batches.push_back(batch);
		nb_axes -= NB_AXES_PER_BATCH - 1;
	}
}

/******************************************************************************
 *
 * PVGL::PVLines::init
 *
 *****************************************************************************/
void PVGL::PVLines::init(Picviz::PVView_p pv_view_)
{
	free_buffers();
	
	picviz_view = pv_view_;
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	glGenBuffers(1, &vbo_color); PRINT_OPENGL_ERROR();
	glGenBuffers(1, &vbo_zla); PRINT_OPENGL_ERROR();

	// Create a TBO holding the current selection as a bit-field.
	glGenBuffers(1, &tbo_selection);                                                          PRINT_OPENGL_ERROR();
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_selection);                                           PRINT_OPENGL_ERROR();
	glBufferData(GL_TEXTURE_BUFFER, PICVIZ_SELECTION_NUMBER_OF_BYTES, NULL, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
	glGenTextures(1, &tbo_selection_texture);                                                 PRINT_OPENGL_ERROR();
	glActiveTexture(GL_TEXTURE1);                                                             PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_BUFFER, tbo_selection_texture);                                  PRINT_OPENGL_ERROR();
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, tbo_selection);                                  PRINT_OPENGL_ERROR();

	// Create a TBO holding the current non-zombie lines as a bit-field.
	glGenBuffers(1, &tbo_zombie);                                                             PRINT_OPENGL_ERROR();
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_zombie);                                              PRINT_OPENGL_ERROR();
	glBufferData(GL_TEXTURE_BUFFER, PICVIZ_SELECTION_NUMBER_OF_BYTES, NULL, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
	glGenTextures(1, &tbo_zombie_texture);                                                    PRINT_OPENGL_ERROR();
	glActiveTexture(GL_TEXTURE2);                                                             PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_BUFFER, tbo_zombie_texture);                                     PRINT_OPENGL_ERROR();
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, tbo_zombie);                                     PRINT_OPENGL_ERROR();

	glActiveTexture(GL_TEXTURE0); PRINT_OPENGL_ERROR();

	create_batches();

	// Init all the needed fbos
	init_main_fbo();
	init_lines_fbo();
	init_zombie_fbo();

	// Restore a sane state.
	glBindVertexArray(0); PRINT_OPENGL_ERROR();
	glUseProgram(0); PRINT_OPENGL_ERROR();
}

/******************************************************************************
 *
 * PVGL::PVLines::change_axes_count
 *
 *****************************************************************************/
void PVGL::PVLines::change_axes_count()
{
	size_t nb_axes = picviz_view->get_axes_count();

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	PVLOG_DEBUG("PVGL::PVLines::%s, changing axe count to %d.\n", __FUNCTION__, nb_axes);
	if (!picviz_view->is_consistent()) {
		return;
	}
	create_batches();

	glBindVertexArray(0); PRINT_OPENGL_ERROR();
	glUseProgram(0); PRINT_OPENGL_ERROR();
	update_arrays_positions();
}

/******************************************************************************
 *
 * PVGL::PVLines::init_main_fbo
 *
 *****************************************************************************/
void PVGL::PVLines::init_main_fbo(void)
{
	std::vector<std::string> attributes;
	GLfloat fbo_vertex_buffer[] = {
		-1,-1, 0,0,
		 1,-1, 1,0,
		 1, 1, 1,1,
		-1, 1, 0,1
	};
	GLuint fbo_vbo;

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	// Creating the FBO itself.
	glGenFramebuffers(1, &main_fbo); PRINT_OPENGL_ERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, main_fbo); PRINT_OPENGL_ERROR();
	glGenTextures(1, &main_fbo_tex); PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_RECTANGLE, main_fbo_tex); PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, FBO_MAX_WIDTH, FBO_MAX_HEIGHT, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, 0); PRINT_OPENGL_ERROR();
	glFramebufferTexture2D(GL_FRAMEBUFFER,
	                       GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_RECTANGLE, main_fbo_tex, 0); PRINT_OPENGL_ERROR();
	check_framebuffer_status();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); PRINT_OPENGL_ERROR();


	// FBOs' VAO (shared by other fbos too)
	glGenVertexArrays(1, &fbo_vao); PRINT_OPENGL_ERROR();
	glBindVertexArray(fbo_vao); PRINT_OPENGL_ERROR();
	glGenBuffers(1, &fbo_vbo); PRINT_OPENGL_ERROR();
	glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo); PRINT_OPENGL_ERROR();
	glBufferData(GL_ARRAY_BUFFER, sizeof fbo_vertex_buffer,
	             fbo_vertex_buffer, GL_STATIC_DRAW); PRINT_OPENGL_ERROR();
	glEnableVertexAttribArray(0); PRINT_OPENGL_ERROR();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0); PRINT_OPENGL_ERROR();
	attributes.push_back("position");
	glEnableVertexAttribArray(1); PRINT_OPENGL_ERROR();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), BUFFER_OFFSET(2 * sizeof(GLfloat))); PRINT_OPENGL_ERROR();
	attributes.push_back("tex_coord"); PRINT_OPENGL_ERROR();
	main_fbo_program = read_shader("parallel/lines_fbo.vert", "", "parallel/lines_fbo.frag", "", "", "", attributes);
	glUniform1i(get_uni_loc(main_fbo_program, "fbo_sampler"), 0); PRINT_OPENGL_ERROR();
	main_fbo_axis_mode_program = read_shader("parallel/lines_fbo_axis_mode.vert", "", "parallel/lines_fbo_axis_mode.frag", "", "", "", attributes);
	glUniform1i(get_uni_loc(main_fbo_axis_mode_program, "fbo_sampler"), 0); PRINT_OPENGL_ERROR();

	set_main_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::init_lines_fbo
 *
 *****************************************************************************/
void PVGL::PVLines::init_lines_fbo(void)
{
	std::vector<std::string> attributes;
	GLuint depth_rb;

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	glGenFramebuffers(1, &lines_fbo); PRINT_OPENGL_ERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, lines_fbo); PRINT_OPENGL_ERROR();
	glGenTextures(1, &lines_fbo_tex); PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_RECTANGLE, lines_fbo_tex); PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, FBO_MAX_WIDTH, FBO_MAX_HEIGHT, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, 0); PRINT_OPENGL_ERROR();
	glFramebufferTexture2D(GL_FRAMEBUFFER,
	                       GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_RECTANGLE, lines_fbo_tex, 0); PRINT_OPENGL_ERROR();
	glGenRenderbuffers(1, &depth_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, FBO_MAX_WIDTH, FBO_MAX_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
	check_framebuffer_status();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); PRINT_OPENGL_ERROR();
}

/******************************************************************************
 *
 * PVGL::PVLines::init_zombie_fbo
 *
 *****************************************************************************/
void PVGL::PVLines::init_zombie_fbo(void)
{
	std::vector<std::string> attributes;

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	glGenFramebuffers(1, &zombie_fbo);                                        PRINT_OPENGL_ERROR();
	glBindFramebuffer(GL_FRAMEBUFFER, zombie_fbo);                            PRINT_OPENGL_ERROR();
	glGenTextures(1, &zombie_fbo_tex);                                        PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_RECTANGLE, zombie_fbo_tex);                      PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST); PRINT_OPENGL_ERROR();
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA,
		   	FBO_MAX_WIDTH, FBO_MAX_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);  PRINT_OPENGL_ERROR();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		   	GL_TEXTURE_RECTANGLE, zombie_fbo_tex, 0);                         PRINT_OPENGL_ERROR();
	check_framebuffer_status();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);                                     PRINT_OPENGL_ERROR();

	attributes.push_back("position");
	attributes.push_back("tex_coord");
	zombie_fbo_program = read_shader("parallel/lines_zombie_fbo.vert", "", "parallel/lines_zombie_fbo.frag", "", "", "", attributes);
	glUniform1i(get_uni_loc(zombie_fbo_program, "fbo_sampler"), 0); PRINT_OPENGL_ERROR();

	set_zombie_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::set_main_fbo_dirty
 *
 *****************************************************************************/
void PVGL::PVLines::set_main_fbo_dirty()
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	main_fbo_dirty = true;
}

/******************************************************************************
 *
 * PVGL::PVLines::set_lines_fbo_dirty
 *
 *****************************************************************************/
void PVGL::PVLines::set_lines_fbo_dirty()
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	lines_fbo_dirty = true;
	drawn_lines = 0;
	idle_manager.new_task(view, IDLE_REDRAW_LINES);
	set_main_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::set_zombie_fbo_dirty
 *
 *****************************************************************************/
void PVGL::PVLines::set_zombie_fbo_dirty()
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	zombie_fbo_dirty = true;
	drawn_zombie_lines = 0;
	idle_manager.new_task(view, IDLE_REDRAW_ZOMBIE_LINES);
	set_main_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::draw_zombie_lines
 *
 *****************************************************************************/
void PVGL::PVLines::draw_zombie_lines(GLfloat modelview[16])
{
	int nb_lines_to_draw = idle_manager.get_number_of_lines(view, IDLE_REDRAW_ZOMBIE_LINES);
	// Note: nb_lines_to_draw is always less than lpr (the idle manager should force this.

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, zombie_fbo); PRINT_OPENGL_ERROR();
	glViewport(0, 0, fbo_width, fbo_height); PRINT_OPENGL_ERROR();
	if (drawn_zombie_lines == 0) {
		glClearColor(0.0, 0.0, 0.0, 0.0); PRINT_OPENGL_ERROR();
		glClear(GL_COLOR_BUFFER_BIT); PRINT_OPENGL_ERROR();
	}
	if (nb_lines_to_draw == 0) {
		return;
	}
	glEnable(GL_BLEND);
	glBlendEquation(GL_MAX);
	PVRow lines_count = picviz_min(nb_lines_to_draw, int(picviz_view->get_row_count() - drawn_zombie_lines));
	fill_vbo_colors_and_zla(drawn_zombie_lines, lines_count);
	for (unsigned i = 0; i < nb_batches; i++) {
		glUseProgram(batches[i].zombie_program); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(batches[i].zombie_program, "zoom"), fbo_width_factor, fbo_height_factor); PRINT_OPENGL_ERROR();
		glUniformMatrix4fv(get_uni_loc(batches[i].zombie_program, "view"), 1, GL_FALSE, modelview); PRINT_OPENGL_ERROR ();
		glUniform1i(get_uni_loc(batches[i].zombie_program, "drawn_lines"), drawn_zombie_lines);
		glBindVertexArray(batches[i].vao); PRINT_OPENGL_ERROR();
		fill_vbo_positions(i, drawn_zombie_lines, lines_count);

		// FIXME: this is probably overkill to do these 3 lines here, but I do have curious error if I remove them.
		glActiveTexture(GL_TEXTURE2); PRINT_OPENGL_ERROR();
		glBindTexture(GL_TEXTURE_BUFFER, tbo_zombie_texture); PRINT_OPENGL_ERROR();
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, tbo_zombie); PRINT_OPENGL_ERROR();

		glDrawArrays(GL_POINTS, 0, lines_count); PRINT_OPENGL_ERROR();
	}
	glBlendEquation(GL_FUNC_ADD); PRINT_OPENGL_ERROR();
	glDisable(GL_BLEND);          PRINT_OPENGL_ERROR();
	drawn_zombie_lines += nb_lines_to_draw;

	PVLOG_DEBUG("PVGL::PVLines::%s: %d zombie lines drawn.\n", __FUNCTION__, drawn_zombie_lines);

	if (drawn_zombie_lines >= int(picviz_view->get_row_count())) {
		idle_manager.remove_task(view, IDLE_REDRAW_ZOMBIE_LINES);
		zombie_fbo_dirty = false;
		drawn_zombie_lines = 0;
	}
}

/******************************************************************************
 *
 * PVGL::PVLines::fill_vbo_colors_and_zla
 *
 *****************************************************************************/
void PVGL::PVLines::fill_vbo_colors_and_zla(GLint start, GLsizei count)
{
  glBindBuffer(GL_ARRAY_BUFFER, vbo_zla);  PRINT_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), &(picviz_view->z_level_array.get_value(start)), GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_color);  PRINT_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(ubvec4), NULL, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, count * sizeof(ubvec4), &picviz_view->post_filter_layer.get_lines_properties().table.at(start), GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
}


/******************************************************************************
 *
 * PVGL::PVLines::fill_vbo_positions
 *
 *****************************************************************************/
void PVGL::PVLines::fill_vbo_positions(unsigned int batch_index, GLuint start, GLsizei count)
{
	glBindBuffer(GL_ARRAY_BUFFER, batches[batch_index].vbo_position); PRINT_OPENGL_ERROR();
	glBufferData(GL_ARRAY_BUFFER, batches[batch_index].vbo_pos_alloc_size, NULL, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
	vec4* buffer = reinterpret_cast<vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

	PVRow nb_row = picviz_view->get_row_count();
	PVCol nb_col = picviz_view->get_axes_count();
	PVCol plotted_col_size = picviz_view->get_original_axes_count();
	const float* plotted_array = picviz_view->get_plotted_parent()->get_table_pointer();

	if (batch_index < nb_batches - 1) {
		unsigned  nb_positions = 0;
		for (PVRow i = start; i < start+count; i++) {
			const float *temp_pointer_in_array = plotted_array + i * plotted_col_size;
			for (int j = 0; j < 4 * NB_ATTRIBUTES_PER_BATCH; j += 4) {
				buffer[nb_positions + j / 4].x = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast((NB_AXES_PER_BATCH - 1) * batch_index + j + 0)];
				buffer[nb_positions + j / 4].y = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast((NB_AXES_PER_BATCH - 1) * batch_index + j + 1)];
				buffer[nb_positions + j / 4].z = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast((NB_AXES_PER_BATCH - 1) * batch_index + j + 2)];
				buffer[nb_positions + j / 4].w = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast((NB_AXES_PER_BATCH - 1) * batch_index + j + 3)];
			}
			nb_positions += NB_ATTRIBUTES_PER_BATCH;
		}
	}
	else {
		// The last batch is a bit special
		unsigned  nb_positions_last_batch = 0;
		for (PVRow i = start; i < start+count; i++) {
			const float *temp_pointer_in_array = plotted_array + i * plotted_col_size;
			for (PVCol j = 0; j < nb_col - (NB_AXES_PER_BATCH - 1) * PVCol(batch_index); j += 4) {
				size_t index = (NB_AXES_PER_BATCH - 1) * batch_index + j;
				buffer[nb_positions_last_batch].x = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast(index + 0)];
				if (PVCol(index + 1) < nb_col)
					buffer[nb_positions_last_batch].y = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast(index + 1)];
				if (PVCol(index + 2) < nb_col)
					buffer[nb_positions_last_batch].z = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast(index + 2)];
				if (PVCol(index + 3) < nb_col)
					buffer[nb_positions_last_batch].w = temp_pointer_in_array[picviz_view->axes_combination.get_axis_column_index_fast(index + 3)];
				nb_positions_last_batch++;
			}
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER); PRINT_OPENGL_ERROR();
}

/******************************************************************************
 *
 * PVGL::PVLines::draw_selected_lines
 *
 *****************************************************************************/
void PVGL::PVLines::draw_selected_lines(GLfloat modelview[16])
{
	int nb_lines_to_draw = idle_manager.get_number_of_lines(view, IDLE_REDRAW_LINES);

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	if (nb_lines_to_draw == 0) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, lines_fbo); PRINT_OPENGL_ERROR();
	glViewport(0, 0, fbo_width, fbo_height); PRINT_OPENGL_ERROR();
	if (drawn_lines == 0) {
		glClearColor(0.0, 0.0, 0.0, 0.0); PRINT_OPENGL_ERROR();
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); PRINT_OPENGL_ERROR();
	}
	PVRow lines_count = picviz_min(nb_lines_to_draw, PVRow(picviz_view->get_row_count() - drawn_lines));
	fill_vbo_colors_and_zla(drawn_lines, lines_count);
	for (unsigned int i = 0; i < nb_batches; i++) {
		glUseProgram(batches[i].program); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(batches[i].program, "zoom"), fbo_width_factor, fbo_height_factor); PRINT_OPENGL_ERROR();
		glUniformMatrix4fv(get_uni_loc(batches[i].program, "view"), 1, GL_FALSE, modelview); PRINT_OPENGL_ERROR ();
		glUniform1i(get_uni_loc(batches[i].program, "eventline_first"), picviz_view->eventline.get_first_index()); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(batches[i].program, "eventline_current"), picviz_view->eventline.get_current_index()); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(batches[i].program, "drawn_lines"), drawn_lines);
		glBindVertexArray(batches[i].vao); PRINT_OPENGL_ERROR();
		fill_vbo_positions(i, drawn_lines, lines_count);

		// FIXME: this is probably overkill to do these 3 lines here, but I do have curious errors if I remove them.
		glActiveTexture(GL_TEXTURE1); PRINT_OPENGL_ERROR();
		glBindTexture(GL_TEXTURE_BUFFER, tbo_selection_texture); PRINT_OPENGL_ERROR();
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, tbo_selection); PRINT_OPENGL_ERROR();

		glDrawArrays(GL_POINTS, 0, lines_count);
	}
	drawn_lines += nb_lines_to_draw;
	if (drawn_lines >= int(picviz_view->get_row_count())) {
		idle_manager.remove_task(view, IDLE_REDRAW_LINES);
		lines_fbo_dirty = false;
		drawn_lines = 0;
	}
}

/******************************************************************************
 *
 * PVGL::PVLines::draw
 *
 *****************************************************************************/
void PVGL::PVLines::draw()
{
	Picviz::PVStateMachine *state_machine = picviz_view->state_machine;

	PVLOG_HEAVYDEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	if (main_fbo_dirty || lines_fbo_dirty || zombie_fbo_dirty) { // We need to redraw the lines into the framebuffer.
		GLfloat modelview[16];
		// Setup the Antialiasing stuff.
		if (state_machine->is_antialiased()) {
			glEnable(GL_BLEND); PRINT_OPENGL_ERROR();
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); PRINT_OPENGL_ERROR();
			glEnable(GL_LINE_SMOOTH); PRINT_OPENGL_ERROR();
			glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST); PRINT_OPENGL_ERROR();
		} else {
			glDisable(GL_BLEND); PRINT_OPENGL_ERROR();
			glDisable(GL_LINE_SMOOTH); PRINT_OPENGL_ERROR();
		}

		// Set the LineWidth.
#ifdef SCREENSHOT
		glLineWidth(4.0f); PRINT_OPENGL_ERROR();
#else
		glLineWidth(1.0f); PRINT_OPENGL_ERROR();
#endif

		glGetFloatv(GL_MODELVIEW_MATRIX, modelview); PRINT_OPENGL_ERROR();

		if (!state_machine->is_grabbed() /* && disabled for now !view->get_map().is_panning()*/) {
			// Draw the zombie/non-zombie lines into their own FBO
			if (zombie_fbo_dirty && (state_machine->are_gl_zombie_visible() || state_machine->are_gl_unselected_visible())) {
				draw_zombie_lines(modelview);
			}
			// Draw the selected lines into their own FBO
			if (lines_fbo_dirty) {
				draw_selected_lines(modelview);
			}
		}

		glDisable(GL_DEPTH_TEST);
		// Draw into the main FBO
		glViewport(0, 0, fbo_width, fbo_height); PRINT_OPENGL_ERROR();
		glBindFramebuffer(GL_FRAMEBUFFER, main_fbo); PRINT_OPENGL_ERROR();

		// Draw the zombie fbo into the main fbo
		glActiveTexture(GL_TEXTURE0); PRINT_OPENGL_ERROR();
		glEnable(GL_TEXTURE_RECTANGLE); PRINT_OPENGL_ERROR();
		glBindTexture(GL_TEXTURE_RECTANGLE, zombie_fbo_tex); PRINT_OPENGL_ERROR();
		glUseProgram(zombie_fbo_program); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(zombie_fbo_program, "zoom"), 1, 1);
		glUniform2f(get_uni_loc(zombie_fbo_program, "offset"), 0, 0); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(zombie_fbo_program, "size"), fbo_width, fbo_height); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(zombie_fbo_program, "draw_zombie"), state_machine->are_gl_zombie_visible()); PRINT_OPENGL_ERROR();
		glUniform1i(get_uni_loc(zombie_fbo_program, "draw_unselected"), state_machine->are_gl_unselected_visible());PRINT_OPENGL_ERROR();
		glBindVertexArray(fbo_vao); PRINT_OPENGL_ERROR();
		glDrawArrays(GL_QUADS, 0, 4); PRINT_OPENGL_ERROR();

		// Draw the "selected lines" into the main FBO
		glBindTexture(GL_TEXTURE_RECTANGLE, lines_fbo_tex); PRINT_OPENGL_ERROR();
		glUseProgram(main_fbo_program); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(main_fbo_program, "zoom"), 1, 1);
		glUniform2f(get_uni_loc(main_fbo_program, "offset"), 0, 0); PRINT_OPENGL_ERROR();
		glUniform2f(get_uni_loc(main_fbo_program, "size"), fbo_width, fbo_height); PRINT_OPENGL_ERROR();
		glBindVertexArray(fbo_vao); PRINT_OPENGL_ERROR();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_QUADS, 0, 4); PRINT_OPENGL_ERROR();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		glColor3f(1.0f, 1.0f, 1.0f);
		glLoadMatrixf(modelview);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); PRINT_OPENGL_ERROR();

		if (!lines_fbo_dirty && !zombie_fbo_dirty) {
			main_fbo_dirty = false; // XXX we could put the axes & names into this fbo too, we will have to change this.
		}
	}

	// Draw the main FBO on the screen.
	glViewport(0, 0, view->get_width(), view->get_height()); PRINT_OPENGL_ERROR();
	glActiveTexture(GL_TEXTURE0);                            PRINT_OPENGL_ERROR();
	glEnable(GL_TEXTURE_RECTANGLE);                          PRINT_OPENGL_ERROR();
	glBindTexture(GL_TEXTURE_RECTANGLE, main_fbo_tex);       PRINT_OPENGL_ERROR();

	GLuint fbo_program;
	if (state_machine->is_axes_mode()) {
		fbo_program = main_fbo_axis_mode_program;
	} else {
		fbo_program = main_fbo_program;
	}
	glUseProgram(fbo_program);                                                          PRINT_OPENGL_ERROR();
	glUniform2f(get_uni_loc(fbo_program, "zoom"), fbo_width_factor, fbo_height_factor); PRINT_OPENGL_ERROR();
	glUniform2f(get_uni_loc(fbo_program, "offset"), offset.x, offset.y);                PRINT_OPENGL_ERROR();
	glUniform2f(get_uni_loc(fbo_program, "size"), fbo_width, fbo_height);               PRINT_OPENGL_ERROR();

	glBindVertexArray(fbo_vao);   PRINT_OPENGL_ERROR();
	glDrawArrays(GL_QUADS, 0, 4); PRINT_OPENGL_ERROR();

	// Restore a known OpenGL state.
	glDisable(GL_BLEND);                                     PRINT_OPENGL_ERROR();
	glDisable(GL_TEXTURE_RECTANGLE);                         PRINT_OPENGL_ERROR();
	glUseProgram(0);                                         PRINT_OPENGL_ERROR();
	glBindVertexArray(0);                                    PRINT_OPENGL_ERROR();
	glViewport(0, 0, view->get_width(), view->get_height()); PRINT_OPENGL_ERROR();
	glEnable(GL_DEPTH_TEST);                                 PRINT_OPENGL_ERROR();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_arrays_z
 *
 *****************************************************************************/
void PVGL::PVLines::update_arrays_z(void)
{
//	int nb_lines;

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	set_lines_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_arrays_colors
 *
 *****************************************************************************/
void PVGL::PVLines::update_arrays_colors(void)
{
//	int nb_lines;

	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	set_lines_fbo_dirty();
	set_zombie_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_arrays_selection
 *
 *****************************************************************************/
void PVGL::PVLines::update_arrays_selection(void)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	// Update the TBO
	PRINT_OPENGL_ERROR();
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_selection); PRINT_OPENGL_ERROR();
	glBufferSubData(GL_TEXTURE_BUFFER, 0, PICVIZ_SELECTION_NUMBER_OF_BYTES,
	                picviz_view->post_filter_layer.get_selection().get_buffer()); PRINT_OPENGL_ERROR();

	view->update_label_lines_selected_eventline();
	set_lines_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_arrays_zombies
 *
 *****************************************************************************/
void PVGL::PVLines::update_arrays_zombies(void)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent()) {
		return;
	}
	// Update the TBO
	glBindBuffer(GL_TEXTURE_BUFFER, tbo_zombie); PRINT_OPENGL_ERROR();
	glBufferSubData(GL_TEXTURE_BUFFER, 0, PICVIZ_SELECTION_NUMBER_OF_BYTES,
		   	picviz_view->layer_stack_output_layer.get_selection().get_buffer()); PRINT_OPENGL_ERROR();

	set_lines_fbo_dirty();
	set_zombie_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_arrays_positions
 *
 *****************************************************************************/
void PVGL::PVLines::update_arrays_positions(void)
{
	PVLOG_DEBUG("PVGL::PVLines::%s\n", __FUNCTION__);

	if (!picviz_view->is_consistent() || !picviz_view->axes_combination.is_consistent()) {
		return;
	}
	set_lines_fbo_dirty();
	set_zombie_fbo_dirty();
}

/******************************************************************************
 *
 * PVGL::PVLines::reinit_picviz_view
 *
 *****************************************************************************/
void PVGL::PVLines::reinit_picviz_view()
{
	change_axes_count();
}

/******************************************************************************
 *
 * PVGL::PVLines::update_lpr
 *
 *****************************************************************************/
void PVGL::PVLines::update_lpr()
{
	size_t nb_axes = picviz_view->get_axes_count();
	size_t lpr = view->get_max_lines_per_redraw();
	for (unsigned i = 0; i < batches.size(); i++) {
		int nb_vec4;
		int nb_axes_in_batch;
		int array_size;

		if (i == nb_batches - 1) { // The last batch is special (incomplete)
			nb_axes_in_batch = nb_axes;
		} else {
			nb_axes_in_batch = NB_AXES_PER_BATCH;
		}
		nb_vec4 = (nb_axes_in_batch + 3) / 4;

		glBindBuffer(GL_ARRAY_BUFFER, batches[i].vbo_position); PRINT_OPENGL_ERROR();
		array_size = nb_vec4 * lpr;
		glBufferData(GL_ARRAY_BUFFER, array_size * sizeof(vec4), 0, GL_DYNAMIC_DRAW); PRINT_OPENGL_ERROR();
	}

}

/******************************************************************************
 *
 * PVGL::PVLines::translate
 *
 *****************************************************************************/
void PVGL::PVLines::translate(int dx, int dy)
{
	if (abs(dx) > 300 || abs(dy) > 200 || main_fbo_dirty || lines_fbo_dirty || zombie_fbo_dirty) {
		reset_offset();
		set_lines_fbo_dirty();
		//map.set_lines_fbo_dirty();
		set_zombie_fbo_dirty();
		//map.set_zombie_fbo_dirty();
	}

}

