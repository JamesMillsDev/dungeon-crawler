#pragma once

#include <vector>
#include <vulkan/vulkan.h>

using std::vector;

#ifdef _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

const vector VALIDATION_LAYERS =
{
	"VK_LAYER_KHRONOS_validation"
};

const vector DEVICE_EXTENSIONS =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};