#pragma once
#include <memory>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include<string>
#include "GraphicBuffer.h"
#include "GlShader.h"
class COpenglCapture
{
public:
	class IFrameAvaliable
	{
	public:
		virtual void OnCapture(const uint8_t* pdata, int nsize, int stride, int64_t pts) = 0;
	};
private:
	static const GLchar g_pVertSrc[];
	static const GLchar g_pFragSrc[];
	int m_nWidth = 0;
	int m_nHeight = 0;
	std::unique_ptr<uint8_t> m_pCapturedData;
	int m_nSize = 0;

	//opengl about
	std::unique_ptr<GraphicBuffer> m_pGraphicBuffer;
	EGLImageKHR m_eglimgkhr;
	GLuint m_nFrameBufferId;
	GLuint m_nTextureId;
	GLuint m_nOriginalFramebufferId;
	std::unique_ptr<CGlShader> m_cGlShader;
	GLint m_TextureLoc;
	int m_nStride = 0;
	bool m_bInit = false;

	//std::weak_ptr<IFrameAvaliable> m_pDataCallback;
	IFrameAvaliable* m_pDataCallback;
	void OnCapture(const uint8_t* pdata, int64_t pts);

	bool InitEGLImage();
	void UninitEGLImage();

	bool InitGl();
	void UninitGl();

	bool OpenglEsCheckError(const std::string& msg);
public:
	bool InitCapture(int width, int height);
	void CaptureFrame(int textureid);
	void Destroy();

	//void SetFrameCallback(std::weak_ptr<IFrameAvaliable> callback)
	void SetFrameCallback(IFrameAvaliable* callback)
	{
		m_pDataCallback = callback;
	}

};


