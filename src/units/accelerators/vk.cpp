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
	
	for(uint32_t i = 0; i < m_tris->size(); i++)
		for(uint32_t j = 0; j < 3; j++)
			vertices.push_back(m_tris->at(i)[j].p);

	for(uint32_t i = 0; i < vertices.size(); i++)
		indices.push_back(i);

	for(uint32_t i = 0; i < m_tris->size(); i++)
		for(uint32_t j = 0; j < 3; j++)
			normals.push_back(m_tris->at(i)[j].n);

	t_data = new tx_data[JOB_SIZE];
	r_data = new rx_data[JOB_SIZE];
	/*pos_data = new vec4f[JOB_SIZE];
	dir_data = new vec4f[JOB_SIZE];*/

	//multithreading
	t_datas = new tx_data**[THREADS];
	r_datas = new rx_data**[THREADS];

	for(int32_t i = 0; i < THREADS; i++)
	{
		t_datas[i] = new tx_data*[2];
		r_datas[i] = new rx_data*[2];

		for(int32_t j = 0; j < 2; j++)
		{
			t_datas[i][j] = new tx_data[JOB_SIZE];
			r_datas[i][j] = new rx_data[JOB_SIZE];
		}
	}

	running = new bool*[THREADS];
	for(int32_t i = 0; i < THREADS; i++)
		running[i] = new bool[2];

	for(int32_t i = 0; i < THREADS; i++)
		for(int32_t j = 0; j < 2; j++)
			running[i][j] = false;

	assemble_everything(); //loads and initializes all of the VK stuff

	/*mgr = kp::Manager(
		std::make_shared<vk::Instance>(vk::Instance(inst.instance)),
		std::make_shared<vk::PhysicalDevice>(vk::PhysicalDevice(pdev.physical_device)),
		std::make_shared<vk::Device>(vk::Device(dev.device))
	);*/
};

#define VK_DEBUG

void vk_accelerator::assemble_everything()
{
	VkResult result;

	std::vector<VkValidationFeatureEnableEXT> val_layers = {
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
		VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
		//VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
		VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
	};

	{ //build the vk instance
		vkb::InstanceBuilder inst_builder;
		inst_builder.require_api_version(VK_API_VERSION_1_3);
#ifdef VK_DEBUG
		for(int i = 0; i < val_layers.size(); i++)
			inst_builder.add_validation_feature_enable(val_layers.at(i));
		inst_builder.enable_validation_layers();
#endif
		inst_builder.set_headless();
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

#ifdef VK_DEBUG
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
#endif

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

		vkGetDeviceQueue(dev,compute_family_index,0,&compute_queue);
	}

	{ //create and define descriptor set layouts
		//accel structure
		desc_set_layout.add_binding(
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			1, VK_SHADER_STAGE_COMPUTE_BIT
		);

		//vertex buffer
		desc_set_layout.add_binding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			1, VK_SHADER_STAGE_COMPUTE_BIT
		);

		//normal buffer
		desc_set_layout.add_binding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			1, VK_SHADER_STAGE_COMPUTE_BIT
		);

		//tx buffer
		desc_set_layout.add_binding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			1, VK_SHADER_STAGE_COMPUTE_BIT
		);

		//rx buffer
		desc_set_layout.add_binding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			1, VK_SHADER_STAGE_COMPUTE_BIT
		);

		desc_set_layout.create(dev);
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

	{ //create the compute pipelines
		create_compute_pipeline(
			&m_ctx,compute_shader_module,"main",
			{desc_set_layout.handle},&compute_pipeline,
			&compute_pipeline_layout
		);

		//multithreading
		/*pipeline_list = new VkPipeline*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			pipeline_list[i] = new VkPipeline[2];

		pipeline_layout_list = new VkPipelineLayout*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			pipeline_layout_list[i] = new VkPipelineLayout[2];

		for(int32_t i = 0; i < THREADS; i++)
			for(int32_t j = 0; j < 2; j++)
				create_compute_pipeline(
					&m_ctx,compute_shader_module,"main",
					{descriptor_set_layout},&pipeline_list[i][j],
					&pipeline_layout_list[i][j]
				);*/
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
		vk_check(result,"Failed to create command pool!\n");

		//multithreading
		pool_list = new VkCommandPool*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			pool_list[i] = new VkCommandPool[2];

		for(int32_t i = 0; i < THREADS; i++)
			for(int32_t j = 0; j < 2; j++)
				vkCreateCommandPool(dev,&command_pool_create_info,nullptr,&pool_list[i][j]);
	}

	{ //descriptor pool
		//acceleration structure
		desc_pool.add_size(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,1);

		//vertex buffer (readable)
		desc_pool.add_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1);

		//normal buffer
		desc_pool.add_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1);

		//tx buffer
		desc_pool.add_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1);

		//rx buffer
		desc_pool.add_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1);

		desc_pool.create(dev.device);
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

		normal_buffer.create(
			&m_ctx,normals.size()*sizeof(vec3f),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		normal_buffer.map();
		normal_buffer.copy_to_device(normals.data());
	}

	cmd_create_blas();
	cmd_create_tlas();

	{ //create data buffers
		t_buff.create(
			&m_ctx,(sizeof(tx_data))*JOB_SIZE,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		t_buff.map();

		r_buff.create(
			&m_ctx,(sizeof(rx_data))*JOB_SIZE,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		r_buff.map();		

		//multithreaded
		t_buffs = new buffer*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			t_buffs[i] = new buffer[2];

		for(int32_t i = 0; i < THREADS; i++)
			for(int32_t j = 0; j < 2; j++)
			{
				t_buffs[i][j].create(
					&m_ctx,(sizeof(tx_data))*JOB_SIZE,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
				t_buffs[i][j].map();
			}
		
		r_buffs = new buffer*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			r_buffs[i] = new buffer[2];

		for(int32_t i = 0; i < THREADS; i++)
			for(int32_t j = 0; j < 2; j++)
			{
				r_buffs[i][j].create(
					&m_ctx,(sizeof(rx_data))*JOB_SIZE,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
				r_buffs[i][j].map();
			}
	}

	{ //descriptor sets, cannot cross threads
		VkDescriptorBufferInfo vert_buff_info =
		{
			.buffer = vertex_buffer.handle,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkDescriptorBufferInfo norm_buff_info =
		{
			.buffer = normal_buffer.handle,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkDescriptorBufferInfo tx_buff_info =
		{
			.buffer = t_buff.handle,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};
		
		VkDescriptorBufferInfo rx_buff_info =
		{
			.buffer = r_buff.handle,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};

		VkWriteDescriptorSetAccelerationStructureKHR desc_asi =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
			.accelerationStructureCount = 1,
			.pAccelerationStructures = &tlas_as,
		};

		desc_set.allocate(&m_ctx,desc_pool.handle,&desc_set_layout.handle,1);
		
		desc_set.write_descriptor_set(
			&desc_asi,1,VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,nullptr
		);

		desc_set.write_descriptor_set(
			nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&vert_buff_info
		);

		desc_set.write_descriptor_set(
			nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&norm_buff_info
		);

		desc_set.write_descriptor_set(
			nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&tx_buff_info
		);

		desc_set.write_descriptor_set(
			nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&rx_buff_info
		);

		//multithreaded
		desc_set_list = new descriptor_set*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			desc_set_list[i] = new descriptor_set[2];

		for(int32_t i = 0; i < THREADS; i++)
		{
			for(int32_t j = 0; j < 2; j++)
			{
				VkDescriptorBufferInfo tx_buff_infos =
				{
					.buffer = t_buffs[i][j].handle,
					.offset = 0,
					.range = VK_WHOLE_SIZE
				};

				VkDescriptorBufferInfo rx_buff_infos =
				{
					.buffer = r_buffs[i][j].handle,
					.offset = 0,
					.range = VK_WHOLE_SIZE
				};

				desc_set_list[i][j].allocate(&m_ctx,desc_pool.handle,&desc_set_layout.handle,1);

				desc_set_list[i][j].write_descriptor_set(
					&desc_asi,1,VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,nullptr
				);

				desc_set_list[i][j].write_descriptor_set(
					nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&vert_buff_info
				);

				desc_set_list[i][j].write_descriptor_set(
					nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&norm_buff_info
				);

				desc_set_list[i][j].write_descriptor_set(
					nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&tx_buff_infos
				);

				desc_set_list[i][j].write_descriptor_set(
					nullptr,1,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&rx_buff_infos
				);
			}
		}
	}

	{ //create command buffers
		cmd_buff.create(&m_ctx,command_pool,"cmd_buff_main");

		//multithreading
		req_cmd_buffs = new command_buffer*[THREADS];
		for(int32_t i = 0; i < THREADS; i++)
			req_cmd_buffs[i] = new command_buffer[2];
		
		for(int32_t i = 0; i < THREADS; i++)
			for(int32_t j = 0; j < 2; j++)
				req_cmd_buffs[i][j].create(&m_ctx,pool_list[i][j],"rcb");
	}

	VkFenceCreateInfo compute_fence_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	result = vkCreateFence(dev,&compute_fence_info,nullptr,&compute_fence);
	vk_check(result,"Failed to create compute fence!\n");

	result = vkWaitForFences(dev,1,&compute_fence,VK_FALSE,UINT64_MAX);
	vk_check(result,"Failed to wait for fences!\n");

	//multithreading
	req_fences = new VkFence*[THREADS];
	for(int32_t i = 0; i < THREADS; i++)
		req_fences[i] = new VkFence[2];

	for(int32_t i = 0; i < THREADS; i++)
		for(int32_t j = 0; j < 2; j++)
			if(vkCreateFence(dev,&compute_fence_info,nullptr,&req_fences[i][j])!=VK_SUCCESS)
				throw std::runtime_error("[MT] Failed to create compute fence!\n");

	for(int32_t i = 0; i < THREADS; i++)
		for(int32_t j = 0; j < 2; j++)
			if(vkWaitForFences(dev,1,&req_fences[i][j],VK_FALSE,UINT64_MAX)!=VK_SUCCESS)
				throw std::runtime_error("[MT] Failed to wait for fences!\n");
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
	command_buffer t_cmd_buff;
	t_cmd_buff.create(&m_ctx,command_pool,"tlas");
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
		&m_ctx,sizeof(VkAccelerationStructureInstanceKHR),
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

	auto as = create_accel_struct(&m_ctx,asci);
	tlas_as = as.accel;

	buffer scratch;
	scratch.create(
		&m_ctx,bsi.buildScratchSize,
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
	command_buffer t_cmd_buff;
	t_cmd_buff.create(&m_ctx,command_pool,"blas");
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
				.maxVertex = (uint32_t)vertices.size() - 1,
				.indexType = VK_INDEX_TYPE_NONE_KHR
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
		&m_ctx,bas.size_info.buildScratchSize,
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

	accel_struct as = create_accel_struct(&m_ctx,asci);
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