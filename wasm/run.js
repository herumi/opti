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

var module = setupWasm('mclbn.wasm', null, function(mod, ns) {
	define_exported_mcl(mod)
	define_extra_functions(mod)
	var r = mclBn_init(0, 4)
	console.log('mclBn_init=' + r)
})

function define_extra_functions(mod) {
	Fr_create = function() {
		return mod._malloc(32)
	}
	Fr_destroy = function(x) {
		mod._free(x)
	}
	Fr_setStr = function(x, buf, ioMode) {
		if (ioMode == null) { ioMode = 10 }
		var stack = mod.Runtime.stackSave()
		var pos = mod.Runtime.stackAlloc(buf.length)
		for (var i = 0; i < buf.length; i++) {
			mod.HEAP8[pos + i] = buf.charCodeAt(i)
		}
		r = mclBnFr_setStr(x, pos, buf.length, ioMode)
		mod.Runtime.stackRestore(stack)
		if (r) console.log('mclBnFr_setStr err ' + r)
	}
	Fr_getStr = function(x, ioMode) {
		if (ioMode == null) { ioMode = 10 }
		var maxBufSize = 512
		var stack = mod.Runtime.stackSave()
		var pos = mod.Runtime.stackAlloc(maxBufSize)
		var n = mclBnFr_getStr(pos, maxBufSize, x, ioMode)
		var s = ''
		for (var i = 0; i < n; i++) {
			s += String.fromCharCode(mod.HEAP8[pos + i])
		}
		mod.Runtime.stackRestore(stack)
		return s
	}
	Fr_add = function(z, x, y) {
		mclBnFr_add(z, x, y)
	}
}

function test_mcl() {
	var x = Fr_create()
	var y = Fr_create()
	var z = Fr_create()
	Fr_setStr(x, getValue('x'))
	Fr_setStr(y, getValue('y'))
	mclBnFr_add(z, x, y)
	setStrValue('z', Fr_getStr(z))
	Fr_destroy(x)
	Fr_destroy(y)
	Fr_destroy(z)
}
