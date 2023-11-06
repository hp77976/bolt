#include "accelerator.h"
#include "../core/scene.h"

accelerator::accelerator(scene* s)
{
	m_tris = s->get_tris();
	m_mats = s->get_mats();
	m_uv_maps = s->get_uvs();
};