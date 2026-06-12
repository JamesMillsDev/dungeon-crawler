#pragma once
#include <variant>
#include <cstdint>

struct DrawTextureCommand
{
	uint32_t textureId;
	float x;
	float y;
	float width;
	float height;
};

struct DrawMeshCommand
{
	uint32_t meshId;
	float x;
	float y;
	float z;
};

using RenderCommand = std::variant<DrawTextureCommand, DrawMeshCommand>;