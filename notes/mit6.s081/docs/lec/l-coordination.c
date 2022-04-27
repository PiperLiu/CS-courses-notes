//
// uart.c
// these functions replace uartputc() and uartintr()
//

static int tx_done; // has the UART finished sending?
static int tx_chan; // &tx_chan is the "wait channel"

// transmit buf[].
void
uartwrite(char buf[], int n)
{
  acquire(&uart_tx_lock);

  int i = 0;
  while(i < n){
    while(tx_done == 0){
      // UART is busy sending a character.
      // wait for it to interrupt.
      sleep(&tx_chan, &uart_tx_lock);
    }
    
    WriteReg(THR, buf[i]);
    i += 1;
    tx_done = 0;
  }

  release(&uart_tx_lock);
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from trap.c.
void
uartintr(void)
{
  acquire(&uart_tx_lock);
  if(ReadReg(LSR) & LSR_TX_IDLE){
    // UART finished transmitting; wake up any sending thread.
    tx_done = 1;
    wakeup(&tx_chan);
  }
  release(&uart_tx_lock);

  // read and process incoming characters.
  while(1){
    int c = uartgetc();
    if(c == -1)
      break;
    consoleintr(c);
  }
}

//
// proc.c
//
// (this sleep() does not work correctly)
//

void
broken_sleep(void *chan)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  acquire(&p->lock);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;
  release(&p->lock);
}
