int32_t sum = job_size.x * job_size.y * job_size.z;

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
	for(int i = 0; i < val_layers.size(); i++)
		inst_builder.add_validation_feature_enable(val_layers.at(i));
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