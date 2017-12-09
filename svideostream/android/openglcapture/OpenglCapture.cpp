#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include "OpenglCapture.h"
#include "GraphicBuffer.h"

#include"Log.h"
#include "GlShader.h"


const GLchar COpenglCapture::g_pVertSrc[] =
"attribute vec2 position;\n"
"attribute vec2 textureCoordinate;\n"
"varying vec2 coordinate;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(position, 0.0, 1.0);\n"
"   coordinate = textureCoordinate;\n"
"}\n";


const GLchar COpenglCapture::g_pFragSrc[] =
"precision mediump float;\n"
"varying vec2 coordinate;\n"
"uniform sampler2D texture;\n"
"void main()\n"
"{\n"
"   gl_FragColor = texture2D(texture, coordinate);\n"
"}\n";

void COpenglCapture::OnCapture(const uint8_t* pdata, int64_t pts)
{
	/*if (auto cb = m_pDataCallback.lock())
	{
	cb->OnCapture(pdata, m_nSize, m_nStride, pts);
	}*/
	if (m_pDataCallback != nullptr)
	{
		m_pDataCallback->OnCapture(pdata, m_nSize, m_nStride, pts);
	}
}

bool COpenglCapture::InitEGLImage()
{
	m_pGraphicBuffer.reset(new GraphicBuffer(m_nWidth, m_nHeight,
		PIXEL_FORMAT_RGBA_8888, GraphicBuffer::USAGE_SW_READ_OFTEN |
		GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_HW_TEXTURE));
	if (m_pGraphicBuffer == nullptr)
	{
		return false;
	}
	LOGD << "new GraphicBuffer ok";
	//delete tmp;
	//return false;
	struct ANativeWindowBuffer *pbuffer = m_pGraphicBuffer->getNativeBuffer();
	// Create the EGLImageKHR from the native buffer
	EGLint eglImgAttrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };

	m_eglimgkhr = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, (EGLClientBuffer)pbuffer, eglImgAttrs);
	EGLint nEGLError = eglGetError();
	if (nEGLError != EGL_SUCCESS) {
		switch (nEGLError)
		{
		case EGL_NOT_INITIALIZED:
			LOGE<<"eglCreateImageKHR: EGL_NOT_INITIALIZED!";
			break;
		case EGL_BAD_ACCESS:
			LOGE<<"eglCreateImageKHR: EGL_BAD_ACCESS!";
			break;
		case EGL_BAD_ALLOC:
			LOGE<<"eglCreateImageKHR: EGL_BAD_ALLOC!";
			break;
		case EGL_BAD_CONFIG:
			LOGE << "eglCreateImageKHR: EGL_BAD_CONFIG!";
			break;
		case EGL_BAD_CONTEXT:
			LOGE << "eglCreateImageKHR: EGL_BAD_CONTEXT!";
			break;
		case EGL_BAD_CURRENT_SURFACE:
			LOGE << "eglCreateImageKHR: EGL_BAD_CURRENT_SURFACE!";
			break;
		case EGL_BAD_DISPLAY:
			LOGE << "eglCreateImageKHR: EGL_BAD_DISPLAY!";
			break;
		case EGL_BAD_MATCH:
			LOGE << "eglCreateImageKHR: EGL_BAD_MATCH!";
			break;
		case EGL_BAD_NATIVE_PIXMAP:
			LOGE << "eglCreateImageKHR: EGL_BAD_NATIVE_PIXMAP!";
			break;
		case EGL_BAD_NATIVE_WINDOW:
			LOGE << "eglCreateImageKHR: EGL_BAD_NATIVE_WINDOW!";
			break;
		case EGL_BAD_PARAMETER:
			LOGE << "eglCreateImageKHR: EGL_BAD_PARAMETER!";
			break;
		case EGL_BAD_SURFACE:
			LOGE << "eglCreateImageKHR: EGL_BAD_SURFACE!";
			break;
		case EGL_CONTEXT_LOST:
			LOGE << "eglCreateImageKHR: EGL_CONTEXT_LOST!";
			break;
		default:
			LOGE << "eglCreateImageKHR: Unknown "<< nEGLError;
			break;
		}
		return false;
	}
	if (m_eglimgkhr == EGL_NO_IMAGE_KHR) {
		LOGE<<"eglCreateImageKHR returned no image.";
		return false;
	}
	LOGI << "eglCreateImageKHR ok";
	//gen frame buffer
	glGenFramebuffers(1, &m_nFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBufferId);
	if (!OpenglEsCheckError("glGenFramebuffers"))
	{
		LOGE << "glGenFramebuffers or glBindFramebuffer failed";
		UninitEGLImage();
		return false;
	}
	LOGI << "nFramebufferID= " << m_nFrameBufferId;
	//texture 
	glGenTextures(1, &m_nTextureId);
	if (!OpenglEsCheckError("glGenTextures"))
	{
		UninitEGLImage();
		return false;
	}
	LOGI << "nTextureID= "<< m_nTextureId;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_nTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nWidth, m_nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Attach the EGLImage to whatever texture is bound to GL_TEXTURE_2D
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_eglimgkhr);
	if (!OpenglEsCheckError("glEGLImageTargetTexture2DOES"))
	{
		UninitEGLImage();
		return false;
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nTextureId, 0);
	m_nStride = m_pGraphicBuffer->getStride();
	m_nStride *= 4;
	LOGI << "m_nStride = "<< m_nStride<<" width = "<<m_pGraphicBuffer->getWidth()<<" height = "<<m_pGraphicBuffer->getHeight();
	return true;
}

void COpenglCapture::UninitEGLImage()
{
	LOGI << "UninitEGLImage start";
	glDeleteTextures(1, &m_nTextureId);
	m_nTextureId = -1;
	glDeleteFramebuffers(1, &m_nFrameBufferId);
	m_nFrameBufferId = -1;
	eglDestroyImageKHR(eglGetCurrentDisplay(), m_eglimgkhr);
	m_eglimgkhr = EGL_NO_IMAGE_KHR;
	LOGI << "delete GraphicBuffer";

	m_pGraphicBuffer.reset(nullptr);
	LOGI << "UninitEGLImage end";
}


bool COpenglCapture::InitGl()
{
	LOGI << "InitGl start";

	m_cGlShader.reset(new CGlShader(g_pVertSrc, g_pFragSrc));
	m_TextureLoc = m_cGlShader->GetUniformLocation("texture");
	LOGI << "InitGl end";

	return true;
}


void COpenglCapture::UninitGl()
{
	LOGI << "UninitGl start";

	m_cGlShader->Release();
	m_cGlShader.reset(nullptr);
	LOGI << "UninitGl end";

}

bool COpenglCapture::OpenglEsCheckError(const std::string& msg)
{
	do 
	{ 
	int nErrno = glGetError(); 
	if (nErrno != GL_NO_ERROR) {
		
		LOGE << msg<< " errno="<< nErrno;
			return false; 
	} 
	} while (0);
	return true;
}

bool COpenglCapture::InitCapture(int width, int height)
{
	LOGI << "InitCapture start width = "<<width << " height = "<<height;
	m_nHeight = height;
	m_nWidth = width;
	if (m_nHeight <= 0 || m_nWidth <= 0)
	{
		m_bInit = false;
		return false;
	}
	LOGI << "InitCapture start width = " << m_nWidth << " height = " << m_nHeight;

	// backup original framebufferid
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&m_nOriginalFramebufferId);
	if (!InitEGLImage())
	{
		m_bInit = false;
		return false;
	}
	if (!InitGl())
	{
		UninitEGLImage();
		m_bInit = false;
		return false;
	}
	m_bInit = true;
	m_nSize = m_nStride * height;
	m_pCapturedData.reset(new uint8_t[m_nSize]);

	//  restore framebufferid
	glBindFramebuffer(GL_FRAMEBUFFER, m_nOriginalFramebufferId);
	LOGI << "InitCapture success";
	return true;
}

void COpenglCapture::CaptureFrame(int textureid)
{
	LOGI << "1 CaptureFrame textureid = "<< textureid;
	if (textureid == -1 || !m_bInit)
	{
		LOGW << "textureid is invalid   || !m_bInit";
		return;
	}
	// backup original framebufferid
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&m_nOriginalFramebufferId);

	//bindframebuffer
	LOGD << "2 bindframebuffer";
	glBindFramebuffer(GL_FRAMEBUFFER, m_nFrameBufferId);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//UseProgram
	LOGD << "3 UseProgram";

	m_cGlShader->UseProgram();
	float vertices[] = { -1, -1, -1, 1, 1, -1, 1, 1 };
	float texs[] = { 0, 1, 0, 0, 1, 1, 1, 0 };
	LOGD << "4 SetVertexAttribArray";

	m_cGlShader->SetVertexAttribArray("position", 2, vertices);
	m_cGlShader->SetVertexAttribArray("textureCoordinate", 2, texs);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureid);
	glUniform1i(m_TextureLoc, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	LOGD << "5 SetVertexAttribArray";
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	LOGI << "6 pre lock";
	//uint8_t *tmpdata = m_pCapturedData.get();
	uint8_t *tmpdata = nullptr;
	LOGD << "7 pre lock";
	status_t retstatus = m_pGraphicBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, (void**)&tmpdata);
	LOGD << "8 lock retstatus = "<< retstatus;
	if (tmpdata != nullptr)
	{
		memcpy(m_pCapturedData.get(), tmpdata, m_nSize);
	}
	m_pGraphicBuffer->unlock();
	LOGD << "9 unlock";

	OnCapture(m_pCapturedData.get(), 0);

	//  restore framebufferid
	glBindFramebuffer(GL_FRAMEBUFFER, m_nOriginalFramebufferId);
}

void COpenglCapture::Destroy()
{
	if (!m_bInit)
	{
		LOGD << "Destroy hasn't inited";
		return;
	}
	LOGD << "COpenglCapture Destroy";
	UninitEGLImage();
	UninitGl();
	m_pCapturedData.reset(nullptr);
	m_nStride = m_nHeight = m_nSize = m_nWidth = 0;
	m_bInit = false;

}
