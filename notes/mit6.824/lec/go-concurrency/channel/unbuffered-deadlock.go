package main

func main() {
	c := make(chan bool)
	c <- true
	<-c
}
