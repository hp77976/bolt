#include "../ext/pybind11/include/pybind11/pybind11.h"
#include "../ext/pybind11/include/pybind11/operators.h"
#include "../ext/pybind11/include/pybind11/stl.h"
#include "core/film.h"
#include "core/logging.h"
#include "core/scene.h"
#include "base/integrator.h"
#include "units/materials/diffuse.h"
#include "units/materials/glass.h"
#include "units/samplers/random.h"
#include "units/cameras/perspective.h"
#include "units/lights/point.h"
#include "units/filters/gaussian.h"
//#include "units/accelerators/embree.h"
//#include "units/accelerators/vk.h"

namespace py = pybind11;

struct instance
{
	scene* s = nullptr;
	vec4f* p = nullptr;

	instance(std::string path)
	{
		srand(time(nullptr));

		//init global color system (do not remove this)
		color co = color();
		co.init();

		log(LOG_STATUS,"INIT\n");

		s = new scene(path.c_str());
		film* f = s->get_integrator()->get_preview();

		int32_t px_count = f->m_size.x * f->m_size.y;
		p = new vec4f[px_count];

		s->render();

		delete f;
		//s->write_output("output.exr");
	};

	std::vector<float> get_rgba_film()
	{
		film* f = s->get_integrator()->get_preview();
		if(f == nullptr)
		{
			printf("tried to early access film!\n");
			return std::vector<float>();
		}

		std::vector<float> ret = {}; ret.reserve(f->m_size.x*f->m_size.y*4);
		f->to_rgba(p);
		for(int i = f->m_size.x; i > 0; i--)
			for(int j = 0; j < f->m_size.y; j++)
				for(int k = 0; k < 4; k++)
					ret.push_back(p[((i-1)*f->m_size.y)+(j)][k]);
		//f->to_rgba((vec4f*)ret.data());
		delete f;
		return ret;
	};

	bool poll() const {return s->get_integrator()->job_size() > 0;};

	void retire()
	{
		if(s->get_integrator()->job_size() == 0)
			s->complete();
	};

	void force_kill() {s->complete();};

	~instance()
	{
		//s->complete();
		//film* f = s->get_integrator()->get_preview();
		//f->write("output.exr",true);
		//delete f;
		delete s; delete p;
	};
};

/*
template <typename sampler_base = sampler>
class py_sampler : public sampler_base
{
	public:
	using sampler_base::sampler_base;
	float get_float() const override {PYBIND11_OVERRIDE_PURE(float,sampler_base,get_float);};
	vec2f get_vec2f() const override {PYBIND11_OVERRIDE_PURE(vec2f,sampler_base,get_vec2f);};
	vec3f get_vec3f() const override {PYBIND11_OVERRIDE_PURE(vec3f,sampler_base,get_vec3f);};
	vec4f get_vec4f() const override {PYBIND11_OVERRIDE_PURE(vec4f,sampler_base,get_vec4f);};

	//not used
	float8 get_float8() const override {PYBIND11_OVERRIDE_PURE(float8,sampler_base,get_float8);};
	vec2f8 get_vec2f8() const override {PYBIND11_OVERRIDE_PURE(vec2f8,sampler_base,get_vec2f8);};
	vec3f8 get_vec3f8() const override {PYBIND11_OVERRIDE_PURE(vec3f8,sampler_base,get_vec3f8);};
	vec4f8 get_vec4f8() const override {PYBIND11_OVERRIDE_PURE(vec4f8,sampler_base,get_vec4f8);};

	//not used
	sampler_base* clone(uint32_t offset) const override
	{PYBIND11_OVERRIDE_PURE(sampler_base*,sampler_base,clone,offset);};

	void generate(const vec2f &p, uint32_t index) override
	{PYBIND11_OVERRIDE_PURE(void,sampler_base,generate,p,index);};

	//not used
	void generate8(uint32_t index) override
	{PYBIND11_OVERRIDE_PURE(void,sampler_base,generate,index);};

	uint32_t get_index() const {PYBIND11_OVERRIDE(uint32_t,sampler_base,get_index);};

	void set_index(uint32_t i) {PYBIND11_OVERRIDE(void,sampler_base,set_index,i);};

	sampler_state save_state() const
	{PYBIND11_OVERRIDE(sampler_state,sampler_base,save_state);};

	void load_state(const sampler_state &s)
	{PYBIND11_OVERRIDE(void,sampler_base,load_state,s);};
};

template <typename material_base = material>
class py_material : public material_base
{
	public:
	using material_base::material_base;
	spectrum sample(bsdf_record &b_rec, sampler* const s) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,material_base,sample,b_rec,s);};

	spectrum eval(const bsdf_record &b_rec, e_measure measure) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,material_base,eval,b_rec,measure);};

	float pdf(const bsdf_record &b_rec, const vec2f &rng, e_measure measure) const override
	{PYBIND11_OVERRIDE_PURE(float,material_base,pdf,b_rec,rng,measure);};
};

template <typename emitter_base = emitter>
class py_emitter : public emitter_base
{
	public:
	using emitter_base::emitter_base;
	spectrum sample_direct(pd_record* const rec, const vec2f &rng) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,emitter_base,sample_direct,rec,rng);};

	spectrum8a sample_direct8(pd_record8* const rec, const vec2f8 &rng) const override
	{PYBIND11_OVERRIDE_PURE(spectrum8a,emitter_base,sample_direct8,rec,rng);};

	spectrum sample_direction(
		direction_record* d_rec, pd_record* p_rec, const vec2f &sample, const vec2f *extra
	) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,emitter_base,sample_direction,d_rec,p_rec,sample,extra);};

	spectrum8a sample_direction8(
		direction_record8* d_rec, pd_record8* p_rec, const vec2f8 &sample, const vec2f8 *extra
	) const override
	{PYBIND11_OVERRIDE_PURE(spectrum8a,emitter_base,sample_direction8,d_rec,p_rec,sample,extra);};

	spectrum sample_position(
		pd_record* p_rec, const vec2f &sample, const vec2f* extra
	) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,emitter_base,sample_position,p_rec,sample,extra);};

	spectrum8a sample_position8(
		pd_record8* p_rec, const vec2f8 &sample, const vec2f8* extra
	) const override
	{PYBIND11_OVERRIDE_PURE(spectrum8a,emitter_base,sample_position8,p_rec,sample,extra);};

	float pdf_direct(const pd_record &d_rec) const override
	{PYBIND11_OVERRIDE_PURE(float,emitter_base,pdf_direct,d_rec);};

	float pdf_direction(const direction_record &d_rec, const pd_record &p_rec) const override
	{PYBIND11_OVERRIDE_PURE(float,emitter_base,pdf_direction,d_rec,p_rec);};

	float pdf_position(const pd_record &d_rec) const override
	{PYBIND11_OVERRIDE_PURE(float,emitter_base,pdf_position,d_rec);};

	float pdf() const override
	{PYBIND11_OVERRIDE_PURE(float,emitter_base,pdf);};

	spectrum eval_direction(const direction_record &d_rec, const pd_record &p_rec) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,emitter_base,eval_direction,d_rec,p_rec);};

	spectrum eval_position(const pd_record &p_rec) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,emitter_base,eval_position,p_rec);};
};

template <typename light_base = light>
class py_light : public light_base
{
	public:
	using light_base::light_base;
};

template <typename camera_base = camera>
class py_camera : public camera_base
{
	public:
	using camera_base::camera_base;
	spectrum sample_ray(ray &r, const vec2f &px, const vec2f &s) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,camera_base,sample_ray,r,px,s);};

	spectrum8a sample_ray8(ray8 &r, const vec2f8 &px, const vec2f &s) const override
	{PYBIND11_OVERRIDE_PURE(spectrum,camera_base,sample_ray8,r,px,s);};

	void get_pdf(
		const ray &r, float dist, float x, float y,	float* const pdf, float* const flux
	) const override
	{PYBIND11_OVERRIDE_PURE(void,camera_base,get_pdf,r,dist,x,y,pdf,flux);};
};

template <typename filter_base = filter>
class py_filter : public filter_base
{
	public:
	using filter_base::filter_base;

	float eval(float x) const override {PYBIND11_OVERRIDE_PURE(float,filter_base,eval,x);};
};

template <typename base_accelerator = accelerator>
class py_accelerator : public base_accelerator
{
	public:
	using base_accelerator::base_accelerator;

	bool intersect(ray &r, hit_record &h_rec) const
	{PYBIND11_OVERRIDE_PURE(bool,base_accelerator,intersect,r,h_rec);};

	bool occluded(const ray &r, const vec3f &pos) const
	{PYBIND11_OVERRIDE_PURE(bool,base_accelerator,occluded,r,pos);};

	float8 intersect8(ray8 &r, hit_record8 &h_rec, int32_t* valid) const
	{PYBIND11_OVERRIDE_PURE(float8,base_accelerator,intersect8,r,h_rec,valid);};

	float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const
	{PYBIND11_OVERRIDE_PURE(float8,base_accelerator,occluded8,r,pos,valid);};

	void intersect_n(ray_bundle &rb)
	{PYBIND11_OVERRIDE_PURE(void,base_accelerator,intersect_n,rb);};

	void intersect_n_mt(int i, ray_bundle &rb)
	{PYBIND11_OVERRIDE_PURE(void,base_accelerator,intersect_n_mt,i,rb);};
};*/

PYBIND11_MODULE(bolt_module,m)
{
	/*{py::class_<scene>(m,"scene")
		.def(py::init<const char*>())
		.def("get_film",&scene::get_film,py::return_value_policy::reference)
		.def("get_sampler",&scene::get_sampler,py::return_value_policy::reference)
		//.def("get_integrator",&scene::get_integrator)
		.def("get_accelerator",&scene::get_accelerator,py::return_value_policy::reference)
		.def("set_accelerator",&scene::set_accelerator,py::return_value_policy::reference)
		.def("get_camera",&scene::get_camera,py::return_value_policy::reference)
		.def("get_random_light",&scene::get_random_light,py::return_value_policy::reference)
		.def("sample_light_direct",&scene::sample_light_direct);}

	{py::class_<ray_bundle>(m,"ray_bundle")
		.def(py::init<>())
		.def_readwrite("r",&ray_bundle::r)
		.def_readwrite("h_rec",&ray_bundle::h_rec)
		.def_readwrite("valid",&ray_bundle::valid);}

	{py::class_<accelerator,py_accelerator<>>(m,"accelerator")
		.def(py::init<scene*>())
		.def("intersect",&accelerator::intersect)
		.def("occluded",&accelerator::occluded)
		.def("intersect_n",&accelerator::intersect_n);}*/
	
	/*{py::class_<embree_accelerator,accelerator>(m,"embree_accelerator")
		.def(py::init<scene*>());}*/

	/*{py::class_<vk_accelerator,accelerator>(m,"vk_accelerator")
		.def(py::init<scene*>());}*/

	{py::class_<instance>(m,"instance")
		.def(py::init<const std::string &>())
		.def("get_rgb_film",&instance::get_rgba_film,"yeah")
		.def("poll",&instance::poll,"poll")
		.def("retire",&instance::retire,"retire")
		.def("force_kill",&instance::force_kill,"force_kill");}

	/*
	{py::class_<spectrum>(m,"spectrum")
		.def(py::init<>())
		.def(py::init<float>())
		//.def(py::init<float,float,float>())
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self / py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= py::self)
		.def(py::self /= py::self)
		.def(float() * py::self)
		.def(float() / py::self)
		.def(py::self * float())
		.def(py::self / float())
		.def(py::self *= float())
		.def(py::self /= float());}

	{py::class_<vec2i>(m,"vec2i")
		.def(py::init<>())
		.def(py::init<int32_t>())
		.def(py::init<int32_t,int32_t>())
		.def_readwrite("x",&vec2i::x)
		.def_readwrite("y",&vec2i::y)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self / py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= py::self)
		.def(py::self /= py::self)
		.def(int32_t() + py::self)
		.def(int32_t() - py::self)
		.def(int32_t() * py::self)
		.def(int32_t() / py::self)
		.def(py::self + int32_t())
		.def(py::self - int32_t())
		.def(py::self * int32_t())
		.def(py::self / int32_t())
		.def(py::self += int32_t())
		.def(py::self -= int32_t())
		.def(py::self *= int32_t())
		.def(py::self /= int32_t());}

	{py::class_<filter,py_filter<>>(m,"filter")
		.def("eval_discretized",&filter::eval_discretized)
		.def("eval",&filter::eval);}

	{py::class_<gaussian_filter,filter>(m,"gaussian_filter")
		.def(py::init<float,float,int32_t>());}

	{py::class_<film>(m,"film")
		.def(py::init<const vec2i&,const vec2i&, const vec2i&, filter*>())
		.def("size",&film::size)
		.def("normalize",py::overload_cast<>(&film::normalize))
		.def("normalize",py::overload_cast<int>(&film::normalize))
		.def("splat",&film::splat)
		.def("write",&film::write);}

	{py::class_<vec2f>(m,"vec2f")
		.def(py::init<>())
		.def(py::init<float>())
		.def(py::init<float,float>())
		.def_readwrite("x",&vec2f::x)
		.def_readwrite("y",&vec2f::y)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self / py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= py::self)
		.def(py::self /= py::self)
		.def(float() + py::self)
		.def(float() - py::self)
		.def(float() * py::self)
		.def(float() / py::self)
		.def(py::self + float())
		.def(py::self - float())
		.def(py::self * float())
		.def(py::self / float())
		.def(py::self += float())
		.def(py::self -= float())
		.def(py::self *= float())
		.def(py::self /= float());}

	{py::class_<vec3f>(m,"vec3f")
		.def(py::init<>())
		.def(py::init<float>())
		.def(py::init<float,float,float>())
		.def_readwrite("x",&vec3f::x)
		.def_readwrite("y",&vec3f::y)
		.def_readwrite("z",&vec3f::z)
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self / py::self)
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= py::self)
		.def(py::self /= py::self)
		.def(float() + py::self)
		.def(float() - py::self)
		.def(float() * py::self)
		.def(float() / py::self)
		.def(py::self + float())
		.def(py::self - float())
		.def(py::self * float())
		.def(py::self / float())
		.def(py::self += float())
		.def(py::self -= float())
		.def(py::self *= float())
		.def(py::self /= float());}

	{py::class_<transform<float>>(m,"transform")
		.def(py::init<>())
		.def(py::init<const matrix4x4<float>>())
		.def(py::init<const matrix4x4<float> &, const matrix4x4<float> &>())
		.def(py::self * py::self);}

	{py::class_<vertex>(m,"vertex")
		.def(py::init<>())
		.def(py::init<const vec3f &, const vec3f &>())
		.def_readwrite("p",&vertex::p)
		.def_readwrite("n",&vertex::n);}

	{py::class_<tri>(m,"tri")
		.def(py::init<>())
		.def(py::init<const vertex &,const vertex &,const vertex &,int64_t,int32_t>())
		.def_readwrite("a",&tri::a)
		.def_readwrite("b",&tri::b)
		.def_readwrite("c",&tri::c)
		.def_readwrite("i",&tri::i)
		.def_readwrite("m",&tri::m);}

	{py::class_<ray>(m,"ray")
		.def(py::init<>())
		.def(py::init<const vec3f &, const vec3f &>())
		.def_readwrite("o",&ray::o)
		.def_readwrite("d",&ray::d)
		.def_readwrite("mint",&ray::mint)
		.def_readwrite("maxt",&ray::maxt);}

	{py::class_<frame<>>(m,"frame")
		.def(py::init<>())
		.def(py::init<const vec3f &>())
		.def(py::init<const vec3f &, const vec3f &, const vec3f &>())
		.def_readwrite("s",&frame<>::s)
		.def_readwrite("t",&frame<>::t)
		.def_readwrite("n",&frame<>::n)
		.def("to_local",&frame<>::to_local)
		.def("to_world",&frame<>::to_world);}

	{py::class_<pd_record>(m,"pd_record")
		.def(py::init<>())
		.def(py::init<const vec3f&, e_measure>())
		.def(py::init<const hit_record&, e_measure>())
		.def_readwrite("pos",&pd_record::pos)
		.def_readwrite("norm",&pd_record::norm)
		.def_readwrite("dir",&pd_record::dir)
		.def_readwrite("ref_pos",&pd_record::ref_pos)
		.def_readwrite("ref_norm",&pd_record::ref_norm)
		.def_readwrite("uv",&pd_record::uv)
		.def_readwrite("dist",&pd_record::dist)
		.def_readwrite("pdf",&pd_record::pdf)
		.def_readwrite("measure",&pd_record::measure)
		.def_readwrite("is_light",&pd_record::is_light);}

	{py::class_<hit_record>(m,"hit_record")
		.def(py::init<>())
		.def_readwrite("tri",&hit_record::tp,py::return_value_policy::reference)
		//.def_readwrite("m_shape",&hit_record::m_shape)
		.def_readwrite("pos",&hit_record::pos)
		.def_readwrite("dist",&hit_record::dist)
		.def_readwrite("wi",&hit_record::wi)
		.def_readwrite("wo",&hit_record::wo)
		.def_readwrite("uv",&hit_record::uv)
		.def_readwrite("bary",&hit_record::bary)
		.def_readwrite("sh_frame",&hit_record::sh_frame)
		.def("fill_records",&hit_record::fill_records)
		.def("from_bsdf",&hit_record::from_bsdf)
		.def("get_material",&hit_record::get_material,py::return_value_policy::reference);}

	{py::class_<bsdf_record>(m,"bsdf_record")
		.def(py::init<>())
		.def(py::init<const hit_record &>())
		.def_readwrite("li",&bsdf_record::li)
		.def_readwrite("lo",&bsdf_record::lo)
		.def_readwrite("pdf",&bsdf_record::pdf)
		.def_readwrite("eta",&bsdf_record::eta)
		.def_readwrite("bary",&bsdf_record::bary)
		.def_readwrite("mode",&bsdf_record::mode)
		.def_readwrite("uv",&bsdf_record::uv)
		.def_readwrite("tri_index",&bsdf_record::tri_index)
		.def_readwrite("req_type",&bsdf_record::req_type)
		.def_readwrite("req_component",&bsdf_record::req_component)
		.def_readwrite("sampled_type",&bsdf_record::sampled_type)
		.def_readwrite("sampled_component",&bsdf_record::sampled_component);}

	{py::class_<sampler_state>(m,"sampler_state")
		.def(py::init<>());}

	{py::class_<sampler,py_sampler<>>(m,"sampler")
		.def(py::init<uint32_t>())
		.def("get_float",&sampler::get_float)
		.def("get_vec2f",&sampler::get_vec2f)
		.def("get_vec3f",&sampler::get_vec3f)
		.def("get_vec4f",&sampler::get_vec4f)
		.def("generate",&sampler::generate)
		.def("save_state",&sampler::save_state)
		.def("load_state",&sampler::load_state)
		.def("set_index",&sampler::set_index)
		.def("get_index",&sampler::get_index);}

	{py::class_<random_sampler,sampler>(m,"random_sampler")
		.def(py::init<uint32_t>());}

	{py::class_<material,py_material<>>(m,"material")
		.def(py::init<>())
		.def("sample",&material::sample)
		.def("eval",&material::eval)
		.def("pdf",&material::pdf)
		.def("get_type",&material::get_type);}

	{py::class_<diffuse,material>(m,"diffuse")
		.def(py::init<const vec3f &,float>());}

	{py::class_<color>(m,"color")
		.def(py::init<>())
		.def("init",&color::init);}

	{py::class_<emitter,py_emitter<>>(m,"emitter")
		.def(py::init<const transform<float> &>())
		.def("sample_direct",&emitter::sample_direct)
		.def("sample_direction",&emitter::sample_direction)
		.def("sample_position",&emitter::sample_position)
		.def("pdf_direct",&emitter::pdf_direct)
		.def("pdf_direction",&emitter::pdf_direction)
		.def("pdf_position",&emitter::pdf_position)
		.def("pdf",&emitter::pdf)
		.def("eval_direction",&emitter::eval_direction)
		.def("eval_position",&emitter::eval_position)
		.def("get_position",&emitter::get_position)
		.def("is_on_surface",&emitter::is_on_surface)
		.def("get_direct_measure",&emitter::get_direct_measure)
		.def("needs_direct_sample",&emitter::needs_direct_sample)
		.def("needs_position_sample",&emitter::needs_position_sample)
		.def("needs_direction_sample",&emitter::needs_direction_sample);};

	{py::class_<light,emitter>(m,"light");}

	{py::class_<point_light,light>(m,"point_light");}

	{py::class_<camera,py_camera<>>(m,"camera")
		.def("sample_ray",&camera::sample_ray)
		.def("get_sample_position",&camera::get_sample_position)
		.def("get_pdf",&camera::get_pdf);}

	{py::class_<perspective_camera,camera>(m,"perspective_camera")
		.def(py::init<float,float,float,const vec2f&,float,const transform<float>&,film*,float>());}

	{py::enum_<transport_mode>(m,"transport_mode",py::arithmetic())
		.value("TM_RADIANCE",TM_RADIANCE)
		.value("TM_IMPORTANCE",TM_IMPORTANCE);}

	{py::enum_<e_measure>(m,"e_measure",py::arithmetic())
		.value("E_INVALID_MEASURE",E_INVALID_MEASURE)
		.value("E_SOLID_ANGLE",E_SOLID_ANGLE)
		.value("E_LENGTH",E_LENGTH)
		.value("E_AREA",E_AREA)
		.value("E_DISCRETE",E_DISCRETE);}

	m.def("normalize",[](const vec3f &f){return normalize(f);});
	m.def("cross",[](const vec3f &a, const vec3f &b){return cross(a,b);});
	m.def("dot",[](const vec3f &a, const vec3f &b){return dot(a,b);});

	m.def("inverse",[](const transform<float> &t){return inverse(t);});
	m.def("transform_point",[](const transform<float> &t, const vec3f &p){return transform_point(t,p);});
	m.def("transform_point_affine",
		[](const transform<float> &t, const vec3f &p){return transform_point_affine(t,p);});
	m.def("transform_vector",[](const transform<float> &t, const vec3f &p){return transform_vector(t,p);});
	m.def("transform_normal",[](const transform<float> &t, const vec3f &p){return transform_normal(t,p);});
	m.def("translate",[](const vec3f &v){return translate(v);});
	m.def("scale",[](const vec3f &v){return scale(v);});
	m.def("rotate",[](const vec3f &v, float angle){return rotate(v,angle);});
	m.def("perspective",[](float fov, float near, float far){return perspective(fov,near,far);});
	m.def("look_at",[](const vec3f &p,const vec3f &t, const vec3f &u){return look_at(p,t,u);});

	m.def("spectrum_to_linear_rgb",&spectrum_to_linear_rgb);
	m.def("luminance",&luminance);
	m.def("is_black",[](const spectrum &s){return is_black(s);});
	
	m.def("mi_weight",&mi_weight<float>);*/

	m.doc() = "you don't get to have docs";

	m.attr("__version__") = "1";
};