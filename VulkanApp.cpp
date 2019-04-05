#include "VulkanApp.h"

VulkanApp::VulkanApp() {
}

VulkanApp::~VulkanApp() {
	cleanup();
}

void VulkanApp::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VulkanApp::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}

void VulkanApp::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	getPhysicalDevice();
	getLogicalDevice();

	initVulkanForSwapchain();

	createSyncObjects();
}

void VulkanApp::initVulkanForSwapchain() {
	createSwapChain();
	createImageViews();

	createDescriptorSetLayout();
	createPipelineLayout();
	createRenderPass();
	createGraphicsPipeline();

	createCommandPool();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	
	createTextureImage(TEXTURE_PATH.c_str(), &textureImage, &textureImageMemory);
	createTextureImage(NORMAL_TEXTURE_PATH.c_str(), &normalImage, &normalImageMemory);
	createTextureImage(SPECULAR_TEXTURE_PATH.c_str(), &specularImage, &specularImageMemory);
	createTextureImageView(&textureImageView, &textureImage);
	createTextureImageView(&normalImageView, &normalImage);
	createTextureImageView(&specularImageView, &specularImage);
	createTextureSampler(&textureSampler);
	createTextureSampler(&normalSampler);
	createTextureSampler(&specularSampler);

	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();

	createDescriptorPool();
	createDescriptorSets();

	createCommandBuffers();
}

void VulkanApp::cleanup() {
	vkDestroyImageView(device, colorImageView, nullptr);
	vkDestroyImage(device, colorImage, nullptr);
	vkFreeMemory(device, colorImageMemory, nullptr);

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroySampler(device, normalSampler, nullptr);
	vkDestroySampler(device, specularSampler, nullptr);

	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImageView(device, normalImageView, nullptr);
	vkDestroyImageView(device, specularImageView, nullptr);

	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	vkDestroyImage(device, normalImage, nullptr);
	vkFreeMemory(device, normalImageMemory, nullptr);

	vkDestroyImage(device, specularImage, nullptr);
	vkFreeMemory(device, specularImageMemory, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	
	glfwDestroyWindow(window);
  	glfwTerminate();
}
