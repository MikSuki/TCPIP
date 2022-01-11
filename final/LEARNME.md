# ch.17 TCP overview
<img src="img/TCP_header.png" style="zoom:100%"/>
  
<br>

# ch.18 TCP Connection Establishment and  Termination

- Three-way handshaking
    + 重點是 Ack number 都是對方 seq + 1
  <img src="img/3way_handshaking.png" style="zoom:100%"/>

- terminate by FIN
    + FIN -> ACK -> FIN -> ACK <br>
  <img src="img/TCP_terminate.png" style="zoom:100%"/>
    

- MSS
- half-close
- TCP state transiton 
    +  client <br>
        * FIN_WAIT_1 
        * FIN_WAIT_2
  <img src="img/TCP_terminate_client.png" style="zoom:100%"/>
    +  server <br>
  <img src="img/TCP_terminate_server.png" style="zoom:100%"/>
- 2MSL Wait State
    + MSL
    + 避免最後的 FIN ACK 遺失
- quiet time
  
<br>

# ch.19 TCP Interactive Data Flow
- Tinygrams
- delayed acknowledges
    + piggyback
- Nagle algorithm
  + outstanding data
  + self-clocking
  
<br>

# ch.20 TCP Bulk Data Flow
- Fast Sender, Slow Receiver => win 0
- push flag
- Sliding Windows
- Slow Start
    + algorithm
      * cwnd &nbsp;(sender's flow control)
      * win &nbsp;&nbsp;&nbsp; (receiver’s flow control)
    + exponential increase
- bandwidth-delay product
- urgent mode
    + out-of-band

<br>

# ch.21 TCP Timeout and Retransmission
- RTO 
    + how to calculate RTO ?
    + RTT
    + Karn’s Algorithm
        * retransmission ambiguity problem
- Congestion
    + Jacobson‘s fast retransmit algorithm
    
- Congestion Avoidence
    + 2 indications of packet loss
    + algorithm 
        * cwnd
        * ssthresh
        * 3 duplicate ack -> slow start
        * TIMEOUT -> congestion
    <img src="img/TCP_congestion.png" style="zoom:100%"/>
    + cwnd ← cwnd＋(segsize × segsize)／cwmd＋ segsize ／ 8

- Fast Retransmit
    + retransmit timer
    + 3 ack duplicate 
- Fast Recovery 
    + 3 duplicate ack後，不進入 slow start，且減半 ssthresh
- Repacketization
- Congestion control
    1. slow start
    2. congestion avoidence
    3. fast retransmit
    4. fast recovery

<br>

# ch.22 TCP Persist Timer
- persist timer
    + when use ?
    + window probes 
- Silly Window Syndrome

<br>

# ch.23 TCP Keepalive Timer
- keepalive timer
- 4 different scenarios

<br>

# ch.24 TCP Futures and Performance
- Long fat pipes 
    + Packets loss in an LFN can reduce throughput drastically. (why?)
- Big Packets or Small Packets?
    + give an example
- PAWS
- TCP Performance 
    + BW = <font color=red>Window / RTT </font>
- TCP受到1G和光速限制


