# Projekat-Orm(Peer-to-Peer File Sharing System TCP)
Overview

This project implements a Peer-to-Peer (P2P) file sharing system in C using TCP sockets and POSIX threads.  
It follows a tracker–peer architecture where a central tracker manages metadata, while all file transfers occur directly between peers.



Architecture

Tracker  
tracks which peer owns which file segments

Handles:  
GET <segment_id> → returns ip:port of a peer  
UPLOAD <segment_id> → registers segment ownership  
Never transfers file data  


Peer  
Each peer has two roles:  
Peer Server (thread): sends binary file segments to other peers  
Peer Client (main thread): communicates with the tracker and downloads segments  



File Segmentation  
File is split into fixed-size binary segments (512 bytes)



Concurrency  
Each socket has a single responsibility, ensuring a clean and deadlock-free design.  
Tracker: multi-threaded (one thread per peer), protected by a mutex  

Peer:  
One server thread  
One main client thread  
No shared memory between peers  

Build & Run  
- `make all` – compiles everything and creates the executables `tracker` and `peer`.
- You can also compile separately using `make tracker` or `make peer`.
- `make clean` – removes all generated files.

 ./tracker  
 ./peer   # run in multiple terminals


Known Issues:

No fault tolerance for peer disconnects

No checksum or integrity verification

Single-segment downloads only (no swarming)

Simple peer selection (no load balancing)

No socket timeouts

Static limits on peers and segments

Tracker is a single point of failure
