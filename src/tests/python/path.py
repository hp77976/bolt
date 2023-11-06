from bin import bolt_module

E_AREA = bolt_module.e_measure.E_AREA
E_SOLID_ANGLE = bolt_module.e_measure.E_SOLID_ANGLE
TM_RADIANCE = bolt_module.transport_mode.TM_RADIANCE

#this has to be done to make color conversions work!
bolt_module.color().init()

scene = bolt_module.scene("/tmp/cornell_reference_3_6/cornell_reference_3_6.bsc")
#embree = bolt_module.embree_accelerator(scene)
#scene.set_accelerator(embree)

accelerator = scene.get_accelerator()
camera = scene.get_camera()
film = scene.get_film()
res = film.size()
sampler = bolt_module.random_sampler(0)

res.x = 100
res.y = 100

for x in range(res.x):
	for y in range(res.y):
		li = bolt_module.spectrum(0.0)
		throughput = bolt_module.spectrum(1.0)
		
		r = bolt_module.ray()
		px_pos = bolt_module.vec2f(x,y) + sampler.get_vec2f() - 0.5

		camera.sample_ray(r,px_pos,bolt_module.vec2f(0.0,0.0))
		
		h_rec = bolt_module.hit_record()

		if accelerator.intersect(r,h_rec):
			for b in range(2):
				h_rec.wi = bolt_module.vec3f(0.0) - r.d
				mat = h_rec.get_material()
				
				d_rec = bolt_module.pd_record(h_rec,E_AREA)
				d_rec.ref_pos = h_rec.pos
				l_value = scene.sample_light_direct(d_rec,sampler.get_vec3f(),True)
				
				if bolt_module.is_black(l_value) is not True and mat.get_type() & 30:
					b_rec = bolt_module.bsdf_record(h_rec)
					b_rec.lo = h_rec.sh_frame.to_local(d_rec.dir)
					b_rec.mode = TM_RADIANCE

					e_weight = mat.eval(b_rec,E_SOLID_ANGLE)
					if bolt_module.is_black(e_weight):
						e_weight = bolt_module.spectrum(1.0)

					if bolt_module.is_black(e_weight) == False:
						e_pdf = mat.pdf(b_rec,sampler.get_vec2f(),E_SOLID_ANGLE)
						if d_rec.measure != E_SOLID_ANGLE: #TODO: check that light is on surface
							e_pdf = 0.0

						mi = bolt_module.spectrum(bolt_module.mi_weight(d_rec.pdf,e_pdf))
						li += throughput * l_value * e_weight * mi
				
				b_rec = bolt_module.bsdf_record(h_rec)
				weight = mat.sample(b_rec,sampler)
				if bolt_module.is_black(weight):
					break

				h_rec.from_bsdf(b_rec)
				r = bolt_module.ray(h_rec.pos,h_rec.wo)
				if accelerator.intersect(r,h_rec) == False:
					break
				
				throughput *= weight
		
		film.splat(li,px_pos)

print("fin")

film.normalize()
film.write("output.exr",True)

#
# if(proposed_pdf / current_pdf) > get_float():
#	current_pdf = proposed_pdf
#	proposed state is accepted
# else:
#	current state is accepted