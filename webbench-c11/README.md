# Describle:
- 这个版本的WebBench源自于基本版本的WebBench(https://github.com/EZLippi/WebBench).EZLippi的版本由c语言,进程和进程件通信的管道组成,我的这个版本将原作者的WebBench更改为c++,线程和锁机制,起初想做一个线上的网站压测工具,涉及资源问题,故而使用线程。



# 使用参数:
"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default 1.MAX 100\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"

目前-p没有参数测试,其余参数都正确.