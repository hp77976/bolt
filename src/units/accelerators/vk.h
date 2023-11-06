#pragma once
#include "../../base/accelerator.h"
#include "../../core/records.h"
#include "../../core/vulkan/vk_core.h"

#include <malloc.h>
#include <xmmintrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <numeric>
#include <memory>

#include "../../../ext/vk-bootstrap/src/VkBootstrap.h"
//#include "../../../ext/kompute/src/include/kompute/Kompute.hpp"
#include <ios>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>
#include <cstring>

class scene;

/*void allocate_bind_buffer(
	VkDevice &dev, VkPhysicalDevice &pdev, VkBuffer &buff, VkDeviceMemory &mem,
	uint32_t mem_size, VkBufferUsageFlags use_flags, VkMemoryPropertyFlags mem_flags
);*/

/*struct buffer_data
{
	vec3f p, d;
};*/

//idx
//dist
//uv (vec2)
//frame (vec9) ?

struct tx_data //32 bytes
{
	ray r;
};

struct rx_data //64 bytes
{
	int32_t index; //4
	float distance; //8
	vec2f uv; //16
	frame<> sh_frame; //52 //need this for post sample ops
	vec3f pos;
};

#define THREADS 32

class vk_accelerator : public accelerator
{
	vkb::Instance inst;
	mutable vkb::PhysicalDevice pdev;
	mutable vkb::Device dev;
	vk_ctx m_ctx;
	
	uint32_t graphics_family_index = -1;
	uint32_t compute_family_index = -1;

	VkQueue compute_queue;

	descriptor_set_layout desc_set_layout;

	VkShaderModule compute_shader_module;

	VkPipelineLayout compute_pipeline_layout;
	VkPipeline compute_pipeline;

	VkCommandPool command_pool;

	descriptor_pool desc_pool;

	std::vector<vec3f> vertices = {};
	std::vector<vec3f> normals = {};
	std::vector<uint32_t> indices = {};

	buffer vertex_buffer;
	buffer normal_buffer;
	buffer index_buffer;

	VkAccelerationStructureKHR blas_as;
	VkAccelerationStructureKHR tlas_as;

	mutable command_buffer cmd_buff;

	//mutable kp::Manager mgr;

	std::vector<uint32_t> shader_code_raw = {};

	vec3i job_size {JOB_X,JOB_Y,JOB_Z};

	/*mutable vec4f* ubo;
	mutable vec4f* ubo2;

	mutable buffer uni_buff1;
	mutable buffer uni_buff2;*/

	uint32_t desc_set_count = 1;
	descriptor_set desc_set;

	VkFence compute_fence;
	VkSemaphore cmp_fin_sem;

	//single threaded
	mutable tx_data* t_data;
	mutable rx_data* r_data;

	mutable buffer t_buff;
	mutable buffer r_buff;

	//mult threaded stuff
	mutable tx_data*** t_datas;
	mutable rx_data*** r_datas;

	mutable buffer** t_buffs;
	mutable buffer** r_buffs;

	//probably want two pools per thread? so [threads][2]
	//then get one fence and cmd buff per pool
	//one descriptor set per pipeline layout (hardware limitation)
	mutable VkCommandPool** pool_list;
	mutable VkFence** req_fences;
	mutable command_buffer** req_cmd_buffs;
	//mutable VkPipeline** pipeline_list;
	//mutable VkPipelineLayout** pipeline_layout_list;
	//mutable VkDescriptorPool** desc_pool_list;
	mutable descriptor_set** desc_set_list;

	mutable bool** running;

	public:
	vk_accelerator(scene* s);

	~vk_accelerator() //cleans up some of the garbage
	{
		vertex_buffer.destroy();
		index_buffer.destroy();
		vkDestroyDescriptorPool(dev,desc_pool.handle,nullptr);
		vkDestroyCommandPool(dev,command_pool,nullptr);
		vkDestroyPipeline(dev,compute_pipeline,nullptr);
		vkDestroyPipelineLayout(dev,compute_pipeline_layout,nullptr);
		vkDestroyDescriptorSetLayout(dev,desc_set_layout.handle,nullptr);
	};

	void assemble_everything();

	void cmd_create_blas();

	void cmd_create_tlas();

	//TODO: make command recording in thread pool, keep memcopy in accelerator

	//run first, then control thread runs record_and_submit_command
	void push_data(int32_t id, ray_bundle &bundle) const
	{
		for(int i = 0; i < JOB_SIZE; i++)
			t_datas[id][0][i].r = bundle.r[i];
		
		t_buffs[id][0].copy_to_device(&t_datas[id][0][0]);

		//worker must mark itself as waiting after running this
	};

	//this has to be run from the control thread and be run after worker pushes data to GPU
	void record_and_submit_command(int32_t id)
	{
		VkResult result = vkResetFences(dev,1,&req_fences[id][0]);
		vk_check(result,"Failed to reset fences!\n");

		req_cmd_buffs[id][0].reset(0);
		req_cmd_buffs[id][0].begin(0);

		vkCmdBindPipeline(
			req_cmd_buffs[id][0].handle,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			compute_pipeline
		);

		vkCmdBindDescriptorSets(
			req_cmd_buffs[id][0].handle,VK_PIPELINE_BIND_POINT_COMPUTE,
			compute_pipeline_layout,0,1,&desc_set_list[id][0].handle,0,nullptr
		);

		//vkCmdDispatch(req_cmd_buffs[id][0].handle,JOB_X/DIV,JOB_Y,JOB_Z);

		req_cmd_buffs[id][0].submit(compute_queue,req_fences[id][0]);

		//after this, signal to thread that its ready to wait on the results

		vkWaitForFences(dev,1,&req_fences[id][0],VK_TRUE,1000000);
		vk_check(result,"Failed to wait for fences!\n");
		//if(result != VK_SUCCESS)
			//return false;
	};

	/*bool fence_wait(int32_t id) const
	{
		VkResult result = vkWaitForFences(dev,1,&req_fences[id][0],VK_TRUE,1000000);
		//vk_check(result,"Failed to wait for fences!\n");
		if(result != VK_SUCCESS)
			return false;
	};*/

	//requires push_data and a ready signal from control thred
	void pull_data(int32_t id) const
	{
		r_buffs[id][0].copy_from_device(&r_datas[id][0][0]);
	};

	void submit_queue_and_get_results() const
	{
		/*mgr.tensor()
		std::shared_ptr<kp::TensorT<float>> ta = mgr.tensor({1.0f,2.0f,3.0f});
		std::shared_ptr<kp::TensorT<float>> tb = mgr.tensor({1.0f,2.0f,3.0f});

		std::vector<std::shared_ptr<kp::Tensor>> params = {ta,tb};

		std::shared_ptr<kp::Algorithm> algo = mgr.algorithm(params,shader_code_raw);

		mgr.sequence()
			->record<kp::OpTensorSyncDevice>(params)
			->record<kp::OpAlgoDispatch>(algo)
			->record<kp::OpTensorSyncLocal>(params)
			->eval();*/

		int32_t sum = job_size.x * job_size.y * job_size.z;

		t_buff.copy_to_device(&t_data[0]);
		//dir_buff.copy_to_device(&dir_data[0]);

		//reset the fences
		VkResult result = vkResetFences(dev,1,&compute_fence);
		vk_check(result,"Failed to reset fences!\n");

		//reset the command buffer
		cmd_buff.reset(0);

		{ //record the compute commands
			cmd_buff.begin(0);

			vkCmdBindPipeline(cmd_buff.handle,VK_PIPELINE_BIND_POINT_COMPUTE,compute_pipeline);

			vkCmdBindDescriptorSets(
				cmd_buff.handle,VK_PIPELINE_BIND_POINT_COMPUTE,
				compute_pipeline_layout,0,1,&desc_set.handle,0,nullptr
			);

			//vkCmdDispatch(cmd_buff.handle,JOB_X/DIV,JOB_Y,JOB_Z);

			cmd_buff.submit(compute_queue,compute_fence);
		}

		result = vkWaitForFences(dev,1,&compute_fence,VK_TRUE,UINT64_MAX);
		vk_check(result,"Failed to wait for fences!\n");

		r_buff.copy_from_device(&r_data[0]);
		//dir_buff.copy_from_device(&dir_data[0]);
	};

	void submit_queue_and_get_results(int32_t id) const
	{
		t_buffs[id][0].copy_to_device(&t_datas[id][0][0]);
		
		//reset the fences
		VkResult result = vkResetFences(dev,1,&req_fences[id][0]);
		vk_check(result,"Failed to reset fences!\n");

		//reset the command buffer
		req_cmd_buffs[id][0].reset(0);

		{ //record the compute commands
			req_cmd_buffs[id][0].begin(0);

			vkCmdBindPipeline(req_cmd_buffs[id][0].handle,VK_PIPELINE_BIND_POINT_COMPUTE,compute_pipeline);

			vkCmdBindDescriptorSets(
				req_cmd_buffs[id][0].handle,VK_PIPELINE_BIND_POINT_COMPUTE,
				compute_pipeline_layout,0,1,&desc_set_list[id][0].handle,0,nullptr
			);

			//vkCmdDispatch(req_cmd_buffs[id][0].handle,JOB_X/DIV,JOB_Y,JOB_Z);

			req_cmd_buffs[id][0].submit(compute_queue,req_fences[id][0]);
		}

		result = vkWaitForFences(dev,1,&req_fences[id][0],VK_TRUE,UINT64_MAX);
		vk_check(result,"Failed to wait for fences!\n");

		r_buffs[id][0].copy_from_device(&r_datas[id][0][0]);
	};

	void submit_request(int32_t id, int32_t index, ray_bundle &bundle) const
	{
		/*for(int i = 0; i < JOB_SIZE; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				//TODO: copy rays across via memcpy
				pos_datas[id][index][i][j] = bundle.r[i].o[j];
				dir_datas[id][index][i][j] = bundle.r[i].d[j];
			}
		}

		pos_buffs[id][index].copy_to_device(&pos_datas[id][index][0]);
		dir_buffs[id][index].copy_to_device(&dir_datas[id][index][0]);

		VkResult result = vkResetFences(dev,1,&req_fences[id][index]);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to reset fences!\n");
		
		req_cmd_buffs[id][index].reset(0);
		req_cmd_buffs[id][index].begin(0);

		vkCmdBindPipeline(
			req_cmd_buffs[id][index].handle,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			compute_pipeline
		);
		
		vkCmdBindDescriptorSets(
			req_cmd_buffs[id][index].handle,VK_PIPELINE_BIND_POINT_COMPUTE,
			compute_pipeline_layout,0,1,&desc_set.handle,0,nullptr
		);

		vkCmdDispatch(req_cmd_buffs[id][index].handle,JOB_SIZE/DIV,1,1);

		req_cmd_buffs[id][index].submit(compute_queue,req_fences[id][index]);

		running[id][index] = true;*/
	};

	void get_results(int32_t id, int32_t index, ray_bundle &bundle) const
	{
		/*if(!running[id][index])
			throw std::runtime_error("This index was not requested!\n");

		VkResult result = vkWaitForFences(dev,1,&req_fences[id][index],VK_TRUE,UINT64_MAX);
		if(result != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for fences!\n");

		pos_buffs[id][index].copy_from_device(&pos_datas[id][index][0]);
		dir_buffs[id][index].copy_from_device(&dir_datas[id][index][0]);

		running[id][index] = false;*/
	};

	bool intersect(ray &r, hit_record &h_rec) const
	{
		/*for(int i = 0; i < 3; i++)
		{
			pos_data[0][i] = r.o[i];
			dir_data[0][i] = r.d[i];
		}

		vec3f o = r.o;
		vec3f d = r.d;

		submit_queue_and_get_results();

		if(pos_data[0].z != -1.0f)
		{
			if((uint32_t)pos_data[0].x >= m_tris->size() || (uint32_t)pos_data[0].x < 0)
				throw std::runtime_error("Invalid tri index!\n");

			tri* t = &m_tris->at(pos_data[0].x);
			vec3f s1 = t->b.p - t->a.p;
			h_rec.m_shape.set_material(m_mats->at(t->m));
			h_rec.tp = t;
			h_rec.wi = r.d;
			h_rec.pos = pos_data[0].y * r.d + r.o;
			h_rec.dist = pos_data[0].y;
			vec2f uv = vec2f(dir_data[0].x,dir_data[0].y);

			float b1 = uv.x;
			float b2 = uv.y;
			float b0 = 1.0f - b1 - b2;
			h_rec.bary = vec3f(b0,b1,b2);
			
			h_rec.sh_frame.n = normalize(t->a.n*b0+t->b.n*b1+t->c.n*b2);
			calc_shading_frame<float>(h_rec.sh_frame.n,s1,h_rec.sh_frame);

			return true;
		}
		else
		{
			return false;
		}*/
	};

	float8 intersect8(ray8 &r, hit_record8 &h_rec, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};

	bool occluded(const ray &r, const vec3f &pos) const
	{
		return false;

		/*ubo[0] = r.o;
		ubo2[0] = r.d;

		submit_queue_and_get_results();

		if(
			ubo[0].x == -1.0f && ubo[0].y == -1.0f && ubo[0].z == -1.0f &&
			ubo2[0].x == -1.0f && ubo2[0].y == -1.0f && ubo2[0].z == -1.0f
		)
		{
			return false;
		}

		if(ubo[0].y > r.mint)
		{
			return false;
		}

		return true;*/
	};

	//returns true on hits, false on misses
	float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};

	void intersect_n(ray_bundle &bundle)
	{
		for(int i = 0; i < JOB_SIZE; i++)
			t_data[i].r = bundle.r[i];

		submit_queue_and_get_results();

		for(int i = 0; i < JOB_SIZE; i++)
		{
			/*tri* t[8];
			for(int j = 0; j < 8; j++)
				t[j] = &m_tris->at(0);

			for(int j = 0; j < 8; j++)
				if(ubo[i*8+j].z != -1.0f)
				{
					t[j] = &m_tris->at((uint32_t)ubo[i*8+j].x);
					bundle.valid[i*8+j] = true;
				}
				else
					bundle.valid[i*8+j] = false;

			vec3f o[8], d[8];
			for(int j = 0; j < 8; j++)
			{
				o[j] = bundle.r[i*8+j].o;
				d[j] = bundle.r[i*8+j].d;
			}

			vec3f s1[8];
			for(int j = 0; j < 3; j++)
				s1[j] = t[j]->b.p - t[j]->a.p;

			float8 b1, b2, b0;
			vec3f n[8];

			for(int j = 0; j < 8; j++)
			{
				bundle.h_rec[i*8+j].wi = bundle.r[i*8+j].d;
				bundle.h_rec[i*8+j].pos = fmadd(vec3f(ubo[i*8+j].y),d[j],o[j]);
				bundle.h_rec[i*8+j].dist = ubo[i*8+j].y;
			}

			for(int j = 0; j < 8; j++)
			{
				b1[j] = ubo2[i*8+j].x;
				b2[j] = ubo2[i*8+j].y;
				b0[j] = 1.0f - b1[j] - b2[j];
			}

			for(int j = 0; j < 8; j++)
			{	
				bundle.h_rec[i*8+j].bary = vec3f(b0[j],b1[j],b2[j]);

				n[j] = normalize(t[j]->a.n*b0[j]+t[j]->b.n*b1[j]+t[j]->c.n*b2[j]);
				calc_shading_frame(n[j],s1[j],bundle.h_rec[i*8+j].sh_frame);
			}

			for(int j = 0; j < 8; j++)
			{
				bundle.h_rec[i*8+j].tp = t[j];
				bundle.h_rec[i*8+j].m_shape.set_material(m_mats->at(t[j]->m));
			}*/
			
			auto rx = r_data[i];
			uint32_t tri_idx = r_data[i].index;

			if(tri_idx > m_tris->size() - 1)
			{
				bundle.valid[i] = false;

				if(r_data[i].index != -1)
				{
					printf("error\n");
				}
			}
			else
			{
				tri* t = &m_tris->at(tri_idx);
				bundle.h_rec[i].tp = t;
				bundle.h_rec[i].m_shape.set_material(m_mats->at(t->m));

				vec2f uv = rx.uv;
				float b1 = uv.x;
				float b2 = uv.y;
				float b0 = 1.0f - b1 - b2;
				bundle.h_rec[i].bary = vec3f(b0,b1,b2);
				
				bundle.h_rec[i].pos = rx.pos;
				bundle.h_rec[i].sh_frame = rx.sh_frame;
				bundle.h_rec[i].wi = bundle.r[i].d;
				bundle.h_rec[i].dist = rx.distance;

				bundle.valid[i] = true;
			}
		}
	};

	//requires get_results to have been run
	void intersect_n_mt(int32_t id, ray_bundle &bundle)
	{
		/*for(int i = 0; i < JOB_SIZE; i++)
			t_datas[id][0][i].r = bundle.r[i];

		submit_queue_and_get_results(id);*/

		for(int i = 0; i < JOB_SIZE; i++)
		{
			auto rx = r_datas[id][0][i];

			uint32_t tri_idx = rx.index;
			if(tri_idx > m_tris->size() - 1)
			{
				bundle.valid[i] = false;
			}
			else
			{
				tri* t = &m_tris->at(tri_idx);
				bundle.h_rec[i].tp = t;
				bundle.h_rec[i].m_shape.set_material(m_mats->at(t->m));
				
				/*vec3f s1 = t->b.p - t->a.p;
				bundle.h_rec[i].wi = bundle.r[i].d;
				bundle.h_rec[i].pos = r_datas[id][0][i].distance * bundle.r[i].d + bundle.r[i].o;
				bundle.h_rec[i].dist = r_datas[id][0][i].distance;*/
				//vec2f uv = vec2f(r_datas[id][0][i].uv.x,r_datas[id][0][i].uv.y);

				vec2f uv = rx.uv;
				float b1 = uv.x;
				float b2 = uv.y;
				float b0 = 1.0f - b1 - b2;
				bundle.h_rec[i].bary = vec3f(b0,b1,b2);
				
				bundle.h_rec[i].pos = rx.pos;
				bundle.h_rec[i].sh_frame = rx.sh_frame;
				bundle.h_rec[i].wi = bundle.r[i].d;
				bundle.h_rec[i].dist = rx.distance;
				
				/*bundle.h_rec[i].sh_frame.n = normalize(t->a.n*b0+t->b.n*b1+t->c.n*b2);
				calc_shading_frame<float>(bundle.h_rec[i].sh_frame.n,s1,bundle.h_rec[i].sh_frame);*/
				
				bundle.valid[i] = true;
			}
		}
	};
};