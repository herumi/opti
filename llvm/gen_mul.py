import sys
sys.path.insert(0, '../../mcl/src')
import gen

fo = open(sys.argv[1], 'w')
unit=int(sys.argv[2])
gen.gen(fo, 'mul.txt', unit, [unit*4])


