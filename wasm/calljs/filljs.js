
var _fillJS = function(p, n) {
	var a = new Uint8Array(n)
	const crypto = window.crypto || window.msCrypto
	crypto.getRandomValues(a)
	for (var i = 0; i < n; i++) {
		HEAP8[p + i] = a[i]
	}
}

var _call2JS = function(x, y) {
	return x + y
}
