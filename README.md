# Projekat-Orm (Peer-to-Peer File Sharing System (TCP))
📌 Overview

This project implements a Peer-to-Peer (P2P) file sharing system in C using TCP sockets and POSIX threads.
It follows a tracker–peer architecture where a central tracker manages metadata, while all file transfers occur directly between peers.

🧱 Architecture
Tracker (Control Plane)

Tracks which peer owns which file segments

Handles:

GET <segment_id> → returns ip:port of a peer

UPLOAD <segment_id> → registers segment ownership

Never transfers file data

Peer

Each peer has two roles:

Peer Server (thread): sends binary file segments to other peers

Peer Client (main thread): communicates with the tracker and downloads segments

🔌 Network Design
Component	Purpose	Port
Tracker	Metadata & coordination	16666
Peer Server	Segment transfer	Peer-specific

Each socket has a single responsibility, ensuring a clean and deadlock-free design.

📂 File Segmentation

File is split into fixed-size binary segments (512 bytes)

Stored as:
segments/segment_0.dat, segment_1.dat, …

Transfers use raw binary send() / recv()

🔒 Concurrency

Tracker: multi-threaded (one thread per peer), protected by a mutex

Peer:

One server thread

One main client thread

No shared memory between peers

🛠️ Build & Run
gcc tracker.c segment.c -o tracker -lpthread
gcc peer.c -o peer -lpthread

./tracker
./peer   # run in multiple terminals

📡 Protocol

Tracker (text):

GET <segment_id>

UPLOAD <segment_id>

Peer-to-peer (binary):

Peer sends: int segment_id (network byte order)

Peer server responds: raw segment data

⚠️ Known Issues

No fault tolerance for peer disconnects

No checksum or integrity verification

Single-segment downloads only (no swarming)

Simple peer selection (no load balancing)

No socket timeouts

Static limits on peers and segments

Tracker is a single point of failure
