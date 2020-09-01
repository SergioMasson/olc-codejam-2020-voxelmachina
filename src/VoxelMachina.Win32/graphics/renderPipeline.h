#pragma once

#include "lights.h"
#include "../camera.h"
#include "meshrenderer.h"

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
		void StartRender(Camera* camera, DirectionalLight lights, SpotLight spotLights, PointLight pointLights);
		void RenderMesh(MeshRenderer const& mesh);

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_sceneConstBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_objectConstBuffer = nullptr;

		Camera* m_camera;
	};
}