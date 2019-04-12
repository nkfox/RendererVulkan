#include "VulkanApp.h"

#include <set>

bool QueueFamilyIndices::isComplete() {
	return graphicsFamily.has_value() && presentationFamily.has_value();
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void VulkanApp::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

std::vector<const char*> VulkanApp::getRequiredExtensions() {

	/*{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		std::cout << "Available extensions:" << std::endl;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}*/

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

bool VulkanApp::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> VulkanApp::getDesiredLayers() {

	/*{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
		std::cout << "Available layers:" << std::endl;
		for (const auto& layer : layers) {
			std::cout << "\t" << layer.layerName << std::endl;
		}
	}*/

	CHECK_RESULT((!enableValidationLayers || checkValidationLayerSupport()), "Validation layers requested, but not available!");
	return enableValidationLayers ? validationLayers : std::vector<const char*>();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanApp::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = 0; // not used now
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
	VULKAN_CHECK_RESULT(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger!");
}

void VulkanApp::createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Vulkan Renderer";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0; // not used now
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> requiredExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	std::vector<const char*> desiredLayers = getDesiredLayers();
	createInfo.enabledLayerCount = (uint32_t)desiredLayers.size();
	createInfo.ppEnabledLayerNames = desiredLayers.data();

	VULKAN_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance), "Failed to create instance!");

}

QueueFamilyIndices VulkanApp::findQueueFamilies(VkPhysicalDevice physicalDevice) {
	uint32_t propertiesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertiesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProps(propertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propertiesCount, queueFamilyProps.data());

	int queueFamilyIndex = 0;
	VkBool32 supportPresentation = false;
	QueueFamilyIndices indices;
	for (VkQueueFamilyProperties queueFamily : queueFamilyProps) {
		if (!queueFamily.queueCount) { queueFamilyIndex++; continue; }

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = queueFamilyIndex;
		}
		
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, swapchain.surface, &supportPresentation);
		if (supportPresentation) {
			indices.presentationFamily = queueFamilyIndex;
		}

		if (indices.isComplete()) {
			break;
		}

		queueFamilyIndex++;
	}
	return indices;
}

bool VulkanApp::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool VulkanApp::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
	
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

void VulkanApp::getPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	CHECK_RESULT(deviceCount, "Failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
	for (VkPhysicalDevice availablePhysicalDevice : physicalDevices) {
		if (isDeviceSuitable(availablePhysicalDevice)) {
			physicalDevice = availablePhysicalDevice;
			return;
		}

	}
}

void VulkanApp::getLogicalDevice() {
	queueFamilyIndices = findQueueFamilies(physicalDevice);
	std::set<uint32_t> queueIndices = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentationFamily.value() };

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	float queuePriority = 1.0f;
	for (uint32_t queueIndex : queueIndices) {
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueIndex; //queueFamilyIndices.graphicsFamily.value();
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;
		queueInfos.push_back(queueInfo);
	}

	VkPhysicalDeviceFeatures features = {};
	//vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	features.samplerAnisotropy = VK_TRUE;
	features.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	info.queueCreateInfoCount = (uint32_t)queueInfos.size();
	info.pQueueCreateInfos = queueInfos.data();

	info.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	info.ppEnabledExtensionNames = deviceExtensions.data();

	info.pEnabledFeatures = &features;

	VULKAN_CHECK_RESULT(vkCreateDevice(physicalDevice, &info, nullptr, &device), "Failed to create logical device!");

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentationFamily.value(), 0, &presentationQueue);
}