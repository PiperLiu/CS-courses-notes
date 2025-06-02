package main

import "time"
import "math/rand"

func main() {
	rand.Seed(time.Now().UnixNano())

	c := make(chan bool)

	for i := 0; i < 10; i++ {
		go func() {
			c <- requestVote()
		}()
	}

	count := 0
	finished := 0

	for {
		vote := <-c
		if vote {
			count++
		}
		finished++
		if count > 5 {
			break // XXX
		}
		if finished == 10 {
			break
		}
	}
	if count >= 5 {
		println("received 5+ votes!")
	} else {
		println("lost")
	}
}

func requestVote() bool {
	time.Sleep(time.Duration(rand.Intn(100)) * time.Millisecond)
	return rand.Int() % 2 == 0
}
