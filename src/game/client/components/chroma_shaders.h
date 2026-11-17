#ifndef GAME_CLIENT_COMPONENTS_CHROMA_SHADERS_H
#define GAME_CLIENT_COMPONENTS_CHROMA_SHADERS_H

#include <game/client/component.h>
#include <vector>
#include <string>

class CChromaShaders : public CComponent
{
public:
	struct SShaderPack
	{
		char m_aName[64];
		char m_aAuthor[64];
		char m_aDescription[128];
		char m_aPath[128];
		char m_aVertPath[128];
		char m_aFragPath[128];

		// Parameters
		float m_LightRadius;
		float m_LightIntensity;
		float m_LightColor[3];
		float m_AmbientDark;
		float m_FogDensity;
		float m_DustIntensity;
		float m_VignettePower;
	};

	CChromaShaders();
	virtual ~CChromaShaders();

	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual void OnReset() override;

	void ScanShaders();
	void SelectShader(int Index);
	void SelectShaderByName(const char *pName);

	const std::vector<SShaderPack> &ShaderList() const { return m_vShaders; }
	int ActiveShaderIndex() const { return m_ActiveShaderIndex; }
	const SShaderPack *ActiveShader() const;

	// Runtime parameter controls
	float GetLightRadius() const;
	void SetLightRadius(float V);

	float GetLightIntensity() const;
	void SetLightIntensity(float V);

	float GetFogDensity() const;
	void SetFogDensity(float V);

	float GetDustIntensity() const;
	void SetDustIntensity(float V);

	float GetVignettePower() const;
	void SetVignettePower(float V);

	void ResetToDefaults();

private:
	std::vector<SShaderPack> m_vShaders;
	int m_ActiveShaderIndex;

	unsigned int m_ProgramId;
	unsigned int m_ScreenTextureId;
	unsigned int m_QuadVAO;
	unsigned int m_QuadVBO;
	bool m_GLInitialized;
	bool m_CompileAttempted;
	bool m_CompileFailed;
	int m_TexWidth;
	int m_TexHeight;

	// Uniform locations
	int m_LocScreenTexture;
	int m_LocResolution;
	int m_LocPlayerPos;
	int m_LocTime;
	int m_LocLightRadius;
	int m_LocLightIntensity;
	int m_LocLightColor;
	int m_LocAmbientDark;
	int m_LocFogDensity;
	int m_LocDustIntensity;
	int m_LocVignettePower;

	bool CompileActiveShader();
	void InitGLResources();
	void CleanupGLResources();
};

#endif // GAME_CLIENT_COMPONENTS_CHROMA_SHADERS_H
