;; square(i32 i) -> i32
(module
  (func (export "square") (param $i i32) (result i32)
    (i32.mul
      (get_local $i)
      (get_local $i))))
