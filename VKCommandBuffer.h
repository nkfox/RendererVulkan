#include "VulkanContext.h"

#include <vector>

class VKCommandBuffer
{
public:
	static void createCommandPool(VkDevice deviceIn, VkQueue graphicsQueueIn, uint32_t graphicsFamilyIndex);
	static void destroyCommandPool();

	static void createCommandBuffers(std::vector<VkFramebuffer> swapChainFramebuffers, VkRenderPass renderPass, VkPipeline pipeline, VkPipelineLayout pipelineLayout, 
		std::vector<VkDescriptorSet> descriptorSets, VkExtent2D extent, VkBuffer vertexBuffer, VkBuffer indexBuffer, std::vector<uint32_t> indices);
	static VkCommandBuffer* getCommandBuffer(int imageIndex);
	static void freeCommandBuffers();

	static VkCommandBuffer beginSingleTimeCommands();
	static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

//private:
	static VkCommandPool commandPool;

private:
	static std::vector<VkCommandBuffer> commandBuffers;
	
	static VkDevice device;
	static VkQueue graphicsQueue;
};