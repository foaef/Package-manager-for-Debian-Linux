# baciu manager
**A lightweight package manager for Debian Linux (Bookworm).**

## Features
* **Index Management:** Downloads and parses the Debian Bookworm package index.
* **Search:** Fast keyword searching using `grep`.
* **Installation:** Handles package downloads via `libcurl` and installation via `dpkg`.
* **Lightweight:** Small tool

## Prerequisites
To build or run `baciu`, you will need:
* `libcurl` (for downloads)
* `zlib` / `gunzip` (to extract the index)
* `dpkg` (for package management)
* `grep` (for searching)

## Installation
```bash
git clone [https://github.com/foaef/Package-manager-for-Debian-Linux.git](https://github.com/foaef/Package-manager-for-Debian-Linux.git)
cd Package-manager-for-Debian-Linux
make
sudo make install
