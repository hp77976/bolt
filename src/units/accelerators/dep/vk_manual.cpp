#include "vk.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
)
{
	if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
};

void allocate_bind_buffer(
	VkDevice &dev, VkPhysicalDevice &pdev, VkBuffer &buff, VkDeviceMemory &mem,
	uint32_t mem_size, VkBufferUsageFlags use_flags, VkMemoryPropertyFlags mem_flags
)
{
	VkBufferCreateInfo bci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.size = mem_size,
		.usage = use_flags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = (uint32_t)-1,
		.pQueueFamilyIndices = nullptr
	};

	if(vkCreateBuffer(dev,&bci,nullptr,&buff)!=VK_SUCCESS)
		throw std::runtime_error("Failed to create buffer!\n");
	
	VkMemoryRequirements memreqs;
	vkGetBufferMemoryRequirements(dev,buff,&memreqs);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo mem_alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &memoryAllocateFlagsInfo,
		.allocationSize = memreqs.size,
		.memoryTypeIndex = get_mem_type_idx(pdev,memreqs,mem_flags)
	};

	if(vkAllocateMemory(dev,&mem_alloc_info,nullptr,&mem)!=VK_SUCCESS)
		throw std::runtime_error("Failled to allocate memory!\n");

	if(vkBindBufferMemory(dev,buff,mem,0)!=VK_SUCCESS)
		throw std::runtime_error("Failed to bind buffery memory!\n");
};

VkDeviceAddress getDeviceAddress(VkDevice logicalDevice, VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR buff_dev_addr_info{};
	buff_dev_addr_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	buff_dev_addr_info.buffer = buffer;
	return reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(
		vkGetDeviceProcAddr(logicalDevice, "vkGetBufferDeviceAddressKHR")
	)(logicalDevice, &buff_dev_addr_info);
};

std::vector<const char*> deviceExtensions = {
	"VK_KHR_swapchain",
	"VK_KHR_ray_query",
	"VK_KHR_acceleration_structure",
	"VK_KHR_spirv_1_4",
	"VK_EXT_descriptor_indexing",
	"VK_KHR_buffer_device_address",
	"VK_KHR_deferred_host_operations",
	"VK_KHR_maintenance4",
};

vk_accelerator::vk_accelerator(scene* s) : accelerator(s)
{
	printf("Selected Vulkan Accelerator\n");
	
	uint64_t tri_count = m_tris->size();
	printf("Tri count: %lu\n",tri_count);
	
	//printf("Vert count: %lu\n",tri_count*3);
	//printf("Index count: %lu\n",tri_count*3);

	//vertices.resize(tri_count*3);
	//indices.resize(tri_count*3);

	ubo  = new vec3f[job_size.x*job_size.y*job_size.z];
	ubo2 = new vec3f[job_size.x*job_size.y*job_size.z];

	for(uint32_t i = 0; i < m_tris->size(); i++)
		for(uint32_t j = 0; j < 3; j++)
			vertices.push_back(m_tris->at(i)[j].p);

	for(uint32_t i = 0; i < vertices.size(); i++)
		indices.push_back(i);

	assemble_everything(); //loads and initializes all of the VK stuff

	mgr = kp::Manager(
		std::make_shared<vk::Instance>(vk::Instance(inst.instance)),
		std::make_shared<vk::PhysicalDevice>(vk::PhysicalDevice(pdev.physical_device)),
		std::make_shared<vk::Device>(vk::Device(dev.device))
	);
};

void vk_accelerator::assemble_everything()
{
	int32_t sum = job_size.x * job_size.y * job_size.z;

	{ //build the vk instance
		vkb::InstanceBuilder inst_builder;
		inst_builder.require_api_version(VK_API_VERSION_1_3);
		inst_builder.enable_validation_layers().set_headless();
		inst_builder.enable_extension(VK_KHR_SURFACE_EXTENSION_NAME);
		auto ib_ret = inst_builder.build();
		if(!ib_ret)
			throw std::runtime_error("Failed to create instance!\n");
		else
			inst = ib_ret.value();
	}

	{ //build the physical device
		vkb::PhysicalDeviceSelector pdev_selector(inst);
		pdev_selector.add_required_extensions(deviceExtensions);

		//all of these features need to be manually defined and enabled
		VkPhysicalDeviceRayQueryFeaturesKHR feat =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
			.pNext = nullptr,
			.rayQuery = VK_TRUE
		};
		VkPhysicalDeviceMaintenance4FeaturesKHR mainentence =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR,
			.pNext = nullptr,
			.maintenance4 = VK_TRUE
		};
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR baf2 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
			.pNext = nullptr,
			.bufferDeviceAddress = VK_TRUE
		};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR asf =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
			.pNext = nullptr,
			.accelerationStructure = VK_TRUE
		};
		pdev_selector.add_required_extension_features(feat);
		pdev_selector.add_required_extension_features(mainentence);
		pdev_selector.add_required_extension_features(baf2);
		pdev_selector.add_required_extension_features(asf);
		//all of these features need to be manually defined and enabled

		pdev_selector.set_minimum_version(1,3);
		auto pds_ret = pdev_selector.select();
		if(!pds_ret)
			throw std::runtime_error("Failed to select physical device!\n");
		else
			pdev = pds_ret.value();
	}

	{ //build the logical device
		vkb::DeviceBuilder dev_builder(pdev);
		auto dev_builder_ret = dev_builder.build();
		if(!dev_builder_ret)
			throw std::runtime_error("Failed to build logical device!\n");
		else
			dev = dev_builder_ret.value();
	}

	m_ctx.inst = inst;
	m_ctx.dev = dev;
	m_ctx.pdev = pdev;

	{ //add debug messenger support
		VkDebugUtilsMessengerCreateInfoEXT debug_util_ci =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags = 0,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
			.pUserData = nullptr
		};

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				inst,"vkCreateDebugUtilsMessengerEXT"
			);
	}

	{ //create compute and graphics queues
		//retrieves which queues are available
		uint32_t queue_family_count = -1;
		vkGetPhysicalDeviceQueueFamilyProperties(pdev,&queue_family_count,nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(pdev,&queue_family_count,queue_families.data());

		for(int i = 0; i < (int)queue_families.size(); i++)
		{
			if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				compute_family_index = i;
			if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphics_family_index = i;
			if(compute_family_index != -1 && graphics_family_index != -1)
				break;
		}

		//vkGetDeviceQueue(dev,graphics_family_index,0,&graphics_queue);
		vkGetDeviceQueue(dev,compute_family_index,0,&compute_queue);
	}

	{ //create and define descriptor set layouts
		int binding_count = 3;
		VkDescriptorSetLayoutBinding layout_bindings[3];

		layout_bindings[0] =
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = nullptr
		};

		layout_bindings[1] =
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = nullptr
		};

		layout_bindings[2] =
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutCreateInfo layout_ci =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.bindingCount = binding_count,
			.pBindings = layout_bindings
		};

		result = vkCreateDescriptorSetLayout(dev,&layout_ci,nullptr,&descriptor_set_layout);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor set layout!\n");
	}

	{ //load the compute shader module
		std::vector<uint32_t> shader_code = read_shader_file(
			"/home/batman/Github/vkrt/src/shaders/ray.spv"
		);

		shader_code_raw = read_shader_file("/home/batman/Github/vkrt/src/shaders/ray.spv");

		VkShaderModuleCreateInfo shader_ci =
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = shader_code.size() * sizeof(uint32_t),
			.pCode = shader_code.data()
		};

		result = vkCreateShaderModule(dev,&shader_ci,nullptr,&compute_shader_module);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create compute shader module!\n");
	}

	{ //create the compute pipeline
		VkPipelineShaderStageCreateInfo comp_shader_stage_info =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = compute_shader_module,
			.pName = "main",
			.pSpecializationInfo = nullptr
		};

		VkPipelineLayoutCreateInfo pllci =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptor_set_layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};

		result = vkCreatePipelineLayout(dev,&pllci,nullptr,&compute_pipeline_layout);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create compute pipeline layout!\n");

		VkComputePipelineCreateInfo cplci =
		{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = comp_shader_stage_info,
			.layout = compute_pipeline_layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		result = vkCreateComputePipelines(dev,VK_NULL_HANDLE,1,&cplci,nullptr,&compute_pipeline);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create compute pipeline!\n");
	}

	{ //create command pools
		VkCommandPoolCreateInfo command_pool_create_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = compute_family_index,
		};

		result = vkCreateCommandPool(dev,&command_pool_create_info,nullptr,&command_pool);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create command pool!\n");
	}

	/*{ //create compute command buffer
		VkCommandBufferAllocateInfo alloc_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		result = vkAllocateCommandBuffers(dev,&alloc_info,&compute_cmd_buff);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate compute command buffer!\n");
	}*/

	{ //descriptor pool
		int pool_size_count = 3;
		VkDescriptorPoolSize pools_size[3];
		uint32_t swap_chain_image_count = 1;
		
		pools_size[0] =
		{
			.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			.descriptorCount = swap_chain_image_count
		};

		pools_size[1] =
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = swap_chain_image_count
		};

		pools_size[2] =
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = swap_chain_image_count
		};

		VkDescriptorPoolCreateInfo desc_pool_create_info =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.maxSets = swap_chain_image_count,
			.poolSizeCount = pool_size_count,
			.pPoolSizes = pools_size
		};

		result = vkCreateDescriptorPool(dev,&desc_pool_create_info,nullptr,&descriptor_pool);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create descriptor pool!\n");
	}

	{ //create vertex buffer and index buffer

		vertex_buffer.create(
			&m_ctx,vertices.size()*sizeof(vec3f),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vertex_buffer.map();
		vertex_buffer.copy_to_device(vertices.data());

		index_buffer.create(
			&m_ctx,indices.size()*sizeof(uint32_t),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		index_buffer.map();
		index_buffer.copy_to_device(indices.data());

		/*uint32_t vert_size = vertices.size()*sizeof(vertex);
		allocate_bind_buffer(
			dev.device,pdev.physical_device,vertex_buffer,vertex_memory,vert_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		result = vkMapMemory(dev,vertex_memory,0,vert_size,0,&vertex_data);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to map vertex memory!\n");
		memcpy(vertex_data,vertices.data(),vert_size);
		//vkUnmapMemory(dev,vertex_memory);

		uint32_t idx_size = indices.size()*sizeof(uint32_t);
		allocate_bind_buffer(
			dev.device,pdev.physical_device,index_buffer,index_memory,idx_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		result = vkMapMemory(dev,index_memory,0,idx_size,0,&index_data);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to map index memory!\n");
		memcpy(index_data,indices.data(),idx_size);*/
		//vkUnmapMemory(dev,index_memory);
	}

	/*{ //create blas
		VkAccelerationStructureGeometryTrianglesDataKHR tri_data =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexStride = sizeof(vertex),
			.maxVertex = (uint32_t)vertices.size(),
			.indexType = VK_INDEX_TYPE_UINT32
		};
		tri_data.transformData.deviceAddress = 0;
		tri_data.transformData.hostAddress = nullptr;
		tri_data.vertexData.deviceAddress = getDeviceAddress(dev,vertex_buffer);
		tri_data.indexData.deviceAddress = getDeviceAddress(dev,index_buffer);

		VkAccelerationStructureGeometryDataKHR geo_data = {.triangles = tri_data};

		VkAccelerationStructureGeometryKHR accel_geo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
			.geometry = geo_data,
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		};

		VkAccelerationStructureBuildGeometryInfoKHR accel_build_geo_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.geometryCount = 1,
			.pGeometries = &accel_geo,
		};

		VkAccelerationStructureBuildSizesInfoKHR accel_build_size_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
		};

		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = 
		reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
			vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureBuildSizesKHR")
		);

		uint32_t triangle_count = vertices.size() / 3;
		vkGetAccelerationStructureBuildSizesKHR(
			dev,VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accel_build_geo_info,&triangle_count,
			&accel_build_size_info
		);

		allocate_bind_buffer(
			dev.device,pdev.physical_device,blas_buffer,blas_memory,
			accel_build_size_info.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | 
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

		allocate_bind_buffer(
			dev.device,pdev.physical_device,blas_scratch_buffer,blas_scratch_memory,
			accel_build_size_info.buildScratchSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

		VkAccelerationStructureCreateInfoKHR accel_ci =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.buffer = blas_buffer,
			.size = accel_build_size_info.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
		};

		auto vkCreateAccelerationStructureKHR = 
		reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
			vkGetDeviceProcAddr(dev.device, "vkCreateAccelerationStructureKHR")
		);

		result = vkCreateAccelerationStructureKHR(dev,&accel_ci,nullptr,&blas_as);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create blas acceleration structure!\n");

		VkAccelerationStructureDeviceAddressInfoKHR accel_dev_addr_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = blas_as
		};

		auto vkGetAccelerationStructureDeviceAddressKHR = 
		reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
			vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureDeviceAddressKHR")
		);
		blas_dev_addr = vkGetAccelerationStructureDeviceAddressKHR(dev,&accel_dev_addr_info);

		auto blas_scratch_addr = getDeviceAddress(dev,blas_scratch_buffer);
		VkAccelerationStructureBuildGeometryInfoKHR accel_build_geo_info2 =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
    		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
    		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
    		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
	    	.dstAccelerationStructure = blas_as,
    		.geometryCount = 1,
    		.pGeometries = &accel_geo,
    		.scratchData = {.deviceAddress = blas_scratch_addr}
		};

		VkAccelerationStructureBuildRangeInfoKHR accel_build_range_info =
		{
			.primitiveCount = triangle_count,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0
		};

		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accel_build_range_info_list =
			{&accel_build_range_info};
		
		VkCommandBufferAllocateInfo command_buffer_allocate_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		result = vkAllocateCommandBuffers(dev,&command_buffer_allocate_info,&cmd_buffer);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers!\n");
		
		VkCommandBufferBeginInfo command_buffer_begin_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		result = vkBeginCommandBuffer(cmd_buffer,&command_buffer_begin_info);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to begin command buffer!\n");

		auto vkCmdBuildAccelerationStructuresKHR = 
		reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
			vkGetDeviceProcAddr(dev.device, "vkCmdBuildAccelerationStructuresKHR")
		);

		vkCmdBuildAccelerationStructuresKHR(
			cmd_buffer,1,&accel_build_geo_info2,accel_build_range_info_list.data()
		);

		result = vkEndCommandBuffer(cmd_buffer);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to end command buffer!\n");

		VkSubmitInfo submit_info =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer
		};

		VkFenceCreateInfo blas_fence_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = 0
		};

		VkFence blas_fence;
		result = vkCreateFence(dev,&blas_fence_info,nullptr,&blas_fence);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to create blas fence!\n");

		result = vkQueueSubmit(compute_queue,1,&submit_info,blas_fence);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to submit to queue!\n");

		result = vkWaitForFences(dev,1,&blas_fence,VK_TRUE,100000000000);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for blas fence!\n");

		vkDestroyFence(dev,blas_fence,nullptr);
		vkFreeCommandBuffers(dev,command_pool,1,&cmd_buffer);
	}*/
	
	/*{ //create tlas
		VkTransformMatrixKHR tx_matrix = {
			1.f,0.f,0.f,0.f,
			0.f,1.f,0.f,0.f,
			0.f,0.f,1.f,0.f
		};

		VkAccelerationStructureInstanceKHR tlas_accel_inst =
		{
			.transform = tx_matrix,
			.instanceCustomIndex = 0,
			.mask = 0xFFFF,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = blas_dev_addr
		};

		VkBuffer inst_buffer;
		VkDeviceMemory inst_dev_memory;
		void* inst_host_mem;

		allocate_bind_buffer(
			dev.device,pdev.physical_device,inst_buffer,inst_dev_memory,
			sizeof(VkAccelerationStructureInstanceKHR),
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		result = vkMapMemory(
			dev,inst_dev_memory,0,sizeof(VkAccelerationStructureInstanceKHR),0,&inst_host_mem
		);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to map tlas instance memory!\n");

		memcpy(inst_host_mem,&tlas_accel_inst,sizeof(VkAccelerationStructureInstanceKHR));

		VkDeviceOrHostAddressConstKHR inst_data_dev_addr =
		{
			.deviceAddress = getDeviceAddress(dev.device,inst_buffer)
		};

		VkAccelerationStructureGeometryKHR tlas_accel_geo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry =
			{
				.instances =
				{
					.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
					.arrayOfPointers = VK_FALSE,
					.data = inst_data_dev_addr
				}
			},
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
		};

		VkAccelerationStructureBuildGeometryInfoKHR tlas_accel_build_geo_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.geometryCount = 1,
			.pGeometries = &tlas_accel_geo
		};

		uint32_t prim_count = 1;

		VkAccelerationStructureBuildSizesInfoKHR tlas_build_sizes_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
		};

		auto vkGetAccelerationStructureBuildSizesKHR =
		reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
			vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureBuildSizesKHR")
		);

		vkGetAccelerationStructureBuildSizesKHR(
			dev,VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&tlas_accel_build_geo_info,&prim_count,&tlas_build_sizes_info
		);

		VkBuffer tlas_buffer;
		VkDeviceMemory tlas_dev_memory;
		allocate_bind_buffer(
			dev.device,pdev.physical_device,tlas_buffer,
			tlas_dev_memory,tlas_build_sizes_info.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		); 

		VkBuffer tlas_scratch_buffer;
		VkDeviceMemory tlas_scratch_dev_memory;
		allocate_bind_buffer(
			dev.device,pdev.physical_device,tlas_scratch_buffer,
			tlas_scratch_dev_memory,tlas_build_sizes_info.buildScratchSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

		VkAccelerationStructureCreateInfoKHR tlas_create_info =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.buffer = tlas_buffer,
			.size = tlas_build_sizes_info.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		};

		auto vkCreateAccelerationStructureKHR =
		reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
			vkGetDeviceProcAddr(dev.device, "vkCreateAccelerationStructureKHR")
		);
		vkCreateAccelerationStructureKHR(dev,&tlas_create_info,nullptr,&tlas_as);

		VkAccelerationStructureBuildGeometryInfoKHR tlas_accel_build_geo_info2 =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.dstAccelerationStructure = tlas_as,
			.geometryCount = 1,
			.pGeometries = &tlas_accel_geo,
			.scratchData = {.deviceAddress = getDeviceAddress(dev,tlas_scratch_buffer)}
		};

		VkAccelerationStructureBuildRangeInfoKHR tlas_build_range_info =
		{
			.primitiveCount = prim_count,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0
		};
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> tlas_build_range_infos;
		tlas_build_range_infos.push_back(&tlas_build_range_info);

		VkCommandBufferAllocateInfo tlas_cmd_buff_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VkCommandBuffer tlas_cmd_buff;
		result = vkAllocateCommandBuffers(dev,&tlas_cmd_buff_info,&tlas_cmd_buff);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allcate tlas command buffer!\n");

		VkCommandBufferBeginInfo tlas_cmd_buff_begin_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};
		result = vkBeginCommandBuffer(tlas_cmd_buff,&tlas_cmd_buff_begin_info);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to begin tlas command buffer!\n");

		auto vkCmdBuildAccelerationStructuresKHR =
		reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
			vkGetDeviceProcAddr(dev.device, "vkCmdBuildAccelerationStructuresKHR")
		);

		vkCmdBuildAccelerationStructuresKHR(
			tlas_cmd_buff,1,&tlas_accel_build_geo_info2,tlas_build_range_infos.data()
		);

		result = vkEndCommandBuffer(tlas_cmd_buff);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to end tlas command buffer!\n");
		
		VkSubmitInfo tlas_submit_info =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &tlas_cmd_buff
		};

		VkFenceCreateInfo tlas_fence_info =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = 0
		};

		VkFence tlas_fence;
		result = vkCreateFence(dev,&tlas_fence_info,nullptr,&tlas_fence);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate tlas fence!\n");

		result = vkQueueSubmit(compute_queue,1,&tlas_submit_info,tlas_fence);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to submit tlas info to queue!\n");
		
		result = vkWaitForFences(dev,1,&tlas_fence,VK_TRUE,100000000000);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for tlas fence!\n");
		
		vkDestroyFence(dev,tlas_fence,nullptr);
		vkFreeCommandBuffers(dev,command_pool,1,&tlas_cmd_buff);
	}*/

	cmd_create_blas();

	cmd_create_tlas();

	//TODO: this will need to be changed to a storage buffer
	{ //uniform buffer
		uni_buff1.create(
			&m_ctx,sizeof(vec3f)*sum,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		uni_buff1.map();
		uni_buff1.copy_to_device(&ubo[0]);

		uni_buff2.create(
			&m_ctx,sizeof(vec3f)*sum,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		uni_buff2.map();
		uni_buff2.copy_to_device(&ubo2[0]);

		/*allocate_bind_buffer(
			dev.device,pdev.physical_device,uni_buffer,
			uni_buffer_memory,sizeof(vec3f)*sum,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vkMapMemory(dev,uni_buffer_memory,0,sizeof(vec3f)*sum,0,&uni_buff_data);
		//write data to the storage buffers
		memcpy(uni_buff_data,&ubo[0],sizeof(vec3f)*sum);
		//vkUnmapMemory(dev,uni_buffer_memory);

		allocate_bind_buffer(
			dev.device,pdev.physical_device,uni_buffer2,
			uni_buffer_memory2,sizeof(vec3f)*sum,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vkMapMemory(dev,uni_buffer_memory2,0,sizeof(vec3f)*sum,0,&uni_buff_data2);
		//write data to the storage buffers
		memcpy(uni_buff_data2,&ubo2[0],sizeof(vec3f)*sum);
		//vkUnmapMemory(dev,uni_buffer_memory2);*/
	}

	{ //descriptor sets
		VkDescriptorSetAllocateInfo desc_set_ai =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &descriptor_set_layout
		};

		result = vkAllocateDescriptorSets(dev,&desc_set_ai,&descriptor_sets[0]);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!\n");

		VkDescriptorBufferInfo buff_info =
		{
			.buffer = uni_buff1.handle,
			.offset = 0,
			.range = sizeof(vec3f)*sum
		};

		VkDescriptorBufferInfo buff_info2 =
		{
			.buffer = uni_buff2.handle,
			.offset = 0,
			.range = sizeof(vec3f)*sum
		};

		VkWriteDescriptorSet desc_writes[3];
		VkWriteDescriptorSetAccelerationStructureKHR desc_asi =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
			.accelerationStructureCount = 1,
			.pAccelerationStructures = &tlas_as
		};
		
		desc_writes[0] =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = &desc_asi,
			.dstSet = descriptor_sets[0],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			//.pBufferInfo = &accel_info
		};
		vkUpdateDescriptorSets(dev,1,&desc_writes[0],0,nullptr);

		desc_writes[1] =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = descriptor_sets[0],
			.dstBinding = 1,
			//.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &buff_info
		};
		vkUpdateDescriptorSets(dev,2,&desc_writes[0],0,nullptr);

		desc_writes[2] =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = descriptor_sets[0],
			.dstBinding = 2,
			//.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.pBufferInfo = &buff_info2
		};
		vkUpdateDescriptorSets(dev,3,&desc_writes[0],0,nullptr);
	}

	{ //create command buffers
		vk_ctx c;
		c.dev = dev;
		c.pdev = pdev;
		c.inst = inst;
		//command_buffer cmd_buff2;
		cmd_buff.create(&c,command_pool,"cmd_buff_main");
		//printf("pre begin main\n");
		//cmd_buff.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		/*VkCommandBufferAllocateInfo cmd_buff_ai =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		result = vkAllocateCommandBuffers(dev,&cmd_buff_ai,&cmd_buffer);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers!\n");

		VkCommandBufferBeginInfo cmd_begin_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		result = vkBeginCommandBuffer(cmd_buffer,&cmd_begin_info);*/
		//vkCmdBindPipeline(cmd_buff.handle,VK_PIPELINE_BIND_POINT_COMPUTE,compute_pipeline);
		//vkCmdBindDescriptorSets(
		//	cmd_buff.handle,VK_PIPELINE_BIND_POINT_COMPUTE,
		//	compute_pipeline_layout,0,1,descriptor_sets,0,nullptr
		//);
		//vkCmdDispatch(cmd_buff.handle,1,1,1);
		//printf("hit first dispatch\n");
		//cmd_buff.submit(compute_queue,true);
	}

	/*VkSemaphoreCreateInfo sem_info;
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sem_info.flags = 0;
	
	result = vkCreateSemaphore(dev,&sem_info,nullptr,&cmp_fin_sem);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphore!\n");*/

	VkFenceCreateInfo compute_fence_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	result = vkCreateFence(dev,&compute_fence_info,nullptr,&compute_fence);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create compute fence!\n");
};

struct build_accel_struct
{
	VkAccelerationStructureBuildGeometryInfoKHR build_info;
	VkAccelerationStructureBuildSizesInfoKHR size_info;
	VkAccelerationStructureBuildRangeInfoKHR range_info;
	VkAccelerationStructureKHR accel_struct;
	buffer buff;

	build_accel_struct()
	{
		build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	};
};

accel_struct create_accel_struct(vk_ctx* c, VkAccelerationStructureCreateInfoKHR &asci)
{
	accel_struct ret;
	ret.buff.create(
		c,asci.size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	asci.buffer = ret.buff.handle;

	auto vkCreateAccelerationStructureKHR = 
	reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
		vkGetDeviceProcAddr(c->dev.device,"vkCreateAccelerationStructureKHR")
	);

	VkResult result = vkCreateAccelerationStructureKHR(c->dev.device,&asci,nullptr,&ret.accel);
	if(result != VK_SUCCESS)
		throw std::runtime_error("Failed to create blas acceleration structure!\n");
	
	return ret;
};

void vk_accelerator::cmd_create_tlas()
{
	vk_ctx c;
	c.dev = dev;
	c.pdev = pdev;
	c.inst = inst;

	command_buffer t_cmd_buff;
	t_cmd_buff.create(&c,command_pool,"tlas");
	t_cmd_buff.begin(0);

	VkAccelerationStructureDeviceAddressInfoKHR blas_dev_addr_info =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = blas_as
	};

	auto vkGetAccelerationStructureDeviceAddressKHR = 
	reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
		vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureDeviceAddressKHR")
	);

	VkDeviceAddress blas_address = vkGetAccelerationStructureDeviceAddressKHR(dev,&blas_dev_addr_info);

	VkTransformMatrixKHR tx_matrix = {
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f
	};

	VkAccelerationStructureInstanceKHR as_inst =
	{
		.transform = tx_matrix,
		.instanceCustomIndex = 0,
		.mask = 0xFF,
		.instanceShaderBindingTableRecordOffset = 0,
		.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
		.accelerationStructureReference = blas_address
		
	};

	buffer inst_buffer;
	inst_buffer.create(
		&c,sizeof(VkAccelerationStructureInstanceKHR),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	inst_buffer.map();
	inst_buffer.copy_to_device(&as_inst);

	VkAccelerationStructureGeometryInstancesDataKHR instances =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
		.arrayOfPointers = VK_FALSE,
		.data = {.deviceAddress = inst_buffer.get_device_address()}
	};

	VkAccelerationStructureGeometryKHR tlas_geo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
		.geometry = {.instances = instances},
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	VkAccelerationStructureBuildGeometryInfoKHR bgi =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &tlas_geo,
	};

	VkAccelerationStructureBuildSizesInfoKHR bsi =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
	};

	auto vkGetAccelerationStructureBuildSizesKHR =
	reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureBuildSizesKHR")
	);

	std::vector<uint32_t> instance_count = {1};
	vkGetAccelerationStructureBuildSizesKHR(
		dev.device,VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&bgi,instance_count.data(),&bsi
	);

	VkAccelerationStructureCreateInfoKHR asci =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.size = bsi.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
	};

	auto as = create_accel_struct(&c,asci);
	tlas_as = as.accel;

	buffer scratch;
	scratch.create(
		&c,bsi.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	bgi.srcAccelerationStructure = VK_NULL_HANDLE;
	bgi.dstAccelerationStructure = tlas_as;
	bgi.geometryCount = 1,
	bgi.pGeometries = &tlas_geo;
	bgi.scratchData.deviceAddress = scratch.get_device_address();

	VkAccelerationStructureBuildRangeInfoKHR boi =
	{
		.primitiveCount = 1,
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0
	};

	const VkAccelerationStructureBuildRangeInfoKHR* pboi = &boi;

	auto vkCmdBuildAccelerationStructuresKHR =
	reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
		vkGetDeviceProcAddr(dev.device, "vkCmdBuildAccelerationStructuresKHR")
	);

	vkCmdBuildAccelerationStructuresKHR(t_cmd_buff.handle,1,&bgi,&pboi);

	//need to submit here so that scratch buffer can safely free
	t_cmd_buff.submit(compute_queue,true);

	scratch.destroy();
	//t_cmd_buff.~command_buffer();
};

void vk_accelerator::cmd_create_blas()
{
	vk_ctx c;
	c.dev = dev;
	c.pdev = pdev;
	c.inst = inst;

	command_buffer t_cmd_buff;
	t_cmd_buff.create(&c,command_pool,"blas");
	t_cmd_buff.begin(0);

	uint32_t triangle_count = vertices.size() / 3;
	
	VkAccelerationStructureGeometryKHR geo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		.geometry =
		{
			.triangles =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
				.vertexStride = sizeof(float) * 3,
				.maxVertex = (uint32_t)vertices.size(),
				.indexType = VK_INDEX_TYPE_UINT32
			}
		},
		.flags = VK_GEOMETRY_OPAQUE_BIT_KHR
	};

	geo.geometry.triangles.vertexData.deviceAddress = getDeviceAddress(dev,vertex_buffer.handle);
	geo.geometry.triangles.indexData.deviceAddress = getDeviceAddress(dev,index_buffer.handle);

	build_accel_struct bas;
	bas.build_info =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		.srcAccelerationStructure = VK_NULL_HANDLE,
		.geometryCount = 1,
		.pGeometries = &geo,
	};
	
	bas.range_info = 
	{
		.primitiveCount = triangle_count,
		.primitiveOffset = 0,
		.firstVertex = 0,
		.transformOffset = 0
	};

	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = 
	reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
		vkGetDeviceProcAddr(dev.device, "vkGetAccelerationStructureBuildSizesKHR")
	);
	
	vkGetAccelerationStructureBuildSizesKHR(
		dev,VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&bas.build_info,&triangle_count,&bas.size_info
	);

	buffer scratch;
	scratch.create(
		&c,bas.size_info.buildScratchSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	
	VkAccelerationStructureCreateInfoKHR asci =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.size = bas.size_info.accelerationStructureSize,
		.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
	};

	accel_struct as = create_accel_struct(&c,asci);
	blas_as = as.accel;

	//structure is created, now it needs to be built
	//the following registers a command to be submitted

	bas.build_info.dstAccelerationStructure = as.accel;
	bas.build_info.scratchData.deviceAddress = scratch.get_device_address();

	auto vkCmdBuildAccelerationStructuresKHR = 
	reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
		vkGetDeviceProcAddr(dev.device, "vkCmdBuildAccelerationStructuresKHR")
	);

	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> range_info = {&bas.range_info};
	vkCmdBuildAccelerationStructuresKHR(t_cmd_buff.handle,1,&bas.build_info,range_info.data());

	//need to submit here so that scratch buffer can safely free
	t_cmd_buff.submit(compute_queue,true);

	scratch.destroy();
	//t_cmd_buff.~command_buffer();
};