#include "pch.h"
#include "meshRenderer.h"
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "../hash.h"
#include "../gameObject.h"

using namespace std;
using namespace DirectX;
using namespace graphics;

std::set<MeshRenderer*> graphics::g_activeMeshRenderers;

std::map<size_t, Microsoft::WRL::ComPtr<ID3D11Buffer>> g_vertexBufferTable{};
std::map<size_t, Microsoft::WRL::ComPtr<ID3D11Buffer>> g_indexBufferTable{};

void Subdivide(MeshData& meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;

	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	UINT numTris = static_cast<UINT>(inputCopy.Indices.size()) / 3;

	for (UINT i = 0; i < numTris; ++i)
	{
		Vertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
		Vertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
		Vertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		Vertex m0, m1, m2;

		// For subdivision, we just care about the position component.  We derive the other
		// vertex components in CreateGeosphere.

		m0.Position = XMFLOAT3(
			0.5f * (v0.Position.x + v1.Position.x),
			0.5f * (v0.Position.y + v1.Position.y),
			0.5f * (v0.Position.z + v1.Position.z));

		m1.Position = XMFLOAT3(
			0.5f * (v1.Position.x + v2.Position.x),
			0.5f * (v1.Position.y + v2.Position.y),
			0.5f * (v1.Position.z + v2.Position.z));

		m2.Position = XMFLOAT3(
			0.5f * (v0.Position.x + v2.Position.x),
			0.5f * (v0.Position.y + v2.Position.y),
			0.5f * (v0.Position.z + v2.Position.z));

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices.push_back(i * 6 + 0);
		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 5);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 2);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 1);
		meshData.Indices.push_back(i * 6 + 4);
	}
}

void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	UINT baseIndex = (UINT)meshData.Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	//
	// Build bottom cap.
	//
	UINT baseIndex = (UINT)meshData.Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}

void graphics::MeshData::CreateBox(float width, float height, float depth, MeshData& meshData)
{
	meshData.BoudingBox = math::BoudingBox(math::Vector3(-width * 0.5f, -height * 0.5f, -depth * 0.5f), math::Vector3(width * 0.5f, height * 0.5f, depth * 0.5f));

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	// Create the indices.

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);
}

void graphics::MeshData::CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	meshData.BoudingBox = math::BoudingBox(math::Vector3(-radius, -radius, -radius), math::Vector3(radius, radius, radius));

	meshData.Vertices.clear();
	meshData.Indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	float phiStep = XM_PI / stackCount;
	float thetaStep = 2.0f * XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			Vertex v;

			// spherical to cartesian
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR p = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

			v.TexUV.x = theta / XM_2PI;
			v.TexUV.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices.push_back(0);
		meshData.Indices.push_back(i + 1);
		meshData.Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	UINT southPoleIndex = (UINT)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(southPoleIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}

void graphics::MeshData::CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
	float radius = 0.0f;

	if (bottomRadius > topRadius)
		radius = bottomRadius;
	else
		radius = topRadius;

	meshData.BoudingBox = math::BoudingBox(math::Vector3(radius, height * 0.5f, radius), math::Vector3(-radius, -height * 0.5f, -radius));

	meshData.Vertices.clear();
	meshData.Indices.clear();

	//
	// Build Stacks.
	//

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	UINT ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * XM_PI / sliceCount;
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			Vertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = XMFLOAT3(r * c, y, r * s);

			vertex.TexUV.x = (float)j / sliceCount;
			vertex.TexUV.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			//
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	UINT ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(i * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.Indices.push_back(i * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
}

void graphics::MeshData::CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData)
{
	UINT vertexCount = m * n;
	UINT faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float radius = sqrtf(halfWidth * halfWidth + halfDepth * halfDepth);

	meshData.BoudingBox = math::BoudingBox(math::Vector3(width * 0.5f, 0, depth * 0.5f), math::Vector3(-width * 0.5f, 0, -depth * 0.5f));

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * n + j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i * n + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * n + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * n + j].TexUV.x = j * du;
			meshData.Vertices[i * n + j].TexUV.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			meshData.Indices[k] = i * n + j;
			meshData.Indices[k + 1] = i * n + j + 1;
			meshData.Indices[k + 2] = (i + 1) * n + j;

			meshData.Indices[k + 3] = (i + 1) * n + j;
			meshData.Indices[k + 4] = i * n + j + 1;
			meshData.Indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
}

void graphics::MeshData::LoadFromOBJFile(const wchar_t* filename, MeshData& meshData)
{
	std::vector<uint32_t> vertexIndices, uvIndices, normalIndices;
	std::vector< DirectX::XMFLOAT3> temp_vertices;
	std::vector< DirectX::XMFLOAT2 > temp_uvs;
	std::vector< DirectX::XMFLOAT3> temp_normals;

	std::vector<uint32_t> indexes;
	std::vector<Vertex> vertex;

	std::string line;
	std::ifstream in(filename);

	float maxX = 0.0f;
	float minX = 0.0f;

	float maxY = 0.0f;
	float minY = 0.0f;

	float maxZ = 0.0f;
	float minZ = 0.0f;

	while (std::getline(in, line))                           // read whole line
	{
		if (line.find_first_of("vVfF") == std::string::npos) continue;     // skip pointless lines

		auto header = line.substr(0, 2);
		auto body = line.substr(2, line.size());

		std::istringstream ss(body);                         // put line into a stream for input

		if (header == "V " || header == "v ")
		{
			float x, y, z;
			ss >> x >> y >> z;
			temp_vertices.push_back({ x, y, z });

			Vertex vert{};
			vert.Position = { x, y, z };
			vert.Normal = { 999, 0, 0 };
			vertex.push_back(vert);

			if (x > maxX)
				maxX = x;

			if (x < minX)
				minX = x;

			if (y > maxY)
				maxY = y;

			if (y < minY)
				minY = y;

			if (z > maxZ)
				maxZ = z;

			if (z < minZ)
				minZ = z;
		}
		else if (header == "VN" || header == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			temp_normals.push_back({ x, y, z });
		}
		else if (header == "VT" || header == "vt")
		{
			float x, y;
			ss >> x >> y;
			temp_uvs.push_back({ x, y });
		}
		else if (header == "F " || header == "f ")
		{
			auto space0 = body.find(' ', 1);
			auto space1 = body.find(' ', space0 + 1);

			auto vertex0 = body.substr(0, space0);
			auto vertex1 = body.substr(space0, space1 - space0);
			auto vertex2 = body.substr(space1, body.size());

			auto tab0 = vertex0.find('/', 0);
			auto tab1 = vertex0.find('/', tab0 + 1);

			auto vertexIndex0 = std::stoi(vertex0.substr(0, tab0)) - 1;
			auto textureInde0 = std::stoi(vertex0.substr(tab0 + 1, tab1 - tab0)) - 1;
			auto normalIndex0 = std::stoi(vertex0.substr(tab1 + 1, body.size())) - 1;

			tab0 = vertex1.find('/', 0);
			tab1 = vertex1.find('/', tab0 + 1);

			auto vertexIndex1 = std::stoi(vertex1.substr(0, tab0)) - 1;
			auto textureInde1 = std::stoi(vertex1.substr(tab0 + 1, tab1 - tab0)) - 1;
			auto normalIndex1 = std::stoi(vertex1.substr(tab1 + 1, body.size())) - 1;

			tab0 = vertex2.find('/', 0);
			tab1 = vertex2.find('/', tab0 + 1);

			auto vertexIndex2 = std::stoi(vertex2.substr(0, tab0)) - 1;
			auto textureInde2 = std::stoi(vertex2.substr(tab0 + 1, tab1 - tab0)) - 1;
			auto normalIndex2 = std::stoi(vertex2.substr(tab1 + 1, body.size())) - 1;

			//this vertex has already been assigned.
			if (vertex[vertexIndex0].Normal.x != 999)
			{
				Vertex newVertex{};
				newVertex.Position = vertex[vertexIndex0].Position;
				newVertex.Normal = temp_normals[normalIndex0];
				newVertex.TexUV = temp_uvs[textureInde0];

				math::Vector3 pos1 = vertex[vertexIndex0].Position;
				math::Vector3 pos2 = vertex[vertexIndex1].Position;
				math::Vector3 normal = vertex[vertexIndex0].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&newVertex.TangentU, tanget);

				vertex.push_back(newVertex);
				indexes.push_back(static_cast<UINT>(vertex.size()) - 1);
			}
			else
			{
				vertex[vertexIndex0].Normal = temp_normals[normalIndex0];
				vertex[vertexIndex0].TexUV = temp_uvs[textureInde0];

				math::Vector3 pos1 = vertex[vertexIndex0].Position;
				math::Vector3 pos2 = vertex[vertexIndex1].Position;
				math::Vector3 normal = vertex[vertexIndex0].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&vertex[vertexIndex0].TangentU, tanget);

				indexes.push_back(vertexIndex0);
			}

			//this vertex has already been assigned.
			if (vertex[vertexIndex1].Normal.x != 999)
			{
				Vertex newVertex{};
				newVertex.Position = vertex[vertexIndex1].Position;
				newVertex.Normal = temp_normals[normalIndex1];
				newVertex.TexUV = temp_uvs[textureInde1];

				math::Vector3 pos1 = vertex[vertexIndex1].Position;
				math::Vector3 pos2 = vertex[vertexIndex2].Position;
				math::Vector3 normal = vertex[vertexIndex1].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&newVertex.TangentU, tanget);

				vertex.push_back(newVertex);
				indexes.push_back(static_cast<UINT>(vertex.size()) - 1);
			}
			else
			{
				vertex[vertexIndex1].Normal = temp_normals[normalIndex1];
				vertex[vertexIndex1].TexUV = temp_uvs[textureInde1];

				math::Vector3 pos1 = vertex[vertexIndex1].Position;
				math::Vector3 pos2 = vertex[vertexIndex2].Position;
				math::Vector3 normal = vertex[vertexIndex1].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&vertex[vertexIndex1].TangentU, tanget);

				indexes.push_back(vertexIndex1);
			}

			//this vertex has already been assigned.
			if (vertex[vertexIndex2].Normal.x != 999)
			{
				Vertex newVertex{};
				newVertex.Position = vertex[vertexIndex2].Position;
				newVertex.Normal = temp_normals[normalIndex2];
				newVertex.TexUV = temp_uvs[textureInde2];

				math::Vector3 pos1 = vertex[vertexIndex2].Position;
				math::Vector3 pos2 = vertex[vertexIndex0].Position;
				math::Vector3 normal = vertex[vertexIndex2].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&newVertex.TangentU, tanget);

				vertex.push_back(newVertex);
				indexes.push_back(static_cast<UINT>(vertex.size()) - 1);
			}
			else
			{
				vertex[vertexIndex2].Normal = temp_normals[normalIndex2];
				vertex[vertexIndex2].TexUV = temp_uvs[textureInde2];

				math::Vector3 pos1 = vertex[vertexIndex2].Position;
				math::Vector3 pos2 = vertex[vertexIndex0].Position;
				math::Vector3 normal = vertex[vertexIndex2].Normal;

				math::Vector3 tanget = math::Normalize(math::Cross(normal, (pos1 - pos2)));
				DirectX::XMStoreFloat3(&vertex[vertexIndex2].TangentU, tanget);

				indexes.push_back(vertexIndex2);
			}
		}
	}

	meshData.Indices = indexes;
	meshData.Vertices = vertex;
	meshData.BoudingBox = math::BoudingBox(math::Vector3{ maxX, maxY, maxZ }, math::Vector3{ minX, minY, minZ });

	in.close();
}

graphics::MeshRenderer::MeshRenderer(GameObject* gameObject, MeshData* data, Material material) :
	m_gameObject{ gameObject },
	m_meshData{ data },
	m_material{ material }
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(m_meshData->Vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = m_meshData->Vertices.data();

	auto resourceHash = HashState(&vinitData);

	auto VertIter = g_vertexBufferTable.find(resourceHash);

	//Check if the mesh data is already on the GPU.
	if (VertIter != g_vertexBufferTable.end())
	{
		m_vertexBuffer = g_vertexBufferTable[resourceHash];
	}
	else
	{
		auto hr = graphics::g_d3dDevice->CreateBuffer(&vbd, &vinitData, m_vertexBuffer.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create vertex buffer.");
		g_vertexBufferTable.insert({ resourceHash,  m_vertexBuffer });
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(m_meshData->Indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = m_meshData->Indices.data();

	resourceHash = HashState(&iinitData);

	auto IndexIter = g_indexBufferTable.find(resourceHash);

	if (IndexIter != g_indexBufferTable.end())
	{
		m_indexBuffer = g_indexBufferTable[resourceHash];
	}
	else
	{
		auto hr = graphics::g_d3dDevice->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf());
		ASSERT_SUCCEEDED(hr, "Fail to create index buffer.");
		g_indexBufferTable.insert({ resourceHash,  m_indexBuffer });
	}

	g_activeMeshRenderers.insert(this);
}

graphics::MeshRenderer::~MeshRenderer()
{
	g_activeMeshRenderers.erase(this);
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}

math::Matrix4 graphics::MeshRenderer::GetWorldMatrix() const
{
	return m_gameObject->GetTransform();
}