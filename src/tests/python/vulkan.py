from bin import bolt_module
import time

#this has to be done to make color conversions work!
bolt_module.color().init()

#scene = bolt_module.scene("/tmp/cornell_reference_3_6/cornell_reference_3_6.bsc")
scene = bolt_module.scene("/tmp/cornell_reference_3_6_vk/cornell_reference_3_6_vk.bsc")

accelerator = scene.get_accelerator()
camera = scene.get_camera()
film = scene.get_film()
sampler = bolt_module.random_sampler(0)

start = time.time()

#doing a full 16x16 block in a single command
def trace_block(ox,oy):
	successes = 0
	failures = 0
	root = 16
	size = root * root
	rb = bolt_module.ray_bundle()
	px_pos = []

	for x in range(root):
		for y in range(root):
			for i in range(1):
				px_pos.append(bolt_module.vec2f(x+1+ox,y+1+oy))

	for i in range(0,size):
		camera.sample_ray(rb.r[i],px_pos[i],bolt_module.vec2f(0.0,0.0))
	
	accelerator.intersect_n(rb)

	for i in range(0,size):
		if rb.valid[i] and rb.h_rec[i].dist > 0.0:
			li = bolt_module.spectrum(rb.h_rec[i].dist / 10.0)
			film.splat(li,px_pos[i])
			successes += 1
		else:
			li = bolt_module.spectrum(0.0)
			film.splat(li,px_pos[i])
			failures += 1
	print("s " + str(successes) + " f " + str(failures))

#for x in range(0,64,32):
	#for y in range(0,64,32):
		#trace_block(x,y)

trace_block(0,0)
trace_block(16,16)

end = time.time()
print(f"time: {end-start}")

film.normalize()
film.write("output.exr",False)