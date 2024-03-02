# rant-c

[Rant](https://rant.anton2920.ru) is the simple [X](https://x.com) clone built using an experimental high-performance HTTP server. It allows you to watch you my short messages, search for them either on the client or on the server and subscribe to updates using [RSS](https://rant.anton2920.ru/rss).

This project is a rewrite of original server used by [rant-go](https://github.com/anton2920/rant-go) in C and inline assembly. It's build without using any libraries, relying only on operating system's system call interface. Currently only `freebsd/amd64` is supported.

HTTP server supports only `GET` requests. Query parameters could be included but must be parsed by hand. It also supports pipelining, infinite keep-alives and a lot of concurrent connections.

For a complete Go version of this server, see [rant-go](https://github.com/anton2920/rant-go).

# Performance

Using [wrk](https://github.com/wg/wrk) I've measured performance of `net/http` and my server using rules of [Plaintext](https://github.com/TechEmpower/FrameworkBenchmarks/wiki/Project-Information-Framework-Tests-Overview#plaintext) benchmark. Results on my `i7 6700K` with 32 GiB RAM, sending 16 pipelined requests and receiving `text/plain` `Hello, world\n` responses are following:

```
net/http:

$ ./wrk -t 4 -c 256 -d 15 --script plaintext.lua http://localhost:7070/plaintext -- 16
Running 15s test @ http://localhost:7070/plaintext
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    11.67ms   21.01ms 358.21ms   95.32%
    Req/Sec   151.80k    13.82k  182.68k    73.17%
  9090809 requests in 15.05s, 1.11GB read
Requests/sec: 603914.99
Transfer/sec:     75.45MB

rant-c:

$ ./wrk -t 5 -c 256 -d 15 --script plaintext.lua http://localhost:7070/plaintext -- 16
Running 15s test @ http://localhost:7070/plaintext
  5 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.88ms   12.80ms 452.16ms   97.08%
    Req/Sec     1.05M   144.17k    1.86M    79.95%
  78760640 requests in 15.10s, 9.54GB read
Requests/sec: 5215131.01
Transfer/sec:    646.56MB
```

Both server and `wrk` were running on one computer. For each server  `wrk` parameters were selected to produce the best results.

Note that currently `rant-c` shows worse results than `rant-go`, primarily because:

- `rant-go` uses SIMD for `FindChar()`, which doubles the performance of this function.
- `rant-go` is compiled with profile-guided optimization, which increases performance by a couple per-cents.
- `rant-c` is an unfinished project, there's a room for experimentation and improvements.

# Copyright

Pavlovskii Anton, 2023-2024 (MIT). See [LICENSE](LICENSE) for more details.
