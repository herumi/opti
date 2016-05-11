import sys
sys.path.insert(0, '../../mcl/src')
import gen

fo = open('mul.ll', 'w')
gen.gen(fo, 'mul.txt', 32, [128])


