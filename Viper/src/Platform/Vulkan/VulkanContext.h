#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Viper/Renderer/GraphicsContext.h"
#include "Viper/Window.h"
#include "Platform/Vulkan/VulkanDebugger.h"

#include <filesystem>

namespace Viper
{
	struct QueueFamilyIndices;
	struct SwapChainSupportDetails;

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

			// position
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			// color
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};




	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(Window *window);
		~VulkanContext();

		void init() override;
		void swapBuffers() override;

		// testing
		void updateVertices(std::pair<float, float> v1, std::pair<float, float> v2, std::pair<float, float> v3) override
		{
			
			this->vertices[0].pos = { v1.first, v1.second };
			this->vertices[1].pos = { v2.first, v2.second };
			this->vertices[2].pos = { v3.first, v3.second };

			// update vertex buffer
			VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);


			// Filling the vertex buffer
			void *data;
			vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, this->vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(this->device, stagingBufferMemory);

			// move the vertex data to the device local buffer
			this->copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

			vkDestroyBuffer(this->device, stagingBuffer, nullptr);
			vkFreeMemory(this->device, stagingBufferMemory, nullptr);

			V_CORE_INFO("vertices updated! - {0}, {1}", this->vertices[0].pos.x, this->vertices[0].pos.y);
		}

	private:
		void createInstance();
		void createSurface();



		/******************** Physical devices and queue families ********************/

		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);



		/******************** Creating the logical device ********************/

		void createLogicalDevice();



		/******************** Swap chain ********************/

		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

		// Choosing the right settings for the swap chain
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

		// Creating the swap chain
		void createSwapChain();



		/******************** Recreating the swap chain ********************/

		void recreateSwapChain();
		void cleanupSwapChain();



		/******************** Image views ********************/

		void createImageViews();



		/******************** Graphics pipeline ********************/

		void createGraphicsPipeline();
		VkShaderModule createShaderModule(const std::vector<char> &code);



		/******************** Render passes ********************/

		void createRenderPass();



		/******************** Framebuffers ********************/

		void createFramebuffers();



		/******************** Command pools and command buffers ********************/

		void createCommandPool();
		void createCommandBuffers();



		/******************** Drawing ********************/

		void drawFrame();
		void createSyncObjects();



		/******************** Vertex buffer creation ********************/

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void createVertexBuffer();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void createIndexBuffer();

		/********************  ********************/
	private:
		Window *window;
		GLFWwindow *windowHandle;

		VkInstance instance;
		VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImageView> swapChainImageViews;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		size_t currentFrame = 0;

		std::vector<Vertex> vertices =
		{
			//{{0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
			//{{-0.5f, -0.8f}, {1.0f, 0.0f, 0.0f}},
			//{{0.5f, -0.8f}, {0.0f, 1.0f, 0.0f}},

			//{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			//{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			//{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			//{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

			//{{-0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}},
			//{{0.1f, -0.1f}, {0.0f, 0.0f, 0.0f}},
			//{{0.1f, 0.1f}, {0.0f, 0.0f, 0.0f}},
			//{{-0.1f, 0.1f}, {0.0f, 0.0f, 0.0f}}

			{{-1.0f, -1.0f}, {1.0f, 0.0f, 1.0f}},
			{{1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
			{{0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}}
		};

		std::vector<uint16_t> indices =
		{
			//0, 1, 2, 2, 3, 0,
			//3, 4, 5, 5, 6, 3,
			//7, 8, 9, 9, 10, 7

			0, 1, 2, 2, 1, 0
		};

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;

		bool enableValidationLayers = true;
		VulkanDebugger *debugger = new VulkanDebugger(enableValidationLayers);
	};

}