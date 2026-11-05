/*
 * Nuklear - v1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_LUM_GL2_H_
#define NK_LUM_GL2_H_

#include <Lumina/lumina.h>

enum nk_lum_init_state{
    NK_LUMINA_DEFAULT = 0,
    NK_LUMINA_INSTALL_CALLBACKS
};
NK_API struct nk_context*   nk_lumina_init(LUMwindow *win, enum nk_lum_init_state);
NK_API void                 nk_lumina_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_lumina_font_stash_end(void);

NK_API void                 nk_lumina_new_frame(void);
NK_API void                 nk_lumina_render(enum nk_anti_aliasing);
NK_API void                 nk_lumina_shutdown(void);

NK_API void                 nk_lumina_char_callback(LUMwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(LUMwindow *win, double xoff, double yoff);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_LUM_GL2_IMPLEMENTATION

#ifndef NK_LUM_TEXT_MAX
#define NK_LUM_TEXT_MAX 256
#endif
#ifndef NK_LUM_DOUBLE_CLICK_LO
#define NK_LUM_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_LUM_DOUBLE_CLICK_HI
#define NK_LUM_DOUBLE_CLICK_HI 0.2
#endif

struct nk_lum_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint font_tex;
};

struct nk_lum_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_lum {
    LUMwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_lum_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_LUM_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} lumina;

NK_INTERN void
nk_lumina_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_lum_device *dev = &lumina.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_lumina_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_lum_device *dev = &lumina.ogl;
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)lumina.display_width,(GLsizei)lumina.display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, lumina.width, lumina.height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_lum_vertex);
        size_t vp = offsetof(struct nk_lum_vertex, position);
        size_t vt = offsetof(struct nk_lum_vertex, uv);
        size_t vc = offsetof(struct nk_lum_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_lum_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_lum_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_lum_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_lum_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_lum_vertex);
        config.null = dev->null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&lumina.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &lumina.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * lumina.fb_scale.x),
                (GLint)((lumina.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * lumina.fb_scale.y),
                (GLint)(cmd->clip_rect.w * lumina.fb_scale.x),
                (GLint)(cmd->clip_rect.h * lumina.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&lumina.ctx);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

NK_API void
nk_lumina_char_callback(LUMwindow *win, unsigned int codepoint)
{
    (void)win;
    if (lumina.text_len < NK_LUM_TEXT_MAX)
        lumina.text[lumina.text_len++] = codepoint;
}

NK_API void
nk_gflw3_scroll_callback(LUMwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    lumina.scroll.x += (float)xoff;
    lumina.scroll.y += (float)yoff;
}

NK_API void
nk_lumina_mouse_button_callback(LUMwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != LUM_MOUSE_BUTTON_LEFT) return;
    lumGetCursorPos(window, &x, &y);
    if (action == LUM_PRESS)  {
        double dt = lumGetTime() - lumina.last_button_click;
        if (dt > NK_LUM_DOUBLE_CLICK_LO && dt < NK_LUM_DOUBLE_CLICK_HI) {
            lumina.is_double_click_down = nk_true;
            lumina.double_click_pos = nk_vec2((float)x, (float)y);
        }
        lumina.last_button_click = lumGetTime();
    } else lumina.is_double_click_down = nk_false;
}

NK_INTERN void
nk_lumina_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = lumGetClipboardString(lumina.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_lumina_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    NK_MEMCPY(str, text, (size_t)len);
    str[len] = '\0';
    lumSetClipboardString(lumina.win, str);
    free(str);
}

NK_API struct nk_context*
nk_lumina_init(LUMwindow *win, enum nk_lum_init_state init_state)
{
    lumina.win = win;
    if (init_state == NK_LUMINA_INSTALL_CALLBACKS) {
        lumSetScrollCallback(win, nk_gflw3_scroll_callback);
        lumSetCharCallback(win, nk_lumina_char_callback);
        lumSetMouseButtonCallback(win, nk_lumina_mouse_button_callback);
    }
    nk_init_default(&lumina.ctx, 0);
    lumina.ctx.clip.copy = nk_lumina_clipboard_copy;
    lumina.ctx.clip.paste = nk_lumina_clipboard_paste;
    lumina.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&lumina.ogl.cmds);

    lumina.is_double_click_down = nk_false;
    lumina.double_click_pos = nk_vec2(0, 0);

    return &lumina.ctx;
}

NK_API void
nk_lumina_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&lumina.atlas);
    nk_font_atlas_begin(&lumina.atlas);
    *atlas = &lumina.atlas;
}

NK_API void
nk_lumina_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&lumina.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_lumina_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&lumina.atlas, nk_handle_id((int)lumina.ogl.font_tex), &lumina.ogl.null);
    if (lumina.atlas.default_font)
        nk_style_set_font(&lumina.ctx, &lumina.atlas.default_font->handle);
}

NK_API void
nk_lumina_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context *ctx = &lumina.ctx;
    struct LUMwindow *win = lumina.win;

    lumGetWindowSize(win, &lumina.width, &lumina.height);
    lumGetFramebufferSize(win, &lumina.display_width, &lumina.display_height);
    lumina.fb_scale.x = (float)lumina.display_width/(float)lumina.width;
    lumina.fb_scale.y = (float)lumina.display_height/(float)lumina.height;

    nk_input_begin(ctx);
    for (i = 0; i < lumina.text_len; ++i)
        nk_input_unicode(ctx, lumina.text[i]);

    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        lumSetInputMode(lumina.win, LUM_CURSOR, LUM_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        lumSetInputMode(lumina.win, LUM_CURSOR, LUM_CURSOR_NORMAL);

    nk_input_key(ctx, NK_KEY_DEL, lumGetKey(win, LUM_KEY_DELETE) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, lumGetKey(win, LUM_KEY_ENTER) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, lumGetKey(win, LUM_KEY_TAB) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, lumGetKey(win, LUM_KEY_BACKSPACE) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_UP, lumGetKey(win, LUM_KEY_UP) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, lumGetKey(win, LUM_KEY_DOWN) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, lumGetKey(win, LUM_KEY_HOME) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, lumGetKey(win, LUM_KEY_END) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, lumGetKey(win, LUM_KEY_HOME) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, lumGetKey(win, LUM_KEY_END) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, lumGetKey(win, LUM_KEY_PAGE_DOWN) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, lumGetKey(win, LUM_KEY_PAGE_UP) == LUM_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, lumGetKey(win, LUM_KEY_LEFT_SHIFT) == LUM_PRESS||
                                    lumGetKey(win, LUM_KEY_RIGHT_SHIFT) == LUM_PRESS);

    if (lumGetKey(win, LUM_KEY_LEFT_CONTROL) == LUM_PRESS ||
        lumGetKey(win, LUM_KEY_RIGHT_CONTROL) == LUM_PRESS) {
        nk_input_key(ctx, NK_KEY_COPY, lumGetKey(win, LUM_KEY_C) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, lumGetKey(win, LUM_KEY_V) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, lumGetKey(win, LUM_KEY_X) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, lumGetKey(win, LUM_KEY_Z) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, lumGetKey(win, LUM_KEY_R) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, lumGetKey(win, LUM_KEY_LEFT) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, lumGetKey(win, LUM_KEY_RIGHT) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, lumGetKey(win, LUM_KEY_B) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, lumGetKey(win, LUM_KEY_E) == LUM_PRESS);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, lumGetKey(win, LUM_KEY_LEFT) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, lumGetKey(win, LUM_KEY_RIGHT) == LUM_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    lumGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
    if (ctx->input.mouse.grabbed) {
        lumSetCursorPos(lumina.win, (double)ctx->input.mouse.prev.x, (double)ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, lumGetMouseButton(win, LUM_MOUSE_BUTTON_LEFT) == LUM_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, lumGetMouseButton(win, LUM_MOUSE_BUTTON_MIDDLE) == LUM_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, lumGetMouseButton(win, LUM_MOUSE_BUTTON_RIGHT) == LUM_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)lumina.double_click_pos.x, (int)lumina.double_click_pos.y, lumina.is_double_click_down);
    nk_input_scroll(ctx, lumina.scroll);
    nk_input_end(&lumina.ctx);
    lumina.text_len = 0;
    lumina.scroll = nk_vec2(0,0);
}

NK_API
void nk_lumina_shutdown(void)
{
    struct nk_lum_device *dev = &lumina.ogl;
    nk_font_atlas_clear(&lumina.atlas);
    nk_free(&lumina.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    NK_MEMSET(&lumina, 0, sizeof(lumina));
}

#endif
