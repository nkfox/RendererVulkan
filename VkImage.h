#include "VulkanContext.h"

class VKImage
{
public:
	static void initDevices(VkDevice device, VkPhysicalDevice physicalDevice);

	VKImage(const VKImage &) = delete;

	VKImage() = default;
	~VKImage();

	void clear();

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static VkSampleCountFlagBits getMaxUsableSampleCount();

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	void createImageView(VkFormat format, VkImageAspectFlags aspectFlags);
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createSampler();

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;

	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;

	uint32_t mipLevels;
	static VkSampleCountFlagBits msaaSamples;

private:
	static VkDevice device;
	static VkPhysicalDevice physicalDevice;
};