function getValue(name) { return document.getElementsByName(name)[0].value }
function setStrValue(name, val) { document.getElementsByName(name)[0].value = val }

var module

function setupWasm(fileName) {
	console.log('setupWasm:' + fileName)
	fetch(fileName)
		.then(response => response.arrayBuffer())
		.then(buffer => new Uint8Array(buffer))
		.then(binary => {
			var moduleArgs = {
				wasmBinary: binary,
				onRuntimeInitialized: function () {
					define_exported_functions(module)
					console.log('setup end')
				}
			}
			module = Module(moduleArgs)
		})
}

/*
setupWasm('add.wasm', mcl, function setup(fct) {
	fct.add = module.cwrap('add', 'number', ['number', 'number'])
	fct.str2int = module.cwrap('str2int', 'number', ['number'])
})
*/
setupWasm('mclbn.wasm')

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
	var s = getValue('inp1')
	console.log('s=' + s)
	var a = str2uint8array(s)
	console.log('a=' + a)
	var y = str2int(a)
	console.log('y=' + y)
	setStrValue('ret2', y)
}

function test_mcl() {
	var r = mclBn_init(0, 4)
	console.log('r=' + r)
	r = mclBn_getOpUnitSize()
	console.log('r=' + r)
}
