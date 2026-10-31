// dear imgui: Renderer Backend for OpenGL3+
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32, custom..)

// Implementations of these functions are optional and can be removed if not used/needed.
// This file is designed to be included *after* imgui.cpp and imgui.h.

// Note: GLFW callbacks are not thread-safe. If you use this in a multi-threaded context, you may need
// to add mutex locks around the OpenGL context access.

#include "imgui.h"
#include "imgui_impl_opengl3.h"

// OpenGL includes
#include <GL/gl.h>

// GL includes
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// OpenGL Data
struct ImGui_ImplOpenGL3_Data
{
    GLuint       FontTexture;
    GLint        AttribLocationTex;
    GLint        AttribLocationProjMtx;
    GLuint       VboHandle;
    GLuint       ElementsHandle;
    GLsizei      VertexBufferSize;
    GLsizei      IndexBufferSize;
    GLuint       VertexArrayObject;
    GLchar       GlslVersionString[32];
    GLuint       ShaderHandle;
    GLuint       VertHandle;
    GLuint       FragHandle;
    bool         HasClipOrigin;
    bool         UseBufferSubData;

    ImGui_ImplOpenGL3_Data()
    {
        memset(this, 0, sizeof(*this));
        VertexBufferSize = 0;
        IndexBufferSize = 0;
        HasClipOrigin = false;
        UseBufferSubData = true;
    }
};

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : NULL;
}

// Functions
bool ImGui_ImplOpenGL3_Init(const char* glsl_version)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplOpenGL3_Data* bd = IM_NEW(ImGui_ImplOpenGL3_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_opengl3";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

    // Store GLSL version string so we can refer to it later in case we recreate shaders.
    // Note: we DO NOT parse the string to extract version, we will use it as-is.
    if (glsl_version == NULL)
    {
        // Try to detect GLSL version based on the current context
#if defined(IMGUI_IMPL_OPENGL_ES2)
        strcpy(bd->GlslVersionString, "#version 100");
#elif defined(__APPLE__)
        strcpy(bd->GlslVersionString, "#version 150");
#else
        strcpy(bd->GlslVersionString, "#version 130");
#endif
    }
    else
    {
        strncpy(bd->GlslVersionString, glsl_version, sizeof(bd->GlslVersionString) - 1);
    }

    // Check for GL version
    int gl_version[2] = { 0, 0 };
    glGetIntegerv(GL_MAJOR_VERSION, &gl_version[0]);
    glGetIntegerv(GL_MINOR_VERSION, &gl_version[1]);
    bd->HasClipOrigin = (gl_version[0] >= 4 || (gl_version[0] == 3 && gl_version[1] >= 2));

    // Check for buffer sub data
    bd->UseBufferSubData = true;

    // Create font texture
    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Setup render callbacks
    io.SetRendererName("imgui_impl_opengl3");
    io.RenderDrawListsFn = ImGui_ImplOpenGL3_RenderDrawData;

    return true;
}

void ImGui_ImplOpenGL3_Shutdown()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != NULL && "No renderer backend to shutdown, or already shutdown?");

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();

    io.BackendRendererName = NULL;
    io.BackendRendererUserData = NULL;
    IM_DELETE(bd);
}

void ImGui_ImplOpenGL3_NewFrame()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplOpenGL3_Init()?");

    if (!bd->ShaderHandle)
        ImGui_ImplOpenGL3_CreateDeviceObjects();
}

// OpenGL3 Render function.
// (this is the function you need to call in your render loop)
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;

    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    GLboolean last_enable_primitive_restart = glIsEnabled(GL_PRIMITIVE_RESTART);
    GLboolean last_enable_polygon_offset_fill = glIsEnabled(GL_POLYGON_OFFSET_FILL);

    // Setup desired GL state
    // Note: We don't modify the depth test or culling, as those are often set per-draw call.
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+draw_data->DisplaySize (bottom right).
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float ortho_projection[4][4] =
    {
        { 2.0f / (R - L),   0.0f,               0.0f,   0.0f },
        { 0.0f,               2.0f / (T - B),   0.0f,   0.0f },
        { 0.0f,               0.0f,              -1.0f,  0.0f },
        { (R + L) / (L - R), (T + B) / (B - T), 0.0f,   1.0f }
    };
    glUseProgram(bd->ShaderHandle);
    glUniformMatrix4fv(bd->AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glUniform1i(bd->AttribLocationTex, 0);

    // Recreate the VAO every time (this is more efficient than trying to save/restore state)
    glGenVertexArrays(1, &bd->VertexArrayObject);
    glBindVertexArray(bd->VertexArrayObject);

    // Bind vertex/index buffers and attributes
    glBindBuffer(GL_ARRAY_BUFFER, bd->VboHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle);

    // Setup vertex attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

    // Draw
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawCmd* pcmd_begin = cmd_list->CmdBuffer.Data;
        const ImDrawCmd* pcmd_end = pcmd_begin + cmd_list->CmdBuffer.Size;

        // Upload vertex/index buffers
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (const ImDrawCmd* pcmd = pcmd_begin; pcmd != pcmd_end; pcmd++)
        {
            if (pcmd->UserCallback != NULL)
            {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - draw_data->DisplayPos.x) * io.DisplayFramebufferScale.x;
                clip_rect.y = (pcmd->ClipRect.y - draw_data->DisplayPos.y) * io.DisplayFramebufferScale.y;
                clip_rect.z = (pcmd->ClipRect.z - draw_data->DisplayPos.x) * io.DisplayFramebufferScale.x;
                clip_rect.w = (pcmd->ClipRect.w - draw_data->DisplayPos.y) * io.DisplayFramebufferScale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

                    // Bind texture, Draw
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)pcmd->IdxOffset);
                }
            }
        }
    }

    // Restore modified GL state
    glDeleteVertexArrays(1, &bd->VertexArrayObject);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glActiveTexture((GLenum)last_active_texture);
    glUseProgram((GLuint)last_program);
    glBindVertexArray((GLuint)last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)last_element_array_buffer);
    glBlendEquationSeparate((GLenum)last_blend_equation_rgb, (GLenum)last_blend_equation_alpha);
    glBlendFunc((GLenum)last_blend_src, (GLenum)last_blend_dst);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    if (last_enable_primitive_restart) glEnable(GL_PRIMITIVE_RESTART); else glDisable(GL_PRIMITIVE_RESTART);
    if (last_enable_polygon_offset_fill) glEnable(GL_POLYGON_OFFSET_FILL); else glDisable(GL_POLYGON_OFFSET_FILL);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

// If you get a crash here you need to call ImGui_ImplOpenGL3_Init() first!
static void ImGui_ImplOpenGL3_CreateFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Create OpenGL texture
    glGenTextures(1, &bd->FontTexture);
    glBindTexture(GL_TEXTURE_2D, bd->FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

#if defined(GL_UNPACK_ALIGNMENT)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->FontTexture);

    // Restore state
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    if (bd->FontTexture)
    {
        glDeleteTextures(1, &bd->FontTexture);
        ImGui::GetIO().Fonts->SetTexID(0);
        bd->FontTexture = 0;
    }
}

static void ImGui_ImplOpenGL3_CreateDeviceObjects()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar* vertex_shader =
        "#version 130\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "   Frag_UV = UV;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 130\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    bd->ShaderHandle = glCreateProgram();
    bd->VertHandle = glCreateShader(GL_VERTEX_SHADER);
    bd->FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(bd->VertHandle, 1, &vertex_shader, 0);
    glShaderSource(bd->FragHandle, 1, &fragment_shader, 0);
    glCompileShader(bd->VertHandle);
    glCompileShader(bd->FragHandle);
    glAttachShader(bd->ShaderHandle, bd->VertHandle);
    glAttachShader(bd->ShaderHandle, bd->FragHandle);
    glLinkProgram(bd->ShaderHandle);

    bd->AttribLocationTex = glGetUniformLocation(bd->ShaderHandle, "Texture");
    bd->AttribLocationProjMtx = glGetUniformLocation(bd->ShaderHandle, "ProjMtx");
    bd->AttribLocationVtxPos = glGetAttribLocation(bd->ShaderHandle, "Position");
    bd->AttribLocationVtxUV = glGetAttribLocation(bd->ShaderHandle, "UV");
    bd->AttribLocationVtxColor = glGetAttribLocation(bd->ShaderHandle, "Color");

    glGenBuffers(1, &bd->VboHandle);
    glGenBuffers(1, &bd->ElementsHandle);

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)last_array_buffer);
    glBindVertexArray((GLuint)last_vertex_array);
}

static void ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    if (bd->ShaderHandle)
    {
        glDetachShader(bd->ShaderHandle, bd->VertHandle);
        glDetachShader(bd->ShaderHandle, bd->FragHandle);
        glDeleteShader(bd->VertHandle);
        glDeleteShader(bd->FragHandle);
        glDeleteProgram(bd->ShaderHandle);
    }
    if (bd->VboHandle) glDeleteBuffers(1, &bd->VboHandle);
    if (bd->ElementsHandle) glDeleteBuffers(1, &bd->ElementsHandle);
    bd->ShaderHandle = bd->VertHandle = bd->FragHandle = 0;
    bd->VboHandle = bd->ElementsHandle = 0;
}
