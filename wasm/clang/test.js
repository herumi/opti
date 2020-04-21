(gen => {
  console.log(`exports=${exports}`)
  gen(exports)
})(exports => {
  console.log(`exports 2=${exports}`)
  const fs = require('fs')
  const buf = fs.readFileSync('./addsub.wasm')
  // 1 = 64KiB
  const memory = new WebAssembly.Memory({initial:1, maximum:1})
  console.log(`memory=${memory}`)
  const imports = {
    env : {
      memory: memory,
    }
  }
  WebAssembly.instantiate(buf, imports).then(
    ret => {
      exports.mod = ret.instance.exports
      const mem = exports.mod.memory.buffer
      exports.mem = mem
      const u8 = new Uint8Array(mem)
      console.log(`add=${exports.mod.add(12, 34)}`)
      console.log(`sub=${exports.mod.sub(12, 34)}`)
      const p = exports.mod.get()
      const p2 = exports.mod.get2()
      console.log(`p=${p} p2=${p2}`)
      exports.mod.setMem(u8, 10)
      for (let i = 0; i < 10; i++) {
        console.log(`mem[${i}]=${u8[i]}`)
      }
      for (let i = 0; i < 10; i++) {
        console.log(`mem[p+${i}]=${u8[p+i]}`)
      }
      console.log(`mem length=${mem.byteLength}`)
    }
  )
  return exports
})
