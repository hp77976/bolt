from bin import bolt_module
import struct, mathutils

#this has to be done to make spectrum values work!
bolt_module.color().init()

rgb = bolt_module.vec3f(1.0,1.0,1.0)
diff = bolt_module.diffuse(rgb,1.0)
rs = bolt_module.random_sampler(0)

def fn(lo,count):
	min =  8.0
	max = -8.0
	b_rec = bolt_module.bsdf_record()
	b_rec.lo = lo

	pdf = 0.0
	for i in range(count):
		dir = b_rec.lo
		b_rec.li = bolt_module.vec3f(dir.x,dir.y,dir.z)
		diff.sample(b_rec,rs)
		pdf += b_rec.pdf
		if b_rec.pdf < min:
			min = b_rec.pdf
		if b_rec.pdf > max:
			max = b_rec.pdf

	print(pdf/count)
	print(min)
	print(max)

lo = bolt_module.vec3f(1.0,0.4,0.1)
lo = bolt_module.normalize(lo)
#fn(lo,1)
#fn(lo,10)
#fn(lo,100)
#fn(lo,1000)
#fn(lo,10000)
#fn(lo,100000)
#fn(lo,1000000)

scene = bolt_module.scene("/tmp/cornell_reference_3_6/cornell_reference_3_6.bsc")
embree = bolt_module.embree_accelerator(scene)
scene.set_accelerator(embree)

camera = scene.get_camera()
film = scene.get_film()

class mcmc:
	def __init__(self):
		self.state = 0