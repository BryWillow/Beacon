## Why Beacon?
The word "Beacon" represents a guiding light—a source of hope for solving tough challenges. This project serves as a platform to explore, innovate, and measure progress in the HFT landscape.

## Project Overview
The Beacon project is an ecosystem of modern C++ applications, each designed with a single purpose yet working seamlessly together. It functions as a live back-tester, and ***so much more***.

## Key Features
You are in control of the following:
- The entire market-place. The symbols and their movement correlations.
- Generating highly-tailored market data for building custom order books.
- Market data speed playback: microbursts, speed factors, real-time.
- A simple way to test out new strategies.
- Highly-performant, tested, lock-free data structures.
- Latency measurements of each component.
- An entire system that abides by exchange specifications.
- Keeping things simple: 1-n symbols, 1-n messages

## Beacon Core Applications and Workflow
|Core App  | Purpose | Communication Type |
| --: | :-- | :--
|<a href="https://github.com/BryWillow/Beacon/wiki/md_creator">md_creator</a> | Generates your custom datasets. | File I/O.
|<a href="https://github.com/BryWillow/Beacon/wiki/md_server">md_server</a> | Market Data Exchange. Broadcasts your custom datasets. | File I/O (in), UDP (out). 
|<a href="https://github.com/BryWillow/Beacon/wiki/md_client"> md_client</a>| Your Feed Handler and Algo. Sends order decisions (if any). | UDP (in) / TCP (out).
|<a href="https://github.com/BryWillow/Beacon/wiki/ex_match">ex_match</a> | Exchange Matching Engine. Decodes, acks, matches, etc. | TCP (in) / TCP (out). 

## Building Beacon
Beacon provides a script `build.py` script at the top level of the repository. This is the simplest way to build the entire repository. Note that the *all* keyword specified below simply means to build both **debug** and **release** builds. 
Usage:
```bash
./build.py all
```
<a href="https://github.com/BryWillow/Beacon/wiki/Building-Beacon">Building Beacon</a> provides a comprehensive list of command-line options.

## Running Beacon
- Running Beacon is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.
- Analyzing Beacon stats is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.

## Project Diagram
<pre>
        +--------------------------------------------------------------------+
        | FILE GENERATOR                                       (./md_create) |
        |  • To run: ./bin/md_create                                         |
        |  • Creates raw, binary output files, to be consumed by .md_server. |
        |  • File contents abide by the exchange spec, identically.          |                                      
        |  • Flexible file creation options. See usage for details.          |
        +----------------+---------------------------------------------------+
                         | (Creates)
                         |  
                         v
        +--------------------------------------------------------------------+
        | BINARY FILE (/data/<distinct_file_name>.dat)                                           |
        |  • Created by FILE GENERATOR                                       |
        |  • Read by EXCHANGE MARKET DATA SERVER               (./md_server) |
        +<---------------->+-------------------------------------------------+
                         ^
                         |
                         | (Reads from generated file.)
        +--------------------------------------------------------------------+
        | MARKET DATA EXCHANGE                                 (./md_server) |
        |  • Reads the binary file at startup, avoids file I/O latency       |
        |  • Acts as an Exchange, e.g., NSDQ, CME, NYSE                      |
        |  • Broadcasts data via UDP to (./md_client)                        |
        |  • Highly flexible playback option, with bursts, etc. See usage.   |
        +----------------+---------------------------------------------------+
                         | (Broadcasts generated file contents via UDP.)
                         |
                         | 
                         v (Receives raw exchange data from generated file.)
        +--------------------------------------------------------------------+
        | MARKET DATA CLIENT -- YOUR ALGO --                   (./md_client) |
        | • Receives and decodes raw exchange packets.                       |
        | • Your algorithm lives here and makes decisions.                   |
        | • Your algorithm submits new, udpdates and cancels orders via TCP. |                                 
        +----------------+---------------------------------------------------+
          (Sub/Mod/Cxl)  | (TCP) ^
                         |       |
                         |       |   
                         v       | (All / Exec Rpt)
        +----------------+------------------------------------------------------+
        | EXCHANGE MATCHING ENGINE                                 (./ex_match) |
        | • Receives algorithm submissions. Acks them.                          |
        | • Modifies, Cancels, Orders, Acts Operations.                         |
        | • Sends Execution reports when an order is partially or fully filled. |
        +-----------------------------------------------------------------------+
</pre>
