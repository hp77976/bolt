#include "vk2.h"
#include "../../core/scene.h"
#include "../../base/light.h"
#include <iostream>

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

std::vector<VkValidationFeatureEnableEXT> val_layers = {
	VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
	VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
	VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
	VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
};

vk_accelerator2::vk_accelerator2(vec3i jd, vec3i sz, scene* s) : accelerator(s)
{
	job_dims = jd;
	job_size = jd.x * jd.y * jd.z;
	submit_size = sz;
	printf("job size: %i\n",job_size);

	for(int32_t i = 0; i < m_tris->size(); i++)
	{
		for(int32_t j = 0; j < 3; j++)
		{
			vertices.push_back(m_tris->at(i)[j].p);

			vec3f p = m_tris->at(i)[j].p;
			vec3f n = m_tris->at(i)[j].n;
			
			vertex_struct vs;
			vs.pos = p;
			vs.normal = n;
			vertex_data.push_back(vs);
		}
	}

	auto lights = s->get_lights();
	for(int32_t i = 0; i < lights->size(); i++)
	{
		light_struct ls;
		//ls.matrix = lights->at(i)->get_matrix();
		ls.pos = lights->at(i)->get_position();
		ls.type = lights->at(i)->type();
		light_data.push_back(ls);
	}

	printf("Building Vulkan Accelerator 2\n");

	assemble_everything();
};

#define DISABLE_VK_DEBUG

void vk_accelerator2::assemble_everything()
{
	vkb::Instance vkb_inst;
	vkb::PhysicalDevice vkb_p_dev;
	vkb::Device vkb_dev;
	
	{ //build the vk instance
		vkb::InstanceBuilder inst_builder;
		inst_builder.require_api_version(VK_API_VERSION_1_3);
#ifndef DISABLE_VK_DEBUG
		//for(int i = 0; i < val_layers.size(); i++)
			//inst_builder.add_validation_feature_enable(val_layers.at(i));
		//inst_builder.enable_validation_layers();
#endif
		inst_builder.set_headless();
		inst_builder.enable_extension(VK_KHR_SURFACE_EXTENSION_NAME);
		auto ib_ret = inst_builder.build();
		if(!ib_ret)
		{
			std::cerr << "Error: " << ib_ret.error().message() << "\n";
			throw std::runtime_error("Failed to create instance!\n");
		}
		else
			vkb_inst = ib_ret.value();
	}

	{ //build the physical device
		vkb::PhysicalDeviceSelector pdev_selector(vkb_inst);
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
			vkb_p_dev = pds_ret.value();
	}

	{ //build the logical device
		vkb::DeviceBuilder dev_builder(vkb_p_dev);
		auto dev_builder_ret = dev_builder.build();
		if(!dev_builder_ret)
			throw std::runtime_error("Failed to build logical device!\n");
		else
			vkb_dev = dev_builder_ret.value();
	}

	dev = vkb_dev.device;
	p_dev = vkb_dev.physical_device;
	inst = vkb_inst.instance;

#ifndef DISABLE_VK_DEBUG
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

	compute_index = uvk::get_compute_queue(dev,p_dev,queue);

	//for(int32_t i = 0; i < SETS; i++)
	//	cmd_pool[i].create(dev,VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,compute_index);

	cmd_pool[0].create(dev,VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,compute_index);
	as.create(dev,p_dev,cmd_pool[0].handle,queue,vertices.size(),(float*)vertices.data());
	//cmd_pool[0].destroy();

	std::vector<VkDescriptorType> desc_types = {
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, //light
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, //vertex
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, //pos
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, //submit
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER  //result
	};

	for(int32_t i = 0; i < desc_types.size(); i++)
		desc_layout.add_binding(desc_types.at(i),1,VK_SHADER_STAGE_COMPUTE_BIT);
	desc_layout.create(dev);

	shader.create(dev,"/home/batman/Github/bolt-repo2/src/units/shaders/basic.spv",nullptr,0);

	pipeline.create(dev,shader.handle,"main",{desc_layout.handle});

	for(int32_t i = 0; i < desc_types.size(); i++)
		desc_pool.add_size(desc_types.at(i),1);
	desc_pool.create(dev);

	{ //readonly data
		vertex_buffer.create(
			dev,p_dev,sizeof(vertex_struct)*vertex_data.size(),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SHARING_MODE_EXCLUSIVE
		);
		vertex_buffer.map();
		vertex_buffer.copy_to_device(vertex_data.data());
		vertex_buffer.unmap();

		/*normal_buffer.create(
			dev,p_dev,sizeof(vec4f)*normal_data.size(),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE
		);
		normal_buffer.map();
		normal_buffer.copy_to_device(normal_data.data());
		normal_buffer.unmap();*/

		printf("lights: %lu\n",light_data.size());
		light_buffer.create(
			dev,p_dev,sizeof(light_struct)*light_data.size(),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			VK_SHARING_MODE_EXCLUSIVE
		);
		light_buffer.map();
		light_buffer.copy_to_device(light_data.data());
		light_buffer.unmap();
	}
	
	/*for(int i = 0; i < SETS; i++)
	{
		submit_buffer[i].create(
			dev,p_dev,sizeof(submit_struct)*job_size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			VK_SHARING_MODE_EXCLUSIVE
		);
		submit_buffer[i].map();

		result_buffer[i].create(
			dev,p_dev,sizeof(result_struct)*job_size,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			VK_SHARING_MODE_EXCLUSIVE
		);
		result_buffer[i].map();
	}*/

	/*for(int i = 0; i < SETS; i++)
	{
		desc_set[i].create(dev,desc_pool.handle,&desc_layout.handle,1);
		VkWriteDescriptorSetAccelerationStructureKHR desc_asi =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
			.accelerationStructureCount = 1,
			.pAccelerationStructures = &as.tlas_handle,
		};
		desc_set[i].write_descriptor_set(&desc_asi,1,
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,nullptr);
		desc_set[i].write_buffer_info(vertex_buffer.handle);
		//desc_set[i].write_buffer_info(normal_buffer.handle);
		desc_set[i].write_buffer_info(light_buffer.handle);
		desc_set[i].write_buffer_info(submit_buffer[i].handle);
		desc_set[i].write_buffer_info(result_buffer[i].handle);
	}*/

	for(int32_t i = 0; i < SETS; i++)
		cmd_buff[i].create(dev,cmd_pool[0].handle);

	printf("sizeof(pos_struct): %lu\n",sizeof(origin_struct));
	printf("sizeof(submit_struct): %lu\n",sizeof(submit_struct));
	printf("sizeof(result_struct): %lu\n",sizeof(result_struct));

	printf("Creating vk threads\n");
	for(int32_t i = 0; i < 8; i++)
		create_thread(threads[i]);
};