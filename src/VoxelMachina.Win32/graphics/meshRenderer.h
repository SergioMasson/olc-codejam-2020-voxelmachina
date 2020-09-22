#pragma once
#include "material.h"
#include "coreGraphics.h"
#include "../mathHelpers.h"
#include "../math/boundingSphere.h"
#include "../math/boudingBox.h"

class GameObject;

namespace graphics
{
	class MeshRenderer;

	extern std::set<MeshRenderer*> g_activeMeshRenderers;

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

		math::BoudingBox BoudingBox;

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
		MeshRenderer(GameObject* gameObject, MeshData* data, Material material);
		~MeshRenderer();

		void SetMaterial(Material material) { m_material = material; }
		void SetMesh(MeshData* data) { m_meshData = data; };

		inline math::BoudingBox WBoudingBox() const
		{
			return GetWorldMatrix() * m_meshData->BoudingBox;
		}

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

		math::Matrix4 GetWorldMatrix() const;

		const math::Vector3 GetRightVec() const
		{
			return math::Matrix3{ GetWorldMatrix() }.GetX();
		}
		const math::Vector3 GetUpVec() const
		{
			return math::Matrix3{ GetWorldMatrix() }.GetY();
		}
		const math::Vector3 GetForwardVec() const
		{
			return -math::Matrix3{ GetWorldMatrix() }.GetZ();
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;

		Material m_material;
		MeshData* m_meshData;

		Texture2D* m_albedoTexture;
		Texture2D* m_normalMap;
		Texture2D* m_emissionTexture;

		DirectX::XMFLOAT2 m_textureScale{ 1, 1 };
		DirectX::XMFLOAT2 m_textureDisplacement{ 0, 0 };

		GameObject* m_gameObject;

		friend RenderPipeline;
	};
}