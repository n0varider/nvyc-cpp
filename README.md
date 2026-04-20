## nvyc

nvy is designed to be a systems language targeting the LLVM backend. The original implementation is in Java and can be found [here](https://github.com/n0varider/nvyc "here"), though it is no longer maintained in favor of the C++ implementation.

The rewrite is intended to provide access to the LLVM C++ API, enabling advanced optimizations and direct control of the compilation pipeline.

#### Features
The language uses a simple, C-adjacent syntax designed for minimal visual bloat. 
```
func add(int32 a, int32 b) -> int32 {
	return a + b;
}

func main() -> int32 {
	let x = 12;
	let y = 13;
	return add(x, y);
}
```

Variable declaration is unified under the `let` keyword and uses type inference unless an empty cast is provided, such as `let b = (int32);`. 
```
let a = int32[12]; 				// Array
let b = (int32);				    // Unallocated int32
let c = (int64) 12;			   // Casts
let d = list<int32>;		    // Generic constructor/allocation
```


