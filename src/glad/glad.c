/*

    OpenGL ES loader generated by glad 0.1.10a0 on Tue May 24 19:34:21 2016.

    Language/Generator: C/C++
    Specification: gl
    APIs: gles2=3.0
    Profile: core
    Extensions:
        GL_EXT_texture_filter_anisotropic, GL_KHR_debug
    Loader: No

    Commandline:
        --profile="core" --api="gles2=3.0" --generator="c" --spec="gl" --no-loader --extensions="GL_EXT_texture_filter_anisotropic,GL_KHR_debug"
    Online:
        http://glad.dav1d.de/#profile=core&language=c&specification=gl&api=gles2%3D3.0&extensions=GL_EXT_texture_filter_anisotropic&extensions=GL_KHR_debug
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

struct gladGLversionStruct GLVersion;

#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_3_0)
#define _GLAD_IS_SOME_NEW_VERSION 1
#endif

static int max_loaded_major;
static int max_loaded_minor;

static const char *exts = NULL;
static int num_exts_i = 0;
static const char **exts_i = NULL;

static int get_exts(void) {
#ifdef _GLAD_IS_SOME_NEW_VERSION
    if(max_loaded_major < 3) {
#endif
        exts = (const char *)glGetString(GL_EXTENSIONS);
#ifdef _GLAD_IS_SOME_NEW_VERSION
    } else {
        int index;

        num_exts_i = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts_i);
        if (num_exts_i > 0) {
            exts_i = (const char **)realloc((void *)exts_i, num_exts_i * sizeof *exts_i);
        }

        if (exts_i == NULL) {
            return 0;
        }

        for(index = 0; index < num_exts_i; index++) {
            exts_i[index] = (const char*)glGetStringi(GL_EXTENSIONS, index);
        }
    }
#endif
    return 1;
}

static void free_exts(void) {
    if (exts_i != NULL) {
        free((char **)exts_i);
        exts_i = NULL;
    }
}

static int has_ext(const char *ext) {
#ifdef _GLAD_IS_SOME_NEW_VERSION
    if(max_loaded_major < 3) {
#endif
        const char *extensions;
        const char *loc;
        const char *terminator;
        extensions = exts;
        if(extensions == NULL || ext == NULL) {
            return 0;
        }

        while(1) {
            loc = strstr(extensions, ext);
            if(loc == NULL) {
                return 0;
            }

            terminator = loc + strlen(ext);
            if((loc == extensions || *(loc - 1) == ' ') &&
                (*terminator == ' ' || *terminator == '\0')) {
                return 1;
            }
            extensions = terminator;
        }
#ifdef _GLAD_IS_SOME_NEW_VERSION
    } else {
        int index;

        for(index = 0; index < num_exts_i; index++) {
            const char *e = exts_i[index];

            if(strcmp(e, ext) == 0) {
                return 1;
            }
        }
    }
#endif

    return 0;
}
int GLAD_GL_ES_VERSION_2_0;
int GLAD_GL_ES_VERSION_3_0;
PFNGLFLUSHPROC glad_glFlush;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_glGetRenderbufferParameteriv;
PFNGLCLEARCOLORPROC glad_glClearColor;
PFNGLGETUNIFORMBLOCKINDEXPROC glad_glGetUniformBlockIndex;
PFNGLCLEARBUFFERIVPROC glad_glClearBufferiv;
PFNGLSTENCILMASKSEPARATEPROC glad_glStencilMaskSeparate;
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_glGetVertexAttribPointerv;
PFNGLLINKPROGRAMPROC glad_glLinkProgram;
PFNGLBINDTEXTUREPROC glad_glBindTexture;
PFNGLGETSTRINGIPROC glad_glGetStringi;
PFNGLFENCESYNCPROC glad_glFenceSync;
PFNGLUNIFORM3UIPROC glad_glUniform3ui;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;
PFNGLGETSTRINGPROC glad_glGetString;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glad_glCompressedTexSubImage3D;
PFNGLDETACHSHADERPROC glad_glDetachShader;
PFNGLVERTEXATTRIBI4UIVPROC glad_glVertexAttribI4uiv;
PFNGLENDQUERYPROC glad_glEndQuery;
PFNGLBINDSAMPLERPROC glad_glBindSampler;
PFNGLLINEWIDTHPROC glad_glLineWidth;
PFNGLUNIFORM2FVPROC glad_glUniform2fv;
PFNGLGETINTEGERI_VPROC glad_glGetIntegeri_v;
PFNGLGETINTERNALFORMATIVPROC glad_glGetInternalformativ;
PFNGLCOMPILESHADERPROC glad_glCompileShader;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glad_glGetTransformFeedbackVarying;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
PFNGLSTENCILOPSEPARATEPROC glad_glStencilOpSeparate;
PFNGLSTENCILFUNCSEPARATEPROC glad_glStencilFuncSeparate;
PFNGLBINDBUFFERRANGEPROC glad_glBindBufferRange;
PFNGLVERTEXATTRIB4FPROC glad_glVertexAttrib4f;
PFNGLGENQUERIESPROC glad_glGenQueries;
PFNGLGETBUFFERPARAMETERI64VPROC glad_glGetBufferParameteri64v;
PFNGLDEPTHRANGEFPROC glad_glDepthRangef;
PFNGLUNIFORM4IVPROC glad_glUniform4iv;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv;
PFNGLCLEARSTENCILPROC glad_glClearStencil;
PFNGLUNIFORMMATRIX2X3FVPROC glad_glUniformMatrix2x3fv;
PFNGLGENTRANSFORMFEEDBACKSPROC glad_glGenTransformFeedbacks;
PFNGLGETVERTEXATTRIBIUIVPROC glad_glGetVertexAttribIuiv;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage;
PFNGLSAMPLERPARAMETERIPROC glad_glSamplerParameteri;
PFNGLGENTEXTURESPROC glad_glGenTextures;
PFNGLSAMPLERPARAMETERFPROC glad_glSamplerParameterf;
PFNGLDEPTHFUNCPROC glad_glDepthFunc;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D;
PFNGLISVERTEXARRAYPROC glad_glIsVertexArray;
PFNGLUNIFORM1FPROC glad_glUniform1f;
PFNGLGETVERTEXATTRIBFVPROC glad_glGetVertexAttribfv;
PFNGLPROGRAMBINARYPROC glad_glProgramBinary;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv;
PFNGLCREATESHADERPROC glad_glCreateShader;
PFNGLISBUFFERPROC glad_glIsBuffer;
PFNGLUNIFORM1IPROC glad_glUniform1i;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D;
PFNGLDISABLEPROC glad_glDisable;
PFNGLUNIFORM2IPROC glad_glUniform2i;
PFNGLBLENDFUNCSEPARATEPROC glad_glBlendFuncSeparate;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
PFNGLCOLORMASKPROC glad_glColorMask;
PFNGLHINTPROC glad_glHint;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glad_glFramebufferTextureLayer;
PFNGLBLENDEQUATIONPROC glad_glBlendEquation;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer;
PFNGLENDTRANSFORMFEEDBACKPROC glad_glEndTransformFeedback;
PFNGLCULLFACEPROC glad_glCullFace;
PFNGLUNIFORMMATRIX3X2FVPROC glad_glUniformMatrix3x2fv;
PFNGLPAUSETRANSFORMFEEDBACKPROC glad_glPauseTransformFeedback;
PFNGLTEXSTORAGE2DPROC glad_glTexStorage2D;
PFNGLUNIFORM4FVPROC glad_glUniform4fv;
PFNGLBEGINTRANSFORMFEEDBACKPROC glad_glBeginTransformFeedback;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
PFNGLISSAMPLERPROC glad_glIsSampler;
PFNGLVERTEXATTRIBDIVISORPROC glad_glVertexAttribDivisor;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage;
PFNGLWAITSYNCPROC glad_glWaitSync;
PFNGLATTACHSHADERPROC glad_glAttachShader;
PFNGLUNIFORMMATRIX4X3FVPROC glad_glUniformMatrix4x3fv;
PFNGLUNIFORM3IPROC glad_glUniform3i;
PFNGLCLEARBUFFERFVPROC glad_glClearBufferfv;
PFNGLDELETETRANSFORMFEEDBACKSPROC glad_glDeleteTransformFeedbacks;
PFNGLDELETESAMPLERSPROC glad_glDeleteSamplers;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
PFNGLTEXSUBIMAGE3DPROC glad_glTexSubImage3D;
PFNGLGETINTEGER64I_VPROC glad_glGetInteger64i_v;
PFNGLSHADERBINARYPROC glad_glShaderBinary;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D;
PFNGLUNIFORM3FPROC glad_glUniform3f;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer;
PFNGLBINDATTRIBLOCATIONPROC glad_glBindAttribLocation;
PFNGLUNIFORMMATRIX4X2FVPROC glad_glUniformMatrix4x2fv;
PFNGLDRAWELEMENTSPROC glad_glDrawElements;
PFNGLUNIFORM2IVPROC glad_glUniform2iv;
PFNGLVERTEXATTRIB1FVPROC glad_glVertexAttrib1fv;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
PFNGLBINDBUFFERBASEPROC glad_glBindBufferBase;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData;
PFNGLUNIFORM1IVPROC glad_glUniform1iv;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv;
PFNGLMAPBUFFERRANGEPROC glad_glMapBufferRange;
PFNGLREADBUFFERPROC glad_glReadBuffer;
PFNGLTEXSTORAGE3DPROC glad_glTexStorage3D;
PFNGLCLIENTWAITSYNCPROC glad_glClientWaitSync;
PFNGLDRAWARRAYSINSTANCEDPROC glad_glDrawArraysInstanced;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap;
PFNGLGETSHADERIVPROC glad_glGetShaderiv;
PFNGLUNIFORMMATRIX3X4FVPROC glad_glUniformMatrix3x4fv;
PFNGLVERTEXATTRIB3FPROC glad_glVertexAttrib3f;
PFNGLGETACTIVEATTRIBPROC glad_glGetActiveAttrib;
PFNGLBLENDCOLORPROC glad_glBlendColor;
PFNGLGETSHADERPRECISIONFORMATPROC glad_glGetShaderPrecisionFormat;
PFNGLRESUMETRANSFORMFEEDBACKPROC glad_glResumeTransformFeedback;
PFNGLUNMAPBUFFERPROC glad_glUnmapBuffer;
PFNGLDEPTHMASKPROC glad_glDepthMask;
PFNGLUSEPROGRAMPROC glad_glUseProgram;
PFNGLCOPYBUFFERSUBDATAPROC glad_glCopyBufferSubData;
PFNGLSHADERSOURCEPROC glad_glShaderSource;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers;
PFNGLISSYNCPROC glad_glIsSync;
PFNGLISTRANSFORMFEEDBACKPROC glad_glIsTransformFeedback;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers;
PFNGLDRAWARRAYSPROC glad_glDrawArrays;
PFNGLUNIFORM1UIPROC glad_glUniform1ui;
PFNGLISPROGRAMPROC glad_glIsProgram;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D;
PFNGLGETSYNCIVPROC glad_glGetSynciv;
PFNGLGETUNIFORMIVPROC glad_glGetUniformiv;
PFNGLUNIFORM4IPROC glad_glUniform4i;
PFNGLCLEARPROC glad_glClear;
PFNGLVERTEXATTRIB4FVPROC glad_glVertexAttrib4fv;
PFNGLPROGRAMPARAMETERIPROC glad_glProgramParameteri;
PFNGLRELEASESHADERCOMPILERPROC glad_glReleaseShaderCompiler;
PFNGLUNIFORM2FPROC glad_glUniform2f;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
PFNGLDRAWRANGEELEMENTSPROC glad_glDrawRangeElements;
PFNGLBINDBUFFERPROC glad_glBindBuffer;
PFNGLISENABLEDPROC glad_glIsEnabled;
PFNGLSTENCILOPPROC glad_glStencilOp;
PFNGLREADPIXELSPROC glad_glReadPixels;
PFNGLGETQUERYOBJECTUIVPROC glad_glGetQueryObjectuiv;
PFNGLBINDTRANSFORMFEEDBACKPROC glad_glBindTransformFeedback;
PFNGLUNIFORM4FPROC glad_glUniform4f;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glad_glGetFramebufferAttachmentParameteriv;
PFNGLUNIFORMMATRIX2X4FVPROC glad_glUniformMatrix2x4fv;
PFNGLUNIFORM2UIVPROC glad_glUniform2uiv;
PFNGLUNIFORM3FVPROC glad_glUniform3fv;
PFNGLBUFFERDATAPROC glad_glBufferData;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glad_glCompressedTexImage3D;
PFNGLDELETESYNCPROC glad_glDeleteSync;
PFNGLCOPYTEXSUBIMAGE3DPROC glad_glCopyTexSubImage3D;
PFNGLGETFRAGDATALOCATIONPROC glad_glGetFragDataLocation;
PFNGLGETERRORPROC glad_glGetError;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glad_glGetActiveUniformBlockName;
PFNGLGETVERTEXATTRIBIVPROC glad_glGetVertexAttribiv;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv;
PFNGLGETPROGRAMBINARYPROC glad_glGetProgramBinary;
PFNGLVERTEXATTRIB3FVPROC glad_glVertexAttrib3fv;
PFNGLGETFLOATVPROC glad_glGetFloatv;
PFNGLUNIFORM3IVPROC glad_glUniform3iv;
PFNGLVERTEXATTRIB2FVPROC glad_glVertexAttrib2fv;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers;
PFNGLDRAWBUFFERSPROC glad_glDrawBuffers;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glad_glGetActiveUniformBlockiv;
PFNGLSTENCILFUNCPROC glad_glStencilFunc;
PFNGLGETINTEGERVPROC glad_glGetIntegerv;
PFNGLGETATTACHEDSHADERSPROC glad_glGetAttachedShaders;
PFNGLUNIFORMBLOCKBINDINGPROC glad_glUniformBlockBinding;
PFNGLISRENDERBUFFERPROC glad_glIsRenderbuffer;
PFNGLGETBUFFERPOINTERVPROC glad_glGetBufferPointerv;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
PFNGLUNIFORM1FVPROC glad_glUniform1fv;
PFNGLBEGINQUERYPROC glad_glBeginQuery;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_glRenderbufferStorageMultisample;
PFNGLUNIFORM4UIVPROC glad_glUniform4uiv;
PFNGLISQUERYPROC glad_glIsQuery;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray;
PFNGLVERTEXATTRIBI4IVPROC glad_glVertexAttribI4iv;
PFNGLGETQUERYIVPROC glad_glGetQueryiv;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
PFNGLGETSAMPLERPARAMETERFVPROC glad_glGetSamplerParameterfv;
PFNGLSTENCILMASKPROC glad_glStencilMask;
PFNGLUNIFORM4UIPROC glad_glUniform4ui;
PFNGLSAMPLERPARAMETERFVPROC glad_glSamplerParameterfv;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
PFNGLISTEXTUREPROC glad_glIsTexture;
PFNGLGETUNIFORMINDICESPROC glad_glGetUniformIndices;
PFNGLISSHADERPROC glad_glIsShader;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
PFNGLGETINTEGER64VPROC glad_glGetInteger64v;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv;
PFNGLINVALIDATESUBFRAMEBUFFERPROC glad_glInvalidateSubFramebuffer;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv;
PFNGLDRAWELEMENTSINSTANCEDPROC glad_glDrawElementsInstanced;
PFNGLENABLEPROC glad_glEnable;
PFNGLGETACTIVEUNIFORMSIVPROC glad_glGetActiveUniformsiv;
PFNGLVERTEXATTRIBI4IPROC glad_glVertexAttribI4i;
PFNGLVERTEXATTRIBI4UIPROC glad_glVertexAttribI4ui;
PFNGLDELETEQUERIESPROC glad_glDeleteQueries;
PFNGLBLENDEQUATIONSEPARATEPROC glad_glBlendEquationSeparate;
PFNGLGENBUFFERSPROC glad_glGenBuffers;
PFNGLFINISHPROC glad_glFinish;
PFNGLGETATTRIBLOCATIONPROC glad_glGetAttribLocation;
PFNGLDELETESHADERPROC glad_glDeleteShader;
PFNGLBLENDFUNCPROC glad_glBlendFunc;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
PFNGLTEXIMAGE3DPROC glad_glTexImage3D;
PFNGLGENSAMPLERSPROC glad_glGenSamplers;
PFNGLGETSAMPLERPARAMETERIVPROC glad_glGetSamplerParameteriv;
PFNGLISFRAMEBUFFERPROC glad_glIsFramebuffer;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glad_glFlushMappedBufferRange;
PFNGLVIEWPORTPROC glad_glViewport;
PFNGLINVALIDATEFRAMEBUFFERPROC glad_glInvalidateFramebuffer;
PFNGLUNIFORM1UIVPROC glad_glUniform1uiv;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glad_glTransformFeedbackVaryings;
PFNGLVERTEXATTRIB2FPROC glad_glVertexAttrib2f;
PFNGLVERTEXATTRIB1FPROC glad_glVertexAttrib1f;
PFNGLUNIFORM2UIPROC glad_glUniform2ui;
PFNGLGETUNIFORMFVPROC glad_glGetUniformfv;
PFNGLGETUNIFORMUIVPROC glad_glGetUniformuiv;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;
PFNGLGETVERTEXATTRIBIIVPROC glad_glGetVertexAttribIiv;
PFNGLGETACTIVEUNIFORMPROC glad_glGetActiveUniform;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf;
PFNGLCLEARBUFFERFIPROC glad_glClearBufferfi;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
PFNGLFRONTFACEPROC glad_glFrontFace;
PFNGLCLEARDEPTHFPROC glad_glClearDepthf;
PFNGLGETSHADERSOURCEPROC glad_glGetShaderSource;
PFNGLSAMPLERPARAMETERIVPROC glad_glSamplerParameteriv;
PFNGLSCISSORPROC glad_glScissor;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv;
PFNGLPIXELSTOREIPROC glad_glPixelStorei;
PFNGLVALIDATEPROGRAMPROC glad_glValidateProgram;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset;
PFNGLCLEARBUFFERUIVPROC glad_glClearBufferuiv;
PFNGLUNIFORM3UIVPROC glad_glUniform3uiv;
PFNGLVERTEXATTRIBIPOINTERPROC glad_glVertexAttribIPointer;
int GLAD_GL_KHR_debug = 0;
int GLAD_GL_EXT_texture_filter_anisotropic;
PFNGLDEBUGMESSAGECONTROLPROC glad_glDebugMessageControl;
PFNGLDEBUGMESSAGEINSERTPROC glad_glDebugMessageInsert;
PFNGLDEBUGMESSAGECALLBACKPROC glad_glDebugMessageCallback;
PFNGLGETDEBUGMESSAGELOGPROC glad_glGetDebugMessageLog;
PFNGLPUSHDEBUGGROUPPROC glad_glPushDebugGroup;
PFNGLPOPDEBUGGROUPPROC glad_glPopDebugGroup;
PFNGLOBJECTLABELPROC glad_glObjectLabel;
PFNGLGETOBJECTLABELPROC glad_glGetObjectLabel;
PFNGLOBJECTPTRLABELPROC glad_glObjectPtrLabel;
PFNGLGETOBJECTPTRLABELPROC glad_glGetObjectPtrLabel;
PFNGLGETPOINTERVPROC glad_glGetPointerv;
PFNGLDEBUGMESSAGECONTROLKHRPROC glad_glDebugMessageControlKHR;
PFNGLDEBUGMESSAGEINSERTKHRPROC glad_glDebugMessageInsertKHR;
PFNGLDEBUGMESSAGECALLBACKKHRPROC glad_glDebugMessageCallbackKHR;
PFNGLGETDEBUGMESSAGELOGKHRPROC glad_glGetDebugMessageLogKHR;
PFNGLPUSHDEBUGGROUPKHRPROC glad_glPushDebugGroupKHR;
PFNGLPOPDEBUGGROUPKHRPROC glad_glPopDebugGroupKHR;
PFNGLOBJECTLABELKHRPROC glad_glObjectLabelKHR;
PFNGLGETOBJECTLABELKHRPROC glad_glGetObjectLabelKHR;
PFNGLOBJECTPTRLABELKHRPROC glad_glObjectPtrLabelKHR;
PFNGLGETOBJECTPTRLABELKHRPROC glad_glGetObjectPtrLabelKHR;
PFNGLGETPOINTERVKHRPROC glad_glGetPointervKHR;
static void load_GL_ES_VERSION_2_0(GLADloadproc load) {
	if(!GLAD_GL_ES_VERSION_2_0) return;
	glad_glActiveTexture = (PFNGLACTIVETEXTUREPROC)load("glActiveTexture");
	glad_glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
	glad_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)load("glBindAttribLocation");
	glad_glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
	glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
	glad_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)load("glBindRenderbuffer");
	glad_glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
	glad_glBlendColor = (PFNGLBLENDCOLORPROC)load("glBlendColor");
	glad_glBlendEquation = (PFNGLBLENDEQUATIONPROC)load("glBlendEquation");
	glad_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)load("glBlendEquationSeparate");
	glad_glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
	glad_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)load("glBlendFuncSeparate");
	glad_glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
	glad_glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
	glad_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
	glad_glClear = (PFNGLCLEARPROC)load("glClear");
	glad_glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
	glad_glClearDepthf = (PFNGLCLEARDEPTHFPROC)load("glClearDepthf");
	glad_glClearStencil = (PFNGLCLEARSTENCILPROC)load("glClearStencil");
	glad_glColorMask = (PFNGLCOLORMASKPROC)load("glColorMask");
	glad_glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
	glad_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)load("glCompressedTexImage2D");
	glad_glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)load("glCompressedTexSubImage2D");
	glad_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)load("glCopyTexImage2D");
	glad_glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)load("glCopyTexSubImage2D");
	glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
	glad_glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
	glad_glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
	glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");
	glad_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
	glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
	glad_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)load("glDeleteRenderbuffers");
	glad_glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
	glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
	glad_glDepthFunc = (PFNGLDEPTHFUNCPROC)load("glDepthFunc");
	glad_glDepthMask = (PFNGLDEPTHMASKPROC)load("glDepthMask");
	glad_glDepthRangef = (PFNGLDEPTHRANGEFPROC)load("glDepthRangef");
	glad_glDetachShader = (PFNGLDETACHSHADERPROC)load("glDetachShader");
	glad_glDisable = (PFNGLDISABLEPROC)load("glDisable");
	glad_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load("glDisableVertexAttribArray");
	glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
	glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
	glad_glEnable = (PFNGLENABLEPROC)load("glEnable");
	glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
	glad_glFinish = (PFNGLFINISHPROC)load("glFinish");
	glad_glFlush = (PFNGLFLUSHPROC)load("glFlush");
	glad_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)load("glFramebufferRenderbuffer");
	glad_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
	glad_glFrontFace = (PFNGLFRONTFACEPROC)load("glFrontFace");
	glad_glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
	glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)load("glGenerateMipmap");
	glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)load("glGenFramebuffers");
	glad_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)load("glGenRenderbuffers");
	glad_glGenTextures = (PFNGLGENTEXTURESPROC)load("glGenTextures");
	glad_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)load("glGetActiveAttrib");
	glad_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)load("glGetActiveUniform");
	glad_glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)load("glGetAttachedShaders");
	glad_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)load("glGetAttribLocation");
	glad_glGetBooleanv = (PFNGLGETBOOLEANVPROC)load("glGetBooleanv");
	glad_glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)load("glGetBufferParameteriv");
	glad_glGetError = (PFNGLGETERRORPROC)load("glGetError");
	glad_glGetFloatv = (PFNGLGETFLOATVPROC)load("glGetFloatv");
	glad_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)load("glGetFramebufferAttachmentParameteriv");
	glad_glGetIntegerv = (PFNGLGETINTEGERVPROC)load("glGetIntegerv");
	glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
	glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
	glad_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)load("glGetRenderbufferParameteriv");
	glad_glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
	glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
	glad_glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)load("glGetShaderPrecisionFormat");
	glad_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)load("glGetShaderSource");
	glad_glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
	glad_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)load("glGetTexParameterfv");
	glad_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)load("glGetTexParameteriv");
	glad_glGetUniformfv = (PFNGLGETUNIFORMFVPROC)load("glGetUniformfv");
	glad_glGetUniformiv = (PFNGLGETUNIFORMIVPROC)load("glGetUniformiv");
	glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
	glad_glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)load("glGetVertexAttribfv");
	glad_glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)load("glGetVertexAttribiv");
	glad_glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)load("glGetVertexAttribPointerv");
	glad_glHint = (PFNGLHINTPROC)load("glHint");
	glad_glIsBuffer = (PFNGLISBUFFERPROC)load("glIsBuffer");
	glad_glIsEnabled = (PFNGLISENABLEDPROC)load("glIsEnabled");
	glad_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)load("glIsFramebuffer");
	glad_glIsProgram = (PFNGLISPROGRAMPROC)load("glIsProgram");
	glad_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)load("glIsRenderbuffer");
	glad_glIsShader = (PFNGLISSHADERPROC)load("glIsShader");
	glad_glIsTexture = (PFNGLISTEXTUREPROC)load("glIsTexture");
	glad_glLineWidth = (PFNGLLINEWIDTHPROC)load("glLineWidth");
	glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
	glad_glPixelStorei = (PFNGLPIXELSTOREIPROC)load("glPixelStorei");
	glad_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)load("glPolygonOffset");
	glad_glReadPixels = (PFNGLREADPIXELSPROC)load("glReadPixels");
	glad_glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)load("glReleaseShaderCompiler");
	glad_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)load("glRenderbufferStorage");
	glad_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)load("glSampleCoverage");
	glad_glScissor = (PFNGLSCISSORPROC)load("glScissor");
	glad_glShaderBinary = (PFNGLSHADERBINARYPROC)load("glShaderBinary");
	glad_glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
	glad_glStencilFunc = (PFNGLSTENCILFUNCPROC)load("glStencilFunc");
	glad_glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)load("glStencilFuncSeparate");
	glad_glStencilMask = (PFNGLSTENCILMASKPROC)load("glStencilMask");
	glad_glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)load("glStencilMaskSeparate");
	glad_glStencilOp = (PFNGLSTENCILOPPROC)load("glStencilOp");
	glad_glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)load("glStencilOpSeparate");
	glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
	glad_glTexParameterf = (PFNGLTEXPARAMETERFPROC)load("glTexParameterf");
	glad_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)load("glTexParameterfv");
	glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");
	glad_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)load("glTexParameteriv");
	glad_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)load("glTexSubImage2D");
	glad_glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
	glad_glUniform1fv = (PFNGLUNIFORM1FVPROC)load("glUniform1fv");
	glad_glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
	glad_glUniform1iv = (PFNGLUNIFORM1IVPROC)load("glUniform1iv");
	glad_glUniform2f = (PFNGLUNIFORM2FPROC)load("glUniform2f");
	glad_glUniform2fv = (PFNGLUNIFORM2FVPROC)load("glUniform2fv");
	glad_glUniform2i = (PFNGLUNIFORM2IPROC)load("glUniform2i");
	glad_glUniform2iv = (PFNGLUNIFORM2IVPROC)load("glUniform2iv");
	glad_glUniform3f = (PFNGLUNIFORM3FPROC)load("glUniform3f");
	glad_glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
	glad_glUniform3i = (PFNGLUNIFORM3IPROC)load("glUniform3i");
	glad_glUniform3iv = (PFNGLUNIFORM3IVPROC)load("glUniform3iv");
	glad_glUniform4f = (PFNGLUNIFORM4FPROC)load("glUniform4f");
	glad_glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
	glad_glUniform4i = (PFNGLUNIFORM4IPROC)load("glUniform4i");
	glad_glUniform4iv = (PFNGLUNIFORM4IVPROC)load("glUniform4iv");
	glad_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)load("glUniformMatrix2fv");
	glad_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
	glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
	glad_glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
	glad_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)load("glValidateProgram");
	glad_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)load("glVertexAttrib1f");
	glad_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)load("glVertexAttrib1fv");
	glad_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)load("glVertexAttrib2f");
	glad_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)load("glVertexAttrib2fv");
	glad_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)load("glVertexAttrib3f");
	glad_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)load("glVertexAttrib3fv");
	glad_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)load("glVertexAttrib4f");
	glad_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)load("glVertexAttrib4fv");
	glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
	glad_glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
}
static void load_GL_ES_VERSION_3_0(GLADloadproc load) {
	if(!GLAD_GL_ES_VERSION_3_0) return;
	glad_glReadBuffer = (PFNGLREADBUFFERPROC)load("glReadBuffer");
	glad_glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)load("glDrawRangeElements");
	glad_glTexImage3D = (PFNGLTEXIMAGE3DPROC)load("glTexImage3D");
	glad_glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)load("glTexSubImage3D");
	glad_glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)load("glCopyTexSubImage3D");
	glad_glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)load("glCompressedTexImage3D");
	glad_glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)load("glCompressedTexSubImage3D");
	glad_glGenQueries = (PFNGLGENQUERIESPROC)load("glGenQueries");
	glad_glDeleteQueries = (PFNGLDELETEQUERIESPROC)load("glDeleteQueries");
	glad_glIsQuery = (PFNGLISQUERYPROC)load("glIsQuery");
	glad_glBeginQuery = (PFNGLBEGINQUERYPROC)load("glBeginQuery");
	glad_glEndQuery = (PFNGLENDQUERYPROC)load("glEndQuery");
	glad_glGetQueryiv = (PFNGLGETQUERYIVPROC)load("glGetQueryiv");
	glad_glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)load("glGetQueryObjectuiv");
	glad_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)load("glUnmapBuffer");
	glad_glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)load("glGetBufferPointerv");
	glad_glDrawBuffers = (PFNGLDRAWBUFFERSPROC)load("glDrawBuffers");
	glad_glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)load("glUniformMatrix2x3fv");
	glad_glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)load("glUniformMatrix3x2fv");
	glad_glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)load("glUniformMatrix2x4fv");
	glad_glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)load("glUniformMatrix4x2fv");
	glad_glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)load("glUniformMatrix3x4fv");
	glad_glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)load("glUniformMatrix4x3fv");
	glad_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)load("glBlitFramebuffer");
	glad_glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)load("glRenderbufferStorageMultisample");
	glad_glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)load("glFramebufferTextureLayer");
	glad_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)load("glMapBufferRange");
	glad_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)load("glFlushMappedBufferRange");
	glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
	glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
	glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
	glad_glIsVertexArray = (PFNGLISVERTEXARRAYPROC)load("glIsVertexArray");
	glad_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)load("glGetIntegeri_v");
	glad_glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)load("glBeginTransformFeedback");
	glad_glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)load("glEndTransformFeedback");
	glad_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)load("glBindBufferRange");
	glad_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)load("glBindBufferBase");
	glad_glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)load("glTransformFeedbackVaryings");
	glad_glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)load("glGetTransformFeedbackVarying");
	glad_glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
	glad_glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)load("glGetVertexAttribIiv");
	glad_glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)load("glGetVertexAttribIuiv");
	glad_glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)load("glVertexAttribI4i");
	glad_glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)load("glVertexAttribI4ui");
	glad_glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)load("glVertexAttribI4iv");
	glad_glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)load("glVertexAttribI4uiv");
	glad_glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)load("glGetUniformuiv");
	glad_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)load("glGetFragDataLocation");
	glad_glUniform1ui = (PFNGLUNIFORM1UIPROC)load("glUniform1ui");
	glad_glUniform2ui = (PFNGLUNIFORM2UIPROC)load("glUniform2ui");
	glad_glUniform3ui = (PFNGLUNIFORM3UIPROC)load("glUniform3ui");
	glad_glUniform4ui = (PFNGLUNIFORM4UIPROC)load("glUniform4ui");
	glad_glUniform1uiv = (PFNGLUNIFORM1UIVPROC)load("glUniform1uiv");
	glad_glUniform2uiv = (PFNGLUNIFORM2UIVPROC)load("glUniform2uiv");
	glad_glUniform3uiv = (PFNGLUNIFORM3UIVPROC)load("glUniform3uiv");
	glad_glUniform4uiv = (PFNGLUNIFORM4UIVPROC)load("glUniform4uiv");
	glad_glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)load("glClearBufferiv");
	glad_glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)load("glClearBufferuiv");
	glad_glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)load("glClearBufferfv");
	glad_glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)load("glClearBufferfi");
	glad_glGetStringi = (PFNGLGETSTRINGIPROC)load("glGetStringi");
	glad_glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)load("glCopyBufferSubData");
	glad_glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)load("glGetUniformIndices");
	glad_glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)load("glGetActiveUniformsiv");
	glad_glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)load("glGetUniformBlockIndex");
	glad_glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)load("glGetActiveUniformBlockiv");
	glad_glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)load("glGetActiveUniformBlockName");
	glad_glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)load("glUniformBlockBinding");
	glad_glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)load("glDrawArraysInstanced");
	glad_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)load("glDrawElementsInstanced");
	glad_glFenceSync = (PFNGLFENCESYNCPROC)load("glFenceSync");
	glad_glIsSync = (PFNGLISSYNCPROC)load("glIsSync");
	glad_glDeleteSync = (PFNGLDELETESYNCPROC)load("glDeleteSync");
	glad_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)load("glClientWaitSync");
	glad_glWaitSync = (PFNGLWAITSYNCPROC)load("glWaitSync");
	glad_glGetInteger64v = (PFNGLGETINTEGER64VPROC)load("glGetInteger64v");
	glad_glGetSynciv = (PFNGLGETSYNCIVPROC)load("glGetSynciv");
	glad_glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)load("glGetInteger64i_v");
	glad_glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)load("glGetBufferParameteri64v");
	glad_glGenSamplers = (PFNGLGENSAMPLERSPROC)load("glGenSamplers");
	glad_glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)load("glDeleteSamplers");
	glad_glIsSampler = (PFNGLISSAMPLERPROC)load("glIsSampler");
	glad_glBindSampler = (PFNGLBINDSAMPLERPROC)load("glBindSampler");
	glad_glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)load("glSamplerParameteri");
	glad_glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)load("glSamplerParameteriv");
	glad_glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)load("glSamplerParameterf");
	glad_glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)load("glSamplerParameterfv");
	glad_glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)load("glGetSamplerParameteriv");
	glad_glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)load("glGetSamplerParameterfv");
	glad_glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");
	glad_glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)load("glBindTransformFeedback");
	glad_glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)load("glDeleteTransformFeedbacks");
	glad_glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)load("glGenTransformFeedbacks");
	glad_glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)load("glIsTransformFeedback");
	glad_glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)load("glPauseTransformFeedback");
	glad_glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)load("glResumeTransformFeedback");
	glad_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)load("glGetProgramBinary");
	glad_glProgramBinary = (PFNGLPROGRAMBINARYPROC)load("glProgramBinary");
	glad_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)load("glProgramParameteri");
	glad_glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)load("glInvalidateFramebuffer");
	glad_glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)load("glInvalidateSubFramebuffer");
	glad_glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)load("glTexStorage2D");
	glad_glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)load("glTexStorage3D");
	glad_glGetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)load("glGetInternalformativ");
}
static void load_GL_KHR_debug(GLADloadproc load) {
	if(!GLAD_GL_KHR_debug) return;
	glad_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)load("glDebugMessageControl");
	glad_glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)load("glDebugMessageInsert");
	glad_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)load("glDebugMessageCallback");
	glad_glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)load("glGetDebugMessageLog");
	glad_glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)load("glPushDebugGroup");
	glad_glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)load("glPopDebugGroup");
	glad_glObjectLabel = (PFNGLOBJECTLABELPROC)load("glObjectLabel");
	glad_glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)load("glGetObjectLabel");
	glad_glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)load("glObjectPtrLabel");
	glad_glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)load("glGetObjectPtrLabel");
	glad_glGetPointerv = (PFNGLGETPOINTERVPROC)load("glGetPointerv");
	glad_glDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC)load("glDebugMessageControlKHR");
	glad_glDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC)load("glDebugMessageInsertKHR");
	glad_glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC)load("glDebugMessageCallbackKHR");
	glad_glGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC)load("glGetDebugMessageLogKHR");
	glad_glPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC)load("glPushDebugGroupKHR");
	glad_glPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC)load("glPopDebugGroupKHR");
	glad_glObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC)load("glObjectLabelKHR");
	glad_glGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC)load("glGetObjectLabelKHR");
	glad_glObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC)load("glObjectPtrLabelKHR");
	glad_glGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC)load("glGetObjectPtrLabelKHR");
	glad_glGetPointervKHR = (PFNGLGETPOINTERVKHRPROC)load("glGetPointervKHR");
}
static int find_extensionsGLES2(void) {
	if (!get_exts()) return 0;
	GLAD_GL_EXT_texture_filter_anisotropic = has_ext("GL_EXT_texture_filter_anisotropic");
	GLAD_GL_KHR_debug = has_ext("GL_KHR_debug");
	free_exts();
	return 1;
}

static void find_coreGLES2(void) {

    /* Thank you @elmindreda
     * https://github.com/elmindreda/greg/blob/master/templates/greg.c.in#L176
     * https://github.com/glfw/glfw/blob/master/src/context.c#L36
     */
    int i, major, minor;

    const char* version;
    const char* prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        NULL
    };

    version = (const char*) glGetString(GL_VERSION);
    if (!version) return;

    for (i = 0;  prefixes[i];  i++) {
        const size_t length = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], length) == 0) {
            version += length;
            break;
        }
    }

/* PR #18 */
#ifdef _MSC_VER
    sscanf_s(version, "%d.%d", &major, &minor);
#else
    sscanf(version, "%d.%d", &major, &minor);
#endif

    GLVersion.major = major; GLVersion.minor = minor;
    max_loaded_major = major; max_loaded_minor = minor;
	GLAD_GL_ES_VERSION_2_0 = (major == 2 && minor >= 0) || major > 2;
	GLAD_GL_ES_VERSION_3_0 = (major == 3 && minor >= 0) || major > 3;
	if (GLVersion.major > 3 || (GLVersion.major >= 3 && GLVersion.minor >= 0)) {
		max_loaded_major = 3;
		max_loaded_minor = 0;
	}
}

int gladLoadGLES2Loader(GLADloadproc load) {
	GLVersion.major = 0; GLVersion.minor = 0;
	glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
	if(glGetString == NULL) return 0;
	if(glGetString(GL_VERSION) == NULL) return 0;
	find_coreGLES2();
	load_GL_ES_VERSION_2_0(load);
	load_GL_ES_VERSION_3_0(load);

	if (!find_extensionsGLES2()) return 0;
	load_GL_KHR_debug(load);
	return GLVersion.major != 0 || GLVersion.minor != 0;
}

