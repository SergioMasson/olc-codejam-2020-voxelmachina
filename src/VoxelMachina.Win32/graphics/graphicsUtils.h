#pragma once
namespace graphics
{
	template<class T>
	static void BuildConstantBuffer(ID3D11Buffer** constantBuffer)
	{
		// constant colour buffer. TODO(Replace this with all camera and light buffers.)
		D3D11_BUFFER_DESC bd = { 0 };
		bd.ByteWidth = sizeof(T);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		auto hr = graphics::g_d3dDevice->CreateBuffer(&bd, nullptr, constantBuffer);

		ASSERT_SUCCEEDED(hr, "Fail to create const buffer");
	}
}