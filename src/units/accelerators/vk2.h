#pragma once
#include "../../base/accelerator.h"
#include "../../core/records.h"

#include <malloc.h>
#include <xmmintrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <numeric>
#include <memory>

#include "../../../ext/vk-bootstrap/src/VkBootstrap.h"
//#include "../../../ext/kompute/src/include/kompute/Kompute.hpp"
#include "../../../ext/uvk/src/uvk.h"
#include <ios>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>

class scene;

/*
notes:
	working sizes:
		root,	div,	shader, errors, image,		bundle, result
		64		128		128		false	correct		64*64	16
		128		1024	1024	false	correct		128*128	16		//best so far, also max size
*/

struct alignas(16) light_struct //16 bytes
{
	//matrix4x4<float> matrix;
	vec3f pos;
	uint32_t type;
};

struct alignas(16) origin_struct //16 bytes
{
	vec3f pos;
	uint32_t pad;
};

struct alignas(16) submit_struct //16 bytes
{
	vec3f d;
	float maxt;
};

struct result_struct_thin //16 bytes
{
	uint32_t tri_index;
	vec2h uv;
	float dist;
	uint32_t hit_light;
};

struct result_struct //32 bytes
{
	uint32_t tri_index;
	vec2h uv;
	float dist;
	vec3h sh_n;
	vec3h sh_s;
	vec3h sh_t;
	half hit_light;
};

struct tempo
{
	uint32_t tri_index;
	uint32_t uv;
	float dist;
	uint32_t nxny;
	uint32_t nzsx;
	uint32_t sysz;
	uint32_t txty;
	uint32_t tzhit;
};

struct vertex_struct
{
	vec3f pos;
	uint32_t __pad0;
	vec3f normal;
	uint32_t __pad1;
};

struct vk_thread
{
	uvk::command_pool cmd_pool;
	uvk::command_buffer cmd_buff[SETS];

	uvk::descriptor_set desc_set[SETS];

	uvk::buffer origin_buffer[SETS];
	origin_struct* origin_data[SETS];

	uvk::buffer submit_buffer[SETS];
	submit_struct* submit_data[SETS];
	
	uvk::buffer result_buffer[SETS];
	result_struct* result_data[SETS];

	VkFence uni_fence;
};

class vk_accelerator2 : public accelerator
{
	VkDevice dev;
	VkPhysicalDevice p_dev;
	VkInstance inst;

	VkQueue queue;
	uint32_t compute_index = 0;

	uvk::command_pool cmd_pool[SETS];
	uvk::command_buffer cmd_buff[SETS];

	uvk::descriptor_pool desc_pool;
	uvk::descriptor_set_layout desc_layout;
	uvk::descriptor_set desc_set[SETS];

	uvk::shader_module shader;

	uvk::compute_pipeline pipeline;
	
	uvk::acceleration_structure as;

	std::vector<vec3f> vertices = {};

	uvk::buffer vertex_buffer;
	std::vector<vertex_struct> vertex_data = {};

	uvk::buffer light_buffer;
	std::vector<light_struct> light_data = {};

	int32_t job_size = 0;
	vec3i job_dims = 0;
	vec3i submit_size = 0;
	int32_t divisor = 0;

	vk_thread threads[8];

	std::mutex queue_lock;

	public:
	vk_accelerator2(vec3i jd, vec3i sz, scene* s);

	~vk_accelerator2() //cleans up some of the garbage
	{
		for(int32_t i = 0; i < 4; i++)
		{
			for(int32_t j = 0; j < SETS; j++)
			{
				threads[i].cmd_buff[j].destroy();

				threads[i].submit_buffer[j].unmap();
				threads[i].result_buffer[j].unmap();

				threads[i].submit_buffer[j].destroy();
				threads[i].result_buffer[j].destroy();
			}
		}

		for(int32_t j = 0; j < SETS; j++)
			cmd_buff[j].destroy();
	};

	void assemble_everything();

	int32_t get_job_size() const {return job_size;};
	vec3i get_job_dims() const {return job_dims;};

	void create_thread(vk_thread &t)
	{
		for(int32_t i = 0; i < SETS; i++)
			t.origin_data[i] = new origin_struct[job_size];

		for(int32_t i = 0; i < SETS; i++)
			t.submit_data[i] = new submit_struct[job_size];
	
		/*for(int32_t i = 0; i < SETS; i++)
			t.result_data[i] = new result_struct[job_size];*/

		t.cmd_pool.create(dev,VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,compute_index);

		for(int32_t i = 0; i < SETS; i++)
		{
			t.origin_buffer[i].create(
				dev,p_dev,sizeof(origin_struct)*job_size,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
				VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
				//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SHARING_MODE_EXCLUSIVE
			);
			t.origin_buffer[i].map();
			//t.origin_data[i] = (origin_struct*)t.origin_buffer[i].dst;

			t.submit_buffer[i].create(
				dev,p_dev,sizeof(submit_struct)*job_size,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				//VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
				VK_SHARING_MODE_EXCLUSIVE
			);
			t.submit_buffer[i].map();
			//t.submit_data[i] = (submit_struct*)t.submit_buffer[i].dst;

			t.result_buffer[i].create(
				dev,p_dev,sizeof(result_struct)*job_size,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
				VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
				VK_SHARING_MODE_EXCLUSIVE
			);
			t.result_buffer[i].map();
			t.result_data[i] = (result_struct*)t.result_buffer[i].dst;
		}

		for(int32_t i = 0; i < SETS; i++)
		{
			t.desc_set[i].create(dev,desc_pool.handle,&desc_layout.handle,1);
			VkWriteDescriptorSetAccelerationStructureKHR desc_asi =
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
				.accelerationStructureCount = 1,
				.pAccelerationStructures = &as.tlas_handle,
			};
			t.desc_set[i].write_descriptor_set(&desc_asi,1,
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,nullptr);
			t.desc_set[i].write_buffer_info(light_buffer.handle);
			t.desc_set[i].write_buffer_info(vertex_buffer.handle);
			t.desc_set[i].write_buffer_info(t.origin_buffer[i].handle);
			t.desc_set[i].write_buffer_info(t.submit_buffer[i].handle);
			t.desc_set[i].write_buffer_info(t.result_buffer[i].handle);
		}

		for(int32_t i = 0; i < SETS; i++)
			t.cmd_buff[i].create(dev,t.cmd_pool.handle);
		
		VkFenceCreateInfo fci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		vkCreateFence(dev,&fci,nullptr,&t.uni_fence);
		vkResetFences(dev,1,&t.uni_fence);
	};

	//this has to be run from the control thread and be run after worker pushes data to GPU
	void submit(int32_t id)
	{
		/*for(int32_t i = 0; i < SETS; i++)
			submit_buffer[i].sync_memory();*/

		for(int32_t i = 0; i < SETS; i++)
		{
			cmd_buff[i].begin();

			vkCmdBindPipeline(
				cmd_buff[i].handle,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline.handle
			);

			vkCmdBindDescriptorSets(
				cmd_buff[i].handle,VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeline.layout,0,1,&desc_set[i].handle,0,nullptr
			);

			printf("job_size: %i\n",job_size);
			vkCmdDispatch(cmd_buff[i].handle,job_size,2,1);

			cmd_buff[i].submit(queue,false);
		}
	};

	void submit_t(int32_t id)
	{
		for(int32_t i = 0; i < SETS; i++)
		{
			//threads[id].origin_buffer[i].sync_memory();
			threads[id].submit_buffer[i].sync_memory();
		}

		threads[id].cmd_pool.reset();

		for(int32_t i = 0; i < SETS; i++)
		{
			threads[id].cmd_buff[i].begin();

			vkCmdBindPipeline(
				threads[id].cmd_buff[i].handle,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline.handle
			);

			vkCmdBindDescriptorSets(
				threads[id].cmd_buff[i].handle,VK_PIPELINE_BIND_POINT_COMPUTE,
				pipeline.layout,0,1,&threads[id].desc_set[i].handle,0,nullptr
			);

			vkCmdDispatch(threads[id].cmd_buff[i].handle,submit_size.x,submit_size.y,submit_size.z);

			//threads[id].cmd_buff[i].submit(queue,true);
			VkResult result = vkEndCommandBuffer(threads[id].cmd_buff[i].handle);
			uvk::vk_check(result,"Failed to end command buffer!\n");
			threads[id].cmd_buff[i].recording = false;
		}

		std::vector<VkCommandBuffer> cbs = {};
		for(int32_t i = 0; i < SETS; i++)
			cbs.push_back(threads[id].cmd_buff[i].handle);

		VkSubmitInfo si =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = SETS,
			.pCommandBuffers = cbs.data()
		};

		{
			std::unique_lock<std::mutex> lock(queue_lock);
			VkResult result = vkQueueSubmit(queue,1,&si,threads[id].uni_fence);
			uvk::vk_check(result,"Failed to submit!\n");
		}

		VkResult result = vkWaitForFences(dev,1,&threads[id].uni_fence,true,UINT64_MAX);
		uvk::vk_check(result,"Failed to wait!\n");
		result = vkResetFences(dev,1,&threads[id].uni_fence);
		uvk::vk_check(result,"Failed to reset!\n");
	};

	bool intersect(ray &r, hit_record &h_rec) const
	{
		return false;
	};

	float8 intersect8(ray8 &r, hit_record8 &h_rec, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};

	bool occluded(const ray &r, const vec3f &pos) const
	{
		return false;
	};

	float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};

	/*void intersect_n(ray_bundle* bundle)
	{
		for(int32_t j = 0; j < SETS; j++)
		{
			for(int32_t i = 0; i < job_size; i++)
			{
			
				submit_data[j][i].p = bundle[j].r[i].o;
				submit_data[j][i].mint = bundle[j].r[i].mint;
				submit_data[j][i].d = bundle[j].r[i].d;
				submit_data[j][i].maxt = bundle[j].r[i].maxt;
			}
		}

		//record_and_submit_command(0);

		for(int32_t j = 0; j < SETS; j++)
		{
			for(int32_t i = 0; i < job_size; i++)
			{
			
				auto& h_rec = bundle[j].h_rec[i];
				auto& result = result_data[j][i];			

				h_rec.pos = result.pos;
				h_rec.dist = result.dist;
				h_rec.bary = result.bary;
				h_rec.sh_frame.n = result.sh_n;
				h_rec.sh_frame.s = result.sh_s;
				h_rec.sh_frame.t = result.sh_t;
				h_rec.wi = bundle[j].r[i].d;

				h_rec.light_pdf = result.light_pdf;

				if(result.dist != -1.0f)
				{
					uint32_t tri_idx = result.tri_index;
					tri* t = &m_tris->at(tri_idx);
					h_rec.m_shape.set_material(m_mats->at(t->m));
					h_rec.tp = t;
					bundle[j].valid[i] = true;
				}
				else
				{
					bundle[j].valid[i] = false;
				}
			}
		}
	};*/

	/*void push_data(int32_t id, int32_t index, ray_bundle &bundle)
	{
		//cmd_buff[index].reset();
		//submit_buffer[index].copy_to_device(&bundle.r[0]);
	};*/

	void push_dir(int32_t id, int32_t index, ray_bundle &bundle)
	{
		for(int32_t i = 0; i < job_size; i++)
			threads[id].submit_data[index][i].d = bundle.r[i].d;
		threads[id].submit_buffer[index].copy_to_device(threads[id].submit_data[index]);
		//threads[id].submit_buffer[index].copy_to_device(&bundle.r[0]);
	};

	void push_pos(int32_t id, int32_t index, ray_bundle &bundle)
	{
		for(int32_t i = 0; i < job_size; i++)
			threads[id].origin_data[index][i].pos = bundle.r[i].o;
		threads[id].origin_buffer[index].copy_to_device(threads[id].origin_data[index]);
	};

	void pull_dir(int32_t id, int32_t index, ray_bundle &bundle)
	{
		threads[id].result_buffer[index].sync_memory();
		//threads[id].result_buffer[index].copy_from_device(threads[id].result_data[index]);
	};

	void pull_pos(int32_t id, int32_t index, ray_bundle &bundle)
	{
		threads[id].origin_buffer[index].sync_memory();
		threads[id].origin_buffer[index].copy_from_device(threads[id].origin_data[index]);
		for(int32_t i = 0; i < job_size; i++)
			bundle.r[i].o = threads[id].origin_data[index][i].pos;
	};

	void reset_cmd_buff(int32_t id, int32_t index) {threads[id].cmd_buff[index].reset();};

	//intersect single thread
	void intersect_s(int32_t id, int32_t index, ray_bundle &bundle)
	{
		//while(cmd_buff[index].wait(UINT64_MAX,false) != VK_SUCCESS) {1+1;};
		/*cmd_buff[index].wait();
		result_buffer[index].copy_from_device(result_data[index]);
		result_buffer[index].sync_memory();

		for(int32_t i = 0; i < job_size; i++)
		{
			auto& h_rec = bundle.h_rec[i];
			auto& result = result_data[index][i];			

			h_rec.pos = result.pos;
			h_rec.dist = result.dist;
			h_rec.bary = result.bary;
			h_rec.sh_frame.n = result.sh_n;
			h_rec.sh_frame.s = result.sh_s;
			h_rec.sh_frame.t = result.sh_t;
			h_rec.wi = bundle.r[i].d;

			h_rec.light_pdf = result.light_pdf;

			if(result.dist != -1.0f)
			{
				uint32_t tri_idx = result.tri_index;
				tri* t = &m_tris->at(tri_idx);
				h_rec.m_shape.set_material(m_mats->at(t->m));
				h_rec.tp = t;
				bundle.valid[i] = true;
			}
			else
			{
				bundle.valid[i] = false;
			}
		}*/
	};

	//intersect multi thread
	void intersect_n(int32_t id, int32_t index, ray_bundle &bundle)
	{
		threads[id].result_buffer[index].sync_memory();
		//threads[id].result_buffer[index].copy_from_device(threads[id].result_data[index]);
		
		for(int32_t i = 0; i < job_size; i++)
		{
			auto& h_rec = bundle.h_rec[i];
			auto& result = threads[id].result_data[index][i];			

			h_rec.dist = result.dist;
			h_rec.wi = bundle.r[i].d;
			h_rec.hit_light = result.hit_light > 0.0f;

			if(result.dist != -1.0f)
			{
				uint32_t tri_idx = result.tri_index;
				tri* t = &m_tris->at(tri_idx);
				h_rec.m_shape.set_material(m_mats->at(t->m));
				h_rec.tp = t;

				//tri orientation
				//vec3f s1 = t->b.p - t->a.p;
		
				for(int32_t j = 0; j < 3; j++)
				{
					h_rec.sh_frame.n[j] = result.sh_n[j];
					h_rec.sh_frame.s[j] = result.sh_s[j];
					h_rec.sh_frame.t[j] = result.sh_t[j];
				}

				h_rec.wi = bundle.r[i].d;
				h_rec.pos = float(result.dist) * bundle.r[i].d + bundle.r[i].o;
				h_rec.dist = result.dist;
				vec2f uv;
				uv.x = result.uv.x;
				uv.y = result.uv.y;

				float b1 = uv.x;
				float b2 = uv.y;
				float b0 = 1.0f - b1 - b2;
				h_rec.bary = vec3f(b0,b1,b2);
			
				/*h_rec.sh_frame.n = normalize(t->a.n*b0+t->b.n*b1+t->c.n*b2);
				calc_shading_frame<float>(h_rec.sh_frame.n,s1,h_rec.sh_frame);*/
				//tri orientation

				bundle.valid[i] = true;
				total_hits++;
			}
			else
			{
				bundle.valid[i] = false;
			}
		}
	};
};