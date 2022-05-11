package main

// 参考 https://darjun.github.io/2020/05/08/godailylib/rpc/

import (
	"fmt"
	"log"
	"net"
	// 由于其它语言不支持gob编解码方式，因此使用net/rpc库实现的RPC方法是没有办法进行跨语言调用
	"net/rpc"
	"sync"
)

//
// Common RPC request/reply definitions
//

const (
	OK       = "OK"
	ErrNoKey = "ErrNoKey"
)

type Err string

type PutArgs struct {
	Key   string
	Value string
}

type PutReply struct {
	Err Err
}

type GetArgs struct {
	Key string
}

type GetReply struct {
	Err   Err
	Value string
}

//
// Client
//
// 返回一个建立 tcp 连接的客户端，这个客户端使用完了就理解 close
func connect() *rpc.Client {
	client, err := rpc.Dial("tcp", ":1234")
	if err != nil {
		log.Fatal("dialing:", err)
	}
	return client
}

func get(key string) string {
	client := connect()
	args := GetArgs{key}
	reply := GetReply{}
	err := client.Call("KV.Get", &args, &reply)
	if err != nil {
		log.Fatal("error:", err)
	}
	client.Close()
	return reply.Value
}

func put(key string, val string) {
	client := connect()
	args := PutArgs{key, val}
	reply := PutReply{}
	err := client.Call("KV.Put", &args, &reply)
	if err != nil {
		log.Fatal("error:", err)
	}
	client.Close()
}

//
// Server
//

type KV struct {
	mu   sync.Mutex
	data map[string]string
}

func server() {
	kv := new(KV)
	kv.data = map[string]string{}
	rpcs := rpc.NewServer()
	/** 调用 rpcs.Register(kv) 会注册下面的两个关于 KV 的 GET PUT 方法
	 * rpc库对注册的方法有一定的限制，
	 * 方法必须满足签名func (t *T) MethodName(argType T1, replyType *T2) error：
	 * - 首先，方法必须是导出的（名字首字母大写）；
	 * - 其次，方法接受两个参数，必须是导出的或内置类型。
	 *   第一个参数表示客户端传递过来的请求参数，第二个是需要返回给客户端的响应。
	 *   第二个参数必须为指针类型（需要修改）；
     * - 最后，方法必须返回一个error类型的值。返回非nil的值，表示调用出错。
	 */
	rpcs.Register(kv)
	l, e := net.Listen("tcp", ":1234")
	if e != nil {
		log.Fatal("listen error:", e)
	}
	go func() {
		for {
			conn, err := l.Accept()
			if err == nil {
				// 我们可以自己接受连接，然后在此连接上应用 rpc 协议
				go rpcs.ServeConn(conn)
			} else {
				break
			}
		}
		l.Close()
	}()
}

// Get 和 Put 方法在客户端被调用
func (kv *KV) Get(args *GetArgs, reply *GetReply) error {
	// go 的 defer 运行我们在临界区上锁后立即书写解锁的代码
	kv.mu.Lock()
	defer kv.mu.Unlock()

	val, ok := kv.data[args.Key]
	if ok {
		reply.Err = OK
		reply.Value = val
	} else {
		reply.Err = ErrNoKey
		reply.Value = ""
	}
	return nil
}

func (kv *KV) Put(args *PutArgs, reply *PutReply) error {
	// go 的 defer 运行我们在临界区上锁后立即书写解锁的代码
	kv.mu.Lock()
	defer kv.mu.Unlock()

	kv.data[args.Key] = args.Value
	reply.Err = OK
	return nil
}

//
// main
//

func main() {
	server()

	put("subject", "6.824")
	fmt.Printf("Put(subject, 6.824) done\n")
	put("grade", "100")
	fmt.Printf("Put(grade, 100) done\n")
	fmt.Printf("get(subject) -> %s\n", get("subject"))
	fmt.Printf("get(subject) -> %s\n", get("subject"))
	fmt.Printf("get(grade) -> %s\n", get("grade"))
}