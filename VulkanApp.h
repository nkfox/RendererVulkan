#include "VulkanContext.h"

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

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentationFamily;

	bool isComplete();
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
	VkSampleCountFlagBits getMaxUsableSampleCount();

	void getPhysicalDevice();
	void getLogicalDevice();

	//---------presentation------------------------------------

	void createSurface();

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	void createSwapChain();
	void recreateSwapChain();
	void cleanupSwapChain();
	void createImageViews();

	//---------graphics----------------------------------------

	VkShaderModule createShaderModule(const std::string fileName);
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createRenderPass();
	void createGraphicsPipeline();

	//---------drawing----------------------------------------

	void createFramebuffers();
	void createCommandPool();

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void createTextureImage(const char* filename, VkImage* image, VkDeviceMemory* imageMemory);
	void createTextureImageView(VkImageView* view, VkImage* image);
	void createTextureSampler(VkSampler* sampler);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	void createColorResources();
	void createDepthResources();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void loadModel();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);

	void createDescriptorPool();
	void createDescriptorSets();

	void createCommandBuffers();
	void createSyncObjects();
	void drawFrame();

	//----------------------------------------------------------

private:
	GLFWwindow * window;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline pipeline;

	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;

	uint32_t mipLevels;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage normalImage;
	VkDeviceMemory normalImageMemory;
	VkImageView normalImageView;
	VkSampler normalSampler;

	VkImage specularImage;
	VkDeviceMemory specularImageMemory;
	VkImageView specularImageView;
	VkSampler specularSampler;
	
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	size_t currentFrame = 0;
public:
	bool framebufferResized = false;
};

