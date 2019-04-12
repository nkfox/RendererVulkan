#include "VulkanApp.h"
#include "VKCommandBuffer.h"

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
	swapchain.createSurface(instance, window);
	getPhysicalDevice();
	getLogicalDevice();

	VKImage::initDevices(device, physicalDevice);
	VKBuffer::initDevices(device, physicalDevice);
	VKSwapchain::init(device, physicalDevice, queueFamilyIndices);

	initVulkanForSwapchain();

	createSyncObjects();
}

void VulkanApp::initVulkanForSwapchain() {
	swapchain.createSwapChain();
	swapchain.createImageViews();

	createDescriptorSetLayout();
	createPipelineLayout();
	createRenderPass();
	createGraphicsPipeline();

	VKCommandBuffer::createCommandPool(device, graphicsQueue, queueFamilyIndices.graphicsFamily.value());
	createColorResources();
	createDepthResources();
	swapchain.createFramebuffers(renderPass, colorImage, depthImage);
	
	createTextureImage(TEXTURE_PATH.c_str(), &textureImage);
	createTextureImage(NORMAL_TEXTURE_PATH.c_str(), &normalImage);
	createTextureImage(SPECULAR_TEXTURE_PATH.c_str(), &specularImage);

	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();

	createDescriptorPool();
	createDescriptorSets();

	VKCommandBuffer::createCommandBuffers(swapchain.framebuffers, renderPass, pipeline, pipelineLayout, descriptorSets, swapchain.extent, vertexBuffer.buffer, indexBuffer.buffer, indices);
}

void VulkanApp::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	initVulkanForSwapchain();
}

void VulkanApp::cleanupSwapChain() {
	/*for (size_t i = 0; i < framebuffers.size(); i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
	}*/

	VKCommandBuffer::freeCommandBuffers();

	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	/*for (size_t i = 0; i < imageViews.size(); i++) {
		vkDestroyImageView(device, imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapchain, nullptr);*/
	swapchain.free();
}

void VulkanApp::cleanup() {
	colorImage.clear();
	depthImage.clear();

	cleanupSwapChain();

	textureImage.clear();
	normalImage.clear();
	specularImage.clear();

	indexBuffer.clear();
	vertexBuffer.clear();
	for (size_t i = 0; i < swapchain.images.size(); i++) {
		uniformBuffers[i].clear();
	}
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	VKCommandBuffer::destroyCommandPool();

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	
	glfwDestroyWindow(window);
  	glfwTerminate();
}
