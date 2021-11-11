#include "pch.h"
#include "Mesh.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::System::System(const uint32_t size) : SparseSet<Mesh>(size)
{

}

void Mesh::System::Load(const std::string& fileName, std::vector<Vertex3d>& outVertices, std::vector<uint16_t>& outIndices)
{
	// It's far from the best method, but it's not the end of the world since I'm only planning to load in very simple models.

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string path = "Meshes/" + fileName;
	const bool success = LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	assert(success);

	if (!warn.empty())
		std::cout << warn << std::endl;

	if (!err.empty())
		std::cerr << err << std::endl;

	auto& vertices = attrib.vertices;
	auto& normals = attrib.normals;
	auto& texCoords = attrib.texcoords;

	for (const auto& shape : shapes)
	{
		size_t index_offset = 0;

		auto& faces = shape.mesh.num_face_vertices;
		auto& inds = shape.mesh.indices;

		for (const auto& face : faces)
		{
			const auto fv = size_t(face);

			for (size_t v = 0; v < fv; v++)
			{
				Vertex3d vertex{};
				tinyobj::index_t idx = inds[index_offset + v];

				auto& position = vertex.position;
				const auto vertSize = 3 * size_t(idx.vertex_index);			

				position.x = vertices[vertSize];
				position.y = vertices[vertSize + 1];
				position.z = vertices[vertSize + 2];

				if (idx.normal_index >= 0) 
				{
					auto& normal = vertex.normal;
					const auto normSize = 3 * size_t(idx.normal_index);

					normal.x = normals[normSize];
					normal.y = normals[normSize + 1];
					normal.z = normals[normSize + 2];
				}

				if (idx.texcoord_index >= 0) 
				{
					auto& texCoord = vertex.textureCoordinates;
					const auto texSize = 2 * size_t(idx.texcoord_index);

					texCoord.x = texCoords[texSize];
					texCoord.y = texCoords[texSize + 1];
				}

				outVertices.push_back(vertex);
			}

			index_offset += fv;
		}
	}

	outIndices.resize(outVertices.size());
	for (uint32_t i = 0; i < outVertices.size(); ++i)
		outIndices[i] = i;
}
