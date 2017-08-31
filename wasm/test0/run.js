function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

function setupWasm(fileName, nameSpace, setupFct) {
	console.log('setupWasm ' + fileName)
	var mod = {}
	fetch(fileName)
		.then(response => response.arrayBuffer())
		.then(buffer => new Uint8Array(buffer))
		.then(binary => {
			mod['wasmBinary'] = binary
			mod['onRuntimeInitialized'] = function() {
				setupFct(mod, nameSpace)
				console.log('setupWasm end')
			}
			Module(mod)
		})
	return mod
}

var mcl = []
var module = setupWasm('add.wasm', mcl, function(mod, ns) {
	ns.add = mod.cwrap('add', 'number', ['number', 'number'])
	ns.str2int = mod.cwrap('str2int', 'number', ['number'])
	ns.str2int2 = mod.cwrap('str2int', 'number', ['string'])
	ns.int2str = mod.cwrap('int2str', 'number', ['number', 'number', 'number'])
})

function test_add() {
	var x = getValue('ret1')
	var y = mcl.add(x, 5)
	setStrValue('ret1', y)
}

function str2uint8array(s) {
	a = new Uint8Array(s.length)
	for (var i = 0; i < s.length; i++) {
		a[i] = s.charCodeAt(i)
	}
	return a
}

function str2int(d) {
	var p = module._malloc(d.length)
	console.log('d=' + d)
	module.HEAPU8.set(d, p)
	console.log('p=' + p)
	var v = mcl.str2int(p)
	console.log('v=' + v)
	module._free(p)
	return v
}

function test_str2int() {
	var s = getValue('inp2')
	console.log('s=' + s)
	var a = str2uint8array(s)
	console.log('a=' + a)
	var y = str2int(a)
	console.log('y=' + y)
	setStrValue('ret2', y)
}

function test_str2int2() {
	var s = getValue('inp3')
	console.log('s=' + s)
	var y = 0
	if (0) {
		y = mcl.str2int2(s)
	} else {
		var stack = module.Runtime.stackSave()
		var pos = module.Runtime.stackAlloc(s.length)
		for (var i = 0; i < s.length; i++) {
			module.HEAP8[pos + i] = s.charCodeAt(i)
		}
		y = mcl.str2int(pos)
		module.Runtime.stackRestore(stack)
	}
	console.log('y=' + y)
	setStrValue('ret3', y)
}

function test_int2str() {
	var v = getValue('inp4') | 0
	console.log('v=' + v)
	var maxBufSize = 10
	var s = ''
	{
		var stack = module.Runtime.stackSave()
		var pos = module.Runtime.stackAlloc(maxBufSize)
		console.log('pos=' + pos)
		var n = mcl.int2str(pos, maxBufSize, v)
		console.log('n=' + n)
		for (var i = 0; i < n; i++) {
			s += String.fromCharCode(module.HEAP8[pos + i])
		}
		module.Runtime.stackRestore(stack)
	}
	setStrValue('ret4', s)
}

