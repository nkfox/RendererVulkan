#include "VKSwapchain.h"
#include "VKCommandBuffer.h"

#include <algorithm>
#include <array>

VkDevice VKSwapchain::device = nullptr;
VkPhysicalDevice VKSwapchain::physicalDevice = nullptr;
QueueFamilyIndices VKSwapchain::queueFamilyIndices = { 0, 0 };

void VKSwapchain::init(VkDevice deviceIn, VkPhysicalDevice physicalDeviceIn, QueueFamilyIndices queueFamilyIndicesIn)
{
	device = deviceIn;
	physicalDevice = physicalDeviceIn;
	queueFamilyIndices = queueFamilyIndicesIn;
}

void VKSwapchain::createSwapChain() {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableFormats.data());
	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(availableFormats);

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	uint32_t modeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, nullptr);
	std::vector<VkPresentModeKHR> availableModes(modeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, availableModes.data());

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = imageCount;

	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;

	info.imageExtent = chooseExtent(surfaceCapabilities);
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentationFamily.value()) {
		uint32_t indices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentationFamily.value() };
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = indices;
	}
	else {
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0;
		info.pQueueFamilyIndices = nullptr;
	}

	info.preTransform = surfaceCapabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = choosePresentMode(availableModes);
	info.clipped = VK_TRUE;
	info.oldSwapchain = 0;

	VULKAN_CHECK_RESULT(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain), "Failed to create swap chain!");

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;
	extent = info.imageExtent;
}

void VKSwapchain::free() {
	for (size_t i = 0; i < framebuffers.size(); i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}

	for (size_t i = 0; i < imageViews.size(); i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void VKSwapchain::createSurface(VkInstance instance, GLFWwindow* windowIn) {
	window = windowIn;

	VkWin32SurfaceCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hwnd = glfwGetWin32Window(window);
	info.hinstance = GetModuleHandle(nullptr);
	VULKAN_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface), "Failed to create window surface!");
}

void VKSwapchain::createFramebuffers(VkRenderPass renderPass, VKImage colorImage, VKImage depthImage) {
	framebuffers.resize(images.size());
	for (size_t i = 0; i < imageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorImage.imageView,
			depthImage.imageView,
			imageViews[i]
		};

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = renderPass;
		info.attachmentCount = static_cast<uint32_t>(attachments.size());
		info.pAttachments = attachments.data();
		info.height = extent.height;
		info.width = extent.width;
		info.layers = 1;

		VULKAN_CHECK_RESULT(vkCreateFramebuffer(device, &info, nullptr, &framebuffers[i]), "Failed to create framebuffer!");
	}
}

VkSurfaceFormatKHR VKSwapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkExtent2D VKSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max<uint32_t>(capabilities.minImageExtent.width, std::min<uint32_t>(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max<uint32_t>(capabilities.minImageExtent.height, std::min<uint32_t>(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkPresentModeKHR VKSwapchain::choosePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

void VKSwapchain::createImageViews() {
	imageViews.resize(images.size());
	for (uint32_t i = 0; i < images.size(); i++) {
		imageViews[i] = VKImage::createImageView(images[i], imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}