- FNV-1A using iterators, can be done at UART receive time if over the encoded
  data
- COBS using iterators, cannot be done at receive time as it might need to
  reject the frame, so need a temporary frame receive buffer first
- Lamport-queue for packet bytes
