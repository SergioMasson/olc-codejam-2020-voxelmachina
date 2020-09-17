#pragma once

#include "lights.h"
#include "../camera.h"
#include "meshrenderer.h"
#include "texture2D.h"

class Camera;

namespace graphics
{
	class MeshRenderer;

	class RenderPipeline
	{
	public:
		RenderPipeline();
		~RenderPipeline();
		void LoadShader(const BYTE* vertexShader, SIZE_T vertexShaderSize, const BYTE* pixelShader, SIZE_T pixelShaderSize);
		void StartRender(Camera* camera);
		void RenderMesh(MeshRenderer const& mesh);
		void SetSkyboxTexture(Texture2D* skybox) { m_skyboxTexture = skybox; };
		void AddLight(Light* light);

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_sceneConstBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_objectConstBuffer = nullptr;

		//Skybox
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skyboxVertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skyboxPixelShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyboxInputLayout = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_skyboxConstBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_skyboxRasterizerState = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_skyboxDepthStencilState = nullptr;

		MeshRenderer m_skyboxMeshRenerer;
		Texture2D* m_skyboxTexture;

		Camera* m_camera;
		std::vector<Light*> m_sceneLights;

	private:
		void CreateSkybox();
		void RenderSkybox(Camera* camera);
	};
}