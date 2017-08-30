import sys, re

#RE_PROTOTYPE = re.compile(r'MCLBN_DLL_API\s\w\s\w\([^)]*\);')
RE_PROTOTYPE = re.compile(r'MCLBN_DLL_API\s(\w*)\s(\w*)\(([^)]*)\);')
def export_functions(fileName, mode):
	with open(fileName, 'rb') as f:
		if mode == 'js':
			print 'function define_exported_functions(mod) {'
		comma = ''
		for line in f.readlines():
			p = RE_PROTOTYPE.search(line)
			if p:
				ret = p.group(1)
				name = p.group(2)
				arg = p.group(3)
				if mode == 'js':
					retType = 'null' if ret == 'void' else 'number'
					if arg == '' or arg == 'void':
						paramType = '[]'
					else:
						paramType = '[' + ("'number', " * len(arg.split(','))) + ']'
					print "{0} = mod.cwrap('{1}', '{2}', {3})".format(name, name, retType, paramType)
				else:
					print comma + "'_" + name + "'",
					if comma == '':
						comma = ','
		if mode == 'js':
			print '}'

def main():
	args = len(sys.argv)
	mode = ''
	if args <= 1:
		print('export_functions header [-js]')
		sys.exit(1)
	if args == 3 and sys.argv[2] == '-js':
		mode = 'js'
	export_functions(sys.argv[1], mode)

if __name__ == '__main__':
    main()

			
