#include "VulkanContext.h"
#include "VkImage.h"

#include <vector>

class VKSwapchain
{
public:
	static void init(VkDevice device, VkPhysicalDevice physicalDevice, QueueFamilyIndices queueFamilyIndicesIn);

	void createSwapChain();
	void free();

	void createSurface(VkInstance instance, GLFWwindow* window);
	void createFramebuffers(VkRenderPass renderPass, VKImage colorImage, VKImage depthImage);

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	void createImageViews();

	VkSwapchainKHR swapchain;

	GLFWwindow* window;
	VkSurfaceKHR surface;
	
	VkFormat imageFormat;
	VkExtent2D extent;

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	std::vector<VkFramebuffer> framebuffers;
	
private:
	static VkDevice device;
	static VkPhysicalDevice physicalDevice;
	static QueueFamilyIndices queueFamilyIndices;
};