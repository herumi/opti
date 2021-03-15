

const mod = {}

async function init () {
  const fs = require('fs')
  const buf = fs.readFileSync('./add.wasm')
  const memory = await new WebAssembly.Memory({initial:100})
  const imports = {
    env : {
      addJS : (x, y) => { return x + y },
      memory: memory,
    }
  }
  const ret = await WebAssembly.instantiate(buf, imports)
  return ret.instance.exports
} 

(async () => {
  mod.exports = await init()
  const exports = mod.exports
  mod.u32 = new Uint32Array(exports.memory.buffer)
  mod.u8 = new Uint8Array(exports.memory.buffer)
  const x = 123
  const y = exports.add(x, 5)
  console.log(`y=${y}`)
  console.log(`subsub=${exports.subsub(2, 9)}`)
  console.log(`callJS=${exports.callJS(1000)}`)
  mod.u32[0] = 3
  mod.u32[1] = 5
  console.log(`addmem=${exports.addmem(mod.u32, 2)}`)
  for (let i = 0; i < 5; i++) {
    console.log(`counter1=${exports.getCount1()}`)
    console.log(`counter2=${exports.getCount2()}`)
  }
  console.log('before')
  for (let i = 0; i < 10; i++) {
    console.log(`mod.u8[${i}]=${mod.u8[i]}`)
  }
  const memPos = 2
  exports.setn(memPos, 3)
  console.log('after')
  for (let i = 0; i < 10; i++) {
    console.log(`mod.u8[${i}]=${mod.u8[i]}`)
  }
/*
  console.log('aaa')
  {
    const m = new Uint32Array(exports.memory.buffer, 4, 2)
    m[0] = 0x01020304
    m[1] = 0x0a0b0c0d
    m[2] = 0x99999999 // not written
  }
  for (let i = 0; i < 20; i++) {
    console.log(`mod.u8[${i}]=${mod.u8[i]}`)
  }
*/
})()
