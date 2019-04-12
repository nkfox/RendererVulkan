#pragma once

#include <iostream>
#include <optional>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef NDEBUG
#define enableValidationLayers false
#else
#define enableValidationLayers true
#endif

#define CHECK_RESULT(result, message) if (!result) throw std::runtime_error(message)
#define VULKAN_CHECK_RESULT(result, message) if (result != VK_SUCCESS) throw std::runtime_error(message)

const int WIDTH = 800;
const int HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentationFamily;

	bool isComplete();
};