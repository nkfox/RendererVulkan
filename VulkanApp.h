#include "VulkanContext.h"
#include "VKBuffer.h"
#include "VKSwapchain.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <optional>
#include <array>

const std::string MODEL_PATH = "models/african_head.obj";
const std::string TEXTURE_PATH = "textures/african_head_diffuse.jpg";
const std::string NORMAL_TEXTURE_PATH = "textures/african_head_nm.jpg";
const std::string SPECULAR_TEXTURE_PATH = "textures/african_head_spec.jpg";

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
	//"VK_LAYER_RENDERDOC_Capture"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

void framebufferResizeCallback(GLFWwindow* window, int width, int height);

std::vector<char> readFile(const std::string& filename);

class VulkanApp
{
public:
	VulkanApp();
	~VulkanApp();

	void run();
	void mainLoop();
	void initVulkan();
	void initWindow();
	void cleanup();

private:

	//---------init---------------------------------------------

	void initVulkanForSwapchain();

	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	std::vector<const char*> getDesiredLayers();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void setupDebugMessenger();

	void createInstance();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);

	void getPhysicalDevice();
	void getLogicalDevice();

	//---------presentation------------------------------------

	void recreateSwapChain();
	void cleanupSwapChain();

	//---------graphics----------------------------------------

	VkShaderModule createShaderModule(const std::string fileName);
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createRenderPass();
	void createGraphicsPipeline();

	//---------drawing----------------------------------------
	
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void createTextureImage(const char* filename, VKImage* image);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	void createColorResources();
	void createDepthResources();
	
	void loadModel();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);

	void createDescriptorPool();
	void createDescriptorSets();

	void createSyncObjects();
	void drawFrame();

	//----------------------------------------------------------

private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	QueueFamilyIndices queueFamilyIndices;

	GLFWwindow* window;
	VKSwapchain swapchain;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline pipeline;

	VKImage textureImage;
	VKImage normalImage;
	VKImage specularImage;
	
	VKImage colorImage;
	VKImage depthImage;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VKBuffer vertexBuffer;
	VKBuffer indexBuffer;
	std::vector<VKBuffer> uniformBuffers;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	size_t currentFrame = 0;

public:
	bool framebufferResized = false;
};

