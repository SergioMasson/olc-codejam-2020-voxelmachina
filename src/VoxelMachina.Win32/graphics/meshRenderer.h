#pragma once
#include "material.h"
#include "coreGraphics.h"
#include "../mathHelpers.h"

namespace graphics
{
	struct Vertex
	{
		Vertex() { ZeroMemory(this, sizeof(this)); }
		Vertex(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT2& uv)
			: Position(p), Normal(n), TangentU(t), TexUV(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: Position(px, py, pz), Normal(nx, ny, nz),
			TangentU(tx, ty, tz), TexUV(u, v) {}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 TangentU;
		DirectX::XMFLOAT2 TexUV;
	};

	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<UINT> Indices;

		///<summary>
		/// Creates a box centered at the origin with the given dimensions.
		///</summary>
		static void CreateBox(float width, float height, float depth, MeshData& meshData);

		///<summary>
		/// Creates a sphere centered at the origin with the given radius.  The
		/// slices and stacks parameters control the degree of tessellation.
		///</summary>
		static void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);

		///<summary>
		/// Creates a cylinder parallel to the y-axis, and centered about the origin.
		/// The bottom and top radius can vary to form various cone shapes rather than true
		// cylinders.  The slices and stacks parameters control the degree of tessellation.
		///</summary>
		static void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);

		///<summary>
		/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
		/// at the origin with the specified width and depth.
		///</summary>
		static void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);

		static void LoadFromOBJFile(const wchar_t* filename, MeshData& meshData);
	};

	class MeshRenderer
	{
	public:
		MeshRenderer(MeshData data, Material material, math::Vector3 position, math::Quaternion rotation, math::Vector3 scale = { 1, 1, 1 });
		MeshRenderer() = default;
		~MeshRenderer() = default;

		void SetRotation(math::Quaternion basisRotation)
		{
			m_transform.SetRotation(math::Quaternion(DirectX::XMQuaternionNormalize(basisRotation)));
			m_worldMatrix = m_transform;
		}

		const math::Quaternion GetRotation() const { return m_transform.GetRotation(); }

		void SetPosition(math::Vector3 worldPos)
		{
			m_transform.SetTranslation(worldPos);
			m_worldMatrix = m_transform;
		}

		const math::Vector3 GetPosition() const { return m_transform.GetTranslation(); }

		void SetAlbedoTexture(Texture2D* texture)
		{
			m_albedoTexture = texture;
		}

		void SetNormalMap(Texture2D* normalMap)
		{
			m_normalMap = normalMap;
		}

		void SetEmissionMap(Texture2D* emissionMap)
		{
			m_emissionTexture = emissionMap;
		}

		void SetTextureDisplacement(float x, float y)
		{
			m_textureDisplacement = { x, y };
		}

		void SetTextureScale(float x, float y)
		{
			m_textureScale = { x, y };
		}

		void SetEmission(DirectX::XMFLOAT4 emission)
		{
			m_material.Emission = emission;
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;

		Material m_material;
		MeshData m_meshData;

		math::Transform m_transform;
		math::Matrix4 m_worldMatrix;

		Texture2D* m_albedoTexture;
		Texture2D* m_normalMap;
		Texture2D* m_emissionTexture;

		DirectX::XMFLOAT2 m_textureScale{ 1, 1 };
		DirectX::XMFLOAT2 m_textureDisplacement{ 0, 0 };

		friend RenderPipeline;
	};
}