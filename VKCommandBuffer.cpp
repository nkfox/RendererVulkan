#include "VKCommandBuffer.h"

#include <array>

VkCommandPool VKCommandBuffer::commandPool;
std::vector<VkCommandBuffer> VKCommandBuffer::commandBuffers;
VkDevice VKCommandBuffer::device;
VkQueue VKCommandBuffer::graphicsQueue;


void VKCommandBuffer::createCommandPool(VkDevice deviceIn, VkQueue graphicsQueueIn, uint32_t graphicsFamilyIndex) {
	device = deviceIn;
	graphicsQueue = graphicsQueueIn;

	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = 0;
	info.queueFamilyIndex = graphicsFamilyIndex;

	VULKAN_CHECK_RESULT(vkCreateCommandPool(device, &info, nullptr, &commandPool), "Failed to create command pool!");
}

void VKCommandBuffer::destroyCommandPool() {
	vkDestroyCommandPool(device, commandPool, nullptr);
}

void VKCommandBuffer::createCommandBuffers(std::vector<VkFramebuffer> swapChainFramebuffers, VkRenderPass renderPass, VkPipeline pipeline, VkPipelineLayout pipelineLayout, 
	std::vector<VkDescriptorSet> descriptorSets, VkExtent2D extent, VkBuffer vertexBuffer, VkBuffer indexBuffer, std::vector<uint32_t> indices) {
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo  info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = VKCommandBuffer::commandPool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = (uint32_t)commandBuffers.size();

	VULKAN_CHECK_RESULT(vkAllocateCommandBuffers(device, &info, commandBuffers.data()), "Failed to create command buffer!");

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VULKAN_CHECK_RESULT(vkBeginCommandBuffer(commandBuffers[i], &beginInfo), "Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		VULKAN_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer!");
	}
}

VkCommandBuffer* VKCommandBuffer::getCommandBuffer(int imageIndex) {
	return &commandBuffers[imageIndex];
}

void VKCommandBuffer::freeCommandBuffers() {
	vkFreeCommandBuffers(device, VKCommandBuffer::commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}

VkCommandBuffer VKCommandBuffer::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VKCommandBuffer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
