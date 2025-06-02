package main

import "sync"

func main() {
	var wg sync.WaitGroup
	for i := 0; i < 5; i++ {
		wg.Add(1)
		go func() {
			sendRPC(i)
			wg.Done()
		}()
	}
	wg.Wait()
}

func sendRPC(i int) {
	println(i)
}
