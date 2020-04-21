const mod = {}

const fs = require('fs')
const buf = fs.readFileSync('./addsub.wasm')
// 1 = 64KiB
const memory = new WebAssembly.Memory({initial:1, maximum:100})
console.log(`memory=${memory}`)
const imports = {
  env : {
    memory: memory,
  }
}
WebAssembly.instantiate(buf, imports).then(
  ret => {
    const exports = ret.instance.exports
    const mem = ret.instance.exports.memory.buffer
    const u8 = new Uint8Array(mem)
    console.log(`add=${exports.add(12, 34)}`)
    console.log(`sub=${exports.sub(12, 34)}`)
    console.log(u8)
    exports.setMem(u8, 10)
    for (let i = 0; i < 10; i++) {
      console.log(`mem[${i}]=${u8[i]}`)
    }
    console.log(`mem length=${mem.byteLength}`)
  }
)
