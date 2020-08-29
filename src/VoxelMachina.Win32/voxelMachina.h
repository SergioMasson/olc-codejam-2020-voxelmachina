#pragma once

#include "application.h"
#include "mathHelpers.h"

class VoxelMachinaApp : public IGameApp
{
public:
	virtual void Startup(void) override;

	virtual void Cleanup(void) override;

	virtual bool IsDone(void) override;

	virtual void Update(float deltaT) override;

	virtual void RenderScene(void) override;

	virtual void RenderUI(void) override;

private:

	//Render pipeline.
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constBuffer = nullptr;

	DirectX::XMFLOAT4X4 m_WorldMatrix = math::Identity4x4();
	DirectX::XMFLOAT4X4 m_ViewMatrix = math::Identity4x4();
	DirectX::XMFLOAT4X4 m_ProjMatrix = math::Identity4x4();

	float mTheta = 1.5f * DirectX::XM_PI;
	float mPhi = DirectX::XM_PIDIV4;
	float mRadius = 5.0f;
};