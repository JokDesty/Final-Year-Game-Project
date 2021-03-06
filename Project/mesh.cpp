#include <stdio.h>

#include <vector>

#include "mesh.h"

inline u32 to_unsigned_normal(float value, float scale)
{
	return u32(round(clamp(value, 0.0f, 1.0f) * scale));
}

inline u32 pack_vec3_into_u32(vec3 vec)
{
	u32 result = 0;
	u8* dst = (u8*)&result;

	dst[0] = u8(to_unsigned_normal(vec.x * 0.5f + 0.5f, 255.0f));
	dst[1] = u8(to_unsigned_normal(vec.y * 0.5f + 0.5f, 255.0f));
	dst[2] = u8(to_unsigned_normal(vec.z * 0.5f + 0.5f, 255.0f));

	return result;
}

mesh* mesh_create(char* asset_id, pos_normal_vertex* vertices, u32 vertex_count, u32* indices, u32 index_count)
{
	mesh* result = (mesh*) debug_malloc(sizeof(mesh));

	result->asset_id = asset_id;
	result->asset_type = ASSET_TYPE_MESH;

	bgfx_vertex_decl_t decl;
	bgfx_vertex_decl_begin(&decl, BGFX_RENDERER_TYPE_NOOP);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_NORMAL, 4, BGFX_ATTRIB_TYPE_UINT8, true, true);
	bgfx_vertex_decl_end(&decl);

	result->vertex_count = vertex_count;
	u32 vertex_byte_count = vertex_count * sizeof(pos_normal_vertex);

	const bgfx_memory_t* vertices_mem = bgfx_make_ref(vertices, vertex_byte_count);
	result->vb_handle = bgfx_create_vertex_buffer(vertices_mem, &decl, BGFX_BUFFER_NONE);
	result->vertices = vertices;
	bgfx_make_ref_release(vertices, vertex_byte_count, 0, 0);

	result->index_count = index_count;
	u32 index_byte_count = index_count * sizeof(u32);

	const bgfx_memory_t* vertex_index_mem = bgfx_make_ref(indices, index_byte_count);
	result->idb_handle = bgfx_create_index_buffer(vertex_index_mem, BGFX_BUFFER_INDEX32);
	result->indices = indices;
	bgfx_make_ref_release(vertex_index_mem, index_byte_count, 0, 0);

	return result;
}

// @Todo: this doesn't unalloc obj mesh buffers :/ (vertex and index buffer)
void mesh_destroy(mesh* m)
{
	free(m);

	bgfx_destroy_vertex_buffer(m->vb_handle);
	bgfx_destroy_index_buffer(m->idb_handle);
}

mesh* load_obj_mesh(char* asset_id, char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "r");

	if (file == NULL)
	{
		printf("Failed to load obj mesh from file %s\n", filename);

		// @Safety: safe return a default mesh? like a cube?
		return{};
	}

	std::vector<u32> vertex_ids, uv_ids, normal_ids;
	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;

	while (true)
	{
		// @Todo: the first word of the line can be longer than 128 apparently?
		char header[128];
		int res = fscanf(file, "%s", header);
		if (res == EOF) break;

		if (strcmp(header, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(header, "vt") == 0)
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(header, "vn") == 0)
		{
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(header, "f") == 0)
		{
			u32 vertex_index[3], uv_index[3], normal_index[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertex_index[0], &uv_index[0], &normal_index[0], &vertex_index[1], &uv_index[1], &normal_index[1], &vertex_index[2], &uv_index[2], &normal_index[2]);
			vertex_ids.push_back(vertex_index[0]);
			vertex_ids.push_back(vertex_index[1]);
			vertex_ids.push_back(vertex_index[2]);
			uv_ids.push_back(uv_index[0]);
			uv_ids.push_back(uv_index[1]);
			uv_ids.push_back(uv_index[2]);
			normal_ids.push_back(normal_index[0]);
			normal_ids.push_back(normal_index[1]);
			normal_ids.push_back(normal_index[2]);
		}
	}

	fclose(file);

	// @Speed: you don't need 2 debug_mallocs here
	u32 vertex_count = temp_vertices.size();
	pos_normal_vertex* vertices = (pos_normal_vertex*) debug_malloc(vertex_count * sizeof(pos_normal_vertex));

	// Index all the normals for each vertex so we don't have to search for the normal for each vertex
	vec3* vertex_normals = (vec3*) debug_calloc(sizeof(vec3), vertex_count);
	vec2* vertex_uvs = (vec2*) debug_malloc(vertex_count * sizeof(vec2));

	for (u32 face_id = 0; face_id < vertex_ids.size(); face_id++)
	{
		int vertex_id = vertex_ids[face_id] - 1;

		vertex_normals[vertex_id] += temp_normals[normal_ids[face_id] - 1];

		if(temp_uvs.size() > 0) vertex_uvs[vertex_id] = temp_uvs[uv_ids[face_id] - 1];
	}

	for (u32 i = 0; i < vertex_count; i++)
	{
		vec3 ver = temp_vertices[i];
		vec2 uv;

		if(temp_uvs.size() > 0)
		{
			uv = vertex_uvs[i];
		}
		else
		{
			uv = vec2(0.0f);
		}

		vertices[i] = { ver.x, ver.y, ver.z, uv.x, uv.y, pack_vec3_into_u32(normalize(vertex_normals[i])) };
	}

	u32 index_count = vertex_ids.size();
	u32 index_byte_count = index_count * sizeof(u32);
	u32* indices = (u32*) debug_malloc(index_byte_count);

	for (u32 i = 0; i < index_count; i++)
	{
		indices[i] = vertex_ids[i] - 1;
	}

	return mesh_create(asset_id, vertices, vertex_count, indices, index_count);
}