package main

import "time"
import "fmt"

func main() {
	c := make(chan bool, 1)
	go func() {
		time.Sleep(1 * time.Second)
		<-c
	}()
	start := time.Now()
	c <- true
	fmt.Printf("send took %v\n", time.Since(start))

	start = time.Now()
	c <- true
	fmt.Printf("send took %v\n", time.Since(start))
}
