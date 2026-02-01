# Projekat-Orm (Peer-to-Peer File Sharing System (TCP))
This project implements a Peer-to-Peer (P2P) file sharing system in C using TCP sockets and POSIX threads.
The system is based on a tracker–peer architecture, where a central tracker coordinates peers, while the actual file transfer happens directly between peers.

📌 System Architecture

The system consists of three clearly separated roles:

1️⃣ Tracker (Central Server)

The tracker is responsible for metadata management only.

Responsibilities:

Keeps track of which peer owns which file segments

Handles peer requests:

GET <segment_id> – returns ip:port of a peer that owns the segment

UPLOAD <segment_id> – registers that a peer now owns a segment

Does not transfer file data

The tracker acts as a control plane, not a data source.

2️⃣ Peer Server

Each peer runs its own lightweight peer server in a separate thread.

Responsibilities:

Listens on a dedicated TCP port

Sends requested file segments to other peers

Transfers raw binary data (no text protocol)

Peer servers form the data plane of the system.

3️⃣ Peer Client

The peer client logic coordinates communication.

Workflow:

Connects to the tracker

Requests a segment (GET)

Receives peer address (ip:port)

Connects directly to the peer server

Downloads the segment

Notifies the tracker (UPLOAD)

🔌 Network Design
Component	Purpose	Port
Tracker	Metadata & coordination	16666
Peer Server	Segment transfer	Random / peer-specific

Each socket connection has exactly one responsibility, ensuring a clean and deadlock-free design.

📂 File Segmentation

The original file is split into fixed-size segments (default: 512 bytes)

Segments are stored as binary files:

segments/segment_0.dat
segments/segment_1.dat
...


Transfers are done using binary send() / recv() calls

🔒 Concurrency & Thread Safety

The tracker is multi-threaded

One thread per connected peer

Shared metadata protected using a mutex

Peers use:

One thread for the peer server

One main thread for user interaction

No shared memory between peers → no mutex needed on the peer side

🛠️ Build & Run
Compile
gcc tracker.c segment.c -o tracker -lpthread
gcc peer.c -o peer -lpthread

Run Tracker
./tracker

Run Peers (in separate terminals)
./peer

📡 Protocol Overview
Tracker Protocol (Text-based)
GET <segment_id>
UPLOAD <segment_id>

Peer-to-Peer Protocol (Binary)

Peer sends:

int segment_id (network byte order)


Peer server responds:

raw binary segment data

🎯 Key Design Principles

Clear separation of concerns

Tracker never sends file data

Peers communicate directly

One socket = one responsibility

Thread-safe shared state

Modular and extensible architecture

🚀 Possible Extensions

Downloading segments from multiple peers (swarming)

Load balancing / peer selection strategies

Checksum-based segment verification

Peer disconnect handling

Replication strategies

⚠️ Known Issues

No fault tolerance
If a peer disconnects unexpectedly, the tracker is not notified and may still advertise that peer as a valid segment owner.

No verification of segment integrity
Downloaded segments are not validated using checksums or hashes, so corrupted transfers are not detected.

Single-segment download at a time
A peer downloads one segment at a time. Parallel downloads (swarming) are not implemented.

Simple peer selection strategy
The tracker always returns the first peer found with the requested segment. There is no load balancing or randomness.

No timeout handling
Socket operations (connect, recv, send) do not use timeouts, which may cause blocking in edge cases.

Static limits
The maximum number of peers and segments is fixed at compile time.

Tracker is a single point of failure
If the tracker goes down, peers cannot discover each other, even if they already have segments.
