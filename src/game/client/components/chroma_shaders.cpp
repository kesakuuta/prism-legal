#include "chroma_shaders.h"

#include <base/io.h>
#include <base/log.h>
#include <base/mem.h>
#include <GL/glew.h>
#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>
#include <engine/shared/json.h>
#include <game/client/gameclient.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

CChromaShaders::CChromaShaders() :
	m_ActiveShaderIndex(-1),
	m_ProgramId(0),
	m_ScreenTextureId(0),
	m_QuadVAO(0),
	m_QuadVBO(0),
	m_GLInitialized(false),
	m_CompileAttempted(false),
	m_CompileFailed(false),
	m_TexWidth(0),
	m_TexHeight(0),
	m_LocScreenTexture(-1),
	m_LocResolution(-1),
	m_LocPlayerPos(-1),
	m_LocTime(-1),
	m_LocLightRadius(-1),
	m_LocLightIntensity(-1),
	m_LocLightColor(-1),
	m_LocAmbientDark(-1),
	m_LocFogDensity(-1),
	m_LocDustIntensity(-1),
	m_LocVignettePower(-1)
{
}

CChromaShaders::~CChromaShaders()
{
	CleanupGLResources();
}

void CChromaShaders::OnInit()
{
	ScanShaders();
	m_ActiveShaderIndex = -1;
	if(!m_vShaders.empty())
	{
		for(size_t i = 0; i < m_vShaders.size(); ++i)
		{
			if(str_comp_nocase(m_vShaders[i].m_aName, g_Config.m_ChromaActiveShader) == 0)
			{
				m_ActiveShaderIndex = (int)i;
				break;
			}
		}
		if(m_ActiveShaderIndex < 0)
			m_ActiveShaderIndex = 0;
		str_copy(g_Config.m_ChromaActiveShader, m_vShaders[m_ActiveShaderIndex].m_aName);
	}
	m_CompileAttempted = false;
	m_CompileFailed = false;
}

void CChromaShaders::OnReset()
{
}

void CChromaShaders::CleanupGLResources()
{
	if(m_ProgramId)
	{
		glDeleteProgram(m_ProgramId);
		m_ProgramId = 0;
	}
	if(m_ScreenTextureId)
	{
		glDeleteTextures(1, &m_ScreenTextureId);
		m_ScreenTextureId = 0;
	}
	if(m_QuadVAO)
	{
		glDeleteVertexArrays(1, &m_QuadVAO);
		m_QuadVAO = 0;
	}
	if(m_QuadVBO)
	{
		glDeleteBuffers(1, &m_QuadVBO);
		m_QuadVBO = 0;
	}
	m_GLInitialized = false;
	m_TexWidth = 0;
	m_TexHeight = 0;
}

void CChromaShaders::InitGLResources()
{
	if(m_GLInitialized)
		return;

	// Create fullscreen quad VAO/VBO
	// Vertex layout: [pos.x, pos.y, uv.x, uv.y]
	const float QuadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_QuadVAO);
	glGenBuffers(1, &m_QuadVBO);

	glBindVertexArray(m_QuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate screen capture texture
	glGenTextures(1, &m_ScreenTextureId);
	glBindTexture(GL_TEXTURE_2D, m_ScreenTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_GLInitialized = true;
}

void CChromaShaders::ScanShaders()
{
	m_vShaders.clear();

	// Default embedded shaderpack: Atmospheric Amber Fog
	SShaderPack Amber;
	str_copy(Amber.m_aName, "atmospheric_amber");
	str_copy(Amber.m_aAuthor, "chroma.");
	str_copy(Amber.m_aDescription, "Cinematic volumetric lighting with warm dust motes and soft vignette");
	str_copy(Amber.m_aPath, "chroma/shaders/atmospheric_amber");
	str_copy(Amber.m_aVertPath, "chroma/shaders/atmospheric_amber/atmospheric_glow.vert");
	str_copy(Amber.m_aFragPath, "chroma/shaders/atmospheric_amber/atmospheric_glow.frag");
	Amber.m_LightRadius = 0.45f;
	Amber.m_LightIntensity = 1.35f;
	Amber.m_LightColor[0] = 1.0f;
	Amber.m_LightColor[1] = 0.78f;
	Amber.m_LightColor[2] = 0.45f;
	Amber.m_AmbientDark = 0.12f;
	Amber.m_FogDensity = 0.35f;
	Amber.m_DustIntensity = 0.65f;
	Amber.m_VignettePower = 0.85f;

	// Load JSON config if present
	IOHANDLE ConfigFile = Storage()->OpenFile("chroma/shaders/atmospheric_amber/shader.json", IOFLAG_READ, IStorage::TYPE_ALL);
	if(ConfigFile)
	{
		io_seek(ConfigFile, 0, EIoSeekOrigin::END);
		long Size = io_tell(ConfigFile);
		io_seek(ConfigFile, 0, EIoSeekOrigin::START);
		if(Size > 0)
		{
			std::vector<char> Buffer(Size + 1, 0);
			io_read(ConfigFile, Buffer.data(), Size);
			json_value *pJson = json_parse(Buffer.data(), Size);
			if(pJson && pJson->type == json_object)
			{
				const json_value &Defaults = (*pJson)["defaults"];
				if(Defaults.type == json_object)
				{
					if(Defaults["u_LightRadius"].type == json_double)
						Amber.m_LightRadius = (float)Defaults["u_LightRadius"].u.dbl;
					if(Defaults["u_LightIntensity"].type == json_double)
						Amber.m_LightIntensity = (float)Defaults["u_LightIntensity"].u.dbl;
					if(Defaults["u_AmbientDark"].type == json_double)
						Amber.m_AmbientDark = (float)Defaults["u_AmbientDark"].u.dbl;
					if(Defaults["u_FogDensity"].type == json_double)
						Amber.m_FogDensity = (float)Defaults["u_FogDensity"].u.dbl;
					if(Defaults["u_DustIntensity"].type == json_double)
						Amber.m_DustIntensity = (float)Defaults["u_DustIntensity"].u.dbl;
					if(Defaults["u_VignettePower"].type == json_double)
						Amber.m_VignettePower = (float)Defaults["u_VignettePower"].u.dbl;
				}
				json_value_free(pJson);
			}
		}
		io_close(ConfigFile);
	}

	m_vShaders.push_back(Amber);
}

const CChromaShaders::SShaderPack *CChromaShaders::ActiveShader() const
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		return &m_vShaders[m_ActiveShaderIndex];
	return nullptr;
}

void CChromaShaders::SelectShader(int Index)
{
	if(Index < 0 || Index >= (int)m_vShaders.size())
		return;

	m_ActiveShaderIndex = Index;
	str_copy(g_Config.m_ChromaActiveShader, m_vShaders[Index].m_aName);
	m_CompileAttempted = false;
	m_CompileFailed = false;
	CompileActiveShader();
}

void CChromaShaders::SelectShaderByName(const char *pName)
{
	for(size_t i = 0; i < m_vShaders.size(); ++i)
	{
		if(str_comp_nocase(m_vShaders[i].m_aName, pName) == 0)
		{
			SelectShader(i);
			return;
		}
	}
}

bool CChromaShaders::CompileActiveShader()
{
	m_CompileAttempted = true;
	const SShaderPack *pPack = ActiveShader();
	if(!pPack)
	{
		m_CompileFailed = true;
		return false;
	}

	if(glCreateShader == nullptr)
	{
		m_CompileFailed = true;
		log_warn("chroma_shaders", "OpenGL shaders not supported on this context/thread");
		return false;
	}

	if(m_ProgramId)
	{
		glDeleteProgram(m_ProgramId);
		m_ProgramId = 0;
	}

	IOHANDLE VertFile = Storage()->OpenFile(pPack->m_aVertPath, IOFLAG_READ, IStorage::TYPE_ALL);
	IOHANDLE FragFile = Storage()->OpenFile(pPack->m_aFragPath, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!VertFile || !FragFile)
	{
		if(VertFile) io_close(VertFile);
		if(FragFile) io_close(FragFile);
		log_warn("chroma_shaders", "Shader files '%s' or '%s' not found", pPack->m_aVertPath, pPack->m_aFragPath);
		return false;
	}

	io_seek(VertFile, 0, EIoSeekOrigin::END);
	long VertSize = io_tell(VertFile);
	io_seek(VertFile, 0, EIoSeekOrigin::START);
	std::vector<char> VertSrc(VertSize + 1, 0);
	io_read(VertFile, VertSrc.data(), VertSize);
	io_close(VertFile);

	io_seek(FragFile, 0, EIoSeekOrigin::END);
	long FragSize = io_tell(FragFile);
	io_seek(FragFile, 0, EIoSeekOrigin::START);
	std::vector<char> FragSrc(FragSize + 1, 0);
	io_read(FragFile, FragSrc.data(), FragSize);
	io_close(FragFile);

	// Compile Vertex Shader
	GLuint VertShader = glCreateShader(GL_VERTEX_SHADER);
	const char *pVertPtr = VertSrc.data();
	glShaderSource(VertShader, 1, &pVertPtr, nullptr);
	glCompileShader(VertShader);

	GLint Success = 0;
	glGetShaderiv(VertShader, GL_COMPILE_STATUS, &Success);
	if(!Success)
	{
		char aLog[1024] = {0};
		glGetShaderInfoLog(VertShader, sizeof(aLog), nullptr, aLog);
		log_error("chroma_shaders", "Vertex shader compilation failed for '%s':\n%s", pPack->m_aName, aLog);
		glDeleteShader(VertShader);
		m_CompileFailed = true;
		return false;
	}

	// Compile Fragment Shader
	GLuint FragShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *pFragPtr = FragSrc.data();
	glShaderSource(FragShader, 1, &pFragPtr, nullptr);
	glCompileShader(FragShader);

	glGetShaderiv(FragShader, GL_COMPILE_STATUS, &Success);
	if(!Success)
	{
		char aLog[1024] = {0};
		glGetShaderInfoLog(FragShader, sizeof(aLog), nullptr, aLog);
		log_error("chroma_shaders", "Fragment shader compilation failed for '%s':\n%s", pPack->m_aName, aLog);
		glDeleteShader(VertShader);
		glDeleteShader(FragShader);
		m_CompileFailed = true;
		return false;
	}

	// Link Program
	m_ProgramId = glCreateProgram();
	glAttachShader(m_ProgramId, VertShader);
	glAttachShader(m_ProgramId, FragShader);

	// Explicitly bind vertex attribute locations before linking (for GLSL 1.30 / legacy GL profile)
	glBindAttribLocation(m_ProgramId, 0, "inPosition");
	glBindAttribLocation(m_ProgramId, 1, "inTexCoord");

	glLinkProgram(m_ProgramId);

	glDeleteShader(VertShader);
	glDeleteShader(FragShader);

	glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &Success);
	if(!Success)
	{
		char aLog[1024] = {0};
		glGetProgramInfoLog(m_ProgramId, sizeof(aLog), nullptr, aLog);
		log_error("chroma_shaders", "Shader program linking failed for '%s':\n%s", pPack->m_aName, aLog);
		glDeleteProgram(m_ProgramId);
		m_ProgramId = 0;
		m_CompileFailed = true;
		return false;
	}

	// Cache uniform locations
	m_LocScreenTexture = glGetUniformLocation(m_ProgramId, "u_ScreenTexture");
	m_LocResolution = glGetUniformLocation(m_ProgramId, "u_Resolution");
	m_LocPlayerPos = glGetUniformLocation(m_ProgramId, "u_PlayerPos");
	m_LocTime = glGetUniformLocation(m_ProgramId, "u_Time");
	m_LocLightRadius = glGetUniformLocation(m_ProgramId, "u_LightRadius");
	m_LocLightIntensity = glGetUniformLocation(m_ProgramId, "u_LightIntensity");
	m_LocLightColor = glGetUniformLocation(m_ProgramId, "u_LightColor");
	m_LocAmbientDark = glGetUniformLocation(m_ProgramId, "u_AmbientDark");
	m_LocFogDensity = glGetUniformLocation(m_ProgramId, "u_FogDensity");
	m_LocDustIntensity = glGetUniformLocation(m_ProgramId, "u_DustIntensity");
	m_LocVignettePower = glGetUniformLocation(m_ProgramId, "u_VignettePower");

	log_info("chroma_shaders", "Successfully compiled and activated shaderpack '%s'", pPack->m_aName);
	return true;
}

void CChromaShaders::OnRender()
{
	if(!g_Config.m_ChromaShaders)
		return;

	// Only apply in-game or when viewing demo/game world
	if(Client()->State() == IClient::STATE_OFFLINE)
		return;

	if(m_CompileFailed)
		return;

	if(!m_GLInitialized)
		InitGLResources();

	if(!m_ProgramId)
	{
		if(m_CompileFailed || !CompileActiveShader())
			return;
	}

	const SShaderPack *pPack = ActiveShader();
	if(!pPack)
		return;

	int Width = Graphics()->ScreenWidth();
	int Height = Graphics()->ScreenHeight();
	if(Width <= 0 || Height <= 0)
		return;

	// Update screen texture size if window resized
	glBindTexture(GL_TEXTURE_2D, m_ScreenTextureId);
	if(m_TexWidth != Width || m_TexHeight != Height)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		m_TexWidth = Width;
		m_TexHeight = Height;
	}

	// Capture currently rendered world buffer into texture
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Width, Height);

	// Calculate normalized local player position [0, 1] on screen
	vec2 PlayerPosNorm(0.5f, 0.5f);
	if(GameClient()->m_Snap.m_pLocalCharacter)
	{
		// Target local Tee position in screen space
		vec2 TeePos = GameClient()->m_Controls.m_aTargetPos[0];
		PlayerPosNorm.x = std::clamp((TeePos.x / (float)Width) + 0.5f, 0.0f, 1.0f);
		PlayerPosNorm.y = std::clamp((TeePos.y / (float)Height) + 0.5f, 0.0f, 1.0f);
	}

	// Render Fullscreen Post-Processing Quad
	glUseProgram(m_ProgramId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ScreenTextureId);
	if(m_LocScreenTexture >= 0) glUniform1i(m_LocScreenTexture, 0);

	if(m_LocResolution >= 0) glUniform2f(m_LocResolution, (float)Width, (float)Height);
	if(m_LocPlayerPos >= 0) glUniform2f(m_LocPlayerPos, PlayerPosNorm.x, PlayerPosNorm.y);
	if(m_LocTime >= 0) glUniform1f(m_LocTime, (float)Client()->LocalTime());

	if(m_LocLightRadius >= 0) glUniform1f(m_LocLightRadius, pPack->m_LightRadius);
	if(m_LocLightIntensity >= 0) glUniform1f(m_LocLightIntensity, pPack->m_LightIntensity);
	if(m_LocLightColor >= 0) glUniform3f(m_LocLightColor, pPack->m_LightColor[0], pPack->m_LightColor[1], pPack->m_LightColor[2]);
	if(m_LocAmbientDark >= 0) glUniform1f(m_LocAmbientDark, pPack->m_AmbientDark);
	if(m_LocFogDensity >= 0) glUniform1f(m_LocFogDensity, pPack->m_FogDensity);
	if(m_LocDustIntensity >= 0) glUniform1f(m_LocDustIntensity, pPack->m_DustIntensity);
	if(m_LocVignettePower >= 0) glUniform1f(m_LocVignettePower, pPack->m_VignettePower);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(m_QuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

float CChromaShaders::GetLightRadius() const
{
	const SShaderPack *p = ActiveShader();
	return p ? p->m_LightRadius : 0.45f;
}

void CChromaShaders::SetLightRadius(float V)
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		m_vShaders[m_ActiveShaderIndex].m_LightRadius = std::clamp(V, 0.1f, 1.5f);
}

float CChromaShaders::GetLightIntensity() const
{
	const SShaderPack *p = ActiveShader();
	return p ? p->m_LightIntensity : 1.35f;
}

void CChromaShaders::SetLightIntensity(float V)
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		m_vShaders[m_ActiveShaderIndex].m_LightIntensity = std::clamp(V, 0.2f, 3.0f);
}

float CChromaShaders::GetFogDensity() const
{
	const SShaderPack *p = ActiveShader();
	return p ? p->m_FogDensity : 0.35f;
}

void CChromaShaders::SetFogDensity(float V)
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		m_vShaders[m_ActiveShaderIndex].m_FogDensity = std::clamp(V, 0.0f, 1.0f);
}

float CChromaShaders::GetDustIntensity() const
{
	const SShaderPack *p = ActiveShader();
	return p ? p->m_DustIntensity : 0.65f;
}

void CChromaShaders::SetDustIntensity(float V)
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		m_vShaders[m_ActiveShaderIndex].m_DustIntensity = std::clamp(V, 0.0f, 2.0f);
}

float CChromaShaders::GetVignettePower() const
{
	const SShaderPack *p = ActiveShader();
	return p ? p->m_VignettePower : 0.85f;
}

void CChromaShaders::SetVignettePower(float V)
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
		m_vShaders[m_ActiveShaderIndex].m_VignettePower = std::clamp(V, 0.0f, 2.0f);
}

void CChromaShaders::ResetToDefaults()
{
	if(m_ActiveShaderIndex >= 0 && m_ActiveShaderIndex < (int)m_vShaders.size())
	{
		m_vShaders[m_ActiveShaderIndex].m_LightRadius = 0.45f;
		m_vShaders[m_ActiveShaderIndex].m_LightIntensity = 1.35f;
		m_vShaders[m_ActiveShaderIndex].m_LightColor[0] = 1.0f;
		m_vShaders[m_ActiveShaderIndex].m_LightColor[1] = 0.78f;
		m_vShaders[m_ActiveShaderIndex].m_LightColor[2] = 0.45f;
		m_vShaders[m_ActiveShaderIndex].m_AmbientDark = 0.12f;
		m_vShaders[m_ActiveShaderIndex].m_FogDensity = 0.35f;
		m_vShaders[m_ActiveShaderIndex].m_DustIntensity = 0.65f;
		m_vShaders[m_ActiveShaderIndex].m_VignettePower = 0.85f;
	}
}
