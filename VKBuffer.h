#include "VulkanContext.h"

class VKBuffer
{
public:
	static void initDevices(VkDevice device, VkPhysicalDevice physicalDevice);

	~VKBuffer();

	void clear();

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VKBuffer* buffer);
	static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void createBuffer(VkDeviceSize size, void* data, VkBufferUsageFlags usage);

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

private:
	static VkDevice device;
	static VkPhysicalDevice physicalDevice;
};