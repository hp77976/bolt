from bin import bolt_module

def error_out(text):
	print(text)
	exit()

def test_spectrum():
	s = bolt_module.spectrum(1.0)
	s *= 2.0
	s += s
	s *= s
	s /= bolt_module.spectrum(1.1)
	if bolt_module.is_black(s):
		error_out("is_black failed")

def test_vec3f():
	v = bolt_module.vec3f(0.0)
	v += 1.0
	v *= 2.0
	v.y = 5.0
	v.x -= 0.1
	if v.x < 1.89 and v.x > 1.91 :
		error_out(str(v.x))

test_vec3f()
test_spectrum()
print("pass")