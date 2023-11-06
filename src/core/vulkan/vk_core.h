#pragma once
#include "../../../ext/vk-bootstrap/src/VkBootstrap.h"
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include <cstring>
#include <stdexcept>

#include "shaders.h"

//#define SKIP_VULKAN_CHECKS

inline void vk_check(VkResult r, std::string s)
{
#ifndef SKIP_VULKAN_CHECKS
	if(r != VK_SUCCESS)
		throw std::runtime_error(s);
#endif
};

struct vk_ctx
{
	vkb::Instance inst;
	vkb::Device dev;
	vkb::PhysicalDevice pdev;
};

inline uint32_t get_mem_type_idx(
	VkPhysicalDevice &pdev,
	VkMemoryRequirements mr,
	VkMemoryPropertyFlags bits
)
{
	VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(pdev, &mem_props);

	uint32_t ret = -1;
	for(uint32_t x = 0; x < mem_props.memoryTypeCount; x++)
	{
		if(
			(mr.memoryTypeBits & (1 << x)) && 
			(mem_props.memoryTypes[x].propertyFlags & bits) == bits
		)
		{
			ret = x;
			break;
		}
	}

	return ret;
};

struct buffer
{
	vk_ctx* ctx = nullptr;
	void* dst = nullptr;
	VkBuffer handle;
	VkDeviceMemory dev_mem;
	size_t size;
	bool allocated = false;
	bool mapped = false;

	buffer() {};

	void create(
		vk_ctx* c, size_t mem_size,
		VkBufferUsageFlags use_flags,
		VkMemoryPropertyFlags mem_flags
	)
	{
		ctx = c; size = mem_size;
		VkBufferCreateInfo bci =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			//.flags = 0,
			.size = mem_size,
			.usage = use_flags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			/*.queueFamilyIndexCount = (uint32_t)-1,
			.pQueueFamilyIndices = nullptr*/
		};

		VkResult result;
		result = vkCreateBuffer(ctx->dev.device,&bci,nullptr,&handle);
		vk_check(result,"Failed to create buffer!\n");
	
		VkMemoryRequirements memreqs;
		vkGetBufferMemoryRequirements(ctx->dev.device,handle,&memreqs);

		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

		VkMemoryAllocateInfo mem_alloc_info =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &memoryAllocateFlagsInfo,
			.allocationSize = memreqs.size,
			.memoryTypeIndex = get_mem_type_idx(
				ctx->pdev.physical_device,memreqs,mem_flags
			)
		};

		result = vkAllocateMemory(ctx->dev.device,&mem_alloc_info,nullptr,&dev_mem);
		vk_check(result,"Failled to allocate memory!\n");

		result = vkBindBufferMemory(ctx->dev.device,handle,dev_mem,0);
		vk_check(result,"Failed to bind buffery memory!\n");

		allocated = true;
	};

	void map()
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Buffer not allocated!\n");
#endif
		VkResult result = vkMapMemory(ctx->dev.device,dev_mem,0,size,0,&dst);
		vk_check(result,"Failed to map memory!\n");
		mapped = true;
	};

	void unmap()
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Buffer not allocated!\n");
		if(!mapped)
			throw std::runtime_error("Cannot unmap, memory was not mapped!\n");
#endif
		vkUnmapMemory(ctx->dev.device,dev_mem);
		mapped = false;
	};

	void copy_to_device(void* data)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Buffer not allocated!\n");
		if(!mapped)
			throw std::runtime_error("Cannot copy while not mapped!\n");
#endif
		memcpy(dst,data,size);
	};

	void copy_from_device(void* data)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Buffer not allocated!\n");
		if(!mapped)
			throw std::runtime_error("Cannot copy while not mapped!\n");
#endif
		memcpy(data,dst,size);
	};

	VkDeviceAddress get_device_address()
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Buffer not allocated!\n");
#endif
		VkBufferDeviceAddressInfoKHR bdai =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = handle
		};

		return reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(
			vkGetDeviceProcAddr(ctx->dev.device,"vkGetBufferDeviceAddressKHR")
		)(ctx->dev.device,&bdai);
	};

	void destroy()
	{
		if(!allocated)
			return;
		if(mapped)
			unmap();

		vkFreeMemory(ctx->dev.device,dev_mem,nullptr);
		vkDestroyBuffer(ctx->dev.device,handle,nullptr);
	};

	~buffer() {};
};

struct command_buffer
{
	vk_ctx* ctx = nullptr;
	VkCommandBuffer handle = VK_NULL_HANDLE;
	VkCommandPool cmd_pool = VK_NULL_HANDLE;
	bool recording = false;
	bool allocated = false;
	std::string m_name;

	command_buffer() {};

	void create(vk_ctx* c, VkCommandPool cmd_pool_, std::string name)
	{
		ctx = c; cmd_pool = cmd_pool_; m_name = name;
		VkCommandBufferAllocateInfo ai =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = cmd_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VkResult result = vkAllocateCommandBuffers(ctx->dev,&ai,&handle);
		vk_check(result,"Failed to allocate command buffer!\n");
		allocated = true;
	};

	void begin(VkCommandBufferUsageFlags use_flags)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Not allocated, cannot start!\n");
		if(recording)
			throw std::runtime_error("Already recording, cannot start!\n");
#endif
		VkCommandBufferBeginInfo bi =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = use_flags
		};

		VkResult result = vkBeginCommandBuffer(handle,&bi);
		vk_check(result,"Failed to begin command buffer!\n");

		recording = true;
	};

	void submit(VkQueue queue, bool fence)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Not allocated, cannot submit!\n");
		if(!recording)
			throw std::runtime_error("Command buffer not started!\n");
#endif
		VkResult result = vkEndCommandBuffer(handle);
		vk_check(result,"Failed to end command buffer!\n");
		
		VkSubmitInfo si =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &handle
		};

		if(fence) //create fence to wait for command return
		{
			VkFenceCreateInfo fci =
			{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = 0
			};

			VkFence f;

			result = vkCreateFence(ctx->dev.device,&fci,nullptr,&f);
			vk_check(result,"Failed to create fence!\n");

			result = vkQueueSubmit(queue,1,&si,f);
			vk_check(result,"Failed to submit to queue!\n");

			result = vkWaitForFences(ctx->dev.device,1,&f,VK_TRUE,100000000000);
			vk_check(result,"Failed to wait for fence!\n");

			vkDestroyFence(ctx->dev.device,f,nullptr);
		}
		else
		{
			result = vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);
			vk_check(result,"Failed to submit to queue!\n");
		}

		printf("Submitted: %s\n",m_name.c_str());
		recording = false;
	};

	void submit(VkQueue queue, VkFence fence)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Not allocated, cannot submit!\n");
		if(!recording)
			throw std::runtime_error("Command buffer not started!\n");
#endif
		VkResult result = vkEndCommandBuffer(handle);
		vk_check(result,"Failed to end command buffer!\n");

		VkSubmitInfo si =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &handle
		};

		result = vkQueueSubmit(queue,1,&si,fence);
		vk_check(result,"Failed to submit to queue!\n");

		result = vkWaitForFences(ctx->dev.device,1,&fence,VK_TRUE,100000000000);
		vk_check(result,"Failed to wait for fence!\n");

		recording = false;
	};

	void reset(VkCommandBufferResetFlags flags)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(!allocated)
			throw std::runtime_error("Not allocated, cannot reset!\n");
		if(recording)
			throw std::runtime_error("Cannot reset while recording!\n");
#endif
		VkResult result = vkResetCommandBuffer(handle,flags);
		vk_check(result,"Failed to reset command buffer!\n");
	};

	~command_buffer()
	{
		if(handle == VK_NULL_HANDLE)
			return;

		if(recording == true)
		{
			if(vkEndCommandBuffer(handle) != VK_SUCCESS)
			{
				printf("%s was not submitted!\n",m_name.c_str());
				throw std::runtime_error("Command buffer is still recording!\n");
			}
		}
		
		vkFreeCommandBuffers(ctx->dev.device,cmd_pool,1,&handle);
	};
};

struct accel_struct
{
	VkAccelerationStructureKHR accel = VK_NULL_HANDLE;
	buffer buff;
};

inline void create_shader_module(vk_ctx* c, std::string path, VkShaderModule* s)
{
	std::vector<uint32_t> shader_code = read_shader_file(path);

	VkShaderModuleCreateInfo sci =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = shader_code.size() * sizeof(uint32_t),
		.pCode = shader_code.data()
	};

	VkResult result = vkCreateShaderModule(c->dev.device,&sci,nullptr,s);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!\n");
};

inline void create_compute_pipeline(
	vk_ctx* c, VkShaderModule shader_module, std::string entry_name,
	std::vector<VkDescriptorSetLayout> dsls,
	VkPipeline* pipeline, VkPipelineLayout* layout
)
{
	VkPipelineShaderStageCreateInfo plssci =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = shader_module,
		.pName = entry_name.c_str(),
		.pSpecializationInfo = nullptr
	};

	VkPipelineLayoutCreateInfo pllci =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = dsls.size(),
		.pSetLayouts = dsls.data(),
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	};

	VkResult result = vkCreatePipelineLayout(c->dev.device,&pllci,nullptr,layout);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create compute pipeline layout!\n");

	VkComputePipelineCreateInfo cplci =
	{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = plssci,
		.layout = *layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	result = vkCreateComputePipelines(c->dev.device,VK_NULL_HANDLE,1,&cplci,nullptr,pipeline);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create compute pipeline!\n");
};

struct descriptor_set
{
	vk_ctx* m_ctx = nullptr;
	VkDescriptorSet handle;
	uint32_t binding_index = 0;
	std::vector<VkWriteDescriptorSet> wds_list = {};

	descriptor_set() {};

	void allocate(
		vk_ctx* ctx, VkDescriptorPool desc_pool,
		VkDescriptorSetLayout* layouts, uint32_t layout_count)
	{
		m_ctx = ctx;

		VkDescriptorSetAllocateInfo ai =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = desc_pool,
			.descriptorSetCount = layout_count,
			.pSetLayouts = layouts,
		};

		VkResult result = vkAllocateDescriptorSets(m_ctx->dev,&ai,&handle);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!\n");
	};

	void write_descriptor_set(
		void* pnext, int32_t count,
		VkDescriptorType type,
		VkDescriptorBufferInfo* p_buffer_info
	)
	{
		VkWriteDescriptorSet wds =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = pnext,
			.dstSet = handle,
			.dstBinding = binding_index,
			.dstArrayElement = 0,
			.descriptorCount = count,
			.descriptorType = type,
			.pBufferInfo = p_buffer_info
		};
		binding_index++;
		wds_list.push_back(wds);
		vkUpdateDescriptorSets(m_ctx->dev,(uint32_t)wds_list.size(),wds_list.data(),0,nullptr);
	};
};

struct descriptor_pool
{
	VkDescriptorPool handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorPoolSize> sizes = {};

	void add_size(VkDescriptorType type, uint32_t count) {sizes.push_back({type,count});};

	void create(VkDevice dev, uint32_t max_sets = 8192, VkDescriptorPoolCreateFlags flags = 0)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(sizes.size() == 0)
			throw std::runtime_error("No sizes for descriptor pool added!\n");
#endif
		VkDescriptorPoolCreateInfo dpci =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags,
			.maxSets = max_sets,
			.poolSizeCount = sizes.size(),
			.pPoolSizes = sizes.data()
		};

		VkResult result = vkCreateDescriptorPool(dev,&dpci,nullptr,&handle);
		vk_check(result,"Failed to create descriptor pool!\n");
	};
};

struct descriptor_set_layout
{
	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> bindings = {};

	void add_binding(VkDescriptorType type, uint32_t count, VkShaderStageFlags flags)
	{
		VkDescriptorSetLayoutBinding dslb =
		{
			.binding = bindings.size(),
			.descriptorType = type,
			.descriptorCount = count,
			.stageFlags = flags,
			.pImmutableSamplers = nullptr
		};

		bindings.push_back(dslb);
	};

	void create(VkDevice dev, void* pnext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0)
	{
#ifndef SKIP_VULKAN_CHECKS
		if(bindings.size() == 0)
			throw std::runtime_error("No bindings for descriptor set layout added!\n");
#endif
		VkDescriptorSetLayoutCreateInfo dslci =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data()
		};

		VkResult result = vkCreateDescriptorSetLayout(dev,&dslci,nullptr,&handle);
		vk_check(result,"Failed to create descriptor set layout!\n");
	};
};