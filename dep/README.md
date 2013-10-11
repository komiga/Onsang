
## Dependencies!

Symlink/clone/download the following libraries here (take care they go
into the proper directories):

* [duct++](/komiga/duct-cpp) into `duct/`:
  `git clone git://github.com/komiga/duct-cpp.git duct/`

* [murk](/komiga/murk) into `murk/`:
  `git clone git://github.com/komiga/murk.git murk/`

* [trait_wrangler](/komiga/trait_wrangler) into `trait_wrangler/`:
  `git clone git://github.com/komiga/trait_wrangler.git trait_wrangler/`

* [ceformat](/komiga/ceformat) into `ceformat/`:
  `git clone git://github.com/komiga/ceformat.git ceformat/`

* [Hord](/komiga/hord) into `hord/`:
  `git clone git://github.com/komiga/hord.git hord/`

And compile-copy/download/magically spontaniate the following libraries:

* [ØMQ](http://zeromq.org/intro:get-the-software) 4.0.1 into `zeromq/`:
  - Compile with [libsodium](https://github.com/jedisct1/libsodium) support
  - Copy `zmq.h` and `libzmq.a` (or `libzmq.so`)
