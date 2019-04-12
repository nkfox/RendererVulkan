#include "VKBuffer.h"
#include "VKCommandBuffer.h"

VkDevice VKBuffer::device = nullptr;
VkPhysicalDevice VKBuffer::physicalDevice = nullptr;

void VKBuffer::initDevices(VkDevice deviceIn, VkPhysicalDevice physicalDeviceIn)
{
	device = deviceIn;
	physicalDevice = physicalDeviceIn;
}

VKBuffer::~VKBuffer()
{
	clear();
}

void VKBuffer::clear()
{
	if (buffer)
		vkDestroyBuffer(device, buffer, nullptr);
	if (bufferMemory)
		vkFreeMemory(device, bufferMemory, nullptr);
}

void VKBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VKBuffer* buffer) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VULKAN_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer), "Failed to create buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer->buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	VULKAN_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &buffer->bufferMemory), "Failed to allocate buffer memory!");

	vkBindBufferMemory(device, buffer->buffer, buffer->bufferMemory, 0);
}

void VKBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = VKCommandBuffer::beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	VKCommandBuffer::endSingleTimeCommands(commandBuffer);
}

void VKBuffer::createBuffer(VkDeviceSize size, void* data, VkBufferUsageFlags usage) {

	VKBuffer stagingBuffer;
	createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer);

	void* dstData;
	vkMapMemory(device, stagingBuffer.bufferMemory, 0, size, 0, &dstData);
	memcpy(dstData, data, (size_t)size);
	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this);

	copyBuffer(stagingBuffer.buffer, buffer, size);
}

uint32_t VKBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	CHECK_RESULT(true, "Failed to find suitable memory type!");
}