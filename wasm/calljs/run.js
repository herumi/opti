function getValue(name) { return document.getElementsByName(name)[0].value }
function setValue(name, val) { document.getElementsByName(name)[0].value = val }
function getText(name) { return document.getElementsByName(name)[0].innerText }
function setText(name, val) { document.getElementsByName(name)[0].innerText = val }

let mod = {}
mod.fillJS = function(p, n) {
	a = new Uint8Array(n)
	const crypto = window.crypto || window.msCrypto
	crypto.getRandomValues(a)
	for (let i = 0; i < n; i++) {
		mod.HEAP8[p + i] = a[i]
	}
}

mod.callAdd1 = function(x, y) {
	return x + y
}

function bench(msg, count, func) {
	let start = Date.now()
	for (let i = 0; i < count; i++) {
		func()
	}
	let end = Date.now()
	let t = (end - start) / count
	setText(msg, t)
}

function setupWasm(fileName, setupFct) {
	console.log('setupWasm ' + fileName)
	fetch(fileName)
		.then(response => response.arrayBuffer())
		.then(buffer => new Uint8Array(buffer))
		.then(binary => {
			mod['wasmBinary'] = binary
			mod['onRuntimeInitialized'] = function() {
				setupFct()
				console.log('setupWasm end')
			}
			Module(mod)
		})
}

setupWasm('fill.wasm', function() {
	fill1 = mod.cwrap('fill1', 'null', ['number', 'number'])
	fill2 = mod.cwrap('fill2', 'null', ['number', 'number'])
	call0 = mod.cwrap('call0', 'number', ['number', 'number'])
	call1 = mod.cwrap('call1', 'number', ['number', 'number'])
	call2 = mod.cwrap('call2', 'number', ['number', 'number'])
})

function test_fill() {
	let n = 8
	let stack = mod.Runtime.stackSave()
	let pos = mod.Runtime.stackAlloc(n)
	console.log('pos=' + pos)
	console.log('fill1')
	fill1(pos, n)
	for (let i = 0; i < n; i++) {
		console.log(i + ':' + mod.HEAP8[pos + i])
	}
	console.log('fill2')
	fill2(pos, n)
	for (let i = 0; i < n; i++) {
		console.log(i + ':' + mod.HEAP8[pos + i])
	}
	bench('bench_fill1', 10000, () => { fill1(pos, n) })
	bench('bench_fill2', 10000, () => { fill2(pos, n) })
	mod.Runtime.stackRestore(stack)
	console.log('a=' + a)
}

function test() {
	let x = 8
	let y = 10
	let z = 0
	z = call0(x, y)
	setText('test0', (x + y == z))
	z = call1(x, y)
	setText('test1', (x + y == z))
	z = call2(x, y)
	setText('test2', (x + y == z))
}

function test_bench() {
	var x = 3
	var y = 4
	const C = 1000000
	bench('bench_testJS', C, () => { x = x + y })
	bench('bench_test0', C, () => { x = call0(x, y) })
	bench('bench_test1', C, () => { x = call1(x, y) })
	bench('bench_test2', C, () => { x = call2(x, y) })
}
