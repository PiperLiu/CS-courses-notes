package main

import "sync"

func main() {
	var a string
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		a = "hello world"
		wg.Done()
	}()
	wg.Wait()
	println(a)
}
