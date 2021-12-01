# My pwnable.kr coin1 solution
clone with `git clone --recurse-submodules https://github.com/GorgWorgington/pwnable_coin1_solution.git`  
One is written in C, the other in Rust

To build the C version, run `gcc -lm -o solution solution.c`  
Then run with `./solution 128.61.240.205 9007`

To build and run the Rust solution go into rust_solution/ and run  
`cargo run www.pwnable.kr 9007`  
you'll need cargo installed of course.