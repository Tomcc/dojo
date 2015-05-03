
coords = ['x', 'y', 'z']

for a in coords:
	for b in coords:
		for c in coords:
			print("\t\tVector " + a + b + c + "() const {")
			print("\t\t\treturn{" + a +", " + b + ", " + c + "};")
			print("\t\t}")
