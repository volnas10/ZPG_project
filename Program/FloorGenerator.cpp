#include "FloorGenerator.h"

#include <glm/glm.hpp>
#include <cstdlib>
/*
FloorGenerator::FloorGenerator(int dimension, float tile_size, std::vector<Texture*> textures) {
	this->dimension = dimension;
	this->tile_size = tile_size;
	this->textures = textures;
}
*/
object::Object* FloorGenerator::generate() {

	std::vector<glm::vec3> vertices;
	vertices.reserve(4 * dimension * dimension);
	std::vector<glm::vec3> normals(4 * dimension * dimension, glm::vec3(0, 1, 0));
	std::vector<glm::vec2> uvs;
	uvs.reserve(4 * dimension * dimension);
	std::vector<glm::vec3> tangents;
	tangents.reserve(4 * dimension * dimension);
	std::vector<glm::vec3> bitangents;
	bitangents.reserve(4 * dimension * dimension);
	std::vector<unsigned int> indices;
	indices.reserve(6 * dimension * dimension);

	glm::vec3 tmp_tans[] = { glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), glm::vec3(-1, 0, 0), glm::vec3(0, 0, -1) };
	glm::vec2 tmp_uvs[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1)};
	float half_size = (tile_size * dimension) / 2;
	for (int i = 0; i < dimension; i++) {
		float x_offset = (float)i * tile_size - half_size;
		for (int j = 0; j < dimension; j++) {
			float y_offset = (float)j * tile_size - half_size;

			indices.push_back(vertices.size());
			vertices.push_back(glm::vec3(x_offset, 0, y_offset));

			indices.push_back(vertices.size());
			vertices.push_back(glm::vec3(x_offset + tile_size, 0, y_offset));

			indices.push_back(vertices.size());
			indices.push_back(vertices.size());
			vertices.push_back(glm::vec3(x_offset + tile_size, 0, y_offset + tile_size));

			indices.push_back(vertices.size());
			vertices.push_back(glm::vec3(x_offset, 0, y_offset + tile_size));

			indices.push_back(vertices.size() - 4);

			int rotation = rand() % 4;

			glm::vec3 tangent = tmp_tans[rotation];
			glm::vec3 bitangent = tmp_tans[(rotation + 1) % 4];
			tangents.insert(tangents.end(), 4, tangent);
			bitangents.insert(bitangents.end(), 4, bitangent);

			uvs.push_back(tmp_uvs[rotation]);
			uvs.push_back(tmp_uvs[(rotation + 1) % 4]);
			uvs.push_back(tmp_uvs[(rotation + 2) % 4]);
			uvs.push_back(tmp_uvs[(rotation + 3) % 4]);
		}
	}

	object::Material material;
	material.diffuse_color = glm::vec4(0.8, 0.8, 0.8, 1.0);
	material.ambient_color = glm::vec4(0.2, 0.2, 0.2, 1.0);
	material.specular_color = glm::vec4(0.2, 0.2, 0.2, 1.0);

	object::Object* obj = new object::Object();
	obj->addMesh(vertices, normals, uvs, tangents, bitangents, indices, material);

	return obj;
}
