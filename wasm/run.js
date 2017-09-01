function getValue(name) { return document.getElementsByName(name)[0].value }
function setValue(name, val) { document.getElementsByName(name)[0].value = val }
function getText(name) { return document.getElementsByName(name)[0].innerText }
function setText(name, val) { document.getElementsByName(name)[0].innerText = val }

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

var MCLBN_FP_UNIT_SIZE = 4

var module = setupWasm('mclbn.wasm', null, function(mod, ns) {
	define_exported_mcl(mod)
	define_extra_functions(mod)
	var r = mclBn_init(0, MCLBN_FP_UNIT_SIZE)
	setText('status', r ? 'err:' + r : 'ok')
})

function define_extra_functions(mod) {
	gen_setStr = function(func) {
		return function(x, buf, ioMode) {
			if (ioMode == null) { ioMode = 0 }
			var stack = mod.Runtime.stackSave()
			var pos = mod.Runtime.stackAlloc(buf.length)
			for (var i = 0; i < buf.length; i++) {
				mod.HEAP8[pos + i] = buf.charCodeAt(i)
			}
			r = func(x, pos, buf.length, ioMode)
			mod.Runtime.stackRestore(stack)
			if (r) console.log('err gen_setStr ' + r)
		}
	}
	gen_getStr = function(func) {
		return function(x, ioMode) {
			if (ioMode == null) { ioMode = 0 }
			var maxBufSize = 512
			var stack = mod.Runtime.stackSave()
			var pos = mod.Runtime.stackAlloc(maxBufSize)
			var n = func(pos, maxBufSize, x, ioMode)
			if (n < 0) {
				console.log('err gen_getStr')
				return ''
			}
			var s = ''
			for (var i = 0; i < n; i++) {
				s += String.fromCharCode(mod.HEAP8[pos + i])
			}
			mod.Runtime.stackRestore(stack)
			return s
		}
	}
	gen_deserialize = function(func) {
		return function(x, buf) {
			var stack = mod.Runtime.stackSave()
			var pos = mod.Runtime.stackAlloc(buf.length)
			if (typeof(buf) == "string") {
				for (var i = 0; i < buf.length; i++) {
					mod.HEAP8[pos + i] = buf.charCodeAt(i)
				}
			} else {
				for (var i = 0; i < buf.length; i++) {
					mod.HEAP8[pos + i] = buf[i]
				}
			}
			r = func(x, pos, buf.length)
			mod.Runtime.stackRestore(stack)
			if (r) console.log('err gen_deserialize ' + r)
		}
	}
	gen_serialize = function(func) {
		return function(x) {
			var maxBufSize = 512
			var stack = mod.Runtime.stackSave()
			var pos = mod.Runtime.stackAlloc(maxBufSize)
			var n = func(pos, maxBufSize, x)
			if (n < 0) {
				console.log('err gen_serialize')
				return ''
			}
			var a = new Uint8Array(n)
			for (var i = 0; i < n; i++) {
				a[i] = mod.HEAP8[pos + i]
			}
			mod.Runtime.stackRestore(stack)
			return a
		}
	}
	///////////////////////////////////////////////////////////////
	mclBnFr_create = function() {
		return mod._malloc(MCLBN_FP_UNIT_SIZE * 8)
	}
	mclBnFr_destroy = function(x) {
		mod._free(x)
	}
	mclBnFr_deserialize = gen_deserialize(_mclBnFr_deserialize)
	mclBnFr_setLittleEndian = gen_deserialize(_mclBnFr_setLittleEndian)
	mclBnFr_setStr = gen_setStr(_mclBnFr_setStr)
	mclBnFr_getStr = gen_getStr(_mclBnFr_getStr)
	mclBnFr_setHashOf = gen_deserialize(_mclBnFr_setHashOf)


	mclBnG1_create = function() {
		return mod._malloc(MCLBN_FP_UNIT_SIZE * 8 * 3)
	}
	mclBnG2_create = function() {
		return mod._malloc(MCLBN_FP_UNIT_SIZE * 8 * 2 * 3)
	}
	mclBnGT_create = function() {
		return mod._malloc(MCLBN_FP_UNIT_SIZE * 8 * 12)
	}
	mclBnG1_destroy = function(x) {
		mod._free(x)
	}
	mclBnG2_destroy = function(x) {
		mod._free(x)
	}
	mclBnGT_destroy = function(x) {
		mod._free(x)
	}
}

function test_mcl() {
	var x = mclBnFr_create()
	var y = mclBnFr_create()
	var z = mclBnFr_create()


	mclBnFr_setStr(x, getValue('x'))
//	mclBnFr_setInt(x, getValue('x') | 0)
	mclBnFr_setStr(y, getValue('y'))
	mclBnFr_add(z, x, y)
	setValue('ret_add', mclBnFr_getStr(z))
	mclBnFr_sub(z, x, y)
	setValue('ret_sub', mclBnFr_getStr(z))
	mclBnFr_mul(z, x, y)
	setValue('ret_mul', mclBnFr_getStr(z))
	if (!mclBnFr_isZero(y)) {
		mclBnFr_div(z, x, y)
		setValue('ret_div', mclBnFr_getStr(z))
	} else {
		setValue('ret_div', 'err : y is zero')
	}
	mclBnFr_setHashOf(x, getValue('hash_str'))
	setValue('ret_hash', mclBnFr_getStr(x))


	mclBnFr_destroy(x)
	mclBnFr_destroy(y)
	mclBnFr_destroy(z)
}
