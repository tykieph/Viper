#include "vpch.h"
#include "VulkanContext.h"

#include <glm/glm.hpp>

namespace Viper
{

	#define MAX_FRAMES_IN_FLIGHT 2

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};



	struct SwapChainSupportDetails
	{
		/*
			- Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
			- Surface formats (pixel format, color space)
			- Available presentation modes
		*/

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};



	static std::vector<char> readFile(const std::string &filename)
	{
		/*
			Read all of the bytes from the specified file and return them in a byte array managed by std::vector.
			Flags:
				- ate: Start reading at the end of the file
				- binary: Read the file as binary file (avoid text transformations)
		*/

		std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}



	VulkanContext::VulkanContext(Window *window)
		: window(window)
	{
		windowHandle = static_cast<GLFWwindow *>(window->getNativeWindow());
		V_CORE_ASSERT(windowHandle, "Window handle is null!");
	}



	VulkanContext::~VulkanContext()
	{
		/*
			Cleanup memory
		*/

		this->cleanupSwapChain();

		vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
		vkFreeMemory(this->device, this->indexBufferMemory, nullptr);

		vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
		vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(this->device, this->renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(this->device, this->imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(this->device, this->inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(this->device, this->commandPool, nullptr);

		vkDestroyDevice(this->device, nullptr);

		if (this->debugger->enableValidationLayers)
			this->debugger->DestroyDebugUtilsMessengerEXT(this->instance, this->debugger->debugMessenger, nullptr);

		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
		vkDestroyInstance(this->instance, nullptr);
	}



	void VulkanContext::init()
	{
		this->createInstance();
		this->debugger->setupDebugMessenger(this->instance);
		this->createSurface();
		this->pickPhysicalDevice();
		this->createLogicalDevice();
		this->createSwapChain();
		this->createImageViews();
		this->createRenderPass();
		this->createGraphicsPipeline();
		this->createFramebuffers();
		this->createCommandPool();
		this->createVertexBuffer();
		this->createIndexBuffer();
		this->createCommandBuffers();
		this->createSyncObjects();
	}



	void VulkanContext::swapBuffers()
	{
		glfwPollEvents();
		this->drawFrame();
	}



	void VulkanContext::createInstance()
	{
		/*
			Create an instance.
			The instance is the connection between your application and the Vulkan library and creating it involves specifying some details about your application to the driver.
		*/

		// check layer validation
		V_CORE_ASSERT(this->enableValidationLayers && this->debugger->checkValidationLayerSupport(),
					  "validation layers requested, but not available!")

		// application info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Application name";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Viper";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// instance info
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		// include validation layer names if enabled
		if (this->enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(this->debugger->validationLayers.size());
			createInfo.ppEnabledLayerNames = this->debugger->validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// load extensions
		std::vector<const char *> extensions = this->debugger->getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Create instance
		if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
			V_CORE_ASSERT(false, "failed to create instance!");
	}



	void VulkanContext::createSurface()
	{
		/*
			Window surface creation.
		*/

		if (glfwCreateWindowSurface(this->instance, this->windowHandle, nullptr, &this->surface) != VK_SUCCESS)
			V_CORE_ASSERT(false, "failed to create window surface!");
	}



	/******************** Physical devices and queue families ********************/

	void VulkanContext::pickPhysicalDevice()
	{
		/*
			pick a proper graphics card
		*/


		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

		V_CORE_ASSERT(deviceCount, "failed to find GPUs with Vulkan support!");

		// allocate an array to hold all of the VkPhysicalDevice handles
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

		// find a suitable device
		for (const auto &device : devices)
		{
			if (this->isDeviceSuitable(device))
			{
				this->physicalDevice = device;
				break;
			}
		}

		if (this->physicalDevice == VK_NULL_HANDLE)
			V_CORE_ASSERT(false, "failed to find a suitable GPU!");
	}

	bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device)
	{
		/*
			check if graphics card is suitable
		*/

		QueueFamilyIndices indices = this->findQueueFamilies(device);

		// check extensions support
		bool extensionsSupported = this->checkDeviceExtensionSupport(device);

		// verify that swap chain support is adequate
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device)
	{
		/*
			check which queue families are supported by the device and which one of these supports the commands that we want to use
		*/

		QueueFamilyIndices indices;

		// retrieve the list of queue families
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (auto const &queueFamily : queueFamilies)
		{
			// Querying for presentation support
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
			}

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				break;
			}

			i++;
		}


		return indices;
	}



	/******************** Creating the logical device ********************/

	void VulkanContext::createLogicalDevice()
	{
		/*
			Create logical device and retrieve queue handles
		*/

		// Specifying the queues to be created
		QueueFamilyIndices indices = this->findQueueFamilies(this->physicalDevice);

		// create a set of all unique queue families that are necessary for the required queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies =
		{
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// Specifying used device features
		VkPhysicalDeviceFeatures deviceFeatures = {};

		// Creating the logical device
		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		// Enabling device extensions
		createInfo.enabledExtensionCount = static_cast<uint32_t>(this->debugger->deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = this->debugger->deviceExtensions.data();


		// Enabling validation layers
		if (this->debugger->enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(this->debugger->validationLayers.size());
			createInfo.ppEnabledLayerNames = this->debugger->validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		// Retrieving queue handles
		vkGetDeviceQueue(this->device, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
		vkGetDeviceQueue(this->device, indices.presentFamily.value(), 0, &this->presentQueue);
	}



	/******************** Swap chain ********************/

	bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		/*
			Checking for swap chain support
		*/

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions;
		requiredExtensions.insert(this->debugger->deviceExtensions.begin(), this->debugger->deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device)
	{
		/*
			Querying details of swap chain support.

			There are basically three kinds of properties we need to check:
				- Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
				- Surface formats (pixel format, color space)
				- Available presentation modes
		*/

		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &details.capabilities);

		// querying the supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

		if (formatCount > 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, details.formats.data());
		}

		// querying the supported presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, nullptr);

		if (presentModeCount > 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		/*
			Surface format (color depth).
		*/

		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// if previous statements failed - return first format that is specified
		return availableFormats[0];
	}

	VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		/*
			Presentation mode (conditions for "swapping" images to the screen).

			There are four possible modes available:
				- VK_PRESENT_MODE_IMMEDIATE_KHR
				- VK_PRESENT_MODE_FIFO_KHR
				- VK_PRESENT_MODE_FIFO_RELAXED_KHR
				- VK_PRESENT_MODE_MAILBOX_KHR
		*/

		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto &availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
	{
		/*
			The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the resolution of the window that we're drawing to.
			The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure.
		*/

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(this->windowHandle, &width, &height);
		}

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(this->window->getWidth()),
			static_cast<uint32_t>(this->window->getHeight())
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	void VulkanContext::createSwapChain()
	{
		/*
			The swap chain is essentially a queue of images that are waiting to be presented to the screen.
		*/

		SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(this->physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = this->chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = this->chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = this->chooseSwapExtent(swapChainSupport.capabilities);

		// We have to decide how many images we would like to have in the swap chain. 
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Fill in the structure
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = this->surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;

		// The imageArrayLayers specifies the amount of layers each image consists of. 
		// This is always 1 unless you are developing a stereoscopic 3D application.
		createInfo.imageArrayLayers = 1;

		// The imageUsage bit field specifies what kind of operations we'll use the images in the swap chain for. 
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// We need to specify how to handle swap chain images that will be used across multiple queue families.
		// We'll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue
		QueueFamilyIndices indices = this->findQueueFamilies(this->physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		// We can specify that a certain transform should be applied to images in the swap chain if it is supported (supportedTransforms in capabilities), 
		// like a 90 degree clockwise rotation or horizontal flip.
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in the window system.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapChain) != VK_SUCCESS)
			V_CORE_ASSERT(false, "failed to create swap chain!");

		// Retrieve the handles
		// First query the final number of images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to retrieve the handles.
		vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, nullptr);
		this->swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, this->swapChainImages.data());

		this->swapChainImageFormat = surfaceFormat.format;
		this->swapChainExtent = extent;
	}



	/******************** Recreating the swap chain ********************/

	void VulkanContext::recreateSwapChain()
	{
		// pause program if window is minimalized
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(this->windowHandle, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(this->device);

		this->cleanupSwapChain();

		this->createSwapChain();
		this->createImageViews();
		this->createRenderPass();
		this->createGraphicsPipeline();
		this->createFramebuffers();
		this->createCommandBuffers();
	}

	void VulkanContext::cleanupSwapChain()
	{
		/*
			Cleanup swap chain memory
		*/

		for (auto framebuffer : this->swapChainFramebuffers)
			vkDestroyFramebuffer(this->device, framebuffer, nullptr);

		vkFreeCommandBuffers(this->device, this->commandPool, static_cast<uint32_t>(this->commandBuffers.size()), this->commandBuffers.data());
		vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
		vkDestroyRenderPass(this->device, this->renderPass, nullptr);

		for (auto imageView : this->swapChainImageViews)
			vkDestroyImageView(this->device, imageView, nullptr);

		vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);
	}



	/******************** Image views ********************/

	void VulkanContext::createImageViews()
	{
		/*
			Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
		*/

		this->swapChainImageViews.resize(this->swapChainImages.size());

		for (std::size_t i = 0; i < this->swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = this->swapChainImages[i];

			// The viewType and format fields specify how the image data should be interpreted. 
			// The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = this->swapChainImageFormat;

			// The components field allows you to swizzle the color channels around.
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// The subresourceRange field describes what the image's purpose is and which part of the image should be accessed. 
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(this->device, &createInfo, nullptr, &this->swapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create image views!");
		}
	}



	/******************** Graphics pipeline ********************/

	void VulkanContext::createGraphicsPipeline()
	{
		/*
			The graphics pipeline is the sequence of operations that take the vertices and textures of your meshes all the way to the pixels in the render targets.

			Stages: input assembler -> vertex shader -> tesselation -> geometry shader -> rasterization -> fragment shader -> color blending
			Fixed-function stages: input assembler, rasterization, color blending
			Programmable stages: vertex shader, tesselation, geometry shader, fragment shader
		*/
		
		auto vertShaderCode = readFile("..//Viper//src//Viper//Renderer//Shaders//vert.spv");
		auto fragShaderCode = readFile("..//Viper//src//Viper//Renderer//Shaders//frag.spv");

		VkShaderModule vertShaderModule = this->createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = this->createShaderModule(fragShaderCode);

		//////////////////// Vertex shader stage creation
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		// shader module containing the code
		vertShaderStageInfo.module = vertShaderModule;

		// entrypoint, function to invoke 
		vertShaderStageInfo.pName = "main";


		//////////////////// Fragment shader stage creation
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		// shader module containing the code
		fragShaderStageInfo.module = fragShaderModule;

		// entrypoint, function to invoke 
		fragShaderStageInfo.pName = "main";


		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


		//////////////////// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescriptions = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		// The pVertexBindingDescriptions and pVertexAttributeDescriptions members point to an array of structs that describe the aforementioned details for loading vertex data.
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


		//////////////////// Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};

		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		//////////////////// Viewports and scissors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->swapChainExtent.width;
		viewport.height = (float)this->swapChainExtent.height;

		// The minDepth and maxDepth values specify the range of depth values to use for the framebuffer. 
		// These values must be within the [0.0f, 1.0f] range, but minDepth may be higher than maxDepth.
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = this->swapChainExtent;

		//////////////////// Viewport state
		VkPipelineViewportStateCreateInfo viewportState = {};

		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


		//////////////////// Rasterizer state
		VkPipelineRasterizationStateCreateInfo rasterizer = {};

		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

		// If depthClampEnable is set to VK_TRUE, 
		// then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them. 
		rasterizer.depthClampEnable = VK_FALSE;

		// If rasterizerDiscardEnable is set to VK_TRUE, 
		// then geometry never passes through the rasterizer stage. 
		// This basically disables any output to the framebuffer.
		rasterizer.rasterizerDiscardEnable = VK_FALSE;

		// The polygonMode determines how fragments are generated for geometry. 
		// The following modes are available: VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


		//////////////////// Multisampling state
		VkPipelineMultisampleStateCreateInfo multisampling = {};

		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional


		//////////////////// Color blending state
		// blend colors returned from fragment shader
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};

		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		//////////////////// Dynamic state 
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		//////////////////// Pipeline layout creation
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");


		//////////////////// Graphics pipeline creation
		VkGraphicsPipelineCreateInfo pipelineInfo = {};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		// We start by referencing the array of VkPipelineShaderStageCreateInfo structs.
		pipelineInfo.pStages = shaderStages;

		// Then we reference all of the structures describing the fixed-function stage.
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		pipelineInfo.layout = this->pipelineLayout;
		pipelineInfo.renderPass = this->renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = nullptr; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional


		if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");

		vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
		vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
	}

	VkShaderModule VulkanContext::createShaderModule(const std::vector<char> &code)
	{
		/*
			The function takes a buffer with the bytecode as parameter and create a VkShaderModule from it.
		*/

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module!");

		return shaderModule;
	}



	/******************** Render passes ********************/

	void VulkanContext::createRenderPass()
	{
		/*
			A render pass represents a collection of attachments, subpasses, and dependencies between the subpasses,
			and describes how the attachments are used over the course of the subpasses.
		*/

		//////////////////// Attachment description
		VkAttachmentDescription colorAttachment = {};

		colorAttachment.format = this->swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		// The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering. 
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// The initialLayout specifies which layout the image will have before the render pass begins. 
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// The finalLayout specifies the layout to automatically transition to when the render pass finishes. 
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		//////////////////// Subpasses and attachment references.

		// Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes, 
		// for example a sequence of post-processing effects that are applied one after another. 

		VkAttachmentReference colorAttachmentRef = {};

		// The attachment parameter specifies which attachment to reference by its index in the attachment descriptions array. 
		colorAttachmentRef.attachment = 0;

		// The layout specifies which layout we would like the attachment to have during a subpass that uses this reference.
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass = {};

		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;


		//////////////////// Subpass dependencies
		VkSubpassDependency dependency = {};

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//////////////////// Render pass
		VkRenderPassCreateInfo renderPassInfo = {};

		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");
	}



	/******************** Framebuffers ********************/

	void VulkanContext::createFramebuffers()
	{
		/*
			A framebuffer object references all of the VkImageView objects that represent the attachments.
		*/

		// resize the container to hold all of the framebuffers
		this->swapChainFramebuffers.resize(this->swapChainImageViews.size());

		for (size_t i = 0; i < this->swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				this->swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};

			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = this->renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = this->swapChainExtent.width;
			framebufferInfo.height = this->swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}



	/******************** Command pools ********************/

	void VulkanContext::createCommandPool()
	{
		/*
			Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
		*/

		QueueFamilyIndices queueFamilyIndices = this->findQueueFamilies(this->physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};

		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0; // optional

		if (vkCreateCommandPool(this->device, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");
	}

	void VulkanContext::createCommandBuffers()
	{
		/*
			Command buffers are objects used to record commands which can be subsequently submitted to a device queue for execution.
		*/

		this->commandBuffers.resize(this->swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)this->commandBuffers.size();


		if (vkAllocateCommandBuffers(this->device, &allocInfo, this->commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to create command buffers!");

		//////////////////// Starting command buffer recording

		for (size_t i = 0; i < this->commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};

			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(this->commandBuffers[i], &beginInfo) != VK_SUCCESS)
				throw std::runtime_error("failed to begin recording command buffer!");


			//////////////////// Starting a render pass
			VkRenderPassBeginInfo renderPassInfo = {};

			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = this->renderPass;
			renderPassInfo.framebuffer = this->swapChainFramebuffers[i];
			// The render area defines where shader loads and stores will take place.
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = this->swapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(this->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(this->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

			// Binding the vertex buffer
			VkBuffer vertexBuffers[] = { this->vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(this->commandBuffers[i], 0, 1, vertexBuffers, offsets);

			// Using an index buffer
			vkCmdBindIndexBuffer(this->commandBuffers[i], this->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(this->commandBuffers[i], static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(this->commandBuffers[i]);

			if (vkEndCommandBuffer(this->commandBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to record command buffer!");
		}
	}



	/******************** Drawing ********************/

	void VulkanContext::drawFrame()
	{
		/*
			- Acquire an image from the swap chain
			- Execute the command buffer with that image as attachment in the framebuffer
			- Return the image to the swap chain for presentation
		*/

		// The vkWaitForFences function takes an array of fences and waits for either any or all of them to be signaled before returning.
		vkWaitForFences(this->device, 1, &this->inFlightFences[this->currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		//////////////////// Acquire an image from the swap chain

		uint32_t imageIndex;
		// Using the maximum value of a 64 bit unsigned integer disables the timeout.
		VkResult result = vkAcquireNextImageKHR(this->device, this->swapChain, std::numeric_limits<uint64_t>::max(), this->imageAvailableSemaphores[this->currentFrame], VK_NULL_HANDLE, &imageIndex);

		// Now we just need to figure out when swap chain recreation is necessary and call our new recreateSwapChain function. 
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			this->recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//////////////////// Queue submission and synchronization is configured through parameters in the VkSubmitInfo structure.
		VkSubmitInfo submitInfo = {};

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &this->commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Unlike the semaphores, we manually need to restore the fence to the unsignaled state by resetting it with the vkResetFences call.
		vkResetFences(this->device, 1, &this->inFlightFences[this->currentFrame]);

		if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFences[this->currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		//////////////////// Presentation
		VkPresentInfoKHR presentInfo = {};

		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { this->swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(this->presentQueue, &presentInfo);


		// check if window has been resized
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->window->getFramebufferResizeState())
		{
			this->window->setFramebufferResizeState(false);
			this->recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		vkQueueWaitIdle(this->presentQueue);

		this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanContext::createSyncObjects()
	{
		/*
			Semaphores are used to coordinate queue operations both within a queue and between different queues.
			A semaphore’s status is always either signaled or unsignaled.
		*/

		this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};

		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		//////////////////// To perform CPU-GPU synchronization, Vulkan offers a second type of synchronization primitive called fences. 
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(this->device, &fenceInfo, nullptr, &this->inFlightFences[i]))
				throw std::runtime_error("failed to create semaphores for a frame!");
		}
	}



	/******************** Vertex buffer creation ********************/

	void VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
	{
		// Buffer creation
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(this->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create vertex buffer!");


		// Memory requirements
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(this->device, buffer, &memRequirements);


		// Memory allocation
		VkMemoryAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = this->findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(this->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate vertex buffer memory!");


		// Associate memory with buffer
		vkBindBufferMemory(this->device, buffer, bufferMemory, 0);
	}

	void VulkanContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		/*
			copy the contents from one buffer to another
		*/

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(this->device, &allocInfo, &commandBuffer);

		// start recording the command buffer
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		// Contents of buffers are transferred using the vkCmdCopyBuffer
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		// execute the command buffer to complete the transfer
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(this->graphicsQueue);

		vkFreeCommandBuffers(this->device, this->commandPool, 1, &commandBuffer);
	}

	void VulkanContext::createVertexBuffer()
	{
		/*
			A vertex buffer is a CPU-visible and GPU-visible buffer that contains the vertex data that describes the geometry of the object(s) you wish to render.
			In general, the vertex data consists of position (x,y,z) data and the optional color, normal, or other information.
		*/

		VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);


		// Filling the vertex buffer
		void *data;
		vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, this->vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(this->device, stagingBufferMemory);

		this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory);

		// move the vertex data to the device local buffer
		this->copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

		vkDestroyBuffer(this->device, stagingBuffer, nullptr);
		vkFreeMemory(this->device, stagingBufferMemory, nullptr);
	}

	uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("failed to find suitable memory type!");

		return uint32_t();
	}

	void VulkanContext::createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(this->indices[0]) * this->indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, this->indices.data(), (size_t)bufferSize);
		vkUnmapMemory(this->device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->indexBuffer, this->indexBufferMemory);

		copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

		vkDestroyBuffer(this->device, stagingBuffer, nullptr);
		vkFreeMemory(this->device, stagingBufferMemory, nullptr);
	}


}